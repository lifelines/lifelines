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

#ifndef _LL_STDLIB_H
#define _LL_STDLIB_H

/*
 * standard.h:
 *
 * for INT, STRING, LIST, VPTR, BOOLEAN
 * pulls in config.h (via llnsl.h)
 * pulls in sys_inc.h (which pulls in a bunch of system includes)
 * defines macros for gettext if appropriate (eg, _(String))
 */
#include "standard.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

/* Current internal codeset (the same as that of the loaded database). */
extern BOOLEAN uu8;         /* flag set if int_codeset is UTF-8 */
extern BOOLEAN gui8;        /* flag if display output encoding is UTF-8 */
extern STRING  int_codeset;

/* appendstr.c */
void appendstr(STRING * pdest, INT * len, int utf8, CNSTRING src);
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
void __fatal(STRING file, int line, CNSTRING details);
void crashlog(STRING fmt, ...);
void crashlogn(STRING fmt, ...);


/* dirs.c */
BOOLEAN mkalldirs(STRING);

/* environ.c */
#define PROGRAM_LIFELINES 1
#define PROGRAM_BTEDIT 2
STRING environ_determine_editor(INT program);
STRING environ_determine_tempfile(void);

/* errlog.c */
void crash_setcrashlog(STRING crashlog);
void crash_setdb(STRING dbname);
void errlog_out(CNSTRING title, CNSTRING msg, CNSTRING file, int line);

/* fileops.c macros */
/* (add source location to calls) */
#define CHECKED_fclose(qfp, qfn) do_checked_fclose(qfp, qfn, __FILE__, __LINE__)
#define CHECKED_fflush(qfp, qfn) do_checked_fflush(qfp, qfn, __FILE__, __LINE__)
#define CHECKED_fseek(qfp, qoff, qwh, qfn) do_checked_fseek(qfp, qoff, qwh, qfn, qfn, __FILE__, __LINE__)
#define CHECKED_fwrite(qbuf, qsz, qct, qfp, qfn) do_checked_fwrite(qbuf, qsz, qct, qfp, qfn, __FILE__, __LINE__)

/* fileops.c functions */
int do_checked_fclose(FILE *fp, STRING filename, STRING srcfile, int srcline);
int do_checked_fflush(FILE *fp, STRING filename, STRING srcfile, int srcline);
int do_checked_fseek(FILE *fp, long offset, int whence, STRING filename, STRING srcfile, int srcline);
size_t do_checked_fwrite(const void *buf, size_t size, size_t count, FILE *fp, STRING filename, STRING srcfile, int srcline);

/* listener.c */
	/* callback for language change */
typedef void (*CALLBACK_FNC)(VPTR);
void add_listener(LIST * notifiees, CALLBACK_FNC fncptr, VPTR uparm);
void delete_listener(LIST * notifiees, CALLBACK_FNC fncptr, VPTR uparm);
void notify_listeners(LIST * notifiees);
void remove_listeners(LIST * notifiees);

/* lldate.c */
void get_current_lldate(LLDATE * creation);

/* llstrcmp.c */
int ll_strcmploc(const char*, const char*);
CNSTRING ll_what_collation(void);
int ll_strncmp(const char*, const char*, int);
typedef BOOLEAN (*usersortfnc)(const char *str1, const char *str2, INT * rtn);
void set_usersort(usersortfnc fnc);

/* memalloc.c */
void *__allocate(int, STRING file, int line);
void __deallocate(void*, STRING file, int line);
void * __reallocate(void*, INT size, STRING file, int line);
INT alloc_count(void);
void report_alloc_live_count(STRING str);
char * ngettext_null (const char *, const char *, unsigned long int);
char * dngettext_null(const char *, const char *, const char *, unsigned long int);
char * dcngettext_null(const char *, const char *, const char *, unsigned long int, int);

/* norm_charmap.c */
char *norm_charmap(char *name);

/* path.c */
STRING check_file_for_unicode(FILE * fp);
INT chop_path(CNSTRING path, STRING dirs);
void closefp(FILE **pfp);
STRING compress_path(CNSTRING path, INT len);
STRING concat_path(CNSTRING dir, CNSTRING file, INT utf8, STRING buffer, INT buflen);
STRING concat_path_alloc(CNSTRING dir, CNSTRING file);
BOOLEAN expand_special_fname_chars(STRING buffer, INT buflen, INT uu8);
STRING filepath(CNSTRING name, CNSTRING mode, CNSTRING path, CNSTRING ext, INT utf8);
FILE* fopenpath(STRING, STRING, STRING, STRING, INT utf8, STRING*);
CNSTRING get_first_path_entry(CNSTRING path);
BOOLEAN is_path(CNSTRING dir);
BOOLEAN is_dir_sep(char c);
BOOLEAN is_path_sep(char c);
STRING lastpathname(CNSTRING);
INT path_cmp(CNSTRING path1, CNSTRING path2);
BOOLEAN path_match(CNSTRING path1, CNSTRING path2);


