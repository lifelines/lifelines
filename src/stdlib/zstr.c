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

/* reallocate buffer to be at least newmax */
static void
zalloc (ZSTR * pzstr, unsigned int newmax)
{
	ZSTR zstr = *pzstr;
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
	unsigned int bksiz = (min<2048)?(min<64?32:128):(min<16384?2048:16384);
	while (bksiz < min)
		bksiz = bksiz << 1;
	zstr->str = (char *)malloc(bksiz);
	zstr->str[0] = 0;
	zstr->end = zstr->str;
	zstr->max = bksiz;
	zstr->magic = 7843;
	DBGCHK(zstr);
	return zstr;
}
/* delete zstring & clear caller's pointer */
void
zs_free (ZSTR * pzstr)
{
	ZSTR zstr = *pzstr;
	if  (!zstr) return;
	DBGCHK(zstr);
	memset(zstr->str, 0, zstr->max);
	memset(zstr, 0, sizeof(*zstr));
	free(zstr->str);
	free(zstr);
	*pzstr = NULL;
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
	if (!zstr) return 0;
	DBGCHK(zstr);
	return zstr->end - zstr->str;
}
/* return current size of underlying buffer */
unsigned int
zs_allocsize (ZSTR zstr)
{
	if (!zstr) return 0;
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
zs_set_len (ZSTR * pzstr, unsigned int len)
{
	ZSTR zstr = *pzstr;
	if  (!zstr) {
		zstr = zs_newn(len>0?len:0);
		*pzstr = zstr;
	}
	DBGCHK(zstr);
	if (len == -1) {
		len = strlen(zstr->str);
	}
	ASSERT(len >= 0 && len < zstr->max);
	zstr->end = zstr->str + len;
	return zstr->str;
}
/* set zstring value to input zero-terminated string*/
static char *
zs_set_with_len (ZSTR * pzstr, const char * txt, unsigned int tlen)
{
	ZSTR zstr = *pzstr;
	if (!zstr) {
		*pzstr = zstr = zs_newn(tlen);
	}
	DBGCHK(zstr);
	if (!tlen) return zstr->str;
	if (tlen + 1 > zstr->max) {
		zalloc(&zstr, tlen+1);
		*pzstr = zstr;
	}
	strcpy(zstr->str, txt);
	zstr->end = zstr->str + tlen;
	return zstr->str;
}
/* set zstring value to input zero-terminated string*/
char *
zs_sets (ZSTR * pzstr, const char * txt)
{
	unsigned int tlen = txt ? strlen(txt) : 0;
	return zs_set_with_len(pzstr, txt, tlen);
}
/* set zstring value to copy of another zstring value */
char *
zs_setz (ZSTR * pzstr, ZSTR zsrc)
{
	const char * txt = zsrc ? zsrc->str : "";
	unsigned int tlen = zsrc ? zsrc->end-zsrc->str : 0;
	return zs_set_with_len(pzstr, txt, tlen);
}
/* append zero-terminated input to zstring */
char *
zs_apps (ZSTR * pzstr, const char * txt)
{
	ZSTR zstr = *pzstr;
	int tlen;
	DBGCHK(zstr);
	if (!txt || !txt[0]) return zstr->str;
	tlen = strlen(txt);
	if (zs_len(zstr) + tlen + 1 > zstr->max) {
		zalloc(&zstr, zs_len(zstr) + tlen + 1);
		*pzstr = zstr;
	}
	strcpy(zstr->end, txt);
	zstr->end += tlen;
	return zstr->str;
}
/* append input character to zstring */
char *
zs_appc (ZSTR * pzstr, char ch)
{
	char buffer[2];
	buffer[0] = ch;
	buffer[1] = 0;
	return zs_apps(pzstr, buffer);
}
/* set printf style input to zstring */
char *
zs_setf (ZSTR * pzstr, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	zs_setvf(pzstr, fmt, args);
	va_end(args);
	return (*pzstr)->str;
}
/* append printf style input to zstring */
char *
zs_appf (ZSTR * pzstr, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	zs_appvf(pzstr, fmt, args);
	va_end(args);
	return (*pzstr)->str;
}
/* set varargs printf style input to zstring */
char *
zs_setvf (ZSTR * pzstr, const char * fmt, va_list args)
{
	zs_clear(pzstr);
	zs_appvf(pzstr, fmt, args);
	return (*pzstr)->str;
}
/* append varargs printf style input to zstring */
char *
zs_appvf (ZSTR * pzstr, const char * fmt, va_list args)
{
	/* if we know that the system implementation of snprintf was
	standards conformant, we could use snprintf(0, ...), but how to tell ? */
	static char buffer[4096];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	zs_apps(pzstr, buffer);
	return (*pzstr)->str;
}
/* set zstring to empty */
char *
zs_clear (ZSTR * pzstr)
{
	ZSTR zstr = *pzstr;
	if (!zstr) {
		*pzstr = zstr = zs_new();
	}
	DBGCHK(zstr);
	zstr->str[0] = 0;
	zstr->end = zstr->str;
	return zstr->str;
}
/* ensure at least min bytes in underlying buffer */
char *
zs_reserve (ZSTR * pzstr, unsigned int min)
{
	if (!*pzstr)
		*pzstr = zs_newn(min);
	if (min > (*pzstr)->max)
		zalloc(pzstr, min);
	return (*pzstr)->str;
}
/* add at least min bytes more to underlying buffer */
char *
zs_reserve_extra (ZSTR * pzstr, unsigned int delta)
{
	if (!*pzstr) {
		*pzstr = zs_newn(delta);
	} else {
		zalloc(pzstr, (*pzstr)->max+delta);
	}
	return (*pzstr)->str;
}
