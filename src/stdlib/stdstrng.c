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
/*=============================================================
 * stdstrng.c -- Standard string routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.2 - 31 Dec 94
 *===========================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "arch.h" /* vsnprintf */
#include "mystring.h"
#include "mychar.h"


/*********************************************
 * external/imported variables
 *********************************************/
extern BOOLEAN opt_mychar;

/*********************************************
 * local variables
 *********************************************/

static int hardfail = 0; /* developer mode to help find bad sign-extensions */

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==================================
 * chartype -- Return character type
 *================================*/
INT
chartype (INT c)
{
	if (c<0) {
	/* This most likely means someone assigned a char to an INT
	which is very bad -- it gets sign extended! -- it must always
	be first cast to a uchar (unsigned char), eg
	INT a = (uchar)*p;
	We can't pass this to O/S isx functions, because some of them
	are table driven, and we certainly don't want to give them an
	offset of several billion negative.
	*/
	/* TODO: This will have to be removed if we start passing wchars
	through here, which I think we need to do to support internal
	use of Unicode -- Perry, 2002-11-05 */
	
		return c;
	}
	if (iswhite(c))
		return WHITE;
	if (isletter(c))
		return LETTER;
	if (isnumch(c))
		return DIGIT;
	return c;
}
/*=================================
 * isnumch -- Check if character is a digit
 * Note: input character is passed to isdigit, so it
 *  should be in unsigned char range.
 * TODO: Fix for Unicode
 *===============================*/
BOOLEAN
isnumch (INT c)
{
	if (c<0 || c>0x7F) return FALSE;
	/* Now we know it is ASCII range */
#ifndef OS_NOCTYPE
	return (isdigit(c));
#else
	return (c >= '0' && c <= '9');
#endif
}
/*=================================
 * iswhite -- Check for white space
 * Note: input character is passed to isspace, so it
 *  should be in unsigned char range.
 * TODO: Fix for Unicode
 *===============================*/
BOOLEAN
iswhite (INT c)
{
	if (c<0 || c>0x7F) return FALSE;
	/* Now we know it is ASCII range */
#ifndef OS_NOCTYPE
	return (isspace(c));
#else
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
#endif
}
/*=================================
 * islinebreak -- Check for linebreak
 * TODO: Fix for Unicode
 *===============================*/
BOOLEAN
islinebreak (INT c)
{
	return c == '\n' || c == '\r';
}
/*=============================
 * isletter -- Check for letter
 * TODO: Fix for Unicode
 *===========================*/
BOOLEAN
isletter (INT c)
{
	if (opt_mychar) return mych_isalpha(c);
#ifndef OS_NOCTYPE
	return isalpha(c);
#else
	return isasciiletter(c);
#endif
}
/*=============================
 * isasciiletter -- Check for English letter
 *===========================*/
BOOLEAN
isasciiletter (INT c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
/*==========================================
 * ll_toupper -- Convert letter to uppercase
 *========================================*/
INT
ll_toupper (INT c)
{
	if (opt_mychar) return mych_toupper(c);
#ifndef OS_NOCTYPE
	/* use run-time library */
	if (islower(c)) return toupper(c);
	return c;
#else
	/* use our simple ASCII English function */
	return asc_toupper(c);
#endif
}
/*==========================================
 * ll_tolower -- Convert letter to lowercase
 *========================================*/
INT
ll_tolower (INT c)
{
	if (opt_mychar) return mych_tolower(c);
#ifndef OS_NOCTYPE
	/* use run-time library */
	if (isupper(c)) return tolower(c);
	return c;
#else
	/* use our simple ASCII English function */
	return asc_tolower(c);
#endif
}
/*===============================
 * eqstr_ex -- Are two strings equal (case-sensitive)?
 *  This is just eqstr extended to handle empty or null strings
 *  Like eqstr, this is byte-by-byte comparison, no locale, no UTF-8
 *=============================*/
BOOLEAN
eqstr_ex (CNSTRING s1, CNSTRING s2)
{
	if (!s1 || !s1[0]) {
		if (!s2 || !s2[0])
			return TRUE;
		else
			return FALSE;
	} else {
		if (!s2 || !s2[0])
			return FALSE;
		else
			return eqstr(s1, s2);
	}
}
/*==================================
 * llstrncpy -- strncpy that always zero-terminates
 * handles UTF-8
 *================================*/
char *
llstrncpy (char *dest, const char *src, size_t n, int utf8)
{
	/* must have valid strings, and copying at least one byte */
	if (!dest || n<1) return dest;
	if (!src || !src[0] || n<2) {
		*dest=0;
		return dest;
	}
	strncpy(dest, src, n);
	if (dest[n-1]) {
		chopstr_utf8(dest, n-1, utf8);
	}
	return dest;
}
/*==================================
 * llvsnprintf -- vsnprintf which backs up to last whole character
 * handles UTF-8
 *================================*/
static INT
llvsnprintf (char *dest, size_t len, int utf8, const char * fmt, va_list args)
{
	INT rtn;
	rtn = vsnprintf(dest, len, fmt, args);
	if (rtn >= (int)(len-1) || rtn == -1) {
		/* overflowed -- back up to last character that fits */
		INT width=0;
		STRING prev;
		dest[len-1] = 0; /* ensure zero-termination */
		prev = find_prev_char(&dest[len-1], &width, dest, utf8);
		prev[width]=0;
	}
	return rtn;
}
/*==================================
 * llstrncpyf -- snprintf replacement
 * handles UTF-8
 * Created: 2002/06/16, Perry Rapp
 *================================*/
char *
llstrncpyf (char *dest, size_t n, int utf8, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	llstrncpyvf(dest, n, utf8, fmt, args);
	va_end(args);
	return dest;
}
/*==================================
 * llstrncpyvf -- vsnprintf replacement
 * handles UTF-8
 * Created: 2002/06/16, Perry Rapp
 *================================*/
char *
llstrncpyvf (char *dest, size_t n, int utf8, const char * fmt, va_list args)
{
	if (n<1) return dest;
	dest[0] = 0;
	if (n<2) return dest;
	llvsnprintf(dest, n, utf8, fmt, args);
	return dest;
}
/*==================================
 * ll_atoi -- Wrapper for atoi which handles NULL input
 * Created: 2002/10/19, Perry Rapp
 *================================*/
INT
ll_atoi (CNSTRING str, INT defval)
{
	return str ? atoi(str) : defval;
}
/*==========================================================
 * stdstring_hardfail -- Set programmer debugging mode
 *  to coredump if any wrong characters passed to character
 *  classification routines
 * Created: 2002/01/24 (Perry Rapp)
 *========================================================*/
void
stdstring_hardfail (void)
{
	hardfail = 1;
}
/*====================================================
 * make8char -- coerce input to 8-bit character range
 * This is to catch any errors passing signed characters
 *  as signed integers (which gets them sign-extended to the
 *  negative two billion range!)
 * Created: 2002/01/24 (Perry Rapp)
 *==================================================*/
int
make8char (int c)
{
	if (c<0 || c>255) {
		if (hardfail) {
			ASSERT(0);
		}
		c = 0;
	}
	return c;
}

