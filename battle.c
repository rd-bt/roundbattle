#include "battle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *damage_type_string[3]={"real","physical","magical"};
static const char *types_string[21]={"Void type","Grass","Fire","Water","Steel","Light","Fighting","Wind","Poison","Rock","Electric","Ghost","Ice","Bug","Machine","Soil","Dragon","Normal","Devine grass","Alkali fire","Devine water"};
const char *type2str(int type){
	unsigned int index=type?__builtin_ctz(type)+1:0;
	if(index>=21)
		return "Unknown";
	return types_string[index];
}
int canaction2(struct player *p,int act);
void reporter_default(const struct message *msg){
	const struct player *p,*e;
	/*for_each_effect(ep,msg->field->effects){
		printf("CURRENT %s\n",ep->base->id);
	}

		printf("CURRENT END\n");*/
	switch(msg->type){
		case MSG_ACTION:
			break;
		case MSG_BATTLE_END:
			printf("battle end %s wins.\n",msg->un.p->front->base->id);
			break;
		case MSG_DAMAGE:
			if(msg->un.damage.damage_type==DAMAGE_TOTAL){
				printf("%s get %lu total damage",msg->un.damage.dest->base->id,msg->un.damage.value);
				break;
			}
			printf("%s get %lu %s damage (%s",msg->un.damage.dest->base->id,msg->un.damage.value,damage_type_string[msg->un.damage.damage_type],type2str(msg->un.damage.type));
			if(msg->un.damage.aflag&AF_CRIT)
				printf(",crit");
			if(msg->un.damage.aflag&AF_EFFECT)
				printf(",effect");
			if(msg->un.damage.aflag&AF_WEAK)
				printf(",weak");
			printf(")");
			if(msg->un.damage.src)
				printf(" from %s",msg->un.damage.src->base->id);
			printf(",current hp:%lu\n",msg->un.damage.dest->hp);
			break;
		case MSG_EFFECT:
			if(msg->un.e->dest)
				printf("%s get ",msg->un.e->dest->base->id);
			printf("effect %s",msg->un.e->base->id);
			if(msg->un.e->level)
				printf("(%+ld)",msg->un.e->level);
			printf(" ");
			if(msg->un.e->src)
				printf("from %s ",msg->un.e->src->base->id);
			printf("in ");
			if(msg->un.e->round<0)
				printf("inf ");
			else
				printf("%d ",msg->un.e->round);
			printf("rounds\n");
			break;
		case MSG_EFFECT_END:
			printf("effect %s ",msg->un.e->base->id);
			if(msg->un.e->dest)
				printf("of %s ",msg->un.e->dest->base->id);
			if(msg->un.e->src)
				printf("from %s ",msg->un.e->src->base->id);
			printf("was removed\n");
			break;
		case MSG_EFFECT_EVENT:
			/*printf("effect %s ",msg->un.e->base->id);
			if(msg->un.e->dest)
				printf("of %s ",msg->un.e->dest->base->id);
			if(msg->un.e->src)
				printf("from %s ",msg->un.e->src->base->id);
			printf("is triggered\n");*/
			break;
		case MSG_EFFECT_EVENT_END:
			/*printf("effect %s ",msg->un.e->base->id);
			if(msg->un.e->dest)
				printf("of %s ",msg->un.e->dest->base->id);
			if(msg->un.e->src)
				printf("from %s ",msg->un.e->src->base->id);
			printf("was triggered completed\n");*/
			break;
		case MSG_EVENT:
			/*printf("event %s ",msg->un.event.ev->id);
			if(msg->un.event.src)
				printf("is caused by %s\n",msg->un.event.src->base->id);
			else
				printf("happens\n");*/
			break;
		case MSG_EVENT_END:
			//printf("event %s end\n",msg->un.event.ev->id);
			break;
		case MSG_FAIL:
			printf("%s fails\n",msg->un.u->base->id);
			break;
		case MSG_HEAL:
			printf("%s heals %lu hp,current hp:%lu\n",msg->un.heal.dest->base->id,msg->un.heal.value,msg->un.heal.dest->hp);
			break;
		case MSG_HPMOD:
			printf("%s %+ld hp,current hp:%lu\n",msg->un.hpmod.dest->base->id,msg->un.hpmod.value,msg->un.hpmod.dest->hp);
			break;
		case MSG_MISS:
			printf("%s missed (target:%s)\n",msg->un.u2.src->base->id,msg->un.u2.dest->base->id);
			break;
		case MSG_MOVE:
			printf("%s uses %s (%s)\n",msg->un.move.u->base->id,msg->un.move.m->id,type2str(msg->un.move.m->type));
			break;
		case MSG_NORMALATTACK:
			printf("%s uses Normal attack (%s)\n",msg->un.u2.src->base->id,type2str(msg->un.u2.src->type0));
			break;
		case MSG_ROUND:
			p=msg->field->p;
			e=msg->field->e;
			printf("\nROUND %d %s:%lu/%lu(%.2lf%%) %s:%lu/%lu(%.2lf%%)\n",msg->round,
			p->front->base->id,
			p->front->hp,
			p->front->base->max_hp,
			100.0*p->front->hp/p->front->base->max_hp,
			e->front->base->id,
			e->front->hp,
			e->front->base->max_hp,
			100.0*e->front->hp/e->front->base->max_hp
			);
			break;
		case MSG_ROUNDEND:
			p=msg->field->p;
			e=msg->field->e;
			printf("ROUND %d END\n",msg->round);
			break;
		case MSG_SPIMOD:
			printf("%s %+ld spi,current hp:%ld\n",msg->un.spimod.dest->base->id,msg->un.spimod.value,msg->un.spimod.dest->spi);
			break;
		case MSG_SWITCH:
			printf("%s is on the front\n",msg->un.u2.dest->base->id);
			break;
		case MSG_UPDATE:
		default:
			break;
	}
}

