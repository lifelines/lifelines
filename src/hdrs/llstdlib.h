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

/*
 * for INT, STRING, LIST, VPTR, BOOLEAN
 * pulls in config.h
 * pulls in sys_inc.h (which pulls in a bunch of system includes)
 * defines macros for gettext if appropriate (eg, _(String))
 */
#include "standard.h"


/* This is not the user's codeset! This is the current internal codeset. */
extern BOOLEAN uu8;
extern STRING  int_codeset;

/* appendstr.c */
void appendstr(char ** pdest, int * len, int utf8, const char * src);
void appendstrf(char ** pdest, int * len, int utf8, const char * fmt,...);
void appendstrvf(char ** pdest, int * len, int utf8, const char * fmt, va_list args);
	/* llstrcatn is a bad name, because its prototype is different from strcatn! */
#define llstrcatn(dest, src, len) appendstr(dest, len, uu8, src)

/* assert.c */
/*
Main program (lifelines, btedit, dbverify,...) must provide
an implementation of __fatal and fatalmsg, and it must not
return (eg, these all call exit()).
They must also implement crashlog, but it may do nothing, or
it may just print its printf style args to the screen. It 
should return.
*/
void __fatal(STRING file, int line, STRING details);
void crashlog(STRING fmt, ...);
void crash_setcrashlog(STRING crashlog);
void crash_setdb(STRING dbname);


/* dirs.c */
BOOLEAN mkalldirs(STRING);

/* double.c */
void back_list(LIST, VPTR);
LIST create_list(void);
VPTR dequeue_list(LIST);
BOOLEAN is_empty_list(const LIST);
void enqueue_list(LIST, VPTR);
VPTR get_list_element(LIST, INT);
INT in_list(LIST, VPTR param, BOOLEAN (*func)(VPTR param, VPTR el));
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
char * ngettext_null (const char *, const char *, unsigned long int);
char * dngettext_null(const char *, const char *, const char *, unsigned long int);
char * dcngettext_null(const char *, const char *, const char *, unsigned long int, int);

/* path.c */
STRING check_file_for_unicode(FILE * fp);
INT chop_path(STRING path, STRING dirs);
void closefp(FILE **pfp);
STRING compress_path(CNSTRING path, INT len);
STRING concat_path(CNSTRING dir, CNSTRING file, STRING buffer, INT buflen);
BOOLEAN expand_special_fname_chars(STRING buffer, INT buflen);
STRING filepath(CNSTRING name, CNSTRING mode, CNSTRING path, CNSTRING ext);
FILE* fopenpath(STRING, STRING, STRING, STRING, STRING*);
BOOLEAN is_dir_sep(char c);
BOOLEAN is_path_sep(char c);
STRING lastpathname(CNSTRING);
INT path_cmp(CNSTRING path1, CNSTRING path2);
BOOLEAN path_match(CNSTRING path1, CNSTRING path2);

/* signals.c */
void set_signals(void);
void ll_abort(STRING);

/* sprintpic.c */
void sprintpic0(STRING buffer, INT len, INT utf8, CNSTRING pic);
BOOLEAN sprintpic1(STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1);
BOOLEAN sprintpic2(STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1
	, CNSTRING arg2);
BOOLEAN sprintpic3(STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1
	, CNSTRING arg2, CNSTRING arg3);

/* stdstrng.c */
INT chartype(INT);
BOOLEAN eqstr_ex(STRING s1, STRING s2);
BOOLEAN isletter(INT);
BOOLEAN islinebreak(INT c);
BOOLEAN iswhite(INT);
INT ll_toupper(INT);
INT ll_tolower(INT);
char *llstrncat(char *dest, const char *src, size_t n, int utf8);
char *llstrncpy(char *dest, const char *src, size_t n, int utf8);
char *llstrncpyf(char *dest, size_t n, int utf8, const char * fmt, ...);
char *llstrncpyvf(char *dest, size_t n, int utf8, const char * fmt, va_list args);
char *llstrncpyvf(char *dest, size_t n, int utf8, const char * fmt, va_list args);


/* stralloc.c */
void free_array_strings(INT n, STRING * arr);
STRING strconcat(STRING, STRING);
void strfree(STRING *);
STRING strsave(CNSTRING);
void strupdate(STRING * str, CNSTRING value);

/* strapp.c */
char *llstrapp(char *dest, size_t limit, int utf8, const char *src);
char *llstrappc(char *dest, size_t limit, char ch);
char *llstrappf(char *dest, int limit, int utf8, const char *fmt, ...);
char *llstrappvf(char *dest, int limit, int utf8, const char *fmt, va_list args);

/* strcvt.c */
STRING capitalize(STRING);
BOOLEAN isnumeric(STRING);
STRING lower(STRING);
STRING titlecase(STRING);
STRING upper(STRING);

/* strutf8.c */
STRING find_prev_char(STRING ptr, INT * width, STRING limit, int utf8);
INT utf8len(char ch);

/* strwhite.c */
BOOLEAN allwhite(STRING);
void chomp(STRING);
void striplead(STRING);
void striptrail(STRING);
STRING trim(STRING, INT);

#endif /* _LL_STDLIB_H */
