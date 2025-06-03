#include "utils.h"
#include "moves.h"
#include <string.h>
#include <assert.h>
int ui_create(struct unit_info *ui,const char *id,int level){
	const struct species *spec;
	const char *p;
	spec=get_builtin_species_by_id(id);
	if(!spec)
		return -1;
	memset(ui,0,sizeof(struct unit_info));
	ui->spec=spec;
	ui->level=level;
	if(spec->flag&UF_CANSELECTTYPE){
		ui->type0=spec->max.type0;
		ui->type1=spec->max.type1;
	}
	for(int i=level>150?150:level,r=0;i>=0&&r<8;--i){
		if(!(p=spec->moves[i]))
			continue;
		ui->moves[r++]=p;
	}
	return 0;
}
int ui_create_fromnbt(struct unit_info *ui,const struct nbt_node *np){
	const struct species *spec;
	const char *id;
	const struct nbt_node *n;
	const struct move *m;
	char buf[8];
	size_t r;
	n=nbt_find(np,"id",2);
	assert(n&&n->type==NBT_STR);
	if(!n||n->type!=NBT_STR)
		return -1;
	id=n->data+n->un.v_zd;
	spec=get_builtin_species_by_id(id);
	assert(spec);
	if(!spec)
		return -1;
	memset(ui,0,sizeof(struct unit_info));
	n=nbt_find(np,"level",5);
	assert(n);
	assert(n->type==NBT_ZD);
	if(!n||n->type!=NBT_ZD)
		return -1;
	ui->level=n->un.v_zd;
	ui->spec=spec;
	if(spec->flag&UF_CANSELECTTYPE){
		n=nbt_find(np,"type0",5);
		assert(n);
		assert(n->type==NBT_ZD);
		if(!n||n->type!=NBT_ZD)
			return -1;
		ui->type0=n->un.v_zd;
		n=nbt_find(np,"type1",5);
		assert(n&&n->type==NBT_ZD);
		if(!n||n->type!=NBT_ZD)
			return -1;
		ui->type1=n->un.v_zd;
	}
	memcpy(buf,"move0",6);
	r=0;
	for(int i=0;i<8;++i){
		n=nbt_find(np,buf,5);
		++buf[4];
		if(!n||n->type!=NBT_STR||!(m=get_builtin_move_by_id(n->data+n->un.v_zd)))
			continue;
		ui->moves[r++]=m->id;
	}
	return 0;
}
struct nbt_node *ui_asnbt(const struct unit_info *ui){
	struct nbt_node *r,*np;
	char buf[8];
	const char *cp;
	cp=ui->spec->max.id;
	r=nbt_str("id",2,cp,strlen(cp));
	if(!r)
		return NULL;
	np=nbt_zd("level",5,ui->level);
	if(!np)
		goto fail;
	assert(nbt_add(r,np));
	if(ui->spec->flag&UF_CANSELECTTYPE){
		np=nbt_zd("type0",5,ui->type0);
		if(!np)
			goto fail;
		assert(nbt_add(r,np));
		np=nbt_zd("type1",5,ui->type1);
		if(!np)
			goto fail;
		assert(np);
		assert(nbt_add(r,np));
	}
	memcpy(buf,"move0",6);
	for(int i=0;i<8;++i){
		cp=ui->moves[i];
		if(!cp)
			continue;
		np=nbt_str(buf,5,cp,strlen(cp));
		++buf[4];
		if(!np)
			goto fail;
		assert(nbt_add(r,np));
	}
	return r;
fail:
	nbt_free(r);
	return NULL;
}
struct nbt_node *create_unit_nbt(const char *id,int level){
	struct unit_info ui;
	if(ui_create(&ui,id,level)<0)
		return NULL;
	return ui_asnbt(&ui);
}
int mkbase_id(const char *id,int level,struct unit_base *out){
	const struct unit_base *max;
	double coef;
	struct unit_info info[1];
	const struct move *m;
	memset(out,0,sizeof(struct unit_base));
	if(ui_create(info,id,level)<0)
		return -1;
	max=&info->spec->max;
	coef=(double)info->level/(double)(max->level>0?max->level:150);
	out->id=max->id;
	out->max_hp=coef*max->max_hp;
	out->atk=coef*max->atk;
	out->def=coef*max->def;
	out->speed=coef*max->speed;
	out->hit=max->hit;
	out->avoid=max->avoid;
	out->max_spi=max->max_spi;
	out->crit_effect=max->crit_effect;
	out->physical_bonus=max->physical_bonus;
	out->magical_bonus=max->magical_bonus;
	out->physical_derate=max->physical_derate;
	out->magical_derate=max->magical_derate;
	if(info->spec->flag&UF_CANSELECTTYPE){
		out->type0=info->type0;
		out->type1=info->type1;
	}else {
		out->type0=max->type0;
		out->type1=max->type1;
	}
	out->level=info->level;
	for(int i=0,r=0;i<8;++i){
		if(!info->moves[i])
			continue;
		m=get_builtin_move_by_id(info->moves[i]);
		if(!m)
			continue;
		memcpy(out->moves+r++,m,sizeof(struct move));
	}
	return 0;
}
