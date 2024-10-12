#include "nbt.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define likely(cond) __builtin_expect(!!(cond),1)
#define unlikely(cond) __builtin_expect(!!(cond),0)
static int expr_firstdiff(const char *restrict s1,const char *restrict s2,size_t len){
	int r;
	do {
		r=(unsigned int)*(s1++)-(unsigned int)*(s2++);
		if(unlikely(r))break;
	}while(--len);
	return r;
}
static int expr_strdiff(const char *restrict s1,size_t len1,const char *restrict s2,size_t len2,int *sum){
	int r;
	if(len1==len2){
		r=memcmp(s1,s2,len1);
		if(unlikely(!r))return 0;
		*sum=expr_firstdiff(s1,s2,len1);
		return 1;
	}
	*sum=expr_firstdiff(s1,s2,len1<len2?len1:len2);
	return 1;
}
static struct nbt_node **nbt_findtail(struct nbt_node *list,const char *key,unsigned int keylen){
	int sum;
	for(;;){
		if(!expr_strdiff(key,keylen,list->data,list->keylen,&sum))
			return NULL;
		if(sum>0){
			if(!list->next)
				return &list->next;
			else
				list=list->next;
		}else {
			if(!list->prev)
				return &list->prev;
			else
				list=list->prev;
		}
	}
}
static struct nbt_node **nbt_findentry(struct nbt_node *list,const char *key,unsigned int keylen){
	int sum;
	struct nbt_node **r=NULL;
	for(;;){
		if(!expr_strdiff(key,keylen,list->data,list->keylen,&sum))
			return r;
		if(sum>0){
			r=&list->next;
			if(!*r)
				return NULL;
			else
				list=*r;
		}else {
			r=&list->prev;
			if(!*r)
				return NULL;
			else
				list=*r;
		}
	}
}
struct nbt_node *nbt_find(const struct nbt_node *list,const char *key,unsigned int keylen){
	int sum;
	for(;;){
		if(!expr_strdiff(key,keylen,list->data,list->keylen,&sum))
			return (struct nbt_node *)list;
		if(sum>0){
			if(!list->next)
				return NULL;
			else
				list=list->next;
		}else {
			if(!list->prev)
				return NULL;
			else
				list=list->prev;
		}
	}
}
struct nbt_node *nbt_path(const struct nbt_node *list,const char *path,size_t pathlen){
	const char *endp=path+pathlen,*p;
	const struct nbt_node *np;
	while(path<endp)switch(*path){
		case '/':
			if(list->type!=NBT_LIST)
				return NULL;
			list=list->un.v_list;
			++path;
			continue;
		default:
			p=memchr(path,'/',endp-path);
			np=nbt_find(list,path,(p?p:endp)-path);
			if(!np)
				return NULL;
			if(!p)
				return (struct nbt_node *)np;
			list=np;
			path=p;
			continue;
	}
	return NULL;
}
size_t nbt_size(const struct nbt_node *list){
	size_t s=0;
	if(list->prev)
		s+=nbt_size(list->prev);
	if(list->next)
		s+=nbt_size(list->next);
	switch(list->type){
		case NBT_LIST:
			s+=nbt_size(list->un.v_list)
			+list->len+offsetof(struct nbt_element,data);
			break;
		case NBT_LISTA:
			for(struct nbt_node *const *p=(struct nbt_node *const *)(list->data+list->un.v_zd),*const *endp=p+list->count;p<endp;++p){
				s+=nbt_size(*p);
				
			}
			s+=list->count*sizeof(size_t)
				+nbt_align(list->keylen+1)
				+offsetof(struct nbt_element,data);
			break;
		default:
			s+=list->len+offsetof(struct nbt_element,data);
			break;
	}
	return s;
}
size_t nbt_count(const struct nbt_node *list){
	size_t s=1;
	if(list->prev)
		s+=nbt_count(list->prev);
	if(list->next)
		s+=nbt_count(list->next);
	return s;
}
void nbt_free(struct nbt_node *list){
	if(list->prev)
		nbt_free(list->prev);
	if(list->next)
		nbt_free(list->next);
	switch(list->type){
		case NBT_LIST:
			nbt_free(list->un.v_list);
			break;
		case NBT_LISTA:
			for(struct nbt_node *const *p=(struct nbt_node *const *)(list->data+list->un.v_zd),*const *endp=p+list->count;p<endp;++p){
				nbt_free(*p);
				
			}
			break;
		default:
			break;
	}
	free(list);
}
size_t nbt_delete(struct nbt_node *list,struct nbt_node *node){
	struct nbt_node **e;
	size_t r=0;
	e=nbt_findentry(list,node->data,node->keylen);
	if(!e)
		return SIZE_MAX;
	*e=NULL;
	if(node->prev){
		r+=nbt_addall(list,node->prev);
		node->prev=NULL;
	}
	if(node->next){
		r+=nbt_addall(list,node->next);
		node->next=NULL;
	}
	nbt_free(node);
	return r;
}
int nbt_replace(struct nbt_node *list,struct nbt_node *node,struct nbt_node *new_node){
	struct nbt_node **e;
	e=nbt_findentry(list,node->data,node->keylen);
	if(!e)
		return -1;
	new_node->prev=node->prev;
	new_node->next=node->next;
	node->prev=NULL;
	node->next=NULL;
	*e=new_node;
	nbt_free(node);
	return 0;
}
struct nbt_node *nbt_resize(struct nbt_node *list,struct nbt_node *node,size_t count){
	struct nbt_node **e;
	struct nbt_node *p;
	size_t unit_len,len;
	switch(node->type){
		case NBT_LISTA:
			unit_len=sizeof(struct nbt_node *);
			break;
		case NBT_BYTEA:
			unit_len=sizeof(unsigned char);
			break;
		case NBT_ZDA:
			unit_len=sizeof(ptrdiff_t);
			break;
		case NBT_ZUA:
			unit_len=sizeof(size_t);
			break;
		case NBT_DBLA:
			unit_len=sizeof(double);
			break;
		default:
			return NULL;
	}
	e=nbt_findentry(list,node->data,node->keylen);
	if(!e)
		return NULL;
	if(count<=node->count){
		node->count=count;
		return list;
	}
	len=nbt_align(nbt_align(node->keylen+1)+count*unit_len);
	p=realloc(node,len+offsetof(struct nbt_node,data));
	if(!p)
		return NULL;
	p->len=len;
	*e=p;
	return p;
}
static void nbt_writep(const struct nbt_node *list,void **buf){
	struct nbt_element *e=*buf;
	*(char **)buf+=offsetof(struct nbt_element,data);
	struct nbt_node *const *np;
	size_t *zp,x;
	e->type=list->type;
	e->keylen=list->keylen;
	e->count=list->count;
	x=nbt_align(list->keylen+1);
	memcpy(e->data,list->data,x);
	switch(e->type){
		case NBT_LISTA:
			e->un.v_zd=x;
			e->len=e->un.v_zd+e->count*sizeof(size_t);
			zp=(size_t *)(e->data+e->un.v_zd);
			np=(struct nbt_node *const *)(list->data+list->un.v_zd);
			for(size_t *endp=zp+e->count;zp<endp;++zp,++np){
				*zp=nbt_size(*np);
				e->len+=*zp;
			}
			*buf=zp;
			np=(struct nbt_node *const *)(list->data+list->un.v_zd);
			for(struct nbt_node *const *endp=(struct nbt_node *const *)(np+list->count);np<endp;++np){
				nbt_writep(*np,buf);
			}
			break;
		case NBT_BYTEA:
		case NBT_STR:
			e->len=list->len;
			*(char **)buf+=e->len;
			e->un.v_zd=list->un.v_zd;
			memcpy(e->data+list->un.v_zd,list->data+list->un.v_zd,list->count);
			if(e->len>x+list->count)
				memset(e->data+x+list->count,0,e->len-x-list->count);
			break;
		case NBT_ZDA:
			e->len=list->len;
			*(char **)buf+=e->len;
			e->un.v_zd=list->un.v_zd;
			memcpy(e->data+list->un.v_zd,list->data+list->un.v_zd,list->count*sizeof(ptrdiff_t));
			if(e->len>x+list->count*sizeof(ptrdiff_t))
				memset(e->data+x+list->count*sizeof(ptrdiff_t),0,e->len-x-list->count*sizeof(ptrdiff_t));
			break;
		case NBT_ZUA:
			e->len=list->len;
			*(char **)buf+=e->len;
			e->un.v_zd=list->un.v_zd;
			memcpy(e->data+list->un.v_zd,list->data+list->un.v_zd,list->count*sizeof(size_t));
			if(e->len>x+list->count*sizeof(size_t))
				memset(e->data+x+list->count*sizeof(size_t),0,e->len-x-list->count*sizeof(size_t));
			break;
		case NBT_DBLA:
			e->len=list->len;
			*(char **)buf+=e->len;
			e->un.v_zd=list->un.v_zd;
			memcpy(e->data+list->un.v_zd,list->data+list->un.v_zd,list->count*sizeof(double));
			if(e->len>x+list->count*sizeof(double))
				memset(e->data+x+list->count*sizeof(double),0,e->len-x-list->count*sizeof(double));
			break;
		case NBT_LIST:
			e->un.v_zd=list->len;
			*(char **)buf+=list->len;
			e->len=list->len+nbt_size(list->un.v_list);
			nbt_writep(list->un.v_list,buf);
			break;
		default:
			e->len=list->len;
			*(char **)buf+=e->len;
			memcpy(&e->un,&list->un,sizeof(union nbt_value));
			break;
	}
	if(list->data[list->keylen-1]&1){
		if(list->prev)
			nbt_writep(list->prev,buf);
		if(list->next)
			nbt_writep(list->next,buf);
	}else {
		if(list->next)
			nbt_writep(list->next,buf);
		if(list->prev)
			nbt_writep(list->prev,buf);
	}
}
size_t nbt_write(const struct nbt_node *list,void *buf){
	void *obuf=buf;
	nbt_writep(list,&buf);
	return (char *)buf-(char *)obuf;
}
#define WRITETF(T,f) \
	chadd('[');\
	x=list->count;\
	i=0;\
	if(x)for(const T *p=(const T *)(list->data+list->un.v_zu);;){\
		*buf+=sprintf(test?test:*buf,f,p[i]);\
		++i;\
		if(i<x)\
			chadd(',');\
		else\
			break;\
	}\
	chadd(']')
