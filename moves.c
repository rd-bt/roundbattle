#include "battle-core.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#define printf (use report() instead.)
unsigned long gcdul(unsigned long x,unsigned long y){
	unsigned long r;
	int r1;
	r=__builtin_ctzl(x);
	r1=__builtin_ctzl(y);
	r=r<r1?r:r1;
	x>>=r;
	y>>=r;
	r1=(x<y);
	while(x&&y){
		if(r1^=1)
			x%=y;
		else
			y%=x;
	}
	if(x&&y){
		__builtin_unreachable();
	}else {
		return (x|y)<<r;
	}
}

int abnormal_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=round;
	if(level>0)
		e->level+=level;
	return 0;
}
#define abnormal_damage(name,frac,type,extra)\
void name##_roundend(struct effect *e){\
	effect_event(e);\
	attack(e->dest,NULL,e->dest->base->max_hp/frac,DAMAGE_REAL,0,type);\
	effect_event_end(e);\
}\
int name##_init(struct effect *e,long level,int round){\
	if(!e->round)\
		e->round=round;\
	return 0;\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=abnormal_init,\
	.roundend=name##_roundend,\
	extra\
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,\
	.prior=-64\
}};
#define abnormal_control(name,extra)\
void name##_update_state(struct effect *e,struct unit *u,int *state){\
	if(e->dest==u&&*state==UNIT_NORMAL)\
		*state=UNIT_CONTROLLED;\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=abnormal_init,\
	.update_state=name##_update_state,\
	extra\
	.flag=EFFECT_ABNORMAL|EFFECT_CONTROL|EFFECT_NEGATIVE,\
}};
#define COMMA ,
int poisoned_attack(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&*damage_type==DAMAGE_MAGICAL)
		*value*=0.85;
	return 0;
}
int burnt_attack(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&*damage_type==DAMAGE_PHYSICAL)
		*value*=0.85;
	return 0;
}
abnormal_damage(cursed,5,TYPE_STEEL,)
abnormal_damage(radiated,5,TYPE_LIGHT,)
abnormal_damage(poisoned,10,TYPE_POISON,.attack=poisoned_attack COMMA)
abnormal_damage(burnt,10,TYPE_FIRE,.attack=burnt_attack COMMA)

void parasitized_roundend(struct effect *e){
	effect_event(e);
	heal(e->dest->osite,
		attack(e->dest,NULL,e->dest->base->max_hp/10,DAMAGE_REAL,0,TYPE_GRASS)/2
	);
	effect_event_end(e);
}
const struct effect_base parasitized[1]={{
	.id="parasitized",
	.init=abnormal_init,
	.roundend=parasitized_roundend,
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,
	.prior=-64
}};
struct unit *confused_gettarget(struct effect *e,struct unit *u){
	if(u==e->dest&&test(0.5))
		return u;
	return NULL;
}

const struct effect_base confused[1]={{
	.id="confused",
	.init=abnormal_init,
	.gettarget=confused_gettarget,
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,
}};
int aurora_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&*damage_type!=DAMAGE_REAL){
		effect_event(e);
		effect_reinit(e,src,*value,e->round);
		effect_event_end(e);
		return -1;
	}
	return 0;
}
void aurora_end(struct effect *e){
	if(e->level){
		attack(e->dest,e->src,e->level,DAMAGE_REAL,0,TYPE_VOID);
	}
}
int asleep_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&*damage_type!=DAMAGE_REAL){
		*value*=1.5;
		effect_end(e);
	}
	return 0;
}
int frozen_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&*damage_type!=DAMAGE_REAL&&(*type&TYPE_FIRE)){
		*value*=2;
		effect_end(e);
	}
	return 0;
}
int petrified_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&*damage_type!=DAMAGE_REAL&&(*type&TYPE_FIGHTING)){
		*value*=2;
		effect_end(e);
	}
	return 0;
}
abnormal_control(frozen,.damage=frozen_damage COMMA)
abnormal_control(asleep,.damage=asleep_damage COMMA)
abnormal_control(paralysed,)
abnormal_control(stunned,)
abnormal_control(petrified,.damage=petrified_damage COMMA)
abnormal_control(aurora,.damage=aurora_damage COMMA .end=aurora_end COMMA)
void effect_update_attr(struct effect *e){
	update_attr(e->dest);
}
int attr_init(struct effect *e,long level,int round){
	if(round)
		level+=e->level;
	else
		round=-1;
	if(!e->round)
		e->round=round;
	if(level>ATTR_MAX)
		level=ATTR_MAX;
	if(level<ATTR_MIN)
		level=ATTR_MIN;
	e->level=level;
	if(!level)
		return -1;
	return 0;
}
#define effect_attr(name,attr)\
void name##_update_attr(struct effect *e,struct unit *u){\
	if(e->dest==u){\
		if(e->level<0)\
			u->attr*=pow(M_SQRT1_2,-e->level);\
		else\
			u->attr*=1.0+0.5*e->level;\
	}\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=attr_init,\
	.inited=effect_update_attr,\
	.update_attr=name##_update_attr,\
	.flag=EFFECT_ATTR|EFFECT_KEEP,\
	.prior=64\
}};
effect_attr(ATK,atk)
effect_attr(SPEED,speed)
effect_attr(HIT,hit)
effect_attr(AVOID,avoid)
#define effect_attr_d(name,attr,step)\
void name##_update_attr(struct effect *e,struct unit *u){\
	if(e->dest==u)\
		u->attr+=e->level*step;\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=attr_init,\
	.inited=effect_update_attr,\
	.update_attr=name##_update_attr,\
	.flag=EFFECT_ATTR|EFFECT_KEEP,\
	.prior=64\
}};
effect_attr_d(DEF,def,(e->level>0?0.5:0.2)*u->def)
effect_attr_d(CE,crit_effect,0.75)
effect_attr_d(PDB,physical_bonus,0.5)
effect_attr_d(MDB,magical_bonus,0.5)
effect_attr_d(PDD,physical_derate,0.5)
effect_attr_d(MDD,magical_derate,0.5)
void steel_flywheel(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.2)){
		attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
		effect(cursed,t,s,0,3);
	}
	setcooldown(s,s->move_cur,3);
}
void holylight_heavycannon(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.2)){
		attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
		effect(radiated,t,s,0,3);
	}
	setcooldown(s,s->move_cur,3);
}
void ground_force(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,40,DAMAGE_REAL,0,TYPE_SOIL);
}
void spoony_spell(struct unit *s){
	struct unit *t=s->osite;
	unsigned long dmg=2.1*s->atk+0.3*s->base->max_hp;
	dmg=attack(t,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
	attack(s,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
}
void self_explode(struct unit *s){
	struct unit *t=s->osite;
	unsigned long dmg=1.8*s->atk+0.25*s->base->max_hp;
	attack(t,s,dmg,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	attack(t,s,dmg,DAMAGE_MAGICAL,0,TYPE_NORMAL);
	sethp(s,0);
}
void health_exchange(struct unit *s){
	struct unit *t=s->osite;
	unsigned long b=t->hp;
	sethp(t,s->hp);
	sethp(s,b);
	setcooldown(s,s->move_cur,5);
}
void spi_exchange(struct unit *s){
	struct unit *t=s->osite;
	long b=t->spi;
	setspi(t,s->spi);
	setspi(s,b);
}
void urgently_repair(struct unit *s){
	heal(s,s->base->max_hp/2);
	setcooldown(s,s->move_cur,4);
}
void double_slash(struct unit *s){
	struct unit *t=gettarget(s);
	long vd;
	if(hittest(t,s,1.0))
		attack(t,s,0.3*s->atk,DAMAGE_PHYSICAL,t->hp==t->base->max_hp?AF_CRIT:0,TYPE_WIND);
	if(s->move_cur->mlevel&MLEVEL_CONCEPTUAL){
		vd=40+1.3*s->def;
		if(vd<60)
			vd=60;
		addhp(s->osite,-vd);
	}
}
void petrifying_ray(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		effect(petrified,t,s,0,3);
	}
	setcooldown(s,s->move_cur,4);
}
void leech_seed(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		effect(parasitized,t,s,0,5);
	}
	setcooldown(s,s->move_cur,4);
}
void soften(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		effect(ATK,t,s,-1,-1);
}
void iron_wall(struct unit *s){
	effect(PDD,s,s,1,-1);
	effect(MDD,s,s,1,-1);
}
void spi_blow(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	setspi(t,t->spi+0.02*s->atk);
}
void spi_shattering_slash(struct unit *s){
	struct unit *t;
	if(s->spi){
		setspi(s,0);
		instant_death(s->osite);
		unit_cooldown_decrease(s,3);
		return;

	}
	t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.5*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		effect(paralysed,t,s,0,3);
	}
}
void angry(struct unit *s){
	effect(ATK,s,s,1,-1);
}
void spi_fcrack(struct unit *s){
	struct unit *t=gettarget(s);
	unsigned long x=0,y=0;
	long ds;
	if(hittest(t,s,1.0)){
		x=attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	if(hittest(t,s,1.0)){
		y=attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	ds=0.015*s->atk;
	if(x<20||y<20||gcdul(x,y)==1)
		ds*=2;
	setspi(t,t->spi-ds);
}
int natural_shield_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest)
		return 0;
	effect_event(e);
	if(*value<3)
		*value=0;
	else
		*value=log(*value);
	effect_event_end(e);
	return 0;
}
const struct effect_base natural_shield_effect[1]={{
	.id="natural_shield",
	.flag=EFFECT_POSITIVE,
	.damage=natural_shield_damage,
	.prior=-25
}};
void natural_shield(struct unit *s){
	effect(natural_shield_effect,s,s,0,5);
	setcooldown(s,s->move_cur,13);
}
void effect_destruct(struct effect *e){
	effect_end(e);
}
int alkali_fire_seal_heal(struct effect *e,struct unit *dest,unsigned long *value){
	if(dest!=e->dest)
		return 0;
	effect_event(e);
	*value*=0.85;
	effect_event_end(e);
	return 0;
}
const struct effect_base alkali_fire_seal[1]={{
	.id="alkali_fire_seal",
	.heal=alkali_fire_seal_heal,
	.flag=EFFECT_NEGATIVE|EFFECT_UNPURIFIABLE,
	.prior=-5
}};
void rfdisillusionfr_action(const struct event *ev,struct unit *src){
	unsigned long def=src->def>16?src->def:16;
	struct player *p=src->owner->enemy;
	for_each_unit(u,p){
		if(u!=p->front&&!unit_findeffect(u,alkali_fire_seal))
			continue;
		attack(u,src,def*u->base->max_hp/4096,DAMAGE_REAL,0,TYPE_ALKALIFIRE);
	}
}
const struct event rfdisillusionfr[1]={{
	.id="rfdisillusionfr",
	.un={rfdisillusionfr_action}
}};
void metal_bomb_inited(struct effect *e){
	if((e->dest->type0|
	e->dest->type1|
	e->src->type0|
	e->src->type1)&TYPE_WATER)
		effect_end(e);
}
void metal_bomb_end(struct effect *e){
	effect_event(e);
	attack(e->dest,e->src,1.15*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
	if(unit_findeffect(e->dest,alkali_fire_seal))
		event(rfdisillusionfr,e->src);
	else
		effect(alkali_fire_seal,e->dest,e->src,0,16);
	effect_event_end(e);
}
const struct effect_base metal_bomb_effect[1]={{
	.id="metal_bomb",
	.end=metal_bomb_end,
	.inited=metal_bomb_inited,
	.roundend=effect_destruct,
	.flag=EFFECT_NEGATIVE|EFFECT_ISOLATED,
	.prior=5
}};
void metal_bomb(struct unit *s){
	effect(metal_bomb_effect,s->osite,s,0,0);
}
void fate_destroying_slash(struct unit *s){
	struct unit *t=gettarget(s);
	double coef=(0.12+0.00012*(s->atk+(s->def>0?s->def:0)));
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
	}
	attack(t,s,(t->base->max_hp-t->hp)*coef,DAMAGE_REAL,0,TYPE_FIGHTING);
	if(!isalive(t->state))
		heal(s,s->base->max_hp*coef);
}
int avoid_hittest(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate){
	if(e->dest!=dest)
		return -1;
	effect_event(e);
	effect_event_end(e);
	return 0;
}
const struct effect_base avoid[1]={{
	.hittest=avoid_hittest,
	.prior=0
}};
void mosquito_bump(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.1))
		attack(t,s,0.9*s->atk,DAMAGE_PHYSICAL,0,TYPE_GHOST);
	effect(avoid,s,s,0,1);
	setcooldown(s,s->move_cur,2);
}
int frost_destroying_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=round;
	return 0;
}
void frost_destroying_cd(struct effect *e,struct unit *u,struct move *m,int *round){
	if(e->dest!=u||*round<=0||
		*e->dest->owner->field->stage!=STAGE_ROUNDEND)
		return;
	if(m->cooldown>0&&test(0.5)){
		effect_event(e);
		*round=0;
		effect_event_end(e);
	}
}
int frost_destroying_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(e->dest==dest&&*damage_type==DAMAGE_PHYSICAL){
		effect_event(e);
		*value*=1.1;
		effect_event_end(e);
	}
	return 0;
}
void frost_destroying_move_end(struct effect *e,struct unit *u,struct move *m){
	if(e->dest==u&&m->cooldown>=0){
		effect_event(e);
		setcooldown(u,m,m->cooldown+1);
		//++m->cooldown;
		//report(u->owner->field,MSG_UPDATE,m);
		effect_event_end(e);
	}
}
const struct effect_base frost_destroying[1]={{
	.id="frost_destroying",
	.init=frost_destroying_init,
	.damage=frost_destroying_damage,
	.cooldown_decrease=frost_destroying_cd,
	.move_end=frost_destroying_move_end,
	.flag=EFFECT_NEGATIVE,
	.prior=-18
}};

