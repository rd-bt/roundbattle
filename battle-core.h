#ifndef _BATTLE_CORE_H_
#define _BATTLE_CORE_H_
#include <stddef.h>
#define UNIT_NORMAL 0
#define UNIT_CONTROLLED 1
#define UNIT_INDUCED 2
#define UNIT_SUPPRESSED 3
#define UNIT_FAILED 4
#define UNIT_FREEZING_ROARINGED 5
#define isalive(s) (\
{\
		int _r;\
		switch(s){\
			case UNIT_FAILED:\
			case UNIT_FREEZING_ROARINGED:\
				_r=0;\
				break;\
			default:\
				_r=1;\
				break;\
		}\
		_r;\
}\
)
#define MLEVEL_REGULAR 1
#define MLEVEL_CONCEPTUAL 2
#define MLEVEL_FREEZING_ROARING 4

#define DAMAGE_REAL 0
#define DAMAGE_PHYSICAL 1
#define DAMAGE_MAGICAL 2

#define AF_CRIT 1
#define AF_NODEF 2
#define AF_NOSHIELD 4
#define AF_EFFECT 8
#define AF_WEAK 16
#define AF_NORMAL 32
#define AF_NOFLOAT 64

#define MF_NOCONTROL 1

#define TYPE_VOID (0)
#define TYPE_GRASS (1<<0)
#define TYPE_FIRE (1<<1)
#define TYPE_WATER (1<<2)
#define TYPE_STEEL (1<<3)
#define TYPE_LIGHT (1<<4)
#define TYPE_FIGHTING (1<<5)
#define TYPE_WIND (1<<6)
#define TYPE_POISON (1<<7)
#define TYPE_ROCK (1<<8)
#define TYPE_ELECTRIC (1<<9)
#define TYPE_GHOST (1<<10)
#define TYPE_ICE (1<<11)
#define TYPE_BUG (1<<12)
#define TYPE_MACHINE (1<<13)
#define TYPE_SOIL (1<<14)
#define TYPE_DRAGON (1<<15)
#define TYPE_NORMAL (1<<16)
#define TYPE_DEVINEGRASS (1<<17)
#define TYPE_ALKALIFIRE (1<<18)
#define TYPE_DEVINEWATER (1<<19)

#define TYPES_DEVINE (TYPE_DEVINEGRASS|TYPE_ALKALIFIRE|TYPE_DEVINEWATER)
#define TYPES_ALL (0x0fffff)
#define TYPES_REGULAR (0x01ffff)

#define TYPES_GRASS_EFFECT (TYPE_WATER|TYPE_LIGHT|TYPE_ROCK|TYPE_SOIL)
#define TYPES_GRASS_WEAK (TYPE_FIRE|TYPE_STEEL|TYPE_MACHINE|TYPE_DRAGON)

#define TYPES_FIRE_EFFECT (TYPE_GRASS|TYPE_ICE|TYPE_BUG|TYPE_MACHINE)
#define TYPES_FIRE_WEAK (TYPE_WATER|TYPE_STEEL|TYPE_ROCK|TYPE_SOIL|TYPE_DRAGON)

#define TYPES_WATER_EFFECT (TYPE_FIRE|TYPE_ROCK|TYPE_BUG|TYPE_SOIL)
#define TYPES_WATER_WEAK (TYPE_GRASS|TYPE_WATER|TYPE_ELECTRIC|TYPE_DRAGON)

#define TYPES_STEEL_EFFECT (TYPE_GRASS|TYPE_FIGHTING|TYPE_WIND|TYPE_ROCK|TYPE_GHOST|TYPE_ICE|TYPE_BUG|TYPE_SOIL|TYPE_DRAGON)
#define TYPES_STEEL_WEAK (TYPE_WATER|TYPE_STEEL|TYPE_MACHINE)

#define TYPES_LIGHT_EFFECT (TYPE_FIGHTING|TYPE_GHOST|TYPE_BUG)
#define TYPES_LIGHT_WEAK (TYPE_GRASS|TYPE_MACHINE)

