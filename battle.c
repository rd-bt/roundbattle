#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int canaction2(struct player *p,int act);
const struct move *get_builtin_move_by_id(const char *id){
	unsigned long i;
	for(i=0;builtin_moves[i].id;++i){
		if(!strcmp(id,builtin_moves[i].id))
			return builtin_moves+i;
	}
	return NULL;
}
int rand_selector(struct player *p){
	int x;
redo:
	x=lrand48()%9;
	if(x==8)
		return 8;
	if(p->front->moves[x].id==NULL)
		goto redo;
	return x;
}
int manual_selector(struct player *p){
	char buf[32],*endp;
	int r,n=0,cool;
reselect:
	printf("Select the action of %s\n",p->front->base.name);
	if(!canaction2(p,ACT_NORMALATTACK))
		printf("[x]");
	else
		printf("[ ]");
	printf("a: Normal attack (%s)\n",type2str(p->front->type0));
	for(r=0;r<8;++r){
		if(!p->front->moves[r].id)
			break;
		if(!canaction2(p,r))
			printf("[x]");
		else
			printf("[ ]");
		printf("%d: %s (%s)",r,p->front->moves[r].name,type2str(p->front->moves[r].type));
		cool=p->front->moves[r].cooldown;
		if(cool)
			printf(" cooldown rounds:%d\n",cool);
		else printf("\n");
		++n;
	}
	printf("q: Abort the action\n");
	printf("g: Give up\n");
	printf(">");
	fgets(buf,32,stdin);
	endp=strchr(buf,'\n');
	if(endp)*endp=0;
	switch(*buf){
		case 'Q':
		case 'q':
			if(buf[1]!=0)
				goto unknown;
			r=ACT_ABORT;
			break;
		case 'G':
		case 'g':
			if(buf[1]!=0)
				goto unknown;
			r=ACT_GIVEUP;
			break;
		case 'A':
		case 'a':
			if(buf[1]!=0)
				goto unknown;
			r=ACT_NORMALATTACK;
			break;
		case '0' ... '9':
			r=strtol(buf,&endp,10);
			if(r>=n||*endp)goto unknown;
			break;
		default:
unknown:
			printf("Unkonwn action:%s\n",buf);
			goto reselect;
	}
	if(!canaction2(p,r)){
		printf("The action %s is unavailable now.\n",buf);
		goto reselect;
	}
	return r;
}
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
	u->move_cur=NULL;
}
void player_fillattr(struct player *p){
	for(int i=0;i<6;++i){
		if(!p->units[i].base.name)
			break;
		unit_fillattr(p->units+i);
		p->units[i].owner=p;
	}
}
void player_action(struct player *p){
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			p->front->move_cur=p->front->moves+p->action;
			printf("%s uses %s (%s)\n",p->front->base.name,p->front->move_cur->name,type2str(p->front->move_cur->type));
			p->front->move_cur->action(p->front,0);
			return;
		case ACT_NORMALATTACK:
			p->front->move_cur=NULL;
			printf("%s uses Normal attack (%s)\n",p->front->base.name,type2str(p->front->type0));
			normal_attack(gettarget(p->front),p->front);
			return;
		default:
			p->front->move_cur=NULL;
			return;
	}
}
int getprior(struct player *p){
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			return p->front->moves[p->action].prior;
		case ACT_NORMALATTACK:
		default:
			return 0;
	}
}
int canaction2(struct player *p,int act){
	struct move *m;
	switch(act){
		case ACT_MOVE0 ... ACT_MOVE7:
			m=p->front->moves+act;
			if(m->cooldown)
				return 0;
			switch(p->front->state){
				case UNIT_CONTROLLED:
					if(m->flag&MF_NOCONTROL)
						return 1;
				case UNIT_SUPPRESSED:
					return 0;
				default:
					return 1;
			}
		case ACT_NORMALATTACK:
			switch(p->front->state){
				case UNIT_CONTROLLED:
				case UNIT_SUPPRESSED:
					return 0;
				default:
					return 1;
			}
		case ACT_UNIT0 ... ACT_UNIT5:
			switch(p->front->state){
				case UNIT_CONTROLLED:
				case UNIT_SUPPRESSED:
					return 0;
				default:
					return 1;
			}
		default:
		case ACT_GIVEUP:
		case ACT_ABORT:
			return 1;
	}
}

int canaction(struct player *p){
	return canaction2(p,p->action);
}

void state_correct(struct player *p){
	int r;
	for(r=0;r<6;++r){
		if(!p->units[r].base.name)
			break;
		unit_state_correct(p->units+r);
	}
}
void cooldown_decrease(struct player *p){
	int r;
	for(r=0;r<6;++r){
		if(!p->units[r].base.name)
			break;
		unit_cooldown_decrease(p->units+r,1);
	}
}
#define deadcheck do {\
	r0=!isalive(p->front->state);\
	r1=!isalive(e->front->state);\
	if(r0||r1){\
		if(r0&&r1){\
			if(p->front->speed==e->front->speed)\
				return test(0.5);\
			return p->front->speed<e->front->speed?\
				1:0;\
		}\
		if(r0)\
			return 1;\
		else\
			return 0;\
	}\
}while(0)
int battle(struct player *p){
	struct player *e=p->enemy,*prior,*latter;
	int round,r0,r1;
	if(e->enemy!=p)
		return -1;
	if(!p->units->base.name||!e->units->base.name)
		return -2;
	player_fillattr(p);
	player_fillattr(e);
	p->front=p->units;
	e->front=e->units;
	for(round=0;;++round){
		printf("\nROUND %d %s:%lu/%lu(%.2lf%%) %s:%lu/%lu(%.2lf%%)\n",round,
			p->front->base.name,
			p->front->hp,
			p->front->base.max_hp,
			100.0*p->front->hp/p->front->base.max_hp,
			e->front->base.name,
			e->front->hp,
			e->front->base.max_hp,
			100.0*e->front->hp/e->front->base.max_hp
			);
		if(p->front->speed>e->front->speed)
			prior=p;
		else if(p->front->speed<e->front->speed)
			prior=e;
		else
			prior=test(0.5)?p:e;
		latter=prior->enemy;
		state_correct(prior);
		state_correct(latter);
		prior->action=prior->selector(prior);
		if((unsigned int)prior->action>=ACT_GIVEUP){
			return prior==p?1:0;
		}
		latter->action=latter->selector(latter);
		if((unsigned int)latter->action>=ACT_GIVEUP){
			return latter==p?1:0;
		}
		r0=getprior(prior);
		r1=getprior(latter);
		if(r0<r1){
			prior=prior->enemy;
			latter=latter->enemy;
		}
		if(canaction(prior)){
			if(prior->action==ACT_ABORT)
				printf("%s aborted the action\n",prior->front->base.name);
			else {
				printf("%s actions\n",prior->front->base.name);
				player_action(prior);
				deadcheck;
			}
		}
		state_correct(latter);
		if(canaction(latter)){
			if(latter->action==ACT_ABORT)
				printf("%s aborted the action\n",latter->front->base.name);
			else {
				printf("%s actions\n",latter->front->base.name);
				player_action(latter);
				deadcheck;
			}
		}
		printf("ROUND END\n");
		unit_effect_in_roundend(prior->front);
		unit_effect_in_roundend(latter->front);
		deadcheck;
		unit_effect_round_decrease(prior->front,1);
		unit_effect_round_decrease(latter->front,1);
		cooldown_decrease(prior);
		cooldown_decrease(latter);
	}
}
