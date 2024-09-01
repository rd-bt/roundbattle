#include "battle-core.h"
#include <stddef.h>
#include <limits.h>
#include <math.h>
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
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,\
	.init=name##_init,\
	.roundend=name##_roundend\
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
	.flag=EFFECT_ABNORMAL|EFFECT_CONTROL|EFFECT_NEGATIVE,\
	.init=name##_init,\
	.update_state=name##_update_state\
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
	.flag=EFFECT_ABNORMAL|EFFECT_NEGATIVE,
	.init=parasitized_init,
	.roundend=parasitized_roundend
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
	update_attr(e->dest);
	return 0;
}
#define effect_attr(name,attr)\
void name##_update_attr(struct effect *e,struct unit *u){\
	if(e->dest==u){\
		if(e->level<0)\
			u->attr*=pow(0.75,-e->level);\
		else\
			u->attr*=1.0+0.5*e->level;\
	}\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.flag=EFFECT_ATTR,\
	.init=attr_init,\
	.inited=effect_update_attr,\
	.end=effect_update_attr,\
	.update_attr=name##_update_attr\
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
effect_attr_d(CE,crit_effect,0.8)
effect_attr_d(PDB,physical_bonus,0.75)
effect_attr_d(MDB,magical_bonus,0.75)
effect_attr_d(PDD,physical_derate,0.75)
effect_attr_d(MDD,magical_derate,0.75)
void steel_flywheel(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.2)){
		attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
		effect(cursed,t,s,0,3);
	}
	setcooldown(s->move_cur,3);
}
void holylight_heavycannon(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.2)){
		attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
		effect(radiated,t,s,0,3);
	}
	setcooldown(s->move_cur,3);
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
	setcooldown(s->move_cur,4);
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
	setcooldown(s->move_cur,4);
}
void leech_seed(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		//unit_abnormal(t,ABNORMAL_PARASITIZED,5);
		effect(parasitized,t,s,0,5);
	}
	setcooldown(s->move_cur,4);
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
	setspi(t,t->spi+0.02*t->atk);
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
	unsigned long x=1,y=1,v;
	long ds;
	if(hittest(t,s,1.0)){
		v=attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		if(v>20)
			x=v;
	}
	if(hittest(t,s,1.0)){
		v=attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		if(v>20)
			y=v;
	}
	ds=0.015*t->atk;
	if(gcdul(x,y)==1)
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
	.damage=natural_shield_damage
}};
void natural_shield(struct unit *s){
	effect(natural_shield_effect,s,s,0,5);
	setcooldown(s->move_cur,13);
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
	.flag=EFFECT_NEGATIVE|EFFECT_UNPURIFIABLE,
	.heal=alkali_fire_seal_heal
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
	.action=rfdisillusionfr_action
}};
void metal_bomb_end(struct effect *e){
	struct player *p=e->src->owner->enemy;
	effect_event(e);
	attack(p->front,e->src,1.35*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
	if(unit_findeffect(p->front,alkali_fire_seal))
		event(rfdisillusionfr,e->src);
	else
		effect(alkali_fire_seal,p->front,e->src,0,16);
	effect_event_end(e);
}
const struct effect_base metal_bomb_effect[1]={{
	.id="metal_bomb",
	.end=metal_bomb_end
}};
void metal_bomb(struct unit *s){
	effect(metal_bomb_effect,NULL,s,0,0);
}
void fate_destroying_slash(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
	}
	attack(t,s,(t->base->max_hp-t->hp)*(0.12+0.00012*(s->atk+(s->def>0?s->def:0))),DAMAGE_REAL,0,TYPE_FIGHTING);
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
	{NULL}
};