#define TYPES_FIGHTING_EFFECT (TYPE_STEEL|TYPE_FIGHTING|TYPE_ROCK|TYPE_ICE|TYPE_MACHINE)
#define TYPES_FIGHTING_WEAK (TYPE_WIND|TYPE_POISON|TYPE_BUG|TYPE_SOIL)

#define TYPES_WIND_EFFECT (TYPE_GRASS|TYPE_FIRE|TYPE_GRASS|TYPE_FIGHTING|TYPE_BUG)
#define TYPES_WIND_WEAK (TYPE_STEEL|TYPE_ROCK|TYPE_ELECTRIC|TYPE_MACHINE)

#define TYPES_POISON_EFFECT (TYPE_GRASS|TYPE_FIGHTING|TYPE_BUG)
#define TYPES_POISON_WEAK (TYPE_STEEL|TYPE_POISON|TYPE_ROCK|TYPE_MACHINE)

#define TYPES_ROCK_EFFECT (TYPE_FIRE|TYPE_WIND|TYPE_ICE|TYPE_DRAGON)
#define TYPES_ROCK_WEAK (TYPE_STEEL|TYPE_ROCK|TYPE_SOIL|TYPE_MACHINE)

#define TYPES_ELECTRIC_EFFECT (TYPE_WATER|TYPE_WIND|TYPE_BUG|TYPE_MACHINE)
#define TYPES_ELECTRIC_WEAK (TYPE_GRASS|TYPE_ROCK|TYPE_ELECTRIC|TYPE_SOIL)

#define TYPES_GHOST_EFFECT (TYPE_GHOST)
#define TYPES_GHOST_WEAK (TYPE_STEEL|TYPE_BUG|TYPE_MACHINE)

#define TYPES_ICE_EFFECT (TYPE_GRASS|TYPE_WIND|TYPE_BUG|TYPE_SOIL)
#define TYPES_ICE_WEAK (TYPE_FIRE|TYPE_STEEL|TYPE_ROCK|TYPE_MACHINE)

#define TYPES_BUG_EFFECT (TYPE_GRASS|TYPE_BUG|TYPE_SOIL)
#define TYPES_BUG_WEAK (TYPE_FIRE|TYPE_STEEL|TYPE_MACHINE)

#define TYPES_MACHINE_EFFECT (TYPE_GRASS|TYPE_STEEL|TYPE_ROCK)
#define TYPES_MACHINE_WEAK (TYPE_MACHINE)

#define TYPES_SOIL_EFFECT (TYPE_FIRE|TYPE_POISON|TYPE_ROCK|TYPE_ELECTRIC)
#define TYPES_SOIL_WEAK (TYPE_GRASS|TYPE_STEEL|TYPE_MACHINE|TYPE_SOIL)

#define TYPES_DRAGON_EFFECT (TYPE_DRAGON)
#define TYPES_DRAGON_WEAK (TYPE_MACHINE)

#define TYPES_NORMAL_EFFECT (TYPE_VOID)
#define TYPES_NORMAL_WEAK (TYPE_VOID)

