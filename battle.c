#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void unit_fillattr(struct unit *u){
	u->hp=u->base.max_hp;
	u->atk=u->base.atk;
	u->def=u->base.def;
	u->speed=u->base.speed;
	u->hit=u->base.hit;
	u->avoid=u->base.avoid;
	u->spi=0;
	u->cirt_effect=u->base.cirt_effect;
	u->physical_bonus=u->base.physical_bonus;
	u->magical_bonus=u->base.magical_bonus;
	u->physical_derate=u->base.physical_derate;
	u->magical_derate=u->base.magical_derate;
	memset(&u->strengths,0,sizeof(struct strength));
	memset(&u->abnormals,0,sizeof(struct abnormal));
	//u->effects=NULL;
	u->type0=u->base.type0;
	u->type1=u->base.type1;
	u->state=UNIT_NORMAL;
	u->unused=0;
	memcpy(u->moves,u->base.moves,8*sizeof(struct move));
	memcpy(u->pmoves,u->base.pmoves,2*sizeof(struct move));
}
void player_fillattr(struct player *p){
	for(int i=0;i<6;++i){
		if(!p->units[i].base.name)
			break;
		unit_fillattr(p->units+i);
		p->units[i].owner=p;
	}
}
int player_action(struct player *p,int act){
	switch(act){
		case ACT_MOVE0 ... ACT_MOVE7:
			p->front->current_move=p->front->moves+act;
			printf("%s uses %s\n",p->front->base.name,p->front->current_move->name);
			p->front->current_move->action(p->front,0);
			break;
		case ACT_NORMALATTACK:
			printf("%s uses normal attack\n",p->front->base.name);
			normal_attack(gettarget(p->front),p->front);
			break;
		default:
		case ACT_GIVEUP:
			return 1;
	}
	return 0;
}
#define deadcheck do {\
	r0=!isalive(p->front->state);\
	r1=!isalive(e->front->state);\
	if(r0||r1){\
		if(r0&&r1)\
			return p->front->speed<e->front->speed?\
				1:0;\
		if(r0)\
			return 1;\
		else\
			return 0;\
	}\
}while(0)
int battle(struct player *p){
	struct player *e=p->enemy,*prior,*latter;
	int round,r0,r1,a0,a1;
	if(e->enemy!=p)
		return -1;
	if(!p->units->base.name||!e->units->base.name)
		return -2;
	player_fillattr(p);
	player_fillattr(e);
	p->front=p->units;
	e->front=e->units;
	for(round=0;;++round){
		printf("\nROUND %d %s:%lu/%lu %s:%lu/%lu\n",round,p->front->base.name,p->front->hp,p->front->base.max_hp,e->front->base.name,e->front->hp,e->front->base.max_hp);
		if(p->front->speed>e->front->speed)
			prior=p;
		else if(p->front->speed<e->front->speed)
			prior=e;
		else
			prior=test(0.5)?p:e;
		latter=prior->enemy;
		a0=prior->selector(prior);
		a1=latter->selector(latter);
		printf("%s actions\n",prior->front->base.name);
		if(player_action(prior,a0))
			return prior==p?1:0;
		deadcheck;
		printf("%s actions\n",latter->front->base.name);
		if(player_action(latter,a1))
			return latter==p?1:0;
		deadcheck;
	}
}
