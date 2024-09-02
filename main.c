#include "battle.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
const char *moves[]={"steel_flywheel","fate_destroying_slash","mosquito_bump","metal_bomb","angry","spi_fcrack","natural_shield","spi_shattering_slash",NULL};
int main(){
	struct player p1,p2;
	struct unit_base bt={"tiger",6600,300,224,666,123,10,128,2,0,0,0,0,TYPE_ICE,TYPE_VOID,13,0};
	struct unit_base bb={"bear",7500,203,356,332,100,150,128,2,0,0,0,0,TYPE_GRASS,TYPE_VOID,13,0};
	int r;
	srand48(time(NULL));
	memset(&p1,0,sizeof(p1));
	memset(&p2,0,sizeof(p2));
	p1.enemy=&p2;
	p2.enemy=&p1;
	p1.selector=manual_selector;
	p2.selector=manual_selector;
	for(r=0;r<8;++r){
		bt.moves[r].id=NULL;
		bb.moves[r].id=NULL;
	}
	for(r=0;moves[r];++r)
		memcpy(bb.moves+r,get_builtin_move_by_id(moves[r]),sizeof(struct move));
	for(r=0;moves[r];++r)
		memcpy(bt.moves+r,get_builtin_move_by_id(moves[r]),sizeof(struct move));
	memcpy(bt.pmoves,get_builtin_move_by_id("primordial_breath"),sizeof(struct move));
	p1.units->base=&bt;
	p2.units->base=(p2.units+1)->base=&bb;
	r=battle(&p1,&p2,reporter_default);
	if(r<0)puts("cannot battle");
	else if(r==0)puts("tiger wins");
	else puts("bear wins");
	return 0;
}
