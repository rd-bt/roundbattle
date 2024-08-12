#include "battle-core.h"
#include <stddef.h>
void steel_flywheel(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.2)){
		attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_STEEL);
		unit_abnormal(t,ABNORMAL_CURSED,3);
	}
	setcooldown(s->move_cur,3);
}
void holylight_heavycannon(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.2)){
		attack(t,s,1.25*s->atk,DAMAGE_PHYSICAL,0,TYPE_LIGHT);
		unit_abnormal(t,ABNORMAL_RADIATED,3);
	}
	setcooldown(s->move_cur,3);
}
void ground_force(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,40,DAMAGE_REAL,0,TYPE_SOIL);
}
void spoony_spell(struct unit *s,int arg){
	struct unit *t=s->owner->enemy->front;
	unsigned long dmg=2.1*s->atk+0.3*s->base.max_hp;
	attack(t,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
	attack(s,s,dmg,DAMAGE_REAL,0,TYPE_SOIL);
}
void self_explode(struct unit *s,int arg){
	struct unit *t=s->owner->enemy->front;
	unsigned long dmg=1.8*s->atk+0.25*s->base.max_hp;
	attack(t,s,dmg,DAMAGE_PHYSICAL,0,TYPE_NORMAL);
	attack(t,s,dmg,DAMAGE_MAGICAL,0,TYPE_NORMAL);
	sethp(s,0);
}
void health_exchange(struct unit *s,int arg){
	struct unit *t=s->owner->enemy->front;
	unsigned long a=s->hp,b=t->hp;
	sethp(t,a);
	sethp(s,b);
}
void urgently_repair(struct unit *s,int arg){
	heal(s,s->base.max_hp/2);
	setcooldown(s->move_cur,4);
}
void double_slash(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		attack(t,s,0.8*s->atk,DAMAGE_PHYSICAL,t->hp==t->base.max_hp?AF_CIRT:0,TYPE_WIND);
	if(s->move_cur&&(s->move_cur->mlevel&MLEVEL_CONCEPTUAL))
		addhp(s->owner->enemy->front,-0.8*s->def);
}
void petrifying_ray(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3))
		unit_abnormal(t,ABNORMAL_PETRIFIED,3);
	setcooldown(s->move_cur,4);
}
void leech_seed(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.3))
		unit_abnormal(t,ABNORMAL_PARASITIZED,5);
	setcooldown(s->move_cur,4);
}
void soften(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0))
		unit_attr_set(t,ATTR_ATK,t->attrs.atk-1);
}
void iron_wall(struct unit *s,int arg){
	unit_attr_set(s,ATTR_PDERATE,s->attrs.physical_derate+1);
	unit_attr_set(s,ATTR_MDERATE,s->attrs.magical_derate+1);
}
void spi_blow(struct unit *s,int arg){
	struct unit *t=gettarget(s);
	if(hittest(t,s,1.0)){
		attack(t,s,0.75*s->atk,DAMAGE_PHYSICAL,0,TYPE_MACHINE);
		setspi(t,t->spi+0.02*t->atk);
	}
}
const struct move builtin_moves[]={
	{
		.id="steel_flywheel",
		.name="Steel flywheel",
		.action=steel_flywheel,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="holylight_heavycannon",
		.name="Holylight heavycannon",
		.action=holylight_heavycannon,
		.type=TYPE_LIGHT,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="ground_force",
		.name="Ground force",
		.action=ground_force,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="spoony_spell",
		.name="Spoony spell",
		.action=spoony_spell,
		.type=TYPE_SOIL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="self_explode",
		.name="Self explode",
		.action=self_explode,
		.type=TYPE_NORMAL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="health_exchange",
		.name="Health exchange",
		.action=health_exchange,
		.type=TYPE_GHOST,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="urgently_repair",
		.name="Urgently repair",
		.action=urgently_repair,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="double_slash",
		.name="Double slash",
		.action=double_slash,
		.type=TYPE_WIND,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR|MLEVEL_CONCEPTUAL
	},
	{
		.id="petrifying_ray",
		.name="Petrifying ray",
		.action=petrifying_ray,
		.type=TYPE_ROCK,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="leech_seed",
		.name="Leech_seed",
		.action=leech_seed,
		.type=TYPE_GRASS,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="soften",
		.name="Soften",
		.action=soften,
		.type=TYPE_NORMAL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="iron_wall",
		.name="Iron wall",
		.action=iron_wall,
		.type=TYPE_STEEL,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR,
	},
	{
		.id="spi_blow",
		.name="Spi blow",
		.action=spi_blow,
		.type=TYPE_MACHINE,
		.prior=0,
		.flag=0,
		.mlevel=MLEVEL_REGULAR
	},
	{NULL}
};
