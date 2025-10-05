#include "battle-core.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "moves.h"
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
int isprime(unsigned long n){
	if(n==2)return 1;
	if(!(n&1))return 0;
	unsigned long end=(unsigned long)(sqrt(n)+1.0);
	for(unsigned long i=3;i<end;i+=2)
		if(!(n%i))return 0;
	return 1;
}
long instant_death(struct unit *dest){
	return damage(dest,NULL,dest->max_hp*64,DAMAGE_REAL,AF_IDEATH|AF_INHIBIT,TYPE_VOID);
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
	effect_ev(e)\
		attack(e->dest,NULL,e->dest->max_hp/frac,DAMAGE_REAL,0,type);\
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
}}
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
}}
#define COMMA ,
int poisoned_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&*damage_type==DAMAGE_MAGICAL)
		*value*=0.85;
	return 0;
}
int burnt_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&*damage_type==DAMAGE_PHYSICAL)
		*value*=0.85;
	return 0;
}
abnormal_damage(cursed,5,TYPE_STEEL,);
abnormal_damage(radiated,5,TYPE_LIGHT,);
abnormal_damage(poisoned,10,TYPE_POISON,.attack=poisoned_attack COMMA);
abnormal_damage(burnt,10,TYPE_FIRE,.attack=burnt_attack COMMA);

void parasitized_roundend(struct effect *e){
	effect_ev(e)
		heal(e->dest->osite,attack(e->dest,NULL,e->dest->max_hp/10,DAMAGE_REAL,0,TYPE_GRASS)/2);
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
#define damage_pm(_dt) ((1<<(_dt))&(DAMAGE_PHYSICAL_FLAG|DAMAGE_MAGICAL_FLAG))
#define damage_rpm(_dt) ((1<<(_dt))&(DAMAGE_REAL_FLAG|DAMAGE_PHYSICAL_FLAG|DAMAGE_MAGICAL_FLAG))
int aurora_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&damage_rpm(*damage_type)){
		if(*aflag&DF_TEST)
			return -1;
		effect_ev(e)
			effect_reinit(e,src,*value,e->round);
		return -1;
	}
	return 0;
}
void aurora_end(struct effect *e){
	if(e->level){
		attack(e->dest,NULL,e->level,DAMAGE_REAL,0,TYPE_VOID);
	}
}
int asleep_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&damage_pm(*damage_type)){
		*value*=1.5;
		if(!(*aflag&DF_TEST))
			effect_end(e);
	}
	return 0;
}
int frozen_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&damage_pm(*damage_type)&&(*type&TYPE_FIRE)){
		*value*=2;
		if(!(*aflag&DF_TEST))
			effect_end(e);
	}
	return 0;
}
int petrified_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&damage_pm(*damage_type)&&(*type&TYPE_FIGHTING)){
		*value*=2;
		if(!(*aflag&DF_TEST))
			effect_end(e);
	}
	return 0;
}
abnormal_control(frozen,.attack=frozen_attack COMMA);
abnormal_control(asleep,.attack=asleep_attack COMMA);
abnormal_control(paralysed,);
abnormal_control(stunned,);
abnormal_control(petrified,.attack=petrified_attack COMMA);
abnormal_control(aurora,.damage=aurora_damage COMMA .end=aurora_end COMMA);
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
	if(!level)
		return 1;
	else
		e->level=level;
	return 0;
}
#define effect_attr(name,attr)\
void name##_update_attr(struct effect *e,struct unit *u){\
	if(e->dest==u){\
		if(e->level<0)\
			u->attr*=pow(M_SQRT1_2,-e->level);\
		else\
			u->attr*=1.0+0.25*e->level;\
	}\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=attr_init,\
	.inited=effect_update_attr,\
	.update_attr=name##_update_attr,\
	.flag=EFFECT_ATTR|EFFECT_KEEP,\
	.prior=64\
}}
effect_attr(ATK,atk);
effect_attr(SPEED,speed);
effect_attr(HIT,hit);
effect_attr(AVOID,avoid);
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
}}
effect_attr_d(DEF,def,0.25*u->def);
effect_attr_d(CE,crit_effect,0.5);
effect_attr_d(PDB,physical_bonus,0.25);
effect_attr_d(MDB,magical_bonus,0.25);
effect_attr_d(PDD,physical_derate,0.25);
effect_attr_d(MDD,magical_derate,0.25);
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
	long dmg=2.1*s->atk+0.3*s->max_hp;
	dmg=attack(t,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
	attack(s,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
}
void self_explode(struct unit *s){
	struct unit *t=s->osite;
	long dmg=2.5*s->atk+0.4*s->max_hp;
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
	setcooldown(s,s->move_cur,5);
}
void urgently_repair(struct unit *s){
	heal(s,s->max_hp/2);
	setcooldown(s,s->move_cur,4);
}
void double_slash(struct unit *s){
	struct unit *t=gettarget(s);
	long vd;
	if(hittest(t,s,1.0))
		attack(t,s,0.5*s->atk,DAMAGE_PHYSICAL,t->hp==t->max_hp?AF_CRIT:0,TYPE_WIND);
	if(s->move_cur->mlevel&MLEVEL_CONCEPTUAL){
		vd=40+1.1*s->def;
		if(vd<60)
			vd=60;
		damage(s->osite,s,vd,DAMAGE_VOID,0,TYPE_VOID);
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
		attack(t,s,0.9*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	addspi(t,0.02*s->atk);
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
	long x=0,y=0;
	long ds;
	if(hittest(t,s,1.0)){
		x=attack(t,s,0.45*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	if(hittest(t,s,1.0)){
		y=attack(t,s,0.45*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	ds=0.015*s->def;
	if(x<20||y<20||gcdul(x,y)==1)
		ds*=2;
	addspi(t,-ds);
}
int natural_shield_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest||!e->round)
		return 0;
	effect_ev(e){
		if(*value<3)
			*value=0;
		else
			*value=log(*value);
	}
	return 0;
}
int natural_shield_kill(struct effect *e,struct unit *u){
	if(u==e->dest&&!e->unused){
		effect_ev(e){
			addhp(u,0.05*u->max_hp);
			if(e->round<3)
				effect_setround(e,3);
			e->unused=50;
		}
	}
	return 0;
}
void kaleido_move_end(struct effect *e,struct unit *u,struct move *m){
	struct player *p;
	if(u!=e->dest||(p=u->owner)->move_recursion||isalive(p->enemy->front->state))
		return;
	effect_ev(e)
		heal(u,(u->max_hp-u->hp)/2);
}
void kaleido_attack_end0(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	long dmg;
	if(src!=e->dest||dest->owner!=src->owner->enemy||damage_type!=DAMAGE_PHYSICAL)
		return;
	dmg=0.35*value*derate_coef(dest->physical_derate-src->physical_bonus);
	if(dmg<=0)
		return;
	effect_ev(e)
		attack(dest,src,dmg,DAMAGE_MAGICAL,0,TYPE_DEVINEGRASS);	
}
void kaleido_roundend(struct effect *e){
	long v=(long)e->dest->max_hp-(long)e->dest->hp;
	if(isfront(e->dest)&&e->unused>0)
		--e->unused;
	if(!v)
		return;
	if(v<25)
		v=25;
	effect_ev(e)
		heal(e->dest,v*4/25);
}
int kaleido_end_in_roundend(struct effect *e){
	return -1;
}
const struct effect_base natural_shield_effect[1]={{
	.id="natural_shield",
	.attack_end0=kaleido_attack_end0,
	.roundend=kaleido_roundend,
	.end_in_roundend=kaleido_end_in_roundend,
	.move_end=kaleido_move_end,
	.damage=natural_shield_damage,
	.kill=natural_shield_kill,
	.flag=EFFECT_PASSIVE,
	.prior=-128,
}};
void kaleido_init(struct unit *s){
	effect(natural_shield_effect,s,s,0,0);
}
void natural_shield(struct unit *s){
	effect(natural_shield_effect,s,s,0,*s->owner->field->stage==STAGE_PRIOR?5:6);
	setcooldown(s,s->move_cur,13);
}
void effect_destruct(struct effect *e){
	effect_end(e);
}
int alkali_fire_seal_heal(struct effect *e,struct unit *dest,long *value){
	if(dest!=e->dest)
		return 0;
	*value*=0.85;
	return 0;
}
const struct effect_base alkali_fire_seal[1]={{
	.id="alkali_fire_seal",
	.heal=alkali_fire_seal_heal,
	.flag=EFFECT_NEGATIVE|EFFECT_UNPURIFIABLE,
	.prior=-5
}};
void real_fire_disillusion_fr_action(const struct event *ev,struct unit *src){
	long def=src->def>16?src->def:16;
	struct player *p=src->owner->enemy;
	for_each_unit(u,p){
		if(u!=p->front&&!unit_findeffect(u,alkali_fire_seal))
			continue;
		attack(u,src,def*u->max_hp/4096,DAMAGE_REAL,0,TYPE_ALKALIFIRE);
	}
}
const struct event real_fire_disillusion_fr[1]={{
	.id="real_fire_disillusion_fr",
	.action=real_fire_disillusion_fr_action,
}};
extern const struct effect_base wet[1];
void metal_bomb_inited(struct effect *e){
	if(effectx(wet,e->dest,NULL,0,8,EFFECT_FIND|EFFECT_SELECTALL))
		effect_end(e);
}
void metal_bomb_end(struct effect *e){
	effect_ev(e){
		attack(e->dest,e->src,1.15*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
		if(unit_findeffect(e->dest,alkali_fire_seal))
			event(real_fire_disillusion_fr,e->src);
		else
			effect(alkali_fire_seal,e->dest,e->src,0,16);
	}
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
void pursue(struct unit *s){
	struct unit *t=gettarget(s);
	double coef=(0.12+0.00012*(s->atk+(s->def>0?s->def:0)));
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
	}
	attack(t,s,(t->max_hp-t->hp)*coef,DAMAGE_REAL,0,TYPE_FIGHTING);
	if(!isalive(t->state))
		heal(s,s->max_hp*coef);
}
int avoid_hittest(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate){
	if(e->dest!=dest)
		return -1;
	return 0;
}
const struct effect_base avoid[1]={{
	.hittest=avoid_hittest,
	.prior=0,
	.flag=EFFECT_POSITIVE,
}};
void mosquito_bump(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.1))
		attack(t,s,0.9*s->atk,DAMAGE_PHYSICAL,0,TYPE_GHOST);
	effect(avoid,s,s,0,1);
	setcooldown(s,s->move_cur,2);
}
int frost_destroying_init(struct effect *e,long level,int round){
	if(e->round)
		return -1;
	e->round=round;
	return 0;
}
void frost_destroying_cd(struct effect *e,struct unit *u,struct move *m,int *round){
	if(e->dest!=u||*round<=0||
		*e->dest->owner->field->stage!=STAGE_ROUNDEND)
		return;
	if(m->cooldown>0&&test(0.5)){
		*round=0;
	}
}
int frost_destroying_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(e->dest==dest&&*damage_type==DAMAGE_PHYSICAL){
		*value*=1.125;
	}
	return 0;
}
void frost_destroying_move_end(struct effect *e,struct unit *u,struct move *m){
	if(e->dest==u&&!m->cooldown){
		effect_ev(e)
			setcooldown(u,m,0);
	}
}
void fd_scd(struct effect *e,struct unit *u,struct move *m,int *round){
	if(e->dest==u&&m->cooldown>=0){
		++(*round);
	}
}
const struct effect_base frost_destroying[1]={{
	.id="frost_destroying",
	.init=frost_destroying_init,
	.damage=frost_destroying_damage,
	.cooldown_decrease=frost_destroying_cd,
	.setcooldown=fd_scd,
	.move_end=frost_destroying_move_end,
	.flag=EFFECT_NEGATIVE|EFFECT_KEEP,
	.prior=32
}};

void primordial_breath_roundstart(struct effect *e){
	e->active=0;
}
void primordial_breath_attack_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if(e->dest==src&&
			type==TYPE_ICE&&
			damage_type==DAMAGE_PHYSICAL&&
			dest->owner==e->dest->owner->enemy&&
			!(aflag&AF_CRIT)&&
			value*2<=dest->max_hp&&
			!e->active){
		effect_ev(e){
			e->active=1;
			if(unit_findeffect(dest,frost_destroying)){
				effect(PDD,dest,src,-1,-1);
			}else {
				effect(frost_destroying,dest,src,0,3);
			}
		}
	}
}
const struct effect_base primordial_breath[1]={{
	.id="primordial_breath",
	.attack_end=primordial_breath_attack_end,
	.roundstart=primordial_breath_roundstart,
	.flag=EFFECT_PASSIVE,
	.prior=-32
}};
void primordial_breath_action(struct unit *s){
	attack(s->osite,s,0,DAMAGE_PHYSICAL,ADF_NONHOOKABLE|AF_INHIBIT|AF_FLOAT|AF_EFFECT|AF_WEAK|AF_NODERATE|AF_NODEF,TYPE_ICE);
}
void primordial_breath_init(struct unit *s){
	effect(primordial_breath,s,s,0,-1);
}
void damage_recuring_damage_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if(!src||dest==src)
		return;
	effect_ev(e)
		addhp(src,-(long)value);
}
const struct effect_base damage_recuring[1]={{
	.id="damage_recuring",
	.damage_end=damage_recuring_damage_end,
}};
void damage_recur(struct unit *s){
	effect(damage_recuring,NULL,s,0,4);
	setcooldown(s,s->move_cur,11);
}
#define roaring_common_a(_T) \
		t=gettarget(s);\
		if(hittest(t,s,2.5)){\
			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,_T);\
			if(test(0.2))

#define roaring_common_b \
		}\
		e=unit_findeffect(t,ATK);\
		n=e?e->level:0;\
		e=unit_findeffect(s,ATK);\
		n1=e?e->level:0;\
		if(n>n1+1)\
			n=(n-n1)/2+1;\
		else\
			n=1;\
		effect(ATK,s,s,n,-1);\
		return
