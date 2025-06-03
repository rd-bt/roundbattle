const char *cont_id_fake(int level){
	const char *cp=mobs[(level-1)%arrsize(mobs)];
	const struct species *spec=get_builtin_species_by_id(cp);
	while((spec->flag&UF_EVOLVABLE)&&level>=spec->evolve_level&&spec[1].xp_type<=spec->xp_type*16){
		++spec;
		cp=spec->max.id;
	}
	return cp;
}
void cont_reset(struct unit *u,long level){
	u->state=UNIT_FAILED;
	unit_wipeeffect(u,0);
	mkbase_id(cont_id_fake(level),level,(struct unit_base *)u->base);
	unit_fillattr(u);
	report(u->owner->field,MSG_UPDATE,u);
}
void cont_roundstart(struct effect *e){
	struct unit *u;
	u=e->src;
	if(!isalive(u->state)){
		effect_addlevel(e,1);
		cont_reset(u,e->level);
	}
	u=e->src==e->src->owner->units?e->src+1:e->src-1;
	if(!isalive(u->state)){
		effect_addlevel(e,1);
		cont_reset(u,e->level);
	}
}
void cont_kill_end(struct effect *e,struct unit *u){
	if(u->owner!=e->src->owner)
		return;
	effect_addlevel(e,1);
	if(u==u->owner->units)
		++u;
	else
		--u;
	cont_reset(u,e->level);

}
const struct effect_base cont_battle[1]={{
	//.kill_end=cont_kill_end,
	.roundstart=cont_roundstart,
	.flag=EFFECT_PASSIVE,
}};
void cont_init(struct battle_field *bf){
	effect(cont_battle,NULL,bf->e->front,151,-1);
}
void continuous_battle(struct player_data *p){
	struct unit_base ubp[6],ube[2];
	struct player p0,e0;
	memset(&p0,0,sizeof(struct player));
	memset(&e0,0,sizeof(struct player));
	for(int i=0;i<6;++i){
		if(p->ui[i].spec){
			mkbase(p->ui+i,ubp+i);
			p0.units[i].base=ubp+i;
		}else
			p0.units[i].base=NULL;
	}
	mkbase_id(cont_id_fake(150),150,ube);
	mkbase_id(cont_id_fake(151),151,ube+1);
	e0.units->base=ube;
	e0.units[1].base=ube+1;
	p0.selector=term_selector;
	e0.selector=rand_selector;
	p0.reporter=reporter_term;
	e0.reporter=NULL;
	endwin();
	tm_init();
	battle(&p0,&e0,NULL,cont_init);
	tm_end();
	scr();
}
void hp100_battle(struct player_data *p){
	struct unit_base ubp[6],ube[1];
	struct player p0,e0;
	memset(&p0,0,sizeof(struct player));
	memset(&e0,0,sizeof(struct player));
	for(int i=0;i<6;++i){
		if(p->ui[i].spec){
			mkbase(p->ui+i,ubp+i);
			p0.units[i].base=ubp+i;
			ubp[i].max_hp*=100;
		}else
			p0.units[i].base=NULL;
	}
	mkbase_id(cont_id_fake(randi()),150,ube);
	e0.units->base=ube;
	ube->max_hp*=100;
	p0.selector=term_selector;
	e0.selector=rand_selector;
	p0.reporter=reporter_term;
	e0.reporter=NULL;
	endwin();
	tm_init();
	battle(&p0,&e0,NULL,NULL);
	tm_end();
	scr();
}
void hp1_battle(struct player_data *p){
	struct unit_base ubp[6],ube[1];
	struct player p0,e0;
	memset(&p0,0,sizeof(struct player));
	memset(&e0,0,sizeof(struct player));
	for(int i=0;i<6;++i){
		if(p->ui[i].spec){
			mkbase(p->ui+i,ubp+i);
			p0.units[i].base=ubp+i;
			ubp[i].max_hp=1;
		}else
			p0.units[i].base=NULL;
	}
	mkbase_id(cont_id_fake(randi()),150,ube);
	e0.units->base=ube;
	ube->max_hp=1;
	p0.selector=term_selector;
	e0.selector=rand_selector;
	p0.reporter=reporter_term;
	e0.reporter=NULL;
	endwin();
	tm_init();
	battle(&p0,&e0,NULL,NULL);
	tm_end();
	scr();
}
void hpatkdef10000_battle(struct player_data *p){
	struct unit_base ubp[6],ube[1];
	struct player p0,e0;
	memset(&p0,0,sizeof(struct player));
	memset(&e0,0,sizeof(struct player));
	for(int i=0;i<6;++i){
		if(p->ui[i].spec){
			mkbase(p->ui+i,ubp+i);
			p0.units[i].base=ubp+i;
			ubp[i].max_hp*=10000;
			ubp[i].atk*=10000;
			ubp[i].def*=10000;
		}else
			p0.units[i].base=NULL;
	}
	mkbase_id(cont_id_fake(randi()),150,ube);
	e0.units->base=ube;
	ube->max_hp*=10000;
	ube->atk*=10000;
	ube->def*=10000;
	p0.selector=term_selector;
	e0.selector=rand_selector;
	p0.reporter=reporter_term;
	e0.reporter=NULL;
	endwin();
	tm_init();
	battle(&p0,&e0,NULL,NULL);
	tm_end();
	scr();
}
const struct mm_option fun_modes[]={
	{
		.name="continuous_battle",
		.submenu=continuous_battle,
	},
	{
		.name="hp100",
		.submenu=hp100_battle,
	},
	{
		.name="hp1",
		.submenu=hp1_battle,
	},
	{
		.name="hpatkdef10000",
		.submenu=hpatkdef10000_battle,
	},
	{
		.name="exit",
		.submenu=NULL,
	},
	{NULL}
};
const size_t fun_modes_size=arrsize(fun_modes)-1;
void fun_menu(struct player_data *pd){
	int cur=0;
st:
	clear();
	move(0,0);
	for(int i=0;fun_modes[i].name;++i){
		if(i==cur)
			attron(COLOR_PAIR(1));
		printw("%s\n",ts(fun_modes[i].name));
		if(i==cur)
			attroff(COLOR_PAIR(1));
	}
	move(LINES-1,0);
	printw("q:cancel");
	refresh();
	switch(getch()){
		case KEY_DOWN:
			cur=limit_ring(cur+1,0,(ssize_t)(fun_modes_size-1));
			goto st;
		case KEY_UP:
			cur=limit_ring(cur-1,0,(ssize_t)(fun_modes_size-1));
			goto st;
		case '\n':
			if(!fun_modes[cur].submenu)
				return;
			fun_modes[cur].submenu(pd);
			goto st;
		case 'q':
			return;
		default:
			goto st;
	}
}
