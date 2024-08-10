#include "battle-core.h"
#include <stddef.h>
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
const struct move builtin_moves[]={
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
	{NULL}
};