void scorching_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	n=unit_hasnegative(s);
	if(n<=0){
		roaring_common_a(TYPE_FIRE)
			effect(burnt,t,s,0,5);
		roaring_common_b;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_FIRE);
	heal(s,0.18*n*s->max_hp);
	effect(ATK,s,s,1+n,-1);
}
void thunder_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	long dmg;
	n=unit_hasnegative(s);
	if(n<=0){
		roaring_common_a(TYPE_ELECTRIC)
			effect(paralysed,t,s,0,3);
		roaring_common_b;
	}
	t=s->osite;
	dmg=attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_ELECTRIC);
	for_each_unit(u,t->owner){
		attack(t,s,0.15*dmg+n*0.06*u->max_hp,DAMAGE_REAL,0,TYPE_ELECTRIC);
	}
}
static struct unit *get_unit_by_arg(struct player *p,int isenemy){
	int a=p->arg;
	struct unit *t;
	switch(a){
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
		case 11:
			t=(isenemy?p->enemy:p)->units+(a-1)/2;
			if(t->base)
				return t;
		default:
			return NULL;
	}
}
void freezing_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	unsigned long hp;
	struct move *mp;
	struct player **p1;
	if(!((mp=s->move_cur)->mlevel&MLEVEL_FREEZING_ROARING)||!unit_hasnegative(s)){
		roaring_common_a(TYPE_ICE)
			effect(SPEED,t,s,-1,-1);
		roaring_common_b;
	}
	t=get_unit_by_arg(s->owner,1);
	if(!t||t->state==UNIT_FREEZING_ROARINGED)
		t=s->osite;
	hp=t->hp;
	t->hp=0;
	report(s->owner->field,MSG_DAMAGE,t,s,(long)hp,DAMAGE_MAGICAL,AF_CRIT,TYPE_ICE);
	//show as a critical magical damage corresponding with other roarings.
	unit_setstate(t,UNIT_FREEZING_ROARINGED);
	effectx(NULL,s,NULL,0,0,EFFECT_REMOVE|EFFECT_NEGATIVE|EFFECT_NONHOOKABLE|EFFECT_NODESTRUCT|EFFECT_UNPURIFIABLE);
	p1=&s->owner->field->winner;
	if(*p1==t->owner){
		*p1=NULL;
	}
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
void thorns_damage_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	long dmg;
	if(dest!=e->dest)
		return;
	switch(damage_type){
		case DAMAGE_PHYSICAL:
		case DAMAGE_MAGICAL:
			if(src&&(dmg=value*0.20)>0)
				break;
		default:
			return;
	}
	effect_ev(e)
		attack(src,dest,dmg,DAMAGE_REAL,aflag,type);
}
const struct effect_base thorns[1]={{
	.id="thorns",
	.damage_end=thorns_damage_end,
	.flag=EFFECT_PASSIVE,
}};
void thorns_init(struct unit *s){
	effect(thorns,s,s,0,-1);
}

void combo_attack_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
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
		effect_ev(e)
			normal_attack(src);
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

void hitback_attack_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
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
		effect_ev(e)
			normal_attack(dest);
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
void effect_update_attr_all(struct effect *e){
	update_attr_all((e->dest?e->dest:e->src)->owner->field);
}
void bm_roundend(struct effect *e){
	effect_ev(e){
		for_each_unit(u,e->src->owner->enemy){
			attack(u,NULL,u->max_hp/20,DAMAGE_REAL,0,TYPE_VOID);
		}
		for_each_unit(u,e->src->owner){
			attack(u,NULL,u->max_hp/20,DAMAGE_REAL,0,TYPE_VOID);
		}
	}
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

int hot_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_FIRE|TYPE_ICE))||!damage_pm(*damage_type))
		return 0;
	if(*type&TYPE_FIRE)
		*value*=1.5;
	if(*type&TYPE_ICE)
		*value*=0.5;
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

int wet_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_FIRE|TYPE_WATER))||!damage_pm(*damage_type))
		return 0;
	if(*type&TYPE_WATER)
		*value*=1.5;
	if(*type&TYPE_FIRE)
		*value*=0.5;
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
int cold_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_FIRE|TYPE_ICE))||!damage_pm(*damage_type))
		return 0;
	if(*type&TYPE_ICE)
		*value*=1.5;
	if(*type&TYPE_FIRE)
		*value*=0.5;
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
void gp_damage_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if(dest!=e->dest||!damage_pm(damage_type))
		return;
	effect_ev(e)
		effect_reinit(e,NULL,damage_type==DAMAGE_PHYSICAL?-3:-2,-1);
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
void bh_update_state(struct effect *e,struct unit *u,int *state){
	u->blockade|=ACTS_SWITCHUNIT;
}
int bh_su(struct effect *e,struct unit *u){
	return -1;
}
void bh_end(struct effect *e){
	long gp0,gp1;
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
	effect_ev(e){
		if(gp0<gp1)
			sethp(s,0);
		else
			sethp(t,0);
	}
}
const struct effect_base black_hole[1]={{
	.id="black_hole",
	.inited=bh_inited,
	.switchunit=bh_su,
	.update_state=bh_update_state,
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
int cyce_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if((*aflag&AF_IDEATH)&&duckcheck(dest)){
		return 1;
	}else
		return 0;
}
int cyce_hittest(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate){
	if(duckcheck(src)){
		return 1;
	}
	if(duckcheck(dest)){
		return 0;
	}
	return -1;
}
void cyce_trydamage(struct unit *u){
	if(!duckcheck(u)){
		attack(u,NULL,u->max_hp/9,DAMAGE_REAL,0,TYPE_MACHINE);
		effect(ATK,u,NULL,-1,-1);
		effect(DEF,u,NULL,-1,-1);
	}
}
void cyce_roundend(struct effect *e){
	struct player *p;
	effect_ev(e){
		p=e->src->owner;
		cyce_trydamage(p->front);
		cyce_trydamage(p->enemy->front);
	}
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
	attack(t,s,1.3*s->atk,DAMAGE_PHYSICAL,DF_KEEPALIVE,TYPE_MACHINE);
	effect(ATK,t,s,-2,-1);
}

void soul_back(struct unit *s){
	struct battle_field *f=s->owner->field;
	struct unit *t;
	t=get_unit_by_arg(s->owner,0);
	if(t)
		goto found;
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
	if(!revive(t,t->max_hp)){
		sethp(s,0);
		setcooldown(s,s->move_cur,16);
		return;
	}
fail:
	setcooldown(s,s->move_cur,4);
}
int absolutely_immortal_kill(struct effect *e,struct unit *u){
	if(u==e->dest){
		return -1;
	}
	return 0;
}
const struct effect_base absolutely_immortal_effect[1]={{
	.id="absolutely_immortal",
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
	if(4*labs(t->spi)>t->max_spi*3){
		instant_death(t);
		effect(ATK,s,s,2,-1);
	}else {
		effect(ATK,s,s,1,-1);
		if(hittest(t,s,1.0))
			attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
}
void escape(struct unit *s){
	struct unit *t;
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
	if(hittest(t,s,1.5))
		effect(confused,t,s,0,3);
	setcooldown(s,s->move_cur,6);
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
	effect(DEF,t,s,-1,-1);
	attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
}
void defend(struct unit *s){
	effect(DEF,s,s,2,-1);
}
void heat_engine_move_end(struct effect *e,struct unit *u,struct move *m){
	struct effect *ep;
	if(u==e->dest&&
		(m->type&TYPE_MACHINE)&&
		u->spi>=2-u->max_spi&&
		(ep=unit_findeffect(u->osite,burnt))
	){
		effect_ev(e){
			addspi(u,-2);
			effect_setround(ep,ep->round+1);
			ep->base->roundend(ep);
		}
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
	if(!revive_nonhookable(t,t->max_hp/2)){
		setcooldown(s,s->move_cur,31);
		switchunit(t);
		return;
	}
fail:
	setcooldown(s,s->move_cur,4);
}
long damage_get_in_round(struct unit *s,int round,int damage_types){
	struct battle_field *f=s->owner->field;
	long dmg=0;
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
void do_star_move(struct unit *s){
	long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_ALL_FLAG);
	if(dmg){
		attack(s->osite,s,dmg,DAMAGE_REAL,0,TYPE_VOID);
		heal(s,dmg);
	}
}
void star_move(struct unit *s){
	do_star_move(s);
	setcooldown(s,s->move_cur,s->state==UNIT_CONTROLLED?3:2);
}
struct unit *rebound_gettarget(struct effect *e,struct unit *u){
	if(u->owner!=e->dest->owner){
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
	effect_ev(e){
		memcpy(&am,&force_vh_pm,sizeof(struct move));
		unit_move(u,&am);
	}
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
void back(struct player *p,const struct player *h,long ds[6]){
	struct unit *b;
	for(int i=0;i<6;++i){
		if(!p->units[i].base){
			if(ds)
				ds[i]=0;
			continue;
		}
		if(ds)
			ds[i]=(h->units+i)->spi-(p->units+i)->spi;
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
void do_shear(struct unit *u,long ds){
	damage(u,NULL,SHEAR_COEF*u->max_hp*labs(ds)+1,DAMAGE_SHEAR,0,TYPE_MACHINE);
}
void time_back(struct unit *s){
	struct battle_field *f=s->owner->field;
	struct history *h;
	struct effect *head;
	long pds[6],eds[6];
	int round=*f->round-20,off=s->move_cur-s->moves;
	if(round<0)
		round=0;
	if(round>=f->ht_size)
		return;
	h=f->ht+round;
	back(f->p,&h->p,pds);
	back(f->e,&h->e,eds);
	head=f->effects;
	f->effects=effect_copyall(h->effects);
	effect_freeall(&head,f);
	report(f,MSG_UPDATE,&f->effects);
	update_attr_all(f);
	for(int i=0;i<6;++i){
		if(pds[i])
			do_shear(f->p->units+i,pds[i]);
		if(eds[i])
			do_shear(f->e->units+i,eds[i]);
	}
	if(s->moves[off].action==time_back)
		setcooldown(s,s->moves+off,41);
}
int repeat_action(struct effect *e,struct player *p){
	if(e->dest!=p->front)
		return 0;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
		case ACT_UNIT0 ... ACT_UNIT5:
		case ACT_NORMALATTACK:
		case ACT_ABORT:
			p->action=e->level;
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
		if(!do_repeat(s,t,3))
			setcooldown(s,s->move_cur,5);
	}
}
void silent_update_state(struct effect *e,struct unit *u,int *state){
	struct move *mp;
	if(e->dest!=u)
		return;
	mp=u->moves;
	for(int i=0;i<8;++i){
		if(!mp[i].id||(mp[i].flag&(MOVE_NOCONTROL|MOVE_NORMALATTACK)))
			continue;
		u->blockade|=1<<i;
	}
}
const struct effect_base silent[1]={{
	.id="silent",
	.init=abnormal_init,
	.update_state=silent_update_state,
	.flag=EFFECT_NEGATIVE|EFFECT_CONTROL
}};
void disarmed_update_state(struct effect *e,struct unit *u,int *state){
	struct move *mp;
	if(e->dest!=u)
		return;
	mp=u->moves;
	for(int i=0;i<8;++i){
		if(!mp[i].id||(mp[i].flag&MOVE_NOCONTROL)||!(mp[i].flag&MOVE_NORMALATTACK))
			continue;
		u->blockade|=1<<i;
	}
	u->blockade|=1<<ACT_NORMALATTACK;
}
const struct effect_base disarmed[1]={{
	.id="disarmed",
	.init=abnormal_init,
	.update_state=disarmed_update_state,
	.flag=EFFECT_NEGATIVE|EFFECT_CONTROL
}};
void head_blow(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
		effect(silent,t,s,0,4);
	}
	setcooldown(s,s->move_cur,5);
}
void rock_break(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_ROCK);
		effect(disarmed,t,s,0,4);
	}
	setcooldown(s,s->move_cur,5);
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
}
int perish_revive(struct effect *e,struct unit *u,unsigned long *hp){
	if(u==e->dest){
		return -1;
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
		attack(t,s,0.16*(t->max_hp-t->hp),DAMAGE_REAL,0,TYPE_NORMAL);
	}
	setcooldown(s,s->move_cur,3);
}

int marsh_getprior(struct effect *e,struct player *p){
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
		attack(t,s,t->hp/6,DAMAGE_REAL,0,TYPE_SOIL);
	effect(marsh,NULL,s,0,6);
	setcooldown(s,s->move_cur,5);
}
void nether_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	n=unit_hasnegative(s);
	if(n<=0){
		roaring_common_a(TYPE_GHOST){
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
		roaring_common_b;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_GHOST);
	effect(AVOID,s,s,n*2,-1);
}
#define attr_clear(_u) ((int)(size_t)effectx(NULL,(_u),NULL,0,0,EFFECT_REMOVE|EFFECT_ATTR))
#define attr_clear_positive(_u) ((int)(size_t)effectx(NULL,(_u),NULL,0,0,EFFECT_REMOVE|EFFECT_POSITIVE|EFFECT_ATTR))
void tidal(struct unit *s){
	struct unit *t=gettarget(s);
	int r=attr_clear_positive(t);
	if(r){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_DEVINEWATER);
	}else {
		attack(t,s,1.15*s->atk,DAMAGE_PHYSICAL,0,TYPE_DEVINEWATER);
		heal(s,0.35*s->atk);
	}
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
			p->action=rand_action(p);
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
		roaring_common_a(TYPE_FIGHTING)
			effect(silent,t,s,0,4);
		roaring_common_b;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_FIGHTING);
	if((double)t->hp/t->max_hp<=0.15+n*0.03)
		instant_death(t);
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
	effectx(NULL,s,NULL,0,0,EFFECT_REMOVE|EFFECT_NEGATIVE|EFFECT_ATTR);

}
void byebye(struct unit *s){
	unit_setstate(s,UNIT_FAILED);
}
void scent(struct unit *s){
	effectx(NULL,s,NULL,0,0,EFFECT_REMOVE|EFFECT_ABNORMAL);

}
void synthesis(struct unit *s){
	struct unit *t=gettarget(s);
	heal(s,((unit_type(t)&TYPE_LIGHT)?0.55:0.45)*s->max_hp);
	setcooldown(s,s->move_cur,4);
}
void hail_pm(struct unit *s){
	struct unit *t=gettarget(s);
	attack(t,s,0.45*s->atk+t->max_hp/32.0,DAMAGE_PHYSICAL,0,TYPE_ICE);
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
	effect_ev(e){
		memcpy(&am,&hail_p,sizeof(struct move));
		unit_move(e->dest,&am);
	}
}
void hail_inited(struct effect *e){
	e->active=0;
}
const struct effect_base hail_effect[1]={{
	.id="hail",
	.action_end=hail_action_end,
	.action_fail=hail_action_end,
	.inited=hail_inited,
	.flag=EFFECT_POSITIVE
}};
void hail(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,0.45*s->atk+t->max_hp/32.0,DAMAGE_PHYSICAL,0,TYPE_ICE);
	effect(hail_effect,s,s,0,5);
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
		return 1;
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
	int n=1;
	for_each_effect(e,s->owner->field->effects){
		if(e->dest==t&&(e->base==DEF||e->base==PDD||e->base==MDD)&&e->level>0){
				n+=e->level;
				purify(e);
		}
	}
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	}
	me=effect(maple,t,s,n,-1);
	if(me)
		heal(s,me->level);
}
void piercing_missile(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e=unit_findeffect(t,maple);
	long n;
	double dmg=40+0.1*s->atk+0.03*t->max_hp;
	n=e?e->level:0;
	if(n>0){
		attack(t,s,dmg*pow(1.15,n),DAMAGE_REAL,0,TYPE_NORMAL);
		effect_reinit(e,s,-n,-1);
	}else
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
	long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_ALL_FLAG);
	if(dmg){
		attack(s->osite,s,1.5*dmg,DAMAGE_REAL,0,TYPE_VOID);
		setcooldown(s,s->move_cur,2);
	}
}
void physical_reflex(struct unit *s){
	long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_PHYSICAL_FLAG);
	if(dmg){
		attack(s->osite,s,2*dmg,DAMAGE_REAL,0,TYPE_VOID);
		setcooldown(s,s->move_cur,2);
	}
}
void magical_reflex(struct unit *s){
	long dmg=damage_get_in_round(s,*s->owner->field->round,DAMAGE_MAGICAL_FLAG);
	if(dmg){
		attack(s->osite,s,2*dmg,DAMAGE_REAL,0,TYPE_VOID);
		setcooldown(s,s->move_cur,2);
	}
}
int damage_reverse_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest)
		return 0;
	*value=-*value;
	return 0;
}
const struct effect_base damage_reverse_effect[1]={{
	.damage=damage_reverse_damage,
	.flag=EFFECT_POSITIVE,
}};
void damage_reverse(struct unit *s){
	effect(damage_reverse_effect,s,s,0,1);
	setcooldown(s,s->move_cur,4);
}
int shield_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest||*value<0)
		return 0;
	if(*value>e->level){
		effect_ev(e){
			*value-=e->level;
			if(!(*aflag&DF_TEST))
				effect_reinit(e,src,-e->level,-1);
		}
		return 0;
	}
	if(*aflag&DF_TEST)
		return -1;
	effect_ev(e){
		effect_reinit(e,src,-*value,-1);
	}
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
	.roundend=shield_roundend,
	.data_size=sizeof(double),
}};
int heal_weak_heal(struct effect *e,struct unit *dest,long *value){
	double coef;
	if(dest!=e->dest)
		return 0;
	coef=1-*(double *)e->data;
	if(coef>0.0)
		*value*=coef;
	else
		*value=0;
	return 0;
}
const struct effect_base heal_weak[1]={{
	.id="heal_weak",
	.heal=heal_weak_heal,
	.data_size=sizeof(double),
	.flag=EFFECT_NEGATIVE
}};
int heal_bonus_heal(struct effect *e,struct unit *dest,long *value){
	double coef;
	if(dest!=e->dest)
		return 0;
	coef=1+*(double *)e->data;
	if(coef>1)
		*value*=coef;
	return 0;
}
const struct effect_base heal_bonus[1]={{
	.id="heal_bonus",
	.heal=heal_bonus_heal,
	.data_size=sizeof(double),
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
	.data_size=sizeof(double),
	.flag=EFFECT_POSITIVE
}};
int elbow1_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	double coef;
	if(dest==e->dest&&*dest->owner->field->stage==STAGE_ROUNDEND){
		coef=1-*(double *)e->data;
		if(coef<0.1)
			coef=0.1;
		*value*=coef;
	}
	return 0;
}
const struct effect_base elbow1[1]={{
	.attack=elbow1_attack,
	.data_size=sizeof(double),
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
#define res_register(_id,_max)\
int _id##_init(struct effect *e,long level,int round){\
	long __max=(_max);\
	level+=e->level;\
	if(level>__max)\
		level=__max;\
	if(level<=0)\
		level=0;\
	e->round=-1;\
	e->level=level;\
	return 0;\
}\
const struct effect_base _id[1]={{\
	.id=#_id,\
	.init=_id##_init,\
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP\
}}
res_register(mana,e->dest->level);
void elbow(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	int x;
	long dmg=0.75*s->atk;
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
		heal(s,s->max_hp*(0.07+0.05*l));
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
			effect(shield,s,s,0.08*s->max_hp+0.25*s->atk,3);
		}
	}
	d0=(double)s->hp/(double)s->max_hp;
	d1=(double)t->hp/(double)t->max_hp;
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
	e=unit_findeffectf(s,EFFECT_ABNORMAL);
	if(e){
		if(e->base==cursed||e->base==radiated){
			heal(s,0.15*s->max_hp);
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
	effect(mana,s,s,15+randi()%21,-1);
	heal(s,0.08*s->max_hp);
}
void mana_gather_init(struct unit *s){
	effect(mana,s,s,0.2*s->level,-1);
}
void fury_swipes(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	int i;
	unsigned long r=0;
	e=unit_findeffect(s,mana);
	if(e&&e->level>=8){
		effect_reinit(e,s,-8,-1);
		i=4+randi()%5;
	}else
		i=2+randi()%4;
	for(;i>0;--i)
		if(hittest(t,s,1.0))
			r+=attack(t,s,0.2*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	report(s->owner->field,MSG_DAMAGE,t,s,r,DAMAGE_TOTAL,0,TYPE_NORMAL);
}
void razor_carrot(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.7)){
		attack(t,s,0.45*s->atk,DAMAGE_PHYSICAL,0,TYPE_GRASS);
		effect(aurora,t,s,0,5);
	}
	setcooldown(s,s->move_cur,8);
}


