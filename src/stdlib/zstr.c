#include "llstdlib.h"
#include "arch.h" /* vsnprintf */
#include "zstr.h"


static void dbgchk(ZSTR);

#define DEFSIZE 64


#define DBGCHK(zq) dbgchk(zq)

static char *
safez (char * str)
{
	return str ? str : "";
}

#ifdef TEST_ZSTR
int
main()
{
	ZSTR zstr = zs_new();
	ASSERT(zstr);

	printf("zstr=%s\n", safez(zs_str(zstr)));

	zs_cpy(zstr, "dogs");
	ASSERT(0 == strcmp(zs_str(zstr), "dogs"));
	ASSERT(4 == zs_len(zstr));

	printf("zstr=%s\n", safez(zs_str(zstr)));
  

	zs_del(&zstr);
	ASSERT(!zstr);
}
#endif


struct zstr_s {
	char * str;
	char * end;
	unsigned int max;
	int magic;
};

static void
zalloc (ZSTR zstr, unsigned int newmax)
{
	char * ptr;
	int len = zs_len(zstr);
	while (zstr->max < newmax)
		zstr->max = zstr->max << 1;
	ptr = (char *)malloc(zstr->max);
	/* use memcpy not strcpy in case has embedded nulls */
	memcpy(ptr, zstr->str, len+1);
	free(zstr->str);
	zstr->str = ptr;
	zstr->end = zstr->str + len;
	DBGCHK(zstr);
}
/* validate zstring */
static void
dbgchk (ZSTR zstr)
{
	ASSERT(zstr);
	ASSERT(zstr->magic == 7843);
	ASSERT(zstr->str);
	ASSERT(zstr->end);
	ASSERT(zstr->max);
	ASSERT(zstr->end >= zstr->str && zstr->end < zstr->str + zstr->max);
	ASSERT(zstr->magic == 7843);
}
/* create & return new zstring */
ZSTR
zs_new (void)
{
	return zs_newn(DEFSIZE);
}
/* create & return new zstring with underlying buffer at least min bytes */
ZSTR
zs_newn (unsigned int min)
{
	ZSTR zstr = (ZSTR)malloc(sizeof(*zstr));
	zstr->str = (char *)malloc(min);
	zstr->str[0] = 0;
	zstr->end = zstr->str;
	zstr->max = min;
	zstr->magic = 7843;
	DBGCHK(zstr);
	return zstr;
}
/* delete pointed to zstring, and zero pointer */
void
zs_del (ZSTR * pzstr)
{
	if (*pzstr) {
		zs_free(*pzstr);
		*pzstr = 0;
	}
}
/* delete all contents & allocations */
void
zs_free (ZSTR zstr)
{
	DBGCHK(zstr);
	memset(zstr->str, 0, zstr->max);
	memset(zstr, 0, sizeof(*zstr));
	free(zstr->str);
	free(zstr);
}
/* return current string */
STRING
zs_str (ZSTR zstr)
{
	DBGCHK(zstr);
	return zstr->str;
}
/* return current length of string */
unsigned int
zs_len (ZSTR zstr)
{
	DBGCHK(zstr);
	return zstr->end - zstr->str;
}
/* return current size of underlying buffer */
unsigned int
zs_allocsize (ZSTR zstr)
{
	DBGCHK(zstr);
	return zstr->max;
}
/* update state because caller changed string */
/* Assumes simple zero-terminated string */
char *
zs_fix (ZSTR zstr)
{
	DBGCHK(zstr);
	zstr->end = zstr->str + strlen(zstr->str);
	return zstr->str;
}
/* set length directly; caller may use this if using embedded nulls */
char *
zs_set_len (ZSTR zstr, unsigned int len)
{
	DBGCHK(zstr);
	if (len == -1) {
		len = strlen(zstr->str);
	}
	ASSERT(len >= 0 && len < zstr->max);
	zstr->end = zstr->str + len;
	return zstr->str;
}
/* set zstring value to input zero-terminated string*/
char *
zs_set (ZSTR zstr, const char * txt)
{
	unsigned int tlen;
	DBGCHK(zstr);
	if (!txt || !txt[0]) return zstr->str;
	tlen = strlen(txt);
	if (tlen + 1 > zstr->max)
		zalloc(zstr, tlen+1);
	strcpy(zstr->str, txt);
	zstr->end = zstr->str + tlen;
	return zstr->str;
}
/* append zero-terminated input to zstring */
char *
zs_cat (ZSTR zstr, const char * txt)
{
	int tlen;
	DBGCHK(zstr);
	if (!txt || !txt[0]) return zstr->str;
	tlen = strlen(txt);
	if (zs_len(zstr) + tlen + 1 > zstr->max)
		zalloc(zstr, zs_len(zstr) + tlen + 1);
	strcpy(zstr->end, txt);
	zstr->end += tlen;
	return zstr->str;
}
/* append input character to zstring */
char *
zs_catc (ZSTR zstr, char ch)
{
	char buffer[2];
	buffer[0] = ch;
	buffer[1] = 0;
	return zs_cat(zstr, buffer);
}
/* set printf style input to zstring */
char *
zs_setf (ZSTR zstr, const char * fmt, ...)
{
	va_list args;
	DBGCHK(zstr);
	va_start(args, fmt);
	zs_setv(zstr, fmt, args);
	va_end(args);
	return zstr->str;
}
/* append printf style input to zstring */
char *
zs_catf (ZSTR zstr, const char * fmt, ...)
{
	va_list args;
	DBGCHK(zstr);
	va_start(args, fmt);
	zs_catv(zstr, fmt, args);
	va_end(args);
	return zstr->str;
}
/* set varargs printf style input to zstring */
char *
zs_setv (ZSTR zstr, const char * fmt, va_list args)
{
	zs_clear(zstr);
	zs_catv(zstr, fmt, args);
	return zstr->str;
}
/* append varargs printf style input to zstring */
char *
zs_catv (ZSTR zstr, const char * fmt, va_list args)
{
	/* if we know that the system implementation of snprintf was
	standards conformant, we could use snprintf(0, ...), but how to tell ? */
	static char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	zs_cat(zstr, buffer);
	return zstr->str;
}
/* set zstring to empty */
char *
zs_clear (ZSTR zstr)
{
	DBGCHK(zstr);
	zstr->str[0] = 0;
	zstr->end = zstr->str;
	return zstr->str;
}
/* ensure at least min bytes in underlying buffer */
char *
zs_reserve (ZSTR zstr, unsigned int min)
{
	if (min > zstr->max)
		zalloc(zstr, min);
	return zstr->str;
}
/* add at least min bytes more to underlying buffer */
char *
zs_reserve_extra (ZSTR zstr, unsigned int delta)
{
	zalloc(zstr, zstr->max+delta);
	return zstr->str;
}
