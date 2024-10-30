#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#define SHEAR_COEF (M_SQRT2/128)
#define printf (use report() instead.)
int unit_kill(struct unit *u){
	if(!isalive(u->state)||u->hp)
			return -1;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->kill&&(e->base->kill(e,u),u->hp))
			return -1;
	}
	if(isalive(u->state))
		u->state=UNIT_FAILED;
	if(u==u->owner->front)
		u->owner->action=ACT_ABORT;
	report(u->owner->field,MSG_FAIL,u);
	unit_wipeeffect(u,EFFECT_KEEP);
	for_each_effect(e,u->owner->field->effects){
		if(e->base->kill_end)
			e->base->kill_end(e,u);
	}
	return 0;
}
unsigned long damage(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	switch(damage_type){
		case DAMAGE_REAL:
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return 0;
	}
	if(!isalive(dest->state)||!value)
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
	report(dest->owner->field,MSG_DAMAGE,dest,src,value,damage_type,aflag,type);
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
	unsigned long value_backup;
	double derate;
	int dest_type,aflag_backup;
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
		if(e->base->attack&&e->base->attack(e,dest,src,&value,&damage_type,&aflag,&type))
			return 0;
	}
	value_backup=value;
	aflag_backup=aflag;
	if(aflag&AF_CRIT){
		if(src){
			if(src->crit_effect>=0.5)
				value*=src->crit_effect;
			else
				value/=3-2*src->crit_effect;
		}else
			value*=2;
	}
	if(damage_type!=DAMAGE_REAL){
		dest_type=(aflag&(AF_EFFECT|AF_WEAK));
		if(dest_type){
			if(dest_type==(AF_EFFECT|AF_WEAK)){
				aflag&=~(AF_EFFECT|AF_WEAK);
			}
		}else {
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
	}
	if(!(aflag&AF_NODEF)&&damage_type!=DAMAGE_REAL){
		x=dest->def;
		if(src)
			x+=dest->level-src->level;
		value*=def_coef(x);
	}
	switch(damage_type){
		case DAMAGE_REAL:
			break;
		case DAMAGE_PHYSICAL:
			derate=dest->physical_derate;
			if(src)derate-=src->physical_bonus;
			goto do_derate;
		case DAMAGE_MAGICAL:
			derate=dest->magical_derate;
			if(src)derate-=src->magical_bonus;
			goto do_derate;
do_derate:
		if(derate>0.8)
			value/=5*derate+1;
		else
			value*=1-derate;
		break;
	}
	if(!(aflag&AF_NOFLOAT)&&damage_type!=DAMAGE_REAL)
		value+=(0.1*rand01()-0.05)*value;
	if(!value)
		value=1;
	value=damage(dest,src,value,damage_type,aflag,type);
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->attack_end)
			e->base->attack_end(e,dest,src,value,damage_type,aflag,type);
	}
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->attack_end0)
			e->base->attack_end0(e,dest,src,value_backup,damage_type,aflag_backup,type);
	}
	return value;
}

int hittest(struct unit *dest,struct unit *src,double hit_rate){
	double real_rate;
	int effect_miss=0;
	if(!isalive(dest->state))
			return 0;
	for_each_effect(e,dest->owner->field->effects){
		if(!e->base->hittest)
			continue;
		switch(e->base->hittest(e,dest,src,&hit_rate)){
			case 1:
				goto hit;
			case 0:
				effect_miss=1;
			default:
				continue;
		}
	}
	if(effect_miss)
		goto miss;
	if(!dest->avoid)
		goto hit;
	real_rate=hit_rate*(double)src->hit/(double)dest->avoid;
	if(rand01()>=real_rate)
		goto miss;
hit:
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->hittest_end)
			e->base->hittest_end(e,dest,src,1);
	}
	return 1;
