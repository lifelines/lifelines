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
/* String routines that know about ISO Latin 1 8 bit letters.
 * To be used with LifeLines genealogy program.
 * Copyright(c) 1996 Hannu Väisänen; all rights reserved.
*/

#include "llstdlib.h"
#include "mystring.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif



extern int opt_finnish;	/* use standard strcmp, strncmp if this is FALSE */



/* Finnish sorting order.
 *
 * BUGS Æ æ  is sorted as 'a', it should be sorted as 'ae'.
 *      ß    is sorted as 's', it should be sorted as 'ss'.
 *      Þ þ  is sorted as 't', it should be sorted as 'th'.
 *
 * Since I don't have those letters in my data, I don't care. (-:
 */
const int my_ISO_Latin1_Finnish[] = {
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,
     40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71, /* @ A B C D E F G */
     72,  73,  74,  75,  76,  77,  78,  79, /* H I J K L M N O */
     80,  81,  82,  83,  84,  85,  86,  86, /* P Q R S T U V W */
     88,  89,  90,  91,  92,  93,  94,  95, /* X Y Z [ \ ] */
     96,  65,  66,  67,  68,  69,  70,  71, /* @ a b c d e f g */
     72,  73,  74,  75,  76,  77,  78,  79, /* h i j k l m n o */
     80,  81,  82,  83,  84,  85,  86,  86, /* p q r s t u v w */
     88,  89,  90,  91,  92,  93, 126, 127, /* x y z { | } */
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
     65,  65,  65,  65,  92,  91,  65,  67, /* À Á Â Ã Ä Å Æ Ç */
     69,  69,  69,  69,  73,  73,  73,  73, /* È É Ê Ë Ë Í Î Ï */
     68,  78,  79,  79,  79,  93,  93, 215, /* Ð Ñ Ò Ó Ô Õ Ö × */
     93,  85,  85,  85,  89,  89,  84,  83, /* Ø Ù Û Û Ü Ý Þ ß */
     65,  65,  65,  65,  92,  91,  65,  67, /* à á â ã ä å æ ç */
     69,  69,  69,  69,  73,  73,  73,  73, /* è é ê ë ì í î ï */
     68,  78,  79,  79,  79,  93,  93, 215, /* ð ñ ò ó ô õ ö ÷ */
     93,  85,  85,  85,  89,  89,  84,  89, /* ø ù ú û ü ý þ ÿ */
    };


/*====================================
 * asc_tolower -- Return lowercase version (or input)
 * Lowercases only ASCII English letters
 * (Used for traditional soundex)
 *==================================*/
int
asc_tolower (const int c)
{
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 'a';
	else
		return c;
}
/*====================================
 * asc_toupper -- Return uppercase version (or input)
 * Lowercases only ASCII English letters
 * (Used for traditional soundex)
 *==================================*/
int
asc_toupper (const int c)
{
	if (c >= 'a' && c <= 'z')
		return c + 'A' - 'a';
	else
		return c;
}
/*====================================
 * fi_chrcmp -- Compare two characters, Finnish case-insensitive
 * Assumes Latin1 character set
 *==================================*/
int
fi_chrcmp (const int sa1, const int sa2)
{
	int s1 = make8char(sa1);
	int s2 = make8char(sa2);
	return (my_ISO_Latin1_Finnish[s1] - my_ISO_Latin1_Finnish[s2]);
}


int
my_strcmp (const char *s1, const char *s2, const int cmp_table[])
{
  int i;
  /* to make things easier, work with pointers that give uchars */
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  if(!opt_finnish) return(strcmp(s1,s2));

  for (i=0; p1[i] && p2[i]; i++) {
#if 0
    fprintf (stdout, "%d %c %c %d\n", i, p1[i], p2[i],
             (cmp_table[p1[i]] != cmp_table[p2[i]]));
#endif
    if (cmp_table[p1[i]] != cmp_table[p2[i]]) {
      break;
    }
  }
  return (cmp_table[p1[i]] - cmp_table[p2[i]]);
}

int
my_strncmp (const char *s1,
            const char *s2,
            const int n,
            const int cmp_table[])
{
  int i;
  /* to make things easier, work with pointers that give uchars */
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  if(!opt_finnish) return(strncmp(s1,s2,n));

  for (i=0; i<n && p1[i] && p2[i]; i++) {
    if (cmp_table[p1[i]] != cmp_table[p2[i]]) {
      break;
    }
  }
  if (i == n) return 0;
  return cmp_table[p1[i]] - cmp_table[p2[i]];
}


/* Change this to 1 if you want to test mystring and */
/* compile the progam, e.g. 'cc -I../hdrs mystring.c -o m'.  */
#if 0
#include "sys_inc.h"

int
cmp (const void *s,
     const void *t)
{
  const unsigned char **s1 = (const unsigned char **)s;
  const unsigned char **s2 = (const unsigned char **)t;
/*  fprintf (stdout, "cmp: '%s' '%s'\n", *s1, *s2); */
  return my_strcmp (*s1, *s2, my_ISO_Latin1_Finnish);
}

enum {NCHARS = 256};
int opt_finnish = 1;

int main()
{
#define NN 10
  unsigned char *test[] = {
    "Väisänen",
    "Wäisänen",
    "Wegelius",
    "Varis   ",
    "Waris   ",
    "Voionmaa",
    "Rissanen",
    "Häkkilä ",
    "Hakkila ",
    "Aaltonen"
  };

  const int *t = my_ISO_Latin1_Finnish;
  int i;
  int mini = NCHARS;
  int maxi = 0;
  for (i=0; i<NCHARS; i++) {   /* Min and max code for letters. */
    if (lat1_isalpha(i)) {
      if (mini > i) mini = i;
      if (maxi < i) maxi = i;
    }
  }
  fprintf (stdout, "Finnish sorting order:\n");
  for (i=mini; i<=maxi; i++) {
    int j, flag = 0;
    for (j=0; j<NCHARS; j++) {
      if (lat1_isalpha(j) && t[j] == i) {
        fprintf (stdout, "%c ", j);  
        flag = 1;
      }
    }
    if (flag) fprintf (stdout, "\n");
  }
/***
  fprintf (stdout, "size %d %d\n",
           sizeof(test)/sizeof(test[0]), sizeof(test[0]));
***/
  llqsort (test, sizeof(test)/sizeof(test[0]), sizeof(test[0]), cmp);

  fprintf (stdout, "Sort test.\n");
  for (i=0; i<NN; i++) {
    fprintf (stdout, "%d %s\n", i, test[i]);
  }

  fprintf (stdout, "Uppercase: ");
  for (i=0; i<NCHARS; i++) {
    if (lat1_isupper(i)) fprintf (stdout, "%c", i);
  }
  fprintf (stdout, "\nLowercase: ");
  for (i=0; i<NCHARS; i++) {
    if (lat1_islower(i) && (char)i != 'ß' && (char)i != 'ÿ') {
      fprintf (stdout, "%c", i);
    }
  }
  fprintf (stdout,
    "\nNote that ß and ÿ does not have an uppercase form in ISO Latin 1.\n");
  return 0;
}
#endif
