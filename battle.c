/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
int rand_selector(const struct player *p){
	int n=0,c=0;
	//if(canaction2(p,0))
	//return ACT_NORMALATTACK;
	//return 7;
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
void player_fillattr(struct player *p){
	for(int i=0;i<6;++i){
		if(!p->units[i].base)
			break;
		p->units[i].owner=p;
		unit_fillattr(p->units+i);
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
void player_moveinit(struct player *p){
	for_each_unit(u,p){
		for(int i=0;i<8;++i){
			if(!u->moves[i].id||!u->moves[i].init)
				continue;
			unit_move_init(u,u->moves+i);
		}
	}
}
int player_selectunit(struct player *p){
	int r;
	if(!player_hasunit(p))
		return -1;
	r=p->selector(p);
	switch(r){
		case ACT_UNIT0 ... ACT_UNIT5:
			if(!canaction2(p,r))
				return -1;
			r-=ACT_UNIT0;
			return switchunit(p->units+r);
		default:
			return -1;
	}
}
#define deadcheck do {\
	const struct player *w;\
	int s,ns;\
	ns=0;\
	do {\
		s=0;\
		if(!isalive_s(p->front->state)){\
			if(!player_selectunit(p))\
				s=1;\
			else\
				ns=1;\
		}\
		if(!isalive_s(e->front->state)){\
			if(!player_selectunit(e))\
				s=1;\
			else\
				ns=1;\
		}\
	}while(s);\
	w=(ns?getwinner_nonnull:getwinner)(&field);\
	if(w){\
		ret=(w==p?0:1);\
		goto out;\
	}\
}while(0)

int battle(struct player *p,struct player *e,struct battle_field *bf,void (*init)(struct battle_field *)){
	struct player *prior,*latter;
	struct battle_field field;
	int round,ret,stage;
	if(p==e)
		return -1;
	if(!p->units->base||!e->units->base){
		return -2;
	}
	round=0;
	stage=STAGE_INIT;
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
	p->front=p->units;
	e->front=e->units;
	player_fillattr(p);
	player_fillattr(e);
	prior=getprior(p,e);
	if(init)
		init(&field);
	player_moveinit(prior);
	player_moveinit(prior->enemy);
	for(;;++round){
		if(round>=ROUND_MAX){
			ret=getwinner_nonnull(&field)==p?0:1;
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
		stage=STAGE_SELECT;
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
		stage=STAGE_LATTER;
		if(!canaction(latter))
			latter->action=ACT_ABORT;
		player_action(latter);
		deadcheck;
		stage=STAGE_ROUNDEND;
		report(&field,MSG_ROUNDEND);
		reap_fading(&field);
		deadcheck;
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
	if(bf){
		field.end_round=round;
		memcpy(bf,&field,sizeof(struct battle_field));
	}else
		field_free(&field);
	return ret;
}