/* signals.c */
void set_signals(void);
void ll_optional_abort(STRING);

/* sprintpic.c */
void sprintpic0(STRING buffer, INT len, INT utf8, CNSTRING pic);
BOOLEAN sprintpic1(STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1);
ZSTR zprintpic1(CNSTRING pic, CNSTRING arg1);
BOOLEAN sprintpic2(STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1
	, CNSTRING arg2);
ZSTR zprintpic2(CNSTRING pic, CNSTRING arg1, CNSTRING arg2);
BOOLEAN sprintpic3(STRING buffer, INT len, INT utf8, CNSTRING pic, CNSTRING arg1
	, CNSTRING arg2, CNSTRING arg3);
ZSTR zprintpic3(CNSTRING pic, CNSTRING arg1, CNSTRING arg2, CNSTRING arg3);

/* stdlib.c */
void init_stdlib(void);

/* stdstrng.c */
INT chartype(INT);
BOOLEAN eqstr_ex(CNSTRING s1, CNSTRING s2);
BOOLEAN isasciiletter(INT c);
BOOLEAN isletter(INT);
BOOLEAN islinebreak(INT c);
BOOLEAN isnumch(INT c);
BOOLEAN iswhite(INT);
INT ll_atoi(CNSTRING str, INT defval);
INT ll_toupper(INT);
INT ll_tolower(INT);
char *llstrncat(char *dest, const char *src, size_t n, int utf8);
char *llstrncpy(char *dest, const char *src, size_t n, int utf8);
char *llstrncpyf(char *dest, size_t n, int utf8, const char * fmt, ...);
char *llstrncpyvf(char *dest, size_t n, int utf8, const char * fmt, va_list args);
char *llstrncpyvf(char *dest, size_t n, int utf8, const char * fmt, va_list args);
int make8char(int c);
void stdstring_hardfail(void);

/* stralloc.c */
STRING allocsubbytes(STRING s, INT start, INT num);
void free_array_strings(INT n, STRING * arr);
STRING strconcat(STRING, STRING);
void strfree(STRING *);
STRING strsave(CNSTRING);
void strupdate(STRING * str, CNSTRING value);

/* strapp.c */
char *llstrapps(char *dest, size_t limit, int utf8, const char *src);
char *llstrappc(char *dest, size_t limit, char ch);
char *llstrappf(char *dest, int limit, int utf8, const char *fmt, ...);
char *llstrappvf(char *dest, int limit, int utf8, const char *fmt, va_list args);

/* strset.c */
char *llstrsets(char *dest, size_t limit, int utf8, const char *src);
char *llstrsetc(char *dest, size_t limit, char ch);
char *llstrsetf(char * dest, int limit, int utf8, const char * fmt, ...);
char *llstrsetvf(char * dest, int limit, int utf8, const char * fmt, va_list args);

/* strcvt.c */
BOOLEAN isnumeric(STRING);
ZSTR ll_tocapitalizedz(STRING s, INT utf8);
ZSTR ll_tolowerz(CNSTRING s, INT utf8);
ZSTR ll_totitlecasez(STRING, INT utf8);
ZSTR ll_toupperz(CNSTRING s, INT utf8);
void set_utf8_casing(ZSTR (*ufnc)(CNSTRING), ZSTR (*lfnc)(CNSTRING));
STRING upperascii_s(STRING str);

/* strutf8.c */
void chopstr_utf8(STRING str, size_t index, BOOLEAN utf8);
void limit_width(STRING str, size_t width, BOOLEAN utf8);
STRING find_prev_char(STRING ptr, INT * width, STRING limit, int utf8);
INT next_char32(STRING * ptr, int utf8);
void skip_BOM(STRING * pstr);
size_t str8chlen(CNSTRING str);
void unicode_to_utf8(INT wch, char * utf8);
INT utf8len(char ch);

/* strwhite.c */
BOOLEAN allwhite(STRING);
void chomp(STRING);
void skipws(STRING *ptr);
void striplead(STRING);
void striptrail(STRING);
STRING trim(STRING, INT);

#endif /* _LL_STDLIB_H */
