#include "battle.h"
#include "moves.h"
#include "info.h"
#include "utils.h"
#include "player_data.h"
#include "menu.h"
#include "locale.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include <assert.h>
const char *moves[]={"cycle_erode","defend","razor_carrot","piercing_missile","breach_missile","flamethrower","urgently_repair","metal_syncretize",NULL};

int main(){
	struct player_data p1;
	//size_t r;
	assert(!pdata_load(&p1));
	main_menu(&p1);
	/*scr();
	writemove(p1.ui);
	endwin();
	assert(!pdata_fake(&p2,"icefield_tiger",250));
	srand48(time(NULL));
	tm_init();
	r=pbattle(&p1,&p2,term_selector,rand_selector,reporter_term);
	if(r<0){
//error:
		puts("cannot battle");
	}else {
			if(r==0)puts("tiger wins");
		else puts("bear wins");
	}
	tm_end();*/
	return 0;
}
