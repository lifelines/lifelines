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
 * misc.c -- Various useful, miscellaneous routines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 12 Dec 94
 *   3.0.3 - 06 Aug 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"

/*========================================
 * addat -- Add @'s to both ends of string
 *======================================*/
STRING addat (str)
STRING str;
{
	STRING p;
	static unsigned char buffer[3][20];
	static INT dex = 0;
	if (++dex > 2) dex = 0;
	p = buffer[dex];
	sprintf(p, "@%s@", str);
	return p;
}
/*=============================================
 * rmvat -- Remove @'s from both ends of string
 *===========================================*/
STRING rmvat (str)
STRING str;
{
	STRING p;
	static unsigned char buffer[10][20];
	static INT dex = 0;
	ASSERT(str);
	if (++dex > 9) dex = 0;
	p = buffer[dex];
	strcpy(p, &str[1]);
	p[strlen(p)-1] = 0;
	return p;
}
/*==============================================
 * find_tag -- Search node list for specific tag
 *============================================*/
NODE find_tag (node, str)
NODE node;  STRING str;
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
INT val_to_sex (node)
NODE node;
{
	if (!node || !nval(node)) return SEX_UNKNOWN;
	if (eqstr("M", nval(node))) return SEX_MALE;
	if (eqstr("F", nval(node))) return SEX_FEMALE;
	return SEX_UNKNOWN;
}
/*====================================================
 * full_value -- Return value of node, with CONT lines
 *==================================================*/
STRING full_value (node)
NODE node;
{
	NODE cont;
	INT len = 0;
	STRING p, q, str;
	if (!node) return NULL;
	if (p = nval(node)) len += strlen(p) + 1;
	cont = nchild(node);
	while (cont && eqstr("CONT", ntag(cont))) {
		if (p = nval(cont))
			len += strlen(p) + 1;
		else
			len++;
		cont = nsibling(cont);
	}
	if (len == 0) return NULL;
/*wprintf("full_value: len = %d\n", len);/*DEBUG*/
	str = p = (STRING) stdalloc(len + 1);
	if (q = nval(node)) {
		sprintf(p, "%s\n", q);
		p += strlen(p);
	}
	cont = nchild(node);
	while (cont && eqstr("CONT", ntag(cont))) {
		if (q = nval(cont))
			sprintf(p, "%s\n", q);
		else
			sprintf(p, "\n");
		p += strlen(p);
		cont = nsibling(cont);
	}
	*(p - 1) = 0;
/*wprintf("full_value: str = %s\n", str);/*DEBUG*/
	return str;
}
