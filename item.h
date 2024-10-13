#ifndef _ITEM_H_
#define _ITEM_H_
#include <stddef.h>
#include "player_data.h"
#include "nbt.h"
struct item {
	const char *id;
	void (*onenter)(struct player_data *pd,struct nbt_node *np);
};
extern const struct item builtin_items[];
extern const size_t builtin_items_size;
const struct item *get_builtin_item_by_id(const char *id);
#endif
