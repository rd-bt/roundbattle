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
#define abnormal_control(name)\
void name##_update_state(struct effect *e,struct unit *u,int *state){\
	if(e->dest==u&&*state==UNIT_NORMAL)\
		*state=UNIT_CONTROLLED;\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=abnormal_init,\
	.update_state=name##_update_state,\
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
		attack(e->dest,NULL,e->dest->base->max_hp/10,DAMAGE_REAL,0,TYPE_GRASS)
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
abnormal_control(frozen)
abnormal_control(asleep)
abnormal_control(paralysed)
abnormal_control(stunned)
abnormal_control(petrified)
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
	.end=effect_update_attr,\
	.update_attr=name##_update_attr,\
	.flag=EFFECT_ATTR,\
	.prior=64\
}};
effect_attr(ATK,atk)
//effect_attr(DEF,def)
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
	.flag=EFFECT_ATTR,\
	.init=attr_init,\
	.inited=effect_update_attr,\
	.end=effect_update_attr,\
	.update_attr=name##_update_attr\
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
	unsigned long a=s->hp,b=t->hp;
	sethp(t,a);
	sethp(s,b);
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
		//unit_abnormal(t,ABNORMAL_PETRIFIED,3);
		effect(petrified,t,s,0,3);
	}
	setcooldown(s,s->move_cur,4);
}
void leech_seed(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		//unit_abnormal(t,ABNORMAL_PARASITIZED,5);
		effect(parasitized,t,s,0,5);
	}
	setcooldown(s,s->move_cur,4);
}
void soften(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		effect(ATK,t,s,-1,-1);
		//unit_attr_set(t,ATTR_ATK,t->attrs.atk-1);
}
void iron_wall(struct unit *s){
	effect(PDD,s,s,1,-1);
	effect(MDD,s,s,1,-1);
	//unit_attr_set(s,ATTR_PDERATE,s->attrs.physical_derate+1);
	//unit_attr_set(s,ATTR_MDERATE,s->attrs.magical_derate+1);
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
		//unit_abnormal(t,ABNORMAL_PARALYSED,3);
		effect(paralysed,t,s,0,3);
	}
}
void angry(struct unit *s){
	effect(ATK,s,s,1,-1);
	//unit_attr_set(s,ATTR_CRITEFFECT,s->attrs.crit_effect+2);
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
		attack(u,src,def*u->base->max_hp/2048,DAMAGE_REAL,0,TYPE_ALKALIFIRE);
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
	attack(e->dest,e->src,1.35*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
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
	effect_end(e);
	return 0;
}
const struct effect_base avoid[1]={{
	.id="avoid",
	.hittest=avoid_hittest,
	.flag=EFFECT_ISOLATED,
	.prior=0
}};
void mosquito_bump(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.1))
		attack(t,s,0.9*s->atk,DAMAGE_PHYSICAL,0,TYPE_GHOST);
	effect(avoid,s,s,0,5);
	setcooldown(s,s->move_cur,2);
}
int frost_destroying_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=round;
	return 0;
}
void frost_destroying_cd(struct effect *e,struct unit *u,struct move *m,int *round){
	if(e->dest!=u||round<=0||
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
		++m->cooldown;
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

void primordial_breath_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
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
	.damage_end=primordial_breath_damage_end,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP,
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
	if(!n){
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
	attack(t,s,(0.55+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_FIRE);
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
	if(!n){
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
	dmg=attack(t,s,(0.55+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_ELECTRIC);
	for_each_unit(u,t->owner){
		attack(t,s,0.15*dmg+n*0.06*u->base->max_hp,DAMAGE_REAL,0,TYPE_ELECTRIC);
	}
	heal(s,0.2*dmg);
}

void freezing_roaring(struct unit *s){
	struct unit *t;
	long n,n1;
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
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP,
}};
void thorns_init(struct unit *s){
	effect(thorns,s,s,0,-1);
}

void combo_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
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
	.damage_end=combo_damage_end,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP
}};
void combo_init(struct unit *s){
	effect(combo,s,s,20,-1);
}

void hitback_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
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
	.damage_end=hitback_damage_end,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP
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

void myriad_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(src!=e->dest||dest->owner!=src->owner->enemy||damage_type!=DAMAGE_PHYSICAL||value<3)
		return;
	effect_event(e);
	attack(dest,src,0.35*value,DAMAGE_MAGICAL,0,TYPE_DEVINEGRASS);	
	effect_event_end(e);
}
void myriad_roundend(struct effect *e){
	long v=(long)e->dest->base->max_hp-(long)e->dest->hp;
	if(!v)
		return;
	if(v<25)
		v=25;
	effect_event(e);
	heal(e->dest,v*4/25);
	effect_event_end(e);
}
const struct effect_base myriad[1]={{
	.id="myriad",
	.damage_end=myriad_damage_end,
	.roundend=myriad_roundend,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP
}};
void myriad_init(struct unit *s){
	effect(myriad,s,s,0,-1);
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
	if(!e->level)
		e->round=round;
	e->level=level;
	return 0;
}
void gp_damage_end(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type){
	if(dest!=e->dest||(damage_type!=DAMAGE_PHYSICAL&&damage_type!=DAMAGE_MAGICAL))
		return;
	if(e->level>3){
		e->level-=damage_type==DAMAGE_PHYSICAL?3:2;
		report(e->dest->owner->field,MSG_UPDATE,e);
	}else
		effect_end(e);
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
int cyce_damage(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->src&&(*aflag&AF_IDEATH)){
		effect_event(e);
		effect_event_end(e);
		return 1;
	}else
		return 0;
}
int cyce_hittest(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate){
	if(dest==e->src){
		effect_event(e);
		effect_event_end(e);
		return 0;
	}
	else if(src==e->src){
		effect_event(e);
		effect_event_end(e);
		return 1;
	}
	else
		return -1;
}
const struct effect_base cycle_eden[1]={{
	.id="cycle_eden",
	.damage=cyce_damage,
	.hittest=cyce_hittest,
	.flag=EFFECT_ENV
}};
void cycle_erode(struct unit *s){
	struct unit *t;
	int plv=test(0.5)?2:1;
	t=gettarget(s);
	if(effect(PDD,t,s,-plv,-1))
		effect(PDD,s,s,plv,-1);
	attack(t,s,1.05*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	effect(cycle_eden,NULL,s,0,4);
	setcooldown(s,s->move_cur,16);
}
void cycle_erode_init(struct unit *s){
	struct unit *t;
	if(!isfront(s))
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
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE,
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
void super_scissors(struct unit *s){
	struct unit *t=gettarget(s);
	struct move *mp;
	int r;
	if(hittest(t,s,1.0))
		attack(t,s,0.45*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	switch((r=t->owner->action)){
		case ACT_MOVE0 ... ACT_MOVE7:
			mp=t->moves+r;
			if(mp->action==s->move_cur->action||!(mp->mlevel&MLEVEL_REGULAR))
				break;
			memcpy(s->move_cur,mp,sizeof(struct move));
			s->move_cur->mlevel&=MLEVEL_REGULAR;
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
		++ep->round;
		report(u->owner->field,MSG_UPDATE,ep);
		ep->base->roundend(ep);
		effect_event_end(e);
	}
}
const struct effect_base heat_engine[1]={{
	.id="heat_engine",
	.move_end=heat_engine_move_end,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP
}};
void heat_engine_init(struct unit *s){
	effect(heat_engine,s,s,0,-1);
}

void flamethrower(struct unit *s){
	struct unit *t;
	t=gettarget(s);
	if(hittest(t,s,1.0))
		effect(burnt,t,s,0,5);
}
void time_back(struct unit *s){
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
	if(!revive(t,t->base->max_hp/2)){
		setcooldown(s,s->move_cur,31);
		switchunit(t);
		return;
	}
fail:
	setcooldown(s,s->move_cur,4);
}

void star_move(struct unit *s){
	struct battle_field *f=s->owner->field;
	unsigned long dmg=0;
	int round=*f->round;
	for(const struct message *p=f->rec+f->rec_size-1;p>=f->rec;--p){
		if(p->type!=MSG_DAMAGE
		||damage_type_check(p->un.damage.damage_type)
		||p->un.damage.dest!=s)
			continue;
		if(p->round!=round)
			break;
		dmg+=p->un.damage.value;
	}
	if(dmg){
		attack(s->osite,s,dmg,DAMAGE_REAL,0,TYPE_VOID);
		heal(s,dmg);
		setcooldown(s,s->move_cur,2);
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
	.id="rebound",
	.gettarget=rebound_gettarget,
	.flag=EFFECT_POSITIVE
}};
void rebound(struct unit *s){
	effect(rebound_effect,s,s,0,1);
	setcooldown(s,s->move_cur,3);
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
		.type=TYPE_NORMAL,
		.prior=1,
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
		.type=TYPE_ICE,
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
		.id="myriad",
		.init=myriad_init,
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
		.id="time_back",
		.action=time_back,
		.type=TYPE_LIGHT,
		.prior=5,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="star_move",
		.action=star_move,
		.type=TYPE_STEEL,
		.prior=-6,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="rebound",
		.action=rebound,
		.type=TYPE_NORMAL,
		.prior=5,
		.mlevel=MLEVEL_REGULAR
	},
	{NULL}
};
const size_t builtin_moves_size=sizeof(builtin_moves)/sizeof(builtin_moves[0])-1;
const struct move *get_builtin_move_by_id(const char *id){
	unsigned long i;
	for(i=0;builtin_moves[i].id;++i){
		if(!strcmp(id,builtin_moves[i].id))
			return builtin_moves+i;
	}
	return NULL;
}