void primordial_breath_attack_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(e->dest==src&&type==TYPE_ICE&&damage_type==DAMAGE_PHYSICAL&&dest->owner==e->dest->owner->enemy&&!(aflag&AF_CRIT)&&value*2<=dest->base->max_hp){
		effect_event(e);
		if(unit_findeffect(dest,frost_destroying)){
			effect(PDD,dest,src,-1,-1);
		}else
			effect(frost_destroying,dest,src,0,3);
		effect_event_end(e);
	}
}
const struct effect_base primordial_breath[1]={{
	.id="primordial_breath",
	.attack_end=primordial_breath_attack_end,
	.flag=EFFECT_PASSIVE,
	.prior=32
}};
void primordial_breath_init(struct unit *s){
	effect(primordial_breath,s,s,0,-1);
}
void damage_recuring_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	effect_event(e);
	addhp(dest->osite,-(long)value);
	effect_event_end(e);
}
const struct effect_base damage_recuring[1]={{
	.id="damage_recuring",
	.damage_end=damage_recuring_damage_end,
}};
void damage_recur(struct unit *s){
	effect(damage_recuring,NULL,s,0,4);
	setcooldown(s,s->move_cur,11);
}
void scorching_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	n=unit_hasnegative(s);
	if(n<=0){
		t=gettarget(s);
		if(hittest(t,s,2.5)){
			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
			if(test(0.2))
				effect(burnt,t,s,0,5);
		}
		e=unit_findeffect(t,ATK);
		n=e?e->level:0;
		e=unit_findeffect(s,ATK);
		n1=e?e->level:0;
		if(n>n1+1)
			n=(n-n1)/2+1;
		else
			n=1;
		effect(ATK,s,s,n,-1);
		return;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_FIRE);
	heal(s,0.18*n*s->base->max_hp);

	e=unit_findeffect(s,ATK);
	n1=e?e->level:0;
	if(n1<0)
		purify(e);
	effect(ATK,s,s,1+n,-1);
}
void thunder_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	unsigned long dmg;
	n=unit_hasnegative(s);
	if(n<=0){
		t=gettarget(s);
		if(hittest(t,s,2.5)){

			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_ELECTRIC);
			if(test(0.2))
				effect(paralysed,t,s,0,3);
		}
		e=unit_findeffect(t,ATK);
		n=e?e->level:0;
		e=unit_findeffect(s,ATK);
		n1=e?e->level:0;
		if(n>n1+1)
			n=(n-n1)/2+1;
		else
			n=1;
		effect(ATK,s,s,n,-1);
		return;
	}
	t=s->osite;
	dmg=attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_ELECTRIC);
	for_each_unit(u,t->owner){
		attack(t,s,0.15*dmg+n*0.06*u->base->max_hp,DAMAGE_REAL,0,TYPE_ELECTRIC);
	}
}

void freezing_roaring(struct unit *s){
	struct unit *t;
	long n,n1;
	struct move *mp;
	n=unit_hasnegative(s);
	if(!n||!(s->move_cur->mlevel&MLEVEL_FREEZING_ROARING)){
		struct effect *e;
		t=gettarget(s);
		if(hittest(t,s,2.5)){

			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
			if(test(0.2))
				effect(frozen,t,s,0,3);
		}
		e=unit_findeffect(t,ATK);
		n=e?e->level:0;
		e=unit_findeffect(s,ATK);
		n1=e?e->level:0;
		if(n>n1+1)
			n=(n-n1)/2+1;
		else
			n=1;
		effect(ATK,s,s,n,-1);
		return;
	}
	for_each_effect(e,s->owner->field->effects){
		if(e->dest!=s||!effect_isnegative(e))
			continue;
		effect_final(e);
	}
	t=s->osite;
	n1=(long)t->hp;
	t->hp=0;
	report(s->owner->field,MSG_HPMOD,t,-n1);
	t->state=UNIT_FREEZING_ROARINGED;
	if(t==t->owner->front)
		t->owner->action=ACT_ABORT;
	report(s->owner->field,MSG_FAIL,t);
	unit_wipeeffect(t,0);
	//for(mp=s->moves,*endp=mp+8;mp<endp;++mp){
	//	if(!mp->id)
	//		continue;
	//	if(mp->cooldown){
	//		mp->cooldown=0;
	//		report(s->owner->field,MSG_UPDATE,mp);
	//	}
	//}
	mp=s->move_cur;
	if(mp->cooldown){
		mp->cooldown=0;
		report(s->owner->field,MSG_UPDATE,mp);
	}
}

void triple_cutter(struct unit *s){
	struct unit *t;
	for(int i=0;i<3;++i){
		t=gettarget(s);
		if(hittest(t,s,1.0))
			attack(t,s,0.33*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	}
}
void thorns_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	unsigned long dmg;
	if(dest!=e->dest)
		return;
	switch(damage_type){
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			if(src&&(dmg=value*0.15))
				break;
		default:
			return;
	}
	effect_event(e);
	attack(src,dest,dmg,DAMAGE_REAL,aflag,type);
	effect_event_end(e);
}
const struct effect_base thorns[1]={{
	.id="thorns",
	.damage_end=thorns_damage_end,
	.flag=EFFECT_PASSIVE,
}};
void thorns_init(struct unit *s){
	effect(thorns,s,s,0,-1);
}

void combo_attack_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(!src||src!=e->dest)
		return;
	switch(damage_type){
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return;
	}
	if(test(e->level/100.0)){
		effect_event(e);
		normal_attack(src);
		effect_event_end(e);
	}
}
const struct effect_base combo[1]={{
	.id="combo",
	.attack_end=combo_attack_end,
	.flag=EFFECT_PASSIVE
}};
void combo_init(struct unit *s){
	effect(combo,s,s,20,-1);
}

