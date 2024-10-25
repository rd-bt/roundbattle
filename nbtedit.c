#include "nbt.h"
#include "battle-core.h"
#include "expr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>
#include <assert.h>
#include <math.h>
#include <fcntl.h>
#include "readall.c"
#ifndef _SSIZE_T_DEFINED_
#define _SSIZE_T_DEFINED_
typedef ptrdiff_t ssize_t;
#else
_Static_assert(sizeof(ssize_t)==sizeof(ptrdiff_t),"ssize_t and ptrdiff_t has different size");
#endif
void scr(void){
	assert(initscr()&&has_colors());
	noecho();
	curs_set(0);
	start_color();
	init_pair(1,COLOR_CYAN,COLOR_BLACK);
	init_pair(2,COLOR_RED,COLOR_BLACK);
	init_pair(3,COLOR_BLACK,COLOR_YELLOW);
	keypad(stdscr,1);
	cbreak();
}
static void data_write(const char *data_file,struct nbt_node *np){
	size_t sz;
	void *buf;
	int fd=open(data_file,O_WRONLY|O_CREAT|O_TRUNC,0660);
	if(fd<0)
		goto fail;
	sz=nbt_size(np);
	buf=malloc(sz);
	if(!buf)
		goto fail1;
	assert(sz==nbt_write(np,buf));
	if(write(fd,buf,sz)<sz)
		goto fail2;
	close(fd);
	free(buf);
	return;
fail2:
	free(buf);
fail1:
	close(fd);
fail:
	if(errno)
		perror("cannot write data");
	else
		fprintf(stderr,"cannot write data\n");
}
static struct nbt_node *nbt_empty(const char *key,size_t len,...){
	struct nbt_node *p=nbt_root("root",4),*r;
	if(!p)
		return NULL;
	r=nbt_list(key,len,p);
	if(r)
		return r;
	nbt_free(p);
	return NULL;
}
#define nbt_newroot(key,len,x) nbt_root(key,len)
#define nbt_newstr(key,len,x) nbt_str(key,len,"",0)
#define nbt_newzda(key,len,x) nbt_zda(key,len,NULL,0)
#define nbt_newzua(key,len,x) nbt_zua(key,len,NULL,0)
#define nbt_newdbla(key,len,x) nbt_dbla(key,len,NULL,0)
#define nbt_newbytea(key,len,x) nbt_bytea(key,len,NULL,0)
#define nbt_newlista(key,len,x) nbt_lista(key,len,NULL,0)
const char *inputbox(const char *initial,const char *title,const char *enter,size_t width){
	size_t len=strlen(title);
	size_t elen=strlen(enter);
	size_t x,l,lb;
	static char in[4096];
	static char in_bak[4096];
	int c=-1;
	l=strlen(initial);
	if(l>=width)
		l=width-1;
	memcpy(in,initial,l);
	memcpy(in_bak,in,lb=l);
st:
	clear();
	if(c>=0){
		printw("last key:0%o",c);
	}
	x=len<COLS?(COLS-len)/2:0;
	move(LINES/2-2,x);
	addstr(title);
	x=width<COLS?(COLS-width)/2:0;
	move(LINES/2,x);
	attron(COLOR_PAIR(3));
	for(size_t i=(width-l)/2;i;--i){
		addch(' ');
	}
	in[l]=0;
	addstr(in);
	for(size_t i=(width-l)/2;i;--i){
		addch(' ');
	}
	attroff(COLOR_PAIR(3));
	x=elen<COLS?(COLS-elen)/2:0;
	move(LINES/2+2,x);
	attron(COLOR_PAIR(1));
	addstr(enter);
	attroff(COLOR_PAIR(1));
	switch(c=getch()){
		case KEY_BACKSPACE:
			l=0;
			goto st;
		case '\033':
			in_bak[lb]=0;
			return in_bak;
		case '\n':
			in[l]=0;
			return in;
		case '\177':
			if(l>0){
				--l;
			}
			goto st;
		default:
			if(l<4096&&l<width){
				in[l++]=c;
			}
			goto st;
	}
}
size_t inzu(size_t cur){
	char buf[1024];
	const char *p;
	size_t r;
	double d;
	char c;
	sprintf(buf,"%zu",cur);
	p=inputbox(buf,"input size_t value","enter to apply",32);
	if(sscanf(p,"%zu%c",&r,&c)==1)
		return r;
	d=expr_calc(p);
	return expr_isnan(d)?0:(size_t)d;
}
ptrdiff_t inzd(ptrdiff_t cur){
	char buf[1024];
	const char *p;
	ptrdiff_t r;
	double d;
	char c;
	sprintf(buf,"%zd",cur);
	p=inputbox(buf,"input ptrdiff_t value","enter to apply",32);
	if(sscanf(p,"%zd%c",&r,&c)==1)
		return r;
	d=expr_calc(p);
	return expr_isnan(d)?0:(ptrdiff_t)d;
}
double indbl(double cur){
	char buf[1024];
	const char *p;
	double r;
	sprintf(buf,"%lf",cur);
	p=inputbox(buf,"input double value","enter to apply",32);
	if(!strcmp(p,"nan")||!strcmp(p,"NAN"))
		return NAN;
	r=expr_calc(p);
	return expr_isnan(r)?0:r;
}
const char *type_name[]={"root","zd(ptrdiff_t)","zu(size_t)","dbl(double)","list(nbt *)","zda(ptrdiff_t[])","zua(size_t[])","dbla(double[])","str(char[])","byte(uchar[])","lista(nbt *[])"};
void printvalue(const struct nbt_node *np,ssize_t hcur){
	size_t count,i;
	struct nbt_node **npp;
	switch(np->type){
		case NBT_LISTA:
			addch('[');
			count=np->count;
			npp=nbt_listap(np);
			for(size_t i=0;i<count;){
				if(i==hcur)
					attron(COLOR_PAIR(1));
				printw("(%zu fields)",nbt_count(npp[i]));
				if(i==hcur)
					attroff(COLOR_PAIR(1));
				++i;
				if(i<count)
					addch(',');
			}
			addch(']');
			break;
		case NBT_STR:
			addch('\"');
			addstr(nbt_strp(np));
			addch('\"');
			break;
#define print_elements_in_array(T,fmt) \
			addch('[');\
			count=np->count;\
			i=0;\
			for(T *p=nbt_ap(np);i<count;){\
				if(i==hcur)\
					attron(COLOR_PAIR(1));\
				printw(fmt,p[i]);\
				if(i==hcur)\
					attroff(COLOR_PAIR(1));\
				++i;\
				if(i<count)\
					addch(',');\
			}\
			addch(']')
		case NBT_BYTEA:
			print_elements_in_array(unsigned char,"%hhu");
			break;
		case NBT_ZDA:
			print_elements_in_array(ptrdiff_t,"%zd");
			break;
		case NBT_ZUA:
			print_elements_in_array(size_t,"%zu");
			break;
		case NBT_DBLA:
			print_elements_in_array(double,"%lf");
			break;
		case NBT_LIST:
			printw("(%zu fields)",nbt_count(nbt_listl(np)));
			break;
		case NBT_ZD:
			printw("%zd",nbt_zdl(np));
			break;
		case NBT_ZU:
			printw("%zu",nbt_zul(np));
			break;
		case NBT_DBL:
			printw("%lf",nbt_dbll(np));
			break;
		case NBT_ROOT:
			break;
		default:
			break;
	}
}
int edit_menu(struct nbt_node **npp){
	ssize_t cur=0,count,hcur=0;
	size_t x;
	struct nbt_node *np1,*npc,*np=*npp;
	const char *p;
st_di:
	count=nbt_count(np);
	assert(count);
st:
	cur=limit_ring(cur,0,count-1);
	npc=nbt_byindex(np,cur);
	hcur=limit_ring(hcur,0,npc->count>0?npc->count-1:0);
	clear();
	for(size_t i=0;i<count;++i){
		np1=nbt_byindex(np,i);
		if(np1==np)
			attron(A_UNDERLINE);
		if(i==cur)
			attron(np1->type==NBT_ROOT?COLOR_PAIR(2):COLOR_PAIR(1));
		if(np1->type!=NBT_ROOT){
			printw("[%s] %s:",type_name[np1->type],np1->data);
			if(i==cur)
				attroff(COLOR_PAIR(1));
			printvalue(np1,i==cur?hcur:-1);
			addch('\n');
		}else {
			printw("[%s] %s\n",type_name[np1->type],np1->data);
			if(i==cur)
				attroff(COLOR_PAIR(2));
		}
		if(np1==np)
			attroff(A_UNDERLINE);
	}
	move(LINES-2,0);
	printw("z:zd u:zu f:dbl n:node Z/U/F/N/B:array s:str r:root");
	move(LINES-1,0);
	printw("x:ok enter:edit q:cancel d:delete i:insert m:rename R:setroot");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,count-1);
			hcur=limit_ring(hcur,0,(ssize_t)nbt_byindex(np,cur)->count-1);
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,count-1);
			hcur=limit_ring(hcur,0,(ssize_t)nbt_byindex(np,cur)->count-1);
			goto st;
		case KEY_LEFT:
			hcur=limit_ring(hcur-1,0,(ssize_t)npc->count-1);
			goto st;
		case KEY_RIGHT:
			hcur=limit_ring(hcur+1,0,(ssize_t)npc->count-1);
			goto st;
		case '\n':
			switch(npc->type){
				case NBT_ZD:
					nbt_zdl(npc)=inzd(nbt_zdl(npc));
					break;
				case NBT_ZU:
					nbt_zul(npc)=inzu(nbt_zul(npc));
					break;
				case NBT_DBL:
					nbt_dbll(npc)=indbl(nbt_dbll(npc));
					break;
				case NBT_LIST:
					edit_menu(&nbt_listl(npc));
					break;
				case NBT_ZDA:
					nbt_zdap(npc)[hcur]=inzd(nbt_zdap(npc)[hcur]);
					break;
				case NBT_ZUA:
					nbt_zuap(npc)[hcur]=inzu(nbt_zuap(npc)[hcur]);
					break;
				case NBT_DBLA:
					nbt_dblap(npc)[hcur]=indbl(nbt_dblap(npc)[hcur]);
					break;
				case NBT_BYTEA:
					nbt_byteap(npc)[hcur]=inzu(nbt_byteap(npc)[hcur]);
					break;
				case NBT_STR:
					if(npc==np){
						p=inputbox(nbt_strp(npc),"input string","enter to apply",np->len-nbt_align(np->keylen+1)-1);
						x=strlen(p);
						np->count=x;
						memcpy((char *)nbt_strp(np),p,x);
						memset((char *)nbt_strp(np)+x,0,np->len-nbt_align(np->keylen+1)-x);
						break;
					}
					p=inputbox(nbt_strp(npc),"input string","enter to apply",32);
					np1=nbt_str(npc->data,npc->keylen,p,strlen(p));
					if(!np1)
						break;
					nbt_replace(np,npc,np1);
					break;
				case NBT_LISTA:
					edit_menu(nbt_listap(npc)+hcur);
					break;
				default:
					break;
			}
			goto st;
		case 'i':
			if(np==npc)
				goto st;
			switch(npc->type){
				case NBT_LISTA:
					np1=nbt_root("root",4);
					if(!np1)
						goto st;
					if(!nbt_ainsert(np,npc,hcur,np1))
						nbt_free(np1);
					goto st_di;
				case NBT_BYTEA:
					nbt_ainsert(np,npc,hcur,0);
					goto st_di;
				case NBT_ZDA:
					nbt_ainsert(np,npc,hcur,0l);
					goto st_di;
				case NBT_ZUA:
					nbt_ainsert(np,npc,hcur,0ul);
					goto st_di;
				case NBT_DBLA:
					nbt_ainsert(np,npc,hcur,0.0);
					goto st_di;
				default:
					goto st;
			}
		case 'D':
			goto D;
		case 'd':
			switch(npc->type){
				case NBT_LISTA:
				case NBT_BYTEA:
				case NBT_ZDA:
				case NBT_ZUA:
				case NBT_DBLA:
					if(!npc->count)
						goto D;
					nbt_adelete(npc,hcur);
					goto st;
				default:
D:
					if(np==npc)
						goto st;
					nbt_delete(np,npc);
					goto st_di;
			}
		case 'm':
			if(np==npc)
				goto st;
			p=inputbox(npc->data,"input name","enter to apply",32);
			nbt_rename(np,npc,p,strlen(p));
			goto st;
		case 'R':
			if(np==npc)
				goto st;
			nbt_setroot(np,npc);
			np=npc;
			*npp=np;
			goto st;
