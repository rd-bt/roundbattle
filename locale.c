#include "locale.h"
#include "strmap.h"
#include <string.h>
#include <stdio.h>
#include <alloca.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "readall.c"
static const char *types_string[21]={"void_type","grass","fire","water","steel","light","fighting","wind","poison","rock","electric","ghost","ice","bug","machine","soil","dragon","normal","devine_grass","alkali_fire","devine_water"};
static struct strmap *loc=NULL;
int loc_disable=0;
static void __attribute__((destructor)) locale_end(void){
	if(loc)
		strmap_free(loc);
}
static void load_locale(void){
	ssize_t r;
	char *buf,*p,*p1,*p2;
	int fd;
	fd=open("zh_CN.lang",O_RDONLY);
	assert(fd>=0);
	buf=readall(fd,&r);
	close(fd);
	assert(buf);
	for(p=buf;*p;){
		p1=strchr(p,'\n');
		if(!p1)
			p1=p+strlen(p);
		switch(*p){
			case '#':
				if(!*p1)
					goto end;
				p=p1+1;
				break;
			case '\n':
				++p;
				continue;
			default:
				break;
		}
		if(p1<p)
			__builtin_unreachable();
		p2=memchr(p,'=',p1-p);
		if(p2&&p2>p&&p1>p2+1){
			loc=strmap_add(loc,p,p2-p,p2+1,p1-p2-1);
		}
		if(*p1)
			p=p1+1;
		else
			goto end;
	}
end:
	free(buf);
}
const char *locale(const char *id){
	if(loc_disable)
		return NULL;
	if(!loc){
		load_locale();
	}
	return strmap_find(loc,id,strlen(id));
}

const char *ts(const char *id){
	const char *p;
	p=locale(id);
	if(!p)
		return id;
	return p;
}
const char *type_ts(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"type.%s.name",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}
const char *e2s(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+13);
	sprintf(buf,"effect.%s.name",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}
const char *type2str(int type){
	unsigned int index=type?__builtin_ctz(type)+1:0;
	if(index>=21)
		return type_ts("unknown");
	return type_ts(types_string[index]);
}
const char *move_ts(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"move.%s.name",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}

const char *move_desc(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"move.%s.desc",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}
const char *unit_ts(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"unit.%s.name",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}

const char *unit_desc(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"unit.%s.desc",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}
const char *item_ts(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"item.%s.name",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}

const char *item_desc(const char *id){
	char *buf;
	const char *p;
	size_t l;
	l=strlen(id);
	buf=alloca(l+11);
	sprintf(buf,"item.%s.desc",id);
	p=locale(buf);
	if(!p)
		return id;
	return p;
}
