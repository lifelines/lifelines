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
 * gstrings.c -- Routines to creates child strings
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   3.0.0 - 02 May 94    3.0.2 - 24 Nov 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"

static INT nchil = 0, maxchil = 0;
static STRING *chstrings = NULL, *chkeys = NULL;

/*===================================================================
 * get_child_strings -- Return children strings; each string has name
 *   and event info, if avail  
 *=================================================================*/
STRING *
get_child_strings (NODE fam,
                   INT *pnum,
                   STRING **pkeys)
{
	NODE chil;
	INT i;

	for (i = 0; i < nchil; i++) {
		stdfree(chstrings[i]);
		stdfree(chkeys[i]);
	}
	nchil = *pnum = 0;
	if (!fam || !(chil = CHIL(fam))) return NULL;
	nchil = length_nodes(chil);
	if (nchil == 0) return NULL;
	if (nchil > (maxchil - 1)) {
		if (maxchil) {
			stdfree(chstrings); 
			stdfree(chkeys); 
		}
		chstrings = (STRING *) stdalloc((nchil+5)*sizeof(STRING));
		chkeys = (STRING *) stdalloc((nchil+5)*sizeof(STRING));
		maxchil = nchil + 5;
	}
	FORCHILDREN(fam,child,i)
		chstrings[i-1] = indi_to_list_string(child, NULL, 66);
		chkeys[i-1] = strsave(rmvat(nxref(child)));
	ENDCHILDREN
	*pnum = nchil;
	*pkeys = chkeys;
	return chstrings;
}
/*================================================
 * indi_to_list_string -- Return menu list string.
 *==============================================*/
STRING
indi_to_list_string (NODE indi,
                     NODE fam,
                     INT len)
{
	char unsigned scratch[MAXLINELEN];
	STRING name, evt = NULL, p = scratch;
	TRANTABLE ttd = tran_tables[MINDS];
	int hasparents;
	int hasfamily;
	if (indi) {
		ASSERT(name = indi_to_name(indi, ttd, len));
	} else
		name = (STRING) "Spouse unknown";
	sprintf(p, "%s", name);
	p += strlen(p);
	if (fam)  evt = fam_to_event(fam, ttd, "MARR", "m. ", len, TRUE);
	if (!evt) evt = indi_to_event(indi, ttd, "BIRT", "b. ", len, TRUE);
	if (!evt) evt = indi_to_event(indi, ttd, "CHR", "bap. ", len, TRUE);
	if (!evt) evt = indi_to_event(indi, ttd, "DEAT", "d. ", len, TRUE);
	if (!evt) evt = indi_to_event(indi, ttd, "BURI", "bur. ", len, TRUE);
	if (evt) {
		sprintf(p, ", %s", evt);
		p += strlen(p);
	}
	if (indi && keyflag) {
		sprintf(p, " (%s)", key_of_record(indi));
		p += strlen(p);
	}
	if (fam && keyflag) {
		sprintf(p, " (%s)", key_of_record(fam));
		p += strlen(p);
	}
	if(indi) {
	    if(FAMC(indi)) hasparents = 1;
	    else hasparents = 0;
	    if(FAMS(indi)) hasfamily = 1;
	    else hasfamily = 0;
	    if(hasfamily || hasparents) {
		*p++ = ' ';
		*p++ = '[';
		if(hasparents) *p++ = 'P';
		if(hasfamily) *p++ = 'S';
		*p++ = ']';
		*p = '\0';
	    }
	}
	if ((INT)strlen(scratch) > len)
		scratch[len] = 0;
	return strsave(scratch);
}
