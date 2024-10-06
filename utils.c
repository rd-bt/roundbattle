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
	for(int i=150,r=0;i>=0&&r<8;--i){
		if(!(p=spec->moves[i]))
			continue;
		ui->moves[r++]=p;
	}
	return 0;
}
