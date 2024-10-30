#include "battle.h"
#include "moves.h"
#include "locale.h"
#include "strmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <alloca.h>
#include <stdarg.h>
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define CYAN "\033[96m"
#define WHITE "\033[39m"
#define GREEN_BG "\033[42m"
#define RED_BG "\033[41m"
#define WHITE_BG "\033[47m"
#define BLACK_BG "\033[40m"
#define putn(c,n) for(unsigned short _i=(n),_c=(c);_i>0;--_i)fputc(_c,fp)
#define azero(a) memset((a),0,sizeof(a))
#define REC_SIZE 64
static char rec[REC_SIZE][129];

size_t strrlen(const char *s){
	size_t r=0;
	while(*s){
		if(*s=='\033'&&s[1]=='['&&s[2]&&s[3]&&s[4]=='m')
			s+=5;
		else {
			if(*s&128){
				unsigned int n=__builtin_clz(~(*s<<(8*sizeof(unsigned int)/sizeof(char)-8)));
				//int n=3;
				//fprintf(stderr,"__builtin_clz(~*s)=%d\n",__builtin_clz(~(*s<<24)));
				s+=n;
				r+=2;
			}else {
				++s;
				++r;
			}
		}
	}
	return r;
}
void wm(int who,const char msg[128]){
	int i=0;
	for(i=0;i<REC_SIZE;++i){
		if(!rec[i][0])
			break;
	}
	if(i==REC_SIZE){
		/*for(i=0;i<14;++i){
			memcpy(rec[i],rec[i+1],129);
		}
		i=15;*/
		azero(rec);
		i=0;
	}
	//fprintf(stderr,"set %d :%s\n",i,msg);
	memcpy(rec[i],msg,127);
	rec[i][127]=0;
	rec[i][128]=(char)who;
}
void wmf(int who,const char *fmt,...){
	va_list ap;
	char buf[128];
	va_start(ap,fmt);
	vsnprintf(buf,128,fmt,ap);
	wm(who,buf);
	va_end(ap);
}

