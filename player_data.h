#ifndef _PLAYER_DATA_H_
#define _PLAYER_DATA_H_
#include "utils.h"
#include "battle-core.h"
#include "nbt.h"
struct player_data {
	struct unit_info ui[6];
	unsigned long xp,endless_level,endless_highest;
	struct nbt_node *nbt;
};
int pdata_load(struct player_data *p);
int pdata_save(const struct player_data *p);
int pdata_fake(struct player_data *p,const char *id,int level);
unsigned long pdata_countitem(const struct player_data *p,const char *id);
unsigned long pdata_giveitem(const struct player_data *p,const char *id,long count);
int pbattle(const struct player_data *p,
		const struct player_data *e,
		int (*selector_p)(const struct player *),
		int (*selector_e)(const struct player *),
		void (*reporter_p)(const struct message *msg,const struct player *p),
		void (*reporter_e)(const struct message *msg,const struct player *p),
		void (*init)(struct battle_field *));
int pdata_giveunit(const struct player_data *pd,const char *id,int level);
#endif
