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
 * main.c -- Main program of LifeLines
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Aug 93
 *   2.3.6 - 02 Oct 93    3.0.0 - 11 Oct 94
 *   3.0.1 - 11 Oct 93    3.0.2 - 01 Jan 95
 *   3.0.3 - 02 Jul 96
 *===========================================================*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "sys_inc.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "llstdlib.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"
#include "arch.h"
#include "lloptions.h"

#include "llinesi.h"
#include "screen.h" /* calling initscr, noecho, ... */


#ifdef HAVE_GETOPT
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#endif /* HAVE_GETOPT */

/*********************************************
 * external variables (no header)
 *********************************************/

extern STRING idldir, nodbse, crdbse, nocrdb, iddbse;
extern STRING mtitle, norwandro, nofandl, bdlkar;

extern INT csz_indi, icsz_indi;
extern INT csz_fam, icsz_fam;
extern INT csz_sour, icsz_sour;
extern INT csz_even, icsz_even;
extern INT csz_othr, icsz_othr;
extern INT winx, winy;

extern int opterr;
extern BTREE BTR;

/*********************************************
 * required global variables
 *********************************************/

/* Finnish language support modifies the soundex codes for names, so
 * a database created with this support is not compatible with other
 * databases. 
 *
 * define FINNISH for Finnish Language support
 *
 * define FINNISHOPTION to have a runtime option -F which will enable
 * 	  	Finnish language support, but you risk corrupting your
 * 	  	database if you make modifications while in the wrong mode.
 */

#ifdef FINNISH
# ifdef FINNISHOPTION
int opt_finnish  = FALSE;/* Finnish Language sorting order if TRUE */
static STRING usage = (STRING) "lines [-adkrwifmntcuFyxo] [database]   # Use -F for Finnish database";
# else
int opt_finnish  = TRUE;/* Finnish Language sorting order if TRUE */
static STRING usage = (STRING) "lines [-adkrwifmntcuyxo] [database]   # Finnish database";
# endif
#else
int opt_finnish  = FALSE;/* Finnish Language sorting order id disabled*/
static STRING usage = (STRING) "lines [-adkrwifmntcuyxo] [database]";
#endif

BOOLEAN debugmode = FALSE;     /* no signal handling, so we can get coredump */
BOOLEAN opt_nocb  = FALSE;     /* no cb. data is displayed if TRUE */
BOOLEAN keyflag   = TRUE;      /* show key values */
BOOLEAN readonly  = FALSE;     /* database is read only */
BOOLEAN writeable = FALSE;     /* database must be writeable */
BOOLEAN immutable = FALSE;     /* make no changes at all to database, for access to truly read-only medium */
BOOLEAN cursesio  = TRUE;      /* use curses i/o */
BOOLEAN alldone   = FALSE;     /* completion flag */
BOOLEAN progrunning = FALSE;   /* program is running */
BOOLEAN progparsing = FALSE;   /* program is being parsed */
INT     progerror = 0;         /* error count during report program */
BOOLEAN traceprogram = FALSE;  /* trace program */
BOOLEAN traditional = TRUE;    /* use traditional family rules */
BOOLEAN selftest = FALSE;      /* selftest rules (ignore paths) */
BOOLEAN showusage = FALSE;     /* show usage */
STRING  btreepath = NULL;      /* database path given by user */
STRING  readpath = NULL;       /* database path used to open */
STRING  deflocale = "C";       /* fallback for invalid user options */

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN is_unadorned_directory(STRING path);
static BOOLEAN open_or_create_database(INT alteration, STRING dbrequested, STRING dbused);
static void platform_init(void);
static void show_open_error(INT dberr);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==================================
 * main -- Main routine of LifeLines
 *================================*/
