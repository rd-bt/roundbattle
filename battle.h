/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#ifndef _BATTLE_H_
#define _BATTLE_H_
#include "battle-core.h"
int battle(struct player *p,struct player *e,struct battle_field *bf,void (*init)(struct battle_field *));
int rand_selector(const struct player *p);
#endif
