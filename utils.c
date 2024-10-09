#include "utils.h"
#include <string.h>
int ui_create(struct unit_info *ui,const char *id,int level){
	const struct species *spec;
	const char *p;
	spec=get_builtin_species_by_id(id);
	if(!spec)
		return -1;
	memset(ui,0,sizeof(struct unit_info));
	ui->spec=spec;
	ui->level=level;
	if(spec->flag&UF_CANSELECTTYPE){
		ui->type0=spec->max.type0;
		ui->type1=spec->max.type1;
	}
	for(int i=level>150?150:level,r=0;i>=0&&r<8;--i){
		if(!(p=spec->moves[i]))
			continue;
		ui->moves[r++]=p;
	}
	return 0;
}
