/*=============================================================
 * standard.h -- Define standard macros and types
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 05 May 94    3.0.2 - 23 Nov 94
 *   3.0.3 - 25 Jun 95
 *===========================================================*/

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

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)>(y)?(y):(x))

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
#define eqstr(s,t)    (!strcmp((s),(t)))
#define nestr(s,t)    (strcmp((s),(t)))

#define check_cache()   ___check_cache(__LINE__, __FILE__)

#ifndef WIN32
extern char *malloc();
#endif
extern char *__allocate();
extern __deallocate();

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

extern LIST create_list();
extern BOOLEAN empty_list();
extern push_list();
extern WORD pop_list(), get_list_element();
extern INT length_list();
extern enqueue_list();
extern WORD dequeue_list();

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