miss:
	report(dest->owner->field,MSG_MISS,dest,src);
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->hittest_end)
			e->base->hittest_end(e,dest,src,0);
	}
	return 0;
}
int test(double prob){
	return rand01()<prob;
}
double rand01(void){
	return drand48();
}
long randi(void){
	return lrand48();
}
static void do_normal_attack(struct unit *src){
	struct unit *dest=gettarget(src);
	if(!hittest(dest,src,1.0))
		return;
	attack(dest,src,src->atk,DAMAGE_PHYSICAL,AF_NORMAL,src->type0);
}
void normal_attack(struct unit *src){
	struct move am;
	am.id="normal_attack";
	am.action=do_normal_attack;
	am.init=NULL;
	am.getprior=NULL;
	am.type=src->type0;
	am.mlevel=MLEVEL_REGULAR;
	am.prior=0;
	am.cooldown=0;
	am.flag=MOVE_NORMALATTACK;
	am.unused=0;
	unit_move(src,&am);
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
	report(dest->owner->field,MSG_HEAL,dest,value);
	for_each_effect(e,dest->owner->field->effects){
		if(e->base->heal_end)
			e->base->heal_end(e,dest,value);
	}
	return value;
}
void instant_death(struct unit *dest){
	damage(dest,NULL,dest->base->max_hp*64,DAMAGE_REAL,AF_IDEATH,TYPE_VOID);
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
	report(dest->owner->field,MSG_HPMOD,dest,(long)hp-(long)ohp);
	if(!hp)
		unit_kill(dest);
	return hp;
}

unsigned long addhp(struct unit *dest,long hp){
	long rhp;
	if(!hp)
		return 0;
	if(!isalive(dest->state))
		return 0;
	rhp=hp+(long)dest->hp;
	if(hp<0&&rhp<0)
		rhp=0;
	if((unsigned long)rhp>dest->base->max_hp)
		rhp=dest->base->max_hp;
	dest->hp=rhp;
	report(dest->owner->field,MSG_HPMOD,dest,hp);
	if(!rhp)
		unit_kill(dest);
	return rhp;
}
long setspi(struct unit *dest,long spi){
	long ospi;
	if(!isalive(dest->state))
			return 0;
	ospi=dest->spi;
	if(spi>dest->base->max_spi)
		spi=dest->base->max_spi;
	if(spi<-dest->base->max_spi)
		spi=-dest->base->max_spi;
	if(spi==ospi)
		return spi;
	dest->spi=spi;
	report(dest->owner->field,MSG_SPIMOD,dest,spi-ospi);
	addhp(dest,-(SHEAR_COEF*dest->base->max_hp+1)*labs(spi-ospi));
	return spi;
}
struct unit *gettarget(struct unit *u){
	struct unit *r;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->gettarget&&(r=e->base->gettarget(e,u)))
			return r;
	}
	return u->osite;
}
int setcooldown(struct unit *u,struct move *m,int round){
	for_each_effect(e,u->owner->field->effects){
		if(e->base->setcooldown)
			e->base->setcooldown(u,m,&round);
	}
	m->cooldown=round;
	report(u->owner->field,MSG_UPDATE,m);
	for_each_effect(e,u->owner->field->effects){
		if(e->base->setcooldown_end)
			e->base->setcooldown_end(u,m,round);
	}
	return round;
}

