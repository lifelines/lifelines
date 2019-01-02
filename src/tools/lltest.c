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

#include <stddef.h>	/* offsetof */
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
INT verbose = 0;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void print_usage(void);
static void print_old_and_new_fkey(INT iter, FKEY old, FKEY new, FKEY compare);
static int test_nextfkey(BTREE btree);
static int test_fkey2path2fkey(void);
static int test_rkey2str(void);
static int test_str2rkey(void);
static int test_index(void);
static int test_block(void);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=========================================
 * main -- Main procedure of lltest command
 *=======================================*/
int
main (int argc,
	    char **argv)
{
	BTREE btree;
	char *dbname;
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
	/* also check for verbose flag, useful when debugging. */
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
		if (!strcmp(argv[i], "--verbose")) {
			verbose=1;
		}
	}

	/* Parse Command-Line Arguments */
	if (argc != (2+verbose)) {
		printf(_("lltest requires 1 argument (database)."));
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

	printf("testing block data structure...");
	rc = test_block();
	printf("%s %d\n",(rc==0?"PASS":"FAIL"),rc);

	printf("testing index data structure...");
	rc = test_index();
	printf("%s %d\n",(rc==0?"PASS":"FAIL"),rc);

	printf("testing rkey2str...");
	rc = test_rkey2str();
	printf("%s %d\n",(rc==0?"PASS":"FAIL"),rc);

	printf("testing str2rkey...");
	rc = test_str2rkey();
	printf("%s %d\n",(rc==0?"PASS":"FAIL"),rc);

	printf("testing fkey2path and path2fkey...");
	rc = test_fkey2path2fkey();
	printf("%s %d\n",(rc==0?"PASS":"FAIL"),rc);

	printf("Testing nextfkey...");
	      rc = test_nextfkey(btree);
	printf("%s %d\n",(rc==0?"PASS":"FAIL"),rc);

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
	printf(_("Usage lltest [database] <options>"));
	printf("\n\n");
	printf(_("Options:"));
	printf("\n");
	printf(_("\t--help\tdisplay this help and exit"));
	printf("\n");
	printf(_("\t--version\toutput version information and exit"));
	printf("\n");
	printf(_("\t--verbose\tenable verbose test output"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n");
	printf(_("\tlltest %s"), fname);
	printf("\n");
	printf(_("\tlltest %s --verbose"), fname);
	printf("\n\t\t");
	printf(_("Report bugs to https://github.com/MarcNo/lifelines/issues"));
	printf("\n");
}

/*===============================================
 * print_old_and_new_fkey -- helper function for test_nextfkey
 *=============================================*/
void
print_old_and_new_fkey(INT iter, FKEY old, FKEY new, FKEY compare)
{
	char *result = (new == compare) ? "OK" : "ERROR";
	char *oldpath = (old != -1) ? fkey2path(old) : "     ";

	printf(FMT_INT_02 " %s 0x" FMT_INT32_HEX " %s 0x" FMT_INT32_HEX " %s\n",iter, oldpath, old, fkey2path(new), new, result);
}

/*===============================================
 * test_nextfkey -- tests the nextfkey() function
 *=============================================*/
int
test_nextfkey(BTREE btree)
{
	INT i=0, rc=0;
	FKEY oldfkey=-1;
	FKEY fkey = btree->b_kfile.k_fkey;

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

	if (verbose) { printf("\n"); }

	/* Abort test if initial fkey doesn't match our assumptions */
	if (fkey != fkey_compare[0])
	{
		if (verbose) { printf("ERROR: Can't perform test since database is too large.  This test assumes a database with a single file key.\n"); }
		rc = 1;
		return rc;
	}

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

/*===============================================
 * test_fkey2path2fkey -- tests fkey2path and path2fkey
 *=============================================*/
int
test_fkey2path2fkey(void)
{
	struct tc_fkey { FKEY fkey; char path[6]; } ;
	struct tc_fkey tests[] = { { 0x00000000, "aa/aa" },
				   { 0x00010001, "ab/ab" },
				   { 0x00190019, "az/az" },
				   { 0x0019001a, "az/ba" },
				   { 0x001a001a, "ba/ba" },
				   { 0x02a302a3, "zz/zz" },
				   { 0x02a402a4, "{a/{a" } /* invalid, but we don't bounds check currently */
	                               };
	INT i;
	INT rc=0;

	if (verbose) { printf("\n"); }

	/* Validate Assumptions */
	if (sizeof(FKEY) != sizeof(INT32)) { rc = 1; goto exit; }

	/* Validate Behaviour */
	for (i=0; i<ARRSIZE(tests); i++)
	{
		char *path = fkey2path(tests[i].fkey);
		FKEY fkey = path2fkey(tests[i].path);

		if (verbose)
		{
			printf("0x%08x -> %s expected %s\n",tests[i].fkey, path, tests[i].path);
			printf("%s -> 0x%08x expected 0x%08x\n",tests[i].path, fkey, tests[i].fkey);
		}

		if ((strcmp(path, tests[i].path)) || (fkey != tests[i].fkey)) { rc = 2+i; break; }
	}

exit:
	return rc;
}

/*===============================================
 * test_rkey2str -- tests rkey2str
 *=============================================*/
int
test_rkey2str(void)
{
	struct tc_rkey { RKEY rkey; char rkeystr[RKEYLEN+1]; } ;
	struct tc_rkey tests[] = { { { "     I00"  }, "I00"      }, /* typical */
	                                 { { "I1234567"  }, "I1234567" }, /* =max input record */
	                                 { { "  I123  "  }, "I123  "   }, /* trailing blanks, not typical */
	                               };
	INT i;
	INT rc=0;

	if (verbose) { printf("\n"); }

	/* Validate Assumptions */
	if (RKEYLEN != 8) { rc = 1; goto exit; }

	/* Validate Behaviour */
	for (i=0; i<ARRSIZE(tests); i++)
	{
		char *str = rkey2str(tests[i].rkey);

		if (verbose)
		{
			printf("'%.8s' -> '%s' expected '%s'\n", tests[i].rkey.r_rkey, str, tests[i].rkeystr);
		}

		if (strcmp(str, tests[i].rkeystr)) { rc = 2+i; break; }
		if (strlen(tests[i].rkeystr) > RKEYLEN) { rc = 2+i; break; }
	}

exit:
	return rc;
}

/*===============================================
 * test_str2rkey -- tests str2rkey
 *=============================================*/
int
test_str2rkey(void)
{
	struct tc_rkey { char rkeystr[RKEYLEN+1]; RKEY rkey; } ;
	struct tc_rkey tests[] = { { "I00",       { "     I00" } }, /* typical */
	                                 { "I1234567",  { "I1234567" } }, /* =max record */
	                                 { "I12345678", { "I1234567" } }, /* >max record (truncate) */
	                                 { "I123  ",    { "  I123  " } }, /* trailing blanks */
	                               };
	INT i;
	INT rc=0;

	if (verbose) { printf("\n"); }

	/* Validate Assumptions */
	if (RKEYLEN != 8) { rc = 1; goto exit; }

	/* Validate Behaviour */
	for (i=0; i<ARRSIZE(tests); i++)
	{
		RKEY rkey = str2rkey(tests[i].rkeystr);

		if (verbose)
		{
			printf("'%.8s' -> '%.8s' expected '%.8s'\n", tests[i].rkeystr, rkey.r_rkey, tests[i].rkey.r_rkey);
		}

		if (strncmp(rkey.r_rkey, tests[i].rkey.r_rkey, RKEYLEN)) { rc = 2+i; break; }
	}

exit:
	return rc;
}

/*===============================================
 * test_index -- tests INDEX data structure
 *=============================================*/
int
test_index(void)
{
	INT rc=0;

	if (verbose) { printf("\n"); }

	/* Validate Assumptions */
	if (sizeof(FKEY) != sizeof(INT32)) { rc=1; goto exit; }
	if (sizeof(RKEY) != RKEYLEN)       { rc=2; goto exit; }
	if (NOENTS != 340)                 { rc=3; goto exit; }

	/* Validate Size and Offsets */

	/* WARNING!! WARNING!! WARNING!! WARNING!! WARNING!!  */
	/* This test code assumes 32-bit alignment.  This may */
	/* not be accurate for databases created on older DOS */
	/* or Windows 3.x systems which used 16-bit alignment.*/
	/* WARNING!! WARNING!! WARNING!! WARNING!! WARNING!!  */

	if (verbose)
	{
		printf("%s " FMT_SIZET "\n", "ix_self",    offsetof(INDEXSTRUCT,ix_self));
		printf("%s " FMT_SIZET "\n", "ix_type",    offsetof(INDEXSTRUCT,ix_type));
		printf("%s " FMT_SIZET "\n", "ix_parent",  offsetof(INDEXSTRUCT,ix_parent));
		printf("%s " FMT_SIZET "\n", "ix_nkeys",   offsetof(INDEXSTRUCT,ix_nkeys));
		printf("%s " FMT_SIZET "\n", "ix_rkeys",   offsetof(INDEXSTRUCT,ix_rkeys));
		printf("%s " FMT_SIZET "\n", "ix_fkeys",   offsetof(INDEXSTRUCT,ix_fkeys));
		printf("%s " FMT_SIZET "\n", "INDEXTRUCT", sizeof(INDEXSTRUCT));
	}

	if (offsetof(INDEXSTRUCT, ix_self)   != 0)  { rc=4; goto exit; }
	if (offsetof(INDEXSTRUCT, ix_type)   != 4)  { rc=5; goto exit; }
	if (offsetof(INDEXSTRUCT, ix_parent) != 8)  { rc=6; goto exit; } /* implicit padding on ix_type */
	if (offsetof(INDEXSTRUCT, ix_nkeys)  != 12) { rc=7; goto exit; }
	if (offsetof(INDEXSTRUCT, ix_rkeys)  != 14)   { rc=8;  goto exit; } /* no implicit padding since ix_rkeys is a char array */
	if (offsetof(INDEXSTRUCT, ix_fkeys)  != 2736) { rc=9;  goto exit; } /* implicit padding on ix_rkeys */
	if (sizeof(INDEXSTRUCT)              != 4096) { rc=10; goto exit; }

exit:
	return rc;
}

/*===============================================
 * test_block -- tests BLOCK data structure
 *=============================================*/

int test_block(void)
{
	INT rc=0;

	if (verbose) { printf("\n"); }

	/* Validate Assumptions */
	if (sizeof(FKEY) != sizeof(INT32)) { rc=1; goto exit; }
	if (sizeof(RKEY) != RKEYLEN)       { rc=2; goto exit; }
	if (NORECS != 255)                 { rc=3; goto exit; }

	/* Validate Size and Offsets */

	/* WARNING!! WARNING!! WARNING!! WARNING!! WARNING!!  */
	/* This test code assumes 32-bit alignment.  This may */
	/* not be accurate for databases created on older DOS */
	/* or Windows 3.x systems which used 16-bit alignment.*/
	/* WARNING!! WARNING!! WARNING!! WARNING!! WARNING!!  */

	if (verbose)
	{
		printf("%s " FMT_SIZET "\n", "ix_self",    offsetof(BLOCKSTRUCT,ix_self));
		printf("%s " FMT_SIZET "\n", "ix_type",    offsetof(BLOCKSTRUCT,ix_type));
		printf("%s " FMT_SIZET "\n", "ix_parent",  offsetof(BLOCKSTRUCT,ix_parent));
		printf("%s " FMT_SIZET "\n", "ix_nkeys",   offsetof(BLOCKSTRUCT,ix_nkeys));
		printf("%s " FMT_SIZET "\n", "ix_rkeys",   offsetof(BLOCKSTRUCT,ix_rkeys));
		printf("%s " FMT_SIZET "\n", "ix_offs",    offsetof(BLOCKSTRUCT,ix_offs));
		printf("%s " FMT_SIZET "\n", "ix_lens",    offsetof(BLOCKSTRUCT,ix_lens));
		printf("%s " FMT_SIZET "\n", "BLOCKSTRUCT", sizeof(BLOCKSTRUCT));
	}

	if (offsetof(BLOCKSTRUCT, ix_self)   != 0)    { rc=4;  goto exit; }
	if (offsetof(BLOCKSTRUCT, ix_type)   != 4)    { rc=5;  goto exit; }
	if (offsetof(BLOCKSTRUCT, ix_parent) != 8)    { rc=6;  goto exit; } /* implicit padding on ix_type*/
	if (offsetof(BLOCKSTRUCT, ix_nkeys)  != 12)   { rc=7;  goto exit; }
	if (offsetof(BLOCKSTRUCT, ix_rkeys)  != 14)   { rc=8;  goto exit; } /* no implicit padding since ix_rkeys is a char array */
	if (offsetof(BLOCKSTRUCT, ix_offs)   != 2056) { rc=9;  goto exit; } /* implicit padding on ix_rkeys */
	if (offsetof(BLOCKSTRUCT, ix_lens)   != 3076) { rc=10; goto exit; }
	if (sizeof(BLOCKSTRUCT)              != 4096) { rc=11; goto exit; }

exit:
	return rc;
}

