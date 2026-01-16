/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#ifndef _BATTLE_CORE_H_
#define _BATTLE_CORE_H_
#include <stddef.h>
#include <math.h>

#define SHEAR_COEF (M_SQRT2/128)
#define FLOAT_COEF 0.05

#define ROUND_MAX 1024

#define UNIT_NORMAL 0
#define UNIT_CONTROLLED 1
#define UNIT_SUPPRESSED 2
#define UNIT_FADING 3
#define UNIT_FAILED 4
#define UNIT_VANISHED 5
#define UNIT_FREEZING_ROARINGED 6

#define MLEVEL_REGULAR 1
#define MLEVEL_CONCEPTUAL 2
#define MLEVEL_FREEZING_ROARING 4

#define MOVE_NOCONTROL 1
#define MOVE_NORMALATTACK 2

#define DAMAGE_REAL 0
#define DAMAGE_PHYSICAL 1
#define DAMAGE_MAGICAL 2
#define DAMAGE_HEAL 3
#define DAMAGE_SHEAR 4
#define DAMAGE_VOID 5
#define DAMAGE_ADDHP 6
#define DAMAGE_ADDSPI 7
#define DAMAGE_TOTAL (-1)

#define DAMAGE_REAL_FLAG (1<<DAMAGE_REAL)
#define DAMAGE_PHYSICAL_FLAG (1<<DAMAGE_PHYSICAL)
#define DAMAGE_MAGICAL_FLAG (1<<DAMAGE_MAGICAL)
#define DAMAGE_ALL_FLAG (DAMAGE_REAL_FLAG|DAMAGE_PHYSICAL_FLAG|DAMAGE_MAGICAL_FLAG)

#define AF_CRIT (1<<0)
#define AF_DEF (1<<1)
#define AF_EFFECT (1<<2)
#define AF_WEAK (1<<3)
#define AF_NORMAL (1<<4)
#define AF_FLOAT (1<<5)
#define AF_IDEATH (1<<6)
#define AF_INHIBIT (1<<7)
#define DF_KEEPALIVE (1<<8)
#define AF_NONHOOKABLE (1<<9)
#define DF_NONHOOKABLE (1<<10)
#define AF_NODERATE (1<<11)
#define AF_NOCALLBACK (1<<12)
#define DF_NOCALLBACK (1<<13)
#define AF_POSITIVE (1<<14)
#define DF_IGNOREHP (1<<15)
#define DF_TEST (1<<16)
#define AF_TYPE (1<<17)

#define AF_ALL (AF_CRIT|AF_DEF|AF_EFFECT|AF_WEAK|AF_NORMAL|AF_FLOAT|AF_IDEATH|AF_INHIBIT|AF_NONHOOKABLE|AF_NODERATE|AF_NOCALLBACK|AF_POSITIVE|AF_TYPE)
#define DF_ALL (DF_KEEPALIVE|DF_NONHOOKABLE|DF_NOCALLBACK|DF_IGNOREHP|DF_TEST)
#define ADF_ALL (AF_ALL|DF_ALL)

#define ADF_NONHOOKABLE (AF_NONHOOKABLE|DF_NONHOOKABLE)
#define ADF_NOCALLBACK (AF_NOCALLBACK|DF_NOCALLBACK)

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
#define TYPES_WATER_WEAK (TYPE_GRASS|TYPE_WATER|TYPE_LIGHT|TYPE_ELECTRIC|TYPE_DRAGON)

#define TYPES_STEEL_EFFECT (TYPE_GRASS|TYPE_FIGHTING|TYPE_WIND|TYPE_ROCK|TYPE_GHOST|TYPE_ICE|TYPE_BUG|TYPE_SOIL|TYPE_DRAGON)
#define TYPES_STEEL_WEAK (TYPE_WATER|TYPE_STEEL|TYPE_MACHINE)

