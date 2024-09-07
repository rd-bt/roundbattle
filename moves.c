#include "battle-core.h"
#include <stddef.h>
#include <stdlib.h>
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

#define abnormal_damage(name,frac,type)\
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
	.init=name##_init,\
	.roundend=name##_roundend,\
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,\
	.prior=-64\
}};
#define abnormal_control(name)\
void name##_update_state(struct effect *e,struct unit *u,int *state){\
	if(e->dest==u&&*state==UNIT_NORMAL)\
		*state=UNIT_CONTROLLED;\
}\
int name##_init(struct effect *e,long level,int round){\
	if(!e->round)\
		e->round=round;\
	return 0;\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=name##_init,\
	.update_state=name##_update_state,\
	.flag=EFFECT_ABNORMAL|EFFECT_CONTROL|EFFECT_NEGATIVE,\
}};
abnormal_damage(cursed,5,TYPE_STEEL)
abnormal_damage(radiated,5,TYPE_LIGHT)
abnormal_damage(poisoned,10,TYPE_POISON)
abnormal_damage(burnt,10,TYPE_FIRE)

void parasitized_roundend(struct effect *e){
	effect_event(e);
	heal(e->dest->owner->enemy->front,
		attack(e->dest,NULL,e->dest->base->max_hp/12,DAMAGE_REAL,0,TYPE_GRASS)
	);
	effect_event_end(e);
}
int parasitized_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=round;
	return 0;
}
const struct effect_base parasitized[1]={{
	.id="parasitized",
	.init=parasitized_init,
	.roundend=parasitized_roundend,
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,
	.prior=-64
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
	update_attr(e->dest);
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
effect_attr(DEF,atk)
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
	struct unit *t=s->owner->enemy->front;
	unsigned long dmg=2.1*s->atk+0.3*s->base->max_hp;
	attack(t,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
	attack(s,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
}
void self_explode(struct unit *s){
	struct unit *t=s->owner->enemy->front;
	unsigned long dmg=1.8*s->atk+0.25*s->base->max_hp;
	attack(t,s,dmg,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	attack(t,s,dmg,DAMAGE_MAGICAL,0,TYPE_NORMAL);
	sethp(s,0);
}
void health_exchange(struct unit *s){
	struct unit *t=s->owner->enemy->front;
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
	if(hittest(t,s,1.0))
		attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,t->hp==t->base->max_hp?AF_CRIT:0,TYPE_WIND);
	if(s->move_cur&&(s->move_cur->mlevel&MLEVEL_CONCEPTUAL))
		addhp(s->owner->enemy->front,s->def>16?-s->def:-16);
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
		instant_death(s->owner->enemy->front);
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
	if(*value<8)
		*value=1;
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
	effect(metal_bomb_effect,s->owner->enemy->front,s,0,0);
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
	if(e->dest==u){
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
	addhp(dest->owner->enemy->front,-(long)value);
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
			attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
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
	t=s->owner->enemy->front;
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

			attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_ELECTRIC);
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
	t=s->owner->enemy->front;
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

			attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
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
	t=s->owner->enemy->front;
	n1=(long)t->hp;
	t->hp=0;
	report(s->owner->field,MSG_HPMOD,t,-n1);
	unit_wipeeffect(t,0);
	t->state=UNIT_FREEZING_ROARINGED;
	if(t==t->owner->front)
		t->owner->action=ACT_ABORT;
	report(s->owner->field,MSG_FAIL,t);
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
		normal_attack(dest,src);
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
		normal_attack(src,dest);
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
		.mlevel=MLEVEL_REGULAR|MLEVEL_CONCEPTUAL
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
	{NULL}
};
const struct move *get_builtin_move_by_id(const char *id){
	unsigned long i;
	for(i=0;builtin_moves[i].id;++i){
		if(!strcmp(id,builtin_moves[i].id))
			return builtin_moves+i;
	}
	return NULL;
}