#define effect_types(t) (\
{\
		int _r;\
		switch(t){\
			case TYPE_GRASS:\
				_r=TYPES_GRASS_EFFECT;\
				break;\
			case TYPE_FIRE:\
				_r=TYPES_FIRE_EFFECT;\
				break;\
			case TYPE_WATER:\
				_r=TYPES_WATER_EFFECT;\
				break;\
			case TYPE_STEEL:\
				_r=TYPES_STEEL_EFFECT;\
				break;\
			case TYPE_LIGHT:\
				_r=TYPES_LIGHT_EFFECT;\
				break;\
			case TYPE_FIGHTING:\
				_r=TYPES_FIGHTING_EFFECT;\
				break;\
			case TYPE_WIND:\
				_r=TYPES_WIND_EFFECT;\
				break;\
			case TYPE_POISON:\
				_r=TYPES_POISON_EFFECT;\
				break;\
			case TYPE_ROCK:\
				_r=TYPES_ROCK_EFFECT;\
				break;\
			case TYPE_ELECTRIC:\
				_r=TYPES_ELECTRIC_EFFECT;\
				break;\
			case TYPE_GHOST:\
				_r=TYPES_GHOST_EFFECT;\
				break;\
			case TYPE_ICE:\
				_r=TYPES_ICE_EFFECT;\
				break;\
			case TYPE_BUG:\
				_r=TYPES_BUG_EFFECT;\
				break;\
			case TYPE_MACHINE:\
				_r=TYPES_MACHINE_EFFECT;\
				break;\
			case TYPE_SOIL:\
				_r=TYPES_SOIL_EFFECT;\
				break;\
			case TYPE_DRAGON:\
				_r=TYPES_DRAGON_EFFECT;\
				break;\
			case TYPE_NORMAL:\
				_r=TYPES_NORMAL_EFFECT;\
				break;\
			case TYPE_DEVINEGRASS:\
			case TYPE_ALKALIFIRE:\
			case TYPE_DEVINEWATER:\
				_r=TYPES_ALL;\
				break;\
			default:\
				_r=0;\
				break;\
		}\
		_r;\
}\
)
#define weak_types(t) (\
{\
		int _r;\
		switch(t){\
			case TYPE_GRASS:\
				_r=TYPES_GRASS_WEAK;\
				break;\
			case TYPE_FIRE:\
				_r=TYPES_FIRE_WEAK;\
				break;\
			case TYPE_WATER:\
				_r=TYPES_WATER_WEAK;\
				break;\
			case TYPE_STEEL:\
				_r=TYPES_STEEL_WEAK;\
				break;\
			case TYPE_LIGHT:\
				_r=TYPES_LIGHT_WEAK;\
				break;\
			case TYPE_FIGHTING:\
				_r=TYPES_FIGHTING_WEAK;\
				break;\
			case TYPE_WIND:\
				_r=TYPES_WIND_WEAK;\
				break;\
			case TYPE_POISON:\
				_r=TYPES_POISON_WEAK;\
				break;\
			case TYPE_ROCK:\
				_r=TYPES_ROCK_WEAK;\
				break;\
			case TYPE_ELECTRIC:\
				_r=TYPES_ELECTRIC_WEAK;\
				break;\
			case TYPE_GHOST:\
				_r=TYPES_GHOST_WEAK;\
				break;\
			case TYPE_ICE:\
				_r=TYPES_ICE_WEAK;\
				break;\
			case TYPE_BUG:\
				_r=TYPES_BUG_WEAK;\
				break;\
			case TYPE_MACHINE:\
				_r=TYPES_MACHINE_WEAK;\
				break;\
			case TYPE_SOIL:\
				_r=TYPES_SOIL_WEAK;\
				break;\
			case TYPE_DRAGON:\
				_r=TYPES_DRAGON_WEAK;\
				break;\
			case TYPE_NORMAL:\
				_r=TYPES_NORMAL_WEAK;\
				break;\
			case TYPE_DEVINEGRASS:\
			case TYPE_ALKALIFIRE:\
			case TYPE_DEVINEWATER:\
				_r=TYPE_VOID;\
				break;\
			default:\
				_r=0;\
				break;\
		}\
		_r;\
}\
)



#define ACT_MOVE0 0
#define ACT_MOVE1 1
#define ACT_MOVE2 2
#define ACT_MOVE3 3
#define ACT_MOVE4 4
#define ACT_MOVE5 5
#define ACT_MOVE6 6
#define ACT_MOVE7 7
#define ACT_NORMALATTACK 8
#define ACT_ABORT 9
#define ACT_UNIT0 10
#define ACT_UNIT1 11
#define ACT_UNIT2 12
#define ACT_UNIT3 13
#define ACT_UNIT4 14
#define ACT_UNIT5 15
#define ACT_GIVEUP 16

#define ATTR_MAX (+8)
#define ATTR_MIN (-8)

