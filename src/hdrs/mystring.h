/* String routines that know about ISO Latin 1 8 bit letters.
 * To be used with LifeLines genealogy program.
 * Copyright(c) 1996 Hannu Väisänen; all rights reserved.
*/

#ifndef MY_STRING_H
#define MY_STRING_H

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif


int asc_tolower(const int c);
int asc_toupper(const int c);
int fi_chrcmp(const int s1, const int s2);
int my_strcmp(const char *s1, const char *s2, const int cmp_table[]);
int my_strncmp(const char *s1,
                const char *s2,
                const int n,
                const int cmp_table[]);

extern const int my_ISO_Latin1_Finnish[];


/* Change this if you want to change the sort order. */
#define MY_SORT_TABLE my_ISO_Latin1_Finnish

#define MY_STRCMP(s1,s2) \
	my_strcmp((s1), (s2), MY_SORT_TABLE)
#define MY_STRNCMP(s1,s2,n) \
	my_strncmp((s1), (s2), (n), MY_SORT_TABLE)


#endif
