#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *damage_type_string[3]={"real","physical","magical"};
static const char *types_string[21]={"Void","Grass","Fire","Water","Steel","Light","Fighting","Wind","Poison","Rock","Electric","Ghost","Ice","Bug","Machine","Soil","Dragon","Normal","Devine grass","Alkali fire","Devine water"};
const char *type2str(int type){
	int index=type?__builtin_ctz(type)+1:0;
	if(index>=21)
		return "Unknown";
	return types_string[index];
}
int unit_kill(struct unit *up){
	if(up->hp)return -1;
	switch(up->state){
		case UNIT_FAILED:
		case UNIT_FREEZING_ROARINGED:
			return -1;
		default:
			break;
	}
	up->state=UNIT_FAILED;
	printf("%s failed\n",up->base.name);
	return 0;
}
unsigned long damage(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag){
	char buf[32],*p;
	switch(damage_type){
		case DAMAGE_REAL:
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return 0;
	}
	if(dest->hp>value){
		dest->hp-=value;
	}else {
		dest->hp=0;
	}
	if(aflag&(AF_CIRT|AF_EFFECT|AF_WEAK)){
		strcpy(buf," (");
		if(aflag&AF_CIRT)
			strcat(buf,"cirt,");
		if(aflag&AF_EFFECT)
			strcat(buf,"effect,");
		if(aflag&AF_WEAK)
			strcat(buf,"weak,");
		p=buf+strlen(buf);
		strcpy(p-1,") ");
	}else strcpy(buf," ");
	if(src)printf("%s get %lu %s damage%sfrom %s,current hp:%lu\n",dest->base.name,value,damage_type_string[damage_type],buf,src->base.name,dest->hp);
	else printf("%s get %lu %s damage%s,current hp:%lu\n",dest->base.name,value,damage_type_string[damage_type],buf,dest->hp);
	if(!dest->hp)
		unit_kill(dest);
	return value;
}
unsigned long attack(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	long x;
	int dest_type;
	double derate;
	switch(damage_type){
		case DAMAGE_REAL:
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return 0;
	}
	if(aflag&AF_CIRT)
		value=value*src->cirt_effect;

	if(damage_type!=DAMAGE_REAL&&!(aflag&(AF_EFFECT|AF_WEAK))){
		dest_type=dest->type0|dest->type1;
		x=__builtin_popcount(dest_type&effect_types(type))
		-__builtin_popcount(dest_type&weak_types(type));
		if(x<0){
			value/=(1-x);
			aflag|=AF_WEAK;
		}else if(x>0){
			if(x>1&&(type&TYPES_DEVINE))
				x=1;
			value*=1+x;
			aflag|=AF_EFFECT;
		}
	}
	if(!(aflag&AF_NODEF)&&damage_type!=DAMAGE_REAL){
		x=512+dest->base.level-src->base.level+dest->def;
		if(x<=0)x=1;
		value=value*512/x;
	}
	switch(damage_type){
		case DAMAGE_PHYSICAL:
			derate=dest->physical_derate-src->physical_bonus;
			break;
		case DAMAGE_MAGICAL:
			derate=dest->magical_derate-src->magical_bonus;
			break;
		case DAMAGE_REAL:
			derate=0;
			break;
	}
	if(derate>0.8)
		value/=5*derate+1;
	else
		value*=1-derate;
	if(!(aflag&AF_NOFLOAT)&&damage_type!=DAMAGE_REAL)
		value+=(0.1*drand48()-0.05)*value;
	return damage(dest,src,value,damage_type,aflag);
}

int hittest(struct unit *dest,struct unit *src,double hit_rate){
	double real_rate;
	int x=dest->avoid;
	if(x<=0)x=1;
	real_rate=hit_rate*src->hit/x;
	//printf("rate:%lf real:%lf\n",hit_rate,real_rate);
	x=drand48()<real_rate;
	if(!x){
		if(src)printf("%s missed (target: %s)\n",src->base.name,dest->base.name);
		else printf("missed (target: %s)\n",dest->base.name);
	}
	return x;
}
int test(double prob){
	return drand48()<prob;
}
unsigned long normal_attack(struct unit *dest,struct unit *src){
	if(!hittest(dest,src,1.0))
		return 0;
	return attack(dest,src,src->atk,DAMAGE_PHYSICAL,AF_NORMAL,src->type0);
}
unsigned long heal(struct unit *dest,struct unit *src,unsigned long value){
	unsigned long hp;
	hp=dest->hp+value;
	if(hp>dest->base.max_hp)
		hp=dest->base.max_hp;
	dest->hp=hp;
	printf("%s heals %lu hp,current hp:%lu\n",dest->base.name,value,hp);
	return value;
}
unsigned long sethp(struct unit *dest,unsigned long hp){
	unsigned long ohp=dest->hp;
	int c;
	if(hp>dest->base.max_hp)
		hp=dest->base.max_hp;
	if(hp==ohp)
		return hp;
	dest->hp=hp;
	c=hp>ohp?'+':'-';
	printf("%s %c%lu hp,current hp:%lu\n",dest->base.name,c,hp>ohp?hp-ohp:ohp-hp,hp);
	if(!hp)
		unit_kill(dest);
	return hp;
}

unsigned long addhp(struct unit *dest,long hp){
	long x=(long)dest->hp+hp;
	if(hp<0&&x<0)
		x=0;
	return sethp(dest,x);
}
struct unit *gettarget(struct unit *u){
	return u->owner->enemy->front;
}
int rand_selector(struct player *p){
	int x;
redo:
	x=lrand48()%9;
	if(x==8)
		return 8;
	if(p->front->moves[x].id==NULL)
		goto redo;
	return x;
}
int manual_selector(struct player *p){
	char buf[32],*endp;
	int r,n=0;
reselect:
	printf("Select the action of %s\n",p->front->base.name);
	printf("a: Normal attack (%s)\n",type2str(p->front->type0));
	for(r=0;r<8;++r){
		if(!p->front->moves[r].id)
			break;
		printf("%d: %s (%s)\n",r,p->front->moves[r].name,type2str(p->front->moves[r].type));
		++n;
	}
	printf(">");
	fgets(buf,32,stdin);
	endp=strchr(buf,'\n');
	if(endp)*endp=0;
	switch(*buf){
		case 'A':
		case 'a':
			if(buf[1]!=0)
				goto unknown;
			return ACT_NORMALATTACK;
		case '0' ... '9':
			r=strtol(buf,&endp,10);
			if(r>=n||*endp)goto unknown;
			return r;
		default:
unknown:
			printf("Unkonwn action: %s\n",buf);
			goto reselect;
	}
}