#define EFFECT_ATTR 1
#define EFFECT_ABNORMAL 2
#define EFFECT_CONTROL 4
#define EFFECT_POSITIVE 8
#define EFFECT_NEGATIVE 16
#define EFFECT_UNPURIFIABLE 32
#define for_each_effect(_var,_ehead) for(struct effect *_var=(_ehead),*_p=_var?_var->next:NULL;_p=_var?_var->next:NULL,_var;_var=_p)
#define for_each_unit(_var,_player) for(struct unit *_var=(_player)->units,*_p0=_var+6;_var<_p0&&_var->base;++_var)
struct unit;
struct player;
struct battle_field;
struct effect;
struct move {
	const char *id;
	void (*action)(struct unit *subject);
	int type,mlevel,prior,cooldown,flag,unused;
};
struct unit_base {
	const char *id;
	unsigned long max_hp,atk;
	long def;
	unsigned long speed,hit,avoid,max_spi;
	double crit_effect,
		physical_bonus,magical_bonus,
		physical_derate,magical_derate;
	int type0,type1,level,unused;
	struct move moves[8];
	struct move pmoves[2];
};
struct effect_base {
	const char *id;
	int (*init)(struct effect *,long level,int round);
	void (*inited)(struct effect *);
	void (*end)(struct effect *);
	int (*attack)(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type);
	void (*attack_end)(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type);
	int (*damage)(struct effect *e,struct unit *dest,struct unit *src,unsigned long *value,int *damage_type,int *aflag,int *type);
	void (*damage_end)(struct effect *e,struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type);
	int (*effect)(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long *level,int *round);
	void (*effect_end)(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round);
	int (*heal)(struct effect *e,struct unit *dest,unsigned long *value);
	void (*heal_end)(struct effect *e,struct unit *dest,unsigned long value);
	void (*kill)(struct effect *e,struct unit *u);
	void (*kill_end)(struct effect *e,struct unit *u);
	void (*roundend)(struct effect *e);
	void (*roundstart)(struct effect *e);
	void (*update_attr)(struct effect *e,struct unit *u);
	void (*update_state)(struct effect *e,struct unit *u,int *state);
	int flag,unused;
};
struct effect {
	const struct effect_base *base;
	struct unit *dest;
	struct unit *src;
	struct effect *next,*prev;
	int round,active;
	long level;
	char data[64];
};

struct event {
	const char *id;
	void (*action)(const struct event *ev,struct unit *src);
};
struct unit {
	const struct unit_base *base;
	unsigned long hp,atk;
	long def;
	unsigned long speed,hit,avoid;
	long spi;
	double crit_effect,
		physical_bonus,magical_bonus,
		physical_derate,magical_derate;
	int type0,type1,state,unused;
	struct move moves[8];
	struct move pmoves[2];
	struct player *owner;
	struct move *move_cur;
};

struct player {
	struct unit units[6];
	int (*selector)(struct player *);
	struct unit *front;
	struct player *enemy;
	struct battle_field *field;
	int action,unused;
};
struct battle_field {
	struct player *p,*e;
	struct effect *effects;
};
int unit_kill(struct unit *up);

unsigned long damage(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type);

unsigned long attack(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type);

int hittest(struct unit *dest,struct unit *src,double hit_rate);

int test(double prob);

double rand01(void);

long randi(void);

unsigned long normal_attack(struct unit *dest,struct unit *src);

unsigned long heal(struct unit *dest,unsigned long value);

void instant_death(struct unit *dest);

unsigned long sethp(struct unit *dest,unsigned long hp);

unsigned long addhp(struct unit *dest,long hp);

long setspi(struct unit *dest,long spi);

struct effect *effect(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round);

int effect_end(struct effect *e);

int purify(struct effect *e);

int revive(struct unit *u,unsigned long hp);

int event(const struct event *ev,struct unit *src);

void effect_event(struct effect *e);

void effect_event_end(struct effect *e);

struct unit *gettarget(struct unit *u);

void update_attr(struct unit *u);

void update_state(struct unit *u);

void unit_cooldown_decrease(struct unit *u,int round);

void unit_effect_in_roundend(struct unit *u);

void effect_in_roundend(struct effect *effects);

void effect_in_roundstart(struct effect *effects);

void unit_effect_round_decrease(struct unit *u,int round);

void effect_round_decrease(struct effect *effects,int round);

int setcooldown(struct move *m,int round);

const char *type2str(int type);

struct effect *unit_findeffect(struct unit *u,const struct effect_base *base);

int effect_isnegative(const struct effect *e);

int unit_hasnegative(struct unit *u);

int unit_move(struct unit *u,struct move *m);

int canaction2(struct player *p,int act);

struct player *getprior(struct player *p,struct player *e);

#endif
