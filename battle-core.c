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
int unit_kill(struct unit *u){
	if(!isalive(u->state))
			return -1;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->kill)
			e->base->kill(e,u);
	}
	if(u->hp)return -1;
	u->state=UNIT_FAILED;
	printf("%s failed\n",u->base->id);
	for_each_effect(e,u->owner->field->effects){
		if(e->base->kill_end)
			e->base->kill_end(e,u);
	}
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
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->damage&&e->base->damage(e,dest,src,&value,&damage_type,&aflag,&type))
			return 0;
	}
	if(dest->hp>value){
		dest->hp-=value;
	}else {
		dest->hp=0;
	}
	if((aflag&(AF_CRIT|AF_EFFECT|AF_WEAK))||type){
		strcpy(buf," (");
		if(type){
			strcat(buf,type2str(type));
			strcat(buf,",");
		}
		if(aflag&AF_CRIT)
			strcat(buf,"crit,");
		if(aflag&AF_EFFECT)
			strcat(buf,"effect,");
		if(aflag&AF_WEAK)
			strcat(buf,"weak,");
		p=buf+strlen(buf);
		strcpy(p-1,") ");
	}else strcpy(buf," ");
	if(src)printf("%s get %lu %s damage%sfrom %s,current hp:%lu\n",dest->base->id,value,damage_type_string[damage_type],buf,src->base->id,dest->hp);
	else printf("%s get %lu %s damage%s,current hp:%lu\n",dest->base->id,value,damage_type_string[damage_type],buf,dest->hp);
	if(!dest->hp)
		unit_kill(dest);
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->damage_end)
			e->base->damage_end(e,dest,src,value,damage_type,aflag,type);
	}
	return value;
}
unsigned long attack(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	long x;
	unsigned long ret;
	double derate;
	int dest_type;
	switch(damage_type){
		case DAMAGE_REAL:
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return 0;
	}
	if(!isalive(dest->state))
			return -1;
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->attack&&e->base->attack(e,dest,src,&value,&damage_type,&aflag,&type))
			return 0;
	}
	if(aflag&AF_CRIT){
		if(src){
			if(src->crit_effect>=0.5)
				value*=src->crit_effect;
			else
				value/=3-2*src->crit_effect;
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
		if(src)x+=dest->base->level-src->base->level;
		if(x<=0)
			value*=1-dest->def;
		else
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
			goto no_derate;
			break;
	}
	if(derate>0.8)
		value/=5*derate+1;
	else
		value*=1-derate;
no_derate:
	if(!(aflag&AF_NOFLOAT)&&damage_type!=DAMAGE_REAL)
		value+=(0.1*drand48()-0.05)*value;
	ret=damage(dest,src,value,damage_type,aflag,type);
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->attack_end)
			e->base->attack_end(e,dest,src,value,damage_type,aflag,type);
	}
	return ret;
}

int hittest(struct unit *dest,struct unit *src,double hit_rate){
	double real_rate;
	int x;
	if(!isalive(dest->state))
			return 0;
	x=dest->avoid;
	if(x<=0)x=1;
	real_rate=hit_rate*src->hit/x;
	//printf("rate:%lf real:%lf\n",hit_rate,real_rate);
	x=drand48()<real_rate;
	if(!x){
		if(src)printf("%s missed (target: %s)\n",src->base->id,dest->base->id);
		else printf("missed (target: %s)\n",dest->base->id);
	}
	return x;
}
int test(double prob){
	return drand48()<prob;
}
double rand01(void){
	return drand48();
}
long randi(void){
	return lrand48();
}
unsigned long normal_attack(struct unit *dest,struct unit *src){
	if(!hittest(dest,src,1.0))
		return 0;
	return attack(dest,src,src->atk,DAMAGE_PHYSICAL,AF_NORMAL,src->type0);
}
unsigned long heal(struct unit *dest,unsigned long value){
	unsigned long hp;
	if(!isalive(dest->state))
			return 0;
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->heal&&e->base->heal(e,dest,&value))
			return 0;
	}
	hp=dest->hp+value;
	if(hp>dest->base->max_hp)
		hp=dest->base->max_hp;
	dest->hp=hp;
	printf("%s heals %lu hp,current hp:%lu\n",dest->base->id,value,hp);
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->heal_end)
			e->base->heal_end(e,dest,value);
	}
	return value;
}
void instant_death(struct unit *dest){
	damage(dest,NULL,dest->base->max_hp*64,DAMAGE_REAL,0,TYPE_VOID);
}
unsigned long sethp(struct unit *dest,unsigned long hp){
	unsigned long ohp;
	if(!isalive(dest->state))
			return 0;
	ohp=dest->hp;
	if(hp>dest->base->max_hp)
		hp=dest->base->max_hp;
	if(hp==ohp)
		return hp;
	dest->hp=hp;
	printf("%s %+ld hp,current hp:%lu\n",dest->base->id,(long)hp-(long)ohp,hp);
	if(!hp)
		unit_kill(dest);
	return hp;
}