#define chadd(c) if(test)(*buf)++;else *((*buf)++)=(c)
static size_t nbt_writenode(const struct nbt_node *list,char *buf,char *test);
static void nbt_writevalue(const struct nbt_node *list,char **buf,char *test){
	size_t x,i;
	switch(list->type){
		case NBT_LISTA:
			chadd('[');
			x=list->count;
			i=0;
			if(x)for(struct nbt_node *const *p=(struct nbt_node *const *)(list->data+list->un.v_zu);;){
				chadd('{');
				(*buf)+=nbt_writenode(p[i],*buf,test);
				chadd('}');
				++i;
				if(i<x)
					chadd(',');
				else
					break;
			}
			chadd(']');
			break;
		case NBT_STR:
			chadd('\"');
			if(!test)
				memcpy(*buf,list->data+list->un.v_zd,list->count);
			(*buf)+=list->count;
			chadd('\"');
			break;
		case NBT_BYTEA:
			WRITETF(unsigned char,"%hhub");
			break;
		case NBT_ZDA:
			WRITETF(ptrdiff_t,"%zd");
			break;
		case NBT_ZUA:
			WRITETF(size_t,"%zuu");
			break;
		case NBT_DBLA:
			WRITETF(double,"%lfd");
			break;
		case NBT_LIST:
			chadd('{');
			(*buf)+=nbt_writenode(list->un.v_list,*buf,test);
			chadd('}');
			break;
		case NBT_ZD:
			(*buf)+=sprintf(test?test:*buf,"%zd",list->un.v_zd);
			break;
		case NBT_ZU:
			(*buf)+=sprintf(test?test:*buf,"%zuu",list->un.v_zu);
			break;
		case NBT_DBL:
			(*buf)+=sprintf(test?test:*buf,"%lfd",list->un.v_dbl);
			break;
		default:
			break;
	}
}
static size_t nbt_writenode(const struct nbt_node *list,char *buf,char *test){
	char *obuf=buf;
	if(list->prev){
		buf+=nbt_writenode(list->prev,buf,test);
		if(test)buf++;else *(buf++)=',';
	}
	buf+=sprintf(test?test:buf,"%s:",list->data);
	nbt_writevalue(list,&buf,test);
	if(list->next){
		if(test)buf++;else *(buf++)=',';
		buf+=nbt_writenode(list->next,buf,test);
	}
	return buf-obuf;
}
size_t nbt_writestr(const struct nbt_node *list,char *buf){
	size_t r;
	*(buf++)='{';
	r=nbt_writenode(list,buf,NULL);
	buf+=r;
	*(buf++)='}';
	*(buf++)=0;
	return r+2;
}
size_t nbt_strlen(const struct nbt_node *list){
	char test[1024];
	return nbt_writenode(list,NULL,test)+3;
}
struct nbt_node *nbt_read(const void *buf,size_t size){
	struct nbt_node *r=NULL,*p;
	struct nbt_node **pp;
	const struct nbt_element *e;
	size_t x,x1,x2,x3;
	const size_t *zp;
	const char *cp;
	while(size){
		if(size<nbt_align(offsetof(struct nbt_element,data)+2))
			break;
		e=buf;
		if((e->keylen+1>e->keylen?e->keylen+1:e->keylen)>size-nbt_align(offsetof(struct nbt_element,data)))
			break;
		x=e->len+offsetof(struct nbt_element,data);
		if(e->len>size||x>size||e->len<nbt_align(e->keylen+1))
			break;
		size-=x;
		*(char **)&buf+=x;
		p=NULL;
		switch(e->type){
			case NBT_LISTA:
				x1=e->count;
				if(e->un.v_zd!=nbt_align(e->keylen+1)||x1>(e->len-e->un.v_zd)/sizeof(size_t))
					break;
				zp=(size_t *)(e->data+e->un.v_zd);
				x2=0;
				for(x=0;x<x1;++x){
					x3=x2;
					x2+=zp[x];
					if(x2<x3)
						goto lista_fail0;
				}
				if(x2>e->len-e->un.v_zd-x1*sizeof(size_t))
					break;
				pp=malloc(x1*sizeof(struct nbt_node *));
				if(!pp)
					break;
				cp=(char *)(zp+x1);
				for(x=0;x<x1;++x){
					p=nbt_read((const void *)cp,zp[x]);
					if(!p)
						goto lista_fail;
					pp[x]=p;
					cp+=zp[x];
				}
				p=nbt_lista(e->data,e->keylen,pp,x1);
				free(pp);
				break;
lista_fail:
				for(x3=0;x3<x;++x3){
					free(pp[x3]);
				}
				free(pp);
lista_fail0:
				p=NULL;
				break;
			case NBT_BYTEA:
				if(e->un.v_zd!=nbt_align(e->keylen+1)||e->count>e->len-e->un.v_zd)
					break;
				p=nbt_bytea(e->data,e->keylen,e->data+e->un.v_zd,e->count);
				break;
			case NBT_STR:
				if(e->un.v_zd!=nbt_align(e->keylen+1)||e->count>=e->len-e->un.v_zd)
					break;
				p=nbt_str(e->data,e->keylen,e->data+e->un.v_zd,e->count);
				break;
			case NBT_ZDA:
				if(e->un.v_zd!=nbt_align(e->keylen+1)||e->count>(e->len-e->un.v_zd)/sizeof(ptrdiff_t))
					break;
				p=nbt_zda(e->data,e->keylen,(ptrdiff_t *)(e->data+e->un.v_zd),e->count);
				break;
			case NBT_ZUA:
				if(e->un.v_zd!=nbt_align(e->keylen+1)||e->count>(e->len-e->un.v_zd)/sizeof(size_t))
					break;
				p=nbt_zua(e->data,e->keylen,(size_t *)(e->data+e->un.v_zd),e->count);
				break;
			case NBT_DBLA:
				if(e->un.v_zd!=nbt_align(e->keylen+1)||e->count>(e->len-e->un.v_zd)/sizeof(double))
					break;
				p=nbt_dbla(e->data,e->keylen,(double *)(e->data+e->un.v_zd),e->count);
				break;
			case NBT_LIST:
				p=nbt_read(e->data+e->un.v_zd,e->len-e->un.v_zd);
				if(!p)
					break;
				p=nbt_list(e->data,e->keylen,p);
				break;
			case NBT_ZD:
				p=nbt_zd(e->data,e->keylen,e->un.v_zd);
				break;
			case NBT_ZU:
				p=nbt_zu(e->data,e->keylen,e->un.v_zu);
				break;
			case NBT_DBL:
				p=nbt_dbl(e->data,e->keylen,e->un.v_dbl);
				break;
			default:
				break;
		}
		if(!p)
			break;
		if(!r){
			r=p;
		}else {
			if(!nbt_add(r,p))
				nbt_free(p);
			//fprintf(stderr,"add %s\n",e->data);
		}

	}
	return r;
}
static struct nbt_node *nbt_new(const char *key,unsigned int keylen){
	struct nbt_node *r;
	size_t len;
	len=nbt_align(keylen+1);
	r=malloc(len+offsetof(struct nbt_node,data));
	r->keylen=keylen;
	r->len=len;
	r->prev=NULL;
	r->next=NULL;
	memcpy(r->data,key,keylen);
	memset(r->data+keylen,0,len-keylen);
	return r;
}
static struct nbt_node *nbt_newz(const char *key,unsigned int keylen,size_t extra){
	struct nbt_node *r;
	size_t len;
	ptrdiff_t zd;
	len=nbt_align((zd=nbt_align(keylen+1))+extra);
	r=malloc(len+offsetof(struct nbt_node,data));
	r->keylen=keylen;
	r->len=len;
	r->un.v_zd=zd;
	r->prev=NULL;
	r->next=NULL;
	memcpy(r->data,key,keylen);
	memset(r->data+keylen,0,len-keylen);
	return r;
}
struct nbt_node *nbt_add(struct nbt_node *list,struct nbt_node *node){
	struct nbt_node **pp;
	pp=nbt_findtail(list,node->data,node->len);
	if(!pp)
		return NULL;
	*pp=node;
	return node;
}
size_t nbt_addall(struct nbt_node *list,struct nbt_node *node){
	size_t r=0;
	struct nbt_node *p,*n;
	if(node->prev){
		p=node->prev;
		node->prev=NULL;
	}else
		p=NULL;
	if(node->next){
		n=node->next;
		node->next=NULL;
	}else
		n=NULL;
	if(!nbt_add(list,node)){
		nbt_free(node);
		++r;
	}
	if(p)
		r+=nbt_addall(list,p);
	if(n)
		r+=nbt_addall(list,n);
	return r;
}
struct nbt_node *nbt_list(const char *key,unsigned int keylen,struct nbt_node *np){
	struct nbt_node *r;
	r=nbt_new(key,keylen);
	if(!r)
		return NULL;
	r->type=NBT_LIST;
	r->count=1;
	r->un.v_list=np;
	return r;
}
struct nbt_node *nbt_zd(const char *key,unsigned int keylen,ptrdiff_t value){
	struct nbt_node *r;
	r=nbt_new(key,keylen);
	if(!r)
		return NULL;
	r->type=NBT_ZD;
	r->count=1;
	r->un.v_zd=value;
	return r;
}
struct nbt_node *nbt_zu(const char *key,unsigned int keylen,size_t value){
	struct nbt_node *r;
	r=nbt_new(key,keylen);
	if(!r)
		return NULL;
	r->type=NBT_ZU;
	r->count=1;
	r->un.v_zu=value;
	return r;
}
struct nbt_node *nbt_dbl(const char *key,unsigned int keylen,double value){
	struct nbt_node *r;
	r=nbt_new(key,keylen);
	if(!r)
		return NULL;
	r->type=NBT_DBL;
	r->count=1;
	r->un.v_dbl=value;
	return r;
}
struct nbt_node *nbt_zda(const char *key,unsigned int keylen,const ptrdiff_t *value,size_t count){
	struct nbt_node *r;
	r=nbt_newz(key,keylen,count*sizeof(ptrdiff_t));
	if(!r)
		return NULL;
	r->type=NBT_ZDA;
	r->count=count;
	memcpy(r->data+r->un.v_zd,value,count*sizeof(ptrdiff_t));
	return r;
}
struct nbt_node *nbt_zua(const char *key,unsigned int keylen,const size_t *value,size_t count){
	struct nbt_node *r;
	r=nbt_newz(key,keylen,count*sizeof(size_t));
	if(!r)
		return NULL;
	r->type=NBT_ZUA;
	r->count=count;
	memcpy(r->data+r->un.v_zd,value,count*sizeof(size_t));
	return r;
}
struct nbt_node *nbt_dbla(const char *key,unsigned int keylen,const double *value,size_t count){
	struct nbt_node *r;
	r=nbt_newz(key,keylen,count*sizeof(double));
	if(!r)
		return NULL;
	r->type=NBT_DBLA;
	r->count=count;
	memcpy(r->data+r->un.v_zd,value,count*sizeof(double));
	return r;
}
struct nbt_node *nbt_str(const char *key,unsigned int keylen,const char *value,size_t size){
	struct nbt_node *r;
	r=nbt_newz(key,keylen,nbt_align(size+1));
	if(!r)
		return NULL;
	r->type=NBT_STR;
	r->count=size;
	memcpy(r->data+r->un.v_zd,value,size);
	return r;
}
struct nbt_node *nbt_bytea(const char *key,unsigned int keylen,const void *value,size_t count){
	struct nbt_node *r;
	r=nbt_newz(key,keylen,nbt_align(count));
	if(!r)
		return NULL;
	r->type=NBT_BYTEA;
	r->count=count;
	memcpy(r->data+r->un.v_zd,value,count);
	return r;
}
struct nbt_node *nbt_lista(const char *key,unsigned int keylen,struct nbt_node *const *value,size_t count){
	struct nbt_node *r;
	r=nbt_newz(key,keylen,count*sizeof(struct nbt_node *));
	if(!r)
		return NULL;
	r->type=NBT_LISTA;
	r->count=count;
	memcpy(r->data+r->un.v_zd,value,count*sizeof(struct nbt_node *));
	return r;
}
