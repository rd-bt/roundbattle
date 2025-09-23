#include "moves.h"
#include "species.h"
#include "locale.h"
#include "utils.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <alloca.h>
static void move_title_print(const struct move *p){
	int div;
	printf("%s",move_ts(p->id));
	printf("(%s:%s","id",p->id);
	printf(",%s:%s",ts("type"),type2str(p->type));
	if(p->prior)
		printf(",%s:%+d",ts("prior"),p->prior);
	if(p->mlevel){
		div=0;
		printf(",%s:",ts("mlevel"));
		if(p->mlevel&MLEVEL_REGULAR)
			printf("%s",ts("regular")),div=1;
		if(p->mlevel&MLEVEL_CONCEPTUAL)
			printf("%s%s",div?"/":"",ts("conceptual")),div=1;
		if(p->mlevel&MLEVEL_FREEZING_ROARING)
			printf("%s%s",div?"/":"",ts("freezing_roaring")),div=1;
	}
	printf(")");
}
void p_move(void){
	const char *p;
	for(size_t i=0;i<builtin_moves_size;++i){
		p=builtin_moves[i].id;
		move_title_print(builtin_moves+i);
		//printf("%s(%s:%s):%s\n",move_ts(p),p1,type2str(builtin_moves[i].type),move_desc(p));
		printf(":%s\n",move_desc(p));
	}
}
void listmove(const struct species *spec){
	size_t i;
	int x=0;
	const char *slv=ts("level");
	printf("[");
	for(i=0;i<=150;++i){
		if(!spec->moves[i]){
			continue;
		}
		printf("%s%s(%s:%zu)",x?",":"",move_ts(spec->moves[i]),slv,i);
		x=1;
	}
	printf("]");
}
void p_unit(void){
	const struct unit_base *p;
	const char *cp;
	for(ssize_t i=0;i<builtin_species_size;++i){
		p=&builtin_species[i].max;
		printf("%s",unit_ts(p->id));
		printf(" %s:%s",ts("type"),type2str(p->type0));
		if(p->type1)
			printf("/%s",type2str(p->type1));
		switch(builtin_species[i].type){
			case UTYPE_WILD:
				cp="wild";
				break;
			case UTYPE_PLOT:
				cp="plot_character";
				break;
			default:
				cp="unknown";
				break;
		}
		printf(" %s:%s ",ts("unit_type"),ts(cp));
	p=&builtin_species[i].max;
	printf("%s:{",ts("attribute_on_max_level"));
	printf("%s:%d %s:%lu %s:%lu %s:%ld %s:%lu %s:%lu %s:%lu %s:%.2lf%% %s:%.2lf%% %s:%.2lf%% %s:%.2lf%% %s:%.2lf%% ",
			ts("level"),p->level,
			ts("max_hp"),p->max_hp,
			ts("atk"),p->atk,
			ts("def"),p->def,
			ts("speed"),p->speed,
			ts("hit"),p->hit,
			ts("avoid"),p->avoid,
			ts("crit_effect"),100*p->crit_effect,
			ts("physical_bonus"),100*p->physical_bonus,
			ts("magical_bonus"),100*p->magical_bonus,
			ts("physical_derate"),100*p->physical_derate,
			ts("magical_derate"),100*p->magical_derate
			);
	printf("%s:%ld} ",ts("max_spi"),p->max_spi);
	//printf("%s:%d %s:%lu ",ts("base_xp"),builtin_species[i].xp_type,ts("xp_from_1_to_max_level"),xp_require_fromto(builtin_species+i,1,150));
	if(builtin_species[i].flag&UF_EVOLVABLE){
		printf("%s:%d(%s) ",ts("evolve_level"),builtin_species[i].evolve_level,unit_ts(builtin_species[i+1].max.id));
	}else
		printf("%s ",ts("unevolvable"));
	/*if(builtin_species[i].flag&UF_CANSELECTTYPE)
		printf("%s:%d (%s) ",ts("evolve_level"),builtin_species[i].evolve_level,unit_ts(builtin_species[i+1].max.id));
	else
		printf("%s ",ts("unevolvable"));
	if(builtin_species[i].flag&UF_CANSELECTTYPE)
		printf("%s ",ts("can_select_type"));*/
	printf("id: %s ",builtin_species[i].max.id);
	printf("%s:",ts("move"));
	listmove(builtin_species+i);
	printf("(%s)",unit_desc(p->id));
	printf("\n");
	}
}
void p_effect(void){
	const char *p;
	int x,flag;
	for(size_t i=0;i<effects_size;++i){
		p=effects[i]->id;
		printf("%s(id:%s",e2s(p),p);
		flag=effects[i]->flag;
#define show_etype(_fl,_id) \
		if(flag&_fl){\
			if(x)\
				printf(" ");\
			else\
				x=1;\
			printf("%s",ts(_id));\
		}
		if(flag){
			if((flag&EFFECT_PASSIVE)==EFFECT_PASSIVE)
				printf(",%s:%s",ts("effect_type"),ts("passive"));
			else {
				printf(",%s:",ts("effect_type"));
				x=0;
				show_etype(EFFECT_ATTR,"attr_bonus");
				show_etype(EFFECT_POSITIVE,"positive");
				show_etype(EFFECT_NEGATIVE,"negative");
				show_etype(EFFECT_ABNORMAL,"abnormal");
				show_etype(EFFECT_CONTROL,"control");
				show_etype(EFFECT_ENV,"environment");
				show_etype(EFFECT_UNPURIFIABLE,"unpurifiable");
				show_etype(EFFECT_NONHOOKABLE,"effect_nonhookable");
			}
		}
	printf(",%s:%d):%s\n",ts("effect_prior"),effects[i]->prior,e2desc(p));
	}
}
void p_types(void){
	int ew;
	for(int t=TYPE_GRASS;t&TYPES_ALL;t<<=1){
		printf("%s:",type2str(t));
		printf("%s:",ts("type_effect"));
		ew=effect_types(t);
		for(int i=TYPE_GRASS;i&TYPES_ALL;i<<=1){
			if(i&ew){
				if(i&TYPES_DEVINE)
					printf(" [%s]",type2str(i));
				else
					printf(" %s",type2str(i));
			}
		}
		printf(" %s:",ts("type_weakened"));
		ew=weak_types(t);
		for(int i=TYPE_GRASS;i&TYPES_ALL;i<<=1){
			if(i&ew){
				if(i&TYPES_DEVINE)
					printf(" [%s]",type2str(i));
				else
					printf(" %s",type2str(i));
			}
		}
		printf("\n");
	}
}
void p_tutorial(const char *item){
	size_t len=strlen(item);
	char *buf=alloca(len+4);
	const char *p;
	memcpy(buf,item,len);
	for(int i=0;i<100;++i){
		sprintf(buf+len,"_%d",i);
		p=locale(buf);
		if(!p)
			break;
		printf("%s\n",ts(p));
	}
}
int main(int argc,char **argv){
	if(argc<2){
		printf("move effect types unit\n");
		return 0;
	}
	for(int i=1;i<argc;++i)
		if(!strcmp(argv[i],"move"))
			p_move();
		else if(!strcmp(argv[i],"effect"))
			p_effect();
		else if(!strcmp(argv[i],"types"))
			p_types();
		else if(!strcmp(argv[i],"unit"))
			p_unit();
		else
			p_tutorial(argv[i]);
	return 0;
}
