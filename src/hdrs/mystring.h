/* String routines that know about ISO Latin 1 8 bit letters.
 * To be used with LifeLines genealogy program.
 * Copyright(c) 1996 Hannu Väisänen; all rights reserved.
*/

#ifndef MY_STRING_H
#define MY_STRING_H

int my_isalpha (const int c);
int my_iscntrl (const int c);
int my_islower (const int c);
int my_isprint (const int c);
int my_isupper (const int c);

int my_tolower (const int c);
int my_toupper (const int c);

int my_strcmp  (const unsigned char *s1,
                const unsigned char *s2,
                const int cmp_table[]);
int my_strncmp (const unsigned char *s1,
                const unsigned char *s2,
                const int n,
                const int cmp_table[]);

int my_chrcmp (const int s1, const int s2);

extern const int my_ISO_Latin1_Finnish[];

/* Change this if you want to change the sort order. */
#define MY_SORT_TABLE my_ISO_Latin1_Finnish

#define MY_STRCMP(s1,s2) \
	my_strcmp((const unsigned char *)(s1), \
		  (const unsigned char *)(s2),MY_SORT_TABLE)
#define MY_STRNCMP(s1,s2,n) \
	my_strncmp((const unsigned char *)(s1), \
		   (const unsigned char *)(s2),(n),MY_SORT_TABLE)

void llstrcatn(char ** pdest, const char * src, int * len);
char *llstrncpy(char *dest, const char *src, size_t n);

#endif
