
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
/* modified 17 Aug 2000 by Matt Emmerton (matt@gsicomp.on.ca)  */
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * init.c -- Initialize LifeLines data structures
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 05 Oct 94    3.0.2 - 09 Nov 94
 *   3.0.3 - 21 Sep 95
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"

TABLE tagtable;		/* table for tag strings */
TABLE placabbvs;	/* table for place abbrevs */
TABLE useropts;		/* table for user options */
BTREE BTR = NULL;	/* database */
STRING editstr, editfile;
STRING llarchives, llreports, llprograms;
/* MTE: If this needs to be defined, please figure out which system header */
/*      isn't being included and make the change to std_inc.h */
/* char *getenv(); */

/*=================================
 * init_lifelines -- Open LifeLines
 *===============================*/
void
init_lifelines (void)
{
	STRING e, emsg;

	tagtable = create_table();
	placabbvs = create_table();
	useropts = create_table();
	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", useropts, '=', &emsg);
	init_caches();
	init_browse_lists();
	init_mapping();
	e = environ_determine_editor(PROGRAM_LIFELINES);
	editfile = strsave(environ_determine_tempfile());
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	llprograms = (STRING) getenv("LLPROGRAMS");
	if (!llprograms || *llprograms == 0) llprograms = (STRING) ".";
	llreports = (STRING) getenv("LLREPORTS");
	if (!llreports || *llreports == 0) llreports = (STRING) ".";
	llarchives = (STRING) getenv("LLARCHIVES");
	if (!llarchives || *llarchives == 0) llarchives = (STRING) ".";
	openxref();
}
/*===================================
 * close_lifelines -- Close LifeLines
 *=================================*/
void
close_lifelines (void)
{
	closexref();
	unlink(editfile);
	if(BTR) closebtree(BTR);
}
