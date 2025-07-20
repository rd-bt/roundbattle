/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include "battle-core.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define printf (use report() instead.)
static void clearhp(struct unit *u){
	long hp;
	hp=(long)u->hp;
	u->hp=0;
	report(u->owner->field,MSG_HPMOD,u,-hp);
}
static int iffr(const struct unit *u,const struct move *m){
	return m->id&&(m->mlevel&MLEVEL_FREEZING_ROARING)&&unit_hasnegative(u);
}
int unit_setstate(struct unit *u,int state){
	struct player *p;
	switch(state){
		case UNIT_NORMAL:
		case UNIT_CONTROLLED:
		case UNIT_SUPPRESSED:
		case UNIT_FADING:
			if(u->state==UNIT_FREEZING_ROARINGED)
				return -1;
			if(u->state==state)
				return 0;
			u->state=state;
			goto end;
		case UNIT_FAILED:
			switch(u->state){
				case UNIT_FAILED:
					return 0;
				case UNIT_FREEZING_ROARINGED:
					return -1;
				default:
					break;
			}
			if(u->hp)
				clearhp(u);
			//dealing the freezing_roaring
			p=u->owner;
			if(p->acted)
				goto fr_end;
			switch((state=u->owner->action)){
				case ACT_MOVE0 ... ACT_MOVE7:
					if(p->acted||*p->field->stage>=STAGE_ROUNDEND||!iffr(u,u->moves+state))
						break;
					if(u->state==UNIT_FADING)
						return 1;
					u->state=UNIT_FADING;
					goto end1;
				default:
					break;
			}
fr_end:
			//end
			u->state=UNIT_FAILED;
			if(u==u->owner->front)
				u->owner->action=ACT_ABORT;
			report(u->owner->field,MSG_FAIL,u);
			unit_wipeeffect(u,EFFECT_KEEP);
			return 0;
		case UNIT_FREEZING_ROARINGED:
			switch(u->state){
				case UNIT_FREEZING_ROARINGED:
					return 0;
				default:
					break;
			}
			if(u->hp)
				clearhp(u);
			u->state=UNIT_FREEZING_ROARINGED;
			if(u==u->owner->front)
				u->owner->action=ACT_ABORT;
			memset(u->moves,0,8*sizeof(struct move));
			report(u->owner->field,MSG_FAIL,u);
			unit_wipeeffect(u,0);
			return 0;
		default:
			return -1;
	}
end:
	report(u->owner->field,MSG_UPDATE,u);
	return 0;
end1:
	report(u->owner->field,MSG_UPDATE,u);
	return 1;
}
int unit_kill(struct unit *u){
	if(!isalive(u->state)||u->hp)
			return -1;
	for_each_effectf(e,u->owner->field->effects,kill){
		if(e->base->kill(e,u))
			return -1;
	}
	if(!isalive(u->state)||u->hp)
			return -1;
	unit_setstate(u,UNIT_FAILED);
	for_each_effectf(e,u->owner->field->effects,kill_end){
		e->base->kill_end(e,u);
	}
	return 0;
}
static int checkalive(struct unit *dest,struct unit *src){
	if(src&&src->state==UNIT_FREEZING_ROARINGED)
		return 0;
	if(!dest)
		return 1;
	return isalive(dest->state);
}
unsigned long damage(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	unsigned long ohp;
	switch(damage_type){
		case DAMAGE_REAL:
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return 0;
	}
	if(!checkalive(dest,src)||!value)
			return 0;
	if(!(aflag&AF_NONHOOKABLE_D)){
		for_each_effectf(e,dest->owner->field->effects,damage){
			if(e->base->damage(e,dest,src,&value,&damage_type,&aflag,&type))
				return 0;
		}
	}
	ohp=dest->hp;
	if(ohp>value){
		dest->hp-=value;
	}else {
		dest->hp=0;
	}
	report(dest->owner->field,MSG_DAMAGE,dest,src,value,damage_type,aflag,type);
	if(dest->hp!=ohp){
		for_each_effectf(e,dest->owner->field->effects,hpmod){
			e->base->hpmod(e,dest,(long)dest->hp-(long)ohp,HPMOD_DAMAGE);
		}
	}
	if(!dest->hp&&!(aflag&AF_KEEPALIVE))
		unit_kill(dest);
	for_each_effectf(e,dest->owner->field->effects,damage_end){
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
	if(!checkalive(dest,src))
			return 0;
	value_backup=value;
	aflag_backup=aflag;
	if(!(aflag&AF_NONHOOKABLE)){
		for_each_effectf(e,dest->owner->field->effects,attack){
			if(e->base->attack(e,dest,src,&value,&damage_type,&aflag,&type))
				return 0;
		}
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
	if(damage_type!=DAMAGE_REAL){
		dest_type=(aflag&(AF_EFFECT|AF_WEAK));
		if(dest_type){
			if(dest_type==(AF_EFFECT|AF_WEAK)){
				aflag&=~(AF_EFFECT|AF_WEAK);
			}
		}else {
			x=effect_weak_level(dest->type0|dest->type1,type);
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
	}else
		aflag&=~(AF_EFFECT|AF_WEAK);
	if(!(aflag&AF_NODEF)==(damage_type!=DAMAGE_REAL)){
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
	if(!(aflag&AF_NOINHIBIT)&&dest->spi)
		value*=inhibit_coef(dest->spi);
	if(!(aflag&AF_NOFLOAT)&&damage_type!=DAMAGE_REAL)
		value+=(0.1*rand01()-0.05)*value;
	if(!value)
		value=1;
	if((aflag&AF_IDEATH)&&value<dest->hp&&value_backup>=dest->hp)
		value=dest->hp;
	value=damage(dest,src,value,damage_type,aflag,type);
	for_each_effectf(e,dest->owner->field->effects,attack_end){
		e->base->attack_end(e,dest,src,value,damage_type,aflag,type);
	}
	for_each_effectf(e,dest->owner->field->effects,attack_end0){
		e->base->attack_end0(e,dest,src,value_backup,damage_type,aflag_backup,type);
	}
	return value;
}

int hittest(struct unit *dest,struct unit *src,double hit_rate){
	double real_rate;
	int effect_miss=0;
	if(!checkalive(dest,src))
			return 0;
	for_each_effectf(e,dest->owner->field->effects,hittest){
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
	for_each_effectf(e,dest->owner->field->effects,hittest_end){
		e->base->hittest_end(e,dest,src,1);
	}
	return 1;
miss:
	report(dest->owner->field,MSG_MISS,dest,src);
	for_each_effectf(e,dest->owner->field->effects,hittest_end){
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
long heal3(struct unit *dest,long value,int aflag){
	unsigned long ohp;
	long hp;
	if(!isalive(dest->state))
			return 0;
	if(!(aflag&AF_NONHOOKABLE)){
		for_each_effectf(e,dest->owner->field->effects,heal){
			if(e->base->heal(e,dest,&value))
				return 0;
		}
	}
	if(!(aflag&AF_NOINHIBIT)&&dest->spi)
		value*=inhibit_coef(dest->spi);
	ohp=dest->hp;
	if(ohp>=dest->max_hp&&value>=0)
		hp=(long)ohp;
	else {
		hp=(long)ohp+value;
		if(hp>(long)dest->max_hp)
			hp=(long)dest->max_hp;
		if(hp<0)
			hp=0;
	}
	dest->hp=hp;
	report(dest->owner->field,MSG_HEAL,dest,value);
	if(hp!=ohp){
		for_each_effectf(e,dest->owner->field->effects,hpmod){
			e->base->hpmod(e,dest,(long)hp-(long)ohp,HPMOD_HEAL);
		}
	}
	if(!dest->hp&&!(aflag&AF_KEEPALIVE))
		unit_kill(dest);
	for_each_effectf(e,dest->owner->field->effects,heal_end){
		e->base->heal_end(e,dest,value);
	}
	return value;
}
long heal(struct unit *dest,long value){
	return heal3(dest,value,0);
}
unsigned long instant_death(struct unit *dest){
	return damage(dest,NULL,dest->max_hp*64,DAMAGE_REAL,AF_IDEATH,TYPE_VOID);
}
int addhp3(struct unit *dest,long hp,int flag){
	unsigned long ohp;
	long rhp;
	if(!isalive(dest->state))
		return -1;
	if(!hp)
		return 0;
	ohp=dest->hp;
	if(ohp>=dest->max_hp&&hp>=0)
		rhp=(long)ohp;
	else {
		rhp=(long)ohp+hp;
		if(rhp>(long)dest->max_hp)
			rhp=(long)dest->max_hp;
		if(rhp<0)
			rhp=0;
	}
	dest->hp=(unsigned long)rhp;
	report(dest->owner->field,MSG_HPMOD,dest,hp);
	if(rhp!=ohp){
		for_each_effectf(e,dest->owner->field->effects,hpmod){
			e->base->hpmod(e,dest,rhp-(long)ohp,flag);
		}
	}
	if(!dest->hp)
		unit_kill(dest);
	return 0;
}
int sethp(struct unit *dest,unsigned long hp){
	return addhp3(dest,hp-(long)dest->hp,HPMOD_SETHP);
}
int addhp(struct unit *dest,long hp){
	return addhp3(dest,hp,HPMOD_SETHP);
}
int setspi(struct unit *dest,long spi){
	long ospi,ds;
	if(!isalive(dest->state))
		return -1;
	ospi=dest->spi;
	if(spi>dest->max_spi)
		spi=dest->max_spi;
	if(spi<-dest->max_spi)
		spi=-dest->max_spi;
	if(spi==ospi)
		return 0;
	dest->spi=spi;
	ds=spi-ospi;
	report(dest->owner->field,MSG_SPIMOD,dest,ds);
	addhp3(dest,-(SHEAR_COEF*dest->max_hp*labs(ds)+1),HPMOD_SHEAR);
	for_each_effectf(e,dest->owner->field->effects,spimod){
		e->base->spimod(e,dest,ds);
	}
	return 0;
}
struct unit *gettarget(struct unit *u){
	struct unit *r;
	for_each_effectf(e,u->owner->field->effects,gettarget){
		if((r=e->base->gettarget(e,u)))
			return r;
	}
	return u->osite;
}
int setcooldown(struct unit *u,struct move *m,int round){
	for_each_effectf(e,u->owner->field->effects,setcooldown){
		e->base->setcooldown(u,m,&round);
	}
	m->cooldown=round;
	report(u->owner->field,MSG_UPDATE,m);
	for_each_effectf(e,u->owner->field->effects,setcooldown_end){
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
static int frindex(const struct unit *u){
	const struct move *m;
	for(int i=0;i<8;++i){
		m=u->moves+i;
		if(!m->id)
			continue;
		if(m->mlevel&MLEVEL_FREEZING_ROARING)
			return i;
	}
	return -1;
}
struct effect *effect(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round){
	return effectx(base,dest,src,level,round,0);
}
static int checkalive3(struct unit *dest,struct unit *src,int xflag){
	if(src&&src->state==UNIT_FREEZING_ROARINGED)
		return 0;
	if(!dest)
		return 1;
	switch(dest->state){
		case UNIT_FADING:
		case UNIT_FAILED:
			return !!(xflag&EFFECT_ALLOWFAILED);
		case UNIT_FREEZING_ROARINGED:
			return 0;
		default:
			return 1;
	}
}
struct effect *effectx(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round,int xflag){
	struct effect *ep=NULL;
	struct battle_field *f=NULL;
	int new;
	if(dest)
		f=dest->owner->field;
	else if(src)
		f=src->owner->field;
	xflag^=base->flag;
	if(!checkalive3(dest,src,xflag))
		return NULL;
	if(!(xflag&EFFECT_NONHOOKABLE)){
		for_each_effectf(e,f->effects,effect){
			if(e->base->effect(e,base,dest,src,&level,&round))
				return NULL;
		}
	}
	if(dest&&frindex(dest)>=0){
		if((!src||src->owner==dest->owner)&&effect_isnegative_base(base,dest,src,level))
			return NULL;
	}
	if(!(xflag&EFFECT_ISOLATED)){
		for_each_effect(e,f->effects){
			if(e->dest==dest&&e->base==base){
				ep=e;
				break;
			}
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
		switch(base->init(ep,level,round)){
			case 0:
				break;
			case 1:
				if(new)
					free(ep);
				else
					effect_final(ep);
				return NULL;
			default:
				if(new)
					free(ep);
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
	for_each_effectf(e,f->effects,effect_end){
		e->base->effect_end(e,ep,dest,src,level,round);
	}
	return ep;
}
int effect_reinit(struct effect *ep,struct unit *src,long level,int round){
	return effect_reinitx(ep,src,level,round,0);
}
int effect_reinitx(struct effect *ep,struct unit *src,long level,int round,int xflag){
	struct battle_field *f=effect_field(ep);
	xflag^=ep->base->flag;
	if(!(xflag&EFFECT_NONHOOKABLE)){
		for_each_effectf(e,f->effects,effect){
			if(e->base->effect(e,ep->base,ep->dest,src,&level,&round))
				return -1;
		}
	}
	if(ep->dest&&frindex(ep->dest)>=0){
		if((!src||src->owner==ep->dest->owner)&&effect_isnegative_base(ep->base,ep->dest,src,level))
			return -1;
	}
	ep->src1=src;
	if(!(xflag&EFFECT_NOCONSTRUCT)&&ep->base->init){
		switch(ep->base->init(ep,level,round)){
			case 0:
				break;
			case 1:
				effect_final(ep);
			default:
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
	for_each_effectf(e,f->effects,effect_end){
		e->base->effect_end(e,ep,ep->dest,src,level,round);
	}
	return 0;
}
int effect_setlevel(struct effect *e,long level){
	struct battle_field *f=effect_field(e);
	if(level==e->level)
		return 0;
	e->level=level;
	report(f,MSG_EFFECT,e,level,e->round);
	return 0;
}
int effect_addlevel(struct effect *e,long level){
	struct battle_field *f=effect_field(e);
	if(!level)
		return 0;
	e->level+=level;
	report(f,MSG_EFFECT,e,level,e->round);
	return 0;
}
int effect_setround(struct effect *e,int round){
	struct battle_field *f=effect_field(e);
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
	f=effect_field(e);
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
	f=effect_field(e);
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
int purify(struct effect *ep){
	struct battle_field *f;
	int r;
	if(ep->base->flag&EFFECT_UNPURIFIABLE)
		return -1;
	f=effect_field(ep);
	for_each_effectf(v,f->effects,purify){
		if(v->base->purify(v,ep))
			return -1;
	}
	r=effect_end(ep);
	for_each_effectf(v,f->effects,purify_end){
		v->base->purify_end(v,ep);
	}
	return r;
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
void wipetrash(struct battle_field *f){
	struct effect *e,*p;
	if(!(e=f->trash))
		return;
	for(;;e=p){
		p=e->next;
		free(e);
		if(!p)
			break;
	}
	f->trash=NULL;
}
void effect_event(struct effect *e){
	struct battle_field *f=effect_field(e);
	++e->inevent;
	report(f,MSG_EFFECT_EVENT,e);
	//printf("effect event %s\n",e->base->id);
}
void effect_event_end(struct effect *e){
	struct battle_field *f=effect_field(e);
	--e->inevent;
	report(f,MSG_EFFECT_EVENT_END,e);
	//printf("effect event %s end\n",e->base->id);
}
int revive_nonhookable(struct unit *u,unsigned long hp){
	if(isalive(u->state))
		return -1;
	if(unit_setstate(u,UNIT_NORMAL))
		return -1;
	sethp(u,hp);
	update_state(u);
	update_attr(u);
	for_each_effectf(e,u->owner->field->effects,revive_end){
		e->base->revive_end(e,u,hp);
	}
	return 0;
}
int revive(struct unit *u,unsigned long hp){
	if(isalive(u->state))
		return -1;
	for_each_effectf(e,u->owner->field->effects,revive){
		if(e->base->revive(e,u,&hp))
			return -1;
	}
	if(unit_setstate(u,UNIT_NORMAL))
		return -1;
	sethp(u,hp);
	update_state(u);
	update_attr(u);
	for_each_effectf(e,u->owner->field->effects,revive_end){
		e->base->revive_end(e,u,hp);
	}
	return 0;
}
int event(const struct event *ev,struct unit *src){
	if(src->state==UNIT_FREEZING_ROARINGED)
		return -1;
//	report(src->owner->field,MSG_EVENT,ev,src);
	event_start(ev,src);
	if(ev->action)
		ev->action(ev,src);
//	report(src->owner->field,MSG_EVENT_END,ev,src);
	event_end(ev,src);
	return 0;
}
void unit_cooldown_decrease(struct unit *u,int round){
	int r,r1,r2;
	for(r=0;r<8;++r){
		if(!u->moves[r].id)
			continue;
		r1=u->moves[r].cooldown;
		if(!r1||r1<0)
			continue;
		r2=round;
		for_each_effectf(e,u->owner->field->effects,cooldown_decrease){
			e->base->cooldown_decrease(e,u,u->moves+r,&r2);
		}
		r1-=r2;
		if(r1<0)
			r1=0;
		u->moves[r].cooldown=r1;
		report(u->owner->field,MSG_UPDATE,u->moves+r);
	}
}
void unit_fillattr(struct unit *u){
	update_attr_init(u);
	u->spi=0;
	u->hp=u->max_hp;
	memcpy(u->moves,u->base->moves,8*sizeof(struct move));
	u->move_cur=NULL;
	u->state=UNIT_NORMAL;
}
void update_attr_init(struct unit *u){
	u->max_hp=u->base->max_hp;
	u->max_spi=u->base->max_spi;
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
}
void update_attr(struct unit *u){
	if(u->state==UNIT_FREEZING_ROARINGED)
		return;
	update_attr_init(u);
	for_each_effectf(e,u->owner->field->effects,update_attr){
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
	struct move *m;
	int r;
	if(!isalive(u->state))
		return;
	r=UNIT_NORMAL;
	u->blockade=0;
	for(int i=0;i<8;++i){
		m=u->moves+i;
		if(!m->id||!m->available)
			continue;
		if(!m->available(u,m))
			u->blockade|=1<<i;
	}
	for_each_effectf(e,u->owner->field->effects,update_state){
		e->base->update_state(e,u,&r);
	}
	unit_setstate(u,r);
}
void effect_in_roundstart(struct effect *effects){
	for_each_effectf(e,effects,roundstart){
		e->base->roundstart(e);
	}
}
void effect_in_roundend(struct effect *effects){
	for_each_effectf(e,effects,roundend){
		e->base->roundend(e);
	}
}
void unit_effect_in_roundend(struct unit *u){
	for_each_effectf(e,u->owner->field->effects,roundend){
		if(e->dest==u)
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
		if(!v->round){
			if(!v->base->end_in_roundend||!v->base->end_in_roundend(v))
				effect_end(v);
		}
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
		if(!v->round){
			if(!v->base->end_in_roundend||!v->base->end_in_roundend(v))
				effect_end(v);
		}
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
int effect_isnegative_base(const struct effect_base *base,const struct unit *dest,const struct unit *src,long level){
	if(base->flag&(EFFECT_NEGATIVE|EFFECT_ABNORMAL|EFFECT_CONTROL))
		return 1;
	else if(base->flag&EFFECT_POSITIVE)
		return 0;
	else if(base->flag&EFFECT_ATTR)
		return level<0;
	else
		return src&&dest&&src->owner==dest->owner->enemy;
}
int effect_isnegative(const struct effect *e){
	return effect_isnegative_base(e->base,e->dest,e->src,e->level);
}
int unit_hasnegative(const struct unit *u){
	int r=0;
	const struct move *m;
	for_each_effect(e,u->owner->field->effects){
		if(e->dest==u&&effect_isnegative(e))
			++r;
	}
	if(r)
		return r;
	switch(u->state){
		case UNIT_NORMAL:
		case UNIT_FADING:
			r=u->blockade&255;
			if(!r)
				return 0;
			for(int i=0;i<8;++i){
				if(!((1<<i)&r))
					continue;
				m=u->moves+i;
				if(!m->id)
					continue;
				if(m->mlevel&MLEVEL_FREEZING_ROARING)
					return -1;
			}
			return 0;
		case UNIT_CONTROLLED:
		case UNIT_SUPPRESSED:
			return -1;
		default:
			return 0;
	}
}
int unit_move_nonhookable(struct unit *u,struct move *m){
	struct move *backup;
	switch(u->state){
		case UNIT_FAILED:
		case UNIT_FREEZING_ROARINGED:
			return -1;
		default:
			if(!m->action)
				return -1;
			break;
	}
	report(u->owner->field,MSG_MOVE,u,m);
	backup=u->move_cur;
	u->move_cur=m;
	m->action(u);
	u->move_cur=backup;
	report(u->owner->field,MSG_MOVE_END,u,m);
	for_each_effectf(e,u->owner->field->effects,move_end){
		e->base->move_end(e,u,m);
	}
	return 0;
}
int unit_move(struct unit *u,struct move *m){
	ptrdiff_t md;
	switch(u->state){
		case UNIT_FAILED:
		case UNIT_FREEZING_ROARINGED:
			return -1;
		case UNIT_SUPPRESSED:
			switch((md=m-u->moves)){
				case ACT_MOVE0 ... ACT_MOVE7:
					if(u->owner->action==md&&(m->mlevel&MLEVEL_CONCEPTUAL))
						break;
				default:
					return -1;
			}
		default:
			if(!m->action)
				return -1;
			break;
	}
	for_each_effectf(e,u->owner->field->effects,move){
		if(e->base->move(e,u,m))
			return -1;
	}
	return unit_move_nonhookable(u,m);
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
	for_each_effectf(e,to->owner->field->effects,switchunit){
		if(e->base->switchunit(e,to)&&!enforce)
			return -1;
	}
	to->owner->action=ACT_ABORT;
	to->owner->front=to;
	report(to->owner->field,MSG_SWITCH,to,f);
	for_each_effectf(e,to->owner->field->effects,switchunit_end){
		e->base->switchunit_end(e,to);
	}
	return 0;
}
int canaction2(const struct player *p,int act){
	struct move *m;
	int cpfr,blockade=!!((1<<act)&p->front->blockade),stage;
	switch(act){
		case ACT_MOVE0 ... ACT_MOVE7:
			m=p->front->moves+act;
			if(!m->id||!m->action)
				return 0;
			stage=*p->field->stage;
			cpfr=(stage!=STAGE_SELECT&&(m->mlevel&MLEVEL_CONCEPTUAL))||iffr(p->front,m);
			switch(p->front->state){
				case UNIT_CONTROLLED:
					if(!(m->flag&MOVE_NOCONTROL)&&!cpfr)
						return 0;
				case UNIT_SUPPRESSED:
					if(!cpfr)
						return 0;
				default:
					if(((stage==STAGE_SELECT&&m->cooldown)||blockade)&&!cpfr)
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
	int r,nonh=0;
	if(p->acted)
		return;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			if(iffr(p->front,p->front->moves+p->action)){
				nonh=1;
				goto fr;
			}
		default:
			for_each_effectf(e,p->field->effects,action){
				if(e->base->action(e,p))
					goto fail;
			}
			break;
	}

	if(p->action==ACT_ABORT||!canaction2(p,p->action)){
fail:
		for_each_effectf(e,p->field->effects,action_fail){
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
			(nonh?unit_move_nonhookable:unit_move)(p->front,p->front->moves+p->action);
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
	for_each_effectf(e,p->field->effects,action_end){
		e->base->action_end(e,p);
	}
}
int unit_reap_fading(struct unit *u){
	if(u->state!=UNIT_FADING)
		return -1;
	return unit_setstate(u,UNIT_FAILED);
}
void reap_fading(struct battle_field *f){
	for_each_unit(u,f->p){
		unit_reap_fading(u);
	}
	for_each_unit(u,f->e){
		unit_reap_fading(u);
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
	for_each_effectf(e,p->field->effects,getprior){
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
	size_t len,index;
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
	index=f->rec_size++;
	p=f->rec+index;
	memcpy(p,msg,sizeof(struct message));
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
			msg.un.heal.value=va_arg(ap,long);
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
		case MSG_MOVE_END:
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
const struct message *message_findsource(const struct message *msg){
	const struct message *p=msg->field->rec,*p1;
	size_t sz=msg->field->rec_size;
	int r=msg->round;
	p1=msg->type==MSG_UPDATE?p+sz-1:p+sz-2;
	for(;p1>=p;--p1){
continue0:
		if(p1->round!=r)
			return NULL;
		switch(p1->type){
			case MSG_EFFECT_EVENT:
			case MSG_EVENT:
			case MSG_MOVE:
				return p1;
			case MSG_EFFECT_EVENT_END:
			case MSG_EVENT_END:
			case MSG_MOVE_END:
				--p1;
				for(size_t d=1;d&&p1>=p;--p1){
					switch(p1->type){
						case MSG_EFFECT_EVENT:
						case MSG_EVENT:
						case MSG_MOVE:
							--d;
							continue;
						case MSG_EFFECT_EVENT_END:
						case MSG_EVENT_END:
						case MSG_MOVE_END:
							++d;
						default:
							continue;
					}
					
				}
				goto continue0;
			default:
				continue;
		}
	}
	return NULL;
}
const char *message_id(const struct message *msg){
	switch(msg->type){
		case MSG_EFFECT_EVENT:
			return msg->un.e->base->id;
		case MSG_EVENT:
			return msg->un.event.ev->id;
		case MSG_MOVE:
			return msg->un.move.m->id;
		default:
			return NULL;
	}
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
void field_free(struct battle_field *field){
	wipetrash(field);
	if(field->rec)
		free(field->rec);
	if(field->ht)
		free(field->ht);
}
int player_hasunit(struct player *p){
	for(int i=0;i<6;++i){
		if(!p->units[i].base)
			break;
		if(isalive_s(p->units[i].state)){
			return 1;
		}
	}
	return 0;
}
const struct player *getwinner(struct battle_field *f){
	struct player *p=f->p,*e=f->e;
	int r0,r1,r2,r3;
	r0=isalive_s(r2=p->front->state);
	r1=isalive_s(r3=e->front->state);
	if(!r0)
		r0=player_hasunit(p);
	if(!r1)
		r1=player_hasunit(e);
	if(r0&&r1)
		return NULL;
	if(r0!=r1)
		return r0?p:e;
	r0=(r2==UNIT_FREEZING_ROARINGED);
	r1=(r3==UNIT_FREEZING_ROARINGED);
	if(r0!=r1)
		return r1?p:e;
#define cmpfld(fld) \
	if(p->front->fld!=e->front->fld)\
		return p->front->fld>e->front->fld?p:e
	cmpfld(level);
	cmpfld(speed);
	return test(0.5)?p:e;
}

