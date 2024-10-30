#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
int rand_selector(const struct player *p){
	int n=0,c=0;
	//if(canaction2(p,0))
	//return 0;
	//if(*p->field->round==1)return 7;
	if(!isalive(p->front->state)){
		/*for(int i=ACT_UNIT0;i<=ACT_UNIT5;++i)
			if(canaction2(p,i)){
				c|=1<<i;
				++n;
			}
		if(!n)
			return ACT_GIVEUP;

		n=randi()%n;
		for(int i=ACT_UNIT0;i<=ACT_UNIT5;++i){
			if((1<<i)&c){
				if(!n)
					return i;
				--n;
			}
		}*/
		for(int i=ACT_UNIT0;i<=ACT_UNIT5;++i)
			if(canaction2(p,i))
				return i;
		return ACT_GIVEUP;
	}
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
#define printf (use report() instead.)
void unit_fillattr(struct unit *u){
	u->hp=u->base->max_hp;
	u->atk=u->base->atk;
	u->def=u->base->def;
	u->speed=u->base->speed;
	u->hit=u->base->hit;
	u->avoid=u->base->avoid;
	u->spi=0;
	u->crit_effect=u->base->crit_effect;
	u->physical_bonus=u->base->physical_bonus;
	u->magical_bonus=u->base->magical_bonus;
	u->physical_derate=u->base->physical_derate;
	u->magical_derate=u->base->magical_derate;
	u->type0=u->base->type0;
	u->type1=u->base->type1;
	u->state=UNIT_NORMAL;
	u->level=u->base->level;
	u->blockade=0;
	memcpy(u->moves,u->base->moves,8*sizeof(struct move));
	u->move_cur=NULL;
}
void player_fillattr(struct player *p){
	for(int i=0;i<6;++i){
		if(!p->units[i].base)
			break;
		unit_fillattr(p->units+i);
		p->units[i].owner=p;
	}
}

int canaction(struct player *p){
	return canaction2(p,p->action);
}

void player_update_state(struct player *p){
	int r;
	for(r=0;r<6;++r){
		if(!p->units[r].base)
			break;
		update_state(p->units+r);
	}
}
void cooldown_decrease(struct player *p){
	int r;
	for(r=0;r<6;++r){
		if(!p->units[r].base)
			break;
		unit_cooldown_decrease(p->units+r,1);
	}
}
int player_hasunit(struct player *p){
	for(int i=0;i<6;++i){
		if(!p->units[i].base)
			break;
		if(isalive(p->units[i].state)){
			return 1;
		}
	}
	return 0;
}
int player_selectunit(struct player *p){
	int r=0;
	r=p->selector(p);
	switch(r){
		case ACT_UNIT0 ... ACT_UNIT5:
			if(!canaction2(p,r))
				return -1;
			r-=ACT_UNIT0;
			switchunit(p->units+r);
			return 0;
		default:
			return -1;
	}
}
void player_moveinit(struct player *p){
	for_each_unit(u,p){
		for(int i=0;i<8;++i){
			if(!u->moves[i].id||!u->moves[i].init)
				continue;
			unit_move_init(u,u->moves+i);
		}
	}
}
#define deadcheck do {\
	int r0,r1,r2,r3;\
	r0=!isalive(p->front->state);\
	r1=!isalive(e->front->state);\
	r2=r0?!player_hasunit(p):0;\
	r3=r1?!player_hasunit(e):0;\
	if(r2!=r3){\
		ret=!r3;\
		goto out;\
	}\
	if(r2){\
		if(p->front->speed==e->front->speed)\
			ret=test(0.5);\
		else\
			ret=p->front->speed<e->front->speed;\
		goto out;\
	}\
	if(r0){\
		r0=player_selectunit(p);\
	}\
	if(r1){\
		r1=player_selectunit(e);\
	}\
	if(r0||r1){\
		if(r0&&r1){\
			if(p->front->speed==e->front->speed)\
				ret=test(0.5);\
			else\
				ret=p->front->speed<e->front->speed;\
			goto out;\
		}\
		if(r0){\
			ret=1;\
			goto out;\
		}\
		else{\
			ret=0;\
			goto out;\
		}\
	}\
}while(0)
int battle(struct player *p,struct player *e){
	struct player *prior,*latter;
	struct battle_field field;
	int round=0,ret,stage=STAGE_INIT;
	if(p==e)
		return -1;
	if(!p->units->base||!e->units->base){
		return -2;
	}
	e->enemy=p;
	p->enemy=e;
	field.p=p;
	field.e=e;
	field.effects=NULL;
	field.trash=NULL;
	field.round=&round;
	field.stage=&stage;
	field.rec=NULL;
	field.rec_size=0;
	field.rec_length=0;
	field.ht=NULL;
	field.ht_size=0;
	field.ht_length=0;
	p->field=&field;
	e->field=&field;
	player_fillattr(p);
	player_fillattr(e);
	p->front=p->units;
	e->front=e->units;
	prior=getprior(p,e);
	player_moveinit(prior);
	player_moveinit(prior->enemy);
	for(;;++round){
		if(round>=INT_MAX){
			if(p->front->speed==e->front->speed)
				ret=test(0.5);
			else
				ret=p->front->speed<e->front->speed;
			goto out;
		}
		stage=STAGE_ROUNDSTART;
		p->action=ACT_ABORT;
		p->acted=0;
		e->action=ACT_ABORT;
		e->acted=0;
		report(&field,MSG_ROUND);
		history_add(&field);
		effect_in_roundstart(field.effects);
		deadcheck;
		if(p->front->speed>e->front->speed)
			prior=p;
		else if(p->front->speed<e->front->speed)
			prior=e;
		else
			prior=test(0.5)?p:e;
		latter=prior->enemy;
		player_update_state(prior);
		player_update_state(latter);
		prior->action=prior->selector(prior);
		if((unsigned int)prior->action>=ACT_GIVEUP){
			ret=prior==p?1:0;
			goto out;
		}
		if(!canaction(prior))
			prior->action=ACT_ABORT;
		latter->action=latter->selector(latter);
		if((unsigned int)latter->action>=ACT_GIVEUP){
			ret=latter==p?1:0;
			goto out;
		}
		if(!canaction(latter))
			latter->action=ACT_ABORT;
		latter=(prior=getprior(p,e))->enemy;
		stage=STAGE_PRIOR;
		player_action(prior);
		deadcheck;
		player_update_state(prior);
		player_update_state(latter);
		if(!canaction(latter))
			latter->action=ACT_ABORT;
		stage=STAGE_LATTER;
		player_action(latter);
		deadcheck;
		stage=STAGE_ROUNDEND;
		report(&field,MSG_ROUNDEND);
		effect_in_roundend(field.effects);
		deadcheck;
		cooldown_decrease(prior);
		deadcheck;
		cooldown_decrease(latter);
		deadcheck;
		effect_round_decrease(field.effects,1);
		deadcheck;
	}
out:
	stage=STAGE_BATTLE_END;
	report(&field,MSG_BATTLE_END,ret?e:p);
	for_each_effect(ep,field.effects){
		effect_final(ep);
	}
	wipetrash(&field);
	if(field.rec){
//		for(size_t i=0;i<field.rec_size;++i)
//			fprintf(stderr,"record:%d\n",field.rec[i].type);
		free(field.rec);
	}
	if(field.ht){
		free(field.ht);
	}
	return ret;
}
