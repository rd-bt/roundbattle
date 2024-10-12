#ifndef _NBT_H_
#define _NBT_H_
#define NBT_ALIGN 8
#include <stddef.h>

#define NBT_ZD 1
#define NBT_ZU 2
#define NBT_DBL 3
#define NBT_LIST 4
#define NBT_ZDA 5
#define NBT_ZUA 6
#define NBT_DBLA 7
#define NBT_STR 8
#define NBT_BYTEA 9
#define NBT_LISTA 10

#define nbt_align(x) (((x)+(NBT_ALIGN-1))/NBT_ALIGN*NBT_ALIGN)

#define nbt_zdl(p) ((p)->un.v_zd)
#define nbt_zul(p) ((p)->un.v_zu)
#define nbt_dbll(p) ((p)->un.v_dbl)
#define nbt_listl(p) ((p)->un.v_list)
#define nbt_zdap(p) ({__auto_type _p=(p);(ptrdiff_t *)(_p->data+_p->un.v_zd);})
#define nbt_zuap(p) ({__auto_type _p=(p);(size_t *)(_p->data+_p->un.v_zd);})
#define nbt_dblap(p) ({__auto_type _p=(p);(double *)(_p->data+_p->un.v_zd);})
#define nbt_strp(p) ({__auto_type _p=(p);(const char *)(_p->data+_p->un.v_zd);})
#define nbt_byteap(p) ({__auto_type _p=(p);(unsigned char *)(_p->data+_p->un.v_zd);})
#define nbt_listp(p) ({__auto_type _p=(p);(struct nbt_node **)(_p->data+_p->un.v_zd);})

union nbt_value {
	ptrdiff_t v_zd;
	size_t v_zu;
	double v_dbl;
	struct nbt_node *v_list;
};
struct nbt_node {
	int type;
	unsigned int keylen;
	size_t len,count;
	struct nbt_node *prev,*next;
	union nbt_value un;
	char data[];
};
struct nbt_element {
	int type;
	unsigned int keylen;
	size_t len,count;
	union nbt_value un;
	char data[];
};
struct nbt_node *nbt_find(const struct nbt_node *list,const char *key,unsigned int keylen);
struct nbt_node *nbt_path(const struct nbt_node *list,const char *path,size_t pathlen);
size_t nbt_size(const struct nbt_node *list);
size_t nbt_count(const struct nbt_node *list);
void nbt_free(struct nbt_node *list);
size_t nbt_delete(struct nbt_node *list,struct nbt_node *node);
int nbt_replace(struct nbt_node *list,struct nbt_node *node,struct nbt_node *new_node);
struct nbt_node *nbt_resize(struct nbt_node *list,struct nbt_node *node,size_t count);
size_t nbt_write(const struct nbt_node *list,void *buf);
size_t nbt_writestr(const struct nbt_node *list,char *buf);
size_t nbt_strlen(const struct nbt_node *list);
struct nbt_node *nbt_read(const void *buf,size_t size);
struct nbt_node *nbt_add(struct nbt_node *list,struct nbt_node *node);
size_t nbt_addall(struct nbt_node *list,struct nbt_node *node);
struct nbt_node *nbt_list(const char *key,unsigned int keylen,struct nbt_node *np);
struct nbt_node *nbt_zd(const char *key,unsigned int keylen,ptrdiff_t value);
struct nbt_node *nbt_zu(const char *key,unsigned int keylen,size_t value);
struct nbt_node *nbt_dbl(const char *key,unsigned int keylen,double value);
struct nbt_node *nbt_zda(const char *key,unsigned int keylen,const ptrdiff_t *value,size_t count);
struct nbt_node *nbt_zua(const char *key,unsigned int keylen,const size_t *value,size_t count);
struct nbt_node *nbt_dbla(const char *key,unsigned int keylen,const double *value,size_t count);
struct nbt_node *nbt_str(const char *key,unsigned int keylen,const char *value,size_t size);
struct nbt_node *nbt_bytea(const char *key,unsigned int keylen,const void *value,size_t count);
struct nbt_node *nbt_lista(const char *key,unsigned int keylen,struct nbt_node *const *value,size_t count);
#endif
