
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
char *getenv();



/*=================================
 * figure_tempfile -- calculate temporary file (fully qualified path)
 *===============================*/
static STRING
figure_tempfile()
{
#ifdef WIN32
	STRING e;
	char win32_tempfile[PATH_MAX]; 

	/* windows has per-user temporary directory, depending on version */
	e = (STRING)getenv("TEMP");
	if (!e || *e == 0) e = (STRING)getenv("TMP");
	if (!e || *e == 0) e = "\\temp";
	strcpy(win32_tempfile, e);
	strcat(win32_tempfile, "\\lltmpXXXXXX");
	return mktemp(win32_tempfile);
#else
	static char unix_tempfile[] = "/tmp/lltmpXXXXXX";
	return mktemp(unix_tempfile);
#endif
}

/*=================================
 * figure_editor -- calculate editor program to use
 *===============================*/
static STRING
figure_editor()
{
	STRING e;

	e = (STRING) getenv("LLEDITOR");
	if (!e || *e == 0) e = (STRING) getenv("ED");
	if (!e || *e == 0) e = (STRING) getenv("EDITOR");
#ifdef WIN32
	/* win32 fallback is notepad */
	if (!e || *e == 0) e = (STRING) "notepad.exe";
#else
	/* unix fallback is vi */
	if (!e || *e == 0) e = (STRING) "vi";
#endif
	return e;
}

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
	e = figure_editor();
	editfile = strsave(figure_tempfile());
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	llprograms = (STRING) getenv("LLPROGRAMS");
	if (!llprograms || *llprograms == 0) llprograms = (STRING) ".";
	llreports = (STRING) getenv("LLREPORTS");
	if (!llreports || *llreports == 0) llreports = (STRING) ".";
	llarchives = (STRING) getenv("LLARCHIVES");
	if (!llarchives || *llarchives == 0) llarchives = (STRING) ".";
	openxref();
	init_show_module();
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
