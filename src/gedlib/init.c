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
 * init.c -- Initialize LifeLines data structures
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 05 Oct 94    3.0.2 - 09 Nov 94
 *===========================================================*/

#include "standard.h"
#include "btree.h"
#include "table.h"
#include "gedcom.h"

TABLE tagtable;		/* table for tag strings */
TABLE placabbvs;	/* table for place abbrevs */
TABLE useropts;		/* table for user options */
BTREE BTR = NULL;	/* database */
STRING editstr, editfile;
STRING llarchives, llreports, llprograms;
STRING getenv();

/*=================================
 * init_lifelines -- Open LifeLines
 *===============================*/
init_lifelines ()
{
	STRING e, emsg;
	char scratch[100];
	tagtable = create_table();
	placabbvs = create_table();
	useropts = create_table();
	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", useropts, '=', &emsg);
	init_caches();
	init_browse_lists();
	init_mapping();
	e = getenv("LLEDITOR");
	if (!e || *e == 0) e = getenv("ED");
	if (!e || *e == 0) e = getenv("EDITOR");
	if (!e || *e == 0) e = (STRING) "vi";
	sprintf(scratch, "/tmp/%dltmp", getpid());
	editfile = strsave(scratch);
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	llprograms = getenv("LLPROGRAMS");
	if (!llprograms || *llprograms == 0) llprograms = (STRING) ".";
	llreports = getenv("LLREPORTS");
	if (!llreports || *llreports == 0) llreports = (STRING) ".";
	llarchives = getenv("LLARCHIVES");
	if (!llarchives || *llarchives == 0) llarchives = (STRING) ".";
	openxref();
}
/*===================================
 * close_lifelines -- Close LifeLines
 *=================================*/
close_lifelines ()
{
	char scratch[40];
	closexref();
	sprintf(scratch, "rm -f %s\n", editfile);
	system(scratch);
	closebtree(BTR);
}