static const char *sstr[5]={"normal","controlled","spuuressed","failed","freezing_roaringed"};
void frash(const struct player *p,FILE *fp,int current){
	struct player *e=p->enemy;
	struct unit *u;
	struct battle_field *f=p->field;
	unsigned short s,n,s1,line=0;
	struct winsize ws;
	char *buf;
	size_t buflen,sr;
	int r,r1,c;
	int found0=0,found1=0,cf0,cf1;
	fputs("\033c",fp);
	fprintf(fp,"ROUND %d\n",*f->round);
	++line;
	ioctl(STDIN_FILENO,TIOCGWINSZ,&ws);
	s=(ws.ws_col-3)/2;
	putn('-',s);
	putn(' ',ws.ws_col-2*s);
	putn('-',s);
	fputs("\n|",fp);
	++line;
	u=p->front;
	n=(u->hp<u->base->max_hp/2?ceil:floor)((s-2)*(double)u->hp/u->base->max_hp);
	fputs(RED_BG,fp);
	putn('=',n);
	fputs(BLACK_BG,fp);
	putn(' ',s-2-n);
	fputc('|',fp);
	putn(' ',ws.ws_col-2*s);
	fputc('|',fp);
	u=e->front;
	n=(u->hp<u->base->max_hp/2?ceil:floor)((s-2)*(double)u->hp/u->base->max_hp);
	fputs(RED_BG,fp);
	putn('=',n);
	fputs(BLACK_BG,fp);
	putn(' ',s-2-n);
	fputs("|\n",fp);
	++line;
	putn('-',s);
	putn(' ',ws.ws_col-2*s);
	putn('-',s);
	fputc('\n',fp);
	++line;
	buf=alloca(buflen=(ws.ws_col+16)&~15);
	--buflen;
	buf[buflen]=0;
#define print_attr(...) \
	s1=ws.ws_col;\
	u=p->front;\
	snprintf(buf,buflen,__VA_ARGS__);\
	sr=strrlen(buf);\
	if(s1>=sr){\
		s1-=sr;\
		fputs(buf,fp);\
	}\
	u=e->front;\
	snprintf(buf,buflen,__VA_ARGS__);\
	sr=strrlen(buf);\
	if(s1>=sr){\
		s1-=sr;\
		putn(' ',s1);\
	}else\
		fputc('\n',fp);\
	fputs(buf,fp);\
	fputc('\n',fp);\
	++line
	print_attr("%s%s%s [%ld]%s",type2str(u->type0),u->type1?"/":"",u->type1?type2str(u->type1):"",u-u->owner->units,unit_ts(u->base->id));
	print_attr("%lu/%lu %.2lf%%",u->hp,u->base->max_hp,100.0*u->hp/u->base->max_hp);
	print_attr("%s:%d %s:%lu %s:%ld",ts("level"),u->level,ts("atk"),u->atk,ts("def"),u->def);
	print_attr("%s:%lu %s:%lu %s:%lu",ts("speed"),u->speed,ts("hit"),u->hit,ts("avoid"),u->avoid);
	if(p->front->crit_effect!=2.0||e->front->crit_effect!=2.0){
		print_attr("%s:%.2lf%%",ts("crit_effect"),100*u->crit_effect);
	}
	if(p->front->physical_bonus!=0.0||e->front->physical_bonus!=0.0){
		print_attr("%s:%.2lf%%",ts("physical_bonus"),100*u->physical_bonus);
	}
	if(p->front->magical_bonus!=0.0||e->front->magical_bonus!=0.0){
		print_attr("%s:%.2lf%%",ts("magical_bonus"),100*u->magical_bonus);
	}
	if(p->front->physical_derate!=0.0||e->front->physical_derate!=0.0){
		print_attr("%s:%.2lf%%",ts("physical_derate"),100*u->physical_derate);
	}
	if(p->front->magical_derate!=0.0||e->front->magical_derate!=0.0){
		print_attr("%s:%.2lf%%",ts("magical_derate"),100*u->magical_derate);
	}
	if(p->front->spi||e->front->spi){
		print_attr("%s:%ld/%ld %.2lf%%",ts("spi_force"),u->spi,u->base->max_spi,100.0*u->spi/u->base->max_spi);
	}
	for(struct effect *e0,*e1;;){
		cf0=0;
		cf1=0;
		e0=NULL;
		e1=NULL;
		for_each_effect(ep,f->effects){
			if(ep->dest==p->front&&ep->base->id){
				if(cf0++<found0)
					continue;
				e0=ep;
				++found0;
				break;
			}
		}
		for_each_effect(ep,f->effects){
			if(ep->dest==e->front&&ep->base->id){
				if(cf1++<found1)
					continue;
				e1=ep;
				++found1;
				break;
			}
		}
		//printf("e0=%p e1=%p found0=%d found1=%d\n",e0,e1,found0,found1);
		if(!e0&&!e1)break;
		s1=ws.ws_col;
#define peffect(e0) \
			r=snprintf(buf,buflen,"%s",e2s(e0->base->id));\
			if(e0->level){\
				r1=snprintf(buf+r,buflen-r,"(%+ld)",e0->level);\
				r+=r1;\
			}\
			if(e0->round>=0){\
				snprintf(buf+r,buflen-r,"[%d]",e0->round);\
			}\
			s1-=strrlen(buf)
		if(e0){
			peffect(e0);
			if(e0->inevent)
				fputs(YELLOW,fp);
			else if(effect_isnegative(e0))
				fputs(BLUE,fp);
			fputs(buf,fp);
			fputs(WHITE,fp);
		}
		if(e1){
			peffect(e1);
			putn(' ',s1);
			if(e1->inevent)
				fputs(YELLOW,fp);
			else if(effect_isnegative(e1))
				fputs(BLUE,fp);
			fputs(buf,fp);
			fputs(WHITE,fp);
		}
		fputc('\n',fp);
		++line;
	}
	for_each_effect(ep,f->effects){
		if(ep->dest||!ep->base->id)
			continue;
		peffect(ep);
		if(ep->base->flag&EFFECT_ENV){
			fputs(ep->inevent?YELLOW:CYAN,fp);
			fprintf(fp,"%s:%s\n",ts("environment"),buf);
		}else {
			if(ep->inevent)
				fputs(YELLOW,fp);
			fprintf(fp,"%s:%s\n",ts("global"),buf);
		}
		fputs(WHITE,fp);
		++line;
	}
	putn('-',ws.ws_col);
	fputc('\n',fp);
	++line;
	/*switch(*p->field->stage){
		case STAGE_INIT:
		case STAGE_BATTLE_END:
			break;
		default:
			break;
	}*/

	for(c=0;c<REC_SIZE&&rec[c][0];++c);
	c+=line+11;
	for(int i=ws.ws_row<c?c-ws.ws_row:0;i<REC_SIZE&&rec[i][0];++i){
		if(rec[i][128]){
			print_attr("%s",u==p->front?"":rec[i]);
		}else {
			print_attr("%s",u==e->front?"":rec[i]);
		}
	}
	u=p->front;
	if(ws.ws_row>line+11)
		putn('\n',ws.ws_row-line-11);
	putn('-',ws.ws_col);
	fputc('\n',fp);
	c=canaction2(p,ACT_NORMALATTACK);
	r=snprintf(buf,buflen,"(%s)%s",type2str(u->type0),move_ts("normal_attack"));
	snprintf(buf+r,buflen-r,"%s",c?"":"[X]");
	r=strrlen(buf);
	if(current==ACT_NORMALATTACK)
		fputs(c?GREEN:RED,fp);
	fputs(buf,fp);
	if(current==ACT_NORMALATTACK)
		fputs(WHITE,fp);
	switch(current){
		case ACT_MOVE0 ... ACT_MOVE7:
			r1=snprintf(buf,buflen,"prior:%d",u->moves[current].prior);
			break;
		case ACT_UNIT0 ... ACT_UNIT5:
			r1=p->units[current-ACT_UNIT0].base?
				snprintf(buf,buflen,"state:%s",sstr[p->units[current-ACT_UNIT0].state])
				:0;
			break;
		case ACT_NORMALATTACK:
			r1=snprintf(buf,buflen,"prior:0");
			break;
		default:
			r1=0;
			break;
	}
	if(ws.ws_col>r+r1)
		putn(' ',ws.ws_col-r-r1);
	if(r1){
		fputs(canaction2(p,current)?GREEN:RED,fp);
		fputs(buf,fp);
		fputs(WHITE,fp);
	}
	fputs("\n",fp);
	for(int i=0;i<8;++i){
		int color;
		s1=ws.ws_col;
		if(!u->moves[i].id){
			snprintf(buf,buflen,"(%s)",ts("no_move"));
			c=0;
			goto no_move;
		}
		c=canaction2(p,i);
		r=snprintf(buf,buflen,"(%s)%s",type2str(u->moves[i].type),move_ts(u->moves[i].id));
		if(u->moves[i].cooldown)
			r1=u->moves[i].cooldown<0?snprintf(buf+r,buflen-r,"[inf]"):snprintf(buf+r,buflen-r,"[%d]",u->moves[i].cooldown);
		else if(!c&&u->moves[i].action){
				strncpy(buf+r,"[X]",buflen-r);
		}
no_move:
		s1-=strrlen(buf);
		if(current==i)
			fputs(c?GREEN:RED,fp);
		fputs(buf,fp);
		if(current==i)
			fputs(WHITE,fp);
		switch(i){
			case 0 ... 5:
				color=current==i+ACT_UNIT0;
				if(!p->units[i].base){
					snprintf(buf,buflen,"(%s)",ts("no_unit"));
					c=0;
					goto no_unit;
				}
				r=snprintf(buf,buflen,"(%s%s%s)%s %lu/%lu",type2str(p->units[i].type0),p->units[i].type1?"/":"",p->units[i].type1?type2str(p->units[i].type1):"",unit_ts(p->units[i].base->id),p->units[i].hp,p->units[i].base->max_hp);
				c=canaction2(p,i+ACT_UNIT0);
				switch(p->units[i].state){
					case UNIT_NORMAL:
						if(p->units+i==p->front)
							strncpy(buf+r,"[B]",buflen-r);
						break;
					case UNIT_CONTROLLED:
					case UNIT_SUPPRESSED:
						strncpy(buf+r,"[C]",buflen-r);
						break;
					case UNIT_FAILED:
						strncpy(buf+r,"[X]",buflen-r);
						break;
					case UNIT_FREEZING_ROARINGED:
						strncpy(buf+r,"[F]",buflen-r);
						break;
				}
				break;
			case 6:
				color=current==ACT_ABORT;
				c=1;
				strncpy(buf,ts("abort"),buflen);
				break;
			case 7:
				color=current==ACT_GIVEUP;
				c=1;
				strncpy(buf,ts("give_up"),buflen);
				break;
		}

no_unit:
		s1-=strrlen(buf);
		putn(' ',s1);
		if(color)
			fputs(c?GREEN:RED,fp);
		fputs(buf,fp);
		if(color)
			fputs(WHITE,fp);
		fputc('\n',fp);
	}
	fflush(fp);
	if(*f->stage==STAGE_BATTLE_END){
		usleep(800000);
		//read(STDIN_FILENO,buf,buflen);
		azero(rec);
	}
}
static struct termios tm0;
int tm_set=0;
void tm_init(void){
	struct termios tm;
	if(tm_set||tcgetattr(STDIN_FILENO,&tm)<0)
		goto fail;
	memcpy(&tm0,&tm,sizeof(struct termios));
	tm.c_lflag&=~(ICANON|ECHO);
	tm.c_cc[VMIN]=1;
	tm.c_cc[VTIME]=0;
	if(tcsetattr(STDIN_FILENO,TCSANOW,&tm)<0)
		goto fail;
	tm_set=1;
	return;
fail:
	fputs("CANNOT SET TERMINAL\n",stderr);
	abort();
}
void tm_end(void){
	if(tm_set)
		tcsetattr(STDIN_FILENO,TCSANOW,&tm0);
	tm_set=0;
}
static int cur=ACT_NORMALATTACK;
//static const int dtc[3]={'R','P','M'};
static const char *dtco[3]={YELLOW,RED,CYAN};
void reporter_term(const struct message *msg,const struct player *p){
	char buf[128];
	char buf1[128];
	int r;
	if(*msg->field->stage==STAGE_BATTLE_END&&msg->type!=MSG_BATTLE_END)
		return;
	switch(msg->type){
		case MSG_ACTION:
		case MSG_UPDATE:
			break;
		case MSG_BATTLE_END:
			wmf(msg->un.p==p?0:1,"WIN");
			break;
		case MSG_DAMAGE:
			if(msg->un.damage.damage_type==DAMAGE_TOTAL){
				wmf(msg->un.damage.dest->owner==p?0:1,"%s -%lu",ts("total"),msg->un.damage.value);
			}else {
				buf[0]=0;
				buf1[0]=0;
				if(msg->un.damage.dest!=msg->un.damage.dest->owner->front){
					snprintf(buf1,128,"[%ld]%s ",msg->un.damage.dest-msg->un.damage.dest->owner->units,unit_ts(msg->un.damage.dest->base->id));
				}
				if(msg->un.damage.aflag&AF_CRIT)
					strcat(buf," C");
				if(msg->un.damage.aflag&AF_EFFECT)
					strcat(buf," E");
				if(msg->un.damage.aflag&AF_WEAK)
					strcat(buf," W");
				wmf(msg->un.damage.dest->owner==p?0:1,"%s%s-%lu%s" WHITE,buf1,dtco[msg->un.damage.damage_type],msg->un.damage.value,buf);
			}
			goto delay;
		case MSG_EFFECT:
			if(*msg->field->stage==STAGE_INIT)
				break;
			if(msg->un.e->dest&&msg->un.e->base->id){
				if(!isalive(msg->un.e->dest->state))
					break;
				buf[0]=0;
				buf[127]=0;
				r=0;
				buf1[0]=0;
				if(msg->un.e->dest!=msg->un.e->dest->owner->front){
					snprintf(buf1,128,"[%ld]%s ",msg->un.e->dest-msg->un.e->dest->owner->units,unit_ts(msg->un.e->dest->base->id));
				}
				if(msg->un.e_init.level)
					snprintf(buf+r,127-r," %+ld",msg->un.e_init.level);
				wmf(msg->un.e->dest->owner==p?0:1,"%s" YELLOW "%s%s" WHITE,buf1,e2s(msg->un.e->base->id),buf);
				goto delay;
			}
			break;
		case MSG_EFFECT_END:
			if(msg->un.e->dest&&msg->un.e->base->id){
				if(!isalive(msg->un.e->dest->state))
					break;
				buf[0]=0;
				buf1[0]=0;
				if(msg->un.e->dest!=msg->un.e->dest->owner->front){
					snprintf(buf1,128,"[%ld]%s ",msg->un.e->dest-msg->un.e->dest->owner->units,unit_ts(msg->un.e->dest->base->id));
				}
				wmf(msg->un.e->dest->owner==p?0:1,"%s" YELLOW "%s%s" WHITE " %s",buf1,e2s(msg->un.e->base->id),buf,ts("end"));
				goto delay;
			}
			break;
		case MSG_EFFECT_EVENT:
			break;
		case MSG_EFFECT_EVENT_END:
			break;
		case MSG_EVENT:
			break;
		case MSG_EVENT_END:
			break;
		case MSG_FAIL:
			wmf(msg->un.u->owner==p?0:1,ts("failed"));
			break;
		case MSG_HEAL:
			buf1[0]=0;
			if(msg->un.heal.dest!=msg->un.heal.dest->owner->front){
				snprintf(buf1,128,"[%ld]%s ",msg->un.heal.dest-msg->un.heal.dest->owner->units,unit_ts(msg->un.heal.dest->base->id));
			}
			wmf(msg->un.heal.dest->owner==p?0:1,"%s" GREEN "+%lu" WHITE,buf1,msg->un.heal.value);
			goto delay;
		case MSG_HPMOD:
			buf1[0]=0;
			if(msg->un.hpmod.dest!=msg->un.hpmod.dest->owner->front){
				snprintf(buf1,128,"[%ld]%s ",msg->un.hpmod.dest-msg->un.hpmod.dest->owner->units,unit_ts(msg->un.hpmod.dest->base->id));
			}
			wmf(msg->un.hpmod.dest->owner==p?0:1,"%s%+ld",buf1,msg->un.hpmod.value);
			goto delay;
		case MSG_MISS:
			wmf(msg->un.u2.dest->owner==p?0:1,ts("miss"));
			goto delay;
		case MSG_MOVE:
			buf1[0]=0;
			if(msg->un.move.u!=msg->un.move.u->owner->front){
				snprintf(buf1,128,"[%ld]%s ",msg->un.move.u-msg->un.move.u->owner->units,unit_ts(msg->un.move.u->base->id));
			}
			wmf(msg->un.move.u->owner==p?0:1,"%s" CYAN "%s" WHITE,buf1,move_ts(msg->un.move.m->id));
			goto delay;
		case MSG_ROUND:
			break;
		case MSG_ROUNDEND:
			wmf(0,ts("round_end"));
			break;
		case MSG_SPIMOD:
			buf1[0]=0;
			if(msg->un.spimod.dest!=msg->un.spimod.dest->owner->front){
				snprintf(buf1,128,"[%ld]%s ",msg->un.spimod.dest-msg->un.spimod.dest->owner->units,unit_ts(msg->un.spimod.dest->base->id));
			}
			wmf(msg->un.spimod.dest->owner==p?0:1,"%s%+ld spi",buf1,msg->un.spimod.value);
			goto delay;
		case MSG_SWITCH:
			wmf(msg->un.u2.dest->owner==p?0:1,"[%ld]%s",msg->un.u2.dest-msg->un.u2.dest->owner->units,unit_ts(msg->un.u2.dest->base->id));
			break;
		default:
			break;
	}
	frash(msg->field->p,stdout,-1);
	return;
delay:
	frash(msg->field->p,stdout,-1);
	usleep(200000);
}
void print_unit(const struct unit *u){
	int neg;
	fprintf(stdout,"(%s%s%s) %s[%ld] %lu/%lu %.2lf%% %s\n",type2str(u->type0),u->type1?"/":"",u->type1?type2str(u->type1):"",unit_ts(u->base->id),u-u->owner->units,u->hp,u->base->max_hp,100.0*u->hp/u->base->max_hp,sstr[u->state]);
	fprintf(stdout,"%s:%d %s:%lu %s:%ld\n",ts("level"),u->level,ts("atk"),u->atk,ts("def"),u->def);
	fprintf(stdout,"%s:%lu %s:%lu %s:%lu\n",ts("speed"),u->speed,ts("hit"),u->hit,ts("avoid"),u->avoid);
	if(u->crit_effect!=2.0){
		fprintf(stdout,"%s:%.2lf%%\n",ts("crit_effect"),100*u->crit_effect);
	}
	if(u->physical_bonus!=0.0){
		fprintf(stdout,"%s:%.2lf%%\n",ts("physical_bonus"),100*u->physical_bonus);
	}
	if(u->magical_bonus!=0.0){
		fprintf(stdout,"%s:%.2lf%%\n",ts("magical_bonus"),100*u->magical_bonus);
	}
	if(u->physical_derate!=0.0){
		fprintf(stdout,"%s:%.2lf%%\n",ts("physical_derate"),100*u->physical_derate);
	}
	if(u->magical_derate!=0.0){
		fprintf(stdout,"%s:%.2lf%%\n",ts("magical_derate"),100*u->magical_derate);
	}
	if(u->spi){
		fprintf(stdout,"%s:%ld/%ld %.2lf%%\n",ts("spi_force"),u->spi,u->base->max_spi,100.0*u->spi/u->base->max_spi);
	}
	for_each_effect(ep,u->owner->field->effects){
		if(ep->dest!=u||!ep->base->id)
			continue;
		neg=effect_isnegative(ep);
		if(neg)
			fputs(BLUE,stdout);
		fprintf(stdout,"%s:%s",ts("effect"),e2s(ep->base->id));
		if(ep->level)
			fprintf(stdout,"(%+ld)",ep->level);
		if(ep->round>=0)
			fprintf(stdout,"[%d]",ep->round);
		if(neg)
			fputs(WHITE,stdout);
		fputc('\n',stdout);
	}
}
int term_selector(const struct player *p){
	char buf[32];
	ssize_t r;
	static int dep=0;
	if(!canaction2(p,cur)){
		if(isalive(p->front->state)){
			if(canaction2(p,ACT_NORMALATTACK)){
				cur=ACT_NORMALATTACK;
				goto refrash;
			}

			for(int i=0;i<8;++i){
				if(canaction2(p,i)){
					cur=i;
					goto refrash;
				}
			}
		}else {
			for(int i=0;i<6;++i){
				if(isalive(p->units[i].state)){
					cur=ACT_UNIT0+i;
					goto refrash;
				}
			}
		}
		cur=ACT_ABORT;
	}
refrash:
	frash(p,stdout,cur);
	fflush(stdin);
	r=read(STDIN_FILENO,buf,31);
	if(r<0)goto refrash;
	buf[r]=0;
	switch(buf[0]){
		case '\033':
			if(buf[1]!='[')
				goto refrash;
			switch(buf[2]){
				case 'A':
					switch(cur){
						default:
						case ACT_MOVE0:
							cur=ACT_NORMALATTACK;
							goto refrash;
						case ACT_UNIT0:
							cur=ACT_GIVEUP;
							goto refrash;
						case ACT_ABORT:
							cur=ACT_UNIT5;
							goto refrash;
						case ACT_GIVEUP:
							cur=ACT_ABORT;
							goto refrash;
						case ACT_MOVE1 ... ACT_MOVE7:
						case ACT_UNIT1 ... ACT_UNIT5:
							--cur;
							goto refrash;
						case ACT_NORMALATTACK:
							cur=ACT_MOVE7;
							goto refrash;
					}
				case 'B':
					switch(cur){
						default:
							cur=ACT_NORMALATTACK;
							goto refrash;
						case ACT_UNIT5:
							cur=ACT_ABORT;
							goto refrash;
						case ACT_ABORT:
							cur=ACT_GIVEUP;
							goto refrash;
						case ACT_GIVEUP:
							cur=ACT_UNIT0;
							goto refrash;
						case ACT_MOVE0 ... ACT_MOVE6:
						case ACT_UNIT0 ... ACT_UNIT4:
							++cur;
							goto refrash;
						case ACT_NORMALATTACK:
							cur=ACT_MOVE0;
							goto refrash;
					}

				case 'D':
					switch(cur){
						case ACT_ABORT:
							cur=ACT_MOVE6;
							goto refrash;
						case ACT_GIVEUP:
							cur=ACT_MOVE7;
							goto refrash;
						case ACT_UNIT0 ... ACT_UNIT5:
							cur-=ACT_UNIT0;
						default:
							goto refrash;
					}
				case 'C':
					switch(cur){
						case ACT_NORMALATTACK:
							cur=ACT_UNIT0;
							goto refrash;
						case ACT_MOVE0 ... ACT_MOVE5:
							cur+=ACT_UNIT0;
							goto refrash;
						case ACT_MOVE6:
							cur=ACT_ABORT;
							goto refrash;
						case ACT_MOVE7:
							cur=ACT_GIVEUP;
						default:
							goto refrash;
					}
				default:
					goto refrash;
			}
		case '\n':
			if(canaction2(p,cur)){
				azero(rec);
				return cur;
			}else {
				wmf(0,RED "the action is unavailable" WHITE);
				goto refrash;
			}
		case 'D':
		case 'd':
			switch(cur){
				case ACT_MOVE0 ... ACT_MOVE7:
					if(!p->front->moves[cur].id)
						break;
					fprintf(stdout,"\033c" GREEN "%s\n%s\n" WHITE,move_ts(p->front->moves[cur].id),move_desc(p->front->moves[cur].id));
					fflush(stdout);
					read(STDIN_FILENO,buf,31);
					break;
				case ACT_UNIT0 ... ACT_UNIT5:
					if(!p->units[cur-ACT_UNIT0].base)
						break;
					fputs("\033c",stdout);
					print_unit(p->units+cur-ACT_UNIT0);
					fflush(stdout);
					read(STDIN_FILENO,buf,31);
					break;
				default:
					break;
			}
			break;
		case 'G':
		case 'g':
			cur=ACT_GIVEUP;
			goto refrash;
		case 'S':
		case 's':
			cur=ACT_ABORT;
			goto refrash;
		case 'A':
		case 'a':
			cur=ACT_NORMALATTACK;
			goto refrash;
		case 'q':
			cur=ACT_MOVE0;
			goto refrash;
		case 'w':
			cur=ACT_MOVE1;
			goto refrash;
		case 'e':
			cur=ACT_MOVE2;
			goto refrash;
		case 'r':
			cur=ACT_MOVE3;
			goto refrash;
		case 't':
			cur=ACT_MOVE4;
			goto refrash;
		case 'y':
			cur=ACT_MOVE5;
			goto refrash;
		case 'u':
			cur=ACT_MOVE6;
			goto refrash;
		case 'i':
			cur=ACT_MOVE7;
			goto refrash;
		case 'Q':
			cur=ACT_UNIT0;
			goto refrash;
		case 'W':
			cur=ACT_UNIT1;
			goto refrash;
		case 'E':
			cur=ACT_UNIT2;
			goto refrash;
		case 'R':
			cur=ACT_UNIT3;
			goto refrash;
		case 'T':
			cur=ACT_UNIT4;
			goto refrash;
		case 'Y':
			cur=ACT_UNIT5;
			goto refrash;
		case 'x':
			frash(p->enemy,stdout,-1);
			read(STDIN_FILENO,buf,31);
			goto refrash;
		case 'c':
			if(!dep){
				++dep;
				term_selector(p->enemy);
				--dep;
			}
			//read(STDIN_FILENO,buf,31);
			goto refrash;
	}
	goto refrash;
}
