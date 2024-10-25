#ifndef _LOCALE_H_
#define _LOCALE_H_
extern int loc_disable;
const char *locale(const char *id);
const char *ts(const char *id);
const char *type_ts(const char *id);
const char *e2s(const char *id);
const char *type2str(int type);
const char *move_ts(const char *id);
const char *move_desc(const char *id);
const char *unit_ts(const char *id);
const char *unit_desc(const char *id);
const char *item_ts(const char *id);
const char *item_desc(const char *id);
#endif