#define TYPES_LIGHT_EFFECT (TYPE_FIGHTING|TYPE_GHOST|TYPE_BUG)
#define TYPES_LIGHT_WEAK (TYPE_GRASS|TYPE_MACHINE)

#define TYPES_FIGHTING_EFFECT (TYPE_STEEL|TYPE_ROCK|TYPE_ICE|TYPE_MACHINE)
#define TYPES_FIGHTING_WEAK (TYPE_LIGHT|TYPE_FIGHTING|TYPE_WIND|TYPE_POISON|TYPE_BUG|TYPE_SOIL)

#define TYPES_WIND_EFFECT (TYPE_GRASS|TYPE_FIRE|TYPE_GRASS|TYPE_FIGHTING|TYPE_BUG)
#define TYPES_WIND_WEAK (TYPE_STEEL|TYPE_ROCK|TYPE_ELECTRIC|TYPE_MACHINE)

#define TYPES_POISON_EFFECT (TYPE_GRASS|TYPE_FIGHTING|TYPE_BUG)
#define TYPES_POISON_WEAK (TYPE_STEEL|TYPE_POISON|TYPE_ROCK|TYPE_GHOST|TYPE_MACHINE)

#define TYPES_ROCK_EFFECT (TYPE_FIRE|TYPE_WIND|TYPE_DRAGON)
#define TYPES_ROCK_WEAK (TYPE_STEEL|TYPE_ROCK|TYPE_SOIL|TYPE_MACHINE)

#define TYPES_ELECTRIC_EFFECT (TYPE_WATER|TYPE_WIND|TYPE_BUG|TYPE_MACHINE)
#define TYPES_ELECTRIC_WEAK (TYPE_GRASS|TYPE_ROCK|TYPE_ELECTRIC|TYPE_SOIL)

#define TYPES_GHOST_EFFECT (TYPE_GHOST)
#define TYPES_GHOST_WEAK (TYPE_STEEL|TYPE_BUG|TYPE_MACHINE)

#define TYPES_ICE_EFFECT (TYPE_GRASS|TYPE_WIND|TYPE_BUG|TYPE_SOIL)
#define TYPES_ICE_WEAK (TYPE_FIRE|TYPE_STEEL|TYPE_ROCK|TYPE_ICE|TYPE_MACHINE)

#define TYPES_BUG_EFFECT (TYPE_GRASS|TYPE_SOIL)
#define TYPES_BUG_WEAK (TYPE_FIRE|TYPE_STEEL|TYPE_GHOST|TYPE_ICE|TYPE_BUG|TYPE_MACHINE)

#define TYPES_MACHINE_EFFECT (TYPE_GRASS|TYPE_STEEL|TYPE_ROCK)
#define TYPES_MACHINE_WEAK (TYPE_FIRE|TYPE_MACHINE)

#define TYPES_SOIL_EFFECT (TYPE_FIRE|TYPE_POISON|TYPE_ROCK|TYPE_ELECTRIC)
#define TYPES_SOIL_WEAK (TYPE_GRASS|TYPE_STEEL|TYPE_MACHINE|TYPE_SOIL)

#define TYPES_DRAGON_EFFECT (TYPE_DRAGON|TYPES_DEVINE)
#define TYPES_DRAGON_WEAK (TYPE_MACHINE)

#define TYPES_NORMAL_EFFECT (TYPE_VOID)
#define TYPES_NORMAL_WEAK (TYPE_VOID)

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

#define ACTS_MOVE 0x0ff
#define ACTS_MOVE_A 0x1ff
#define ACTS_SWITCHUNIT 0x0fc00

#define ATTR_MAX (+8)
#define ATTR_MIN (-8)