const struct event derate_count[1]={{NULL}};
const struct event def_count[1]={{NULL}};
struct u_dbl {
	struct unit *u;
	double d;
	int damage_type,type;
};
struct u_l {
	struct unit *u;
	long l;
	int damage_type,type;
};
int derate_counter(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	struct u_dbl ud;
	double d;
	if(*aflag&AF_NODERATE)
		return 0;
	switch(*damage_type){
		case DAMAGE_PHYSICAL:
			d=dest->physical_derate;
			if(src)
				d-=src->physical_bonus;
			break;
		case DAMAGE_MAGICAL:
			d=dest->magical_derate;
			if(src)
				d-=src->magical_bonus;
			break;
		default:
			d=0.0;
			break;
	}
	ud.d=d;
	ud.u=dest;
	ud.damage_type=*damage_type;
	ud.type=*type;
	event_callback(derate_count,src,&ud);
	*value*=derate_coef(ud.d);
	*aflag|=AF_NODERATE;
	return 0;
}
int def_counter(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	struct u_l ul;
	long def;
	if(*aflag&AF_NODEF)
		return 0;
	def=dest->def+dest->level-src->level;
	ul.l=def;
	ul.u=dest;
	ul.damage_type=*damage_type;
	ul.type=*type;
	event_callback(def_count,src,&ul);
	*value*=def_coef(ul.l);
	*aflag|=AF_NODEF;
	return 0;
}
const struct effect_base dcounter[1]={{
	.attack=derate_counter,
	.flag=EFFECT_PASSIVE,
	.prior=-32768,
}};
const struct effect_base defcounter[1]={{
	.attack=def_counter,
	.flag=EFFECT_PASSIVE,
	.prior=-32768,
}};