unsigned long addhp(struct unit *dest,long hp){
	long ohp,rhp;
	if(!hp)
		return 0;
	if(!isalive(dest->state))
			return 0;
	ohp=(long)dest->hp;
	rhp=hp+ohp;
	if(hp<0&&rhp<0)
		rhp=0;
	if((unsigned long)rhp>dest->base->max_hp)
		rhp=dest->base->max_hp;
	dest->hp=rhp;
	printf("%s %+ld hp,current hp:%lu\n",dest->base->id,hp,(unsigned long)rhp);
	if(!rhp)
		unit_kill(dest);
	return rhp;
}
long setspi(struct unit *dest,long spi){
	long ospi;
	if(!isalive(dest->state))
			return 0;
	ospi=dest->spi;
	if(spi>(long)dest->base->max_spi)
		spi=(long)dest->base->max_spi;
	if(spi<-(long)dest->base->max_spi)
		spi=-(long)dest->base->max_spi;
	if(spi==ospi)
		return spi;
	dest->spi=spi;
	printf("%s %+ld spi,current spi:%ld\n",dest->base->id,spi-ospi,spi);
	addhp(dest,-(SHEAR_COEF*dest->base->max_hp+1)*labs(spi-ospi));
	return spi;
}
struct unit *gettarget(struct unit *u){
	return u->owner->enemy->front;
}
int setcooldown(struct move *m,int round){
	m->cooldown=round;
	return round;
}
struct effect *effect(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round){
	struct effect *ep=NULL,*ep1;
	struct battle_field *f=NULL;
	int new;
	if(dest)
		f=dest->owner->field;
	else if(src)
		f=src->owner->field;
	if(!f)
		return NULL;
	for_each_effect(e,f->effects){
		if(e->base->effect&&e->base->effect(e,base,dest,src,&level,&round))
			return NULL;
	}
	for_each_effect(e,f->effects){
		if(dest&&e->dest==dest&&e->base==base){
			ep=e;
			break;
		}
	}
	if(!ep){
		ep=malloc(sizeof(struct effect));
		if(!ep)
			return NULL;
		memset(ep,0,sizeof(struct effect));
		new=1;
		ep->base=base;
		ep->dest=dest;
		ep->src=src;
	}else
		new=0;
	if(base->init){
		if(base->init(ep,level,round)){
			effect_end(ep);
			return NULL;
		}
	}else {
		ep->level=level;
		ep->round=round;
	}
	if(dest){
		if(src)printf("%s get effect %s(%+ld) from %s in %d rounds\n",dest->base->id,ep->base->id,ep->level,src->base->id,ep->round);
		else printf("%s get effect %s(%+ld) in %d rounds\n",dest->base->id,ep->base->id,ep->level,ep->round);
	}
	if(new){
		ep1=f->effects;
		f->effects=ep;
		ep->next=ep1;
		if(ep1)
			ep1->prev=ep;
	}

	if(base->inited)
		base->inited(ep);
	for_each_effect(e,f->effects){
		if(e->base->effect_end)
			e->base->effect_end(e,base,dest,src,level,round);
	}
	return ep;
}
int effect_end(struct effect *e){
	struct battle_field *f=NULL;
	if(e->dest)
		f=e->dest->owner->field;
	else if(e->src)
		f=e->src->owner->field;
	if(!f)
		return -1;
	if(e->prev){
		e->prev->next=e->next;
	}else {
		f->effects=e->next;
	}
	if(e->next)
		e->next->prev=e->prev;
	if(e->base->end)
		e->base->end(e);
	if(e->dest)
		printf("the effect %s(%+ld) of %s was removed\n",e->base->id,e->level,e->dest->base->id);
	free(e);
	return 0;
}

int purify(struct effect *e){
	if(e->base->flag&EFFECT_UNPURIFIABLE)
		return -1;
	return effect_end(e);
}

void effect_event(struct effect *e){
	printf("effect event %s\n",e->base->id);
}
void effect_event_end(struct effect *e){
	printf("effect event %s end\n",e->base->id);
}
int revive(struct unit *u,unsigned long hp){
	if(u->state!=UNIT_FAILED||!hp)
		return -1;
	u->state=UNIT_NORMAL;
	sethp(u,hp);
	update_state(u);
	return 0;
}
int event(const struct event *ev,struct unit *src){
	printf("event %s",ev->id);
	if(src)
		printf(" caused by %s\n",src->base->id);
	else
		printf("\n");
	if(ev->action)
		ev->action(ev,src);
	printf("event %s end\n",ev->id);
	return 0;
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

void update_attr(struct unit *u){
	u->atk=u->base->atk;
	u->def=u->base->def;
	u->speed=u->base->speed;
	u->hit=u->base->hit;
	u->avoid=u->base->avoid;
	u->crit_effect=u->base->crit_effect;
	u->physical_bonus=u->base->physical_bonus;
	u->magical_bonus=u->base->magical_bonus;
	u->physical_derate=u->base->physical_derate;
	u->magical_derate=u->base->magical_derate;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->update_attr)
			e->base->update_attr(e,u);
	}
}

