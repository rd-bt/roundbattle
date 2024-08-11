#ifndef _BATTLE_CORE_H_
#define _BATTLE_CORE_H_

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

#define AF_CIRT 1
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

#define ABNORMAL_BURNT 1
#define ABNORMAL_POISONED 2
#define ABNORMAL_PARASITIZED 4
#define ABNORMAL_CURSED 8
#define ABNORMAL_RADIATED 16
#define ABNORMAL_ASLEEP 32
#define ABNORMAL_FROZEN 64
#define ABNORMAL_PARALYSED 256
#define ABNORMAL_STUNNED 512
#define ABNORMAL_PETRIFIED 1024
#define ABNORMAL_CONTROL (ABNORMAL_ASLEEP|ABNORMAL_FROZEN|ABNORMAL_PARALYSED|ABNORMAL_STUNNED|ABNORMAL_PETRIFIED)
#define ABNORMAL_ALL 2047

#define ATTR_ATK 1
#define ATTR_DEF 2
#define ATTR_SPEED 4
#define ATTR_HIT 8
#define ATTR_AVOID 16
#define ATTR_CIRTEFFECT 32
#define ATTR_PBONUS 64
#define ATTR_MBONUS 128
#define ATTR_PDERATE 256
#define ATTR_MDERATE 512
#define ATTR_MAX (+8)
#define ATTR_MIN (-8)
struct unit;
struct player;
struct move {
	const char *id,*name;
	void (*action)(struct unit *subject,int arg);
	int type,mlevel,prior,cooldown,flag,unused;
};
struct unit_base {
	const char *name;
	unsigned long max_hp,atk;
	long def;
	unsigned long speed,hit,avoid,max_spi;
	double cirt_effect,
		physical_bonus,magical_bonus,
		physical_derate,magical_derate;
	int type0,type1,level,unused;
	struct move moves[8];
	struct move pmoves[2];
};
struct attr {
	int atk,def,speed,hit,avoid,
	    cirt_effect,
	    physical_bonus,magical_bonus,
	    physical_derate,magical_derate;
};
struct abnormal {
	int burnt,poisoned,parasitized,cursed,radiated;//controlled
	int asleep,frozen,paralysed,stunned,petrified;
};

struct unit {
	struct unit_base base;
	unsigned long hp,atk;
	long def;
	unsigned long speed,hit,avoid;
	long spi;
	double cirt_effect,
		physical_bonus,magical_bonus,
		physical_derate,magical_derate;
	//struct effect *effects;
	int type0,type1,state,unused;
	struct move moves[8];
	struct move pmoves[2];
	struct attr attrs;
	struct abnormal abnormals;
	struct player *owner;
	struct move *move_cur;
};
struct player {
	struct unit units[6];
	struct unit *front;
	struct player *enemy;
	int (*selector)(struct player *);
	int action,unused;
};

int unit_kill(struct unit *up);

unsigned long damage(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type);

unsigned long attack(struct unit *dest,struct unit *src,unsigned long value,int damage_type,int aflag,int type);

int hittest(struct unit *dest,struct unit *src,double hit_rate);

int test(double prob);

unsigned long normal_attack(struct unit *dest,struct unit *src);

unsigned long heal(struct unit *dest,unsigned long value);

unsigned long sethp(struct unit *dest,unsigned long hp);

unsigned long addhp(struct unit *dest,long hp);

struct unit *gettarget(struct unit *u);

void unit_abnormal(struct unit *u,int abnormals,int round);

void unit_abnormal_purify(struct unit *u,int abnormals);
void unit_attr_set_force(struct unit *u,int attrs,int level);

void unit_attr_set(struct unit *u,int attrs,int level);

void unit_update_attr(struct unit *u);

void unit_state_correct(struct unit *u);

void unit_cooldown_decrease(struct unit *u,int round);

void unit_effect_in_roundend(struct unit *u);

void unit_effect_round_decrease(struct unit *u,int round);

int setcooldown(struct move *m,int round);

const char *type2str(int type);


#endif