int moon_elf_shield_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest!=e->dest||!e->level||*value<0)
		return 0;
	if(*type){
		if(*type&e->unused){
			if(*value>1)
				*value=1;
		}else if(e->unused==TYPE_STEEL){
			int t;
			if(*aflag&DF_TEST){
				return -1;
			}
			t=*type;
			t|=((t&TYPES_DEVINE)>>17)|((t&(TYPE_GRASS|TYPE_FIRE|TYPE_WATER))<<17);
			e->unused=t;
			if(*value>1)
				*value=1;
		}
	}
	if(*value>e->level){
		effect_ev(e){
			*value-=e->level;
			if(!(*aflag&DF_TEST))
				effect_reinit(e,src,-e->level,-1);
		}
		return 0;
	}
	effect_ev(e){
		effect_reinit(e,src,-*value,-1);
	}
	return -1;
}
int moon_elf_shield_init(struct effect *e,long level,int round){
	if(!e->round)
		e->round=-1;
	if(level>=0){
		e->level=level;
		if(level>0)
			e->unused=TYPE_STEEL;
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
	if(dest!=e->dest)
		return;
	if(unit_findeffect(dest,moon_elf_shield_cooldown))
		return;
	effect_ev(e){
		if(!effectx(NULL,e->dest,NULL,0,0,EFFECT_FIND|EFFECT_CONTROL))
			return;
		effectx(NULL,e->dest,NULL,0,0,EFFECT_REMOVE|EFFECT_CONTROL);
		effect_reinit(e,dest,3*dest->base->def,-1);
		effect(moon_elf_shield_cooldown,dest,dest,0,5);
	}
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
int adbd_event(struct effect *e,const struct event *ev,struct unit *src,void *arg){
	struct u_l *ul;
	if(ev!=def_count)
		return 0;
	if(src!=e->dest)
		return 0;
	ul=arg;
	if(!damage_pm(ul->damage_type))
		return 0;
	if(src->def<3)
		return 0;
	ul->l-=0.35*src->def;
	return 0;
}
const struct effect_base anti_def_by_def[1]={{
	.id="anti_def_by_def",
	.event=adbd_event,
	.flag=EFFECT_PASSIVE,
}};
void anti_def_by_def_p(struct unit *s){
	effect(anti_def_by_def,s,s,0,-1);
	effect(defcounter,NULL,s,0,-1);
}
void burn_boat(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	double d0;
	long dmg;
	if(hittest(t,s,1.5)){
		d0=(double)s->hp/(double)s->max_hp;
		dmg=(3.0-2.0*d0)*s->atk;
		attack(t,s,dmg,DAMAGE_PHYSICAL,0,TYPE_FIGHTING);
		e=unit_findeffect(s,moon_elf_shield);
		if(e&&e->level&&e->unused!=TYPE_STEEL)
			attack(t,s,0.4*dmg,DAMAGE_PHYSICAL,0,e->unused);
	}
	if(!isalive(t->state))
		return;
	setcooldown(s,s->move_cur,6);
	effect(stunned,s,s,0,1);
}
int burn_boat_kill(struct effect *e,struct unit *u){
	if(u==e->dest&&!e->level){
		effect_ev(e){
			effect_setlevel(e,2);
			sethp(u,u->max_hp);
			effectx(moon_elf_shield_cooldown,u,NULL,0,0,EFFECT_REMOVE|EFFECT_NONHOOKABLE|EFFECT_UNPURIFIABLE);
			effectx(NULL,u,NULL,0,0,EFFECT_REMOVE|EFFECT_NEGATIVE);
			for_each_move(m,u){
				if(m->id&&m->action==metal_bomb){
					event(real_fire_disillusion_fr,u);
					break;
				}
			}
		}
	}
	return 0;
}
int burn_boat_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	long dmg,def;
	if(dest==e->dest){
		switch(e->level){
			case 2:
				return -1;
			case 1:
				dmg=0.4*dest->max_hp;
				def=e->dest->def;
				if(def)
					dmg*=def_coef(def);
				if(*value>dmg){
					*value=dmg;
				}
				break;
			default:
				break;
		}
	}
	return 0;
}
int burn_boat_effect1(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long *level,int *round,int *xflag){
	struct effect *ep;
	if(dest==e->dest&&e->level){
		if(base==moon_elf_shield_cooldown)
			return -1;
		if(base->flag&EFFECT_CONTROL){
			if(*xflag&EFFECT_TEST)
				return -1;
			effect_ev(e){
				ep=unit_findeffect(dest,moon_elf_shield);
				if(ep)
					effect_reinit(ep,dest,3*dest->base->def,-1);
			}
			return -1;
		}
	}
	return 0;
}
void burn_boat_roundstart(struct effect *e){
	if(e->level==2){
		effect_ev(e){
			effect_setlevel(e,1);
		}
	}
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
		attack(t,s,0.43*s->atk,DAMAGE_PHYSICAL,AF_FLOAT,TYPE_ELECTRIC);
		attack(t,s,0.43*s->atk,DAMAGE_MAGICAL,AF_FLOAT,TYPE_ELECTRIC);
	}
}
void metal_syncretize(struct unit *s){
	struct unit *t=gettarget(s);
	attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
	effect(PDB,s,s,1,-1);
}
int uniform_base_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	unsigned long dmg,x;
	if(dest!=e->dest||*value<=0)
		return 0;
	dmg=*value;
	if(!(dmg&(dmg-1)))
		return 0;
	x=0;
	do {
		dmg>>=1;
		++x;
	}while(dmg&(dmg-1));
	*value=dmg<<x;
	return 0;
}
const struct effect_base uniform_base_effect[1]={{
	.id="uniform_base",
	.flag=EFFECT_PASSIVE,
	.damage=uniform_base_damage,
	.prior=-256
}};
void uniform_base(struct unit *s){
	effect(uniform_base_effect,s,s,0,-1);
}
void uniform_base_a(struct unit *s){
	struct unit *t=s->osite;
	long sd;
	attack(t,s,0.5*s->atk+7.5*labs(s->spi),DAMAGE_PHYSICAL,AF_FLOAT|AF_EFFECT|AF_WEAK,TYPE_MACHINE);
	sd=40+(8*SHEAR_COEF)*t->max_hp;
	if(s->spi){
		sd+=(long)(SHEAR_COEF*t->max_hp*labs(s->spi)+1);
		setspi(s,0);
	}
	damage(t,s,sd,DAMAGE_SHEAR,0,TYPE_MACHINE);
}
void spatially_shatter_pm(struct unit *s){
	struct unit *t=s->osite;
	long vdmg=0.875*t->max_hp;
	long atk=s->atk;
	if(vdmg<atk)
		vdmg=atk;
	damage(t,s,vdmg,DAMAGE_VOID,0,TYPE_VOID);
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
	if(*e->dest->owner->field->round<=e->unused
			||p!=e->dest->owner->enemy
			||p->action!=e->level)
		return;
	effect_ev(e){
		memcpy(&am,&spatially_shatter_p,sizeof(struct move));
		unit_move(e->dest,&am);
	}
	effect_end(e);
}
int spatially_shatter_init(struct effect *e,long level,int round){
	e->unused=*e->dest->owner->field->round;
	e->level=level;
	e->round=round;
	return 0;
}
const struct effect_base spatially_shatter_effect[1]={{
	.id="spatially_shatter",
	.init=spatially_shatter_init,
	.action_end=spatially_shatter_action_end,
	.flag=EFFECT_POSITIVE|EFFECT_ISOLATED
}};
void spatially_shatter(struct unit *s){
	struct unit *t=gettarget(s);
	int a;
	if(hittest(t,s,1.0))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_DRAGON);
	else
		attack(t,s,s->atk/2,DAMAGE_MAGICAL,0,TYPE_DRAGON);
	a=s->owner->enemy->action;
	if(a!=ACT_ABORT)
		effect(spatially_shatter_effect,s,s,a,2);
}
void health_reset(struct unit *s){
	struct unit *t=s->osite;
	sethp(s,s->max_hp);
	sethp(t,t->max_hp);
	setcooldown(s,s->move_cur,21);
}
void soil_loosening(struct unit *s){
	struct unit *t=gettarget(s);
	if(!hittest(t,s,1.0))
		return;
	attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,0,TYPE_SOIL);
	if(unit_type(t)&TYPE_SOIL)
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
void dark_night_light(struct unit *s){
	struct unit *t=gettarget(s);
	unsigned long hp;
	if(!hittest(t,s,1.0))
		return;
	attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
	hp=t->hp;
	if(hp<INT_MAX&&!isprime(hp&0x7ffffffful))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
}
const struct effect_base natural_decay_effect[1];
void natural_decay_pm(struct unit *s){
	struct unit *t;
	struct effect *e=unit_findeffect(s,natural_decay_effect);
	if(!e)
		return;
	if(e->level<2){
		t=gettarget(s);
		attack(t,s,0.4122*s->atk+0.01832*t->max_hp,DAMAGE_PHYSICAL,0,TYPE_DEVINEGRASS);
		effect_addlevel(e,1);
	}else {
		t=s->osite;
		sethp(t,t->hp>2?(unsigned long)log(t->hp):0);
		effect_end(e);
	}
}
const struct move natural_decay_p={
	.id="natural_decay",
	.action=natural_decay_pm,
	.type=TYPE_DEVINEGRASS,
	.prior=0,
	.flag=MOVE_NOCONTROL,
	.mlevel=MLEVEL_CONCEPTUAL
};
void natural_decay_action_end(struct effect *e,struct player *p){
	struct move am;
	if(e->dest->owner!=p)
		return;
	if(!e->active){
		e->active=1;
		return;
	}
	effect_ev(e){
		memcpy(&am,&natural_decay_p,sizeof(struct move));
		unit_move(e->dest,&am);
	}
}
void natural_decay_inited(struct effect *e){
	e->active=0;
}
void natural_decay_switchunit_end(struct effect *e,struct unit *t){
	if(t->owner!=e->dest->owner){
		effect_end(e);
	}
}
const struct effect_base natural_decay_effect[1]={{
	.id="natural_decay",
	.action_end=natural_decay_action_end,
	.action_fail=natural_decay_action_end,
	.switchunit_end=natural_decay_switchunit_end,
	.inited=natural_decay_inited,
	.flag=EFFECT_POSITIVE
}};
void natural_decay(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,0.4122*s->atk+0.01832*t->max_hp,DAMAGE_PHYSICAL,0,TYPE_DEVINEGRASS);
	effect(natural_decay_effect,s,s,0,-1);
}
void dmts_impact(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.4)){
		attack(t,s,0.7*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		effect(stunned,t,s,0,1+randi()%3);
		setcooldown(s,s->move_cur,5);
	}
}
int spi_check_dec(struct unit *s,long n){
	if(n<0){
		if(s->max_spi-s->spi<-n)
			return 0;
	}else {
		if(s->spi+s->max_spi<n)
			return 0;
	}
	addspi(s,-n);
	return 1;
}
#define ava_spi(count)\
static int ava_spi_##count(struct unit *s,struct move *m){\
	return count<0?s->max_spi-s->spi>=-count:s->spi+s->max_spi>=count;\
}
ava_spi(5);
void dmts_spiblade(struct unit *s){
	struct unit *t=gettarget(s);
	if(spi_check_dec(s,5)&&hittest(t,s,1.0)){
		attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		effect(burnt,t,s,0,5);
	}
}
void dmts_pulse(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.5)){
		attack(t,s,0.7*s->atk,DAMAGE_PHYSICAL,0,TYPE_ELECTRIC);
		if(!effect(paralysed,t,s,0,3)){
			attack(t,s,0.1*t->max_hp,DAMAGE_REAL,0,TYPE_ELECTRIC);
		}else
			setcooldown(s,s->move_cur,6);
	}
}
int anticontrol(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long *level,int *round,int *xflag){
	if(dest==e->dest&&(base->flag&EFFECT_CONTROL)){
		return -1;
	}
	return 0;
}

void hf_hpmod(struct effect *e,struct unit *dest,long hp,int flag){
	const struct message *msg;
	if(dest==e->dest&&((e->unused&3)<2)){
		msg=message_find(dest->owner->field,(1<<MSG_HPMOD)|(1<<MSG_HEAL)|(1<<MSG_DAMAGE));
		if(msg&&(msg=message_findsource(msg))){
			if(msg->type==MSG_EVENT&&msg->un.event.ev==spi_modified)
				return;
		}
		effect_ev(e){
			++e->unused;
			effect(ATK,dest,dest,1,-1);
		}
	}
}
void hf_spimod(struct effect *e,struct unit *dest,long hp){
	if(dest==e->dest&&((e->unused>>2)<2)){
		effect_ev(e){
			e->unused+=4;
			effect(DEF,dest,dest,1,-1);
		}
	}
}
void hf_roundstart(struct effect *e){
	e->unused=0;
}
void hf_roundend(struct effect *e){
	struct unit *s=e->dest;
	if(e->round<=1&&e->level<3&&labs(s->spi)>=20){
		effect_ev(e){
			setspi(s,abs_add(s->spi,-10));
			effect_reinitx(e,s,1,e->round+1,EFFECT_ADDLEVEL);
		}
	}
}
const struct effect_base high_frequency[1]={{
	.id="high_frequency",
	.effect=anticontrol,
	.hpmod=hf_hpmod,
	.spimod=hf_spimod,
	.roundend=hf_roundend,
	.roundstart=hf_roundstart,
	.flag=EFFECT_POSITIVE,
}};

