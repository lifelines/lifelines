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
 * main.c -- Main program of LifeLines
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Aug 93
 *   2.3.6 - 02 Oct 93    3.0.0 - 11 Oct 94
 *   3.0.1 - 11 Oct 93    3.0.2 - 01 Jan 95
 *   3.0.3 - 02 Jul 96
 *===========================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include "standard.h"
#include "screen.h"
#include "btree.h"
#include "table.h"
#include "gedcom.h"

extern STRING idldir, nodbse, crdbse, nocrdb, iddbse, usage;

static STRING usage = (STRING) "lines [-akrwf] [database]";

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
STRING version = (STRING) "3.0.3OpenSource";
STRING betaversion = (STRING) "-0.2";
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

	initscr();
	noecho();
	init_screen();
	set_signals();
	opterr = 0;	/* turn off getopt's error message */
	while ((c = getopt(argc, argv, "akrwfmnt")) != -1) {
		switch (c) {
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
			wprintf(usage);
			exit_it(1);
		}
	}
	if (readonly && writeable) {
		wprintf("Select at most one of -r and -w options.");
		exit_it(1);
	}
	c = argc - optind;
	if (c > 1) {
		wprintf(usage);
		exit_it(1);
	}
	if (c <= 0) {
		btreepath = (STRING) ask_for_string(idldir, "enter path: ");
		if (!btreepath || *btreepath == 0) {
			wprintf(iddbse);
			exit_it(1);
		}
		btreepath = strsave(btreepath);
	} else
		btreepath = argv[optind];
	if (!btreepath || *btreepath == 0) {
		wprintf(usage);
		exit_it(1);
	}
	lldatabases = (STRING) getenv("LLDATABASES");
	if (!lldatabases || *lldatabases == 0) lldatabases = (STRING) ".";
	readpath = filepath(btreepath, "r", lldatabases);
	if (!readpath) readpath = btreepath;
	if (forceopen) {
		char scratch[200];
		FILE *fp;
		KEYFILE kfile;
		struct stat sbuf;
		sprintf(scratch, "%s/key", readpath);
		if (stat(scratch, &sbuf) || !sbuf.st_mode&S_IFREG) {
			wprintf("Database error -- ");
			wprintf("could not open, read or write the key file.");
			exit_it(1);
		}
		if (!(fp = fopen(scratch, "r+")) ||
		    fread(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
			wprintf("Database error -- ");
			wprintf("could not open, read or write the key file.");
			exit_it(1);
		}
		kfile.k_ostat = 0;
		rewind(fp);
		if (fwrite(&kfile, sizeof(KEYFILE), 1, fp) != 1) {
			wprintf("Database error -- ");
			wprintf("could not open, read or write the key file.");
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
			wprintf("The database is already opened for ");
			wprintf("write access.\n  Try again later.");
		} else {
			wprintf("The database is already opened for ");
			wprintf("read access by %d users.\n  ", c - 1);
			wprintf("Try again later.");
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
	system("clear");
	exit(code);
}
/*===================================================
 * show_open_error -- Describe database opening error
 *=================================================*/
show_open_error ()
{
	if (bterrno != BTERRWRITER)
		wprintf("Database error -- ");
	switch (bterrno) {
	case BTERRNOBTRE:
		wprintf("requested database does not exist.");
		break;
	case BTERRINDEX:
		wprintf("could not open, read or write an index file.");
		break;
	case BTERRKFILE:
		wprintf("could not open, read or write the key file.");
		break;
	case BTERRBLOCK:
		wprintf("could not open, read or write a block file.");
		break;
	case BTERRLNGDIR:
		wprintf("name of database is too long.");
		break;
	case BTERRWRITER:
		wprintf("The database is already open for writing.");
		break;
	default:
		wprintf("Undefined database error -- This can't happen.");
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