#define EFFECT_ATTR (1<<0)
#define EFFECT_ABNORMAL (1<<1)
#define EFFECT_CONTROL (1<<2)
#define EFFECT_POSITIVE (1<<3)
#define EFFECT_NEGATIVE (1<<4)
#define EFFECT_ENV (1<<5)
#define EFFECT_UNPURIFIABLE (1<<6)
#define EFFECT_ISOLATED (1<<7)
#define EFFECT_KEEP (1<<8)
#define EFFECT_NONHOOKABLE (1<<9)
#define EFFECT_NOCONSTRUCT (1<<10)
#define EFFECT_ADDLEVEL (1<<11)
#define EFFECT_ADDROUND (1<<12)
#define EFFECT_ALLOWFAILED (1<<13)
#define EFFECT_NODESTRUCT (1<<14)
#define EFFECT_OVERRIDESRC (1<<15)
#define EFFECT_FIND (1<<16)
#define EFFECT_REMOVE (1<<17)
#define EFFECT_SELECTALL (1<<18)
#define EFFECT_TEST (1<<19)

#define EFFECT_PASSIVE (EFFECT_POSITIVE|EFFECT_UNPURIFIABLE|EFFECT_KEEP|EFFECT_NONHOOKABLE|EFFECT_ALLOWFAILED)
#define EFFECT_OPTS (EFFECT_FIND|EFFECT_REMOVE)

#define STAGE_INIT 0
#define STAGE_ROUNDSTART 1
#define STAGE_SELECT 2
#define STAGE_PRIOR 3
#define STAGE_LATTER 4
#define STAGE_ROUNDEND 5
#define STAGE_BATTLE_END 6

#define HPMOD_DAMAGE 1
#define HPMOD_HEAL 2
#define HPMOD_SETHP 4
#define HPMOD_SHEAR 8

#define limit(x,inf,sup) (\
{\
		__auto_type _inf=(inf);\
		__auto_type _sup=(sup);\
		__auto_type _x=(x);\
		if(_x>_sup)\
			_x=_sup;\
		if(_x<_inf)\
			_x=_inf;\
		_x;\
}\
)
#define limit_ring(x,inf,sup) (\
{\
		__auto_type _inf=(inf);\
		__auto_type _sup=(sup);\
		__auto_type _x=(x);\
		if(_x>_sup)\
			_x=_inf;\
		if(_x<_inf)\
			_x=_sup;\
		_x;\
}\
)
#define isalive(s) (\
{\
		int _r;\
		switch(s){\
			case UNIT_NORMAL:\
			case UNIT_CONTROLLED:\
			case UNIT_SUPPRESSED:\
				_r=1;\
				break;\
			default:\
				_r=0;\
				break;\
		}\
		_r;\
}\
)

#define isalive_s(s) (\
{\
		int _r;\
		switch(s){\
			case UNIT_NORMAL:\
			case UNIT_CONTROLLED:\
			case UNIT_SUPPRESSED:\
			case UNIT_FADING:\
				_r=1;\
				break;\
			default:\
				_r=0;\
				break;\
		}\
		_r;\
}\
)
#define isvanished(s) (\
{\
		int _r;\
		switch(s){\
			case UNIT_VANISHED:\
			case UNIT_FREEZING_ROARINGED:\
				_r=1;\
				break;\
			default:\
				_r=0;\
				break;\
		}\
		_r;\
}\
)
#define isfront(u) (\
{\
		const struct unit *_u=(u);\
		_u->owner->front==_u;\
}\
)
#define def_coef(d) (\
{\
		int _d=(d);\
		int _x=512+(_d);\
		_x<=0?\
			1.0-_d\
		:\
			512.0/_x;\
}\
)
#define derate_coef(d) (\
{\
		double _d=(d);\
		_d>0.8?1.0/(5*_d+1):1.0-_d;\
}\
)
#define crit_coef(d) (\
{\
		double _k=(d);\
		_k>0.5?_k:1/(3-2*_k);\
}\
)
#define inhibit_coef(s) (\
{\
		double _s=SHEAR_COEF*(s);\
		1.0/(2.0-1.0/(1+_s*_s));\
}\
)
#define effect_weak_coef(level) (\
{\
	long _x;\
	double _r;\
	_x=(level);\
	switch(_x){\
		case 0:\
			_r=1.0;\
			break;\
		case 1:\
			_r=1.5;\
			break;\
		case 2:\
			_r=1.7;\
			break;\
		case -1:\
			_r=0.75;\
			break;\
		case -2:\
			_r=0.6;\
			break;\
		default:\
			_r=_x>0?1.5+0.1*_x:1.0/(1.7-0.1*_x);\
			break;\
	}\
	_r;\
}\
)
#define abs_add(d,v) (\
{\
		__auto_type _d=(d);\
		__typeof(_d) _v=(v);\
		if(_d>=0){\
			if(_v>=0||_d>-_v)\
				_d+=_v;\
			else\
				_d=0;\
		}else {\
			if(_v>=0||_d<_v)\
				_d-=_v;\
			else\
				_d=0;\
		}\
		_d;\
}\
)
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
				_r=TYPE_VOID;\
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
				_r=TYPE_VOID;\
				break;\
		}\
		_r;\
}\
)
#define effect_weak_level(d,s) (\
{\
		int _d=(d),_s=(s);\
		__builtin_popcount(_d&effect_types(_s))\
		-__builtin_popcount(_d&weak_types(_s));\
}\
)
#define effect_weak_flag(l) (\
{\
		int _r=(l);\
		_r?(_r>0?AF_EFFECT:AF_WEAK):0;\
}\
)

