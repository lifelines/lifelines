/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * soundex.c -- soundex routines for name indexing
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "mystring.h" /* fi_chrcmp */
#include "zstr.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN opt_finnish;

/*********************************************
 * local function prototypes
 *********************************************/

static INT trad_sxcodeof(int);

/*********************************************
 * local variables
 *********************************************/

static INT oldsx = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*========================================
 * trad_soundex -- Return name's SOUNDEX code.
 *  returns static buffer
 *======================================*/
CNSTRING
trad_soundex (CNSTRING surname)
{
	static char scratch[6];
	CNSTRING p = surname;
	STRING q = scratch;
	INT c, i, j;
	if (!surname || !surname[0] || eqstr(surname, "____"))
		return (STRING) "Z999";
	/* always copy first letter directly */
	*q++ = ll_toupper((uchar)*p++);
	i = 1;
	oldsx = 0;
	while (*p && (c = ll_toupper((uchar)*p++)) && i < 4) {
		if ((j = trad_sxcodeof(c)) == 0) continue;
		*q++ = j;
		i++;
	}
	while (i < 4) {
		*q++ = '0';
		i++;
	}
	*q = 0;
	return scratch;
}
/*========================================
 * trad_sxcodeof -- Return letter's SOUNDEX code.
 *  letter:  should be capitalized letter
 * returns soundex code, or 0 if not coded
 * Also returns 0 if same as last call (uses static oldsx variable).
 * Note that Finnish version uses a different SOUNDEX
 * scheme here, making databases (name indices)
 * not portable between Finnish & normal LifeLines.
 *======================================*/
static INT
trad_sxcodeof (int letter)
{
	int newsx = 0;

	if(opt_finnish) {
	/* Finnish Language */
		switch (letter) {
		case 'B': case 'P': case 'F': case 'V': case 'W':
			newsx = '1'; break;
		case 'C': case 'S': case 'K': case 'G': case '\337':
		case 'J': case 'Q': case 'X': case 'Z': case '\307':
			newsx = '2'; break;
		case 'D': case 'T': case '\320': case '\336':
			newsx = '3'; break;
		case 'L':
			newsx = '4'; break;
		case 'M': case 'N': case '\321':
			newsx = '5'; break;
		case 'R':
			newsx = '6'; break;
		default:	/* new stays zero */
			break;
		}
	} else {
		/* English Language (Default) */
		switch (letter) {
		case 'B': case 'P': case 'F': case 'V':
			newsx = '1'; break;
		case 'C': case 'S': case 'K': case 'G':
		case 'J': case 'Q': case 'X': case 'Z':
			newsx = '2'; break;
		case 'D': case 'T':
			newsx = '3'; break;
		case 'L':
			newsx = '4'; break;
		case 'M': case 'N':
			newsx = '5'; break;
		case 'R':
			newsx = '6'; break;
		default:	/* new stays zero */
			break;
		}
	}
  
	if (newsx == 0) {
		oldsx = 0;
		return 0;
	}
	if (newsx == oldsx) return 0;
	oldsx = newsx;
	return newsx;
}
/*========================================
 * soundex_count -- Return number of active soundex codes
 *======================================*/
INT
soundex_count (void)
{
	return 1;
}
/*========================================
 * soundex_get -- Return a soundex coding
 *  returns static buffer
 *======================================*/
CNSTRING
soundex_get (INT i, CNSTRING surname)
{
	if (i==0) return trad_soundex(surname);
	return "";
}

