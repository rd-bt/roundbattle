#include <stddef.h>
#include <string.h>
#include "item.h"
void endless_cream_onenter(struct player_data *pd,struct nbt_node *np){
	pd->endless_level=1;
	pdata_giveitem(pd,"endless_cream",-1);
}
const struct item builtin_items[]={
	{
		.id="endless_cream",
		.onenter=endless_cream_onenter,
	},
	{NULL}
};
const size_t builtin_items_size=sizeof(builtin_items)/sizeof(builtin_items[0])-1;
const struct item *get_builtin_item_by_id(const char *id){
	unsigned long i;
	for(i=0;builtin_items[i].id;++i){
		if(!strcmp(id,builtin_items[i].id))
			return builtin_items+i;
	}
	return NULL;
}
