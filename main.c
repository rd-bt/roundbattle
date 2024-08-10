#include "battle.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(){
	struct player p1,p2;
	struct unit_base bt={"tiger",6600,300,224,666,123,10,128,2,0,0,0,0,TYPE_ICE,TYPE_VOID,13,0};
	struct unit_base bb={"bear",7500,203,356,332,100,220,128,2,0,0,0,0,TYPE_GRASS,TYPE_VOID,13,0};
	int r;
	srand48(time(NULL));
	p1.enemy=&p2;
	p2.enemy=&p1;
	p1.selector=rand_selector;
	p2.selector=manual_selector;
	for(r=0;r<8;++r){
		bt.moves[r].id=NULL;
		bb.moves[r].id=NULL;
	}
	memcpy(bb.moves,builtin_moves,7*sizeof(struct move));
	memcpy(&p1.units->base,&bt,sizeof(bt));
	memcpy(&p2.units->base,&bb,sizeof(bb));
	r=battle(&p1);
	if(r<0)puts("cannot battle");
	else if(r==0)puts("tiger wins");
	else puts("bear wins");
	return 0;
}