static void effect_free(struct effect *ep,struct battle_field *f){
	if(ep->intrash){
		return;
	}
	ep->intrash=1;
	ep->prev=NULL;
	ep->next=f->trash;
	if(f->trash){
		f->trash->prev=ep;
	}
	f->trash=ep;
}
static void effect_insert(struct effect *ep,struct battle_field *f){
	if(!f->effects){
		f->effects=ep;
		return;
	}
	if(f->effects->base->prior<ep->base->prior){
		ep->next=f->effects;
		f->effects->prev=ep;
		f->effects=ep;
		return;
	}
	for_each_effect(e,f->effects){
		if(e->next&&e->next->base->prior>=ep->base->prior)
			continue;
		ep->next=e->next;
		ep->prev=e;
		if(e->next){
			e->next->prev=ep;
		}
		e->next=ep;
		return;
	}
}
struct effect *effect(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round){
	return effectx(base,dest,src,level,round,0);
}
struct effect *effectx(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round,int xflag){
	struct effect *ep=NULL;
	struct battle_field *f=NULL;
	int new;
	if(dest)
		f=dest->owner->field;
	else if(src)
		f=src->owner->field;
	if(!f||(dest&&!isalive(dest->state)))
		return NULL;
	xflag^=base->flag;
	if(!(xflag&EFFECT_NONHOOKABLE))for_each_effect(e,f->effects){
		if(e->base->effect&&e->base->effect(e,base,dest,src,&level,&round))
			return NULL;
	}
	for_each_effect(e,f->effects){
		if(e->dest==dest&&e->base==base&&!(xflag&EFFECT_ISOLATED)){
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
	ep->src1=src;
	if(!(xflag&EFFECT_NOCONSTRUCT)&&base->init){
		if(base->init(ep,level,round)){
			if(new)
				free(ep);
			else
				effect_final(ep);
			return NULL;
		}
	}else {
		if(xflag&EFFECT_ADDLEVEL)
			ep->level+=level;
		else
			ep->level=level;
		if(xflag&EFFECT_ADDROUND)
			ep->round+=round;
		else
			ep->round=round;
	}
	if(new){
		effect_insert(ep,f);
	}
	report(f,MSG_EFFECT,ep,level,round);
	if(base->inited)
		base->inited(ep);
	for_each_effect(e,f->effects){
		if(e->base->effect_end)
			e->base->effect_end(e,ep,dest,src,level,round);
	}
	return ep;
}
int effect_reinit(struct effect *ep,struct unit *src,long level,int round){
	return effect_reinitx(ep,src,level,round,0);
}
int effect_reinitx(struct effect *ep,struct unit *src,long level,int round,int xflag){
	struct battle_field *f=NULL;
	if(ep->dest)
		f=ep->dest->owner->field;
	else if(ep->src)
		f=ep->src->owner->field;
	if(!f)
		return -1;
	xflag^=ep->base->flag;
	if(!(xflag&EFFECT_NONHOOKABLE))for_each_effect(e,f->effects){
		if(e->base->effect&&e->base->effect(e,ep->base,ep->dest,src,&level,&round))
			return -1;
	}
	ep->src1=src;
	if(!(xflag&EFFECT_NOCONSTRUCT)&&ep->base->init){
		if(ep->base->init(ep,level,round)){
			effect_final(ep);
			return -1;
		}
	}else {
		if(xflag&EFFECT_ADDLEVEL)
			ep->level+=level;
		else
			ep->level=level;
		if(xflag&EFFECT_ADDROUND)
			ep->round+=round;
		else
			ep->round=round;
	}
	report(f,MSG_EFFECT,ep,level,round);
	if(ep->base->inited)
		ep->base->inited(ep);
	for_each_effect(e,f->effects){
		if(e->base->effect_end)
			e->base->effect_end(e,ep,ep->dest,src,level,round);
	}
	return 0;
}
int effect_setlevel(struct effect *e,long level){
	struct battle_field *f=NULL;
	if(e->dest)
		f=e->dest->owner->field;
	else if(e->src)
		f=e->src->owner->field;
	if(!f)
		return -1;
	if(level==e->level)
		return 0;
	e->level=level;
	report(f,MSG_EFFECT,e,level,e->round);
	return 0;
}
int effect_addlevel(struct effect *e,long level){
	struct battle_field *f=NULL;
	if(e->dest)
		f=e->dest->owner->field;
	else if(e->src)
		f=e->src->owner->field;
	if(!f)
		return -1;
	if(!level)
		return 0;
	e->level+=level;
	report(f,MSG_EFFECT,e,level,e->round);
	return 0;
}
int effect_setround(struct effect *e,int round){
	struct battle_field *f=NULL;
	if(e->dest)
		f=e->dest->owner->field;
	else if(e->src)
		f=e->src->owner->field;
	if(!f)
		return -1;
	if(round==e->round)
		return 0;
	e->round=round;
	report(f,MSG_EFFECT,e,e->level,round);
	return 0;
}
int effect_end(struct effect *e){
	struct battle_field *f;
	if(e->intrash)
		return -1;
	f=NULL;
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
	if(!e->prev&&!e->next)
		f->effects=NULL;
	update_attr_all(f);
	report(f,MSG_EFFECT_END,e);
	if(e->base->end)
		e->base->end(e);
	//printf("FREE1 %p\n",e);
	effect_free(e,f);
	return 0;
}

int effect_final(struct effect *e){
	struct battle_field *f;
	if(e->intrash)
		return -1;
	f=NULL;
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
	if(!e->prev&&!e->next)
		f->effects=NULL;
	update_attr_all(f);
	report(f,MSG_EFFECT_END,e);
	//printf("FREE3 %p\n",e);
	effect_free(e,f);
	return 0;
}
int purify(struct effect *e){
	if(e->base->flag&EFFECT_UNPURIFIABLE)
		return -1;
	return effect_end(e);
}
void wipetrash(struct battle_field *f){
	struct effect *e,*p;
	if(!(e=f->trash))
		return;
	for(e=f->trash;;e=p){
		p=e->next;
		free(e);
		if(!p)
			break;
	}
	f->trash=NULL;
}
int unit_wipeeffect(struct unit *u,int mask){
	int r=0;
	for_each_effect(e,u->owner->field->effects){
		if(e->dest!=u||(e->base->flag&mask))
			continue;
		//printf("WIPE %p\n",e);
		effect_final(e);
		++r;
	}
	return r;
}
void effect_event(struct effect *e){
	struct battle_field *f=NULL;
	if(e->dest)
		f=e->dest->owner->field;
	else if(e->src)
		f=e->src->owner->field;
	if(f){
		++e->inevent;
		report(f,MSG_EFFECT_EVENT,e);
	}
	//printf("effect event %s\n",e->base->id);
}
void effect_event_end(struct effect *e){
	struct battle_field *f=NULL;
	if(e->dest)
		f=e->dest->owner->field;
	else if(e->src)
		f=e->src->owner->field;
	if(f){
		--e->inevent;
		report(f,MSG_EFFECT_EVENT_END,e);
	}
	//printf("effect event %s end\n",e->base->id);
}
int revive_nonhookable(struct unit *u,unsigned long hp){
	if(u->state!=UNIT_FAILED||!hp)
		return -1;
	u->state=UNIT_NORMAL;
	sethp(u,hp);
	update_state(u);
	update_attr(u);
	for_each_effect(e,u->owner->field->effects){
		if(e->base->revive_end)
			e->base->revive_end(e,u,hp);
	}
	return 0;
}
int revive(struct unit *u,unsigned long hp){
	if(u->state!=UNIT_FAILED||!hp)
		return -1;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->revive&&e->base->revive(e,u,&hp))
			return -1;
	}
	u->state=UNIT_NORMAL;
	sethp(u,hp);
	update_state(u);
	update_attr(u);
	for_each_effect(e,u->owner->field->effects){
		if(e->base->revive_end)
			e->base->revive_end(e,u,hp);
	}
	return 0;
}
int event(const struct event *ev,struct unit *src){
	/*printf("event %s",ev->id);
	if(src)
		printf(" caused by %s\n",src->base->id);
	else
		printf("\n");*/
	report(src->owner->field,MSG_EVENT,ev,src);
	if(ev->un.action)
		ev->un.action(ev,src);
	//printf("event %s end\n",ev->id);
	report(src->owner->field,MSG_EVENT_END,ev,src);
	return 0;
}
int event_field(const struct event *ev,struct battle_field *f){
	report(f,MSG_EVENT,ev);
	if(ev->un.action_field)
		ev->un.action_field(ev,f);
	report(f,MSG_EVENT_END,ev);
	return 0;
}
void unit_cooldown_decrease(struct unit *u,int round){
	int r,r1;
	for(r=0;r<8;++r){
		if(!u->moves[r].id)
			continue;
		r1=u->moves[r].cooldown;
		if(!r1||r1<0)
			continue;
		for_each_effect(e,u->owner->field->effects){
			if(e->base->cooldown_decrease)
				e->base->cooldown_decrease(e,u,u->moves+r,&round);
		}
		r1-=round;
		if(r1<0)
			r1=0;
		u->moves[r].cooldown=r1;
		report(u->owner->field,MSG_UPDATE,u->moves+r);
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
	u->level=u->base->level;
	u->type0=u->base->type0;
	u->type1=u->base->type1;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->update_attr)
			e->base->update_attr(e,u);
	}
	report(u->owner->field,MSG_UPDATE,u);
}
void update_attr_all(struct battle_field *f){
	for_each_unit(u,f->p){
		update_attr(u);
	}
	for_each_unit(u,f->e){
		update_attr(u);
	}
}
void update_state(struct unit *u){
	int r;
	if(!isalive(u->state))
		return;
	r=UNIT_NORMAL;
	u->blockade=0;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->update_state){
			e->base->update_state(e,u,&r);
		}
	}
	u->state=r;
	report(u->owner->field,MSG_UPDATE,u);
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
			report(u->owner->field,MSG_UPDATE,v);
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
			report((v->dest?v->dest:v->src)->owner->field,MSG_UPDATE,v);
		}
		if(!v->round)
			effect_end(v);
	}
}
struct effect *unit_findeffect(const struct unit *u,const struct effect_base *base){
	for_each_effect(e,u->owner->field->effects){
		if(e->base==base&&e->dest==u)
			return e;
	}
	return NULL;
}
struct effect *unit_findeffect3(const struct unit *u,const struct effect_base *base,int flag){
	for_each_effect(e,u->owner->field->effects){
		if((!base||(e->base==base&&e->dest==u))&&(e->base->flag&flag))
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
		return e->src&&e->dest&&e->src->owner==e->dest->owner->enemy;
}
int unit_hasnegative(const struct unit *u){
	int r=0;
	for_each_effect(e,u->owner->field->effects){
		if(e->dest==u&&effect_isnegative(e))
			++r;
	}
	if(!r)
		switch(u->state){
			case UNIT_CONTROLLED:
			case UNIT_SUPPRESSED:
				return -1;
			default:
				break;
		}
	return r;
}
static int iffr(const struct unit *u,const struct move *m){
	return m->id&&(m->mlevel&MLEVEL_FREEZING_ROARING)&&unit_hasnegative(u);
}
int unit_move(struct unit *u,struct move *m){
	struct move *backup;
	int fr=iffr(u,m);
	switch(u->state){
		case UNIT_FAILED:
		case UNIT_FREEZING_ROARINGED:
			return -1;
		case UNIT_SUPPRESSED:
			if(!fr)
				return -1;
		default:
			if(!m->action)
				return -1;
			break;
	}
	if(!fr)
		for_each_effect(e,u->owner->field->effects){
			if(e->base->move&&e->base->move(e,u,m))
				return -1;
		}
	backup=u->move_cur;
	u->move_cur=m;
	report(u->owner->field,MSG_MOVE,u,m);
	m->action(u);
	u->move_cur=backup;
	for_each_effect(e,u->owner->field->effects){
		if(e->base->move_end)
			e->base->move_end(e,u,m);
	}
	return 0;
}
void unit_move_init(struct unit *u,struct move *m){
	struct move *backup=u->move_cur;
	u->move_cur=m;
	m->init(u);
	u->move_cur=backup;
}
int switchunit(struct unit *to){
	struct unit *f=to->owner->front;
	int enforce;
	if(f==to||!isalive(to->state))
		return -1;
	enforce=!isalive(f->state);
	for_each_effect(e,to->owner->field->effects){
		if(e->base->switchunit&&e->base->switchunit(e,to)&&!enforce)
			return -1;
	}
	to->owner->action=ACT_ABORT;
	to->owner->front=to;
	report(to->owner->field,MSG_SWITCH,to,f);
	for_each_effect(e,to->owner->field->effects){
		if(e->base->switchunit_end)
			e->base->switchunit_end(e,to);
	}
	return 0;
}
int canaction2(const struct player *p,int act){
	struct move *m;
	int fr,blockade=!!((1<<act)&p->front->blockade);
	switch(act){
		case ACT_MOVE0 ... ACT_MOVE7:
			m=p->front->moves+act;
			if(!m->id||!m->action)
				return 0;
			fr=iffr(p->front,m);
			switch(p->front->state){
				case UNIT_CONTROLLED:
					if(!(m->flag&MOVE_NOCONTROL)&&!fr)
						return 0;
				case UNIT_SUPPRESSED:
					if(!fr)
						return 0;
				default:
					if((m->cooldown||blockade)&&!fr)
						return 0;
					return 1;
				case UNIT_FAILED:
				case UNIT_FREEZING_ROARINGED:
					return 0;
			}
		case ACT_NORMALATTACK:
			switch(p->front->state){
				case UNIT_CONTROLLED:
				case UNIT_SUPPRESSED:
				case UNIT_FAILED:
				case UNIT_FREEZING_ROARINGED:
					return 0;
				default:
					if(blockade)
						return 0;
					return 1;
			}
		case ACT_UNIT0 ... ACT_UNIT5:
			if(p->units+(act-ACT_UNIT0)==p->front||!p->units[act-ACT_UNIT0].base||!isalive(p->units[act-ACT_UNIT0].state))
				return 0;
			switch(p->front->state){
				case UNIT_CONTROLLED:
				case UNIT_SUPPRESSED:
					return 0;
				default:
					if(blockade)
						return 0;
				case UNIT_FAILED:
				case UNIT_FREEZING_ROARINGED:
					return 1;
			}
		case ACT_ABORT:
		case ACT_GIVEUP:
			return 1;
		default:
			return 0;
	}
}

void player_action(struct player *p){
	int r;
	if(p->acted)
		return;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			if(iffr(p->front,p->front->moves+p->action))
				goto fr;
		default:
			for_each_effect(e,p->field->effects){
				if(e->base->action&&e->base->action(e,p))
					return;
			}
			break;
	}

