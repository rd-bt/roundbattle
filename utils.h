#ifndef _UTILS_H_
#define _UTILS_H_
#include "info.h"
#include "nbt.h"
int ui_create(struct unit_info *ui,const char *id,int level);
int ui_create_fromnbt(struct unit_info *ui,const struct nbt_node *np);
struct nbt_node *ui_asnbt(const struct unit_info *ui);
#endif