#define EFFECT_RECURSION_DEFAULT 8
#define effect_recursion_check(e) (\
{\
		const struct effect *_e=(e);\
		unsigned int _rm=_e->base->recursion_max;\
		_rm?(_e->inevent<_rm):(_e->inevent<EFFECT_RECURSION_DEFAULT);\
}\
)

#define effect_size(_base) (sizeof(struct effect)+(_base)->data_size)

#define for_all_effect(_var,_ehead) for(struct effect *_var=(_ehead),*_next=_var?_var->next:NULL;_next=_var?_var->next:NULL,_var;_var=_var->intrash?_next:_var->next)
#define for_each_effect(_var,_ehead) for_all_effect(_var,_ehead)if(!effect_recursion_check(_var))continue;else

#define for_each_effectf(_var,_ehead,_field) for_each_effect(_var,_ehead)if(!_var->base->_field)continue;else

#define for_each_unit(_var,_player) for(struct unit *_var=(_player)->units,*_p0=_var+6;_var<_p0;++_var)if(!_var->base)continue;else

#define for_each_move(_var,_unit) for(struct move *_var=(_unit)->moves,*_p0=_var+6;_var<_p0;++_var)if(!_var->id)continue;else

#define osite owner->enemy->front

#define unit_effect_level(u,E) (\
{\
		const struct effect *_e=unit_findeffect((u),(E));\
		_e?_e->level:0l;\
}\
)
#define unit_effect_round(u,E) (\
{\
		const struct effect *_e=unit_findeffect((u),(E));\
		_e?_e->round:0;\
}\
)
#define unit_type(u) (\
{\
		const struct unit *_u=(u);\
		_u->type0|_u->type1;\
}\
)
#define effect_field(e) (\
{\
		const struct effect *___e=(e);\
		(___e->dest?___e->dest:___e->src)->owner->field;\
}\
)
#define message_isfailed(msg) (\
{\
		const struct message *_msg=(msg);\
		_msg->type==MSG_STATEMOD\
		&&!isalive_s(_msg->un.statemod.state)\
		&&isalive_s(_msg->un.statemod.old);\
}\
)
enum {
	MSG_ACTION=0,
	MSG_BATTLE_END,
	MSG_DAMAGE,
	MSG_EFFECT,
	MSG_EFFECT_END,
	MSG_EFFECT_EVENT,
	MSG_EFFECT_EVENT_END,
	MSG_EFFECT_ROUNDDEC,
	MSG_EVENT,
	MSG_EVENT_END,
	MSG_STATEMOD,
	MSG_HEAL,
	MSG_HPMOD,
	MSG_MISS,
	MSG_MOVE,
	MSG_MOVE_END,
	MSG_ROUND,
	MSG_ROUNDEND,
	MSG_SPIMOD,
	MSG_SWITCH,
	MSG_UPDATE
};

