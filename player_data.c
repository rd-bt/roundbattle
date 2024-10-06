#include "player_data.h"
#include "battle.h"
#include <string.h>
int pdata_load(struct player_data *p){
	memset(p,0,sizeof(struct player_data));
	p->endless_level=1;
	ui_create(p->ui,"icefield_tiger_cub",1);
	ui_create(p->ui+1,"flat_mouth_duck",1);
	ui_create(p->ui+2,"hood_grass",1);
	return 0;
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
		void (*reporter_e)(const struct message *msg,const struct player *p)
		){
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
	return battle(&p0,&e0);
};
