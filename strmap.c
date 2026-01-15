#include "strmap.h"
#include <string.h>
#include <err.h>
#include <stdlib.h>
void *xmalloc(size_t size){
	void *r;
	r=malloc(size);
	if(!r){
		warn("IN xmalloc()\n"
			"CANNOT ALLOCATE MEMORY");
		warnx("ABORTING");
		abort();
	}
	return r;
}
void *xrealloc(void *old,size_t size){
	void *r;
	r=realloc(old,size);
	if(!r){
		warn("IN xrealloc()\n"
			"CANNOT REALLOCATE MEMORY");
		warnx("ABORTING");
		abort();
	}
	return r;
}
void __attribute__((constructor)) h_start(void){
	expr_allocator=xmalloc;
	expr_reallocator=xrealloc;
}
struct strmap *strmap_add(struct strmap *map,const char *from,size_t fl,const char *to,size_t tl){
	char buf[EXPR_SYMLEN];
	if(!map){
		map=xmalloc(sizeof(struct strmap));
		map->esp=new_expr_symset();
	}
	if(!expr_symset_addl(map->esp,from,fl,EXPR_ALIAS,EXPR_SF_INJECTION,to,tl)){
		if(fl>=EXPR_SYMLEN)
			fl=EXPR_SYMLEN-1;
		memcpy(buf,from,fl);
		buf[fl]=0;
		warnx("conflict %s",buf);
	}
	return map;
}
const char *strmap_find(const struct strmap *map,const char *from,size_t fl){
	struct expr_symbol *c;
	if(!map->esp)
		return NULL;
	c=expr_symset_search(map->esp,from,fl);
	if(!c)
		return NULL;
	return expr_symset_hot(c);
}
void strmap_free(struct strmap *map){
	if(map->esp){
		expr_symset_free(map->esp);
	}
	free(map);
}