#define event_start(ev,src) (\
{\
		struct unit *__src=(src);\
		report(__src->owner->field,MSG_EVENT,(ev),__src);\
}\
)
#define event_end(ev,src) (\
{\
		struct unit *__src=(src);\
		report(__src->owner->field,MSG_EVENT_END,(ev),__src);\
}\
)
#define event_do(ev,src) for(void *_src=(src),*_ev=(void *)(ev);_src&&(event_start(_ev,_src),1);event_end(_ev,_src),_src=NULL)

#define effect_event(e) (\
{\
	struct effect *__e=(e);\
	++__e->inevent;\
	report(effect_field(__e),MSG_EFFECT_EVENT,__e);\
}\
)
#define effect_event_end(e) (\
{\
	struct effect *__e=(e);\
	--__e->inevent;\
	report(effect_field(__e),MSG_EFFECT_EVENT_END,__e);\
}\
)
#define effect_ev(e) for(void *_e=(e);_e&&(effect_event(_e),1);effect_event_end(_e),_e=NULL)

struct unit;
struct player;
struct battle_field;
struct effect;
struct message;
struct move {
	const char *id;
	void (*action)(struct unit *);
	void (*init)(struct unit *);
	int (*fprior)(const struct unit *);
	int (*available)(struct unit *u,struct move *m);
	int type,mlevel,prior,cooldown,flag,unused;
};
struct unit_base {
	const char *id;
	unsigned long max_hp,atk;
	long def;
	unsigned long speed,hit,avoid;
	long max_spi;
	double crit_effect,
		physical_bonus,magical_bonus,
		physical_derate,magical_derate;
	int type0,type1,level,unused;
	struct move moves[8];
};
struct damage_type {
	double (*derate_eval)(const struct unit *dest,const struct unit *src);
	void (*action)(struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type,void *arg);
	size_t index;
	int xflag,hpmod_flag;
};
struct event {
	const char *id;
	void (*action)(const struct event *ev,struct unit *src);
};
struct effect_base {
	const char *id;
	int (*init)(struct effect *e,long level,int round);
	void (*inited)(struct effect *e);
	void (*end)(struct effect *e);
	int (*end_in_roundend)(struct effect *e);
	int (*action)(struct effect *e,struct player *p);
	void (*action_end)(struct effect *e,struct player *p);
	void (*action_fail)(struct effect *e,struct player *p);
	int (*attack)(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type,void **arg);
	int (*attack0)(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type,void **arg);
	void (*attack_end)(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type,void *arg);
	void (*attack_end0)(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type,void *arg);
	void (*cooldown_decrease)(struct effect *e,struct unit *u,struct move *m,int *round);
	int (*damage)(struct effect *e,struct unit *dest,struct unit *src,long *value,int *damage_type,int *aflag,int *type,void **arg);
	void (*damage_end)(struct effect *e,struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type,void *arg);
	int (*effect)(struct effect *e,const struct effect_base *base,struct unit *dest,struct unit *src,long *level,int *round,int *xflag);
	void (*effect_end)(struct effect *e,struct effect *ep,struct unit *dest,struct unit *src,long level,int round);
	void (*effect_end0)(struct effect *e,struct effect *ep,struct unit *dest,struct unit *src,long level,int round);
	void (*effect_endt)(struct effect *e,struct effect *ep);
	int (*event)(struct effect *e,const struct event *ev,struct unit *src,void *arg);
	struct unit *(*gettarget)(struct effect *e,struct unit *u);
	int (*getprior)(struct effect *e,struct player *p);
	int (*heal)(struct effect *e,struct unit *dest,long *value);
	void (*heal_end)(struct effect *e,struct unit *dest,long value);
	int (*hittest)(struct effect *e,struct unit *dest,struct unit *src,double *hit_rate);
	void (*hittest_end)(struct effect *e,struct unit *dest,struct unit *src,int hit);
	void (*hpmod)(struct effect *e,struct unit *dest,long hp,int flag);
	int (*kill)(struct effect *e,struct unit *u);
	void (*kill_end)(struct effect *e,struct unit *u);
	int (*move)(struct effect *e,struct unit *u,struct move *m);
	void (*move_end)(struct effect *e,struct unit *u,struct move *m);
	int (*purify)(struct effect *e,struct effect *ep);
	int (*revive)(struct effect *e,struct unit *u,unsigned long *hp);
	void (*revive_end)(struct effect *e,struct unit *u,unsigned long hp);
	void (*roundend)(struct effect *e);
	void (*roundstart)(struct effect *e);
	void (*setcooldown)(struct effect *e,struct unit *u,struct move *m,int *round);
	void (*setcooldown_end)(struct effect *e,struct unit *u,struct move *m,int round);
	int (*setwinner)(struct effect *e,struct player *p);
	void (*spimod)(struct effect *e,struct unit *dest,long spi);
	void (*statemod)(struct effect *e,struct unit *u,int old);
	int (*switchunit)(struct effect *e,struct unit *to);
	void (*switchunit_end)(struct effect *e,struct unit *from);
	void (*update_attr)(struct effect *e,struct unit *u);
	void (*update_state)(struct effect *e,struct unit *u,int *state);
	int flag,prior;
	unsigned int recursion_max,data_size;
};
struct effect {
	const struct effect_base *base;
	struct unit *dest;
	struct unit *src,*src1;
	struct effect *next,*prev;
	int round,unused;
	unsigned int active:1,intrash:1,:0;
	unsigned int inevent;
	long level;
	char data[];
};