#define addnode(t) \
			p=inputbox("","input name","enter to apply",32);\
			if(!*p)\
				goto st;\
			np1=nbt_##t(p,strlen(p),0);\
			if(!nbt_add(np,np1)){\
				nbt_free(np1);\
				goto st;\
			}\
			cur=nbt_index(np,np1);\
			if(cur<0)\
				cur=0;\
			goto st_di
		case 'z':
			addnode(zd);
		case 'u':
			addnode(zu);
		case 'f':
			addnode(dbl);
		case 'n':
			addnode(empty);
		case 'r':
			addnode(newroot);
		case 's':
			addnode(newstr);
		case 'Z':
			addnode(newzda);
		case 'U':
			addnode(newzua);
		case 'F':
			addnode(newdbla);
		case 'N':
			addnode(newlista);
		case 'B':
			addnode(newbytea);
		case 'q':
			goto end;
		case 'x':
			goto endx;
		default:
			goto st;
	}
end:
	return 0;
endx:
	return 1;
}
int edit(char *file){
	int fd;
	struct nbt_node *np;
	ssize_t sz;
	char *buf;
	fd=open(file,O_RDONLY);
	if(fd<0){
		perror("open()");
		exit(EXIT_FAILURE);
	}
	buf=readall(fd,&sz);
	close(fd);
	if(!buf){
		perror("readall()");
		exit(EXIT_FAILURE);
	}
	errno=0;
	np=nbt_read(buf,sz);
	free(buf);
	if(!np){
		if(errno)
			perror("malloc()");
		else
			fprintf(stderr,"bad data\n");
		exit(EXIT_FAILURE);
	}
	scr();
	fd=edit_menu(&np);
	endwin();
	if(fd)
		data_write(file,np);
	nbt_free(np);
	return 0;
}
int main(int argc,char **argv){
	if(argc<2){
		fprintf(stderr,"no file input\n");
		exit(EXIT_FAILURE);
	}
	for(char **p=argv+1;*p;++p){
		edit(*p);
	}
}
