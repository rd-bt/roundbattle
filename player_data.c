#include "player_data.h"
#include "battle.h"
#include "nbt.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include "readall.c"
char *data_file="data.nbt";
static struct nbt_node *data_read(void){
	int fd=open(data_file,O_RDONLY);
	void *buf;
	ssize_t sz;
	struct nbt_node *r;
	errno=0;
	if(!fd)
		goto fail;
	buf=readall(fd,&sz);
	close(fd);
	if(!buf)
		goto fail;
	r=nbt_read(buf,sz);
	free(buf);
	if(!r)
		goto fail;
	return r;
fail:
	/*if(errno)
		perror("cannot read data");
	else
		fprintf(stderr,"cannot read data\n");
	*/
	return NULL;
}
static void data_write(struct nbt_node *np){
	size_t sz;
	void *buf;
	int fd=open(data_file,O_WRONLY|O_CREAT|O_TRUNC,0660);
	if(fd<0)
		goto fail;
	sz=nbt_size(np);
	buf=malloc(sz);
	if(!buf)
		goto fail1;
	assert(sz==nbt_write(np,buf));
	if(write(fd,buf,sz)<sz)
		goto fail2;
	close(fd);
	free(buf);
	return;
fail2:
	free(buf);
fail1:
	close(fd);
fail:
	if(errno)
		perror("cannot write data");
	else
		fprintf(stderr,"cannot write data\n");
}
int pdata_load(struct player_data *p){
	struct nbt_node *np=data_read(),*np1,*np2;
	struct nbt_node *npp[6];
	struct nbt_node **npp1;
	struct unit_info ui;
	memset(p,0,sizeof(struct player_data));
	if(!np){
		assert((np=nbt_zu("xp",2,0)));

		assert(!ui_create(&ui,"icefield_tiger_cub",1));
		np1=ui_asnbt(&ui);
		assert(np1);
		npp[0]=np1;

		assert(!ui_create(&ui,"flat_mouth_duck",1));
		np1=ui_asnbt(&ui);
		assert(np1);
		npp[1]=np1;

		assert(!ui_create(&ui,"hood_grass",1));
		np1=ui_asnbt(&ui);
		assert(np1);
		npp[2]=np1;

		assert(!ui_create(&ui,"defensive_matrix_1",1));
		np1=ui_asnbt(&ui);
		assert(np1);
		npp[3]=np1;

		assert(!ui_create(&ui,"attacking_matrix_1",1));
		np1=ui_asnbt(&ui);
		assert(np1);
		npp[4]=np1;

		assert(!ui_create(&ui,"self_explode_matrix",1));
		np1=ui_asnbt(&ui);
		assert(np1);
		npp[5]=np1;

		np1=nbt_lista("units",5,npp,6);
		assert(np1);
		assert(nbt_add(np,np1));

		np1=nbt_zd("level",5,1);
		assert(np1);
		np2=nbt_zd("highest",7,1);
		assert(np2);
		assert(nbt_add(np1,np2));
		np2=nbt_list("endless",7,np1);
		assert(np2);
		assert(nbt_add(np,np2));

		p->nbt=np;
		pdata_giveunit(p,"attacking_defensive_combined_matrix_1",1);
		pdata_giveunit(p,"cactus_ball",1);
		pdata_giveunit(p,"three_phase_driven_matrix_1_noscm",1);
		pdata_giveunit(p,"little_snow_bear",1);
	}else {
		p->nbt=np;
	}
	np=nbt_find(p->nbt,"xp",2);
	assert(np);
	p->xp=nbt_zul(np);
	np=nbt_path(p->nbt,"endless/level",13);
	assert(np);
	p->endless_level=nbt_zdl(np);
	np=nbt_path(p->nbt,"endless/highest",15);
	assert(np);
	p->endless_highest=nbt_zdl(np);
	np1=nbt_find(p->nbt,"units",5);
	assert(np1);
	assert(np1->type==NBT_LISTA);
	npp1=(struct nbt_node **)(np1->data+np1->un.v_zd);
	for(int i=0;i<8&&i<np1->count;++i){
		assert(!ui_create_fromnbt(p->ui+i,npp1[i]));
	}
	return 0;
}
int pdata_save(const struct player_data *p){
	struct nbt_node *np,**npp;
	int n;
	np=nbt_find(p->nbt,"xp",2);
	assert(np);
	nbt_zul(np)=p->xp;
	np=nbt_path(p->nbt,"endless/level",13);
	assert(np);
	nbt_zdl(np)=p->endless_level;
	np=nbt_path(p->nbt,"endless/highest",15);
	assert(np);
	nbt_zdl(np)=p->endless_highest;
	for(n=0;n<6;){
		if(!p->ui[n].spec)
			break;
		++n;
	}
	npp=alloca(n*sizeof(struct node *));
	for(int i=0;i<n;++i){
		np=ui_asnbt(p->ui+i);
		assert(np);
		npp[i]=np;
	}
	np=nbt_find(p->nbt,"units",5);
	if(np){
		assert(!nbt_delete(p->nbt,np));
	}
	np=nbt_lista("units",5,npp,n);
	assert(nbt_add(p->nbt,np));
	data_write(p->nbt);
	nbt_free(p->nbt);
	return 0;
}
unsigned long pdata_countitem(const struct player_data *p,const char *id){
	struct nbt_node *np;
	np=nbt_find(p->nbt,"items",5);
	if(!np)
		return 0;
	assert(np->type==NBT_LIST);
	np=nbt_find(nbt_listl(np),id,strlen(id));
	if(!np)
		return 0;
	assert(np->type==NBT_LIST);
	np=nbt_find(nbt_listl(np),"count",5);
	assert(np);
	assert(np->type==NBT_ZU);
	return nbt_zul(np);
}
int pdata_giveunit(const struct player_data *pd,const char *id,int level){
	struct nbt_node *np,*np1;
	np1=create_unit_nbt(id,level);
	if(!np1)
		return -1;
	np=nbt_find(pd->nbt,"storage",7);
	if(!np){
		assert(np=nbt_lista("storage",7,NULL,0));
		assert(nbt_add(pd->nbt,np));
	}
	nbt_ainsert(pd->nbt,np,np->count,np1);
	return 0;
}
unsigned long pdata_giveitem(const struct player_data *p,const char *id,long count){
	struct nbt_node *np,*np1,*np0,*np2;
	size_t len=strlen(id);
	long n;
	np=nbt_find(p->nbt,"items",5);
	if(!np){
		np=nbt_root("root",4);
		assert(np);
		np=nbt_list("items",5,np);
		assert(np);
		assert(nbt_add(p->nbt,np));
	}
	assert(np->type==NBT_LIST);
	np=nbt_find(np0=nbt_listl(np),id,len);
	if(!np){
		if(count<0)
			return 0;
		np1=nbt_zu("count",5,count);
		assert(np1);
		np=nbt_list(id,len,np1);
		assert(nbt_add(np0,np));
		return count;
	}
	np=nbt_find(np1=nbt_listl(np2=np),"count",5);
	assert(np);
	n=nbt_zul(np)+count;
	if(n<0)
		n=0;
	if(!n){
		assert(!nbt_delete(np0,np2));
		return 0;
	}
	nbt_zul(np)=n;
	return n;
}
int pdata_fake(struct player_data *p,const char *id,int level){
	memset(p,0,sizeof(struct player_data));
	ui_create(p->ui,id,level);
	return 0;
}
int pbattle(const struct player_data *p,
		const struct player_data *e,
		int (*selector_p)(const struct player *),
		int (*selector_e)(const struct player *),
		void (*reporter_p)(const struct message *msg,const struct player *p),
		void (*reporter_e)(const struct message *msg,const struct player *p),
		void (*init)(struct battle_field *)){
	struct unit_base ubp[6],ube[6];
	struct player p0,e0;
	memset(&p0,0,sizeof(struct player));
	memset(&e0,0,sizeof(struct player));
	for(int i=0;i<6;++i){
		if(p->ui[i].spec){
			mkbase(p->ui+i,ubp+i);
			p0.units[i].base=ubp+i;
		}else
			p0.units[i].base=NULL;
	}
	for(int i=0;i<6;++i){
		if(e->ui[i].spec){
			mkbase(e->ui+i,ube+i);
			e0.units[i].base=ube+i;
		}else
			e0.units[i].base=NULL;
	}
	p0.selector=selector_p;
	e0.selector=selector_e;
	p0.reporter=reporter_p;
	e0.reporter=reporter_e;
	return battle(&p0,&e0,NULL,NULL);
};
