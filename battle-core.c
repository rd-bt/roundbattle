#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define SHEAR_COEF (M_SQRT2/128)
static const char *damage_type_string[3]={"real","physical","magical"};
static const char *types_string[21]={"Void","Grass","Fire","Water","Steel","Light","Fighting","Wind","Poison","Rock","Electric","Ghost","Ice","Bug","Machine","Soil","Dragon","Normal","Devine grass","Alkali fire","Devine water"};
const char *type2str(int type){
	unsigned int index=type?__builtin_ctz(type)+1:0;
	if(index>=21)
		return "Unknown";
	return types_string[index];
}
int unit_kill(struct unit *up){
	if(up->hp)return -1;
	if(!isalive(up->state))
			return -1;
	up->state=UNIT_FAILED;
	printf("%s failed\n",up->base.name);
	return 0;
}
unsigned long damage(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	char buf[32],*p;
	switch(damage_type){
		case DAMAGE_REAL:
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return 0;
	}
	if(!isalive(dest->state))
			return 0;
	if(dest->hp>value){
		dest->hp-=value;
	}else {
		dest->hp=0;
	}
	if((aflag&(AF_CIRT|AF_EFFECT|AF_WEAK))||type){
		strcpy(buf," (");
		if(type){
			strcat(buf,type2str(type));
			strcat(buf,",");
		}
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
	if(aflag&AF_CIRT){
		if(src){
			if(src->cirt_effect>=0.5)
				value*=src->cirt_effect;
			else
				value/=3-2*src->cirt_effect;
		}else
			value*=2;
	}

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
		x=512+dest->def;
		if(src)x+=dest->base.level-src->base.level;
		if(x<=0)x=1;
		value=value*512/x;
	}
	switch(damage_type){
		case DAMAGE_PHYSICAL:
			derate=dest->physical_derate;
			if(src)derate-=src->physical_bonus;
			break;
		case DAMAGE_MAGICAL:
			derate=dest->magical_derate;
			if(src)derate-=src->magical_bonus;
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
	return damage(dest,src,value,damage_type,aflag,type);
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
unsigned long heal(struct unit *dest,unsigned long value){
	unsigned long hp;
	if(!isalive(dest->state))
			return -1;
	hp=dest->hp+value;
	if(hp>dest->base.max_hp)
		hp=dest->base.max_hp;
	dest->hp=hp;
	printf("%s heals %lu hp,current hp:%lu\n",dest->base.name,value,hp);
	return value;
}
unsigned long sethp(struct unit *dest,unsigned long hp){
	unsigned long ohp;
	if(!isalive(dest->state))
			return -1;
	ohp=dest->hp;
	if(hp>dest->base.max_hp)
		hp=dest->base.max_hp;
	if(hp==ohp)
		return hp;
	dest->hp=hp;
	printf("%s %+ld hp,current hp:%lu\n",dest->base.name,(long)hp-(long)ohp,hp);
	if(!hp)
		unit_kill(dest);
	return hp;
}

unsigned long addhp(struct unit *dest,long hp){
	long ohp,rhp;
	if(!hp)
		return 0;
	if(!isalive(dest->state))
			return -1;
	ohp=(long)dest->hp;
	rhp=hp+ohp;
	if(hp<0&&rhp<0)
		rhp=0;
	if((unsigned long)rhp>dest->base.max_hp)
		rhp=dest->base.max_hp;
	dest->hp=rhp;
	printf("%s %+ld hp,current hp:%lu\n",dest->base.name,hp,(unsigned long)rhp);
	if(!rhp)
		unit_kill(dest);
	return rhp;
}
long setspi(struct unit *dest,long spi){
	long ospi;
	if(!isalive(dest->state))
			return -1;
	ospi=dest->spi;
	if(spi>(long)dest->base.max_spi)
		spi=(long)dest->base.max_spi;
	if(spi<-(long)dest->base.max_spi)
		spi=-(long)dest->base.max_spi;
	if(spi==ospi)
		return spi;
	dest->spi=spi;
	printf("%s %+ld spi,current spi:%ld\n",dest->base.name,spi-ospi,spi);
	addhp(dest,-SHEAR_COEF*dest->base.max_hp*labs(spi-ospi));
	return spi;
}
struct unit *gettarget(struct unit *u){
	return u->owner->enemy->front;
}
int setcooldown(struct move *m,int round){
	m->cooldown=round;
	return round;
}
void unit_cooldown_decrease(struct unit *u,int round){
	int r,r1;
	for(r=0;r<8;++r){
		if(!u->moves[r].id)
			break;
		r1=u->moves[r].cooldown;
		if(!r1||r1<0)
			continue;
		r1-=round;
		if(r1<0)
			r1=0;
		u->moves[r].cooldown=r1;
	}
}

void unit_abnormal(struct unit *u,int abnormals,int round){
	if(!(abnormals&ABNORMAL_ALL))
		return;
	if(abnormals&ABNORMAL_BURNT){
		u->abnormals.burnt=round;
		printf("%s is burnt in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_POISONED){
		u->abnormals.poisoned=round;
		printf("%s is poisoned in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_PARASITIZED){
		u->abnormals.parasitized=round;
		printf("%s is parasitized in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_CURSED){
		u->abnormals.cursed=round;
		printf("%s is cursed in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_RADIATED){
		u->abnormals.radiated=round;
		printf("%s is radiated in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_ASLEEP){
		u->abnormals.asleep=round;
		printf("%s is asleep in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_FROZEN){
		u->abnormals.frozen=round;
		printf("%s is frozen in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_PARALYSED){
		u->abnormals.paralysed=round;
		printf("%s is paralysed in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_STUNNED){
		u->abnormals.stunned=round;
		printf("%s is stunned in %d rounds\n",u->base.name,round);
	}
	if(abnormals&ABNORMAL_PETRIFIED){
		u->abnormals.petrified=round;
		printf("%s is petrified in %d rounds\n",u->base.name,round);
	}
}
void unit_abnormal_purify(struct unit *u,int abnormals){
	if(!(abnormals&ABNORMAL_ALL))
		return;
	if(abnormals&ABNORMAL_BURNT){
		if(u->abnormals.burnt){
			u->abnormals.burnt=0;
			printf("%s relieves the burnt effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_POISONED){
		if(u->abnormals.poisoned){
			u->abnormals.poisoned=0;
			printf("%s relieves the poisoned effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_PARASITIZED){
		if(u->abnormals.parasitized){
			u->abnormals.parasitized=0;
			printf("%s relieves the parasitized effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_CURSED){
		if(u->abnormals.cursed){
			u->abnormals.cursed=0;
			printf("%s relieves the cursed effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_RADIATED){
		if(u->abnormals.radiated){
			u->abnormals.radiated=0;
			printf("%s relieves the radiate effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_ASLEEP){
		if(u->abnormals.asleep){
			u->abnormals.asleep=0;
			printf("%s relieves the asleep effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_FROZEN){
		if(u->abnormals.frozen){
			u->abnormals.frozen=0;
			printf("%s relieves the frozen effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_PARALYSED){
		if(u->abnormals.paralysed){
			u->abnormals.paralysed=0;
			printf("%s relieves the paralysed effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_STUNNED){
		if(u->abnormals.stunned){
			u->abnormals.stunned=0;
			printf("%s relieves the stunned effect\n",u->base.name);
		}
	}
	if(abnormals&ABNORMAL_PETRIFIED){
		if(u->abnormals.petrified){
			u->abnormals.petrified=0;
			printf("%s relieves the petrified effect\n",u->base.name);
		}
	}
}

#define setattr(a,A)\
	if((attrs&A)&&u->attrs.a!=level){\
		printf("%s %+d levels at " #a " current %+d\n",u->base.name,level-u->attrs.a,level);\
		u->attrs.a=level;\
	}
void unit_attr_set_force(struct unit *u,int attrs,int level){
	setattr(atk,ATTR_ATK)
	setattr(def,ATTR_DEF)
	setattr(speed,ATTR_SPEED)
	setattr(hit,ATTR_HIT)
	setattr(avoid,ATTR_AVOID)
	setattr(cirt_effect,ATTR_CIRTEFFECT)
	setattr(physical_bonus,ATTR_PBONUS)
	setattr(magical_bonus,ATTR_MBONUS)
	setattr(physical_derate,ATTR_PDERATE)
	setattr(magical_derate,ATTR_MDERATE)
	unit_update_attr(u);
}

void unit_attr_set(struct unit *u,int attrs,int level){
	if(level>8)
		level=8;
	else if(level<-8)
		level=-8;
	unit_attr_set_force(u,attrs,level);
}
#define update(a)\
	r=u->base.a;\
	if(u->attrs.a){\
		if(u->attrs.a>0)\
			r+=r*u->attrs.a/4;\
		else\
			r*=pow(0.75,-u->attrs.a);\
	}\
	u->a=r
#define update_d(a,step)\
	d=u->base.a;\
	if(u->attrs.a){\
		d+=u->attrs.a*(step);\
	}\
	u->a=d
void unit_update_attr(struct unit *u){
	unsigned long r;
	double d;
	update(atk);
	update(def);
	update(speed);
	update(hit);
	update(avoid);
	update_d(cirt_effect,0.75);
	update_d(physical_bonus,0.25);
	update_d(magical_bonus,0.25);
	update_d(physical_derate,0.25);
	update_d(magical_derate,0.25);
}
void unit_state_correct(struct unit *u){
	int r;
	if(!isalive(u->state))
		return;
	r=UNIT_NORMAL;
	if(
		u->abnormals.asleep||
		u->abnormals.frozen||
		u->abnormals.paralysed||
		u->abnormals.stunned||
		u->abnormals.petrified
	)
		r=UNIT_CONTROLLED;
	u->state=r;
}
void unit_effect_in_roundend(struct unit *u){
	if(u->abnormals.burnt){
		attack(u,NULL,u->base.max_hp/10,DAMAGE_REAL,0,TYPE_FIRE);
	}
	if(u->abnormals.poisoned){
		attack(u,NULL,u->base.max_hp/10,DAMAGE_REAL,0,TYPE_POISON);
	}
	if(u->abnormals.parasitized){
		heal(u->owner->enemy->front,
			attack(u,NULL,u->base.max_hp/10,DAMAGE_REAL,0,TYPE_GRASS)/2
		);
	}
	if(u->abnormals.cursed){
		attack(u,NULL,u->base.max_hp/5,DAMAGE_REAL,0,TYPE_STEEL);
	}
	if(u->abnormals.radiated){
		attack(u,NULL,u->base.max_hp/5,DAMAGE_REAL,0,TYPE_LIGHT);
	}
}
#define ab_rounddec(x)\
	if(u->abnormals.x>0){\
		if(u->abnormals.x<=round){\
			u->abnormals.x=0;\
			printf("%s relieves the " #x " effect\n",u->base.name);\
		}else\
			u->abnormals.x-=round;\
	}
void unit_effect_round_decrease(struct unit *u,int round){
	ab_rounddec(burnt)
	ab_rounddec(poisoned)
	ab_rounddec(parasitized)
	ab_rounddec(cursed)
	ab_rounddec(radiated)
	ab_rounddec(asleep)
	ab_rounddec(frozen)
	ab_rounddec(paralysed)
	ab_rounddec(stunned)
	ab_rounddec(petrified)
}

void unit_move(struct unit *u,struct move *m,int arg){
	struct move *backup=u->move_cur;
	u->move_cur=m;
	printf("%s uses %s (%s)\n",u->base.name,m->name,type2str(m->type));
	m->action(u,arg);
	u->move_cur=backup;
}
