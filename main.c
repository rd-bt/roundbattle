#include "battle.h"
#include "moves.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
const char *moves[]={"mud_shot","defend","breach_missile","piercing_missile","iron_wall","flamethrower","speed_up","elbow",NULL};

void reporter_term(const struct message *msg);
int term_selector(const struct player *p);
int main(){
	struct player p1,p2;
	struct unit_base bt={"tiger",1400,300,403,666,100,100,128,2,0,0,0,0,TYPE_ICE,TYPE_VOID,150,0};
	struct unit_base bb={"bear",1700,203,437,332,100,100,128,2,0,0,0,0,TYPE_GRASS,TYPE_VOID,150,0};
	int r;
//	for(r=0;r<20;++r)
//		printf("%d\n",test(0.5));
//	return 0;
	srand48(time(NULL));
	memset(&p1,0,sizeof(p1));
	memset(&p2,0,sizeof(p2));
	p1.enemy=&p2;
	p2.enemy=&p1;
	p1.selector=term_selector;
	p2.selector=rand_selector;
	for(r=0;r<8;++r){
		bt.moves[r].id=NULL;
		bb.moves[r].id=NULL;
	}
	for(r=0;moves[r];++r)
		memcpy(bb.moves+r,get_builtin_move_by_id(moves[r]),sizeof(struct move));
	for(r=0;moves[r];++r)
		memcpy(bt.moves+r,get_builtin_move_by_id(moves[r]),sizeof(struct move));
	memcpy(bb.pmoves,get_builtin_move_by_id("thorns"),sizeof(struct move));
	memcpy(bb.pmoves+1,get_builtin_move_by_id("primordial_breath"),sizeof(struct move));
	memcpy(bt.pmoves,get_builtin_move_by_id("myriad"),sizeof(struct move));
	memcpy(bt.pmoves+1,get_builtin_move_by_id("heat_engine"),sizeof(struct move));
	p1.units->base=&bt;
	(p1.units+1)->base=&bt;
	p2.units->base=&bb;
	(p2.units+1)->base=&bb;
	(p2.units+2)->base=&bb;
	(p2.units+3)->base=&bb;
	(p2.units+4)->base=&bb;
	(p2.units+5)->base=&bb;
	r=battle(&p1,&p2,reporter_term);
	if(r<0)puts("cannot battle");
	else if(r==0)puts("tiger wins");
	else puts("bear wins");
	return 0;
}
