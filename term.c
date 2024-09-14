#include "battle.h"
#include "moves.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <sys/ioctl.h>
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
static const char *types_string[21]={"Void type","Grass","Fire","Water","Steel","Light","Fighting","Wind","Poison","Rock","Electric","Ghost","Ice","Bug","Machine","Soil","Dragon","Normal","Devine grass","Alkali fire","Devine water"};
const char *type2str(int type){
	unsigned int index=type?__builtin_ctz(type)+1:0;
	if(index>=21)
		return "Unknown";
	return types_string[index];
}
size_t strrlen(const char *s){
	size_t r=0;
	while(*s){
		if(*s=='\033'&&s[1]=='['&&s[2]&&s[3]&&s[4]=='m')
			s+=5;
		else {
			++s;
			++r;
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
	strncpy(rec[i],msg,127);
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
void frash(struct player *p,FILE *fp,int current){
	struct player *e=p->enemy;
	struct unit *u;
	struct battle_field *f=p->field;
	unsigned short s,n,s1,line=0;
	struct winsize ws;
	char *buf;
	size_t buflen;
	int r,r1,c;
	int found0=0,found1=0,cf0,cf1;
	static const char *sstr[5]={"normal","controlled","spuuressed","failed","freezing_roaringed"};
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
	buf=alloca(buflen=(ws.ws_col+15)&~15);
#define print_attr(...) \
	s1=ws.ws_col;\
	u=p->front;\
	r=snprintf(buf,buflen,__VA_ARGS__);\
	if(r<0)\
		return;\
	s1-=strrlen(buf);\
	fputs(buf,fp);\
	u=e->front;\
	r=snprintf(buf,buflen,__VA_ARGS__);\
	if(r<0){\
		putc('\n',fp);\
		return;\
	}\
	s1-=strrlen(buf);\
	putn(' ',s1);\
	fputs(buf,fp);\
	fputc('\n',fp);\
	++line
	print_attr("(%s%s%s) %s[%ld] %lu/%lu %.2lf%%",type2str(u->type0),u->type1?"/":"",u->type1?type2str(u->type1):"",u->base->id,u-u->owner->units,u->hp,u->base->max_hp,100.0*u->hp/u->base->max_hp);
	print_attr("lv:%u atk:%lu def:%ld speed:%lu",u->base->level,u->atk,u->def,u->speed);
	print_attr("hit:%lu avd:%lu ce:%.2lf%%",u->hit,u->avoid,100*u->crit_effect);
	if(p->front->physical_bonus!=0.0||p->front->magical_bonus!=0.0||e->front->physical_bonus!=0.0||e->front->magical_bonus!=0.0){
		print_attr("phy:%.2lf%% mag:%.2lf%%",100*u->physical_bonus,100*u->magical_bonus);
	}
	if(p->front->physical_derate!=0.0||p->front->magical_derate!=0.0||e->front->physical_derate!=0.0||e->front->magical_derate!=0.0){
		print_attr("phyd:%.2lf%% magd:%.2lf%%",100*u->physical_derate,100*u->magical_derate);
	}
	if(p->front->spi||e->front->spi){
		print_attr("spi:%ld/%ld %.2lf%%",u->spi,u->base->max_spi,100.0*u->spi/u->base->max_spi);
	}
	for(struct effect *e0,*e1;;){
		cf0=0;
		cf1=0;
		e0=NULL;
		e1=NULL;
		for_each_effect(ep,f->effects){
			if(ep->dest==p->front){
				if(cf0++<found0)
					continue;
				e0=ep;
				++found0;
				break;
			}
		}
		for_each_effect(ep,f->effects){
			if(ep->dest==e->front){
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
			r=snprintf(buf,buflen,"%s",e0->base->id);\
			if(r<0){\
				putc('\n',fp);\
				return;\
			}\
			if(e0->level){\
				r1=snprintf(buf+r,buflen-r,"(%+ld)",e0->level);\
				if(r1<0){\
					putc('\n',fp);\
					return;\
				}\
				r+=r1;\
			}\
			if(e0->round>=0){\
				r1=snprintf(buf+r,buflen-r,"[%d]",e0->round);\
				if(r1<0){\
					putc('\n',fp);\
					return;\
				}\
				r+=r1;\
			}\
			s1-=r
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
		if(ep->dest)
			continue;
		peffect(ep);
		if(ep->base->flag&EFFECT_ENV){
			fputs(ep->inevent?YELLOW:CYAN,fp);
			fprintf(fp,"environment:%s\n",buf);
		}else {
			if(ep->inevent)
				fputs(YELLOW,fp);
			fprintf(fp,"global:%s\n",buf);
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
	r=snprintf(buf,buflen,"(%s)%s",type2str(u->type0),"normal_attack");
	r+=snprintf(buf+r,buflen-r,"%s",c?"":"[X]");
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
			strncpy(buf,"(No move)",buflen);
			c=0;
			goto no_move;
		}
		c=canaction2(p,i);
		r=snprintf(buf,buflen,"(%s)%s",type2str(u->moves[i].type),u->moves[i].id);
		if(r<0)
			return;
		if(!c){
			if(u->moves[i].cooldown){
				r1=snprintf(buf+r,buflen-r,"[%d]",u->moves[i].cooldown);
				if(r1<0)
					return;
			}else
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
					strncpy(buf,"(No unit)",buflen);
					c=0;
					goto no_unit;
				}
				r=snprintf(buf,buflen,"(%s%s%s)%s %lu/%lu",type2str(p->units[i].type0),p->units[i].type1?"/":"",p->units[i].type1?type2str(p->units[i].type1):"",p->units[i].base->id,p->units[i].hp,p->units[i].base->max_hp);
				if(r<0){
					putc('\n',fp);
					return;
				}
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
				strncpy(buf,"abort",buflen);
				break;
			case 7:
				color=current==ACT_GIVEUP;
				c=1;
				strncpy(buf,"give_up",buflen);
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
}
static struct termios tm0;
void __attribute__((constructor)) tm_init(void){
	struct termios tm;
	if(tcgetattr(STDIN_FILENO,&tm)<0)
		goto fail;
	memcpy(&tm0,&tm,sizeof(struct termios));
	tm.c_lflag&=~(ICANON|ECHO);
	tm.c_cc[VMIN]=1;
	tm.c_cc[VTIME]=0;
	if(tcsetattr(STDIN_FILENO,TCSANOW,&tm)<0)
		goto fail;
	return;
fail:
	fputs("CANNOT SET TERMINAL\n",stderr);
	abort();
}
void __attribute__((destructor)) tm_end(void){
	tcsetattr(STDIN_FILENO,TCSANOW,&tm0);
}
static int cur=ACT_NORMALATTACK;
//static const int dtc[3]={'R','P','M'};
static const char *dtco[3]={YELLOW,RED,CYAN};
void reporter_term(const struct message *msg){
	struct player *p=msg->field->p;
	char buf[128];
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
				wmf(msg->un.damage.dest->owner==p?0:1,"total -%lu",msg->un.damage.value);
			}else {
				buf[0]=0;
				if(msg->un.damage.aflag&AF_CRIT)
					strcat(buf," C");
				if(msg->un.damage.aflag&AF_EFFECT)
					strcat(buf," E");
				if(msg->un.damage.aflag&AF_WEAK)
					strcat(buf," W");
				if(msg->un.damage.dest!=msg->un.damage.dest->owner->front){
					strcat(buf," (");
					strcat(buf,msg->un.damage.dest->base->id);
					strcat(buf,")");
				}
				wmf(msg->un.damage.dest->owner==p?0:1,"%s-%lu%s" WHITE,dtco[msg->un.damage.damage_type],msg->un.damage.value,buf);
			}
			goto delay;
		case MSG_EFFECT:
			if(*msg->field->stage==STAGE_INIT)
				break;
			if(msg->un.e->dest){
				if(!isalive(msg->un.e->dest->state))
					break;
				buf[0]=0;
				buf[127]=0;
				r=0;
				if(msg->un.e->dest!=msg->un.e->dest->owner->front){
					r=snprintf(buf,127," (%s)",msg->un.e->dest->base->id);
				}
				if(msg->un.e_init.level)
					snprintf(buf+r,127-r," %+ld",msg->un.e_init.level);
				wmf(msg->un.e->dest->owner==p?0:1,"effect %s%s",msg->un.e->base->id,buf);
				goto delay;
			}
			break;
		case MSG_EFFECT_END:
			if(msg->un.e->dest){
				if(!isalive(msg->un.e->dest->state))
					break;
				buf[0]=0;
				if(msg->un.e->dest!=msg->un.e->dest->owner->front){
					strcat(buf," (");
					strcat(buf,msg->un.e->dest->base->id);
					strcat(buf,")");
				}
				wmf(msg->un.e->dest->owner==p?0:1,"%s end",msg->un.e->base->id,buf);
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
			wmf(msg->un.u->owner==p?0:1,"failed");
			break;
		case MSG_HEAL:
			buf[0]=0;
			if(msg->un.heal.dest!=msg->un.heal.dest->owner->front){
				strcat(buf," (");
				strcat(buf,msg->un.heal.dest->base->id);
				strcat(buf,")");
			}
			wmf(msg->un.heal.dest->owner==p?0:1,GREEN "+%lu%s" WHITE,msg->un.heal.value,buf);
			goto delay;
		case MSG_HPMOD:
			buf[0]=0;
			if(msg->un.hpmod.dest!=msg->un.hpmod.dest->owner->front){
				strcat(buf," (");
				strcat(buf,msg->un.hpmod.dest->base->id);
				strcat(buf,")");
			}
			wmf(msg->un.hpmod.dest->owner==p?0:1,"%+ld%s",msg->un.hpmod.value,buf);
			goto delay;
		case MSG_MISS:
			wmf(msg->un.u2.dest->owner==p?0:1,"MISS");
			goto delay;
		case MSG_MOVE:
			wmf(msg->un.move.u->owner==p?0:1,"%s",msg->un.move.m->id);
			goto delay;
		/*case MSG_NORMALATTACK:
			wmf(msg->un.u2.src->owner==p?0:1,"normal_attack");
			goto delay;*/
		case MSG_ROUND:
			break;
		case MSG_ROUNDEND:
			wmf(0,"ROUND END");
			break;
		case MSG_SPIMOD:
			buf[0]=0;
			if(msg->un.spimod.dest!=msg->un.spimod.dest->owner->front){
				strcat(buf," (");
				strcat(buf,msg->un.spimod.dest->base->id);
				strcat(buf,")");
			}
			wmf(msg->un.spimod.dest->owner==p?0:1,"%+ld spi%s",msg->un.spimod.value,buf);
			goto delay;
		case MSG_SWITCH:
			wmf(msg->un.u2.dest->owner==p?0:1,"%s[%ld]",msg->un.u2.dest->base->id,msg->un.u2.dest-msg->un.u2.dest->owner->units);
			break;
		default:
			break;
	}
	frash(msg->field->p,stdout,-1);
	return;
delay:
	frash(msg->field->p,stdout,-1);
	usleep(250000);
}
int term_selector(struct player *p){
	char buf[32];
	ssize_t r;
	if(!canaction2(p,cur)){
		if(isalive(p->front->state)){
			if(cur!=ACT_NORMALATTACK&&canaction2(p,ACT_NORMALATTACK)){
				cur=ACT_NORMALATTACK;
				goto refrash;
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
	}
	goto refrash;
}