	if(p->action==ACT_ABORT||!canaction2(p,p->action)){
		for_each_effect(e,p->field->effects){
			if(e->base->action_fail)
				e->base->action_fail(e,p);
		}
		return;
	}
fr:
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
		case ACT_NORMALATTACK:
		case ACT_UNIT0 ... ACT_UNIT5:
			report(p->field,MSG_ACTION,p);
			break;
		default:
			break;
	}
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			unit_move(p->front,p->front->moves+p->action);
			break;
		case ACT_NORMALATTACK:
			normal_attack(p->front);
			break;
		case ACT_UNIT0 ... ACT_UNIT5:
			r=p->action-ACT_UNIT0;
			if(p->units[r].base){
				switchunit(p->units+r);
			}
			break;
		default:
			break;
	}
	p->acted=1;
	for_each_effect(e,p->field->effects){
		if(e->base->action_end)
			e->base->action_end(e,p);
	}
}
struct player *getprior(struct player *p,struct player *e){
	struct player *prior;
	int pp,pe,frp=0,fre=0;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			if(!p->front->moves[p->action].id)
				pp=0;
			else
				pp=p->front->moves[p->action].getprior?
					p->front->moves[p->action].getprior(p->front):
					p->front->moves[p->action].prior;
			if(iffr(p->front,p->front->moves+p->action))
				frp=1;
			break;
		default:
			pp=0;
			break;
	}
	switch(e->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			if(!e->front->moves[e->action].id)
				pe=0;
			else
				pe=e->front->moves[e->action].getprior?
					e->front->moves[e->action].getprior(e->front):
					e->front->moves[e->action].prior;
			if(iffr(e->front,e->front->moves+e->action))
				fre=1;
			break;
		default:
			pe=0;
			break;
	}
	if(frp!=fre){
		return frp?p:e;
	}
	if(pp!=pe)
		prior=pp>pe?p:e;
	else if(p->front->speed!=e->front->speed)
		prior=p->front->speed>e->front->speed?p:e;
	else
		prior=test(0.5)?p:e;
	for_each_effect(e,p->field->effects){
		if(!e->base->getprior)
			continue;
		switch(e->base->getprior(e,prior)){
			case 0:
				break;
			case 1:
				return prior;
			case 2:
				return prior->enemy;
			default:
				prior=prior->enemy;
				break;
		}
	}
	return prior;
}
static void message_add(struct battle_field *f,const struct message *msg){
	struct message *p;
	size_t len;
	if(f->rec_size>=f->rec_length){
		len=f->rec_length+1024;
		if(!f->rec){
			p=malloc(len*sizeof(struct message));
			if(!p)
				return;
			f->rec=p;
			f->rec_length=len;
		}else {
			p=realloc(f->rec,len*sizeof(struct message));
			if(p){
				f->rec=p;
				f->rec_length=len;
			}else {
				f->rec_size=0;
			}
		}
	}
	memcpy(f->rec+f->rec_size++,msg,sizeof(struct message));
}
void report(struct battle_field *f,int type,...){
	void (*rr)(const struct message *,const struct player *);
	struct message msg;
	va_list ap;
	memset(&msg,0,sizeof(struct message));
	msg.type=type;
	msg.round=*f->round;
	msg.field=f;
	va_start(ap,type);
	switch(type){
		case MSG_ACTION:
		case MSG_BATTLE_END:
			msg.un.p=va_arg(ap,const struct player *);
			break;
		case MSG_DAMAGE:
			msg.un.damage.dest=va_arg(ap,const struct unit *);
			msg.un.damage.src=va_arg(ap,const struct unit *);
			msg.un.damage.value=va_arg(ap,unsigned long);
			msg.un.damage.damage_type=va_arg(ap,int);
			msg.un.damage.aflag=va_arg(ap,int);
			msg.un.damage.type=va_arg(ap,int);
			break;
		case MSG_EFFECT:
			msg.un.e=va_arg(ap,const struct effect *);
			msg.un.e_init.level=va_arg(ap,long);
			msg.un.e_init.round=va_arg(ap,int);
			break;
		case MSG_EFFECT_END:
		case MSG_EFFECT_EVENT:
		case MSG_EFFECT_EVENT_END:
			msg.un.e=va_arg(ap,const struct effect *);
			break;
		case MSG_EVENT:
		case MSG_EVENT_END:
			msg.un.event.ev=va_arg(ap,const struct event *);
			msg.un.event.src=va_arg(ap,const struct unit *);
			break;
		case MSG_FAIL:
			msg.un.u=va_arg(ap,const struct unit *);
			break;
		case MSG_HEAL:
			msg.un.heal.dest=va_arg(ap,const struct unit *);
			msg.un.heal.value=va_arg(ap,unsigned long);
			break;
		case MSG_HPMOD:
			msg.un.hpmod.dest=va_arg(ap,const struct unit *);
			msg.un.hpmod.value=va_arg(ap,long);
			break;
		case MSG_MISS:
		case MSG_SWITCH:
			msg.un.u2.dest=va_arg(ap,const struct unit *);
			msg.un.u2.src=va_arg(ap,const struct unit *);
			break;
		case MSG_MOVE:
			msg.un.move.u=va_arg(ap,const struct unit *);
			msg.un.move.m=va_arg(ap,const struct move *);
			break;
		case MSG_ROUND:
		case MSG_ROUNDEND:
			break;
		case MSG_SPIMOD:
			msg.un.spimod.dest=va_arg(ap,const struct unit *);
			msg.un.spimod.value=va_arg(ap,long);
			break;
		case MSG_UPDATE:
			msg.un.uaddr=va_arg(ap,const void *);
		default:
			break;
	}
	va_end(ap);
	if(msg.type!=MSG_UPDATE)
		message_add(f,&msg);
	if((rr=f->p->reporter))
		rr(&msg,f->p);
	if((rr=f->e->reporter))
		rr(&msg,f->e);
}
void history_add(struct battle_field *f){
	struct history *h;
	if(f->ht_size>=f->ht_length){
		size_t len;
		len=f->ht_length+128;
		if(!f->ht){
			h=malloc(len*sizeof(struct history));
			if(!h)
				return;
			f->ht=h;
			f->ht_length=len;
		}else {
			h=realloc(f->ht,len*sizeof(struct history));
			if(h){
				f->ht=h;
				f->ht_length=len;
			}else {
				f->ht_size=0;
			}
		}
	}
	h=f->ht+f->ht_size++;
	memcpy(&h->p,f->p,sizeof(struct player));
	memcpy(&h->e,f->e,sizeof(struct player));
}
