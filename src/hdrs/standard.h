/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=============================================================
 * standard.h -- Define standard macros and types
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.0 - 05 May 94    3.0.2 - 23 Nov 94
 *   3.0.3 - 25 Jun 95
 *===========================================================*/
#ifndef _STANDARD_H
#define _STANDARD_H

/*
 * This file includes config.h at bottom via include of llnls.h
 */

#include "sys_inc.h"

#if defined __CYGWIN32__ && !defined __CYGWIN__
# define __CYGWIN__ __CYGWIN32__
#endif

#if defined WIN32 && !defined __CYGWIN__

#define LLREADTEXT "rt"
#define LLREADBINARY "rb"
#define LLREADBINARYUPDATE "r+b"
#define LLWRITETEXT "wt"
#define LLWRITEBINARY "wb"
#define LLAPPENDTEXT "at"

#define LLFILERANDOM "R"
#define LLFILETEMP "T"

#define LLSTRPATHSEPARATOR ";"
#define LLSTRDIRSEPARATOR "\\"
#define LLCHRPATHSEPARATOR ';'
#define LLCHRDIRSEPARATOR '\\'

#else

#define LLREADTEXT "r"
#define LLREADBINARY "r"
#define LLREADBINARYUPDATE "r+"
#define LLWRITETEXT "w"
#define LLWRITEBINARY "w"
#define LLAPPENDTEXT "a"

#define LLFILERANDOM ""
#define LLFILETEMP ""

#define LLSTRPATHSEPARATOR ":"
#define LLSTRDIRSEPARATOR "/"
#define LLCHRPATHSEPARATOR ':'
#define LLCHRDIRSEPARATOR '/'

#endif

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

/* Having 'char *' instead of 'unsigned char *' removes about */
/* 1800 compiler warnings.  We still need to investigate *every* */
/* instance where a STRING is dereferenced and the contents acted */
/* upon directly to determine if there are sign problems */

/* STRING TYPES */
typedef char *STRING;
typedef const char *CNSTRING;
typedef unsigned char uchar;

/* BOOLEAN TYPES */
#ifndef BOOLEAN
#define BOOLEAN	int
#endif
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/* INTEGER AND FLOATING POINT TYPES */
#define INT	int32_t 
#define SHORT	int16_t
#define FLOAT	double

/* VOID TYPE */
typedef void *VPTR;
#define VPTRSIZE sizeof(VPTR)

/* GENERIC TYPE */
typedef union {
        BOOLEAN b;
        INT     i;
        FLOAT   f;
        STRING  s;
        VPTR    w;      /* Try not to use this! */
} UNION;

#define MAXLINELEN 512

/*typedef VPTR (*FUNC)();*/

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef min
#define min(x,y) ((x)>(y)?(y):(x))
#endif

#ifndef NULL
#	define NULL 0
#endif

extern BOOLEAN alloclog;
/* "stdalloc (" -- see macro below */
#define stdalloc(l)   __allocate(l, __FILE__, __LINE__)
/* "stdfree (" -- see macro below */
#define stdfree(p)    __deallocate(p, __FILE__, __LINE__)
/* "stdrealloc (" -- see macro below */
#define stdrealloc(p, size) __reallocate(p, size, __FILE__, __LINE__)
/* "FATAL (" -- see macro below */
#define FATAL()       __fatal(__FILE__, __LINE__, NULL)
/* "FATAL2 (" -- see macro below */
#define FATAL2(qq)    __fatal(__FILE__, __LINE__, qq)
/* "ASSERT (" -- see macro below */
#define ASSERT(b)     if(!(b)) __fatal(__FILE__, __LINE__, NULL)

/*
eqstr does exact byte compare
no locale, no custom sort, no Finnish option, no UTF-8 aware
Perry, 2001/07/20
*/
/* "eqstr (" -- see macro below -- byte-by-byte comparison, no locale, no UTF-8 */
#define eqstr(s,t)       (!strcmp((s),(t)))
/* "eqstrn (" -- see macro below -- byte-by-byte comparison, no locale, no UTF-8 */
#define eqstrn(s,t,len)  (!strncmp((s), (t), (len)))
/* "nestr (" -- see macro below -- byte-by-byte comparison, no locale, no UTF-8 */
#define nestr(s,t)       (strcmp((s),(t)))
/* "cmpstr (" -- see macro below -- byte-by-byte comparison, no locale, no UTF-8*/
#define cmpstr(s,t)      (strcmp((s),(t)))
/* "cmpstrloc (" -- see macro below -- locale-aware comparison, handles UTF-8*/
#define cmpstrloc(s,t)   (ll_strcmploc((s),(t)))

#define check_cache()   ___check_cache(__LINE__, __FILE__)

/* TRANSLFNC must return stdalloc'd memory
translation function type, used by some higher-level
functions to pass in translations to lower-level functions */
typedef STRING (*TRANSLFNC)(STRING str, INT len);

/* types returned by chartype */
#define WHITE  ' '
#define LETTER 'a'
#define DIGIT  '0'
#define ZERO    0

#define ISNULL(k)	(!k || *k == 0)

#include "list.h"


/*
	ARRSIZE is to make compiler insert size of
		(1) static array (# elements)
		or
		(2) static string (# characters) (will matter if we shift to wchar_t)
	ONLY use this for true arrays & locally sized strings.
	NEVER use this for pointers!
*/
#define ARRSIZE(qq) ((INT)(sizeof(qq)/sizeof(qq[0])))

/* forward declaration for containers */
struct tag_vtable;
#ifndef GENERIC_TAG_GENERIC_DECLARED
typedef struct generic_tag GENERIC;
#define GENERIC_TAG_GENERIC_DECLARED
#endif /* #ifndef GENERIC_TAG_GENERIC_DECLARED */

/* forward declaration for objects */
typedef struct tag_vtable ** OBJECT;

/*
 short format ISO style time string:
 20 bytes 0-terminated YYYY-MM-DD-HH:MM:SS ending with Z (GMT)
   0 filled DD,HH, and MM
*/
typedef struct tag_lldate {
	char datestr[21];
} LLDATE;

typedef enum { RECORD_ERROR, RECORD_NOT_FOUND, RECORD_SUCCESS } 
	RECORD_STATUS;

/* types for zstrings */
struct tag_zstr;
typedef struct tag_zstr * ZSTR;
typedef const ZSTR ZCSTR;

/*
 Pull in declarations & macros for NLS (National Language Support)
*/
#include "llnls.h"

#endif
