#include "menu.h"
#include "locale.h"
#include "moves.h"
#include "battle.h"
#include "item.h"
#include "nbt.h"
#include "utils.h"
#include <ncurses.h>
#include <limits.h>
#include <assert.h>
#include <alloca.h>
#include <stdlib.h>
#include <string.h>
void reporter_term(const struct message *msg,const struct player *p);
int term_selector(const struct player *p);
void tm_init(void);
void tm_end(void);
void scr(void){
	assert(initscr()&&has_colors());
	noecho();
	curs_set(0);
	start_color();
	init_pair(1,COLOR_CYAN,COLOR_BLACK);
	init_pair(2,COLOR_RED,COLOR_BLACK);
	keypad(stdscr,1);
	cbreak();
}
const struct species *getunit(void){
	size_t i;
	ssize_t cur=0;
st:
	clear();
	move(0,0);
	for(i=0;builtin_species[i].max.id;++i){
		cur=limit_ring(cur,0,(ssize_t)(builtin_species_size-1));
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%zu:%s\n",i,unit_ts(builtin_species[i].max.id));
		if(i==cur)
			attroff(COLOR_PAIR(1));

	}
	refresh();
	switch(getch()){
		case KEY_DOWN:
			++cur;
			goto st;
		case KEY_UP:
			--cur;
			goto st;
		case '\n':
			return builtin_species+cur;
		default:
			break;
	}
	goto st;
}
void msgbox(const char *msg){
st:
	clear();
	move(LINES/2-1,COLS*5/12);
	addstr(msg);
	move(LINES/2+1,COLS/2-1);
	attron(COLOR_PAIR(1));
	printw("%s",ts("close"));
	attroff(COLOR_PAIR(1));
	refresh();
	switch(getch()){
		case '\n':
		case 'q':
			return;
		default:
			goto st;
	}
}
void giveitem(const struct player_data *p,const char *id,long count){
	unsigned long r=pdata_countitem(p,id),r1;
	r1=pdata_giveitem(p,id,count);
	if(r1<=r)
		return;
st:
	clear();
	move(LINES/2-1,COLS*5/12);
	printw("%s %s *%lu",ts("item_get"),item_ts(id),r1-r);
	move(LINES/2+1,COLS/2-1);
	attron(COLOR_PAIR(1));
	printw("%s",ts("close"));
	attroff(COLOR_PAIR(1));
	refresh();
	switch(getch()){
		case '\n':
		case 'q':
			return;
		default:
			goto st;
	}
}
static int move_type(const char *s){
	const struct move *m;
	m=get_builtin_move_by_id(s);
	if(m){
		return m->type;
	}
	return INT_MIN;
}
static void move_title_print(const struct move *p){
	printw("%s",move_ts(p->id));
	printw(" %s:%s",ts("type"),type2str(p->type));
	if(p->prior)
		printw(" %s:%+d",ts("prior"),p->prior);
	if(p->mlevel){
		printw(" %s:",ts("mlevel"));
		if(p->mlevel&MLEVEL_FREEZING_ROARING)
			printw("%s",ts("freezing_roaring"));
		else if(p->mlevel&MLEVEL_CONCEPTUAL)
			printw("%s",ts("conceptual"));
		else if(p->mlevel&MLEVEL_REGULAR)
			printw("%s",ts("regular"));
	}
}
void writemove(struct unit_info *ui){
	size_t i,r;
	ssize_t cur=0,ri;
	const char *s[8],*s0,*warn=NULL;
	int x,types[151];
	const struct move *m;
	for(i=0,r=0;i<=150&&i<=ui->level;++i){
		if(!ui->spec->moves[i]){
			continue;
		}
		m=get_builtin_move_by_id(ui->spec->moves[i]);
		if(m){
			types[i]=m->type;
		}
		++r;
	}
	memcpy(s,ui->moves,8*sizeof(const char *));
	if(r&&s[0])
		cur=r;
st:
	clear();
	move(0,0);
	r=0;
	ri=-1;
	for(i=0;i<=150&&i<=ui->level;++i){
		if(!ui->spec->moves[i])
			continue;
		if(r==cur){
			ri=i;
		}
		++r;

	}
	if(!r)
		return;
	if(cur>=r&&(cur>=r+8||!s[cur-r])){
		cur=s[0]?r:0;
		goto st;
	}
	r=0;
	printw("%s:\n",ts("avaliable_moves"));
	for(i=0;i<=150&&i<=ui->level;++i){
		if(!ui->spec->moves[i])
			continue;
		if(r==cur)
			attron(COLOR_PAIR(1));
		printw("%zu:(%s)%s\n",r,type2str(types[i]),move_ts(ui->spec->moves[i]));
		if(r==cur)
			attroff(COLOR_PAIR(1));
		++r;

	}
	attron(COLOR_PAIR(1));
	for(i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s:\n",ts("current_moves"));
	for(i=0;i<8;++i){
		if(!s[i])
			continue;
		x=cur>=r&&cur-r==i&&cur<r+8&&s[cur-r];
		if(x)
			attron(COLOR_PAIR(1));
		printw("%zu:(%s)%s\n",i,type2str(move_type(s[i])),move_ts(s[i]));
		if(x)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	if(cur<r)
		s0=ui->spec->moves[ri];
	else if(cur>=r&&cur<r+8&&s[cur-r])
		s0=s[cur-r];
	else
		abort();
	printw("%s\n",move_desc(s0));
	if(warn){
		move(LINES-2,0);
		attron(COLOR_PAIR(2));
		printw("%s",warn);
		attroff(COLOR_PAIR(2));
		warn=NULL;
	}
	move(LINES-1,0);
	printw("x:ok enter:add d:delete c:selected u:up i:down q:cancel");
	refresh();
	switch(x=getch()){
		case KEY_DOWN:
			if(cur==r-1&&s[0])
				cur=r;
			else if(cur>=r)
				cur=cur<r+7&&s[cur-r]?cur+1:0;
			else
				cur=limit_ring(cur+1,0,(ssize_t)(r-1));
			goto st;
		case KEY_UP:
			if(cur>=r)
				--cur;
			else
				cur=limit_ring(cur-1,0,(ssize_t)(r-1));
			goto st;
		case '\n':
			if(ri>=0){
				s0=ui->spec->moves[ri];
				if(s[7]){
					for(int j=0;j<8;++j){
						if(!strcmp(s[j],s0)){
							cur=r+j;
							goto conflict;
						}
					}
					warn="A MOVE SHOULD BE DELECTED";
					goto st;
				}
				for(i=0;i<8;++i){
					if(s[i])
						continue;
					//if(i!=7)abort();
					for(int j=0;j<i;++j){
						if(!strcmp(s[j],s0)){
							cur=r+j;
							goto conflict;
						}
						//fprintf(stderr,"%s != %s\n",s[j],s0);
					}
					s[i]=s0;
					goto st;
conflict:
					warn="CONFLICT MOVE";
					goto st;
				}
			}
			if(cur<r){
				goto st;
			}
		case 'x':
			memcpy(ui->moves,s,8*sizeof(const char *));
			return;
		case 'q':
			return;
		case 'u':
			if(cur<=r)
				goto st;
			s0=s[cur-r-1];
			s[cur-r-1]=s[cur-r];
			s[cur-r]=s0;
			--cur;
			goto st;
		case 'i':
			if(cur<r||cur>=r+7)
				goto st;
			s0=s[cur-r+1];
			s[cur-r+1]=s[cur-r];
			s[cur-r]=s0;
			++cur;
			goto st;
		case 'd':
			if(cur<r)
				goto st;
			if(cur==r+7||!s[cur-r+1]){
				s[cur-r]=NULL;
				--cur;
			}
			for(int j=cur-r;j<7&&s[j+1];){
st1:
				s[j]=s[j+1];
				++j;
				if(j<7&&s[j+1])
					goto st1;
				s[j]=NULL;
				break;
			}
			goto st;
		case 'c':
			if(s[0])
				cur=r;
			goto st;
		case '0' ... '9':
			x-='0';
			if(cur<r){
				if(x<r)
					cur=x;
			}else if(cur>=r&&cur<r+8&&x<8&&s[x]){
				cur=x+r;
			}
		default:
			goto st;
	}
}
void memswap(void *restrict s1,void *restrict s2,size_t len){
	void *swapbuf=alloca(len);
	memcpy(swapbuf,s1,len);
	memcpy(s1,s2,len);
	memcpy(s2,swapbuf,len);
}
#define pwc(cond,fmt,...) do {\
	int _c=!!(cond);\
	if(_c)\
		attron(COLOR_PAIR(1));\
	printw(fmt,__VA_ARGS__);\
	if(_c)\
		attroff(COLOR_PAIR(1));\
}while(0)
#define pwcr(cond,fmt,...) do {\
	int _c=!!(cond);\
	if(_c)\
		attron(COLOR_PAIR(2));\
	printw(fmt,__VA_ARGS__);\
	if(_c)\
		attroff(COLOR_PAIR(2));\
}while(0)
void units_menu(struct player_data *pd){
	int cur=0,hcur=0;
	struct unit_base ub;
	const char *p;
	ssize_t r,r1,r2;
	struct nbt_node *np;
	if(!pd->ui->spec)
		return;
	//char buf[512];
st:
	clear();
	if(!pd->ui[cur].spec)
		return;
	move(0,0);
	for(int i=0;i<6&&pd->ui[i].spec;++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("[%d]%s\n",i,unit_ts(pd->ui[i].spec->max.id));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	mkbase(pd->ui+cur,&ub);
	printw("%s:%s",ts("type"),type2str(ub.type0));
	if(ub.type1)
		printw("/%s",type2str(ub.type1));
	addch('\n');
	printw("%s:%d\t%s:%lu\n%s:%lu\t%s:%ld\n%s:%lu\t%s:%lu\n%s:%lu\t%s:%.2lf%%\n%s:%.2lf%%\t%s:%.2lf%%\n%s:%.2lf%%\t%s:%.2lf%%\n",
			ts("level"),ub.level,
			ts("max_hp"),ub.max_hp,
			ts("atk"),ub.atk,
			ts("def"),ub.def,
			ts("speed"),ub.speed,
			ts("hit"),ub.hit,
			ts("avoid"),ub.avoid,
			ts("crit_effect"),100*ub.crit_effect,
			ts("physical_bonus"),100*ub.physical_bonus,
			ts("magical_bonus"),100*ub.magical_bonus,
			ts("physical_derate"),100*ub.physical_derate,
			ts("magical_derate"),100*ub.magical_derate
			);
	if(ub.max_spi!=128)
		printw("%s:%ld\n",ts("max_spi"),ub.max_spi);
	for(int i=0;i<8;++i){
		if(!pd->ui[cur].moves[i])
			continue;
		p=pd->ui[cur].moves[i];
		printw("%d:(%s)%s\n",i,type2str(move_type(p)),move_ts(p));
	}
	move(LINES-4,0);
	printw("%s:%lu",ts("xp"),pd->xp);
	r=xp_require(pd->ui+cur);
	if(pd->ui[cur].level<150)
		printw(" %s:%zd",ts("xp_require"),r);
	if(pd->ui[cur].spec->flag&UF_EVOLVABLE){
		printw("\n%s:%zd (%s)",ts("evolve_level"),r1=pd->ui[cur].spec->evolve_level,unit_ts(pd->ui[cur].spec[1].max.id));
		if(pd->ui[cur].level>=r1)
			printw(" %s:%zd",ts("xp_for_evolve"),r2=xp_require_evo(pd->ui+cur));
		else
			r2=0;
	}else {
		r2=0;
		r1=0;
		printw("\n%s",ts("unevolvable"));
	}
	addch('\n');
	move(LINES-2,0);
	pwc(hcur==0,"%s",ts("move"));
	addch('\t');
	if(pd->ui[cur].level>=150||pd->xp<r)
		pwcr(hcur==1,"%s",ts("level_up"));
	else
		pwc(hcur==1,"%s",ts("level_up"));
	addch('\t');
	if(!r1||!(pd->ui[cur].spec->flag&UF_EVOLVABLE)||pd->ui[cur].level<r1||pd->xp<r2)
		pwcr(hcur==2,"%s",ts("evolve"));
	else
		pwc(hcur==2,"%s",ts("evolve"));
	addch('\t');
	pwc(hcur==3,"%s",ts("exit"));
	move(LINES-1,0);
	printw("t:top u:up i:down s:storage q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			switch(cur){
				case 0 ... 4:
					if(pd->ui[cur+1].spec){
						++cur;
						break;
					}
				case 5:
					cur=0;
					break;
				default:
					break;
			}
			goto st;
		case KEY_UP:
			switch(cur){
				case 1 ... 5:
					--cur;
					break;
				case 0:
					for(;cur<6&&pd->ui[cur].spec;++cur);
					--cur;
					break;
				default:
					break;
			}
			goto st;
		case KEY_LEFT:
			hcur=limit_ring(hcur-1,0,3);
			goto st;
		case KEY_RIGHT:
			hcur=limit_ring(hcur+1,0,3);
			goto st;
		case 't':
			if(cur){
				memswap(pd->ui,pd->ui+cur,sizeof(struct unit_info));
				cur=0;
			}
			goto st;
		case 'u':
			if(cur){
				memswap(pd->ui+cur-1,pd->ui+cur,sizeof(struct unit_info));
				--cur;
			}
			goto st;
		case 'i':
			if(cur<5&&pd->ui[cur+1].spec){
				memswap(pd->ui+cur+1,pd->ui+cur,sizeof(struct unit_info));
				++cur;
			}
			goto st;
		case 'c':
				if(pd->ui[cur].spec->flag&UF_CANSELECTTYPE){
					pd->ui[cur].type0<<=1;
					if(pd->ui[cur].type0==pd->ui[cur].type1)
						pd->ui[cur].type0<<=1;
					if(!(pd->ui[cur].type0&TYPES_REGULAR))
						pd->ui[cur].type0=TYPE_GRASS;
					if(pd->ui[cur].type0==pd->ui[cur].type1)
						pd->ui[cur].type0<<=1;
				}
				goto st;
		case 'C':
				if(pd->ui[cur].spec->flag&UF_CANSELECTTYPE){
					pd->ui[cur].type1<<=1;
					if(pd->ui[cur].type0==pd->ui[cur].type1)
						pd->ui[cur].type1<<=1;
					if(!pd->ui[cur].type1)
						pd->ui[cur].type1=TYPE_GRASS;
					if(!(pd->ui[cur].type1&TYPES_REGULAR))
						pd->ui[cur].type1=TYPE_VOID;
					if(pd->ui[cur].type0==pd->ui[cur].type1)
						pd->ui[cur].type1<<=1;
				}
				goto st;
		case 'K':
			for(;;){
				if(pd->ui[cur].level>=150)
					break;
				r=xp_require(pd->ui+cur);
				if(pd->xp<r)
					break;
				pd->xp-=r;
				++pd->ui[cur].level;
			}
			goto st;
		case 's':
			np=nbt_find(pd->nbt,"storage",7);
			if(!np){
				assert(np=nbt_lista("storage",7,NULL,0));
				assert(nbt_add(pd->nbt,np));
			}
			nbt_ainsert(pd->nbt,np,np->count,ui_asnbt(pd->ui+cur));
			if(cur>=5||!pd->ui[cur+1].spec)
				pd->ui[cur].spec=NULL;
			else {
				for(int i=cur;i<5;++i){
					if(pd->ui[i+1].spec)
						memcpy(pd->ui+i,pd->ui+i+1,sizeof(struct unit_info));
					else
						pd->ui[i].spec=NULL;
				}
				pd->ui[5].spec=NULL;
			}
			while(cur>0&&!pd->ui[cur].spec)
				--cur;
			goto st;
		case '\n':
			switch(hcur){
				case 0:
					writemove(pd->ui+cur);
					goto st;
				case 1:
					if(pd->ui[cur].level>=150)
						goto st;
					r=xp_require(pd->ui+cur);
					if(pd->xp<r)
						goto st;
					pd->xp-=r;
					++pd->ui[cur].level;
					goto st;
				case 2:
					r=pd->ui[cur].spec->evolve_level;
					if(!r||pd->ui[cur].level<r)
						goto st;
					if(pd->xp<r2)
						goto st;
					pd->xp-=r2;
					++pd->ui[cur].spec;
					if(pd->ui[cur].spec->flag&UF_CANSELECTTYPE){
						pd->ui[cur].type0=pd->ui[cur].spec->max.type0;
						pd->ui[cur].type1=pd->ui[cur].spec->max.type1;
					}
					goto st;
				default:
					return;
			}
			return;
		case 'q':
			return;
		default:
			goto st;
	}
}
static const char *mobs[]={"icefield_tiger_cub","flat_mouth_duck","hood_grass","attacking_defensive_combined_matrix_1","cactus_ball"};
#define arrsize(arr) (sizeof(arr)/sizeof(arr[0]))
void endless_fake(struct player_data *p,int level){
	const char *cp=mobs[(level-1)%arrsize(mobs)];
	const struct species *spec=get_builtin_species_by_id(cp);
	while((spec->flag&UF_EVOLVABLE)&&level>=spec->evolve_level&&spec[1].xp_type<=spec->xp_type*16){
		++spec;
		cp=spec->max.id;
	}
	assert(!pdata_fake(p,cp,level));
}
unsigned long endless_win(struct player_data *pd){
	unsigned long r=183*pd->endless_level;
	pd->xp+=r;
	if(pd->endless_level<INT_MAX)
		++pd->endless_level;
	if(pd->endless_highest<pd->endless_level)
		pd->endless_highest=pd->endless_level;
	return r;
}
void endless_menu(struct player_data *pd){
	int my,mx,cur=0,r=-1;
	struct player_data p,p0;
st:
	clear();
	getmaxyx(stdscr,my,mx);
	move(my/2-2,mx/3-1);
	printw("%s",ts("endless_challenge"));
	move(my/2,mx/3-1);
	printw("%s:%lu",ts("highest_level"),pd->endless_highest);
	move(my/2+1,mx/3-1);
	printw("%s:%lu",ts("current_level"),pd->endless_level);
	move(my/2+3,mx/3-1);
	if(cur==0)
		attron(COLOR_PAIR(1));
	printw("%s",ts("challenge"));
	if(cur==0)
		attroff(COLOR_PAIR(1));
	addch('\t');
	if(cur==1)
		attron(COLOR_PAIR(1));
	printw("%s",ts("exit"));
	if(cur==1)
		attroff(COLOR_PAIR(1));
	addch('\t');
	if(cur==2)
		attron(COLOR_PAIR(1));
	printw("%s",ts("auto"));
	if(cur==2)
		attroff(COLOR_PAIR(1));
	switch(getch()){
		/*case KEY_DOWN:
			--pd->endless_level;
			goto st;
		case KEY_UP:
			++pd->endless_level;
			goto st;*/
		case KEY_LEFT:
			cur=limit_ring(cur-1,0,2);
			goto st;
		case KEY_RIGHT:
			cur=limit_ring(cur+1,0,2);
			goto st;
		case '\n':
			switch(cur){
				default:
					return;
				case 0:
				case 2:
					break;
			}
			switch(cur){
				case 0:
					endless_fake(&p,pd->endless_level);
					memcpy(&p0,pd,sizeof(struct player_data));
					memset(p0.ui+1,0,5*sizeof(struct unit_info));
					endwin();
					tm_init();
					r=pbattle(&p0,&p,term_selector,rand_selector,reporter_term,NULL,NULL);
					tm_end();
					scr();
					if(!r){
						endless_win(pd);
					}
					break;
				case 2:
					memcpy(&p0,pd,sizeof(struct player_data));
					memset(p0.ui+1,0,5*sizeof(struct unit_info));
					for(int i=0;i<1024;++i){
						endless_fake(&p,pd->endless_level);
						r=pbattle(&p0,&p,rand_selector,rand_selector,NULL,NULL,NULL);
						if(!r)
							endless_win(pd);
						if(pd->endless_highest<=pd->endless_level&&pd->endless_level>=150)
							break;
					}
					break;
			}
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
void storage_menu(struct player_data *pd){
	ssize_t cur=0,shift=0;
	struct nbt_node *np=nbt_find(pd->nbt,"storage",7),*p,**npp;
	int nline;
	if(!np){
		assert(np=nbt_lista("storage",7,NULL,0));
		assert(nbt_add(pd->nbt,np));
	}
	if(!np->count)
		return;
	npp=nbt_listap(np);
st:
	if(!np->count)
		return;
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	for(ssize_t i=shift;i-shift<nline&&i<np->count;++i){
		p=npp[i];
		assert(p=nbt_find(p,"id",2));
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("[%zd]%s",i,unit_ts(nbt_strp(p)));
		addch('\n');
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s:\n",ts("deployed_units"));
	for(int i=0;i<6&&pd->ui[i].spec;++i){
		printw("[%d]%s\n",i,unit_ts(pd->ui[i].spec->max.id));
	}
	move(LINES-1,0);
	printw("enter:deploy q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(np->count-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(np->count-1));
			goto st;
		case '\n':
			if(pd->ui[5].spec){
				msgbox(ts("full_deployed_units"));
				goto st;
			}
			for(int i=0;i<6;++i){
				if(pd->ui[i].spec)
					continue;
				ui_create_fromnbt(pd->ui+i,npp[cur]);
				nbt_adelete(np,cur);
				cur=limit(cur,0,(ssize_t)(np->count-1));
				break;
			}
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
struct mm_option {
	const char *name;
	void (*submenu)(struct player_data *);
};
void listmove(const struct species *spec){
	size_t i,r;
	ssize_t cur=0,ri=0,shift=0,s,n;
	int types[151],nline;
	const struct move *m;
	for(i=0,r=0;i<=150;++i){
		if(!spec->moves[i]){
			continue;
		}
		m=get_builtin_move_by_id(spec->moves[i]);
		if(m){
			types[i]=m->type;
		}
		++r;
	}
	if(!r)
		return;
	n=r;
st:
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	s=shift;
	r=0;
	for(i=0;i<=150;++i){
		if(!spec->moves[i])
			continue;
		if(s){
			--s;
			continue;
		}
		if(r+shift==cur){
			attron(COLOR_PAIR(1));
			ri=i;
		}
		printw("%zu:(%s)%s\n",i,type2str(types[i]),move_ts(spec->moves[i]));
		if(r+shift==cur)
			attroff(COLOR_PAIR(1));
		++r;
		if(r==nline)
			break;

	}
	attron(COLOR_PAIR(1));
	for(i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s\n",move_desc(spec->moves[ri]));
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,n-1);
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,n-1);
			goto st;
		case '\n':
		case 'q':
			return;
		default:
			goto st;
	}
}
void wp_menu(struct player_data *pd){
	ssize_t cur=0,shift=0;
	const struct unit_base *p;
	const char *cp;
	int nline;
st:
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	for(ssize_t i=shift;i-shift<nline&&i<builtin_species_size;++i){
		p=&builtin_species[i].max;
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%zu:%s",i,unit_ts(p->id));
		printw(" %s:%s",ts("type"),type2str(p->type0));
		if(p->type1)
			printw("/%s",type2str(p->type1));
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
		printw(" %s:%s",ts("unit_type"),ts(cp));
		addch('\n');
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	p=&builtin_species[cur].max;
	printw("%s:\n",ts("attribute_on_max_level"));
	printw("%s:%s",ts("type"),type2str(p->type0));
	if(p->type1)
		printw("/%s",type2str(p->type1));
	addch('\n');
	printw("%s:%d\t%s:%lu\n%s:%lu\t%s:%ld\n%s:%lu\t%s:%lu\n%s:%lu\t%s:%.2lf%%\n%s:%.2lf%%\t%s:%.2lf%%\n%s:%.2lf%%\t%s:%.2lf%%\n",
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
	if(p->max_spi!=128)
		printw("%s:%ld\n",ts("max_spi"),p->max_spi);
	printw("%s:%d %s:%lu\n",ts("base_xp"),builtin_species[cur].xp_type,ts("xp_from_1_to_max_level"),xp_require_fromto(builtin_species+cur,1,150));
	if(builtin_species[cur].flag&UF_EVOLVABLE){
		printw("%s:%d (%s)",ts("evolve_level"),builtin_species[cur].evolve_level,unit_ts(builtin_species[cur+1].max.id));
	}else
		printw("%s",ts("unevolvable"));
	addch('\n');
	if(builtin_species[cur].flag&UF_CANSELECTTYPE)
		printw("%s\n",ts("can_select_type"));
	move(LINES-1,0);
	printw("id: %s",builtin_species[cur].max.id);
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(builtin_species_size-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(builtin_species_size-1));
			goto st;
		case '\n':
			listmove(builtin_species+cur);
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
struct unit_level {
	const char *id;
	int level,unused;
};
void move_unit_menu(struct player_data *pd,const char *id){
	ssize_t cur=0,shift=0,n=0;
	const char *const *cp;
	struct unit_level *ulp,*ulp1;
	for(size_t i=0;i<builtin_species_size;++i){
		cp=builtin_species[i].moves;
		for(int j=0;j<=150;++j){
			if(cp[j]&&!strcmp(cp[j],id)){
				++n;
				break;
			}
		}
	}
	if(!n)
		return;
	ulp=malloc(n*sizeof(struct unit_level));
	assert(ulp);
	ulp1=ulp;
	for(size_t i=0;i<builtin_species_size;++i){
		cp=builtin_species[i].moves;
		for(int j=0;j<=150;++j){
			if(cp[j]&&!strcmp(cp[j],id)){
				ulp1->id=builtin_species[i].max.id;
				ulp1->level=j;
				++ulp1;
				break;
			}
		}
	}
st:
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(LINES-1)){
		shift=cur-(LINES-1);
	}
	for(ssize_t i=shift;i-shift<LINES&&i<n;++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%zd:%s %s:%d\n",i,unit_ts(ulp[i].id),ts("level"),ulp[i].level);
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,n-1);
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,n-1);
			goto st;
		case '\n':
		case 'q':
			free(ulp);
			return;
		default:
			goto st;
	}
}
void move_menu(struct player_data *pd){
	ssize_t cur=0,shift=0;
	const struct move *p;
	int nline;
st:
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	for(ssize_t i=shift;i-shift<nline&&i<builtin_moves_size;++i){
		p=builtin_moves+i;
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%zu:",i);
		move_title_print(p);
		addch('\n');
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s\n",move_desc(builtin_moves[cur].id));
	move(LINES-1,0);
	printw("id: %s",builtin_moves[cur].id);
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(builtin_moves_size-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(builtin_moves_size-1));
			goto st;
		case '\n':
			move_unit_menu(pd,builtin_moves[cur].id);
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
void effect_menu(struct player_data *pd){
	ssize_t cur=0,shift=0;
	const char *p;
	int nline;
st:
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	for(ssize_t i=shift;i-shift<nline&&i<effects_size;++i){
		p=effects[i];
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%zu:%s",i,e2s(p));
		addch('\n');
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s\n",e2desc(effects[cur]));
	move(LINES-1,0);
	printw("id: %s",effects[cur]);
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(effects_size-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(effects_size-1));
			goto st;
		case '\n':
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
void type_menu(struct player_data *pd){
	int cur=0,shift=0;
	int nline,ew;
st:
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	for(int i=shift;i-shift<nline&&((1<<i)&TYPES_ALL);++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%d:%s\n",i,type2str(1<<i));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s:\n",ts("as_move_type"));
	printw("%s:",ts("type_effect"));
	ew=effect_types(1<<cur);
	for(int i=TYPE_GRASS;i&TYPES_ALL;i<<=1){
		if(i&ew){
			if(i&TYPES_DEVINE)
				printw(" [%s]",type2str(i));
			else
				printw(" %s",type2str(i));
		}
	}
	addch('\n');
	printw("%s:",ts("type_weakened"));
	ew=weak_types(1<<cur);
	for(int i=TYPE_GRASS;i&TYPES_ALL;i<<=1){
		if(i&ew){
			if(i&TYPES_DEVINE)
				printw(" [%s]",type2str(i));
			else
				printw(" %s",type2str(i));
		}
	}
	addch('\n');
	ew=1<<cur;
	if(ew&TYPES_DEVINE){
		printw("%s\n",ts("not_unit_type"));
	}else {
		printw("%s:\n",ts("as_unit_type"));
		printw("%s:",ts("type_effected"));
		for(int i=TYPE_GRASS;i&TYPES_ALL;i<<=1){
			if(effect_types(i)&ew)
				printw(" %s",type2str(i));
		}
		addch('\n');

		printw("%s:",ts("type_weak"));
		for(int i=TYPE_GRASS;i&TYPES_ALL;i<<=1){
			if(weak_types(i)&ew)
				printw(" %s",type2str(i));
		}
		addch('\n');
	}
	move(LINES-1,0);
	printw("id: %s",type2id(1<<cur));
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,19);
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,19);
			goto st;
		case '\n':
		case 'q':
			return;
		default:
			goto st;
	}
}
void item_menu(struct player_data *pd){
	int cur=0,shift=0;
	int nline;
	size_t count;
	const char *cp,*cp0=NULL;
	struct nbt_node *np,*npi;
	const struct item *ip;
st:
	np=nbt_find(pd->nbt,"items",5);
	if(!np||np->type!=NBT_LIST)
		return;
	np=nbt_listl(np);
	count=nbt_count(np);
	if(count<2)
		return;
	nline=LINES/2;
	clear();
	move(0,0);
	if(cur<shift){
		shift=cur;
	}else if(cur>shift+(nline-1)){
		shift=cur-(nline-1);
	}
	for(int i=shift;i-shift<nline&&i<count;++i){
		npi=nbt_byindex(np,i);
		if(npi->type!=NBT_LIST){
			continue;
		}
		cp=npi->data;
		npi=nbt_find(nbt_listl(npi),"count",5);
		assert(npi&&npi->type==NBT_ZU);
		if(i==cur){
			cp0=cp;
			attron(COLOR_PAIR(1));
		}
		printw("%s:%zu\n",item_ts(cp),nbt_zul(npi));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	printw("%s\n",item_desc(cp0));
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)count-1);
			if(nbt_byindex(np,cur)->type!=NBT_LIST)
				cur=limit_ring(cur+1,0,(ssize_t)count-1);
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)count-1);
			if(nbt_byindex(np,cur)->type!=NBT_LIST)
				cur=limit_ring(cur-1,0,(ssize_t)count-1);
			goto st;
		case '\n':
			npi=nbt_byindex(np,cur);
			if(npi->type!=NBT_LIST){
				goto st;
			}
			ip=get_builtin_item_by_id(npi->data);
			if(ip&&ip->onenter)
				ip->onenter(pd,npi);
			goto st;

		case 'q':
			return;
		default:
			goto st;
	}
}
const struct mm_option list_ops[]={
	{
		.name="wild_and_plot_character",
		.submenu=wp_menu,
	},
	{
		.name="move",
		.submenu=move_menu,
	},
	{
		.name="type",
		.submenu=type_menu,
	},
	{
		.name="effect",
		.submenu=effect_menu,
	},
	{
		.name="exit",
		.submenu=NULL,
	},
	{NULL}
};
const size_t list_ops_size=sizeof(list_ops)/sizeof(list_ops[0])-1;
void list_menu(struct player_data *pd){
	int cur=0;
st:
	clear();
	move(0,0);
	for(int i=0;list_ops[i].name;++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%s\n",ts(list_ops[i].name));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	move(LINES-1,0);
	printw("q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(list_ops_size-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(list_ops_size-1));
			goto st;
		case '\n':
			if(!list_ops[cur].submenu)
				return;
			list_ops[cur].submenu(pd);
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
void show_tutorial(const char *item){
	size_t len=strlen(item);
	char *buf=alloca(len+4);
	const char *p;
	int cur=0,n;
	memcpy(buf,item,len);
st:
	n=0;
	clear();
	move(0,0);
	for(int i=0;i<100;++i){
		sprintf(buf+len,"_%d",i);
		p=locale(buf);
		if(!p)
			break;
		++n;
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%s\n",ts(p));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	move(LINES-1,0);
	printw("q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,n-1);
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,n-1);
			goto st;
		case '\n':
		case 'q':
			return;
		default:
			goto st;
	}
}
void tutorial_menu(struct player_data *pd){
	int cur=0;
	static const char *items[]={"type",
		"level_and_evolution",
		"battling",
		"attribute",
		"state",
		"move_level",
	};
st:
	clear();
	move(0,0);
	for(int i=0;i<arrsize(items);++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%s\n",ts(items[i]));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	move(LINES-1,0);
	printw("q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(arrsize(items)-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(arrsize(items)-1));
			goto st;
		case '\n':
			show_tutorial(items[cur]);
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
void mirror_battling(struct player_data *pd){
	int r;
	endwin();
	tm_init();
	r=pbattle(pd,pd,term_selector,rand_selector,reporter_term,NULL,NULL);
	tm_end();
	scr();
	if(!r)
		giveitem(pd,"endless_cream",1);
}
#include "fun.c"
const struct mm_option mmop[]={
	{
		.name="deployed_units",
		.submenu=units_menu,
	},
	{
		.name="storage",
		.submenu=storage_menu,
	},
	{
		.name="endless_challenge",
		.submenu=endless_menu,
	},
	{
		.name="mirror_battling",
		.submenu=mirror_battling,
	},
	{
		.name="fun_mode",
		.submenu=fun_menu,
	},
	{
		.name="item",
		.submenu=item_menu,
	},
	{
		.name="list",
		.submenu=list_menu,
	},
	{
		.name="tutorial",
		.submenu=tutorial_menu,
	},
	{
		.name="exit",
		.submenu=NULL,
	},
	{NULL}
};
const size_t mmop_size=sizeof(mmop)/sizeof(mmop[0])-1;
void main_menu(struct player_data *pd){
	int cur=0;
	scr();
st:
	clear();
	move(0,0);
	printw("%s:%lu\n",ts("xp"),pd->xp);
	attron(COLOR_PAIR(1));
	for(int i=COLS;i>0;--i){
		addch('=');
	}
	attroff(COLOR_PAIR(1));
	for(int i=0;mmop[i].name;++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%s\n",ts(mmop[i].name));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	move(LINES-1,0);
	printw("q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(mmop_size-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(mmop_size-1));
			goto st;
		case '\n':
			if(!mmop[cur].submenu)
				goto end;
			mmop[cur].submenu(pd);
			goto st;
		case 'q':
			goto end;
		default:
			goto st;
	}
end:
	endwin();
}
