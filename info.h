#ifndef _INFO_H_
#define _INFO_H_
#include "species.h"
struct base_point {
	int max_hp,
	    atk,
	    def,
	    crit_effect,
	    physical_bonus,
	    magical_bonus,
	    physical_derate,
	    magical_derate;
};
struct unit_info {
	const struct species *spec;
	struct base_point bp;
	const char *moves[8];
	int level,type0,type1,unused;
};
unsigned long xp_require_fromto(const struct species *spec,int from,int to);
unsigned long xp_require_evo(const struct unit_info *info);
unsigned long xp_require(const struct unit_info *info);
unsigned long xp_require2(const struct species *spec,int level);
int mkbase(const struct unit_info *info,struct unit_base *out);
#endif
