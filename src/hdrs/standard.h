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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * standard.h -- Define standard macros and types
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 05 May 94    3.0.2 - 23 Nov 94
 *   3.0.3 - 25 Jun 95
 *===========================================================*/
#ifndef _STANDARD_H
#define _STANDARD_H

#include "sys_inc.h"

#ifdef WIN32

#define LLREADTEXT "rt"
#define LLREADBINARY "rb"
#define LLREADBINARYUPDATE "r+b"
#define LLWRITETEXT "wt"
#define LLWRITEBINARY "wb"
#define LLAPPENDTEXT "at"

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

#define LLSTRPATHSEPARATOR ":"
#define LLSTRDIRSEPARATOR "/"
#define LLCHRPATHSEPARATOR ':'
#define LLCHRDIRSEPARATOR '/'
#endif

#include "mystring.h"

/* Having 'char *' instead of 'unsigned char *' removes about */
/* 1800 compiler warnings.  We still need to investigate *every* */
/* instance where a STRING is dereferenced and the contents acted */
/* upon directly to determine if there are sign problems */
typedef char *STRING;

#ifndef BOOLEAN
#	define BOOLEAN int
#endif
#ifndef TRUE
#       define TRUE 1
#endif
#ifndef FALSE
#       define FALSE 0
#endif
#define INT int 
#define SHORT short
#define LONG long
#define FLOAT double

typedef void *VPTR;
typedef union {
        BOOLEAN b;
        INT     i;
        FLOAT   f;
        VPTR    w;
        /*LONG   l;*/
} UNION;

#define MAXLINELEN 512

#define VPTRSIZE sizeof(VPTR)

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
#define stdalloc(l)   __allocate(l, __FILE__, __LINE__)
#define stdfree(p)    __deallocate(p, __FILE__, __LINE__)
#define fatal(s)      __fatal(__FILE__, __LINE__)
#define FATAL()       __fatal(__FILE__, __LINE__)
#define ASSERT(b)     if(!(b)) __fatal(__FILE__, __LINE__)
#define eqstr(s,t)    (!ll_strcmp((s),(t)))
#define nestr(s,t)    (ll_strcmp((s),(t)))

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))
#define check_cache()   ___check_cache(__LINE__, __FILE__)

#define WHITE  ' '
#define LETTER 'a'
#define DIGIT  '0'
#define ZERO    0

typedef struct lntag *LNODE;
struct lntag {
	VPTR l_element;
	LNODE l_prev;
	LNODE l_next;
};
#define lelement(n) ((n)->l_element)
#define lprev(n) ((n)->l_prev)
#define lnext(n) ((n)->l_next)

typedef struct ltag {
	INT l_type;
	LNODE l_first;
	LNODE l_last;
	INT l_refcnt;
} *LIST;
#define ltype(l) ((l)->l_type)
#define lfirst(l) ((l)->l_first)
#define llast(l) ((l)->l_last)
#define lrefcnt(l) ((l)->l_refcnt)

#define LISTNOFREE 0
#define LISTDOFREE 1

#define FORLIST(l,e)\
	{\
		LNODE _lnode = l->l_last;\
		VPTR e;\
		while (_lnode) {\
			e = _lnode->l_element;
#define ENDLIST\
			_lnode = _lnode->l_prev;\
		}\
	}

#define ISNULL(k)	(!k || *k == 0)


struct WAREHOUSE_S;
typedef struct WAREHOUSE_S *WAREHOUSE;

/* path.c */
BOOLEAN is_dir_sep(char c);

#endif
