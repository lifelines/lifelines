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

/*========================================
 * addat -- Add @'s to both ends of string
 *  returns static buffer
 *======================================*/
STRING
addat (STRING str)
{
	STRING p;
	static unsigned char buffer[3][20];
	static INT dex = 0;
	if (++dex > 2) dex = 0;
	p = buffer[dex];
	sprintf(p, "@%s@", str);
	return p;
}
/*===========================================
 * rmvat_char -- Remove bracketing characters
 *  from around a string
 *  returns static buffer
 *==========================================*/
static STRING
rmvat_char (STRING str, char c, char d)
{
	STRING p;
	int len;
	/* WARNING: GEDCOM 5.5 specifies that the resulting string (XREF) can be
	 * 1 to 22 characters. Allow a little extra. */
	static unsigned char buffer[32][32];	/* was [10][20] pbm 11-jun-96*/
	static INT dex = 0;
	/* Watch out for bad pointers */
	if((str == NULL) || (*str == '\0')) return(NULL);
	if (str[0] != c) return NULL;
	if (++dex > 31) dex = 0;	/* was 9 pbm 11-jun-96*/
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
rmvat (STRING str)
{
	return rmvat_char(str, '@', '@');
}
/*=============================================
 * rmvbrackets -- Remove <>'s from around a string
 *  returns static buffer
 *===========================================*/
STRING
rmvbrackets (STRING str)
{
	return rmvat_char(str, '<', '>');
}
/*=============================================
 * nod0_to_keynum -- key # of a 0 level node
 * returns 0 for failure
 *===========================================*/
INT
nod0_to_keynum(NODE nod, char ntype)
{
	if (!nod) return 0;
	return xrefval(nxref(nod), ntype);
}
/*=============================================
 * xrefval -- numeric value after removing @'s at both ends
 *===========================================*/
INT
xrefval (STRING str, char ntype)
{
	INT val, i, len;
	if ((str == NULL) || (*str == '\0')) return 0;
	len = strlen(str);
	if (str[0] != '@' || str[len-1] != '@') return 0;
	if (str[1] != ntype) return 0;
	val=0;
	for (i=2; i<len-1; i++) {
		if (chartype(str[i]) != DIGIT) return 0;
		if (i>31) return 0;
		val = val*10 + (str[i]-'0');
	}
	return val;
}
/*==============================================
 * find_tag -- Search node list for specific tag
 *============================================*/
NODE
find_tag (NODE node,
          STRING str)
{
	while (node) {
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
 * full_value -- Return value of node, with CONT lines
 *==================================================*/
STRING
full_value (NODE node)
{
	NODE cont;
	INT len = 0;
	STRING p, q, str;
	if (!node) return NULL;
	if ((p = nval(node))) len += strlen(p) + 1;
	cont = nchild(node);
	while (cont && eqstr("CONT", ntag(cont))) {
		if ((p = nval(cont)))
			len += strlen(p) + 1;
		else
			len++;
		cont = nsibling(cont);
	}
	if (len == 0) return NULL;
#ifdef DEBUG
	llwprintf("full_value: len = %d\n", len);
#endif
	str = p = (STRING) stdalloc(len + 1);
	if ((q = nval(node))) {
		sprintf(p, "%s\n", q);
		p += strlen(p);
	}
	cont = nchild(node);
	while (cont && eqstr("CONT", ntag(cont))) {
		if ((q = nval(cont)))
			sprintf(p, "%s\n", q);
		else
			sprintf(p, "\n");
		p += strlen(p);
		cont = nsibling(cont);
	}
	*(p - 1) = 0;
#ifdef DEBUG
	llwprintf("full_value: str = %s\n", str);
#endif
	return str;
}
