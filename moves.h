
#ifndef _MOVES_H_
#define _MOVES_H_
#include "battle-core.h"
const struct move *get_builtin_move_by_id(const char *id);
extern const struct move builtin_moves[];
#endif