int
main (INT argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	char * msg;
	int c;
	BOOLEAN ok=FALSE;
	STRING dbrequested=NULL; /* database (path) requested */
	STRING dbused=NULL; /* database (path) found */
	BOOLEAN forceopen=FALSE, lockchange=FALSE;
	char lockarg = 0; /* option passed for database lock */
	INT alteration=0;
	STRING dbdir = 0;
	BOOLEAN exprog=FALSE;
	STRING exprog_name="";
	STRING progout=NULL;
	BOOLEAN graphical=TRUE;

#ifdef HAVE_SETLOCALE
	deflocale = strsave(setlocale(LC_ALL, ""));
#endif

	/* Parse Command-Line Arguments */
	opterr = 0;	/* turn off getopt's error message */
	while ((c = getopt(argc, argv, "adkrwil:fmntc:Fu:yx:o:z")) != -1) {
		switch (c) {
		case 'c':	/* adjust cache sizes */
			while(optarg && *optarg) {
				if(isalpha((uchar)*optarg) && isupper((uchar)*optarg))
					*optarg = tolower((uchar)*optarg);
				if(*optarg == 'i') {
					sscanf(optarg+1, "%d,%d", &csz_indi, &icsz_indi);
				}
				else if(*optarg == 'f') {
					sscanf(optarg+1, "%d,%d", &csz_fam, &icsz_fam);
				}
				else if(*optarg == 's') {
					sscanf(optarg+1, "%d,%d", &csz_sour, &icsz_sour);
				}
				else if(*optarg == 'e') {
					sscanf(optarg+1, "%d,%d", &csz_even, &icsz_even);
				}
				else if((*optarg == 'o') || (*optarg == 'x')) {
					sscanf(optarg+1, "%d,%d", &csz_othr, &icsz_othr);
				}
				optarg++;
				while(*optarg && isdigit((uchar)*optarg)) optarg++;
				if(*optarg == ',') optarg++;
				while(*optarg && isdigit((uchar)*optarg)) optarg++;
			}
			break;
#ifdef FINNISH
# ifdef FINNISHOPTION
		case 'F':	/* Finnish sorting order [toggle] */
			opt_finnish = !opt_finnish;
			/*
			TO DO - need to mark Finnish databases, as 
			name records are not interoperable, because of
			different soundex encoding
			2001/02/17, Perry Rapp
			*/
			break;
# endif
#endif
		case 'a':	/* debug allocation */
			alloclog = TRUE;
			break;
		case 'd':	/* debug = no signal catchers */
			debugmode = TRUE;
			break;
		case 'k':	/* don't show key values */
			keyflag = FALSE;
			break;
		case 'r':	/* request for read only access */
			readonly = TRUE;
			break;
		case 'w':	/* request for write access */
			writeable = TRUE;
			break;
		case 'i': /* immutable access */
			immutable = TRUE;
			readonly = TRUE;
			break;
		case 'l': /* locking switch */
			lockchange = TRUE;
			lockarg = *optarg;
			break;
		case 'm':
			cursesio = FALSE;
			break;
		case 'f':	/* force database open in all cases */
			forceopen = TRUE;
			break;
		case 'n':	/* use non-traditional family rules */
			traditional = FALSE;
			break;
		case 't': /* show lots of trace statements for debugging */
			traceprogram = TRUE;
			break;
		case 'u': /* specify screen dimensions */
			sscanf(optarg, "%d,%d", &winx, &winy);
			break;
		case 'y': /* only look in current path for databases */
			selftest = TRUE;
			break;
		case 'x': /* execute program */
			exprog = TRUE;
			exprog_name = optarg;
			break;
		case 'o': /* output directory */
			progout = optarg;
			break;
		case 'z': /* nongraphical box */
			graphical = FALSE;
			break;
		case '?':
			showusage = TRUE;
			goto usage;
			break;
		}
	}

	/* catch any fault, so we can close database */
	if (debugmode)
		stdstring_hardfail();
	else
		set_signals();

	/* Initialize Curses UI */
	initscr();
	platform_init();
	noecho();
	set_displaykeys(keyflag);
	/* initialize curses interface */
	if (!init_screen(graphical))
		goto finish;
	/* initialize options & misc. stuff */
	if (!init_lifelines_global(&msg)) {
		llwprintf("%s", msg);
		goto finish;
	}
	if (selftest) {
		/* need to always find test stuff locally */
		changeoptstr("LLPROGRAMS", strsave("."));
		changeoptstr("LLREPORTS", strsave("."));
		changeoptstr("LLDATABASES", strsave("."));
		changeoptstr("LLNEWDBDIR", strsave("."));
	}
	crash_setcrashlog(getoptstr("CrashLog", NULL));
	init_interpreter(); /* give interpreter its turn at initialization */

	/* Validate Command-Line Arguments */
	if ((readonly || immutable) && writeable) {
		llwprintf(norwandro);
		goto finish;
	}
	if (forceopen && lockchange) {
		llwprintf(nofandl);
		goto finish;
	}
	if (lockchange && lockarg != 'y' && lockarg != 'n') {
		llwprintf(bdlkar);
		goto finish;
	}
	if (forceopen)
		alteration = 3;
	else if (lockchange) {
		if (lockarg == 'y')
			alteration = 2;
		else
			alteration = 1;
	}
	c = argc - optind;
	if (c > 1) {
		showusage = TRUE;
		goto usage;
	}

	dbdir = getoptstr("LLDATABASES", ".");
	/* Get Database Name (Prompt or Command-Line) */
	if (c <= 0) {
		/* ask_for_db_filename returns static buffer, we save it below */
		dbrequested = ask_for_db_filename(idldir, "enter path: ", dbdir);
		if (ISNULL(dbrequested)) {
			llwprintf(iddbse);
			goto finish;
		}
	} else {
		dbrequested = argv[optind];
		if (ISNULL(dbrequested)) {
			showusage = TRUE;
			goto usage;
		}
	}
	/* we will own the memory in dbpath */
	dbrequested = strsave(dbrequested);

	/* search for database */
	/* search for file in lifelines path */
	dbused = filepath(dbrequested, "r", dbdir, NULL);
	if (!dbused) dbused = dbrequested;

	if (!open_or_create_database(alteration, dbrequested, dbused))
		goto finish;

	/* Start Program */
	init_lifelines_db();
	init_show_module();
	init_browse_module();
	if (exprog) {
		if (progout) {
			BOOLEAN append=FALSE;
			set_output_file(progout, append);
		}
		interp_program("main", 0, NULL, 1, &exprog_name, NULL, FALSE);
	} else {
		while (!alldone)
			main_menu();
	}
	term_show_module();
	term_browse_module();
	ok=TRUE;

/*
 * MTE:  Here's were we would free() or deallocate() the dup'd strings
 * returned by strsave() and assigned to lldatabases and btreepath
 */

finish:
	close_lifelines();
	shutdown_ui(!ok);
	strfree(&deflocale);

usage:
	/* Display Command-Line Usage Help */
	if (showusage) puts(usage);

	/* Exit */
	return !ok;
}
/*===================================================
 * shutdown_ui -- Do whatever is necessary to close GUI
 * Created: 2001/11/08, Perry Rapp
 *=================================================*/
