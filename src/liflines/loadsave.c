/* 
   Copyright (c) 2002 Perry Rapp

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
 * loadsave.c -- curses user interface for loading & saving GEDCOM
 * NB: Part of curses GUI version
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 *   Created: 2002/06 by Perry Rapp
 *==============================================================*/


#include "llstdlib.h"
#include "liflines.h"
#include "impfeed.h"
#include "mystring.h"
#include "lloptions.h"
#include "llinesi.h"
#include "screen.h"


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void clear_rec_counts(INT pass);
static void export_saved_rec(char ctype, INT count);
static void import_added_rec(char ctype, STRING tag, INT count);
static void import_adding_unused_keys(void);
static void import_beginning_import(STRING msg);
static void import_done(INT nindi, INT nfam, INT nsour, INT neven, INT nothr);
static void import_error_invalid(STRING reason);
static void import_readonly(void);
static void import_validated_rec(char ctype, STRING tag, INT count);
static void import_validating(void);
static void import_validation_error(STRING msg);
static void import_validation_warning(STRING msg);
static void update_rec_count(INT pass, char ctype, STRING tag, INT count);

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING readpath_file;
extern STRING qSoutarc, qSoutfin;
extern STRING qSwhatgedc;

/*********************************************
 * local variables
 *********************************************/

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/


/*================================
 * Functions to display record counts
 *==============================*/
static void
update_rec_count (INT pass, char ctype, STRING tag, INT count)
{
	INT offset = 9*pass;
	STRING numstr=0;
	char msg[100];
	INT row=0;

	switch(ctype) {
	case 'I':
		numstr = _pl("Person", "Persons", count);
		row = 1;
		break;
	case 'F':
		numstr = _pl("Family", "Families", count);
		row = 2;
		break;
	case 'S':
		numstr = _pl("Source", "Sources", count);
		row = 3;
		break;
	case 'E':
		numstr = _pl("Event", "Events", count);
		row = 4;
		break;
	default: 
		numstr = _pl("Other", "Others", count);
		row = 5;
		break;
	}
	snprintf(msg, sizeof(msg), "%6d %s", count, numstr);
	if (row == 5 && tag && tag[0])
		llstrappf(msg, sizeof(msg), uu8, " (%s)", tag);
	row += offset;
	clear_stdout_hseg(row, 1, 70); /* TODO: how wide should this be ? */
	wfield(row, 1, msg);
}
static void
clear_rec_counts (INT pass)
{
	update_rec_count(pass, 'I', 0, 0);
	update_rec_count(pass, 'F', 0, 0);
	update_rec_count(pass, 'S', 0, 0);
	update_rec_count(pass, 'E', 0, 0);
	update_rec_count(pass, 'X', 0, 0);
}
/*================================
 * Feedback functions for import
 *==============================*/
static void
import_validation_warning (STRING msg)
{
	wfield(7, 1, msg);
}
static void
import_validation_error (STRING msg)
{
	wfield(6, 1, msg);
}
static void
import_error_invalid (STRING reason)
{
	wfield(9, 0, reason);
	wpos(10, 0);
}
static void
import_validating (void)
{
	llwprintf(_("Checking GEDCOM file for errors.\n"));

	clear_rec_counts(0);
	wfield(6, 1, "     0 Errors");
	wfield(7, 1, "     0 Warnings");
}
static void
import_beginning_import (STRING msg)
{
	wfield(9,  0, msg);
	clear_rec_counts(1);
}
static void
import_readonly (void)
{
	wfield(10, 0, _("The database is read-only; loading has been canceled."));
	wpos(11, 0);
}
static void
import_adding_unused_keys (void)
{
	wfield(15, 0, _("Adding unused keys as deleted keys..."));
}
static void
import_done (INT nindi, INT nfam, INT nsour, INT neven, INT nothr)
{
	wpos(15, 0);
	msg_info(_("Added (%dP, %dF, %dS, %dE, %dX) records"),
		nindi, nfam, nsour, neven, nothr);
}
static void
import_validated_rec (char ctype, STRING tag, INT count)
{
	update_rec_count(0, ctype, tag, count);
}
static void
import_added_rec (char ctype, STRING tag, INT count)
{
	update_rec_count(1, ctype, tag, count);
}
static void
export_saved_rec (char ctype, INT count)
{
	update_rec_count(0, ctype, "", count);
}
/*================================
 * load_gedcom -- have user select gedcom file & import it
 *==============================*/
void
load_gedcom (BOOLEAN picklist)
{
	FILE *fp=NULL;
	struct import_feedback ifeed;
	STRING srcdir=NULL;
	STRING fullpath=0;

	srcdir = getoptstr("InputPath", ".");
	if (!ask_for_gedcom(LLREADTEXT, _(qSwhatgedc), 0, &fullpath, srcdir, ".ged", picklist)
		|| !(fp = fopen(fullpath, LLREADTEXT))) {
		strfree(&fullpath);
		return;
	}

	memset(&ifeed, 0, sizeof(ifeed));
	ifeed.validating_fnc = import_validating;
	ifeed.validated_rec_fnc = import_validated_rec;
	ifeed.beginning_import_fnc = import_beginning_import;
	ifeed.error_invalid_fnc = import_error_invalid;
	ifeed.error_readonly_fnc = import_readonly;
	ifeed.adding_unused_keys_fnc = import_adding_unused_keys;
	ifeed.import_done_fnc = import_done;
	ifeed.added_rec_fnc = import_added_rec;
	ifeed.validation_error_fnc = import_validation_error;
	ifeed.validation_warning_fnc =  import_validation_warning;

	import_from_gedcom_file(&ifeed, fp);
	
	fclose(fp);
	strfree(&fullpath);
}

/*================================
 * save_gedcom -- save gedcom file
 *==============================*/
BOOLEAN
save_gedcom (void)
{
	FILE *fp=NULL;
	struct export_feedback efeed;
	STRING srcdir=NULL, fname=0, fullpath=0;

	srcdir = getoptstr("LLARCHIVES", ".");
	fp = ask_for_output_file(LLWRITETEXT, _(qSoutarc), &fname, &fullpath, srcdir, ".ged");
	if (!fp) {
		strfree(&fname);
		msg_error(_("The database was not saved."));
		return FALSE; 
	}

	memset(&efeed, 0, sizeof(efeed));
	efeed.added_rec_fnc = export_saved_rec;

	llwprintf("Saving database `%s' in file `%s'.", readpath_file, fname);
	wfield(2, 1, "     0 Persons");
	wfield(3, 1, "     0 Families");
	wfield(4, 1, "     0 Events");
	wfield(5, 1, "     0 Sources");
	wfield(6, 1, "     0 Others");
	

	archive_in_file(&efeed, fp);
	fclose(fp);

	wpos(7,0);
	msg_info(_(qSoutfin), readpath_file, fname);
	strfree(&fname);
	
	return TRUE;
}
