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
 * valtable.c -- Handle value tables in LifeLines
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 12 Sep 94    3.0.2 - 16 Oct 94
 *===========================================================*/

#include "sys_inc.h"
#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"

/*=====================================================
 * init_valtab_from_rec -- Init value table from record
 *===================================================*/
BOOLEAN init_valtab_from_rec (key, tab, sep, pmsg)
STRING key;	/* record key */
TABLE tab;	/* hash table */
INT sep;	/* separator char */
STRING *pmsg;	/* error message */
{
	INT len;
	STRING rec;
	BOOLEAN rc;
	if (!tab) return FALSE;
	if (!(rec = retrieve_record(key, &len))) return FALSE;
	rc = init_valtab_from_string(rec, tab, sep, pmsg);
	stdfree(rec);
	return rc;
}
/*====================================================
 * init_valtab_from_file -- Init value table from file
 *==================================================*/
BOOLEAN init_valtab_from_file (fname, tab, sep, pmsg)
STRING fname;	/* file with value table */
TABLE tab;	/* hash table for values */
INT sep;	/* separator char */
STRING *pmsg;	/* error message */
{
	FILE *fp;
	struct stat buf;
	STRING str;
	BOOLEAN rc;
	if ((fp = fopen(fname, LLREADTEXT)) == NULL) return TRUE;
	ASSERT(fstat(fileno(fp), &buf) == 0);
	if (buf.st_size == 0) {
		fclose(fp);
		return TRUE;
	}
	str = (STRING) stdalloc(buf.st_size+1);
	str[buf.st_size] = 0;
	ASSERT(fread(str, buf.st_size, 1, fp) == 1);
	fclose(fp);
 	rc = init_valtab_from_string(str, tab, sep, pmsg);
	stdfree(str);
	return rc;
}
/*========================================================
 * init_valtab_from_string -- Init value table from string
 *======================================================*/
BOOLEAN init_valtab_from_string (str, tab, sep, pmsg)
STRING str;	/* string with value table */
TABLE tab;	/* hash table for values */
INT sep;	/* separator char */
STRING *pmsg;	/* error message */
{
	STRING tag, val, q, p = str;
	INT c;
	static char errmsg[80];
	INT n = 1;
	*pmsg = NULL;
	while (TRUE) {
		q = tag = p;
		while (iswhite(c = *p++)) {
			if (c == '\n') n++;
		}
		--p;
		if (c == 0) return TRUE;
		while ((c = *q++ = *p++) && c != sep && c != '\n')
			;
		if (c == 0 || c =='\n') {
			sprintf(errmsg, "line %d: no value", n);
			return FALSE;
		}
		*(q - 1) = 0;
		striptrail(tag);
		q = val = p;
		while ((*q++ = c = *p++)) {
			if (c == '\n') break;
			if (c == '\\') {
				if ((c = *p++) == EOF) break;
				*(q - 1) = c;
			}
		}
		*(q - 1) = 0;
#ifdef DEBUG
	llwprintf("val, tag = %s %s\n", val, tag);
#endif
		insert_table(tab, strsave(tag), strsave(val));
		if (c == 0) break;
	}
	return TRUE;
}
#if 0
BOOLEAN lex_valtab (tab, sep, nextc, pmsg)
TABLE *ptab;	/* hash table for values */
INT sep;	/* separator char */
STRING ermsg;	/* error message */

	while (TRUE) {
		p = tag;
		while ((c = *p++ = getc(fp)) != EOF && c != sep && c != '\n')
			;
		if (c == EOF)  {
			fclose(fp);
			return rc ? (p - 1 == tag) : FALSE;
		}
		if (c == '\n') {
			rc = FALSE;
			continue;
		}
		*(p - 1) = 0;
		p = val;
		while ((c = *p++ = getc(fp)) != EOF) {
			if (c == '\n') break;
			if (c == '\\') {
				if ((c = getc(fp)) == EOF) break;
				*(p - 1) = c;
			}
		}
		*(p - 1) = 0;
		insert_table(tab, strsave(tag), strsave(val));
		if (c == EOF) break;
	}
#endif