void
shutdown_ui (BOOLEAN pause)
{
	term_screen();
	if (pause) /* if error, give user a second to read it */
		sleep(1);
	/* TO DO - signals also calls into here -- how do we figure out
	whether or not we should call endwin ? In case something happened
	before curses was invoked, or after it already closed ? */
	/* Terminate Curses UI */
	endwin();
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
	sleep(5);
}
/*==================================================
 * platform_init -- platform specific initialization
 *================================================*/
static void
platform_init (void)
{
#ifdef WIN32
	char buffer[80];
	sprintf(buffer, mtitle, get_lifelines_version(sizeof(buffer)-1-strlen(mtitle)));
	wtitle(buffer);
#endif
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
/*==================================================
 * open_or_create_database -- open database, prompt for
 *  creating new one if it doesn't exist
 * if fails, displays error (show_open_error) and returns 
 *  FALSE
 * dbrequested database specified by user
 * dbused: actual database path (may be relative also, if not found yet)
 * Created: 2001/04/29, Perry Rapp
 *================================================*/
static BOOLEAN
open_or_create_database (INT alteration, STRING dbrequested, STRING dbused)
{
	/* Open Database */
	if (open_database(alteration, dbrequested, dbused))
		return TRUE;
	/* filter out real errors */
	if (bterrno != BTERR_NODB && bterrno != BTERR_NOKEY)
	{
		show_open_error(bterrno);
		return FALSE;
	}
	if (readonly || immutable || alteration)
	{
		llwprintf("Cannot create new database with -r, -i, -l, or -f flags.");
		return FALSE;
	}
	/*
	error was only that db doesn't exist, so lets try
	making a new one 
	If no database directory specified, add prefix llnewdbdir
	*/
	if (!selftest && is_unadorned_directory(dbused)) {
		STRING newdbdir = getoptstr("LLNEWDBDIR", ".");
		STRING temp = dbused;
		dbused = strsave(concat_path(newdbdir, dbused));
		stdfree(temp);
	}

	/* Is user willing to make a new db ? */
	if (!ask_yes_or_no_msg(nodbse, crdbse)) 
		return FALSE;

	/* try to make a new db */
	if (create_database(dbrequested, dbused))
		return TRUE;

	show_open_error(bterrno);
	return FALSE;
}
