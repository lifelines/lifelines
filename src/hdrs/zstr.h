#ifndef zstr_h_included
#define zstr_h_included

struct zstr_s;

ZSTR zs_new(void);
ZSTR zs_newn(unsigned int min);
void zs_free(ZSTR * pzstr);
char * zs_str(ZSTR);
unsigned int zs_len(ZSTR zstr);
unsigned int zs_allocsize(ZSTR zstr);
char * zs_fix(ZSTR zstr);
char * zs_set_len(ZSTR * pzstr, unsigned int len);
char * zs_sets(ZSTR * pzstr, const char *);
char * zs_setz(ZSTR * pzstr, ZSTR zsrc);
char * zs_cats(ZSTR * pzstr, const char *);
char * zs_catc(ZSTR * pzstr, char);
char * zs_setf(ZSTR * pzstr, const char * fmt, ...);
char * zs_catf(ZSTR * pzstr, const char * fmt, ...);
char * zs_setv(ZSTR * pzstr, const char * fmt, va_list args);
char * zs_catv(ZSTR * pzstr, const char * fmt, va_list args);
char * zs_clear(ZSTR * pzstr);
char * zs_reserve(ZSTR * pzstr, unsigned int min);
char * zs_reserve_extra(ZSTR * pzstr, unsigned int delta);

#endif /* zstr_h_included */
