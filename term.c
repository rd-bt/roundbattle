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
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define WHITE "\033[37m"
#define GREEN_BG "\033[42m"
#define RED_BG "\033[41m"
#define WHITE_BG "\033[47m"
#define BLACK_BG "\033[40m"
#define putn(c,n) for(unsigned short _i=(n),_c=(c);_i>0;--_i)fputc(_c,fp)
#define azero(a) memset((a),0,sizeof(a))
#define REC_SIZE 32
static char rec[REC_SIZE][129];
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
	print_attr("lv:%u %s %lu/%lu %.2lf%%",u->base->level,u->base->id,u->hp,u->base->max_hp,100.0*u->hp/u->base->max_hp);
	print_attr("atk:%lu def:%ld speed:%lu",u->atk,u->def,u->speed);
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
			fputs(buf,fp);
		}
		if(e1){
			peffect(e1);
			putn(' ',s1);
			fputs(buf,fp);
		}
		fputc('\n',fp);
		++line;
	}

	for_each_effect(ep,f->effects){
		if(ep->dest)
			continue;
		peffect(ep);
		fprintf(fp,"global:%s\n",buf);
		++line;
	}
	putn('-',ws.ws_col);
	fputc('\n',fp);
	++line;
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
	if(current==ACT_NORMALATTACK)
		fputs(c?GREEN:RED,fp);
	fprintf(fp,"(%s)%s",type2str(u->type0),"normal_attack");
	fputs(c?"\n":"[X]\n",fp);
	if(current==ACT_NORMALATTACK)
		fputs(WHITE,fp);

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
				r=snprintf(buf,buflen,"(%s%s%s)%s",type2str(p->units[i].type0),p->units[i].type1?"/":"",p->units[i].type1?type2str(p->units[i].type1):"",p->units[i].base->id);
				if(r<0){
					putc('\n',fp);
					return;
				}
				c=canaction2(p,i+ACT_UNIT0);
				if(!c){
					strncpy(buf+r,c?"":(p->units+r==p->front?"[F]":"[X]"),buflen-r);
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
static const char *dtco[3]={YELLOW,RED,BLUE};
void reporter_term(const struct message *msg){
	struct player *p=msg->field->p;
	char buf[32];
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
				wmf(msg->un.damage.dest->owner==p?0:1,"%s-%lu%s" WHITE,dtco[msg->un.damage.damage_type],msg->un.damage.value,buf);
			}
			goto delay;
		case MSG_EFFECT:
			if(msg->un.e->dest){
				wmf(msg->un.e->dest->owner==p?0:1,"effect %s",msg->un.e->base->id);
				goto delay;
			}
			break;
		case MSG_EFFECT_END:
			if(msg->un.e->dest){
				wmf(msg->un.e->dest->owner==p?0:1,"%s end",msg->un.e->base->id);
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
			wmf(msg->un.heal.dest->owner==p?0:1,GREEN "+%lu" WHITE,msg->un.heal.value);
			goto delay;
		case MSG_HPMOD:
			wmf(msg->un.hpmod.dest->owner==p?0:1,"%+ld",msg->un.hpmod.value);
			goto delay;
		case MSG_MISS:
			wmf(msg->un.u2.dest->owner==p?0:1,"MISS");
			goto delay;
		case MSG_MOVE:
			wmf(msg->un.move.u->owner==p?0:1,"%s",msg->un.move.m->id);
			goto delay;
		case MSG_NORMALATTACK:
			wmf(msg->un.u2.src->owner==p?0:1,"normal_attack");
			goto delay;
		case MSG_ROUND:
			break;
		case MSG_ROUNDEND:
			wmf(0,"ROUND END");
			break;
		case MSG_SPIMOD:
			wmf(msg->un.spimod.dest->owner==p?0:1,"%+ld spi",msg->un.spimod.value);
			goto delay;
		case MSG_SWITCH:
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
			}
			else
				goto refrash;
	}
	goto refrash;
}
