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
static ZSTR (*upperfunc)(CNSTRING) = 0;
static ZSTR (*lowerfunc)(CNSTRING) = 0;

/*===================================================
 * get_wchar_codeset_name -- name of wchar_t codeset
 * (handles annoyance that MS-Windows wchar_t is small)
 *=================================================*/
static const char *
get_wchar_codeset_name (void)
{
	if (sizeof(wchar_t)==2) {
	/* MS-Windows can't handle UCS-4; could we use UTF-16 ? */
		return "UCS-2-INTERNAL";
	} else if (sizeof(wchar_t)==4) {
		return "UCS-4-INTERNAL";
	} else {
		ASSERT(0);
	}
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
		if (!iconv_trans(int_codeset, dest, str, zstr, '?')) {
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
	if (!iconv_trans(src, int_codeset, zs_str(zwstr), zout, '?'))
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
 * iswletter -- is widechar a letter ?
 *====================================*/
BOOLEAN
iswletter (wchar_t wch)
{
#ifdef HAVE_ISWALPHA
	return iswalpha(wch);
#else
	unsigned int n = wch;
	return (n>='a' && n<='z') || (n>='A' && n<='Z');
#endif
}
#ifdef HAVE_TOWLOWER
/*======================================
 * wz_makelower -- widechar lowercasing
 * Input/output holds wchar_t characters
 *====================================*/
static void
wz_makelower (ZSTR zstr)
{
	wchar_t * wp;
	for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
		if (*wp == 0x3A3) {
			/* Greek capital sigma */
			if (!iswletter(wp[1]))
				*wp = 0x3C2; /* final lower sigma */
			else
				*wp = 0x3C3; /* medial lower sigma */
		} else {
			*wp = towlower(*wp);
		}
	}
}
#endif
#ifdef HAVE_TOWUPPER
/*======================================
 * wz_makeupper -- widechar uppercasing
 * Input/output holds wchar_t characters
 *====================================*/
static void
wz_makeupper (ZSTR zstr)
{
	/* TODO: preprocess for the German special case */
	wchar_t * wp;
	for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
		*wp = towupper(*wp);
	}
}
#endif
/*======================================
 * wz_makenarrow -- convert widechar to UTF-8
 * Done inplace
 *====================================*/
static void
wz_makenarrow(ZSTR zstr)
{
	ZSTR zout = makeznarrow(zstr);
	zs_move(zstr, &zout);
}
/*======================================
 * ll_tolowerz -- Return lowercase version of string
 *====================================*/
ZSTR
ll_tolowerz (CNSTRING s, INT utf8)
{
	ZSTR zstr=0;
	if (utf8) {
		if (lowerfunc)
			return (*lowerfunc)(s);
#ifdef HAVE_TOWLOWER
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			wz_makelower(zstr);
			wz_makenarrow(zstr);
		}
#endif
	}

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			uchar ch = (uchar)*s;
			/* uppercase unless UTF-8 multibyte code */
			if (!utf8 || ch < 0x7f)
				ch = ll_tolower(ch);
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}
/*==========================================
 * set_utf8_casing -- Set plugin functions for UTF-8 casing
 *========================================*/
void
set_utf8_casing (ZSTR (*ufnc)(CNSTRING), ZSTR (*lfnc)(CNSTRING))
{
	upperfunc = ufnc;
	lowerfunc = lfnc;
}
/*==========================================
 * ll_toupperz -- Return uppercase version of string
 *========================================*/
ZSTR
ll_toupperz (CNSTRING s, INT utf8)
{
	ZSTR zstr=0;
	if (utf8) {
		if (upperfunc)
			return (*upperfunc)(s);
#ifdef HAVE_TOWUPPER
		/* This should not normally be used.
		There should be a plugin (upperfunc) */
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			wz_makeupper(zstr);
			wz_makenarrow(zstr);
		}
#endif
	}

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			uchar ch = (uchar)*s;
			/* uppercase unless UTF-8 multibyte code */
			if (!utf8 || ch < 0x7f)
				ch = ll_toupper(ch);
			zs_appc(zstr, ch);
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
 * ll_tocapitalizedz -- Returned capitalized version of string
 *==============================*/
ZSTR
ll_tocapitalizedz (STRING s, INT utf8)
{
	ZSTR zstr=0;
#if defined(HAVE_TOWLOWER) && defined(HAVE_TOWUPPER)
	if (utf8) {
		zstr = makewide(s);
		if (zstr) {
			wchar_t * wp;
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			/* capitalize first letter */
			wp = (wchar_t *)zs_str(zstr);
			*wp = towupper(*wp);
			wz_makenarrow(zstr);
		}
	}
#endif

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			uchar ch = (uchar)*s;
			if (!zs_len(zstr)) {
				/* uppercase unless UTF-8 multibyte code */
				if (!utf8 || ch < 0x7f)
					ch = ll_toupper(ch);
			}
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}
/*================================
 * ll_totitlecasez -- Return titlecased version of string
 *==============================*/
ZSTR
ll_totitlecasez (STRING s, INT utf8)
{
	ZSTR zstr=0;
#if defined(HAVE_TOWLOWER) && defined(HAVE_TOWUPPER) && defined(HAVE_ISWSPACE)
	if (utf8) {
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			BOOLEAN inword = FALSE;
			wchar_t * wp;
			for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
				if (iswspace(*wp)) {
					inword = FALSE;
				} else {
					if (!inword) {
						/* first letter of word */
						*wp = towupper(*wp);
						inword = TRUE;
					}
				}
			}
			wz_makenarrow(zstr);
		}
	}
#endif

	if (!zstr) {
		BOOLEAN inword = FALSE;
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			uchar ch = (uchar)*s;
			if (iswhite((uchar)*s)) {
				inword = FALSE;
			} else if (!inword) {
				/* first letter of word */
				inword = TRUE;
				/* uppercase unless UTF-8 multibyte code */
				if (!utf8 || ch < 0x7f)
					ch = ll_toupper(*s);
			}
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}