void hitback_attack_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(!src||dest!=e->dest)
		return;
	switch(damage_type){
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			break;
		default:
			return;
	}
	if(test(e->level/100.0)){
		effect_event(e);
		normal_attack(dest);
		effect_event_end(e);
	}
}
const struct effect_base hitback[1]={{
	.id="hitback",
	.attack_end=hitback_attack_end,
	.flag=EFFECT_PASSIVE
}};
void hitback_init(struct unit *s){
	effect(hitback,s,s,20,-1);
}
void bm_roundend(struct effect *e){
	effect_event(e);
	for_each_unit(u,e->src->owner->enemy){
		attack(u,NULL,u->base->max_hp/20,DAMAGE_REAL,0,TYPE_VOID);
	}
	for_each_unit(u,e->src->owner){
		attack(u,NULL,u->base->max_hp/20,DAMAGE_REAL,0,TYPE_VOID);
	}
	effect_event_end(e);
}
void effect_update_attr_all(struct effect *e){
	update_attr_all((e->dest?e->dest:e->src)->owner->field);
}
void bm_update_attr(struct effect *e,struct unit *u){
	u->atk*=0.75;
}
const struct effect_base blood_moon[1]={{
	.id="blood_moon",
	.inited=effect_update_attr_all,
	.roundend=bm_roundend,
	.update_attr=bm_update_attr,
	.flag=EFFECT_ENV,
	.prior=30
}};
void thermobaric(struct unit *s){
	struct unit *t;
	t=gettarget(s);
	if(hittest(t,s,1.5))
		attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
	effect(blood_moon,NULL,s,0,3);
	setcooldown(s,s->move_cur,6);
}

void kaleido_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(src!=e->dest||dest->owner!=src->owner->enemy||damage_type!=DAMAGE_PHYSICAL||value<3)
		return;
	effect_event(e);
	attack(dest,src,0.35*value,DAMAGE_MAGICAL,0,TYPE_DEVINEGRASS);	
	effect_event_end(e);
}
void kaleido_roundend(struct effect *e){
	long v=(long)e->dest->base->max_hp-(long)e->dest->hp;
	if(!v)
		return;
	if(v<25)
		v=25;
	effect_event(e);
	heal(e->dest,v*4/25);
	effect_event_end(e);
}
const struct effect_base kaleido[1]={{
	.id="kaleido",
	.damage_end=kaleido_damage_end,
	.roundend=kaleido_roundend,
	.flag=EFFECT_PASSIVE,
	.prior=-128,
}};
void kaleido_init(struct unit *s){
	effect(kaleido,s,s,0,-1);
}
int hot_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_FIRE|TYPE_ICE))||!(*damage_type&(DAMAGE_PHYSICAL|DAMAGE_MAGICAL)))
		return 0;
	effect_event(e);
	if(*type&TYPE_FIRE)
		*value*=1.5;
	if(*type&TYPE_ICE)
		*value*=0.5;
	effect_event_end(e);
	return 0;
}
const struct effect_base hot[1]={{
	.id="hot",
	.damage=hot_damage,
	.flag=EFFECT_ENV,
}};
void ablaze(struct unit *s){
	struct unit *t;
	effect(hot,NULL,s,0,3);
	t=gettarget(s);
	if(hittest(t,s,1.5))
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
	setcooldown(s,s->move_cur,6);
}

int wet_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_FIRE|TYPE_WATER))||!(*damage_type&(DAMAGE_PHYSICAL|DAMAGE_MAGICAL)))
		return 0;
	effect_event(e);
	if(*type&TYPE_WATER)
		*value*=1.5;
	if(*type&TYPE_FIRE)
		*value*=0.5;
	effect_event_end(e);
	return 0;
}
const struct effect_base wet[1]={{
	.id="wet",
	.damage=wet_damage,
	.flag=EFFECT_ENV,
}};
void spray(struct unit *s){
	struct unit *t;
	effect(hot,NULL,s,0,3);
	t=gettarget(s);
	if(hittest(t,s,1.5))
		effect(HIT,t,s,-2,-1);
	setcooldown(s,s->move_cur,6);
}
int cold_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_FIRE|TYPE_ICE))||!(*damage_type&(DAMAGE_PHYSICAL|DAMAGE_MAGICAL)))
		return 0;
	effect_event(e);
	if(*type&TYPE_ICE)
		*value*=1.5;
	if(*type&TYPE_FIRE)
		*value*=0.5;
	effect_event_end(e);
	return 0;
}
const struct effect_base cold[1]={{
	.id="cold",
	.damage=cold_damage,
	.flag=EFFECT_ENV,
}};
void cold_wave(struct unit *s){
	struct unit *t;
	effect(cold,NULL,s,0,3);
	t=gettarget(s);
	if(hittest(t,s,1.5))
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
	setcooldown(s,s->move_cur,6);
}

