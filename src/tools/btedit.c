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
/*=================================================================
 * btedit.c -- Command that allows individual BTREE records to be
 *   edited directly.  Can only be used on records that are in pure
 *   ASCII format.
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 10 Dec 94
 *===============================================================*/

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "btree.h"
#include "version.h"

/*********************************************
 * required global variables
 *********************************************/
/* defined in liflines/main.c */
STRING readpath_file = NULL;
STRING readpath = NULL;
int opt_finnish = 0;
int opt_mychar = 0;
/* defined in gedlib/codesets.c */
BOOLEAN uu8=0;            /* flag if internal codeset is UTF-8 */
STRING int_codeset=0;     /* internal codeset */

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static int llsystem(const char * cmd);
static void print_usage(void);
static void print_version(CNSTRING program);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=========================================
 * main -- Main procedure of btedit command
 *=======================================*/
int
main (int argc,
      char **argv)
{
	BTREE btree;
	char cmdbuf[512];
	char *editor;
	char *dbname, *key;
	RECORD_STATUS recstat;
	BOOLEAN cflag=FALSE; /* create new db if not found */
	BOOLEAN writ=1; /* request write access to database */
	BOOLEAN immut=FALSE; /* immutable access to database */
	INT lldberrnum=0;
	int rtn=0;
	int i=0;

	/* TODO: needs locale & gettext initialization */

#ifdef WIN32
	/* TO DO - research if this is necessary */
	_fmode = O_BINARY;	/* default to binary rather than TEXT mode */
#endif

	/* handle conventional arguments --version and --help */
	/* needed for help2man to synthesize manual pages */
	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "--version")
			|| !strcmp(argv[i], "-v")) {
			print_version("btedit");
			return 0;
		}
		if (!strcmp(argv[i], "--help")
			|| !strcmp(argv[i], "-h")
			|| !strcmp(argv[i], "-?")) {
			print_usage();
			return 0;
		}
	}

	/* Parse Command-Line Arguments */
	if (argc != 3) {
		printf(_("btedit requires 2 arguments (btree and key)."));
		puts("");
		printf(_("See `btedit --help' for more information."));
		puts("");
		return 10;
	}
	dbname = argv[1];
	key = argv[2];
	if (!(btree = bt_openbtree(dbname, cflag, writ, immut, &lldberrnum))) {
		printf(_("Failed to open btree: %s."), dbname);
		puts("");
		return 20;
	}
	recstat = write_record_to_file(btree, str2rkey(key), "btedit.tmp");
	if (recstat != RECORD_SUCCESS) {
		if (recstat == RECORD_NOT_FOUND)
			printf(_("There is no record with key: %s"), key);
		else
			printf(_("Error accessing record: %s"), key);
		puts("");
		rtn = 30;
		goto finish;
	}

	editor = environ_determine_editor(PROGRAM_BTEDIT);
	sprintf(cmdbuf, "%s btedit.tmp", editor);
	if (llsystem(cmdbuf) != 0) {
		printf(_("Editor or system call failed."));
		puts("");
		printf(_("Database was not be modified."));
		puts("");
		rtn = 40;
		goto finish;
	}
	if (!bwrite(btree)) {
		printf(_("Readonly database can not be modified."));
		puts("");
		rtn = 50;
		goto finish;
	}

	if (!addfile(btree, str2rkey(key), "btedit.tmp")) {
		printf(_("Error writing file 'btedit.tmp'."));
		puts("");
		rtn = 60;
		goto finish;
	}
	unlink("btedit.tmp");
	printf(_("Record %s modified."), key);
	puts("");
	rtn = 0;

finish:
	closebtree(btree);
	btree = 0;
	return rtn;
}
/*=============================
 * llsystem -- wrapper for system call
 *  (handles Win32 version as well)
 * return value of 0 is assumed successful
 *===========================*/
static int
llsystem (const char * cmd)
{
	int rtn=-1;
#ifdef WIN32
        /* use w32system, because it will wait for the editor to finish */
        rtn = w32system(cmd);
#else
        rtn = system(cmd);
#endif
	return rtn;
}
/*=============================
 * __fatal -- Fatal error routine
 *  handles null or empty details input
 *===========================*/
void
__fatal (STRING file, int line, CNSTRING details)
{
	printf("FATAL ERROR: ");
	if (details && details[0]) {
		printf(details);
		printf("\nAT: ");
	}
	printf("%s: line %d\n", file, line);
	exit(1);
}
/*===============================================
 * print_version -- display program version
 *  displays to stdout
 *=============================================*/
static void
print_version (CNSTRING program)
{
	printf("%s (lifelines) %s\n", program, LIFELINES_VERSION);
	printf("\n");

	printf(_("Copyright (C) 1991-2007 Thomas T. Wetmore IV et al."));
	printf("\n");
	printf(_("This is free software; see the source for copying conditions.  There is NO\n"
		"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."));
	printf("\n");
	printf("\n");

	printf(_("Written by Tom Wetmore. Maintained at lifelines.sourceforge.net."));
	printf("\n");

}
/*===============================================
 * print_usage -- display program help/usage
 *  displays to stdout
 *=============================================*/
void
print_usage (void)
{
#ifdef WIN32
	char * fname = _("\"\\My Documents\\LifeLines\\Databases\\MyFamily\"");
#else
	char * fname = _("/home/users/myname/lifelines/databases/myfamily");
#endif

	printf(_("lifelines `btedit' edits raw binary btree blocks\n"
		"in lifelines database files. Do NOT use this unless\n"
		"you know what you are doing and you have backed up\n"
		"your database."));
	printf("\n\n");
	printf(_("Usage btedit [database] [rkey]"));
	printf("\n\n");
	printf(_("Options:"));
	printf("\n");
	printf(_("\t--help\tdisplay this help and exit"));
	printf("\n");
	printf(_("\t--version\toutput version information and exit"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n");
	printf(_("\tbtedit %s I34"), fname);
	printf("\n\t\t");
	printf(_("edit raw btree block for person I34"));
	printf("\n\n");
	printf(_("Report bugs to lifelines.sourceforge.net"));
	printf("\n");
}
