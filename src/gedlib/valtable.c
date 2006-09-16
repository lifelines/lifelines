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
 * pre-SourceForge version information:
 *   3.0.0 - 12 Sep 94    3.0.2 - 16 Oct 94
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "gedcom.h"
#include "zstr.h"

/*=====================================================
 * init_valtab_from_rec -- Init value table from record
 *  key:   [IN]  record key (in db)
 *  tab:   [I/O] hash table (values are strings)
 *  sep:   [IN]  separator char in each line between key & value
 *  pmsg:  [OUT] error message
 * Reads record from db, parses it into key/value strings
 * and inserts them into hash table provided (tab).
 * Do not need to translate, as record & table both kept
 * in internal format.
 *===================================================*/
BOOLEAN
init_valtab_from_rec (CNSTRING key, TABLE tab, INT sep, STRING *pmsg)
{
	INT len;
	STRING rawrec, rawrec1;
	BOOLEAN rc;
	if (!tab) return FALSE;
	if (!(rawrec = retrieve_raw_record(key, &len))) return FALSE;
	rawrec1 = rawrec;
	skip_BOM(&rawrec);
	rc = init_valtab_from_string(rawrec, tab, sep, pmsg);
	stdfree(rawrec1);
	return rc;
}
/*====================================================
 * init_valtab_from_file -- Init value table from file
 *  fname:   [IN]  file holding key/values strings
 *  tab:     [I/O] hash table for key/value string pairs
 *  ttm:     [IN]  translation table to use
 *  sep:     [IN]  separator char between key & value
 *  pmsg:    [OUT] error message (set if returns FALSE)
 *==================================================*/
BOOLEAN
init_valtab_from_file (STRING fname, TABLE tab, XLAT ttm, INT sep, STRING *pmsg)
{
	FILE *fp;
	struct stat buf;
	STRING str, str1;
	BOOLEAN rc;
	INT siz;
	ZSTR zstr=0;

	if ((fp = fopen(fname, LLREADTEXT)) == NULL) return TRUE;
	ASSERT(fstat(fileno(fp), &buf) == 0);
	if (buf.st_size == 0) {
		/* TO DO - should pmsg be set here ? Perry, 2001/06/03 */
		fclose(fp);
		return TRUE;
	}
	str = (STRING) stdalloc(buf.st_size+1);
	str1 = str; /* remember it for deallocating */
	str[buf.st_size] = 0;
	siz = fread(str, 1, buf.st_size, fp);
	/* may not read full buffer on Windows due to CR/LF translation */
	ASSERT(siz == buf.st_size || feof(fp));
	fclose(fp);
	/* skip over UTF-8 BOM (byte order mark) if present */
	skip_BOM(&str);
	zstr = translate_string_to_zstring(ttm, str);
	stdfree(str1); /* done with original record - we use translated record */
 	rc = init_valtab_from_string(zs_str(zstr), tab, sep, pmsg);
	zs_free(&zstr);
	return rc;
}
/*========================================================
 * init_valtab_from_string -- Init value table from string
 *  str:     [IN]  string holding all value/values
 *  tab:     [I/O] table in which to put key/value pairs (a string table)
 *  sep:     [IN]  separator between name & value
 *  pmsg:    [OUT] error message (set if returns FALSE)
 *            pmsg points to static buffer
 * eg, "PA:Pennsylvania\nVA:Virginia"
 *  could be passed as str, with sep of :
 *  and two key/value pairs would be inserted
 *  pairs inside str are always separated by \n
 *  sep is the separator between key & value
 *======================================================*/
BOOLEAN
init_valtab_from_string (CNSTRING str, TABLE tab, INT sep, STRING *pmsg)
{
	STRING tag, val, q;
	STRING strsrc = strsave(str);
	STRING p = strsrc;
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
		if (c == 0)
			goto endinitvaltab;
		while ((c = *q++ = *p++) && c != sep && c != '\n')
			;
		if (c == 0 || c =='\n') {
			sprintf(errmsg, "line %ld: no value", n);
			*pmsg = errmsg;
			goto endinitvaltab;
		}
		*(q - 1) = 0;
		striptrail(tag);
		q = val = p;
		while ((*q++ = c = *p++)) {
			if (c == '\n') break;
			if (c == '\\') {
				if ((c = *p++) == 0) break;
					/* handle \n & \t escapes */
				if (c == 'n')
					*(q - 1) = '\n';
				else if (c == 't')
					*(q - 1) = '\t';
				else
					/* skip over backslash */
					*(q - 1) = c;
			}
		}
		*(q - 1) = 0;
		insert_table_str(tab, tag, val);
		if (c == 0) break;
	}

endinitvaltab:
	stdfree(strsrc);
	return (*pmsg == 0);
}
