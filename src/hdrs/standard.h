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

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#include <string.h>

/* WARNING: Borland C++ 5.02 declares wprintf() in stdio.h. Redefine LifeLines
 * wprintf to llwprintf. pbm 07 Jan 2000
 */

#include "mystring.h"

typedef unsigned char *STRING;
#ifndef BOOLEAN
#	define BOOLEAN int
#endif
#ifndef TRUE
#	define TRUE 1
#endif
#ifndef FALSE
#	define FALSE 0
#endif
#define INT int 
#define SHORT short
#define LONG long
#define FLOAT double

typedef void *WORD;
typedef union {
        BOOLEAN b;
        INT     i;
        FLOAT   f;
        WORD    w;
        /*LONG   l;*/
} UNION;

#define MAXLINELEN 512

#define WORDSIZE sizeof(WORD)

/*typedef WORD (*FUNC)();*/

#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef min
#define min(x,y) ((x)>(y)?(y):(x))
#endif

#ifndef NULL
#	define NULL 0
#endif

STRING strsave(), strconcat();

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

extern char *malloc();

#define WHITE  ' '
#define LETTER 'a'
#define DIGIT  '0'
#define ZERO    0

typedef struct lntag *LNODE;
struct lntag {
	WORD l_element;
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
} *LIST;
#define ltype(l) ((l)->l_type)
#define lfirst(l) ((l)->l_first)
#define llast(l) ((l)->l_last)

#define LISTNOFREE 0
#define LISTDOFREE 1

/* External Functions */
LIST create_list(void);
BOOLEAN empty_list(LIST);
WORD pop_list(LIST);
void enqueue_list(LIST, WORD);
WORD dequeue_list(LIST);
void set_list_element(LIST, INT, WORD);
WORD get_list_element(LIST, INT);
INT length_list(LIST);

STRING strsave(STRING);
STRING strconcat(STRING, STRING);
INT chartype(INT);
BOOLEAN iswhite(INT);
BOOLEAN isletter(INT);
BOOLEAN isnumeric(STRING);
STRING lower(STRING);
STRING upper(STRING);
STRING capitalize(STRING);
INT ll_toupper(INT);
INT ll_tolower(INT);
STRING trim(STRING, INT);

STRING filepath(STRING, STRING, STRING, STRING);
FILE* fopenpath(STRING, STRING, STRING, STRING, STRING*);
STRING lastpathname(STRING);

void __fatal(STRING, int);
void __assert(BOOLEAN, STRING, int);

int ll_strcmp(char*, char*);
int ll_strncmp(char*, char*, int);

/* Internal Functions */
void make_list_empty(LIST);
void set_list_type(LIST, INT);
void push_list(LIST, WORD);
void remove_list(LIST, int (*func)());
BOOLEAN in_list(LIST, WORD, int (*func)());
void back_list(LIST, WORD);

void *__allocate(int, STRING, int);
void __deallocate(void*, STRING, int);

void ll_abort(int);

#define FORLIST(l,e)\
	{\
		LNODE _lnode = l->l_last;\
		WORD e;\
		while (_lnode) {\
			e = _lnode->l_element;
#define ENDLIST\
			_lnode = _lnode->l_prev;\
		}\
	}

#endif
