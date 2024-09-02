
#ifndef _BATTLE_H_
#define _BATTLE_H_
#include "battle-core.h"
void reporter_default(const struct message *msg);
const char *type2str(int type);
int battle(struct player *p,struct player *e,void (*reporter)(const struct message *));
int rand_selector(struct player *p);
int manual_selector(struct player *p);
const struct move *get_builtin_move_by_id(const char *id);
extern const struct move builtin_moves[];
#endif
