#ifndef zstr_h_included
#define zstr_h_included

struct zstr_s;

ZSTR zs_new(void);
ZSTR zs_newn(unsigned int min);
void zs_free(ZSTR);
void zs_del(ZSTR *);
char * zs_str(ZSTR);
unsigned int zs_len(ZSTR zstr);
unsigned int zs_allocsize(ZSTR zstr);
char * zs_fix(ZSTR zstr);
char * zs_set_len(ZSTR zstr, unsigned int len);
char * zs_set(ZSTR zstr, const char *);
char * zs_cat(ZSTR zstr, const char *);
char * zs_catc(ZSTR zstr, char);
char * zs_setf(ZSTR zstr, const char * fmt, ...);
char * zs_catf(ZSTR zstr, const char * fmt, ...);
char * zs_setv(ZSTR zstr, const char * fmt, va_list args);
char * zs_catv(ZSTR zstr, const char * fmt, va_list args);
char * zs_clear(ZSTR zstr);
char * zs_reserve(ZSTR zstr, unsigned int min);
char * zs_reserve_extra(ZSTR zstr, unsigned int delta);

#endif /* zstr_h_included */
