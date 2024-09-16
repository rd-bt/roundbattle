#ifndef _STRMAP_H_
#define _STRMAP_H_
#include <stddef.h>
struct strmap {
	size_t from;
	size_t to;
	struct strmap *gt,*lt,*eq;
	char data[];
};
struct strmap *strmap_add(struct strmap *map,const char *from,size_t fl,const char *to,size_t tl);
const char *strmap_find(const struct strmap *map,const char *from,size_t fl);
void strmap_free(struct strmap *map);
#endif
