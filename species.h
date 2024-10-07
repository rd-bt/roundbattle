#ifndef _SPECIES_H_
#define _SPECIES_H_
#include <stddef.h>
#include "battle-core.h"
#define UTYPE_UNKNOWN 0
#define UTYPE_PLOT 1
#define UTYPE_WILD 2
#define UF_EVOLVABLE 1
#define UF_CANSELECTTYPE 2
struct species {
	struct unit_base max;
	int flag,type,evolve_level,xp_type;
	const char *moves[151];
};
extern const struct species builtin_species[];
extern const size_t builtin_species_size;
const struct species *get_builtin_species_by_id(const char *id);
#endif
