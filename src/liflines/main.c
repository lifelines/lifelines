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
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Aug 93
 *   2.3.6 - 02 Oct 93    3.0.0 - 11 Oct 94
 *   3.0.1 - 11 Oct 93    3.0.2 - 01 Jan 95
 *   3.0.3 - 02 Jul 96
 *===========================================================*/

#include "sys_inc.h"
#ifdef OS_LOCALE
#include <locale.h>
#endif
#include "llstdlib.h"
#include "screen.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "liflines.h"
#include "arch.h"

#include "llinesi.h"

#ifdef HAVE_GETOPT
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#endif /* HAVE_GETOPT */

/*********************************************
 * external variables (no header)
 *********************************************/

extern STRING idldir, nodbse, crdbse, nocrdb, iddbse, usage;
extern STRING mtitle;

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
static STRING usage = (STRING) "lines [-akrwfmntcuFy] [database]   # Use -F for Finnish database";
# else
int opt_finnish  = TRUE;/* Finnish Language sorting order if TRUE */
static STRING usage = (STRING) "lines [-akrwfmntcuy] [database]   # Finnish database";
# endif
#else
int opt_finnish  = FALSE;/* Finnish Language sorting order id disabled*/
static STRING usage = (STRING) "lines [-akrwfmntcuy] [database]";
#endif

BOOLEAN opt_nocb  = FALSE;	/* no cb. data is displayed if TRUE */
BOOLEAN alloclog  = FALSE;	/* alloc/free debugging */
BOOLEAN keyflag   = TRUE;	/* show key values */
BOOLEAN readonly  = FALSE;	/* database is read only */
BOOLEAN forceopen = FALSE;	/* force database status to 0 */
BOOLEAN writeable = FALSE;	/* database must be writeable */
BOOLEAN cursesio  = TRUE;	/* use curses i/o */
BOOLEAN alldone   = FALSE;	/* completion flag */
BOOLEAN progrunning = FALSE;	/* program is running */
BOOLEAN progparsing = FALSE;	/* program is being parsed */
BOOLEAN traceprogram = FALSE;	/* trace program */
BOOLEAN traditional = TRUE;	/* use traditional family rules */
BOOLEAN selftest = FALSE; /* selftest rules (ignore paths) */
BOOLEAN showusage = FALSE;	/* show usage */
STRING btreepath;		/* database path given by user */
STRING readpath;		/* database path used to open */
STRING lldatabases;

/*********************************************
 * local function prototypes
 *********************************************/

static void show_open_error(void);
static BOOLEAN trytocreate(STRING);
static void platform_init(void);
static int open_database(void);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==================================
 * main -- Main routine of LifeLines
 *================================*/
int
main (INT argc,
      char **argv)
{
	extern char *optarg;
	extern int optind;
	int c,code=1;

#ifdef OS_LOCALE
	setlocale(LC_ALL, "");
#endif

	/* catch any fault, so we can close database */
	set_signals();

	/* Parse Command-Line Arguments */
	opterr = 0;	/* turn off getopt's error message */
	while ((c = getopt(argc, argv, "akrwfmntc:Fu:y")) != -1) {
		switch (c) {
		case 'c':	/* adjust cache sizes */
			while(optarg && *optarg) {
				if(isalpha(*optarg) && isupper(*optarg))
					*optarg = tolower(*optarg);
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
				while(*optarg && isdigit(*optarg)) optarg++;
				if(*optarg == ',') optarg++;
				while(*optarg && isdigit(*optarg)) optarg++;
			}
			break;
#ifdef FINNISH
# ifdef FINNISHOPTION
		case 'F':	/* Finnish sorting order [toggle] */
			opt_finnish = !opt_finnish;
			break;
# endif
#endif
		case 'a':	/* debug allocation */
			alloclog = TRUE;
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
		case 'm':
			cursesio = FALSE;
			break;
		case 'f':	/* force database open in all cases */
			forceopen = TRUE;
			break;
		case 'n':	/* use non-traditional family rules */
			traditional = FALSE;
			break;
		case 't':
			traceprogram = TRUE;
			break;
		case 'u':
			sscanf(optarg, "%d,%d", &winx, &winy);
			break;
		case 'y':
			selftest = TRUE;
			break;
		case '?':
			showusage = TRUE;
			goto usage;
			break;
		}
	}

	/* Initialize Curses UI */
	initscr();
	platform_init();
	noecho();
	set_displaykeys(keyflag);
	if (!init_screen())
		goto finish;

	/* Validate Command-Line Arguments */
	if (readonly && writeable) {
		llwprintf("Select at most one of -r and -w options.");
		goto finish;
	}
	c = argc - optind;
	if (c > 1) {
		showusage = TRUE;
		goto usage;
	}

	if (selftest == FALSE) {
		lldatabases = environ_determine_database();
		lldatabases = strsave(lldatabases);
	}
	/* Get Database Name (Prompt or Command-Line) */
	if (c <= 0) {
		btreepath = ask_for_lldb(idldir, "enter path: ", lldatabases);
		if (ISNULL(btreepath)) {
			llwprintf(iddbse);
			goto finish;
		}
		btreepath = strsave(btreepath);
	} else {
		btreepath = (unsigned char *)argv[optind];
		if (ISNULL(btreepath)) {
			showusage = TRUE;
			goto usage;
		}
	}

	/* Open Database */
	readpath = filepath(btreepath, "r", lldatabases, NULL);
	if (!readpath) readpath = btreepath;
	if (!open_database()) goto finish;

	/* Start Program */
	init_lifelines();
	init_show_module();
	while (!alldone)
		main_menu();
	close_lifelines();
	code=0;

/*
 * MTE:  Here's were we would free() or deallocate() the dup'd strings
 * returned by strsave() and assigned to lldatabases and btreepath
 */

finish:
	term_screen();
	if (code) /* if error, give user a second to read it */
		sleep(1);
	/* Terminate Curses UI */
	endwin();

usage:
	/* Display Command-Line Usage Help */
	if (showusage) puts(usage);

	/* Exit */
	return(code);
}
/*==========================================
 * trytocreate -- Try to create new database
 *========================================*/
