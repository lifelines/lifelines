/*=============================================================
 * mychar_funcs.c -- character properties 
 * For systems without good libc support
 *===========================================================*/


#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */
#include "mychar.h"

static my_charset_info * current_charset = ISO_Latin1;

/*====================================
 * mych_isalpha -- Is a character alphabetic ?
 *==================================*/
int
mych_isalpha (const int c1)
{
	int c = make8char(c1);
	return current_charset[c].isup || current_charset[c].islow;
}
/*====================================
 * mych_iscntrl -- Is a character in control character range ?
 *==================================*/
int
mych_iscntrl (const int c1)
{
	int c = make8char(c1);
	return current_charset[c].iscntrl;
}
/*====================================
 * mych_islower -- Is a character lowercase ?
 *==================================*/
int
mych_islower (const int c1)
{
	int c = make8char(c1);
	return current_charset[c].islow;
}
/*====================================
 * mych_isprint -- Is a character printable ?
 *==================================*/
int
mych_isprint (const int c1)
{
	int c = make8char(c1);
	return !current_charset[c].iscntrl;
}
/*====================================
 * mych_isupper -- Is a character lowercase ?
 *==================================*/
int
mych_isupper (const int c1)
{
	int c = make8char(c1);
	return current_charset[c].isup;
}
/*====================================
 * mych_set_table -- Set which codepage
 *==================================*/
void
mych_set_table (my_charset_info * charset_table)
{
	current_charset = charset_table;
}
/*====================================
 * mych_tolower -- Return lowercase version (or input)
 *==================================*/
int
mych_tolower (const int c1)
{
	int c = make8char(c1);
	return current_charset[c].tolow;
}
/*====================================
 * mych_toupper -- Return uppercase version (or input)
 *==================================*/
int
mych_toupper (const int c1)
{
  /* BUG: ß is not converted to SS. */
  /* Note that ÿ does not have an   */
  /* uppercase form in ISO Latin 1. */
	int c = make8char(c1);
	return current_charset[c].toup;
}
