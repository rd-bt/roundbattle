
#ifndef _MOVES_H_
#define _MOVES_H_
#include "battle-core.h"

const struct move *get_builtin_move_by_id(const char *id);
extern const struct move builtin_moves[];
extern const size_t builtin_moves_size;
extern const struct effect_base *effects[];
extern const size_t effects_size;
#endif
