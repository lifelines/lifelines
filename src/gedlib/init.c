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
#include "version.h"
#include "lloptions.h"

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable;		/* table for tag strings */
TABLE placabbvs;	/* table for place abbrevs */
TABLE useropts;		/* table for user options */
BTREE BTR = NULL;	/* database */
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN selftest;

/*********************************************
 * local function prototypes
 *********************************************/

static STRING getsaveenv(STRING key);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=================================
 * init_lifelines_global -- Initialize LifeLines
 *  before db opened
 * STRING * pmsg: heap-alloc'd error string if fails
 *===============================*/
BOOLEAN
init_lifelines_global (STRING * pmsg)
{
	STRING e;
	*pmsg = NULL;
	read_lloptions_from_config();
	if (lloptions.lleditor[0])
		e = lloptions.lleditor;
	else
		e = environ_determine_editor(PROGRAM_LIFELINES);
	editfile = environ_determine_tempfile();
	if (!editfile) {
		*pmsg = strsave("Error creating temp file");
		return FALSE;
	}
	editfile = strsave(editfile );
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	/* read dirs from env if lacking */
	if (!lloptions.llprograms[0])
		changeoptstr(&lloptions.llprograms, getsaveenv("LLPROGRAMS"));
	if (!lloptions.llreports[0])
		changeoptstr(&lloptions.llreports, getsaveenv("LLREPORTS"));
	if (!lloptions.llarchives[0])
		changeoptstr(&lloptions.llarchives, getsaveenv("LLARCHIVES"));
	if (!lloptions.lldatabases[0])
		changeoptstr(&lloptions.lldatabases, getsaveenv("LLDATABASES"));
	if (!lloptions.llnewdbdir[0])
		changeoptstr(&lloptions.llnewdbdir, getsaveenv("LLNEWDBDIR"));
	if (selftest) {
		/* need to always find test stuff locally */
		changeoptstr(&lloptions.llprograms, NULL);
		changeoptstr(&lloptions.llreports, NULL);
		changeoptstr(&lloptions.lldatabases, NULL);
		changeoptstr(&lloptions.llnewdbdir, NULL);
	}
	/* fallback for dirs is . */
	if (!lloptions.llprograms[0])
		changeoptstr(&lloptions.llprograms, strsave("."));
	if (!lloptions.llreports[0])
		changeoptstr(&lloptions.llreports, strsave("."));
	if (!lloptions.llarchives[0])
		changeoptstr(&lloptions.llarchives, strsave("."));
	if (!lloptions.lldatabases[0])
		changeoptstr(&lloptions.lldatabases, strsave("."));
	if (!lloptions.llnewdbdir[0])
		changeoptstr(&lloptions.llnewdbdir, strsave("."));
	return TRUE;
}
/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
void
init_lifelines_db (void)
{
	STRING emsg;

	tagtable = create_table();
	placabbvs = create_table();
	useropts = create_table();
	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", useropts, '=', &emsg);
	init_caches();
	init_browse_lists();
	init_mapping();
	read_lloptions_from_db();
	openxref();
}
/*===============================================
 * getsaveenv -- Return strsave'd env value
 *  returns saved("") if getenv was NULL
 * Created: 2001/02/04, Perry Rapp
 *=============================================*/
static STRING
getsaveenv (STRING key)
{
	STRING val = getenv(key);
	if (!val)
		val = "";
	return strsave(val);
}
/*===============================================
 * get_lifelines_version -- Return version string
 *  returns static buffer
 *=============================================*/
STRING
get_lifelines_version (INT maxlen)
{
	static char version[128];
	char *ptr=version;
	INT len=sizeof(version);
	if (len>maxlen)
		len=maxlen;
	llstrcatn(&ptr, LIFELINES_VERSION, &len);
	return version;
}
/*===================================
 * close_lifelines -- Close LifeLines
 *  Safe to call even if not opened
 *=================================*/
void
close_lifelines (void)
{
	closexref();
	unlink(editfile);
	if(BTR) {
		closebtree(BTR);
		BTR=NULL;
	}
	if (editfile) {
		stdfree(editfile);
		editfile=NULL;
	}
	if (editstr) {
		stdfree(editstr);
		editstr=NULL;
	}
}
