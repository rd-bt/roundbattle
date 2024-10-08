#include "info.h"
#include "moves.h"
#include <string.h>
#include <math.h>
unsigned long xp_require_fromto(const struct species *spec,int from,int to){
	if(from>=to)
		return 0;
	return spec->xp_type*(pow(1.056,to)-pow(1.056,from))/0.056;
}
unsigned long xp_require_evo(const struct unit_info *info){
	int b=info->spec->xp_type,a=(info->spec+1)->xp_type;
	if(b>=a)
		return 0;
	return (a-b)*(pow(1.056,info->level)-1.056)/0.056;
}
unsigned long xp_require(const struct unit_info *info){
	return info->spec->xp_type*pow(1.056,info->level);
}
unsigned long xp_require2(const struct species *spec,int level){
	return spec->xp_type*pow(1.056,level);
}
int mkbase(const struct unit_info *info,struct unit_base *out){
	const struct unit_base *max=&info->spec->max;
	double coef=(double)info->level/(double)(max->level>0?max->level:150);
	const struct move *m;
	memset(out,0,sizeof(struct unit_base));
	if(!max->id)
		return -1;
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
	out->type0=max->type0;
	out->type1=max->type1;
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
