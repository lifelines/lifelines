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
 * misc.c -- Various useful, miscellaneous routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 12 Dec 94
 *   3.0.3 - 06 Aug 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "zstr.h"

static INT addat_count = 0;
static INT rmvat_count = 0;

/*========================================
 * addat -- Add @'s to both ends of string
 *  returns static buffer
 *======================================*/
STRING
addat (STRING str)
{
	/*
	 * This static buffer is an array of buffers that can be used for
	 * addat'ed strings.  We need to use an array here since we may have
	 * multiple calls within a single operation and we don't want to overwrite
	 * values that haven't been copied or otherwise made permanent.
	 * Since GEDCOM 5.5 spefies that XREF values can be up to 22 chars,
	 * the static buffer needs to be at least 22+2+1=25 chars, but leave a
	 * little extra.
	 *
	 * Currently we need to support up to 3 addat() calls.
	 */
#define ADDAT_SIZE 3
	static char buffer[ADDAT_SIZE][32];
	static INT dex = 0;
	STRING p;
	addat_count++;
	dex++;
	if (dex == ADDAT_SIZE) dex = 0;
	p = buffer[dex];
	snprintf(p, sizeof(buffer[dex]), "@%s@", str);
	return p;
}
/*===========================================
 * rmvat_char -- Remove bracketing characters
 *  from around a string
 *  returns static buffer
 *==========================================*/
static STRING
rmvat_char (CNSTRING str, char c, char d)
{
	/*
	 * This static buffer is an array of buffers that can be used for
	 * addat'ed strings.  We need to use an array here since we may have
	 * multiple calls within a single operation and we don't want to
	 * overwrite values that haven't been copied or made permanent.
	 * Since GEDCOM 5.5 spefies that XREF values can be up to 22 chars,
	 * the static buffer needs to be at least 22+2+1=25 chars, but leave a
	 * little extra.
	 *
	 * Currently we need to support a large number of rmvat() calls.
	 *
	 * The current limit of 32 appears sufficient, but there may be cases
	 * where static buffer reuse causes data corruption of various types.
	 * The solutions are to a) ensure that the values returned by rmvat()
	 * are saved, or b) increase the buffer size.
	 *
	 * Refer to https://github.com/lifelines/lifelines/issues/439 for one
	 * such scenario; a single INDI with 32 ASSO tags caused the INDI key
	 * to become corrupted because the ASSO tag processing caused the rmvat
	 * buffer to wrap.
	 */
#define RMVAT_SIZE 32
	static char buffer[RMVAT_SIZE][32];
	static INT dex = 0;

	STRING p;
	int len;
	/* Watch out for bad pointers */
	if((str == NULL) || (*str == '\0')) return(NULL);
	if (str[0] != c) return NULL;
	rmvat_count++;
	dex++;
	if (dex == RMVAT_SIZE) dex = 0;
	p = buffer[dex];
	len = strlen(str+1);
	if (str[len] != d) return NULL;
	if(len > 31) len = 31;		/* 31 characters is maximum */
	else if(len > 0) len--;
	strncpy(p, &str[1], len);
	p[len] = 0;	/* overwrite trailing "@" with null */
	return p;
}
/*=============================================
 * rmvat -- Remove @'s from both ends of string
 *  returns static buffer
 *===========================================*/
STRING
rmvat (CNSTRING str)
{
	return rmvat_char(str, '@', '@');
}
/*=============================================
 * rmvbrackets -- Remove <>'s from around a string
 *  returns static buffer
 *===========================================*/
STRING
rmvbrackets (CNSTRING str)
{
	return rmvat_char(str, '<', '>');
}
/*=============================================
 * node_to_keynum -- key # of a 0 level node
 * returns 0 for failure
 *===========================================*/
INT
node_to_keynum(char ntype, NODE nod)
{
	if (!nod) return 0;
	return xrefval(ntype, nxref(nod));
}
/*=============================================
 * xrefval -- numeric value after removing @'s at both ends
 *===========================================*/
INT
xrefval (char ntype, STRING str)
{
	INT val, i, len;
	if ((str == NULL) || (*str == '\0')) return 0;
	len = strlen(str);
	if (str[0] != '@' || str[len-1] != '@') return 0;
	if (str[1] != ntype) return 0;
	val=0;
	for (i=2; i<len-1; i++) {
		if (chartype((uchar)str[i]) != DIGIT) return 0;
		if (i>31) return 0;
		val = val*10 + (str[i]-'0');
	}
	return val;
}
/*==============================================
 * find_tag -- Search node list for specific tag
 *============================================*/
NODE
find_tag (NODE node, CNSTRING str)
{
	while (node) {
		ASSERT(ntag(node));
		if (eqstr(str, ntag(node))) return node;
		node = nsibling(node);
	}
	return NULL;
}
/*=================================================
 * val_to_sex -- Convert SEX value to internal form
 *===============================================*/
INT
val_to_sex (NODE node)
{
	if (!node || !nval(node)) return SEX_UNKNOWN;
	if (eqstr("M", nval(node))) return SEX_MALE;
	if (eqstr("F", nval(node))) return SEX_FEMALE;
	return SEX_UNKNOWN;
}
/*====================================================
 * full_value -- Return value of node, with CONC & CONT lines
 * (sep is used before CONT lines, eg, "\n")
 * heap-allocated string is returned
 *==================================================*/
STRING
full_value (NODE node, STRING sep)
{
	NODE child;
	ZSTR zstr = 0;
	STRING str = 0;
	if (!node) return NULL;
	if (nval(node))
		zstr = zs_news(nval(node));
	else
		zstr = zs_new();
	for (child = nchild(node); child	; child = nsibling(child)) {
		if (nchild(child) || !ntag(child)) break;
		if (eqstr("CONC", ntag(child))) {
			if (nval(child)) {
				zs_apps(zstr, nval(child));
			}
		} else if (eqstr("CONT", ntag(child))) {
			if (sep) {
				zs_apps(zstr, sep);
			}
			if (nval(child)) {
				zs_apps(zstr, nval(child));
			}
		} else {
			break;
		}
	}
	str = strdup(zs_str(zstr));
	zs_free(&zstr);
	return str;
}
