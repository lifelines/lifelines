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
 * export.c -- Export GEDCOM file from LifeLines database
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 29 May 94    3.0.2 - 09 Nov 94
 *===========================================================*/

#include "sys_inc.h"
#include <time.h>
#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"
#include "lloptions.h"

#include "llinesi.h"
#include "screen.h" /* calling wfield */

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING btreepath;
extern BTREE BTR;
extern TRANTABLE tran_tables[];
extern STRING qSoutarc,qSoutfin;

/*********************************************
 * local variables
 *********************************************/

static TRANTABLE tran_gedout;
static char *mabbv[] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC",
};

static INT nindi, nfam, neven, nsour, nothr;
static FILE *fn = NULL;

static void copy_and_translate (FILE*, INT, FILE*, INT, TRANTABLE);
static BOOLEAN archive(BTREE, BLOCK);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===================================================
 * archive_in_file -- Archive database in GEDCOM file
 *=================================================*/
BOOLEAN
archive_in_file (void)
{
	char dat[30], tim[20];
	struct tm *pt;
	time_t curtime;
	STRING fname, str;
	STRING archivesdir = getoptstr("LLARCHIVES", ".");

	fn = ask_for_output_file(LLWRITETEXT, _(qSoutarc), &fname, archivesdir, ".ged");
	if (!fn) {
		msg_error("The database was not saved.");
		return FALSE; 
	}
	curtime = time(NULL);
	pt = localtime(&curtime);
	sprintf(dat, "%d %s %d", pt->tm_mday, mabbv[pt->tm_mon],
	    1900 + pt->tm_year);
	sprintf(tim, "%d:%.2d", pt->tm_hour, pt->tm_min);
	fprintf(fn, "0 HEAD\n1 SOUR LIFELINES %s\n1 DEST ANY\n"
		, get_lifelines_version(80));
	/* header date & time */
	fprintf(fn, "1 DATE %s\n2 TIME %s\n", dat, tim);
	/* header submitter entry */
	str = getoptstr("HDR_SUBM", "1 SUBM");
	fprintf(fn, "%s\n", str);
	/* header gedcom version info */
	str = getoptstr("HDR_GEDC", "1 GEDC\n2 VERS 5.5\n2 FORM LINEAGE-LINKED");
	fprintf(fn, "%s\n", str);
	/* header character set info */
	str = getoptstr("HDR_CHAR", "1 CHAR ASCII");
	fprintf(fn, "%s\n", str);
	/* finished header */
	tran_gedout = tran_tables[MINGD];
	nindi = nfam = neven = nsour = nothr = 0;
	llwprintf("Saving database in `%s' in file `%s'.", btreepath, fname);
	wfield(2, 1, "     0 Persons");
	wfield(3, 1, "     0 Families");
	wfield(4, 1, "     0 Events");
	wfield(5, 1, "     0 Sources");
	wfield(6, 1, "     0 Others");
	traverse_index_blocks(BTR, bmaster(BTR), NULL, archive);
	fprintf(fn, "0 TRLR\n");
	fclose(fn);
	wpos(7,0);
	msg_info(_(qSoutfin), btreepath, fname);
	return TRUE;
}
/*========================================================
 * archive -- Traverse function called on each btree block
 *======================================================*/
static BOOLEAN
archive (BTREE btree,
         BLOCK block)
{
	INT i, n, l;
	char scratch[100];
	STRING key;
	FILE *fo;

	sprintf(scratch, "%s/%s", bbasedir(btree), fkey2path(ixself(block)));
	ASSERT(fo = fopen(scratch, LLREADBINARY));
	n = nkeys(block);
	for (i = 0; i < n; i++) {
		key = rkey2str(rkeys(block, i));
		if (*key != 'I' && *key != 'F' && *key != 'E' &&
		    *key != 'S' && *key != 'X')
			continue;
		if (fseek(fo, (long)(offs(block, i) + BUFLEN), 0))
			FATAL();
		if ((l = lens(block, i)) > 6)	/* filter deleted records */
			copy_and_translate(fo, l, fn, *key, tran_gedout);
	}
	fclose(fo);
	return TRUE;
}
/*===================================================
 * copy_and_translate -- Copy record with translation
 *=================================================*/
static void
copy_and_translate (FILE *fo,
                    INT len,
                    FILE *fn,
                    INT c,
                    TRANTABLE tt)
{
	char in[BUFLEN];
	char scratch[10];
	char *inp;
	int remlen;
	
	inp = in;		/* location for next read */
	remlen = BUFLEN;	/* max for next read */
	while (len > 0) {
	    	if(len < remlen) remlen = len;
		ASSERT(fread(inp, remlen, 1, fo) == 1);
		len -= remlen;
		remlen = (inp + remlen) - in;	/* amount in current buffer */
		ASSERT(translate_write(tt, in, &remlen, fn, (len <= 0)));
		inp = in + remlen;		/* position for next read */
		remlen = BUFLEN - remlen;	/* max for next read */
	}
	switch (c) {
	case 'I':
		sprintf(scratch, "%6d", ++nindi);
		wfield(2, 1, scratch);
		break;
	case 'F':
		sprintf(scratch, "%6d", ++nfam);
		wfield(3, 1, scratch);
		break;
	case 'E':
		sprintf(scratch, "%6d", ++neven);
		wfield(4, 1, scratch);
		break;
	case 'S':
		sprintf(scratch, "%6d", ++nsour);
		wfield(5, 1, scratch);
		break;
	case 'X':
		sprintf(scratch, "%6d", ++nothr);
		wfield(6, 1, scratch);
		break;
	}
}