int gp_init(struct effect *e,long level,int round){
	level+=e->level;
	if(level<0)
		level=0;
	if(level>16)
		level=16;
	if(!e->round&&round>=0)
		e->round=round;
	e->level=level;
	return 0;
}
//const struct effect_base gravitational_potential[1];
void gp_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(dest!=e->dest||(damage_type!=DAMAGE_PHYSICAL&&damage_type!=DAMAGE_MAGICAL))
		return;
	//if(e->level>3){
	effect_reinit(e,NULL,damage_type==DAMAGE_PHYSICAL?-3:-2,-1);
		//e->level-=damage_type==DAMAGE_PHYSICAL?3:2;
		//report(e->dest->owner->field,MSG_UPDATE,e);
	//}else
	//	effect_end(e);
}
const struct effect_base gravitational_potential[1]={{
	.id="gravitational_potential",
	.init=gp_init,
	.damage_end=gp_damage_end,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE
}};
void bh_inited(struct effect *e){
	effect(gravitational_potential,e->src,NULL,16,12);
	effect(gravitational_potential,e->src->osite,NULL,16,12);
}
int bh_su(struct effect *e,struct unit *u){
	return -1;
}
void bh_end(struct effect *e){
	unsigned long gp0,gp1;
	struct unit *s=e->src->owner->front;
	struct unit *t=s->osite;
	struct effect *ep;
	ep=unit_findeffect(s,gravitational_potential);
	if(ep){
		gp0=ep->level;
		effect_end(ep);
	}else
		gp0=0;
	ep=unit_findeffect(t,gravitational_potential);
	if(ep){
		gp1=ep->level;
		effect_end(ep);
	}else
		gp1=0;
	if(gp0==gp1){
		gp0=s->speed;
		gp1=t->speed;
	}
	if(gp0<gp1)
		sethp(s,0);
	else
		sethp(t,0);
}
const struct effect_base black_hole[1]={{
	.id="black_hole",
	.inited=bh_inited,
	.switchunit=bh_su,
	.end=bh_end,
	.flag=EFFECT_ENV
}};
void collapse(struct unit *s){
	struct unit *t;
	t=gettarget(s);
	if(hittest(t,s,1.8)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_ROCK);
		effect(DEF,t,s,-1,-1);
	}
	effect(black_hole,NULL,s,0,4);
	setcooldown(s,s->move_cur,9);
}
int duckcheck(const struct unit *u){
	return !strcmp(u->base->id,"giant_mouth_duck");
}
int cyce_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if((*aflag&AF_IDEATH)&&duckcheck(dest)){
		effect_event(e);
		effect_event_end(e);
		return 1;
	}else
		return 0;
}
int cyce_hittest(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate){
	if(duckcheck(dest)){
		effect_event(e);
		effect_event_end(e);
		return 0;
	}else if(duckcheck(src)){
		effect_event(e);
		effect_event_end(e);
		return 1;
	}else
		return -1;
}
void cyce_trydamage(struct unit *u){
	if(!duckcheck(u))
		attack(u,NULL,u->base->max_hp/9,DAMAGE_REAL,0,TYPE_MACHINE);
}
void cyce_roundend(struct effect *e){
	struct player *p;
	effect_event(e);
	p=e->src->owner;
	cyce_trydamage(p->front);
	cyce_trydamage(p->enemy->front);
	effect_event_end(e);
}
const struct effect_base cycle_eden[1]={{
	.id="cycle_eden",
	.damage=cyce_damage,
	.hittest=cyce_hittest,
	.roundend=cyce_roundend,
	.flag=EFFECT_ENV
}};
void cycle_erode(struct unit *s){
	struct unit *t;
	int plv;
	if(!duckcheck(s))
		return;
	plv=test(0.5)?2:1;
	t=gettarget(s);
	if(effect(PDD,t,s,-plv,-1))
		effect(PDD,s,s,plv,-1);
	attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	effect(cycle_eden,NULL,s,0,4);
	setcooldown(s,s->move_cur,8);
}
void cycle_erode_init(struct unit *s){
	struct unit *t;
	if(!isfront(s)||!duckcheck(s))
		return;
	t=s->osite;
	attack(t,s,1.3*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	effect(ATK,t,s,-2,-1);
}

void soul_back(struct unit *s){
	for_each_unit(u,s->owner){
		if(isalive(u->state))
			continue;
		if(!revive(u,u->base->max_hp)){
			sethp(s,0);
			setcooldown(s,s->move_cur,16);
		}
		return;
	}
}
void absolutely_immortal_kill(struct effect *e,struct unit *u){
	if(u==e->dest){
		effect_event(e);
		sethp(u,1);
		effect_event_end(e);
	}
}
int absolutely_immortal_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest){
		if(*value>=dest->hp){
			effect_event(e);
			*value=dest->hp-1;
			effect_event_end(e);
		}
	}
	return 0;
}
const struct effect_base absolutely_immortal_effect[1]={{
	.id="absolutely_immortal",
	.damage=absolutely_immortal_damage,
	.kill=absolutely_immortal_kill,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_NONHOOKABLE,
	.prior=-128
}};
void absolutely_immortal(struct unit *s){
	effect(absolutely_immortal_effect,s,s,0,5);
	setcooldown(s,s->move_cur,15);
}
void overload(struct unit *s){
	struct unit *t=gettarget(s);
	if(5*labs(t->spi)>t->base->max_spi*3){
		instant_death(t);
		effect(ATK,s,s,2,-1);
	}else {
		effect(ATK,s,s,1,-1);
		if(hittest(t,s,1.0))
			attack(t,s,0.95*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
}
void escape(struct unit *s){
	struct unit *t=gettarget(s);
	struct unit *ring[12];
	size_t rsize=0,sindex=0;
	for(int i=0;i<6;++i){
		t=s->owner->units+i;
		if(!t->base||!isalive(t->state))
			continue;
		if(t==s)
			sindex=rsize;
		ring[rsize++]=t;
	}
	memcpy(ring+rsize,ring,rsize*sizeof(struct unit *));
	t=ring[sindex+1];
	if(t==s)
		return;
	switchunit(t);
}

struct move *getmove(struct unit *u){
	struct battle_field *f=u->owner->field;
	const struct move *ret=NULL;
	int round=*f->round,r;
	for(const struct message *p=f->rec+f->rec_size-1;p>=f->rec;--p){
		if(p->type!=MSG_MOVE
		||p->un.move.u!=u)
			continue;
		if(p->round!=round)
			break;
		switch(p->un.move.m-p->un.move.u->moves){
			case 0 ... 7:
				ret=p->un.move.m;
			default:
				break;
		}
	}
	if(!ret){
		switch((r=u->owner->action)){
			case ACT_MOVE0 ... ACT_MOVE7:
				if(u->moves[r].id){
					return u->moves+r;
				}
			default:
				break;
		}
	}
	return (struct move *)ret;
}
void super_scissors(struct unit *s){
	struct unit *t=gettarget(s);
	struct move *mp;
	int r;
	if(hittest(t,s,1.0))
		attack(t,s,0.45*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	switch((r=t->owner->action)){
		case ACT_MOVE0 ... ACT_MOVE7:
			mp=t->moves+r;
			if(!mp->id||mp->action==s->move_cur->action||!(mp->mlevel&MLEVEL_REGULAR))
				break;
			memcpy(s->move_cur,mp,sizeof(struct move));
			s->move_cur->mlevel&=MLEVEL_REGULAR;
			report(s->owner->field,MSG_UPDATE,s->move_cur);
		default:
			break;
	}
}
void sonic(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		effect(confused,t,s,0,3);
}
void maya_mirror(struct unit *s){
	struct unit *t=s->osite;
	struct move m;
	int r;
	if(hittest(t,s,1.0))
		attack(t,s,0.20*s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
	switch((r=t->owner->action)){
		case ACT_MOVE0 ... ACT_MOVE7:
			if(!t->moves[r].id)
				break;
			memcpy(&m,t->moves+r,sizeof(struct move));
			if(m.action==s->move_cur->action||!(m.mlevel&(MLEVEL_REGULAR|MLEVEL_CONCEPTUAL)))
				break;
			m.mlevel&=MLEVEL_REGULAR|MLEVEL_CONCEPTUAL;
			unit_move(s,&m);
			setcooldown(s,s->move_cur,m.cooldown);
			break;
		case ACT_NORMALATTACK:
			normal_attack(s);
			break;
		default:
			break;
	}
}
void cog_hit(struct unit *s){
	struct unit *t=gettarget(s);
	if(!hittest(t,s,1.0))
		return;
	attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	effect(DEF,t,s,-1,-1);
}
void defend(struct unit *s){
	effect(DEF,s,s,2,-1);
}
void heat_engine_move_end(struct effect *e,struct unit *u,struct move *m){
	struct effect *ep;
	if(u==e->dest&&
		(m->type&TYPE_MACHINE)&&
		u->spi>=2-u->base->max_spi&&
		(ep=unit_findeffect(u->osite,burnt))
	){
		effect_event(e);
		setspi(u,u->spi-2);
		effect_setround(ep,ep->round+1);
		//++ep->round;
		//report(u->owner->field,MSG_UPDATE,ep);
		ep->base->roundend(ep);
		effect_event_end(e);
	}
}
const struct effect_base heat_engine[1]={{
	.id="heat_engine",
	.move_end=heat_engine_move_end,
	.flag=EFFECT_PASSIVE
}};
void heat_engine_init(struct unit *s){
	effect(heat_engine,s,s,0,-1);
}

void flamethrower(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		effect(burnt,t,s,0,5);
}
void time_back_locally(struct unit *s){
	struct battle_field *f=s->owner->field;
	struct unit *t;
	for(const struct message *p=f->rec+f->rec_size-1;p>=f->rec;--p){
		if(p->type!=MSG_FAIL
		||p->un.u->owner!=s->owner
		||isalive(p->un.u->state))
			continue;
		t=(struct unit *)p->un.u;
		goto found;
	}
	goto fail;
found:
	if(!revive_nonhookable(t,t->base->max_hp/2)){
		setcooldown(s,s->move_cur,31);
		switchunit(t);
		return;
	}
fail:
	setcooldown(s,s->move_cur,4);
}
unsigned long damage_get_in_round(struct unit *s,int round,int damage_types){
	struct battle_field *f=s->owner->field;
	unsigned long dmg=0;
	for(const struct message *p=f->rec+f->rec_size-1;p>=f->rec;--p){
		if(p->type!=MSG_DAMAGE
		||!(damage_types&(1<<p->un.damage.damage_type))
		||p->un.damage.dest!=s)
			continue;
		if(p->round!=round)
			break;
		dmg+=p->un.damage.value;
	}
	return dmg;
}
void star_move(struct unit *s){
	unsigned long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_ALL_FLAG);
	if(dmg){
		attack(s->osite,s,dmg,DAMAGE_REAL,0,TYPE_VOID);
		heal(s,dmg);
		setcooldown(s,s->move_cur,s->state==UNIT_CONTROLLED?3:2);
	}
}
struct unit *rebound_gettarget(struct effect *e,struct unit *u){
	if(u->owner!=e->dest->owner){
		effect_event(e);
		effect_event_end(e);
		return u;
	}
	return NULL;
}
const struct effect_base rebound_effect[1]={{
	.gettarget=rebound_gettarget,
	.flag=EFFECT_POSITIVE
}};
void rebound(struct unit *s){
	effect(rebound_effect,s,s,0,1);
	setcooldown(s,s->move_cur,3);
}
extern const struct effect_base force_vh_effect[1];
void force_vh_p(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e=unit_findeffect(s,force_vh_effect);
	if(hittest(t,s,1.0))
		attack(t,s,2*s->atk/3,DAMAGE_PHYSICAL,0,TYPE_DRAGON);
	if(e)
		e->active=1;
}

void vh_roundend(struct effect *e){
	struct move *m;
	struct unit *t;
	if(!e->active)
		return;
	m=getmove(t=e->dest->osite);
	if(m){
		setcooldown(t,m,-1);
	}
	effect_end(e);
}
int vh_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=round;
	return 0;
}
const struct move force_vh_pm={
	.id="force_vh",
	.action=force_vh_p,
	.type=TYPE_DRAGON,
	.prior=0,
	.flag=MOVE_NOCONTROL,
	.mlevel=MLEVEL_REGULAR
};
void vh_move_end(struct effect *e,struct unit *u,struct move *m){
	struct move am;
	if(e->round>1||e->active||e->dest!=u||!isfront(u))
		return;
	effect_event(e);
	memcpy(&am,&force_vh_pm,sizeof(struct move));
	unit_move(u,&am);
	effect_event_end(e);
}
const struct effect_base force_vh_effect[1]={{
	.id="force_vh",
	.init=vh_init,
	.move_end=vh_move_end,
	.roundend=vh_roundend,
	.flag=EFFECT_POSITIVE
}};
void force_vh(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,s->atk/3,DAMAGE_PHYSICAL,0,TYPE_DRAGON);
	effect(force_vh_effect,s,s,0,2);
	setcooldown(s,s->move_cur,2);
}
void back(struct player *p,const struct player *h){
	struct unit *b;
	for(int i=0;i<6&&p->units[i].base;++i){
		if(p->units[i].state==UNIT_FREEZING_ROARINGED)
			continue;
		memcpy(p->units+i,h->units+i,offsetof(struct unit,owner));
		report(p->field,MSG_UPDATE,p->units+i);
	}
	p->action=ACT_ABORT;
	if(p->front!=h->front){
		b=p->front;
		p->front=h->front;
		report(p->field,MSG_SWITCH,p->front,b);
	}
}
void time_back(struct unit *s){
	struct battle_field *f=s->owner->field;
	struct history *h;
	int round=*f->round-20,off=s->move_cur-s->moves;
	if(round<0)
		round=0;
	if(round>=f->ht_size)
		return;
	h=f->ht+round;
	back(f->p,&h->p);
	back(f->e,&h->e);
	update_attr_all(f);
	if(s->moves[off].action==time_back)
		setcooldown(s,s->move_cur,41);
}
int repeat_action(struct effect *e,struct player *p){
	if(e->dest!=p->front)
		return 0;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
		case ACT_UNIT0 ... ACT_UNIT5:
		case ACT_NORMALATTACK:
		case ACT_ABORT:
			effect_event(e);
			p->action=e->level;
			effect_event_end(e);
		default:
			break;
	}
	return 0;
}
const struct effect_base repeat_effect[1]={{
	.id="repeat",
	.init=abnormal_init,
	.action=repeat_action,
	.flag=EFFECT_NEGATIVE,
}};
int do_repeat(struct unit *s,struct unit *t,int round){
	int r;
	switch((r=t->owner->action)){
		case ACT_MOVE0 ... ACT_MOVE7:
		case ACT_NORMALATTACK:
			if(effect(repeat_effect,t,s,r,round))
				return 0;
		default:
			break;
	}
	return -1;
}
void repeat(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.8)){
		attack(t,s,s->atk/4,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
		if(!do_repeat(s,t,4))
			setcooldown(s,s->move_cur,9);
	}
}
int silent_move(struct effect *e,struct unit *u,struct move *m){
	if(u!=e->dest||(m->flag&(MOVE_NOCONTROL|MOVE_NORMALATTACK)))
		return 0;
	return -1;
}
const struct effect_base silent[1]={{
	.id="silent",
	.init=abnormal_init,
	.move=silent_move,
	.flag=EFFECT_NEGATIVE|EFFECT_CONTROL
}};
void head_blow(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
		effect(silent,t,s,0,4);
	}
	setcooldown(s,s->move_cur,4);
}

