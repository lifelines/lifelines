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
/*=============================================================
 * strwhite.c -- Functions to remove whitespace from strings
 *===========================================================*/

#include "llstdlib.h"

/* TODO: does Unicode introduce any new whitespace characters ? 2002.06.26, Perry */

/*================================
 * trim -- Trim string if too long
 *  returns static buffer (or NULL)
 *==============================*/
STRING
trim (STRING str, INT len)
{
	static char scratch[MAXLINELEN+1];
	if (!str || strlen(str) > MAXLINELEN) return NULL;
	if (len < 0) len = 0;
	if (len > MAXLINELEN) len = MAXLINELEN;
	strcpy(scratch, str);
	scratch[len] = '\0';
	return scratch;
}
/*=========================================
 * striptrail -- Strip trailing white space
 *  modifies argument (zeros out trailing whitespace)
 *=======================================*/
void
striptrail (STRING p)
{
	STRING q = p + strlen(p) - 1;
	while (q >= p && iswhite((uchar)*q))
		*q-- = '\0';
}
#ifdef UNUSED_CODE
/*=======================================
 * striplead -- Strip leading white space
 *  modifies argument (shifts up string towards
 *  beginning to eliminate any leading whitespace)
 * UNUSED CODE
 *=====================================*/
void
striplead (STRING p)
{
	INT i = strlen(p);
	STRING  e = p + i - 1;
	STRING b = p;
	STRING q = p;

	while (iswhite((uchar)*q) && q <= e) {
		++q;
		--i; /* keep from copying past end of p */
	}
	if (q == p) return;

	while (b <= e && --i >= 0)
		*b++ = *q++;
	*b++ = '\0';
}
#endif /* UNUSED_CODE */
/*=========================================
 * skipws -- Advance pointer over whitespace
 *=======================================*/
void
skipws (STRING * ptr)
{
	while (iswhite(*(uchar *)(*ptr)))
		++(*ptr);
}
/*=========================================
 * allwhite -- Check if string is all white
 *=======================================*/
BOOLEAN
allwhite (STRING p)
{
	while (*p)
		if (!iswhite((uchar)*p++)) return FALSE;
	return TRUE;
}
/*============================================
 * chomp -- remove any trailing carriage return/linefeed
 * Created: 2002/01/03 (Perry Rapp)
 *==========================================*/
void
chomp (STRING str)
{
	STRING p = str + strlen(str) - 1;
	while (p>=str && (*p=='\r' || *p=='\n')) {
		*p=0;
		--p;
	}
}
