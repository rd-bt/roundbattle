
#ifndef _BATTLE_H_
#define _BATTLE_H_
#include "battle-core.h"
int battle(struct player *p);
int rand_selector(struct player *p);
int manual_selector(struct player *p);
extern const struct move builtin_moves[];
#endif