int rand_selector(struct player *p){
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
int manual_selector(struct player *p){
	char buf[32],*endp;
	int r,cool;
reselect:
	printf("Select the action of %s\n",p->front->base->id);
	if(!isalive(p->front->state))
		goto failed;
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
		printf("%d: %s (%s)",r,p->front->moves[r].id,type2str(p->front->moves[r].type));
		cool=p->front->moves[r].cooldown;
		if(cool)
			printf(" cooldown rounds:%d\n",cool);
		else printf("\n");
	}
failed:
	for(int i=0;i<6;++i){
		if(!p->units[i].base)
			break;
		if(p->units+i==p->front)
			continue;
		printf("[%c]%d: %s\n",isalive(p->units[i].state)?' ':'x',ACT_UNIT0+i,p->units[i].base->id);
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
			if(*endp)goto unknown;
			switch(r){
				case ACT_MOVE0 ... ACT_MOVE7:
					if(!p->front->moves[r].id)
						goto unknown;
					break;
				case ACT_UNIT0 ... ACT_UNIT5:
					if(!p->units[r-ACT_UNIT0].base)
						goto unknown;
					break;
				default:
					goto unknown;
			}
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
	u->unused=0;
	memcpy(u->moves,u->base->moves,8*sizeof(struct move));
	memcpy(u->pmoves,u->base->pmoves,2*sizeof(struct move));
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
void player_action(struct player *p){
	int r;
	struct unit *t;
	switch(p->action){
		case ACT_MOVE0 ... ACT_MOVE7:
			unit_move(p->front,p->front->moves+p->action);
			return;
		case ACT_NORMALATTACK:
			t=gettarget(p->front);
			//printf("%s uses Normal attack (%s)\n",p->front->base->id,type2str(p->front->type0));
			normal_attack(t,p->front);
			return;
		case ACT_UNIT0 ... ACT_UNIT5:
			r=p->action-ACT_UNIT0;
			if(p->units[r].base&&isalive(p->units[r].state)){
				p->front=p->units+r;
				report(p->field,MSG_SWITCH,p->units+r,p->front);
				return;
			}
		default:
			return;
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
int player_selectunit(struct player *p){
	int r=0;
	for(int i=0;i<6;++i){
		if(!p->units[i].base)
			break;
		if(isalive(p->units[i].state)){
			//printf("%d:%s\n",ACT_UNIT0+i,p->units[i].base->id);
			r=1;
			break;
		}
	}
	if(!r)
		return -1;
	r=p->selector(p);
	switch(r){
		case ACT_UNIT0 ... ACT_UNIT5:
			if(!canaction2(p,r))
				return -1;
			r-=ACT_UNIT0;
			report(p->field,MSG_SWITCH,p->units+r,p->front);
			p->front=p->units+r;
			return 0;
		default:
			return -1;
	}
}
void player_moveinit(struct player *p){
	for_each_unit(u,p){
		for(int i=0;i<2;++i){
			if(!u->pmoves[i].id)
				break;
			if(u->pmoves[i].init)
				u->pmoves[i].init(u);
		}
		for(int i=0;i<8;++i){
			if(!u->moves[i].id)
				break;
			if(u->moves[i].init)
				u->moves[i].init(u);
		}
	}
}
#define deadcheck do {\
	r0=!isalive(p->front->state);\
	r1=!isalive(e->front->state);\
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
				ret=p->front->speed<e->front->speed?\
				1:0;\
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
int battle(struct player *p,struct player *e,void (*reporter)(const struct message *)){
	struct player *prior,*latter;
	struct battle_field field;
	int round=0,r0,r1,ret,stage=STAGE_INIT;
	e=p->enemy;
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
	field.reporter=reporter;
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
		stage=STAGE_ROUNDSTART;
		report(&field,MSG_ROUND);
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
		latter->action=latter->selector(latter);
		if((unsigned int)latter->action>=ACT_GIVEUP){
			ret=latter==p?1:0;
			goto out;
		}
		latter=(prior=getprior(p,e))->enemy;
		stage=STAGE_PRIOR;
		if(canaction(prior)){
			if(prior->action!=ACT_ABORT){
				report(&field,MSG_ACTION,prior);
				player_action(prior);
				deadcheck;
			}
		}
		player_update_state(latter);
		stage=STAGE_LATTER;
		if(canaction(latter)){
			if(latter->action!=ACT_ABORT){
				report(&field,MSG_ACTION,latter);
				player_action(latter);
				deadcheck;
			}
		}
		stage=STAGE_ROUNDEND;
		report(&field,MSG_ROUNDEND);
		effect_in_roundend(field.effects);
		deadcheck;
		effect_round_decrease(field.effects,1);
		deadcheck;
		cooldown_decrease(prior);
		cooldown_decrease(latter);
	}
out:
	stage=STAGE_BATTLE_END;
	report(&field,MSG_BATTLE_END,ret?e:p);
	for_each_effect(ep,field.effects){
		effect_final(ep);
	}
	wipetrash(&field);
	return ret;
}
