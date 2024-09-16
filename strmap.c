#include "strmap.h"
#include <string.h>
#include <stdlib.h>
#define ssize_t ptrdiff_t
struct strmap *strmap_add(struct strmap *map,const char *from,size_t fl,const char *to,size_t tl){
	int sum,sum0;
	struct strmap *c,*new,**p;
	sum0=-fl;
	for(ssize_t i=fl-1;i>=0;--i)
		sum0+=(signed char)from[i];
	new=malloc(sizeof(struct strmap)+fl+tl+2);
	new->from=fl;
	new->to=tl;
	new->gt=NULL;
	new->lt=NULL;
	new->eq=NULL;
	memcpy(new->data,from,fl);
	memcpy(new->data+fl+1,to,tl);
	new->data[fl]=0;
	new->data[fl+tl+1]=0;
	if(!map)
		return new;
	else {
		c=map;
		while(c){
			sum=-c->from;
			for(ssize_t i=c->from-1;i>=0;--i)
				sum+=(signed char)c->data[i];
			if(sum0<sum)
				p=&c->lt;
			else if(sum0>sum)
				p=&c->gt;
			else
				p=&c->eq;
			if(*p){
				c=*p;
			}else
				break;
		}
		*p=new;
		return map;
	}
}
const char *strmap_find(const struct strmap *map,const char *from,size_t fl){
	const struct strmap *c;
	int sum0=-fl,sum;
	for(ssize_t i=fl-1;i>=0;--i)
		sum0+=(signed char)from[i];
	if(!map){
		__builtin_unreachable();
	}
	c=map;
	while(c){
		if(fl==c->from&&!memcmp(c->data,from,fl)){
			return c->data+fl+1;
		}
		sum=-c->from;
		for(ssize_t i=c->from-1;i>=0;--i)
			sum+=(signed char)c->data[i];
		if(sum0<sum)
			c=c->lt;
		else if(sum0>sum)
			c=c->gt;
		else
			c=c->eq;
	}
	return NULL;
}
void strmap_free(struct strmap *map){
	if(map->gt)
		strmap_free(map->gt);
	if(map->lt)
		strmap_free(map->lt);
	if(map->eq)
		strmap_free(map->eq);
	free(map);
}
