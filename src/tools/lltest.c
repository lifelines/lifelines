/* 
   Copyright (c) 1991-2018 Thomas T. Wetmore IV

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

/*=================================================================
 * lltest.c -- Command that executes various low-level logic tests
 *             on btree functions.
 *===============================================================*/

#include "llstdlib.h"
#include "version.h"
#include "btree.h"
#include "../btree/btreei.h"
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
static void print_usage(void);
static void print_old_and_new_fkey(INT iter, FKEY old, FKEY new, FKEY compare);
static int test_nextfkey(BTREE btree);

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
	int rc=0;

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
			print_version("lltest");
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
	if (argc != 2) {
		printf(_("btedit requires 1 argument (database)."));
		puts("");
		printf(_("See `lltest --help' for more information."));
		puts("");
		return 10;
	}
	dbname = argv[1];
	if (!(btree = bt_openbtree(dbname, cflag, writ, immut, &lldberrnum))) {
		printf(_("Failed to open btree: %s."), dbname);
		puts("");
		return 20;
	}

	printf("Testing nextfkey...");
        rc = test_nextfkey(btree);
	printf("%s\n",(rc==0?"PASS":"FAIL"));

finish:
	closebtree(btree);
	btree = 0;
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
		printf("%s", details);
		printf("\nAT: ");
	}
	printf("%s: line %d\n", file, line);
	exit(1);
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

	printf(_("lifelines `lltest' runs low-level tests of btree code.\n"));
	printf("\n\n");
	printf(_("Usage lltest [database]"));
	printf("\n\n");
	printf(_("Options:"));
	printf("\n");
	printf(_("\t--help\tdisplay this help and exit"));
	printf("\n");
	printf(_("\t--version\toutput version information and exit"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n");
	printf(_("\tbtedit %s"), fname);
	printf("\n\t\t");
	printf(_("Report bugs to https://github.com/MarcNo/lifelines/issues"));
	printf("\n");
}

/*===============================================
 * test_nextfkey -- tests the nextfkey() function
 *=============================================*/
void
print_old_and_new_fkey(INT iter, FKEY old, FKEY new, FKEY compare)
{
	char *result = (new == compare) ? "OK" : "ERROR";
	char *oldpath = (old != -1) ? fkey2path(old) : "     ";

	printf("%02d %s 0x%08x %s 0x%08x %s\n",iter, oldpath, old, fkey2path(new), new, result);
}

int
test_nextfkey(BTREE btree)
{
	INT i=0, rc=0;
	FKEY oldfkey=-1;
	FKEY fkey = btree->b_kfile.k_fkey;
	int verbose=0;

        FKEY fkey_compare[] = { 0x00010001,
                                0x00000001,
                                0x00020000,
                                0x00020001,
                                0x00020002,
                                0x00000002,
                                0x00010002,
                                0x00030000,
                                0x00030001,
                                0x00030002,
                                0x00030003,
                                0x00000003,
                                0x00010003,
                                0x00020003,
                                0x00040000,
                                0x00040001 };

	/* Display current FKEY */
	if (verbose) { print_old_and_new_fkey(i, oldfkey, fkey, fkey_compare[i]); }

	/* Perform 15 iterations of nextfkey */
	while (i<15)
	{
		/* Save FKEY */
		oldfkey = fkey;

		/* Increment FKEY */
		nextfkey(btree);
		i++;
		fkey = btree->b_kfile.k_fkey;

		/* Display old and new FKEY */
		if (verbose) { print_old_and_new_fkey(i, oldfkey, fkey, fkey_compare[i]); }

		/* Compare FKEY */
		if (fkey != fkey_compare[i])
		{
			rc = 1;
			break;
		}
	}

	return rc;
}