struct unit {
	unsigned long hp,atk,max_hp;
	long def;
	unsigned long speed,hit,avoid;
	long spi,max_spi;
	double crit_effect,
		physical_bonus,magical_bonus,
		physical_derate,magical_derate;
	int type0,type1,state,level,blockade,unused;
	struct move moves[8];
	const struct unit_base *base;
	struct player *owner;
	struct move *move_cur;
};

struct player {
	struct unit units[6];
	int (*selector)(const struct player *);
	void (*reporter)(const struct message *msg,const struct player *p);
	struct unit *front;
	struct player *enemy;
	struct battle_field *field;
	unsigned int move_recursion;
	int action,arg;
	unsigned int acted:1,acting:1,:0;
	char data[64];
};
struct history {
	struct player p,e;
	struct effect *effects;
};
struct message {
	int type,round;
	ptrdiff_t index;
	const struct battle_field *field;
	union {
		struct {
			const struct unit *dest,*src;
			long value;
			int damage_type,aflag,type,unused;
			unsigned long oldhp;
			void *arg;
		} damage;
		const struct effect *e;
		struct {
			const struct effect *e;
			const struct effect_base *base;
			long level;
			int round;
		} e_init;
		struct {
			const struct event *ev;
			const struct unit *src;
		} event;
		struct {
			const struct unit *dest;
			long value;
			unsigned long oldhp;
		} heal;
		struct {
			const struct unit *dest;
			long value;
			unsigned long old;
		} hpmod;
		struct {
			const struct unit *u;
			const struct move *m;
		} move;
		const struct player *p;
		struct {
			const struct unit *dest;
			long value,old;
		} spimod;
		struct {
			const struct unit *dest;
			int state,old;
		} statemod;
		const struct unit *u;
		struct {
			const struct unit *dest,*src;
		} u2;
		const void *uaddr;
	} un;
};

