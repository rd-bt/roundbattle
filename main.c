#include "battle.h"
#include "moves.h"
#include "info.h"
#include "utils.h"
#include "player_data.h"
#include "menu.h"
//#include "locale.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include <assert.h>
#include <locale.h>

int main(){
	struct player_data p1;
	setlocale(LC_ALL,"");
	assert(!pdata_load(&p1));
	main_menu(&p1);
	pdata_save(&p1);
	printf("\033c");
	return 0;
}
