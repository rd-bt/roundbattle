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
const struct move builtin_moves[]={
	{
		.id="ground_force",
		.name="Ground force",
		.action=ground_force,
		.type=TYPE_SOIL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="spoony_spell",
		.name="Spoony spell",
		.action=spoony_spell,
		.type=TYPE_SOIL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="self_explode",
		.name="Self explode",
		.action=self_explode,
		.type=TYPE_NORMAL,
		.mlevel=MLEVEL_REGULAR
	},
	{
		.id="health_exchange",
		.name="Health exchange",
		.action=health_exchange,
		.type=TYPE_GHOST,
		.mlevel=MLEVEL_REGULAR
	},
	{NULL}
};