void mecha_update_attr(struct effect *e,struct unit *u){
	if(u==e->dest){
		u->type0=TYPE_MACHINE;
	}
}
const struct effect_base mecha_effect[1]={{
	.id="mecha",
	.inited=effect_update_attr,
	.update_attr=mecha_update_attr
}};
void mecha(struct unit *s){
	effect(mecha_effect,s,s,0,5);
}
void perish_kill_end(struct effect *e,struct unit *u){
	if(u!=e->dest)
		return;
	effect_setround(e,-1);
	//e->round=-1;
	//report(e->dest->owner->field,MSG_UPDATE,e);
}
int perish_revive(struct effect *e,struct unit *u,unsigned long *hp){
	if(u==e->dest){
		effect_event(e);
		return -1;
		effect_event_end(e);
	}
	return 0;
}
const struct effect_base perish_song_effect[1]={{
	.id="perish_song",
	.kill_end=perish_kill_end,
	.revive=perish_revive,
	.flag=EFFECT_NEGATIVE|EFFECT_KEEP
}};
void perish_song(struct unit *s){
	struct unit *t=gettarget(s);
	effect(perish_song_effect,t,s,0,1);
	effect(perish_song_effect,s,s,0,1);
	if(hittest(t,s,1.2)){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
		attack(t,s,0.16*(t->base->max_hp-t->hp),DAMAGE_REAL,0,TYPE_NORMAL);
	}
	setcooldown(s,s->move_cur,3);
}

int marsh_getprior(struct effect *e,struct player *p){
	effect_event(e);
	effect_event_end(e);
	return -1;
}
const struct effect_base marsh[1]={{
	.id="marsh",
	.getprior=marsh_getprior,
	.flag=EFFECT_ENV,
}};
void mud_shot(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.4))
		attack(t,s,1+t->hp/6,DAMAGE_REAL,0,TYPE_SOIL);
	effect(marsh,NULL,s,0,6);
	setcooldown(s,s->move_cur,5);
}
void nether_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	n=unit_hasnegative(s);
	if(n<=0){
		t=gettarget(s);
		if(hittest(t,s,2.5)){
			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_GHOST);
			if(test(0.2)){
				effect(ATK,s,s,1,-1);
				effect(DEF,s,s,1,-1);
				effect(SPEED,s,s,1,-1);
				effect(HIT,s,s,1,-1);
				effect(AVOID,s,s,1,-1);
				effect(CE,s,s,1,-1);
				effect(PDB,s,s,1,-1);
				effect(MDB,s,s,1,-1);
				effect(PDD,s,s,1,-1);
				effect(MDD,s,s,1,-1);
			}
		}
		e=unit_findeffect(t,ATK);
		n=e?e->level:0;
		e=unit_findeffect(s,ATK);
		n1=e?e->level:0;
		if(n>n1+1)
			n=(n-n1)/2+1;
		else
			n=1;
		effect(ATK,s,s,n,-1);
		return;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_GHOST);
	effect(AVOID,s,s,n*2,-1);
}
void tidal(struct unit *s){
	struct unit *t=gettarget(s);
	for_each_effect(e,s->owner->field->effects){
		if(e->dest==t&&(e->base->flag&EFFECT_ATTR)&&e->level>0)
			purify(e);
	}
	attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_DEVINEWATER);
}