void attacking_defensive_combine(struct unit *s){
	struct unit *t=gettarget(s);
	long dmg;
	if(hittest(t,s,2.0)){
		dmg=0.75*s->atk+0.75*s->def+4.4*labs(s->spi);
		if(dmg<1)
			dmg=1;
		attack(t,s,dmg,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	}
	if(s->hp*3>s->max_hp){
		addspi(s,0.1*s->max_spi);
		heal(s,0.5*s->max_hp+6.3*labs(s->spi));
	}else {
		heal(s,0.5*s->max_hp+6.3*labs(s->spi));
		addspi(s,0.1*s->max_spi);
	}
	effect(high_frequency,s,s,0,5);
	setcooldown(s,s->move_cur,11);
}
void elasticity_module_damage_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if((src!=e->dest&&dest!=e->dest)||e->unused>1||damage_type!=DAMAGE_PHYSICAL)
		return;
	effect_ev(e){
		addspi(e->dest,0.05*e->dest->max_spi);
		++e->unused;
	}
}
void elasticity_module_roundstart(struct effect *e){
	e->unused=0;
}
int elasticity_module_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&*damage_type==DAMAGE_PHYSICAL)
		*value*=1.02+0.0075*labs(src->spi);
	return 0;
}
const struct effect_base elasticity_module[1]={{
	.id="elasticity_module",
	.damage_end=elasticity_module_damage_end,
	.roundstart=elasticity_module_roundstart,
	.attack=elasticity_module_attack,
	.flag=EFFECT_PASSIVE,
}};
void elasticity_module_init(struct unit *s){
	effect(elasticity_module,s,s,0,-1);
}
int linearly_dependent_check_64(const long *vec0,const long *vec1,size_t count){
	long v0,v1;
	if(count<2)
		return 1;
	v0=*vec0;
	v1=*vec1;
	while(--count){
		if(*(++vec0)*v1!=*(++vec1)*v0)
			return 0;
	}
	return 1;
}
int unit_attr_level_vector_fill(struct unit *u,long vec[10]){
	int r=0;
	vec[0]=unit_effect_level(u,ATK);
	vec[1]=unit_effect_level(u,DEF);
	vec[2]=unit_effect_level(u,SPEED);
	vec[3]=unit_effect_level(u,HIT);
	vec[4]=unit_effect_level(u,AVOID);
	vec[5]=unit_effect_level(u,PDB);
	vec[6]=unit_effect_level(u,MDB);
	vec[7]=unit_effect_level(u,PDD);
	vec[8]=unit_effect_level(u,MDD);
	vec[9]=unit_effect_level(u,CE);
	for(int i=0;i<10;++i){
		if(vec[i])
			++r;
	}
	return r;
}
void linear_blast(struct unit *s){
	struct unit *t;
	long vec_s[10],vec_t[10];
	int ge2;
	ge2=unit_attr_level_vector_fill(s,vec_s)>=2
	&&unit_attr_level_vector_fill(s->osite,vec_t)>=2;

	if(ge2&&linearly_dependent_check_64(vec_s,vec_t,10)){
		sethp(s->osite,0);
		return;
	}
	t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_GHOST);
	if(!ge2)
		return;
	for(int i=0;i<10;++i){
		if(!vec_s[i]!=!vec_t[i]){
			t=s->osite;
			attack(t,s,0.3*t->max_hp,DAMAGE_MAGICAL,AF_EFFECT|AF_WEAK,TYPE_GHOST);
			return;
		}
	}
	t=s->osite;
	attack(t,s,0.3*t->max_hp,DAMAGE_REAL,0,TYPE_GHOST);
}
void swagger(struct unit *s){
	effect(ATK,s,s,1,-1);
	effect(ATK,s->osite,s,1,-1);
}
void aswap(struct unit *u,struct unit *e,const struct effect_base *base){
	long lu,le;
	lu=unit_effect_level(u,base);
	le=unit_effect_level(e,base);
	if(lu!=le){
		effect(base,u,u,le-lu,-1);
		effect(base,e,u,lu-le,-1);
	}
}
void attribute_swap(struct unit *s){
	struct unit *t=s->osite;
	aswap(s,t,ATK);
	aswap(s,t,DEF);
	aswap(s,t,SPEED);
	aswap(s,t,HIT);
	aswap(s,t,AVOID);
	aswap(s,t,PDB);
	aswap(s,t,MDB);
	aswap(s,t,PDD);
	aswap(s,t,MDD);
	aswap(s,t,CE);
	setcooldown(s,s->move_cur,6);
}
struct unit *rand_backend(struct unit *s){
	struct unit *t;
	struct unit *ava[5];
	size_t rsize=0;
	for(int i=0;i<6;++i){
		t=s->owner->units+i;
		if(t==s||!t->base||!isalive(t->state))
			continue;
		ava[rsize++]=t;
	}
	if(!rsize)
		return NULL;
	return ava[randi()%rsize];
}
struct unit *rand_unit(struct unit *s){
	struct unit *t;
	struct unit *ava[6];
	size_t rsize=0;
	for(int i=0;i<6;++i){
		t=s->owner->units+i;
		if(!t->base||!isalive(t->state))
			continue;
		ava[rsize++]=t;
	}
	if(!rsize)
		return NULL;
	return ava[randi()%rsize];
}
struct unit *deflecting_field_gettarget(struct effect *e,struct unit *u){
	if(u->owner!=e->dest->owner){
		return rand_backend(u);
	}
	return NULL;
}
const struct effect_base deflecting_field_effect[1]={{
	.gettarget=deflecting_field_gettarget,
	.flag=EFFECT_POSITIVE
}};
void deflecting_field(struct unit *s){
	effect(deflecting_field_effect,s,s,0,1);
	setcooldown(s,s->move_cur,3);
}
void clay_charge(struct unit *s){
	struct unit *t;
	int n=0,ne=0;
	for_each_effect(e,s->owner->field->effects){
		if(e->dest!=s||!((e->base->flag&(EFFECT_ABNORMAL|EFFECT_CONTROL))||((e->base->flag&EFFECT_ATTR)&&e->level<0)))
			continue;
		++n;
		if(e->src&&e->src->owner!=s->owner)
			++ne;
		purify(e);
	}
	if(n>=3||ne>=2){
		instant_death(s->osite);
		return;
	}
	t=gettarget(s);
	if(hittest(t,s,1.3))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_SOIL);
	setcooldown(s,s->move_cur,4);
}
int od_action(struct effect *e,struct player *p){
	if(p->front==e->dest)
		return -1;
	return 0;
}
const struct effect_base operation_denied[1]={{
	.action=od_action,
	.flag=EFFECT_UNPURIFIABLE|EFFECT_NONHOOKABLE,
}};
void karma_reverse(struct unit *s){
	struct battle_field *f=s->owner->field;
	struct history *h;
	int round=*f->round-5;
	struct unit *t;
	if(round<0)
		round=0;
	if(round>=f->ht_size)
		return;
	h=f->ht+round;
	for(int i=0;i<6;++i){
		if(f->p->units[i].state==UNIT_FAILED&&h->p.units[i].state!=UNIT_FAILED)
			revive_nonhookable(f->p->units+i,h->p.units[i].hp);
		if(f->e->units[i].state==UNIT_FAILED&&h->e.units[i].state!=UNIT_FAILED)
			revive_nonhookable(f->e->units+i,h->e.units[i].hp);
	}
	if(!s->owner->enemy->acted){
		t=s->owner->enemy->front;
		effect(operation_denied,t,t,0,1);
	}
	setcooldown(s,s->move_cur,36);
}
const struct effect_base anchor_bomb_effect[1];
void anchor_bomb_pm(struct unit *s){
	struct unit *t=s->osite;
	struct effect *e=unit_findeffect(s,anchor_bomb_effect);
	long ds;
	if(!e)
		return;
	ds=e->level;
	if(t->spi+t->max_spi<ds)
		ds=t->spi+t->max_spi;
	if(!ds)
		return;
	addspi(t,-ds);
	addspi(s,ds);
	heal(s,ds*0.02*s->max_hp);
	if(ds==e->level)
		effect(PDB,s,s,1,-1);
}
const struct move anchor_bomb_p={
	.id="anchor_bomb",
	.action=anchor_bomb_pm,
	.type=TYPE_MACHINE,
	.prior=0,
	.flag=MOVE_NOCONTROL,
	.mlevel=MLEVEL_REGULAR
};
void anchor_bomb_action_end(struct effect *e,struct player *p){
	struct move am;
	if(e->dest!=p->front)
		return;
	if(!e->active){
		e->active=1;
		return;
	}
	effect_ev(e){
		memcpy(&am,&anchor_bomb_p,sizeof(struct move));
		unit_move(e->dest,&am);
	}
}
void anchor_bomb_inited(struct effect *e){
	e->active=0;
}
const struct effect_base anchor_bomb_effect[1]={{
	.id="anchor_bomb",
	.action_end=anchor_bomb_action_end,
	.action_fail=anchor_bomb_action_end,
	.inited=anchor_bomb_inited,
	.flag=EFFECT_POSITIVE
}};
void anchor_bomb(struct unit *s){
	struct unit *t=gettarget(s);
	long ds=16;
	if(hittest(t,s,1.3))
		attack(t,s,1.2*s->atk+3.6*labs(s->spi),DAMAGE_PHYSICAL,0,TYPE_MACHINE);
	t=s->osite;
	if(t->spi+ds>t->max_spi)
		ds=t->max_spi-t->spi;
	if(ds){
		addspi(t,ds);
		effect(anchor_bomb_effect,s,s,ds,2);
	}
	setcooldown(s,s->move_cur,4);
}
int time_frozen_action(struct effect *e,struct player *p){
	if(p->front==e->src)
		return 0;
	return -1;
}
int time_frozen_move(struct effect *e,struct unit *u,struct move *m){
	if(u==e->src)
		return 0;
	return -1;
}
int time_frozen_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->src)
		return 0;
	return -1;
}
int time_frozen_heal(struct effect *e,struct unit *dest,long *value){
	if(dest==e->src)
		return 0;
	return -1;
}
const struct effect_base time_frozen[1]={{
	.id="time_frozen",
	.action=time_frozen_action,
	.move=time_frozen_move,
	.damage=time_frozen_damage,
	.heal=time_frozen_heal,
	.flag=EFFECT_UNPURIFIABLE,
}};
void time_solidify(struct unit *s){
	struct unit *t=s->osite;
	attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_DRAGON);
	effect(time_frozen,NULL,s,0,4);
	setcooldown(s,s->move_cur,26);
}
void squeeze(struct unit *s){
	long n=0;
	for_each_unit(u,s->owner){
		if(u==s||!isalive(u->state)||unit_effect_level(u,ATK)==ATTR_MIN)
			continue;
		if(effect(ATK,u,s,-1,-1))
			++n;
	}
	effect(ATK,s,s,n,-1);
	setcooldown(s,s->move_cur,6);
}
int accumulated_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&damage_pm(*damage_type)&&!(*type&TYPES_DEVINE)&&!(*aflag&AF_CRIT)){
		*aflag|=AF_CRIT;
		if(!(*aflag&DF_TEST))
			effect_end(e);
	}
	return 0;
}
const struct effect_base accumulated[1]={{
	.id="accumulated",
	.attack=accumulated_attack,
	.flag=EFFECT_POSITIVE|EFFECT_KEEP,
}};
void accumulate(struct unit *s){
	effect(accumulated,s,s,0,2);
	setcooldown(s,s->move_cur,2);
}
int voltage_transforming_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(src==e->dest&&*damage_type==DAMAGE_PHYSICAL&&!(*type&TYPES_DEVINE)&&!(*aflag&AF_CRIT)&&e->level==1&&labs(src->spi)>=8){
		effect_ev(e){
			if(!(*aflag&DF_TEST))
				setspi(src,abs_add(src->spi,-8));
			*aflag|=AF_CRIT;
		}
	}
	return 0;
}
void voltage_transforming_attack_end0(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if(src==e->dest&&damage_type==DAMAGE_PHYSICAL&&!(type&TYPES_DEVINE)&&e->level==2){
		effect_ev(e){
			addspi(src,6);
			heal(src,0.08*src->max_hp+1.6*labs(src->spi));
		}
	}
}
const struct effect_base voltage_transforming[1]={{
	.id="voltage_transforming",
	.attack=voltage_transforming_attack,
	.attack_end0=voltage_transforming_attack_end0,
	.flag=EFFECT_PASSIVE,
}};
void voltage_transformation_init(struct unit *s){
	effect(voltage_transforming,s,s,2,-1);
}
void voltage_transformation(struct unit *s){
	struct effect *e=unit_findeffect(s,voltage_transforming);
	heal(s,0.2*s->max_hp+2.5*labs(s->spi));
	if(!e)
		return;
	effect_reinit(e,s,(e->level&1)?2:1,-1);
}
int rec_check_dec(const struct effect_base *base,struct unit *s,long n){
	struct effect *e=unit_findeffect(s,base);
	if(!e||e->level<n)
		return 0;
	effect_reinit(e,s,-n,-1);
	return 1;
}
#define ava_rec(id,count)\
static int ava_##id##_##count(struct unit *s,struct move *m){\
	return unit_effect_level(s,id)>=count;\
}
ava_rec(mana,10);
void fire_ball(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(mana,s,10)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
	}
}
void ice_ball(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(mana,s,10)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
	}
}
void water_ball(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(mana,s,10)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_WATER);
	}
}
void lighting_bolt_ball(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(mana,s,10)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_ELECTRIC);
	}
}
res_register(youkai,3*e->dest->max_hp);
res_register(electric,sqrt(3*e->dest->max_hp*e->dest->level));
void three_phase_drive_init(struct unit *s){
	effect(mana,s,s,30,-1);
	effect(youkai,s,s,2500,-1);
	effect(electric,s,s,270,-1);
}
void three_phase_drive(struct unit *s){
	int r=*s->owner->field->round%3;
	effect(mana,s,s,r==0?28:20,-1);
	effect(youkai,s,s,r==1?2262:1600,-1);
	effect(electric,s,s,r==2?254:180,-1);
	heal(s,0.12*s->max_hp);
	setcooldown(s,s->move_cur,3);
}
void celestial_phenomena(struct unit *s){
	effect(CE,s,s,1,-1);
}
ava_rec(electric,90);
void pulse(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(electric,s,90)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_ELECTRIC);
	}
}
ava_rec(electric,220);
void electric_shield(struct unit *s){
	if(rec_check_dec(electric,s,220)){
		effect(shield,s,s,0.5*s->max_hp,3);
		setcooldown(s,s->move_cur,5);
	}
}
ava_rec(youkai,800);
void wind_blade(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(youkai,s,800)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_WIND);
	}
}
int protecting_effect(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long *level,int *round,int *xflag){
	if(dest==e->dest&&effect_isnegative_base(base,dest,src,*level))
		return -1;
	return 0;
}
int protecting_kill(struct effect *e,struct unit *u){
	if(u==e->dest){
		return -1;
	}
	return 0;
}
void protecting_hpmod(struct effect *e,struct unit *dest,long hp,int flag){
	if(dest==e->dest&&hp<0){
		effect_ev(e){
			addhp(dest,-hp);
		}
	}
}
int protecting_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest){
		return -1;
	}
	return 0;
}
const struct effect_base protecting[1]={{
	.id="protecting",
	.damage=protecting_damage,
	.kill=protecting_kill,
	.effect=protecting_effect,
	.hpmod=protecting_hpmod,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_NONHOOKABLE,
}};
void protect(struct unit *s){
	effect(protecting,s,s,0,1);
	setcooldown(s,s->move_cur,4);
}
int ingrained_su(struct effect *e,struct unit *u){
	if(u->owner->front==e->dest)
		return -1;
	return 0;
}
void ingrained_roundend(struct effect *e){
	struct unit *s=e->dest;
	if(!((*s->owner->field->round-e->unused)%5))
		effect(PDD,s,s,1,-1);
	heal(s,s->max_hp/20);
}
void ingrained_inited(struct effect *e){
	e->unused=*e->dest->owner->field->round;
}
void ingrained_update_state(struct effect *e,struct unit *u,int *state){
	if(u==e->dest)
		u->blockade|=ACTS_SWITCHUNIT;
}
const struct effect_base ingrained[1]={{
	.id="ingrained",
	.switchunit=ingrained_su,
	.update_state=ingrained_update_state,
	.inited=ingrained_inited,
	.roundend=ingrained_roundend,
	.flag=EFFECT_PASSIVE,
}};
void ingrain(struct unit *s){
	heal(s,s->max_hp/2);
	effect(ingrained,s,s,0,-1);
	setcooldown(s,s->move_cur,-1);
}
void time_space_roaring(struct unit *s){
	struct unit *t;
	struct effect *e;
	long n,n1;
	n=unit_hasnegative(s);
	if(n<=0||!(s->move_cur->mlevel&MLEVEL_CONCEPTUAL)){
		roaring_common_a(TYPE_DRAGON){
			if(*s->owner->field->stage==STAGE_PRIOR)
				effect(rebound_effect,s,s,0,1);
			else
				do_star_move(s);
		}
		roaring_common_b;
	}
	t=s->osite;
	attack(t,s,(0.6+0.2*n)*s->atk,DAMAGE_MAGICAL,AF_CRIT,TYPE_DRAGON);
	for_each_effect(e,s->owner->field->effects){
		if(e->dest!=s)
			continue;
		if((e->base->flag&EFFECT_ATTR)&&e->level<0){
			effect(e->base,t,s,e->level,-1);
			effect_reinit(e,s,2*(-e->level),-1);
		}else if(e->base->flag&(EFFECT_ABNORMAL|EFFECT_CONTROL)){
			effect(e->base,t,s,e->level,e->round);
			purify(e);
		}
	}
	unit_cooldown_decrease(s,n);
}
#define abnormal_control_p(name,_p)\
void name##_update_state(struct effect *e,struct unit *u,int *state){\
	if(e->dest==u&&*state==UNIT_NORMAL&&test(_p))\
		*state=UNIT_CONTROLLED;\
}\
const struct effect_base name[1]={{\
	.id=#name,\
	.init=abnormal_init,\
	.update_state=name##_update_state,\
	.flag=EFFECT_ABNORMAL|EFFECT_CONTROL|EFFECT_NEGATIVE,\
}}
abnormal_control_p(fear,0.5);
ava_rec(mana,25);
void kill_in_a_hit(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(mana,s,25)){
		attack(t,s,3*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
		if(isalive(t->state)){
			effect(fear,s,s,0,5);
			effect(PDB,t,s,1,-1);
			effect(MDB,t,s,1,-1);
		}
	}
	setcooldown(s,s->move_cur,6);
}
int aiming_hittest(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate){
	if(e->dest!=src)
		return -1;
	return 1;
}
const struct effect_base aiming[1]={{
	.id="aiming",
	.hittest=aiming_hittest,
	.prior=1,
	.flag=EFFECT_POSITIVE,
}};
ava_rec(youkai,650);
ava_rec(youkai,2100);
void target_lock(struct unit *s){
	if(rec_check_dec(youkai,s,650)){
		effect(aiming,s,s,0,2);
	}
}
void fire_sea(struct unit *s){
	struct unit *t=gettarget(s),*u;
	if(rec_check_dec(youkai,s,2100)&&hittest(t,s,0.15)){
		attack(t,s,4.8*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		u=rand_backend(t);
		if(u)
			attack(u,s,0.7*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		if(test(0.35))
			effect(burnt,t,s,0,5);
	}
}
void three_phase_connector(struct unit *s){
	double mm=limit(s->level,1,LONG_MAX),
	       ym=limit(3*s->max_hp,1,LONG_MAX);
	double em=limit(sqrt(mm*ym),1,LONG_MAX);
	long m=unit_effect_level(s,mana),
	       y=unit_effect_level(s,youkai),
	       e=unit_effect_level(s,electric);
	double k;
	k=(m/mm+y/ym+e/em)/3;
	m=k*mm-m;
	y=k*ym-y;
	e=k*em-e;
	effect(mana,s,s,m,-1);
	effect(youkai,s,s,y,-1);
	effect(electric,s,s,e,-1);
	setcooldown(s,s->move_cur,26);
}
void crater_roundend(struct effect *e){
	effect_ev(e){
		attack(e->dest,NULL,0.3*(e->dest->max_hp-e->dest->hp),DAMAGE_MAGICAL,AF_EFFECT|AF_WEAK,TYPE_MACHINE);
	}
}
const struct effect_base crater[1]={{
	.id="crater",
	.roundend=crater_roundend,
	.flag=EFFECT_NEGATIVE,
	.prior=-72
}};
ava_spi(91);
void disintegrate(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&spi_check_dec(s,91)){
		attack(t,s,750+15*s->atk,DAMAGE_PHYSICAL,AF_EFFECT|AF_WEAK,TYPE_MACHINE);
		effect(radiated,t,s,0,1);
	}
}
void ice_bullet(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(youkai,s,800)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
	}
}
void laser(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(mana,s,10)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
	}
}
void laser_ray(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)&&rec_check_dec(youkai,s,800)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
	}
}
ava_rec(youkai,1600);
void high_powered_hit(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.5)&&rec_check_dec(youkai,s,1600)){
		attack(t,s,1.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
		attr_clear_positive(t);
		setcooldown(s,s->move_cur,6);
	}
}
void sunfyre_roundend(struct effect *e){
	effect_ev(e){
		attack(e->dest,e->src,0.4*(e->level+1)*e->src->atk,DAMAGE_MAGICAL,0,TYPE_FIRE);
	}
}
const struct effect_base sunfyre[1]={{
	.id="sunfyre",
	.init=abnormal_init,
	.roundend=sunfyre_roundend,
	.flag=EFFECT_NEGATIVE,
}};
void alkali_sunfyre_roundend(struct effect *e){
	effect_ev(e){
		attack(e->dest,e->src,0.4*(e->level+1)*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
	}
}
const struct effect_base alkali_sunfyre[1]={{
	.id="alkali_sunfyre",
	.init=abnormal_init,
	.roundend=alkali_sunfyre_roundend,
	.flag=EFFECT_NEGATIVE,
}};
void heat_wave(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.7*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		effect(sunfyre,t,s,1,3);
	}
}
void entropy_destroyer_blade(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e;
	if(s->move_cur->type&TYPE_ALKALIFIRE){
		if(hittest(t,s,1.0)){
			attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
		}
		effect(alkali_sunfyre,t,s,1,5);
		s->move_cur->type=TYPE_ICE;
		report(s->owner->field,MSG_UPDATE,s->move_cur);
	}else {
		e=unit_findeffect(t,frost_destroying);
		if(hittest(t,s,1.0)){
			if(e)
				attack(t,s,2.5*s->atk,DAMAGE_PHYSICAL,AF_EFFECT,TYPE_ICE);
			else
				attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_ICE);
		}
		if(!e&&t->owner==s->owner->enemy)
			effect(frost_destroying,t,s,0,5);
		s->move_cur->type=TYPE_ALKALIFIRE;
		report(s->owner->field,MSG_UPDATE,s->move_cur);
	}
}
int edb_event(struct effect *e,const struct event *ev,struct unit *src,void *arg){
	struct u_dbl *ud;
	static const struct effect_base *const bases[]={burnt,frozen,frost_destroying,sunfyre,alkali_sunfyre,NULL};
	if(ev!=derate_count)
		return 0;
	if(src!=e->dest)
		return 0;
	ud=arg;
	if(ud->damage_type!=DAMAGE_PHYSICAL)
		return 0;
	if(!effectx((const void *)bases,ud->u,NULL,0,4,EFFECT_FIND))
		return 0;
	ud->d-=0.4;
	return 0;
}
const struct effect_base entropy_destroyer_blade_p[1]={{
	.id="entropy_destroyer_blade",
	.event=edb_event,
	.flag=EFFECT_PASSIVE,
}};
void entropy_destroyer_blade_init(struct unit *s){
	effect(entropy_destroyer_blade_p,s,s,0,-1);
	effect(dcounter,NULL,s,0,-1);
}
void depositing_roundend(struct effect *e){
	effect_ev(e){
		effect(youkai,e->dest,e->dest,600,-1);
	}
}
const struct effect_base depositing[1]={{
	.id="depositing",
	.roundend=depositing_roundend,
	.flag=EFFECT_PASSIVE,
}};
void deposit_init(struct unit *s){
	effect(depositing,s,s,0,-1);
}
void negative_heal(struct unit *s){
	struct unit *t=gettarget(s);
	heal(t,-0.45*s->atk);
}
void life_limit_end(struct effect *e){
	unit_setstate(e->dest,UNIT_FAILED);
}
const struct effect_base life_limit[1]={{
	.id="life_limit",
	.init=abnormal_init,
	.kill=absolutely_immortal_kill,
	.end=life_limit_end,
	.flag=EFFECT_UNPURIFIABLE,
}};
void life_and_death_register(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_GRASS);
	}
	effect(life_limit,s,s,0,10);
}
int mln_init(struct effect *e,long level,int round){
	for_each_effect(ep,effect_field(e)->effects){
		if(ep->base==blood_moon)
			purify(ep);
	}
	e->round=round;
	e->level=level;
	return 0;
}
int mln_damage(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(!(*type&(TYPE_GRASS|TYPE_LIGHT))||!damage_pm(*damage_type))
		return 0;
	if(*type&TYPE_LIGHT)
		*value*=1.35;
	if(*type&TYPE_GRASS)
		*value*=0.75;
	return 0;
}
const struct effect_base moonless_night[1]={{
	.id="moonless_night",
	.init=mln_init,
	.damage=mln_damage,
	.flag=EFFECT_ENV,
}};
void white_phosphorus_bomb(struct unit *s){
	struct unit *t;
	t=gettarget(s);
	if(hittest(t,s,1.8))
		attack(t,s,1.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
	effect(moonless_night,NULL,s,0,5);
	setcooldown(s,s->move_cur,9);
}
void cm_end(struct effect *e){
	struct unit *s=e->src;
	struct unit *t=s->osite;
	effect_ev(e){
		for(long i=limit(e->level,0,16);i;--i){
			t=rand_unit(t);
			if(!t)
				break;
			attack(t,s,0.7*s->atk+0.06*t->max_hp,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		}
	}
}
const struct effect_base cm_effect[1]={{
	.id="cluster_missile_splinter",
	.end=cm_end,
	.roundend=effect_destruct,
	.flag=EFFECT_ISOLATED,
}};
void cluster_missile(struct unit *s){
	struct unit *t;
	int n;
	t=gettarget(s);
	if(hittest(t,s,INFINITY)){
		attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		addspi(t,5);
	}
	n=4+rand()%4;
	effect(cm_effect,NULL,s,n,-1);
	setcooldown(s,s->move_cur,n+1);
}
void suppressed_update_state(struct effect *e,struct unit *u,int *state){
	if(e->dest!=u)
		return;
	switch(*state){
		case UNIT_NORMAL:
		case UNIT_CONTROLLED:
			*state=UNIT_SUPPRESSED;
		default:
			return;
	}
}
const struct effect_base suppressed[1]={{
	.id="suppressed",
	.update_state=suppressed_update_state,
	.flag=EFFECT_NONHOOKABLE|EFFECT_UNPURIFIABLE|EFFECT_CONTROL|EFFECT_NEGATIVE,
}};
const struct effect_base vanished[1]={{
	.id="vanished",
	.revive=perish_revive,
	.flag=EFFECT_ALLOWFAILED|EFFECT_NONHOOKABLE|EFFECT_KEEP|EFFECT_UNPURIFIABLE|EFFECT_NEGATIVE,
}};
void plasmatizing_lightcannon(struct unit *s){
	struct unit *t=s->osite;
	int cd;
	instant_death(t);
	if(isalive(t->state)){
		cd=70*(1.0-(double)t->hp/t->max_hp);
		effect(suppressed,t,s,0,5);
	}else {
		cd=70;
		effect(vanished,t,s,0,-1);
	}
	setcooldown(s,s->move_cur,cd+1);
}
int countunit(struct player *p){
	int r=0;
	for_each_unit(u,p){
		if(u->state!=UNIT_FREEZING_ROARINGED&&!unit_findeffect(u,vanished))
			++r;
	}
	return r;
}
int countunit_alive(struct player *p){
	int r=0;
	for_each_unit(u,p){
		if(isalive(u->state)&&!unit_findeffect(u,vanished))
			++r;
	}
	return r;
}
int countunit_failed(struct player *p){
	int r=0;
	for_each_unit(u,p){
		switch(u->state){
			case UNIT_FADING:
			case UNIT_FAILED:
				if(!unit_findeffect(u,vanished))
					++r;
			default:
				break;
		}
	}
	return r;
}
void dominate(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.5)){
		attack(t,s,1.25*(1+0.5*countunit_failed(s->owner))*s->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	}
	setcooldown(s,s->move_cur,6);
}
void globality_reset(struct unit *s){
	struct battle_field *f=s->owner->field;
	struct history *h;
	struct effect *head;
	int off=s->move_cur-s->moves;
	if(!f->ht_size)
		return;
	h=f->ht;
	back(f->p,&h->p,NULL);
	back(f->e,&h->e,NULL);
	head=f->effects;
	f->effects=effect_copyall(h->effects);
	effect_freeall(&head,f);
	report(f,MSG_UPDATE,&f->effects);
	update_attr_all(f);
	if(s->moves[off].action==globality_reset)
		setcooldown(s,s->moves+off,-1);
}
const struct effect_base monoxide[1]={{
	.id="monoxide",
	.flag=EFFECT_ADDLEVEL|EFFECT_NEGATIVE,
}};
void kf_roundend(struct effect *e){
	effect_ev(e){
		attack(e->dest,e->src,0.4*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		if(unit_effect_level(e->dest,monoxide)<16){
			effect(monoxide,e->dest,e->src,1,-1);
		}else {
			attack(e->dest,e->src,e->dest->max_hp,DAMAGE_MAGICAL,0,TYPE_POISON);
		}
	}
}
const struct effect_base karmic_fire[1]={{
	.id="karmic_fire",
	.roundend=kf_roundend,
	.flag=EFFECT_NEGATIVE,
}};
void karmic_burn(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.4*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
	}
	effect(karmic_fire,t,s,0,3);
}
int ap_purify(struct effect *e,struct effect *ep){
	if(e!=ep||e->level<=3)
		return 0;
	effect_reinit(e,e->src,-3,-1);
	return -1;
}
void ap_inited(struct effect *e){
	if(e->level>=36)
		unit_setstate(e->dest,UNIT_FAILED);
}
int ap_end_in_roundend(struct effect *e){
	if(e->level>=27)
		effect_reinit(e,e->src,-2,0);
	return -1;
}
const struct effect_base assimilation_progress[1]={{
	.id="assimilation_progress",
	.init=maple_init,
	.inited=ap_inited,
	.purify=ap_purify,
	.end_in_roundend=ap_end_in_roundend,
	.flag=EFFECT_NONHOOKABLE|EFFECT_NEGATIVE,
}};
const struct event elf_light_impact_le[1]={{
	.id="elf_light_impact_le",
}};
void dyed_amod(struct effect *e,long level){
	if(e->unused>=3)
		return;
	event_do(elf_light_impact_le,e->src){
		attack(e->dest,e->src,0.45*level*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
		effect(assimilation_progress,e->dest,e->src,level,4);
		if(!e->intrash)
			++e->unused;
	}
}
void dyed_effect_endt(struct effect *e,struct effect *ep){
	long level;
	if(e->dest!=ep->dest||!(ep->base->flag&EFFECT_ATTR))
		return;
	level=ep->level;
	if(!level)
		return;
	dyed_amod(e,labs(level));
}
void dyed_effect_end0(struct effect *e,struct effect *ep,struct unit *dest,struct unit *src,long level,int round){
	if(e->dest!=dest||!(ep->base->flag&EFFECT_ATTR))
		return;
	level=ep->level-level;
	if(!level)
		return;
	dyed_amod(e,labs(level));
}
const struct effect_base dyed[1]={{
	.id="dyed",
	.roundstart=hf_roundstart,
	.effect_end0=dyed_effect_end0,
	.effect_endt=dyed_effect_endt,
	.flag=EFFECT_NEGATIVE,
}};
void dye_bomb(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3)){
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
	}
	effect(dyed,t,s,0,5);
	setcooldown(s,s->move_cur,4);
}
void light_curtain(struct unit *s){
	attr_clear(s);
	attr_clear(s->osite);
	setcooldown(s,s->move_cur,6);
}
const struct effect_base gate[1];
void sr_switchunit_end(struct effect *e,struct unit *from){
	if(from==e->dest&&!unit_findeffect(from,gate)){
		effect_ev(e)
			purify(e);
	}
}
int sr_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	if(dest==e->dest&&damage_pm(*damage_type))
		*value*=1+(*damage_type==DAMAGE_PHYSICAL?0.03:0.02)*e->level;
	return 0;
}
const struct effect_base san_reduce[1]={{
	.id="san_reduce",
	.init=maple_init,
	.attack=sr_attack,
	.switchunit_end=sr_switchunit_end,
	.flag=EFFECT_NEGATIVE
}};
void ps_attack_end(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if(e->dest==src&&damage_pm(damage_type)&&dest->owner==e->dest->owner->enemy){
		effect_ev(e){
			effect(san_reduce,dest,src,(damage_type==DAMAGE_PHYSICAL?3:2)+!!(aflag&AF_CRIT),-1);
		}
	}
}
const struct effect_base psychic_suppress_effect[1]={{
	.id="psychic_suppress",
	.attack_end=ps_attack_end,
	.flag=EFFECT_PASSIVE,
}};
void psychic_suppress_init(struct unit *s){
	effect(psychic_suppress_effect,s,s,0,-1);
}
void nameless_mist(struct unit *s){
	struct unit *t=s->osite;
	for_each_effect(e,s->owner->field->effects){
		if(e->dest==t&&(e->base->flag&EFFECT_ATTR)&&e->level>0)
			effectx(e->base,t,s,-2*e->level,-1,EFFECT_NONHOOKABLE|EFFECT_NOCONSTRUCT|EFFECT_ADDLEVEL);
	}
	effectx(san_reduce,t,s,10,-1,EFFECT_NONHOOKABLE);
	effect(suppressed,t,s,0,5);
	setcooldown(s,s->move_cur,31);
}
long getsan(const struct unit *u){
	return 100l-unit_effect_level(u,san_reduce);
}
void resetcd(struct unit *u,void (*ma)(struct unit *)){
	for_each_move(m,u){
		if(m->cooldown&&m->action==ma){
			setcooldown(u,m,0);
		}
	}
}
void gate_key(struct unit *s);
void gate_end(struct effect *e){
	if(e->active)
		return;
	e->active=1;
	effect_ev(e){
		attack(e->dest,e->src,0.35*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
		attack(e->dest,e->src,0.35*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
		attack(e->dest,e->src,0.35*e->src->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
		if(getsan(e->dest)<15)
			instant_death(e->dest);
		if(!isalive(e->dest->state)){
			effect(vanished,e->dest,e->src,0,-1);
			resetcd(e->src,gate_key);
		}
	}
}
struct unit *gate_gettarget(struct effect *e,struct unit *u){
	if(u->owner!=e->dest->owner){
		return e->dest;
	}
	return NULL;
}
void gate_kill_end(struct effect *e,struct unit *u){
	if(u!=e->dest)
		return;
	if(e->active)
		return;
	e->active=1;
	effect_ev(e){
		effect(vanished,e->dest,e->src,0,-1);
		resetcd(e->src,gate_key);
	}
	effect_end(e);
}
const struct effect_base gate[1]={{
	.id="gate",
	.end=gate_end,
	.kill_end=gate_kill_end,
	.gettarget=gate_gettarget,
	.flag=EFFECT_UNPURIFIABLE|EFFECT_KEEP|EFFECT_NEGATIVE
}};
void gate_key(struct unit *s){
	struct unit *t=s->osite;
	if(hittest(t,s,2.3)){
		attack(t,s,1.05*s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
	}
	if(isalive(t->state)){
		effect(gate,t,s,0,4);
		setcooldown(s,s->move_cur,14);
	}
}
void ms_attack_end0(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type){
	if(e->dest==src&&dest->owner==e->dest->owner->enemy&&e->unused<2){
		effect_ev(e){
			++e->unused;
			attack(dest,src,0.45*src->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		}
	}
}
const struct effect_base missile_support_effect[1]={{
	.id="missile_support",
	.roundstart=hf_roundstart,
	.attack_end0=ms_attack_end0,
	.flag=EFFECT_POSITIVE,
	.recursion_max=1,
}};
void missile_support(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.6))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
	effect(missile_support_effect,s,s,0,8);
	setcooldown(s,s->move_cur,7);
}
int cr_attack(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type){
	_Static_assert(ATTR_MAX>0.0,"ATTR_MAX is zero or negative");
	if(src==e->dest&&damage_pm(*damage_type)&&test(e->level/(double)ATTR_MAX))
		*aflag|=AF_CRIT;
	return 0;
}
const struct effect_base CR[1]={{
	.id="CR",
	.init=attr_init,
	.attack=cr_attack,
	.flag=EFFECT_ATTR|EFFECT_KEEP,
}};
void boomerang(struct unit *s){
	struct unit *t=gettarget(s);
	struct effect *e=unit_findeffect(t,gate);
	if(hittest(t,s,1.3)){
		if(e){
			attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,AF_CRIT,TYPE_STEEL);
			effect_end(e);
		}else
			attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
	}
	heal(s,0.25*s->atk);
	effect(CR,s,s,1,-1);
	setcooldown(s,s->move_cur,4);
}
void star_elf_ray(struct unit *s){
	struct unit *t=gettarget(s);
	struct move *mp=NULL;
	int cd;
	for_each_move(m,s){
		if(!m->action||m->cooldown<=0)
			continue;
		if(!mp||m->cooldown>mp->cooldown){
			mp=m;
		}
	}
	cd=mp?mp->cooldown:0;
	if(hittest(t,s,0.9+0.1*(cd+!cd)))
		attack(t,s,s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
	if(cd){
		setcooldown(s,mp,0);
		setcooldown(s,s->move_cur,cd);
	}
}
void nano_spimod(struct effect *e,struct unit *dest,long hp){
	if(dest==e->dest){
		effect_ev(e){
			effect_reinit(e,NULL,-1,-1);
			if(e->level<=0){
				effect_end(e);
				attack(dest,dest,0.4*dest->max_hp,DAMAGE_REAL,0,TYPE_VOID);
				effect(stunned,dest,dest,0,5);
			}
		}
	}
}
int nano_init(struct effect *e,long level,int round){
	e->round=round;
	if(level>0)
		e->level=level;
	else
		e->level+=level;
	return 0;
}
const struct effect_base nano_shield[1]={{
	.id="nano_shield",
	.init=nano_init,
	.damage=protecting_damage,
	.kill=protecting_kill,
	.hpmod=protecting_hpmod,
	.spimod=nano_spimod,
	.flag=EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_NONHOOKABLE,
}};
int nano_ava(struct unit *s,struct move *m){
	return !unit_findeffect(s,nano_shield);
}
void nano_shield_action(struct unit *s){
	effect(nano_shield,s,s,18,-1);
}
const struct effect_base bloom_point[1]={{
	.flag=EFFECT_PASSIVE|EFFECT_ADDLEVEL,
}};
struct bloom_state {
	int cd_p,cd_e;
	unsigned char u_p[6],u_e[6],unused[4];
};
void bloma(const struct event *ev,struct unit *src){
	effectx(bloom_point,src,src,3,0,EFFECT_ADDLEVEL);
	event_callback(ev,src,NULL);
}
const struct event bloom[1]={{
	.id="bloom",
	.action=bloma,
}};
void try_bloom(struct unit *u);
void bloms_he(struct effect *e,struct unit *dest,struct unit *src,int hit){
	if(!hit||!isalive(src->state)||!isfront(src))
		return;
	effect_ev(e){
		try_bloom(src);
	}
}
void bloms_roundstart(struct effect *e){
	struct bloom_state *b=(void *)e->data;
	if(b->cd_p>0)
		--b->cd_p;
	if(b->cd_e>0)
		--b->cd_e;
}
const struct effect_base bloom_system[1]={{
	.roundstart=bloms_roundstart,
	.hittest_end=bloms_he,
	.flag=EFFECT_PASSIVE,
	.data_size=sizeof(struct bloom_state),
}};
struct bloom_state *getbloms(struct battle_field *bf){
	for_each_effect(e,bf->effects){
		if(e->base==bloom_system)
			return (void *)e->data;
	}
	return NULL;
}
int unit_available(struct unit *u){
	return u->state!=UNIT_FREEZING_ROARINGED&&!unit_findeffect(u,vanished);
}
const struct effect_base high_pressure[1]={{
	.id="high_pressure",
	.flag=EFFECT_ENV,
}};
void blom_me(struct effect *e,struct unit *u,struct move *m){
	if(e->dest!=u||u->owner->move_recursion+1!=e->level)
		return;
	event(bloom,u);
	effect_end(e);
}
const struct effect_base blooming[1]={{
	.move_end=blom_me,
	.flag=EFFECT_PASSIVE,
}};
void try_bloom(struct unit *u){
	struct unit *u1;
	struct player *p;
	struct battle_field *bf;
	struct bloom_state *b=getbloms(bf=(p=u->owner)->field);
	int *cd;
	unsigned char *a;
	int r;
	if(!b)
		return;
	cd=(p==bf->p?&b->cd_p:&b->cd_e);
	if(*cd)
		return;
	a=(p==bf->p?b->u_p:b->u_e);
	u1=(p==bf->p?bf->p->units:bf->e->units);
	r=0;
	for(int i=0;i<6;++i,++u1){
		if(!a[i])
			continue;
		if(!unit_available(u1))
			continue;
		r=1;
	}
	if(!r)
		return;
	if(findeffect(bf->effects,high_pressure))
		*cd=3;
	else
		*cd=4;
	effect(blooming,u,u,u->owner->move_recursion,0);
}
void unit_setbloom(struct unit *u){
	struct battle_field *bf;
	struct effect *e;
	struct player *p;
	unsigned char *a;
	struct bloom_state *b=getbloms(bf=(p=u->owner)->field);
	if(!b){
		e=effect(bloom_system,NULL,u,0,-1);
		if(!e)
			return;
		b=(void *)e->data;
	}
	a=(p==bf->p?b->u_p:b->u_e);
	a[u-p->units]=1;
}
#define ckblom {\
	struct effect *e11;\
	if(ev!=bloom||src->owner!=e->dest->owner)\
		return 0;\
	e11=unit_findeffect(src,bloom_point);\
	if(!e11)\
		return 0;\
	effect_reinit(e11,src,-1,0);\
	if(e11->level<=0)\
		return -1;\
}
int fairyland_gate_event(struct effect *e,const struct event *ev,struct unit *src,void *arg){
	ckblom;
	effect_ev(e){
		src=e->dest;
		attack(gettarget(src),src,1.4*src->atk,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	}
	return 0;
}
const struct effect_base fairyland_gate[1]={{
	.id="fairyland_gate",
	.event=fairyland_gate_event,
	.flag=EFFECT_PASSIVE,
}};
void fairyland_gate_p(struct unit *s){
	unit_setbloom(s);
	effect(fairyland_gate,s,s,0,-1);
}
int flog_event(struct effect *e,const struct event *ev,struct unit *src,void *arg){
	ckblom;
	if(unit_effect_level(src,ATK)>=4)
		return 0;
	effect_ev(e){
		src=e->dest;
		effect(ATK,src,src,1,-1);
	}
	return 0;
}
const struct effect_base flog[1]={{
	.id="flog",
	.event=flog_event,
	.flag=EFFECT_PASSIVE,
}};
void flog_p(struct unit *s){
	unit_setbloom(s);
	effect(flog,s,s,0,-1);
}
void high_pressure_watercannon(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,2.0))
		attack(t,s,1.5*s->atk,DAMAGE_PHYSICAL,0,TYPE_WATER);
	effect(high_pressure,NULL,s,0,8);
	setcooldown(s,s->move_cur,11);
}
void air_cannon(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.5))
		attack(t,s,1.6*s->atk,DAMAGE_PHYSICAL,0,TYPE_WIND);
	event(bloom,s);
	setcooldown(s,s->move_cur,6);
}
void air_force(struct unit *s){
	struct unit *t=gettarget(s);
	switch(randi()%5){
		case 0:
			if(hittest(t,s,2.0)){
				attack(t,s,2*s->atk,DAMAGE_MAGICAL,0,TYPE_POISON);
				effect(poisoned,t,s,0,5);
			}
			break;
		case 1:
			if(hittest(t,s,2.0)){
				attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
				effect(stunned,t,s,0,4);
			}
			break;
		case 2:
			if(hittest(t,s,2.0))
				attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
			effect(blood_moon,NULL,s,0,8);
			break;
		case 3:
			if(hittest(t,s,2.0))
				attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
			addspi(t,t->spi>0?-20:+20);
			break;
		case 4:
			if(hittest(t,s,2.0))
				attack(t,s,2*s->atk,DAMAGE_PHYSICAL,0,TYPE_ALKALIFIRE);
			event(real_fire_disillusion_fr,s);
			break;
	}
	setcooldown(s,s->move_cur,11);
}
void spi_gather(struct unit *s){
	addspi(s,5+randi()%11);
}
const struct effect_base reduced[1];
void reduced_inited(struct effect *e){
	for_each_unit(u,e->dest->owner){
		if(isalive_s(u->state)&&!unit_findeffect(u,reduced))
			return;
	}
	effect_ev(e)
		setwinner(effect_field(e),e->dest->owner->enemy);
}
const struct effect_base reduced[1]={{
	.id="reduced",
	.inited=reduced_inited,
	.flag=EFFECT_NONHOOKABLE|EFFECT_NEGATIVE,
}};
void red_lotus(struct unit *s){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.4*s->atk,DAMAGE_PHYSICAL,0,TYPE_FIRE);
		effect(reduced,t,s,0,5);
	}
}
void air_breaking_thorn_a(struct unit *s);
void air_breaking_thorn_effect_end(struct effect *e,struct effect *ep,struct unit *dest,struct unit *src,long level,int round){
	if(!(ep->base->flag&EFFECT_ENV)||src->owner==e->dest->owner||!isfront(e->dest))
		return;
	effect_ev(e){
		effect_setround(e,2);
		resetcd(e->dest,air_breaking_thorn_a);
	}
}
const struct effect_base air_breaking_thorn[1]={{
	.effect_end=air_breaking_thorn_effect_end,
	.end_in_roundend=kaleido_end_in_roundend,
	.flag=EFFECT_PASSIVE,
}};
long oxidate(struct unit *dest){
	struct effect *e=unit_findeffect(dest,reduced);
	if(e){
		effect_end(e);
		return damage(dest,NULL,dest->max_hp/3,DAMAGE_MAGICAL,0,TYPE_POISON);
	}
	return damage(dest,NULL,64*dest->max_hp,DAMAGE_MAGICAL,AF_IDEATH|AF_CRIT,TYPE_POISON);
}
void air_breaking_thorn_i(struct unit *s){
	effect(air_breaking_thorn,s,s,0,0);
}
void air_breaking_thorn_a(struct unit *s){
	struct unit *t;
	long dmg;
	if(unit_effect_round(s,air_breaking_thorn)){
		oxidate(t=s->osite);
		heal(s,s->max_hp/2);
		effectx(NULL,NULL,t,0,0,EFFECT_REMOVE|EFFECT_SELECTALL|EFFECT_ENV);
	}else {
		t=gettarget(s);
		if(hittest(t,s,1.5)){
			dmg=attack(t,s,1.4*s->atk,DAMAGE_PHYSICAL,AF_NODEF,TYPE_WIND);
			if(dmg>128)
				heal(s,dmg-128);
		}
	}
	setcooldown(s,s->move_cur,6);
}
//list
const struct move builtin_moves[]={
	{
		.id="steel_flywheel",
		.action=steel_flywheel,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="holylight_heavycannon",
		.action=holylight_heavycannon,
		.type=TYPE_LIGHT,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="ground_force",
		.action=ground_force,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
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
		.init=kaleido_init,
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
		.id="pursue",
		.action=pursue,
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
		.action=primordial_breath_action,
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
		//.init=freezing_roaring_init,
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
		.action=uniform_base_a,
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
		.flag=MOVE_NORMALATTACK,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="rock_break",
		.action=rock_break,
		.type=TYPE_ROCK,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="dark_night_light",
		.action=dark_night_light,
		.type=TYPE_LIGHT,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="natural_decay",
		.action=natural_decay,
		.type=TYPE_DEVINEGRASS,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="dmts_impact",
		.action=dmts_impact,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="dmts_spiblade",
		.action=dmts_spiblade,
		.type=TYPE_MACHINE,
		.flag=0,
		.available=ava_spi_5,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="dmts_pulse",
		.action=dmts_pulse,
		.type=TYPE_ELECTRIC,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="attacking_defensive_combine",
		.action=attacking_defensive_combine,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="elasticity_module",
		.init=elasticity_module_init,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="linear_blast",
		.action=linear_blast,
		.type=TYPE_GHOST,
		.prior=1,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="swagger",
		.action=swagger,
		.type=TYPE_NORMAL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="attribute_swap",
		.action=attribute_swap,
		.type=TYPE_GHOST,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="deflecting_field",
		.action=deflecting_field,
		.type=TYPE_FIGHTING,
		.prior=5,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="clay_charge",
		.action=clay_charge,
		.type=TYPE_SOIL,
		.prior=1,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="karma_reverse",
		.action=karma_reverse,
		.type=TYPE_DRAGON,
		.prior=5,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="anchor_bomb",
		.action=anchor_bomb,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="time_solidify",
		.action=time_solidify,
		.type=TYPE_DRAGON,
		.prior=5,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="squeeze",
		.action=squeeze,
		.type=TYPE_NORMAL,
		.prior=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="accumulate",
		.action=accumulate,
		.type=TYPE_NORMAL,
		.prior=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="voltage_transformation",
		.action=voltage_transformation,
		.init=voltage_transformation_init,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="fire_ball",
		.action=fire_ball,
		.type=TYPE_FIRE,
		.flag=0,
		.available=ava_mana_10,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="ice_ball",
		.action=ice_ball,
		.type=TYPE_ICE,
		.flag=0,
		.available=ava_mana_10,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="water_ball",
		.action=water_ball,
		.type=TYPE_WATER,
		.flag=0,
		.available=ava_mana_10,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="lighting_bolt_ball",
		.action=lighting_bolt_ball,
		.type=TYPE_ELECTRIC,
		.flag=0,
		.available=ava_mana_10,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="three_phase_drive",
		.action=three_phase_drive,
		.init=three_phase_drive_init,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="celestial_phenomena",
		.action=celestial_phenomena,
		.type=TYPE_ROCK,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="pulse",
		.action=pulse,
		.type=TYPE_ELECTRIC,
		.flag=0,
		.available=ava_electric_90,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="electric_shield",
		.action=electric_shield,
		.type=TYPE_ELECTRIC,
		.flag=0,
		.available=ava_electric_220,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="wind_blade",
		.action=wind_blade,
		.type=TYPE_WIND,
		.flag=0,
		.available=ava_youkai_800,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="protect",
		.action=protect,
		.type=TYPE_NORMAL,
		.flag=0,
		.prior=3,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="ingrain",
		.action=ingrain,
		.type=TYPE_GRASS,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="time_space_roaring",
		.action=time_space_roaring,
		.type=TYPE_DRAGON,
		.flag=0,
		.mlevel=MLEVEL_REGULAR|MLEVEL_CONCEPTUAL
	},
	{
		.id="kill_in_a_hit",
		.action=kill_in_a_hit,
		.type=TYPE_NORMAL,
		.flag=0,
		.available=ava_mana_25,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="target_lock",
		.action=target_lock,
		.type=TYPE_NORMAL,
		.flag=0,
		.available=ava_youkai_650,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="fire_sea",
		.action=fire_sea,
		.type=TYPE_FIRE,
		.flag=0,
		.available=ava_youkai_2100,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="three_phase_connector",
		.action=three_phase_connector,
		//.init=three_phase_connector_init,
		.type=TYPE_MACHINE,
		.flag=0,
		.cooldown=15,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="disintegrate",
		.action=disintegrate,
		.type=TYPE_MACHINE,
		.flag=0,
		.available=ava_spi_91,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="ice_bullet",
		.action=ice_bullet,
		.type=TYPE_ICE,
		.flag=0,
		.available=ava_youkai_800,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="laser",
		.action=laser,
		.type=TYPE_LIGHT,
		.flag=0,
		.available=ava_mana_10,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="laser_ray",
		.action=laser_ray,
		.type=TYPE_LIGHT,
		.flag=0,
		.available=ava_youkai_800,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="high_powered_hit",
		.action=high_powered_hit,
		.type=TYPE_LIGHT,
		.flag=0,
		.available=ava_youkai_1600,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="heat_wave",
		.action=heat_wave,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="entropy_destroyer_blade",
		.action=entropy_destroyer_blade,
		.init=entropy_destroyer_blade_init,
		.type=TYPE_ICE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="deposit",
		.init=deposit_init,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="negative_heal",
		.action=negative_heal,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="life_and_death_register",
		.action=life_and_death_register,
		.type=TYPE_GRASS,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="white_phosphorus_bomb",
		.action=white_phosphorus_bomb,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="cluster_missile",
		.action=cluster_missile,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="plasmatizing_lightcannon",
		.action=plasmatizing_lightcannon,
		.type=TYPE_LIGHT,
		.flag=0,
		.cooldown=40,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="dominate",
		.action=dominate,
		.type=TYPE_NORMAL,
		.prior=1,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="globality_reset",
		.action=globality_reset,
		.type=TYPE_DRAGON,
		.prior=5,
		.flag=MOVE_NOCONTROL,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="karmic_burn",
		.action=karmic_burn,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="dye_bomb",
		.action=dye_bomb,
		.type=TYPE_LIGHT,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="light_curtain",
		.action=light_curtain,
		.type=TYPE_LIGHT,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="psychic_suppress",
		.init=psychic_suppress_init,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="nameless_mist",
		.action=nameless_mist,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="gate_key",
		.action=gate_key,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_CONCEPTUAL
	},
	{
		.id="missile_support",
		.action=missile_support,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="boomerang",
		.action=boomerang,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="star_elf_ray",
		.action=star_elf_ray,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="nano_shield",
		.action=nano_shield_action,
		.available=nano_ava,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="fairyland_gate",
		.init=fairyland_gate_p,
		.type=TYPE_NORMAL,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="flog",
		.init=flog_p,
		.type=TYPE_FIGHTING,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="high_pressure_watercannon",
		.action=high_pressure_watercannon,
		.type=TYPE_WATER,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="air_cannon",
		.action=air_cannon,
		.type=TYPE_WIND,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="air_force",
		.action=air_force,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spi_gather",
		.action=spi_gather,
		.type=TYPE_MACHINE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="red_lotus",
		.action=red_lotus,
		.type=TYPE_FIRE,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="air_breaking_thorn",
		.action=air_breaking_thorn_a,
		.init=air_breaking_thorn_i,
		.type=TYPE_WIND,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{.id=NULL}
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
const struct effect_base *effects[]={
cursed,
radiated,
poisoned,
burnt,
parasitized,
frozen,
asleep,
paralysed,
stunned,
petrified,
aurora,
ATK,
DEF,
SPEED,
HIT,
AVOID,
CE,
PDB,
MDB,
PDD,
MDD,
natural_shield_effect,
alkali_fire_seal,
metal_bomb_effect,
frost_destroying,
primordial_breath,
thorns,
combo,
hitback,
blood_moon,
hot,
wet,
cold,
black_hole,
gravitational_potential,
cycle_eden,
absolutely_immortal_effect,
confused,
heat_engine,
force_vh_effect,
repeat_effect,
silent,
disarmed,
mecha_effect,
perish_song_effect,
marsh,
interfered,
hail_effect,
maple,
shield,
heal_weak,
heal_bonus,
mana,
moon_elf_shield,
moon_elf_shield_cooldown,
anti_def_by_def,
burn_boat_effect,
uniform_base_effect,
spatially_shatter_effect,
natural_decay_effect,
high_frequency,
elasticity_module,
anchor_bomb_effect,
time_frozen,
accumulated,
voltage_transforming,
youkai,
electric,
protecting,
ingrained,
fear,
aiming,
crater,
sunfyre,
alkali_sunfyre,
depositing,
life_limit,
moonless_night,
cm_effect,
suppressed,
vanished,
monoxide,
karmic_fire,
assimilation_progress,
dyed,
san_reduce,
psychic_suppress_effect,
gate,
missile_support_effect,
CR,
nano_shield,
fairyland_gate,
flog,
high_pressure,
entropy_destroyer_blade_p,
reduced,
NULL};
const size_t effects_size=sizeof(effects)/sizeof(effects[0])-1;
