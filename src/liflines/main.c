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

#ifdef OS_LOCALE
#include <locale.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include "standard.h"
#include "screen.h"
#include "btree.h"
#include "table.h"
#include "gedcom.h"

extern STRING idldir, nodbse, crdbse, nocrdb, iddbse, usage;

extern INT csz_indi, icsz_indi;
extern INT csz_fam, icsz_fam;
extern INT csz_sour, icsz_sour;
extern INT csz_even, icsz_even;
extern INT csz_othr, icsz_othr;

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
static STRING usage = (STRING) "lines [-akrwfmntcF] [database]   # Use -F for Finnish database";
# else
int opt_finnish  = TRUE;/* Finnish Language sorting order if TRUE */
static STRING usage = (STRING) "lines [-akrwfmntc] [database]   # Finnish database";
# endif
#else
int opt_finnish  = FALSE;/* Finnish Language sorting order id disabled*/
static STRING usage = (STRING) "lines [-akrwfmntc] [database]";
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
BOOLEAN traceprogram = FALSE;	/* trace program */
BOOLEAN traditional = TRUE;	/* use traditional family rules */
STRING btreepath;		/* database path given by user */
STRING readpath;		/* database path used to open */
STRING version = (STRING) "3.0.5";
#ifdef WIN32
STRING betaversion = (STRING) "-0.4";
#else
STRING betaversion = (STRING) "-0.4Win32j";
#endif
extern int opterr;
extern BTREE BTR;
char *getenv();
STRING lldatabases;
STRING filepath();

/*==================================
 * main -- Main routine of LifeLines
 *================================*/
main (argc, argv)
INT argc;
char **argv;
{
	extern char *optarg;
	extern int optind;
	int c;

#ifdef OS_LOCALE
	setlocale(LC_ALL, "");
#endif

	initscr();
	noecho();
	init_screen();
	set_signals();
	opterr = 0;	/* turn off getopt's error message */
	while ((c = getopt(argc, argv, "akrwfmntc:F")) != -1) {
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
		case '?':
			llwprintf(usage);
			exit_it(1);
		}
	}
	if (readonly && writeable) {
		llwprintf("Select at most one of -r and -w options.");
		exit_it(1);
	}
	c = argc - optind;
	if (c > 1) {
		llwprintf(usage);
		exit_it(1);
	}
	if (c <= 0) {
		btreepath = (STRING) ask_for_string(idldir, "enter path: ");
		if (!btreepath || *btreepath == 0) {
			llwprintf(iddbse);
			exit_it(1);
		}
		btreepath = strsave(btreepath);
	} else
		btreepath = (unsigned char *)argv[optind];
	if (!btreepath || *btreepath == 0) {
		llwprintf(usage);
		exit_it(1);
	}
	lldatabases = (STRING) getenv("LLDATABASES");
	if (!lldatabases || *lldatabases == 0) lldatabases = (STRING) ".";
	readpath = filepath(btreepath, "r", lldatabases, NULL);
	if (!readpath) readpath = btreepath;
	if (forceopen) {
		char scratch[200];
		FILE *fp;
		KEYFILE kfile;
		struct stat sbuf;
		sprintf(scratch, "%s/key", readpath);
		if (stat(scratch, &sbuf) || !sbuf.st_mode&S_IFREG) {
			llwprintf("Database error -- ");
			llwprintf("could not open, read or write the key file.");
			exit_it(1);
		}
		if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
		    fread(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
			llwprintf("Database error -- ");
			llwprintf("could not open, read or write the key file.");
			exit_it(1);
		}
		kfile.k_ostat = 0;
		rewind(fp);
		if (fwrite(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
			llwprintf("Database error -- ");
			llwprintf("could not open, read or write the key file.");
			printf("Cannot properly write the new key file.\n");
			fclose(fp);
	 		exit_it(1);
		}
		fclose(fp);
	}
	if (!(BTR = openbtree(readpath, FALSE, !readonly))) {
		switch (bterrno) {
		case BTERRNOBTRE:
		case BTERRKFILE:	/*NEW*/
	    		if(!trytocreate(readpath)) {
				show_open_error();
				exit_it(1);
			}
			break;
		default:
			show_open_error();
			exit_it(1);
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
		exit_it(1);
	}
	init_lifelines();
	while (!alldone)
		main_menu();
	close_lifelines();
	exit_it(0);
}
/*==========================================
 * trytocreate -- Try to create new database
 *========================================*/
BOOLEAN trytocreate (path)
STRING path;
{
	if (!ask_yes_or_no_msg(nodbse, crdbse)) return FALSE;

	if (!(BTR = openbtree(path, TRUE, !readonly))) {
		mprintf(nocrdb, path);
		return FALSE;
	}
	initxref();
	return TRUE;
}
/*====================
 * exit_it -- All done
 *==================*/
exit_it (code)
INT code;
{
	endwin();
	sleep(1);
#ifndef WIN32
	system("clear");
#endif
	exit(code);
}
/*===================================================
 * show_open_error -- Describe database opening error
 *=================================================*/
show_open_error ()
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
	default:
		llwprintf("Undefined database error -- This can't happen.");
		break;
	}
}
/*===============
 * final_cleanup
 *==============*/
final_cleanup ()
{
	close_lifelines();
	endwin();
}