void update_state(struct unit *u){
	int r;
	if(!isalive(u->state))
		return;
	r=UNIT_NORMAL;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->update_state){
			e->base->update_state(e,u,&r);
		}
	}
	u->state=r;
}
void effect_in_roundstart(struct effect *effects){
	for_each_effect(e,effects){
		if(e->base->roundstart)
			e->base->roundstart(e);
	}
}
void effect_in_roundend(struct effect *effects){
	for_each_effect(e,effects){
		if(e->base->roundend)
			e->base->roundend(e);
	}
}
void unit_effect_in_roundend(struct unit *u){
	for_each_effect(e,u->owner->field->effects){
		if(e->dest==u&&e->base->roundend)
			e->base->roundend(e);
	}
}

void unit_effect_round_decrease(struct unit *u,int round){
	struct battle_field *f=u->owner->field;
	for_each_effect(v,f->effects){
		if(v->dest!=u)
			continue;
		if(v->round>0){
			if(v->round>round)
				v->round-=round;
			else
				v->round=0;
		}
		if(!v->round)
			effect_end(v);
	}
}
void effect_round_decrease(struct effect *effects,int round){
	for_each_effect(v,effects){
		if(v->round>0){
			if(v->round>round)
				v->round-=round;
			else
				v->round=0;
		}
		if(!v->round)
			effect_end(v);
	}
}
struct effect *unit_findeffect(struct unit *u,const struct effect_base *base){
	for_each_effect(e,u->owner->field->effects){
		if(e->base==base&&e->dest==u)
			return e;
	}
	return NULL;
}
int effect_isnegative(const struct effect *e){
	if(e->base->flag&(EFFECT_NEGATIVE|EFFECT_ABNORMAL|EFFECT_CONTROL))
		return 1;
	else if(e->base->flag&EFFECT_POSITIVE)
		return 0;
	else if(e->base->flag&EFFECT_ATTR)
		return e->level<0;
	else
		return e->src&&e->dest&&e->src->owner!=e->dest->owner;
}
int unit_hasnegative(struct unit *u){
	for_each_effect(e,u->owner->field->effects){
		if(e->dest==u&&effect_isnegative(e))
			return 1;
	}
	return 0;
}
int unit_move(struct unit *u,struct move *m){
	struct move *backup=u->move_cur;
	if(!isalive(u->state))
			return 0;
	u->move_cur=m;
	printf("%s uses %s (%s)\n",u->base->id,m->id,type2str(m->type));
	m->action(u);
	u->move_cur=backup;
	return 0;
}
int canaction2(struct player *p,int act){
	struct move *m;
	switch(act){
		case ACT_MOVE0 ... ACT_MOVE7:
			m=p->front->moves+act;
			if(!m->id||(m->cooldown&&!(m->mlevel&MLEVEL_FREEZING_ROARING)))
				return 0;
			switch(p->front->state){
				case UNIT_CONTROLLED:
					if(m->flag&MF_NOCONTROL)
						return 1;
				case UNIT_SUPPRESSED:
					if(m->mlevel&MLEVEL_FREEZING_ROARING)
						return 1;
				case UNIT_FAILED:
				case UNIT_FREEZING_ROARINGED:
					return 0;
				default:
					return 1;
			}
		case ACT_NORMALATTACK:
			switch(p->front->state){
				case UNIT_CONTROLLED:
				case UNIT_SUPPRESSED:
				case UNIT_FAILED:
				case UNIT_FREEZING_ROARINGED:
					return 0;
				default:
					return 1;
			}
		case ACT_UNIT0 ... ACT_UNIT5:
			if(!p->units[act-ACT_UNIT0].base)
				return 0;
			switch(p->front->state){
				case UNIT_CONTROLLED:
				case UNIT_SUPPRESSED:
					return 0;
				default:
					return 1;
			}
		default:
		case ACT_GIVEUP:
		case ACT_ABORT:
			return 1;
	}
}
struct player *getprior(struct player *p,struct player *e){
	int pp,pe,frp=0,fre=0;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			pp=p->front->moves[p->action].prior;
			if((p->front->moves[p->action].mlevel&MLEVEL_FREEZING_ROARING)&&unit_hasnegative(p->front))
				frp=1;
			break;
		default:
			pp=0;
			break;
	}
	switch(e->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			pe=e->front->moves[e->action].prior;
			if((e->front->moves[e->action].mlevel&MLEVEL_FREEZING_ROARING)&&unit_hasnegative(e->front))
				fre=1;
			break;
		default:
			pe=0;
			break;
	}
	if(frp!=fre){
		return frp?p:e;
	}
	if(pp!=pe){
		return pp>pe?p:e;
	}
	if(p->front->speed!=e->front->speed)
		return p->front->speed>e->front->speed?p:e;
	return test(0.5)?p:e;
}
