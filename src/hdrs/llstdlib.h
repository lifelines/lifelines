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

#ifndef _LL_STDLIB_H
#define _LL_STDLIB_H

#include "standard.h"	/* for INT, STRING, LIST, WORD, BOOLEAN */
#include "interp.h"	/* for PVALUE */

/* assert.c */
void __fatal(STRING, int);
void __assert(BOOLEAN, STRING, int);

/* double.c */
void back_list(LIST, WORD);
LIST create_list(void);
WORD dequeue_list(LIST);
BOOLEAN empty_list(LIST);
void enqueue_list(LIST, WORD);
WORD get_list_element(LIST, INT);
BOOLEAN in_list(LIST, WORD, int (*func)(PVALUE, PVALUE));
INT length_list(LIST);
void make_list_empty(LIST);
WORD pop_list(LIST);
void push_list(LIST, WORD);
void remove_list(LIST, int (*func)(WORD));
void set_list_element(LIST, INT, WORD);
void set_list_type(LIST, INT);

/* llstrcmp.c */
int ll_strcmp(char*, char*);
int ll_strncmp(char*, char*, int);

/* memalloc.c */
void *__allocate(int, STRING, int);
void __deallocate(void*, STRING, int);

/* path.c */
STRING filepath(STRING, STRING, STRING, STRING);
FILE* fopenpath(STRING, STRING, STRING, STRING, STRING*);
STRING lastpathname(STRING);

/* signals.c */
void set_signals(void);

/* stdstrng.c */
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

#endif /* _LL_STDLIB_H */
