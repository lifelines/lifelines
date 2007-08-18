/* 
   Copyright (c) 1991-2005 Thomas T. Wetmore IV

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
 * selectdb.c -- Code handling user choice of database
 *===========================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"
#include "arch.h"
#include "lloptions.h"
#include "interp.h"

#include "llinesi.h"
#include "screen.h" /* calling initscr, noecho, ... */


/*********************************************
 * external variables (no header)
 *********************************************/

extern STRING qSnodbse,qScrdbse;
extern STRING qSidldir,qSidldrp,qSiddbse;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN is_unadorned_directory(STRING path);
static void show_open_error(INT dberr);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==================================================
 * select_database -- open database (prompting if appropriate)
 * if fail, return FALSE, and possibly a message to display
 *  perrmsg - [OUT]  translated error message
 *================================================*/
BOOLEAN
select_database (STRING dbrequested, INT alteration, STRING * perrmsg)
{
	STRING dbdir = getlloptstr("LLDATABASES", ".");
	STRING dbused = 0;

	/* Get Database Name (Prompt or Command-Line) */
	if (!dbrequested || !dbrequested[0]) {
		char dbname[MAXPATHLEN];
		/* ask_for_db_filename returns static buffer, we save it below */
		if (!ask_for_db_filename(_(qSidldir), _(qSidldrp), dbdir, dbname, sizeof(dbname))
			|| !dbname[0]) {
			dbrequested = NULL;
			*perrmsg = _(qSiddbse);
			return FALSE;
		}
		dbrequested = strsave(dbname);
		if (eqstr(dbrequested, "?")) {
			INT n=0;
			LIST dblist=0, dbdesclist=0;
			strfree(&dbrequested);
			if ((n=get_dblist(dbdir, &dblist, &dbdesclist)) > 0) {
				INT i;
				i = choose_from_list(
					_("Choose database to open")
					, dbdesclist);
				if (i >= 0) {
					dbrequested = strsave(get_list_element(dblist, i+1, NULL));
				}
				release_dblist(dblist);
				release_dblist(dbdesclist);
			} else {
				*perrmsg = _("No databases found in database path");
				return FALSE;
			}
			if (!dbrequested) {
				*perrmsg = _(qSiddbse);
				return FALSE;
			}
		}
	}

	/* search for database */
	/* search for file in lifelines path */
	dbused = filepath(dbrequested, "r", dbdir, NULL, uu8);
	/* filepath returns alloc'd string */
	if (!dbused) dbused = strsave(dbrequested);

	if (!open_or_create_database(alteration, &dbused)) {
		return FALSE;
	}

	return TRUE;
}
/*==================================================
 * open_or_create_database -- open database, prompt for
 *  creating new one if it doesn't exist
 * if fails, displays error (show_open_error) and returns 
 *  FALSE
 *  alteration:   [IN]  flags for locking, forcing open...
 *  dbused:       [I/O] actual database path (may be relative)
 * If this routine creates new database, it will alter dbused
 * Created: 2001/04/29, Perry Rapp
 *================================================*/
BOOLEAN
open_or_create_database (INT alteration, STRING *dbused)
{
	INT lldberrnum=0;
	/* Open Database */
	if (open_database(alteration, *dbused, &lldberrnum))
		return TRUE;

	/* filter out real errors */
	if (lldberrnum != BTERR_NODB && lldberrnum != BTERR_NOKEY)
	{
		show_open_error(lldberrnum);
		return FALSE;
	}

	if (readonly || immutable || alteration)
	{
		llwprintf(_("Cannot create new database with -r, -i, -l, or -f flags."));
		return FALSE;
	}
	/*
	error was only that db doesn't exist, so lets try
	making a new one 
	If no database directory specified, add prefix llnewdbdir
	*/
	if (is_unadorned_directory(*dbused)) {
		STRING dbpath = getlloptstr("LLDATABASES", ".");
		CNSTRING newdbdir = get_first_path_entry(dbpath);
		STRING temp = *dbused;
		if (newdbdir) {
			char tempth[MAXPATHLEN];
			newdbdir = strdup(newdbdir);
			concat_path(newdbdir, *dbused, uu8, tempth, sizeof(tempth));
			*dbused = strsave(tempth);
			stdfree(temp);
			stdfree((STRING)newdbdir);
		}
	}

	/* Is user willing to make a new db ? */
	if (!ask_yes_or_no_msg(_(qSnodbse), _(qScrdbse))) 
		return FALSE;

	/* try to make a new db */
	if (create_database(*dbused, &lldberrnum))
		return TRUE;

	show_open_error(lldberrnum);
	return FALSE;
}
/*===================================================
 * show_open_error -- Display database opening error
 *=================================================*/
static void
show_open_error (INT dberr)
{
	char buffer[256];
	describe_dberror(dberr, buffer, ARRSIZE(buffer));
	llwprintf(buffer);
	llwprintf("\n");
	sleep(5);
}
/*==================================================
 * is_unadorned_directory -- is it a bare directory name,
 *  with no subdirectories ?
 * Created: 2001/01/24, Perry Rapp
 *================================================*/
static BOOLEAN
is_unadorned_directory (STRING path)
{
	for ( ; *path; path++) {
		if (is_dir_sep(*path))
			return FALSE;
	}
	return TRUE;
}
