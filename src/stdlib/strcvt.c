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
 * strcvt.c -- string conversion functions
 *===========================================================*/

#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#include "zstr.h"
#include "icvt.h"
#include "stdlibi.h"

static const char * get_wchar_codeset_name(void);

/*===================================================
 * get_wchar_codeset_name -- name of wchar_t codeset
 * (handles annoyance that MS-Windows wchar_t is small)
 *=================================================*/
static const char *
get_wchar_codeset_name (void)
{
#ifdef _WIN32
	/* MS-Windows can't handle UCS-4; could we use UTF-16 ? */
	return "UCS-2-INTERNAL";
#else
	return "UCS-4-INTERNAL";
#endif
}
/*===================================================
 * makewide -- convert internal to wchar_t *
 * handling annoyance that MS-Windows wchar_t is small
 * Either returns 0 if fails, or a zstring which actually
 * contains wchar_t characters.
 *=================================================*/
ZSTR
makewide (const char *str)
{
	ZSTR zstr=0;
	if (int_codeset && int_codeset[0]) {
		CNSTRING dest = get_wchar_codeset_name();
		zstr = zs_new();
		/* dest = "wchar_t" doesn't work--Perry, 2002-11-20 */
		if (!iconv_trans(int_codeset, dest, str, zstr, "?")) {
			zs_free(&zstr);
		}
	}
	return zstr;
}
/*===================================================
 * makeznarrow -- Inverse of makewide
 *  Created: 2002-12-15 (Perry Rapp)
 *=================================================*/
ZSTR
makeznarrow (ZSTR zwstr)
{
	ZSTR zout=zs_new();
	CNSTRING src = get_wchar_codeset_name();
	if (!iconv_trans(src, int_codeset, zs_str(zwstr), zout, "?"))
		zs_free(&zout);
	return zout;
}
/*=========================================
 * isnumeric -- Check string for all digits
 * TODO: convert to Unicode -- but must find where we make
 * numeric equivalent & convert it as well
 *=======================================*/
BOOLEAN
isnumeric (STRING str)
{
	INT c;
	if (!str) return FALSE;
	while ((c = (uchar)*str++)) {
#ifndef OS_NOCTYPE
		if (chartype(c) != DIGIT) return FALSE;
#else
		if (!isdigit(c)) return FALSE;
#endif
	}
	return TRUE;
}
/*======================================
 * lower -- Convert string to lower case
 *  returns static buffer
 *====================================*/
STRING
lower (STRING str)
{
	static char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c, i=0;
#ifdef HAVE_TOWLOWER
	ZSTR zstr=makewide(str);
	if (zstr) {
		ZSTR zout=0;
		wchar_t * wp;
		for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
			*wp = towlower(*wp);
		}
		zout = makeznarrow(zstr);
		llstrsets(scratch, sizeof(scratch), uu8, zs_str(zout));
		zs_free(&zstr);
		zs_free(&zout);
#else
	if (0) {
#endif
	} else {
		while ((c = (uchar)*str++) && (++i < MAXLINELEN+1))
			*p++ = ll_tolower(c);
		*p = '\0';
	}
	return scratch;
}
/*==========================================
 * ll_toupperz -- Convert string to uppercase
 *========================================*/
ZSTR
ll_toupperz (STRING s, INT utf8)
{
	/* TODO: preprocess for the German & Greek special cases */
	ZSTR zstr=0;
#ifdef HAVE_TOWUPPER
	if (utf8) {
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			ZSTR zout=0;
			wchar_t * wp;
			/* convert to uppercase in place */
			for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
				*wp = towupper(*wp);
			}
			zout = makeznarrow(zstr);
			zs_free(&zstr);
			zstr = zout;
		}
	}
#endif

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			zs_appc(zstr, (unsigned char)ll_toupper(*s));
		}
	}
	return zstr;
}
/*======================================
 * upperascii_s -- Convert string to uppercase
 * This only handle ASCII letters
 *  returns static buffer
 * (It is a fast, cheap solution appropriate
 *  for GEDCOM keyword parsing.)
 *====================================*/
STRING
upperascii_s (STRING str)
{
	static char scratch[MAXLINELEN+1];
	STRING p = scratch;
	INT c, i=0;
	while ((c = (uchar)*str++) && (++i < MAXLINELEN+1)) {
		if (c>='a' && c<='z')
			c += 'A' - 'a';
		*p++ = (unsigned int)c;
	}
	*p = '\0';
	return scratch;
}
/*================================
 * capitalize -- Capitalize string
 *  returns static buffer (borrowed from lower)
 *  TODO: convert to Unicode
 *==============================*/
STRING
capitalize (STRING str)
{
	STRING p = lower(str);
	*p = ll_toupper((uchar)*p);
	return p;
}
/*================================
 * titlecase -- Titlecase string
 * Created: 2001/12/30 (Perry Rapp)
 *  returns static buffer (borrowed from lower)
 *  TODO: convert to Unicode
 *==============================*/
STRING
titlecase (STRING str)
{
	/* % sequences aren't a problem, as % isn't lower */
	STRING p = lower(str), buf=p;
	if (!p[0]) return p;
	while (1) {
		/* capitalize first letter of word */
		*p = ll_toupper((uchar)*p);
		/* skip to end of word */
		while (*p && !iswhite((uchar)*p))
			++p;
		if (!*p) return buf;
		/* skip to start of next word */
		while (*p && iswhite((uchar)*p))
			++p;
		if (!*p) return buf;
	}
}