struct battle_field {
	struct player *p,*e,*winner;
	struct effect *effects,*trash;
	struct message *rec;
	size_t rec_size,rec_length;
	struct history *ht;
	size_t ht_size,ht_length;
	const volatile int *round;
	const volatile int *stage;
	int end_round,unused;
};
extern const struct damage_type damage_types[];
extern size_t damage_types_size;
extern const struct event spi_modified[];

int unit_setstate(struct unit *u,int state);

int unit_kill(struct unit *up);

long damage(struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type,const void *arg);

long attack(struct unit *dest,struct unit *src,long value,int damage_type,int aflag,int type,const void *arg);

int hittest(struct unit *dest,struct unit *src,double hit_rate);

int test(double prob);

double rand01(void);

long randi(void);

void normal_attack(struct unit *src);

long heal3(struct unit *dest,long value,int aflag);

long heal(struct unit *dest,long value);

void sethp(struct unit *dest,unsigned long hp);

void addhp(struct unit *dest,long hp);

void setspi(struct unit *dest,long spi);

void addspi(struct unit *dest,long spi);

struct effect *effect(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round);

struct effect *effectx(const struct effect_base *base,struct unit *dest,struct unit *src,long level,int round,int xflag);

int effect_reinit(struct effect *ep,struct unit *src,long level,int round);

int effect_reinitx(struct effect *ep,struct unit *src,long level,int round,int xflag);

int effect_setlevel(struct effect *e,long level);

int effect_addlevel(struct effect *e,long level);

int effect_setround(struct effect *e,int round);

int effect_end(struct effect *e);

int effect_final(struct effect *e);

void effect_freeall(struct effect **head);

void wipetrash(struct battle_field *f);

struct effect *effect_copyall(struct effect *head);

int purify(struct effect *e);

int unit_wipeeffect(struct unit *u,int mask);

int revive(struct unit *u,unsigned long hp);

int revive_nonhookable(struct unit *u,unsigned long hp);

int event_callback(const struct event *ev,struct unit *src,void *arg);

int event(const struct event *ev,struct unit *src);

struct unit *gettarget(struct unit *u);

void unit_fillattr(struct unit *u);

void update_attr_init(struct unit *u);

void update_attr(struct unit *u);

void update_attr_all(struct battle_field *f);

void update_state(struct unit *u);

void unit_cooldown_decrease(struct unit *u,int round);

void unit_effect_in_roundend(struct unit *u);

void effect_in_roundend(struct effect *effects);

void effect_in_roundstart(struct effect *effects);

void effect_round_decrease(struct effect *effects,int round);

int setcooldown(struct unit *u,struct move *m,int round);

struct effect *unit_findeffect(const struct unit *u,const struct effect_base *base);

struct effect *unit_findeffectf(const struct unit *u,int flag);

struct effect *findeffect(const struct effect *head,const struct effect_base *base);

int effect_isnegative_base(const struct effect_base *base,const struct unit *dest,const struct unit *src,long level);

int effect_isnegative(const struct effect *e);

int effect_ispositive_base(const struct effect_base *base,const struct unit *dest,const struct unit *src,long level);

int effect_ispositive(const struct effect *e);

int unit_hasnegative(const struct unit *u);

int unit_move_nonhookable(struct unit *u,struct move *m);

int unit_move(struct unit *u,struct move *m);

void unit_move_init(struct unit *u,struct move *m);

int switchunit(struct unit *to);

int canaction2(const struct player *p,int act);

int player_select(struct player *p);

void player_action(struct player *p);

int unit_reap_fading(struct unit *u);

void reap_fading(struct battle_field *f);

struct player *getprior(struct player *p,struct player *e);

void report(struct battle_field *f,int type,...);

const struct message *message_find(const struct battle_field *field,int types);

const struct message *message_findsource(const struct message *msg);

void history_add(struct battle_field *f);

void field_free(struct battle_field *field);

int player_hasunit(struct player *p);

const struct player *getwinner(struct battle_field *f);

const struct player *getwinner_nonnull(struct battle_field *f);

int setwinner(struct battle_field *f,struct player *p);

#endif
