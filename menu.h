#ifndef _MENU_H_
#define _MENU_H_
#include "info.h"
#include "player_data.h"
const struct species *getunit(void);
void writemove(struct unit_info *ui);
void main_menu(struct player_data *pd);
#endif