static BOOLEAN
trytocreate (STRING path)
{
	if (!ask_yes_or_no_msg(nodbse, crdbse)) return FALSE;

	if (!(BTR = openbtree(path, TRUE, !readonly))) {
		mprintf_error(nocrdb, path);
		return FALSE;
	}
	initxref();
	return TRUE;
}
/*===================================================
 * show_open_error -- Describe database opening error
 *=================================================*/
static void
show_open_error (void)
{
	if (bterrno != BTERRWRITER)
		llwprintf("Database error -- ");
	switch (bterrno) {
	case BTERRNOBTRE:
		llwprintf("requested database does not exist.");
		break;
	case BTERRINDEX:
		llwprintf("could not open, read or write an index file.");
		break;
	case BTERRKFILE:
		llwprintf("could not open, read or write the key file.");
		break;
	case BTERRBLOCK:
		llwprintf("could not open, read or write a block file.");
		break;
	case BTERRLNGDIR:
		llwprintf("name of database is too long.");
		break;
	case BTERRWRITER:
		llwprintf("The database is already open for writing.");
		break;
	case BTERRILLEGKF:
		llwprintf("keyfile is corrupt.");
		break;
	case BTERRALIGNKF:
		llwprintf("keyfile is wrong alignment.");
		break;
	case BTERRVERKF:
		llwprintf("keyfile is wrong version.");
		break;
	case BTERREXISTS:
		llwprintf("Existing database found.");
		break;
	default:
		llwprintf("Undefined database error -- This can't happen.");
		break;
	}
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
 * open_database -- open database
 *================================================*/
static int
open_database (void)
{
	int c;
	if (forceopen) {
		/*
		Forcefully alter reader/writer count to 0.
		But do check keyfile2 checks first (in case it is a
		database from a different alignment).
		*/
		char scratch[200];
		FILE *fp;
		KEYFILE1 kfile1;
		KEYFILE2 kfile2;
		struct stat sbuf;
		sprintf(scratch, "%s/key", readpath);
		if (stat(scratch, &sbuf) || !S_ISREG(sbuf.st_mode)) {
			llwprintf("Database error -- ");
			llwprintf("could not open, read or write the key file.");
			return FALSE;
		}
		if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
		    fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
			llwprintf("Database error -- ");
			llwprintf("could not open, read or write the key file.");
			return FALSE;
		}
		if (fread(&kfile2, sizeof(kfile2), 1, fp) == 1) {
			if (!validate_keyfile2(&kfile2)) {
				llwprintf("Database error -- ");
				llwprintf("Invalid keyfile!");
				return FALSE;
			}
		}
		kfile1.k_ostat = 0;
		rewind(fp);
		if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
			llwprintf("Database error -- ");
			llwprintf("could not open, read or write the key file.");
			printf("Cannot properly write the new key file.\n");
			fclose(fp);
	 		return FALSE;
		}
		fclose(fp);
	}
	if (!(BTR = openbtree(readpath, FALSE, !readonly))) {
		switch (bterrno) {
		case BTERRNOBTRE:
		case BTERRKFILE:	{/*NEW*/
				if (!selftest && is_unadorned_directory(btreepath)) {
					STRING llnewdbdir = environ_determine_newdbdir();
					readpath = strsave(concat_path(llnewdbdir, btreepath));
				}
				if(!trytocreate(readpath)) {
					show_open_error();
					return FALSE;
				}
			}
			break;
		default:
			show_open_error();
			return FALSE;
		}
	}
	readonly = !bwrite(BTR);
	if (readonly && writeable) {
		c = bkfile(BTR).k_ostat;
		if (c < 0) {
			llwprintf("The database is already opened for ");
			llwprintf("write access.\n  Try again later.");
		} else {
			llwprintf("The database is already opened for ");
			llwprintf("read access by %d users.\n  ", c - 1);
			llwprintf("Try again later.");
		}
		close_lifelines();
		return FALSE;
	}
	return TRUE;
}