int rand_action(const struct player *p){
	int n=0,c=0;
	for(int i=ACT_MOVE0;i<ACT_ABORT;++i)
		if(canaction2(p,i)){
			c|=1<<i;
			++n;
		}
	if(!n)
		return ACT_ABORT;
	n=randi()%n;
	for(int i=ACT_MOVE0;i<ACT_ABORT;++i){
		if((1<<i)&c){
			if(!n)
				return i;
			--n;
		}
	}
	__builtin_unreachable();
}
int interfered_action(struct effect *e,struct player *p){
	if(e->dest!=p->front)
		return 0;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			if(p->front->moves[p->action].flag&MOVE_NOCONTROL)
				break;
		case ACT_UNIT0 ... ACT_UNIT5:
		case ACT_NORMALATTACK:
		case ACT_ABORT:
			effect_event(e);
			p->action=rand_action(p);
			effect_event_end(e);
		default:
			break;
	}
	return 0;
}
const struct effect_base interfered[1]={{
	.id="interfered",
	.init=abnormal_init,
	.action=interfered_action,
	.flag=EFFECT_NEGATIVE
}};
void interference(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.8)){
		attack(t,s,s->atk/4,DAMAGE_PHYSICAL,0,TYPE_BUG);
		if(effect(interfered,t,s,0,4))
			setcooldown(s,s->move_cur,9);
	}
}
void anger_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	n=unit_hasnegative(s);
	if(n<=0){
		t=gettarget(s);
		if(hittest(t,s,2.5)){
			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
			if(test(0.2))
				effect(silent,t,s,0,4);
		}
		e=unit_findeffect(t,ATK);
		n=e?e->level:0;
		e=unit_findeffect(s,ATK);
		n1=e?e->level:0;
		if(n>n1+1)
			n=(n-n1)/2+1;
		else
			n=1;
		effect(ATK,s,s,n,-1);
		return;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_FIGHTING);
	if((double)t->hp/t->base->max_hp<=0.15+n*0.03)
		instant_death(t);
	else
		attack(t,s,(0.14+n*0.02)*t->base->max_hp,DAMAGE_REAL,0,TYPE_FIGHTING);
}
void poison_sting(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,0,TYPE_POISON);
		if(test(0.35))
			effect(poisoned,t,s,0,5);
	}
}
void hypnosis(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		effect(asleep,t,s,0,3);
	}
	setcooldown(s,s->move_cur,4);
}
void rest(struct unit *s){
	;
}
void moonlight(struct unit *s){
	for_each_effect(e,s->owner->field->effects){
		if(e->dest!=s||!(e->base->flag&EFFECT_ATTR)||e->level>0)
			continue;
		purify(e);
	}

}
void byebye(struct unit *s){
	sethp(s,0);
}
void scent(struct unit *s){
	for_each_effect(e,s->owner->field->effects){
		if(e->dest!=s||!(e->base->flag&EFFECT_ABNORMAL))
			continue;
		purify(e);
	}

}
void synthesis(struct unit *s){
	struct unit *t=gettarget(s);
	heal(s,((t->type0|t->type1)&TYPE_LIGHT?0.55:0.45)*s->base->max_hp);
	setcooldown(s,s->move_cur,4);
}
void hail_pm(struct unit *s){
	struct unit *t=gettarget(s);
	attack(t,s,0.45*s->atk+t->base->max_hp/32.0,DAMAGE_PHYSICAL,0,TYPE_ICE);
}
const struct move hail_p={
	.id="hail",
	.action=hail_pm,
	.type=TYPE_ICE,
	.prior=0,
	.flag=MOVE_NOCONTROL,
	.mlevel=MLEVEL_REGULAR
};
void hail_action_end(struct effect *e,struct player *p){
	struct move am;
	if(e->dest!=p->front)
		return;
	if(!e->active){
		e->active=1;
		return;
	}
	effect_event(e);
	memcpy(&am,&hail_p,sizeof(struct move));
	unit_move(e->dest,&am);
	effect_event_end(e);
}
void hail_inited(struct effect *e){
	e->active=0;
}
const struct effect_base hail_effect[1]={{
	.id="hail",
	.action_end=hail_action_end,
	.action_fail=hail_action_end,
	.flag=EFFECT_POSITIVE
}};
void hail(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	if(hittest(t,s,1.0))
		attack(t,s,0.45*s->atk+t->base->max_hp/32.0,DAMAGE_PHYSICAL,0,TYPE_ICE);
	e=effect(hail_effect,s,s,0,5);
	if(e)
		e->active=0;
}
void clip3(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	if(hittest(t,s,1.0)){
		e=unit_findeffect(t,SPEED);
		attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,e&&e->level<0?AF_CRIT:0,TYPE_NORMAL);
	}
}
void mighty_wave_roundend(struct effect *e){
	struct move *m;
	struct unit *t;
	int n=e->level;
	m=getmove(t=e->dest->osite);
	if(m&&m->cooldown>=0){
		setcooldown(t,m,n+(m->cooldown?m->cooldown:1));
	}
	effect_end(e);
}
const struct effect_base mighty_wave_effect[1]={{
	.roundend=mighty_wave_roundend,
	.flag=EFFECT_POSITIVE
}};
void mighty_wave(struct unit *s){
	struct unit *t=gettarget(s);
	int n=1;
	for_each_effect(e,s->owner->field->effects){
		if(e->dest!=s||!(e->base->flag&EFFECT_ABNORMAL))
			continue;
		purify(e);
		++n;
	}
	if(hittest(t,s,1.2)){
		attack(t,s,0.9*s->atk,DAMAGE_PHYSICAL,0,TYPE_WATER);
	}
	effect(mighty_wave_effect,s,s,n,1);
	setcooldown(s,s->move_cur,3);
}
void cold_wind(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.5*s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
		effect(SPEED,t,s,-1,-1);
	}
}
void entangle(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		effect(SPEED,t,s,-2,-1);
	}
}
int maple_init(struct effect *e,long level,int round){
	level+=e->level;
	if(level<=0)
		return -1;
	e->round=round;
	e->level=level;
	return 0;
}
const struct effect_base maple[1]={{
	.id="maple",
	.init=maple_init,
	.flag=EFFECT_NEGATIVE
}};
void breach_missile(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *me;
	int n=0;
	for_each_effect(e,s->owner->field->effects){
		if(e->dest==t&&(e->base==DEF||e->base==PDD||e->base==MDD)&&e->level>0){
				purify(e);
				n+=e->level;
		}
	}
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	}
	me=effect(maple,t,s,n+1,-1);
	if(me)
		heal(s,10+me->level);
}
void piercing_missile(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e=unit_findeffect(t,maple);
	int n=e?e->level:0;
	double dmg=40+0.05*s->atk+0.03*t->base->max_hp;
	if(n){
		attack(t,s,dmg*pow(1.15,n),DAMAGE_REAL,0,TYPE_NORMAL);
		effect_reinit(e,s,-n,-1);
	}
	else
		attack(t,s,dmg,DAMAGE_REAL,0,TYPE_NORMAL);
}
void amuck(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e=unit_findeffect(t,maple);
	if(e){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,AF_CRIT,TYPE_NORMAL);
		effect(maple,t,s,-1,-1);
	}
	else if(hittest(t,s,1.0))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
}
void reflex(struct unit *s){
	unsigned long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_ALL_FLAG);
	if(dmg){
		attack(s->osite,s,1.5*dmg,DAMAGE_REAL,0,TYPE_VOID);
		setcooldown(s,s->move_cur,2);
	}
}
void physical_reflex(struct unit *s){
	unsigned long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_PHYSICAL_FLAG);
	if(dmg){
		attack(s->osite,s,2*dmg,DAMAGE_REAL,0,TYPE_VOID);
		setcooldown(s,s->move_cur,2);
	}
}
void magical_reflex(struct unit *s){
	unsigned long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_MAGICAL_FLAG);
	if(dmg){
		attack(s->osite,s,2*dmg,DAMAGE_REAL,0,TYPE_VOID);
		setcooldown(s,s->move_cur,2);
	}
}
int damage_reverse_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest)
		return 0;
	effect_event(e);
	heal(dest,*value);
	effect_event_end(e);
	return -1;
}
const struct effect_base damage_reverse_effect[1]={{
	.id="damage_reverse",
	.damage=damage_reverse_damage,
	.flag=EFFECT_POSITIVE,
}};
void damage_reverse(struct unit *s){
	effect(damage_reverse_effect,s,s,0,1);
	setcooldown(s,s->move_cur,4);
}
int shield_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest)
		return 0;
	if(*value>e->level){
		effect_event(e);
		*value-=e->level;
		effect_reinit(e,src,-e->level,-1);
		effect_event_end(e);
		return 0;
	}
	effect_event(e);
	effect_reinit(e,src,-(long)*value,-1);
	effect_event_end(e);
	return -1;
}
int shield_init(struct effect *e,long level,int round){
	if(round&&level>0)
		e->round=round;
	if(level>0)
		e->level=level;
	else if(level<0){
		level+=e->level;
		if(level<0)
			level=0;
		e->level=level;
	}else
		return -1;
	return 0;
}
void shield_roundend(struct effect *e){
	if(!e->level)
		effect_end(e);
}
const struct effect_base shield[1]={{
	.id="shield",
	.flag=EFFECT_POSITIVE,
	.init=shield_init,
	.damage=shield_damage,
	.roundend=shield_roundend
}};
int heal_weak_heal(struct effect *e,struct unit *dest,unsigned long *value){
	double coef;
	if(dest!=e->dest)
		return 0;
	effect_event(e);
	coef=1-*(double *)e->data;
	if(coef>0.0)
		*value*=coef;
	else
		*value=0;
	effect_event_end(e);
	return 0;
}
const struct effect_base heal_weak[1]={{
	.id="heal_weak",
	.heal=heal_weak_heal,
	.flag=EFFECT_NEGATIVE
}};
int heal_bonus_heal(struct effect *e,struct unit *dest,unsigned long *value){
	double coef;
	if(dest!=e->dest)
		return 0;
	effect_event(e);
	coef=1+*(double *)e->data;
	if(coef>1)
		*value*=coef;
	effect_event_end(e);
	return 0;
}
const struct effect_base heal_bonus[1]={{
	.id="heal_bonus",
	.heal=heal_bonus_heal,
	.flag=EFFECT_POSITIVE
}};
void elbow_roundend(struct effect *e){
	struct move *m;
	struct unit *t;
	int n=e->level;
	if(e->level<=0)
		return;
	m=getmove(t=e->dest->osite);
	if(m&&m->cooldown>=0){
		setcooldown(t,m,n+(m->cooldown?m->cooldown:1));
	}else {
		attack(t,e->dest,(0.3+0.1*n)*(e->dest->atk+0.075*t->hp),DAMAGE_REAL,0,TYPE_FIGHTING);
	}
	effect_end(e);
}
const struct effect_base elbow_effect[1]={{
	.roundend=elbow_roundend,
	.flag=EFFECT_POSITIVE
}};
int elbow1_attack(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	double coef;
	if(dest==e->dest&&*dest->owner->field->stage==STAGE_ROUNDEND){
		coef=1-*(double *)e->data;
		if(coef<0.1)
			coef=0.1;
		effect_event(e);
		*value*=coef;
		effect_event_end(e);
	}
	return 0;
}
const struct effect_base elbow1[1]={{
	.attack=elbow1_attack,
	.flag=EFFECT_POSITIVE
}};
void elbow2_update_attr(struct effect *e,struct unit *u){
	if(e->dest==u)
		u->atk*=1.15;
}
const struct effect_base elbow2[1]={{
	.inited=effect_update_attr,
	.update_attr=elbow2_update_attr,
	.flag=EFFECT_POSITIVE,
	.prior=32
}};
void elbow3_update_attr(struct effect *e,struct unit *u){
	if(e->dest==u){
		if(u->def>=u->base->def)
			u->def*=1.15;
		else
			u->def+=u->base->def*0.15;
	}
}
const struct effect_base elbow3[1]={{
	.inited=effect_update_attr,
	.update_attr=elbow3_update_attr,
	.flag=EFFECT_POSITIVE,
	.prior=32
}};
int mana_init(struct effect *e,long level,int round){
	level+=e->level;
	if(level<=0)
		level=0;
	e->round=-1;
	e->level=level;
	return 0;
}
const struct effect_base mana[1]={{
	.id="mana",
	.init=mana_init,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP
}};
void elbow(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	int x;
	unsigned long dmg=1.05*s->atk;
	long l;
	double d0,d1;
	if(hittest(t,s,1.0))
		attack(t,s,dmg,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
	else
		attack(t,s,0.3*dmg,DAMAGE_REAL,0,TYPE_FIGHTING);
	if(*s->owner->field->stage==STAGE_PRIOR){
		if(!effect(silent,t,s,0,1)){
			attack(t,s,dmg,DAMAGE_MAGICAL,0,TYPE_FIGHTING);
			effect(avoid,s,s,0,1);
		}
	}else {
		dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_ALL_FLAG);
		if(dmg){
			attack(t,s,1.75*dmg,DAMAGE_REAL,0,TYPE_FIGHTING);
			addhp(s,0.25*dmg);
		}
	}
	e=unit_findeffect(s,SPEED);
	l=e?e->level:0;
	if(l>0){
		effect(SPEED,s,s,-2*l,-1);
		heal(s,s->base->max_hp*(0.07+0.05*l));
	}else if(l<0){
		purify(e);
		e=unit_findeffect(s,ATK);
		if(e&&e->level<0){
			l+=e->level;
			purify(e);
		}
		effect(elbow_effect,s,s,1-l,1);
	}else {
		x=0;
		e=unit_findeffect(t,ATK);
		if(e&&e->level>0){
			++x;
			if(purify(e))
				l+=e->level;
		}
		e=unit_findeffect(t,DEF);
		if(e&&e->level>0){
			++x;
			if(purify(e))
				l+=e->level;
		}
		if(l)
			attack(t,s,(0.3+0.1*l)*s->atk,DAMAGE_REAL,0,TYPE_FIGHTING);
		if(!x){
			effect(ATK,s,s,1,-1);
			effect(SPEED,s,s,1,-1);
			effect(shield,s,s,0.08*s->base->max_hp+0.25*s->atk,3);
		}
	}
	d0=(double)s->hp/(double)s->base->max_hp;
	d1=(double)t->hp/(double)t->base->max_hp;
	if(d0<=d1){
		e=effect(heal_bonus,s,s,0,2);
		if(e)
			*(double *)e->data=0.3+(1-d0);
		e=effect(elbow1,s,s,0,1);
		if(e)
			*(double *)e->data=0.3*(2-d0);
	}else {
		e=effect(heal_weak,t,s,0,2);
		if(e)
			*(double *)e->data=0.35+0.2*d1;
	}
	e=unit_findeffect3(s,NULL,EFFECT_ABNORMAL);
	if(e){
		if(e->base==cursed||e->base==radiated){
			heal(s,0.15*s->base->max_hp);
		}else if(e->base==poisoned){
			attack(t,s,0.4*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
		}else if(e->base==burnt){
			attack(t,s,0.4*s->atk,DAMAGE_MAGICAL,0,TYPE_FIGHTING);
		}else if(e->base==parasitized){
			attack(t,s,0.2*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
			attack(t,s,0.2*s->atk,DAMAGE_MAGICAL,0,TYPE_FIGHTING);
		}else
			goto no_abnormal;
	}else {
no_abnormal:
			attack(t,s,0.1*s->atk,DAMAGE_REAL,0,TYPE_FIGHTING);
	}
	if((x=s->moves->cooldown)<=0||setcooldown(s,s->moves,x-1)>=x)
		effect(s->speed>t->speed?elbow2:elbow3,s,s,0,2);
	if(t->owner==s->owner->enemy){
		if(unit_effect_level(t,PDD)-unit_effect_level(s,PDD)>2)
			effect(frost_destroying,t,s,0,2);
		if(unit_effect_level(t,MDD)-unit_effect_level(s,MDD)>2)
			do_repeat(s,t,2);
	}
	e=unit_findeffect(t,mana);
	if(e&&e->level>=unit_effect_level(s,mana)+20){
		effect_reinit(e,s,-10,-1);
		effect(mana,s,s,10,-1);
	}
}
void speed_up(struct unit *s){
	effect(SPEED,s,s,2,-1);
}
void peanut_powder(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,1.05*s->atk,DAMAGE_MAGICAL,0,TYPE_POISON);
}
void mana_gather(struct unit *s){
	effect(mana,s,s,10+randi()%21,-1);
}
void fury_swipes(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	int i;
	e=unit_findeffect(s,mana);
	if(e&&e->level>=8){
		effect_reinit(e,s,-8,-1);
		i=4+randi()%5;
	}else
		i=2+randi()%4;
	for(;i>0;--i)
		if(hittest(t,s,1.0))
			attack(t,s,0.2*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
}
void razor_carrot(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.7)){
		attack(t,s,0.45*s->atk,DAMAGE_PHYSICAL,0,TYPE_GRASS);
		effect(aurora,t,s,0,5);
	}
	setcooldown(s,s->move_cur,8);
}
int moon_elf_shield_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest||!e->level)
		return 0;
	if(*aflag&AF_IDEATH){
		effect_event(e);
		effect_event_end(e);
		return -1;
	}
	if(*type){
		if(*type&*(int *)e->data){
			if(*value>1)
				*value=1;
		}else if(*(int *)e->data==TYPE_STEEL){
			*(int *)e->data=*type;
			if(*value>1)
				*value=1;
		}
	}
	if(*value>e->level){
		effect_event(e);
		*value-=e->level;
		effect_reinit(e,src,-e->level,-1);
		effect_event_end(e);
		return 0;
	}
	effect_event(e);
	effect_reinit(e,src,-(long)*value,-1);
	effect_event_end(e);
	return -1;
}
int moon_elf_shield_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=-1;
	if(level>=0){
		e->level=level;
		*(int *)e->data=TYPE_STEEL;
	}else {
		level+=e->level;
		if(level<0)
			level=0;
		e->level=level;
	}
	return 0;
}
extern const struct effect_base moon_elf_shield[1];
void moon_elf_shield_effect_end(struct effect *e,struct effect *ep,struct unit *dest,struct unit *src,long level,int round);
void moon_elf_shield_cooldown_end(struct effect *e){
	struct effect *ep=unit_findeffect(e->dest,moon_elf_shield);
	if(ep)
		moon_elf_shield_effect_end(ep,NULL,ep->dest,NULL,0,0);
}
const struct effect_base moon_elf_shield_cooldown[1]={{
	.id="moon_elf_shield_cooldown",
	.flag=EFFECT_NEGATIVE|EFFECT_UNPURIFIABLE,
	.end=moon_elf_shield_cooldown_end
}};
void moon_elf_shield_effect_end(struct effect *e,struct effect *ep,struct unit *dest,struct unit *src,long level,int round){
	int x=0;
	if(dest!=e->dest)
		return;
	if(unit_findeffect(dest,moon_elf_shield_cooldown))
		return;
	for_each_effect(ep2,dest->owner->field->effects){
		if((ep2->dest==dest)&&ep2->base->flag&EFFECT_CONTROL){
			if(!x){
				effect_event(e);
				x=1;
			}
			purify(ep2);
		}
	}
	if(!x){
		return;
	}
	effect_reinit(e,dest,dest->base->def,-1);
	effect(moon_elf_shield_cooldown,dest,dest,0,3);
	effect_event_end(e);
}
const struct effect_base moon_elf_shield[1]={{
	.id="moon_elf_shield",
	.flag=EFFECT_PASSIVE,
	.init=moon_elf_shield_init,
	.damage=moon_elf_shield_damage,
	.effect_end=moon_elf_shield_effect_end,
}};
void moon_elf_shield_p(struct unit *s){
	effect(moon_elf_shield,s,s,0,-1);
}
int adbd_attack(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	long pdef;
	if(src==e->dest&&*damage_type!=DAMAGE_REAL&&!(*aflag&AF_NODEF)){
		pdef=src->def;
		if(pdef>2){
			pdef*=0.35;
			*value*=def_coef(dest->def-pdef+dest->level-src->level);
			*aflag|=AF_NODEF;
		}
	}
	return 0;
}
const struct effect_base anti_def_by_def[1]={{
	.id="anti_def_by_def",
	.flag=EFFECT_PASSIVE,
	.attack=adbd_attack,
}};
void anti_def_by_def_p(struct unit *s){
	effect(anti_def_by_def,s,s,0,-1);
}
void burn_boat(struct unit *s){
	struct unit *t=gettarget(s);
	double d0;
	if(hittest(t,s,1.5)){
		d0=(double)s->hp/(double)s->base->max_hp;
		attack(t,s,(3.0-2.0*d0)*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
	}
	if(!isalive(t->state))
		return;
	setcooldown(s,s->move_cur,6);
	effect(stunned,s,s,0,1);
}
void burn_boat_kill(struct effect *e,struct unit *u){
	struct effect *ep;
	if(u==e->dest&&!e->level){
		effect_event(e);
		sethp(u,u->base->max_hp);
		effect_setlevel(e,2);
		ep=unit_findeffect(u,moon_elf_shield_cooldown);
		if(ep)
			effect_end(ep);
		effect_event_end(e);
	}
}
int burn_boat_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	unsigned long dmg;
	if(dest==e->dest){
		if(e->level&&(*aflag&AF_IDEATH)){
			effect_event(e);
			effect_event_end(e);
			return -1;
		}
		switch(e->level){
			case 2:
				return -1;
			case 1:
				dmg=dest->base->max_hp/4;
				if(*value>dmg){
					effect_event(e);
					*value=dmg;
					effect_event_end(e);
				}
				break;
			default:
				break;
		}
	}
	return 0;
}
int burn_boat_effect1(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long *level,int *round){
	struct effect *ep;
	if(dest==e->dest&&e->level){
		if(base==moon_elf_shield_cooldown)
			return -1;
		if(base->flag&EFFECT_CONTROL){
			effect_event(e);
			ep=unit_findeffect(dest,moon_elf_shield);
			if(ep)
				effect_reinit(ep,dest,dest->base->def,-1);
			effect_event_end(e);
			return -1;
		}
	}
	return 0;
}
void burn_boat_roundstart(struct effect *e){
	if(e->level==2)
		effect_setlevel(e,1);
}
const struct effect_base burn_boat_effect[1]={{
	.id="burn_boat",
	.flag=EFFECT_PASSIVE,
	.damage=burn_boat_damage,
	.effect=burn_boat_effect1,
	.kill=burn_boat_kill,
	.roundstart=burn_boat_roundstart,
}};
void burn_boat_p(struct unit *s){
	effect(burn_boat_effect,s,s,0,-1);
}
void dartle(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,1.5*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
		effect(DEF,s,s,-1,-1);
	}
}
void electric_arc(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.43*s->atk,DAMAGE_PHYSICAL,AF_NOFLOAT,TYPE_ELECTRIC);
		attack(t,s,0.43*s->atk,DAMAGE_MAGICAL,AF_NOFLOAT,TYPE_ELECTRIC);
	}
}
void metal_syncretize(struct unit *s){
	struct unit *t=gettarget(s);
	attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
	effect(PDB,s,s,1,-1);
}
int uniform_base_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	unsigned long dmg,x;
	if(dest!=e->dest)
		return 0;
	dmg=*value;
	if(!(dmg&(dmg-1)))
		return 0;
	effect_event(e);
	x=0;
	do {
		dmg>>=1;
		++x;
	}while(dmg&(dmg-1));
	*value=dmg<<x;
	effect_event_end(e);
	return 0;
}
const struct effect_base uniform_base_effect[1]={{
	.id="uniform_base",
	.flag=EFFECT_PASSIVE,
	.damage=uniform_base_damage,
	.prior=-26
}};
void uniform_base(struct unit *s){
	effect(uniform_base_effect,s,s,0,-1);
}
void spatially_shatter_pm(struct unit *s){
	struct unit *t=s->osite;
	unsigned dmg=0.875*t->base->max_hp;
	attack(t,s,s->atk>dmg?s->atk:dmg,DAMAGE_REAL,0,TYPE_DRAGON);
}
const struct move spatially_shatter_p={
	.id="spatially_shatter",
	.action=spatially_shatter_pm,
	.type=TYPE_DRAGON,
	.prior=0,
	.flag=MOVE_NOCONTROL,
	.mlevel=MLEVEL_CONCEPTUAL
};
void spatially_shatter_action_end(struct effect *e,struct player *p){
	struct move am;
	if(!e->active||p!=e->dest->owner->enemy||p->action!=e->level)
		return;
	effect_event(e);
	memcpy(&am,&spatially_shatter_p,sizeof(struct move));
	unit_move(e->dest,&am);
	effect_event_end(e);
	effect_end(e);
}
void spatially_shatter_roundstart(struct effect *e){
	e->active=1;
}
const struct effect_base spatially_shatter_effect[1]={{
	.id="spatially_shatter",
	.action_end=spatially_shatter_action_end,
	.roundstart=spatially_shatter_roundstart,
	.flag=EFFECT_POSITIVE
}};
void spatially_shatter(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	int a;
	if(hittest(t,s,1.0))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_DRAGON);
	else
		attack(t,s,s->atk/2,DAMAGE_MAGICAL,0,TYPE_DRAGON);
	a=s->owner->enemy->action;
	if(a==ACT_ABORT)
		return;
	e=unit_findeffect(s,spatially_shatter_effect);
	if(!e)
		effect(spatially_shatter_effect,s,s,a,2);
}
void health_reset(struct unit *s){
	struct unit *t=s->osite;
	sethp(s,s->base->max_hp);
	sethp(t,t->base->max_hp);
	setcooldown(s,s->move_cur,21);
}
void soil_loosening(struct unit *s){
	struct unit *t=gettarget(s);
	if(!hittest(t,s,1.0))
		return;
	attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,0,TYPE_SOIL);
	if((t->type0|t->type1)&TYPE_SOIL)
		effect(DEF,t,s,-1,-1);
}
void flash(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		effect(HIT,t,s,-1,-1);
}
void harden(struct unit *s){
	if(test(0.5))
		effect(DEF,s,s,1,-1);
}
void blow_down(struct unit *s){
	struct unit *t=gettarget(s);
	attack(t,s,0.9*s->atk,DAMAGE_PHYSICAL,0,TYPE_WIND);
}
const struct move builtin_moves[]={
	{
		.id="steel_flywheel",
		.action=steel_flywheel,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="holylight_heavycannon",
		.action=holylight_heavycannon,
		.type=TYPE_LIGHT,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="ground_force",
		.action=ground_force,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="spoony_spell",
		.action=spoony_spell,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="self_explode",
		.action=self_explode,
		.type=TYPE_NORMAL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="health_exchange",
		.action=health_exchange,
		.type=TYPE_GHOST,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spi_exchange",
		.action=spi_exchange,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="urgently_repair",
		.action=urgently_repair,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="double_slash",
		.action=double_slash,
		.type=TYPE_WIND,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR|MLEVEL_CONCEPTUAL
	},
	{
		.id="petrifying_ray",
		.action=petrifying_ray,
		.type=TYPE_ROCK,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="leech_seed",
		.action=leech_seed,
		.type=TYPE_GRASS,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="soften",
		.action=soften,
		.type=TYPE_NORMAL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="iron_wall",
		.action=iron_wall,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="spi_blow",
		.action=spi_blow,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spi_shattering_slash",
		.action=spi_shattering_slash,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="angry",
		.action=angry,
		.type=TYPE_FIGHTING,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spi_fcrack",
		.action=spi_fcrack,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="natural_shield",
		.action=natural_shield,
		.type=TYPE_DEVINEGRASS,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="metal_bomb",
		.action=metal_bomb,
		.type=TYPE_ALKALIFIRE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="fate_destroying_slash",
		.action=fate_destroying_slash,
		.type=TYPE_FIGHTING,
		.prior=1,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="mosquito_bump",
		.action=mosquito_bump,
		.type=TYPE_GHOST,
		.prior=1,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="primordial_breath",
		.init=primordial_breath_init,
		.type=TYPE_ICE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="damage_recur",
		.action=damage_recur,
		.type=TYPE_GHOST,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="scorching_roaring",
		.action=scorching_roaring,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="thunder_roaring",
		.action=thunder_roaring,
		.type=TYPE_ELECTRIC,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="freezing_roaring",
		.action=freezing_roaring,
		.type=TYPE_ICE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR|MLEVEL_FREEZING_ROARING
	},
	{
		.id="triple_cutter",
		.action=triple_cutter,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="thorns",
		.init=thorns_init,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="combo",
		.init=combo_init,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="hitback",
		.init=hitback_init,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="thermobaric",
		.action=thermobaric,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="kaleido",
		.init=kaleido_init,
		.type=TYPE_DEVINEGRASS,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="ablaze",
		.action=ablaze,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spray",
		.action=spray,
		.type=TYPE_WATER,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="cold_wave",
		.action=cold_wave,
		.type=TYPE_ICE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="collapse",
		.action=collapse,
		.type=TYPE_ROCK,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="cycle_erode",
		.action=cycle_erode,
		.init=cycle_erode_init,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="soul_back",
		.action=soul_back,
		.type=TYPE_NORMAL,
		.prior=5,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="absolutely_immortal",
		.action=absolutely_immortal,
		.type=TYPE_NORMAL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="overload",
		.action=overload,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="escape",
		.action=escape,
		.type=TYPE_NORMAL,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="super_scissors",
		.action=super_scissors,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="sonic",
		.action=sonic,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="maya_mirror",
		.action=maya_mirror,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="cog_hit",
		.action=cog_hit,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="defend",
		.action=defend,
		.type=TYPE_NORMAL,
		.prior=5,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="heat_engine",
		.init=heat_engine_init,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="flamethrower",
		.action=flamethrower,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="time_back_locally",
		.action=time_back_locally,
		.type=TYPE_DRAGON,
		.prior=5,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="star_move",
		.action=star_move,
		.type=TYPE_STEEL,
		.prior=-6,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="rebound",
		.action=rebound,
		.type=TYPE_NORMAL,
		.flag=0,
		.prior=5,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="force_vh",
		.action=force_vh,
		.type=TYPE_DRAGON,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="time_back",
		.action=time_back,
		.type=TYPE_DRAGON,
		.prior=5,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="repeat",
		.action=repeat,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="head_blow",
		.action=head_blow,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="mecha",
		.action=mecha,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="perish_song",
		.action=perish_song,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="mud_shot",
		.action=mud_shot,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="nether_roaring",
		.action=nether_roaring,
		.type=TYPE_GHOST,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="tidal",
		.action=tidal,
		.type=TYPE_DEVINEWATER,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="interference",
		.action=interference,
		.type=TYPE_BUG,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="anger_roaring",
		.action=anger_roaring,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="poison_sting",
		.action=poison_sting,
		.type=TYPE_POISON,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="hypnosis",
		.action=hypnosis,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="moonlight",
		.action=moonlight,
		.type=TYPE_LIGHT,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="byebye",
		.action=byebye,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="rest",
		.action=rest,
		.type=TYPE_NORMAL,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="scent",
		.action=scent,
		.type=TYPE_GRASS,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="synthesis",
		.action=synthesis,
		.type=TYPE_GRASS,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="hail",
		.action=hail,
		.type=TYPE_ICE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="clip3",
		.action=clip3,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="mighty_wave",
		.action=mighty_wave,
		.type=TYPE_WATER,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="cold_wind",
		.action=cold_wind,
		.type=TYPE_ICE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="breach_missile",
		.action=breach_missile,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="piercing_missile",
		.action=piercing_missile,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="amuck",
		.action=amuck,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="reflex",
		.action=reflex,
		.type=TYPE_STEEL,
		.prior=-6,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="physical_reflex",
		.action=physical_reflex,
		.type=TYPE_STEEL,
		.prior=-6,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="magical_reflex",
		.action=magical_reflex,
		.type=TYPE_STEEL,
		.prior=-6,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="damage_reverse",
		.action=damage_reverse,
		.type=TYPE_GHOST,
		.prior=4,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="elbow",
		.action=elbow,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="speed_up",
		.action=speed_up,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="peanut_powder",
		.action=peanut_powder,
		.type=TYPE_POISON,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="mana_gather",
		.action=mana_gather,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="fury_swipes",
		.action=fury_swipes,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="razor_carrot",
		.action=razor_carrot,
		.type=TYPE_GRASS,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="moon_elf_shield",
		.init=moon_elf_shield_p,
		.type=TYPE_STEEL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="anti_def_by_def",
		.init=anti_def_by_def_p,
		.type=TYPE_NORMAL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="burn_boat",
		.action=burn_boat,
		.init=burn_boat_p,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="dartle",
		.action=dartle,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="electric_arc",
		.action=electric_arc,
		.type=TYPE_ELECTRIC,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="metal_syncretize",
		.action=metal_syncretize,
		.type=TYPE_ALKALIFIRE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="entangle",
		.action=entangle,
		.type=TYPE_GRASS,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="uniform_base",
		.init=uniform_base,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spatially_shatter",
		.action=spatially_shatter,
		.type=TYPE_DRAGON,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="health_reset",
		.action=health_reset,
		.type=TYPE_GHOST,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="soil_loosening",
		.action=soil_loosening,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="flash",
		.action=flash,
		.type=TYPE_LIGHT,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="harden",
		.action=harden,
		.type=TYPE_BUG,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="blow_down",
		.action=blow_down,
		.type=TYPE_WIND,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{.id=NULL}
};
const size_t builtin_moves_size=sizeof(builtin_moves)/sizeof(builtin_moves[0])-1;
const struct move *get_builtin_move_by_id(const char *id){
	unsigned long i;
	for(i=0;builtin_moves[i].id;++i){
//	fprintf(stderr,"strcmp starting\n");
//	fprintf(stderr,"strcmpp(%p)\n",id);
//	fprintf(stderr,"strcmps(%s)\n",id);
		if(!strcmp(id,builtin_moves[i].id))
			return builtin_moves+i;
//	fprintf(stderr,"strcmp ending\n");
	}
	return NULL;
}
