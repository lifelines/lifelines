#include <io.h> /* open() */
/* all MS' open stuff has leading underscores */
#define stat _stat
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
#define fstat _fstat

//#include <malloc.h>
#define STDC_HEADERS 1
#define HAVE_STRING_H 1
#define HAVE_STRCHR 1
#define HAVE_STDLIB_H 1
#define HAVE_MBLEN 1
#define HAVE_VPRINTF 1
#define HAVE_GETCWD 1
#define __STDC__ 1
#include <stddef.h> /* for size_t for stpncpy.c */
//#include <ctype.h> /* for isascii for print_parse.h */
#define isascii __isascii
	/* print-parse.h uses inline, but MSVC doesn't support it for C */
#define inline 

/* I don't know what to do with these -- Perry, 2002/01/01 */
#define GNULOCALEDIR "."
#define LOCALE_ALIAS_PATH "."
#define LOCALEDIR "."

/*
#define stat _stat

#define open _open
#define O_RDONLY _O_RDONLY
*/

