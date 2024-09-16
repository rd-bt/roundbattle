
#ifndef _BATTLE_H_
#define _BATTLE_H_
#include "battle-core.h"
int battle(struct player *p,struct player *e,void (*reporter)(const struct message *));
int rand_selector(const struct player *p);
#endif
