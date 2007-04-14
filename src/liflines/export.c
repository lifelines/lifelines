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
#include "llinesi.h"
#include "feedback.h"
#include "lloptions.h"
#include "codesets.h"
#include "impfeed.h"
#include "xlat.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern BTREE BTR;

/*********************************************
 * local types
 *********************************************/

struct tag_trav_parm {
	struct tag_export_feedback * efeed;
	FILE * fp;
};

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN archive(BTREE btree, BLOCK block, void * param);
static void copy_and_translate(FILE *fo, INT len, struct tag_trav_parm * travparm, INT c, XLAT xlat);

/*********************************************
 * local variables
 *********************************************/

static XLAT xlat_gedout; /* TODO: could do away with this via param to traverse */
static char *mabbv[] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC",
};

static INT nindi, nfam, neven, nsour, nothr;


/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===================================================
 * archive_in_file -- Archive database in GEDCOM file
 *=================================================*/
BOOLEAN
archive_in_file (struct tag_export_feedback * efeed, FILE *fp)
{
	char dat[30]="", tim[20]="";
	struct tm *pt=0;
	time_t curtime;
	STRING str=0;
	struct tag_trav_parm travparm;
	xlat_gedout = transl_get_predefined_xlat(MINGD); /* internal to GEDCOM */

	curtime = time(NULL);
	pt = localtime(&curtime);
	sprintf(dat, "%d %s %d", pt->tm_mday, mabbv[pt->tm_mon],
	    1900 + pt->tm_year);
	sprintf(tim, "%d:%.2d", pt->tm_hour, pt->tm_min);
	fprintf(fp, "0 HEAD\n1 SOUR LIFELINES %s\n1 DEST ANY\n"
		, get_lifelines_version(80));
	/* header date & time */
	fprintf(fp, "1 DATE %s\n2 TIME %s\n", dat, tim);
	/* header submitter entry */
	str = getlloptstr("HDR_SUBM", "1 SUBM");
	fprintf(fp, "%s\n", str);
	/* header gedcom version info */
	str = getlloptstr("HDR_GEDC", "1 GEDC\n2 VERS 5.5\n2 FORM LINEAGE-LINKED");
	fprintf(fp, "%s\n", str);
	/* header character set info */
	/* should be outcharset; that is what is being used */
	str = getlloptstr("HDR_CHAR", 0);
	if (str) {
		fprintf(fp, "%s\n", str);
	} else {
		/* xlat_gedout is the actual conversion used, so
		we should use the name of its output */
		CNSTRING outcharset = xl_get_dest_codeset(xlat_gedout);
		fprintf(fp, "1 CHAR %s\n", outcharset);
	}
	/* finished header */

	nindi = nfam = neven = nsour = nothr = 0;
	memset(&travparm, 0, sizeof(travparm));
	travparm.efeed = efeed;
	travparm.fp = fp;
	traverse_index_blocks(BTR, bmaster(BTR), &travparm, NULL, archive);
	fprintf(fp, "0 TRLR\n");
	return TRUE;
}
/*========================================================
 * archive -- Traverse function called on each btree block
 *======================================================*/
static BOOLEAN
archive (BTREE btree, BLOCK block, void * param)
{
	INT i, n, l;
	char scratch[100];
	STRING key;
	FILE *fo;
	struct tag_trav_parm * travparm = (struct tag_trav_parm *)param;

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
			copy_and_translate(fo, l, travparm, *key, xlat_gedout);
	}
	fclose(fo);
	return TRUE;
}
/*===================================================
 * copy_and_translate -- Copy record with translation
 *=================================================*/
static void
copy_and_translate (FILE *fo, INT len, struct tag_trav_parm * travparm, INT c, XLAT xlat)
{
	char in[BUFLEN];
	char *inp;
	int remlen, num;
	FILE * fn = travparm->fp;
	struct tag_export_feedback * efeed = travparm->efeed;
	
	inp = in;		/* location for next read */
	remlen = BUFLEN;	/* max for next read */
	while (len > 0) {
	    	if(len < remlen) remlen = len;
		ASSERT(fread(inp, remlen, 1, fo) == 1);
		len -= remlen;
		remlen = (inp + remlen) - in;	/* amount in current buffer */
		ASSERT(translate_write(xlat, in, &remlen, fn, (len <= 0)));
		inp = in + remlen;		/* position for next read */
		remlen = BUFLEN - remlen;	/* max for next read */
	}
	num = 0;
	switch (c) {
	case 'I': num = ++nindi; break;
	case 'F': num = ++nfam;  break;
	case 'E': num = ++neven; break;
	case 'S': num = ++nsour; break;
	case 'X': num = ++nothr; break;
	default: FATAL();
	}
	if (efeed && efeed->added_rec_fnc)
		efeed->added_rec_fnc((char)c, num);
}
