
#ifndef _BATTLE_H_
#define _BATTLE_H_
#include "battle-core.h"
int battle(struct player *p);
int rand_selector(struct player *p);
int manual_selector(struct player *p);
const struct move *get_builtin_move_by_id(const char *id);
extern const struct move builtin_moves[];
#endif
