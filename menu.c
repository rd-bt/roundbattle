#include "menu.h"
#include "locale.h"
#include "moves.h"
#include "battle.h"
#include <ncurses.h>
#include <limits.h>
#include <assert.h>
#include <alloca.h>
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
static int move_type(const char *s){
	const struct move *m;
	m=get_builtin_move_by_id(s);
	if(m){
		return m->type;
	}
	return INT_MIN;
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
			++cur;
			goto st;
		case KEY_UP:
			if(!cur){
				for(i=0;i<=150&&i<=ui->level;++i)
					if(ui->spec->moves[i])
						++cur;
				if(cur)
					--cur;
			}else
				--cur;
			goto st;
		case '\n':
			if(ri>=0){
				for(i=0;i<8;++i){
					if(s[7]){
						warn="A MOVE SHOULD BE DELECTED";
						s0=ui->spec->moves[ri];
						for(int j=0;j<8;++j){
							if(s[j]==s0){
								cur=r+j;
								break;
							}
						}
						goto st;
					}
					if(s[i])
						continue;
					s0=ui->spec->moves[ri];
					for(int j=0;j<8;++j){
						if(s[j]==s0){
							cur=r+j;
							break;
						}
						if(j==7){
							s[i]=s0;
							goto st;
						}
					}
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
#define pwc(cond,fmt,__VA_ARGS__) do {\
	int _c=!!(cond);\
	if(_c)\
		attron(COLOR_PAIR(1));\
	printw(fmt,__VA_ARGS__);\
	if(_c)\
		attroff(COLOR_PAIR(1));\
}while(0)
#define pwcr(cond,fmt,__VA_ARGS__) do {\
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
	int r,r1;
	//char buf[512];
st:
	clear();
	assert(pd->ui[cur].spec);
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
			ts("crit_effect"),ub.crit_effect,
			ts("physical_bonus"),ub.physical_bonus,
			ts("magical_bonus"),ub.magical_bonus,
			ts("physical_derate"),ub.physical_derate,
			ts("magical_derate"),ub.magical_derate
			);
	if(ub.max_spi!=128)
		printw("%s:%ld\n",ts("max_spi"),ub.max_spi);
	for(int i=0;i<8;++i){
		if(!pd->ui[cur].moves[i])
			continue;
		p=pd->ui[cur].moves[i];
		printw("%d:(%s)%s\n",i,type2str(move_type(p)),move_ts(p));
	}
	move(LINES-3,0);
	printw("%s:%lu",ts("xp"),pd->xp);
	r=xp_require(pd->ui+cur);
	if(pd->ui[cur].level<150)
		printw(" %s:%d",ts("xp_require"),r);
	r1=pd->ui[cur].spec->evolve_level;
	if(r1){
		printw(" %s:%d (%s)",ts("evolve_level"),r1,unit_ts(pd->ui[cur].spec[1].max.id));
	}else
		printw(" %s",ts("unevolvable"));
	addch('\n');
	move(LINES-2,0);
	pwc(hcur==0,"%s",ts("move"));
	addch('\t');
	if(pd->ui[cur].level>=150||pd->xp<r)
		pwcr(hcur==1,"%s",ts("level_up"));
	else
		pwc(hcur==1,"%s",ts("level_up"));
	addch('\t');
	if(!r1||pd->ui[cur].level<r1)
		pwcr(hcur==2,"%s",ts("evolve"));
	else
		pwc(hcur==2,"%s",ts("evolve"));
	addch('\t');
	pwc(hcur==3,"%s",ts("exit"));
	move(LINES-1,0);
	printw("t:top u:up i:down q:cancel");
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
		case 'K':
			for(;;){
				if((pd->ui+cur)->level>=150)
					break;
				r=xp_require(pd->ui+cur);
				if(pd->xp<r)
					break;
				pd->xp-=r;
				++(pd->ui+cur)->level;
			}
			goto st;
		case '\n':
			switch(hcur){
				case 0:
					writemove(pd->ui+cur);
					goto st;
				case 1:
					if((pd->ui+cur)->level>=150)
						goto st;
					r=xp_require(pd->ui+cur);
					if(pd->xp<r)
						goto st;
					pd->xp-=r;
					++(pd->ui+cur)->level;
					goto st;
				case 2:
					r=(pd->ui+cur)->spec->evolve_level;
					if(!r||(pd->ui+cur)->level<r)
						goto st;
					++(pd->ui+cur)->spec;
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
void endless_win(struct player_data *pd){
	pd->xp+=271*pd->endless_level;
	if(pd->endless_level<INT_MAX)
		++pd->endless_level;
}
void endless_menu(struct player_data *pd){
	int my,mx,cur=0,r=-1;
	struct player_data p;
st:
	clear();
	getmaxyx(stdscr,my,mx);
	move(my/2-1,mx/3-1);
	printw("%s",ts("endless_challenge"));
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
		case KEY_DOWN:
			--pd->endless_level;
			goto st;
		case KEY_UP:
			++pd->endless_level;
			goto st;
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
					assert(!pdata_fake(&p,"icefield_tiger",pd->endless_level));
					endwin();
					tm_init();
					r=pbattle(pd,&p,term_selector,rand_selector,reporter_term,NULL);
					tm_end();
					scr();
					if(!r)
						endless_win(pd);
					break;
				case 2:
					for(int i=0;i<1024;++i){
						assert(!pdata_fake(&p,"icefield_tiger",pd->endless_level));
						r=pbattle(pd,&p,rand_selector,rand_selector,NULL,NULL);
						if(!r)
							endless_win(pd);
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
struct mm_option {
	const char *name;
	void (*submenu)(struct player_data *);
};
const struct mm_option mmop[]={
	{
		.name="units_on_battling",
		.submenu=units_menu,
	},
	{
		.name="endless_challenge",
		.submenu=endless_menu,
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
			if(cur<mmop_size-1)
				++cur;
			goto st;
		case KEY_UP:
			if(cur>0)
				--cur;
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
