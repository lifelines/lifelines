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

#include "standard.h"	/* for INT, STRING, LIST, VPTR, BOOLEAN */
#include "interp.h"	/* for PVALUE */

/* assert.c */
/*
Main program (lifelines, btedit, dbverify,...) must provide
an implementation of __fatal and fatalmsg, and it must not
return (eg, these all call exit()).
*/
void __fatal(STRING file, int line, STRING details);

/* dirs.c */
BOOLEAN mkalldirs(STRING);

/* double.c */
void back_list(LIST, VPTR);
LIST create_list(void);
VPTR dequeue_list(LIST);
BOOLEAN empty_list(LIST);
void enqueue_list(LIST, VPTR);
VPTR get_list_element(LIST, INT);
BOOLEAN in_list(LIST, VPTR, int (*func)(PVALUE, PVALUE));
INT length_list(LIST);
void make_list_empty(LIST);
VPTR pop_list(LIST);
void push_list(LIST, VPTR);
void remove_list(LIST, void (*func)(VPTR));
void set_list_element(LIST, INT, VPTR);
void set_list_type(LIST, INT);

/* environ.c */
#define PROGRAM_LIFELINES 1
#define PROGRAM_BTEDIT 2
STRING environ_determine_config_file(void);
STRING environ_determine_editor(INT program);
STRING environ_determine_tempfile(void);

/* lldate.c */
void get_current_lldate(LLDATE * creation);

/* llstrcmp.c */
int ll_strcmploc(char*, char*);
int ll_strncmp(char*, char*, int);
typedef BOOLEAN (*usersortfnc)(char *str1, char *str2, INT * rtn);
void set_usersort(usersortfnc fnc);

/* memalloc.c */
void *__allocate(int, STRING file, int line);
void __deallocate(void*, STRING file, int line);
void * __reallocate(void*, int size, STRING file, int line);
INT alloc_count(void);
void report_alloc_live_count(STRING str);

/* path.c */
STRING concat_path(STRING dir, STRING file);
STRING filepath(STRING name, STRING mode, STRING path, STRING ext);
FILE* fopenpath(STRING, STRING, STRING, STRING, STRING*);
BOOLEAN is_dir_sep(char c);
STRING lastpathname(STRING);

/* signals.c */
void set_signals(void);

/* stdstrng.c */
STRING strsave(STRING);
void strfree(STRING *);
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
void striptrail(STRING);
void striplead(STRING);
BOOLEAN allwhite(STRING);

#endif /* _LL_STDLIB_H */
