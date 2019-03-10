/* 
   Copyright (c) 2019 Matthew Emmerton

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
 * lldump.c -- Command that dumps LifeLines databaes.
 *===============================================================*/

#include "llstdlib.h"
#include "btree.h"
#include "gedcom.h"
#include "version.h"

/*********************************************
 * required global variables
 *********************************************/
/* defined in liflines/main.c */
STRING readpath_file = NULL;    /* normally defined in liflines/main.c */
STRING readpath = NULL;         /* normally defined in liflines/main.c */
BOOLEAN readonly = FALSE;       /* normally defined in liflines/main.c */
BOOLEAN writeable = FALSE;      /* normally defined in liflines/main.c */
BOOLEAN immutable = FALSE;  /* normally defined in liflines/main.c */
int opt_finnish = 0;
int opt_mychar = 0;
BTREE BTR;

/*==================================
 * work -- what the user wants to do
 *================================*/
struct work {
        INT dump_btree;
        INT dump_key;
        INT dump_record;
        INT dump_xref;
};
static struct work todo;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
void crashlog (STRING fmt, ...);
void crashlogn (STRING fmt, ...);
void dump_btree(BTREE btree);
void dump_keyfile(STRING dir);
void dump_xref(STRING dir);
void print_block(BLOCK block);
void print_index(INDEX index);
void print_keyfile1(void *kf1, int kf1size);
void print_keyfile2(void *kf2, int kf2size);
static void print_usage(void);
void print_xrefs(void);
static void vcrashlog (int newline, const char * fmt, va_list args);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=========================================
 * main -- Main procedure of lldump command
 *=======================================*/
int
main (int argc,
      char **argv)
{
	char cmdbuf[512];
	char *ptr, *flags, *dbname;
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
			print_version("lldump");
			return 0;
		}
		if (!strcmp(argv[i], "--help")
			|| !strcmp(argv[i], "-h")
			|| !strcmp(argv[i], "-?")) {
			print_usage();
			return 0;
		}
	}

       if (argc != 3 || argv[1][0] != '-' || argv[1][1] == '\0') {
                print_usage();
                goto finish;
        }
        flags = argv[1];
        dbname = argv[2];
        for (ptr=&flags[1]; *ptr; ++ptr) {
                switch(*ptr) {
                case 'a': todo.dump_btree=TRUE; todo.dump_key=TRUE; todo.dump_record=TRUE; todo.dump_xref=TRUE; break;
                case 'b': todo.dump_btree=TRUE; break;
                case 'k': todo.dump_key=TRUE; break;
                case 'r': todo.dump_record=TRUE; break;
                case 'x': todo.dump_xref=TRUE; break;
                default: print_usage(); goto finish;
                }
        }

	if (!(BTR = bt_openbtree(dbname, cflag, writ, immut, &lldberrnum))) {
		printf(_("Failed to open btree: %s."), dbname);
		puts("");
		return 20;
	}

	rtn = 0;

	if (todo.dump_btree) { }
	if (todo.dump_key) { dump_keyfile(dbname); }
	if (todo.dump_record) { }
	if (todo.dump_xref) { dump_xref(dbname); }

finish:
	closebtree(BTR);
	BTR = NULL;
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

	printf(_("lifelines `lldump' dumpss a lifelines database file.\n"));
	printf("\n\n");
	printf(_("Usage lldump [options] <database>"));
	printf("\n\n");
        printf(_("options:\n"));
        printf(_("\t-a = Dump ALL records\n"));
        printf(_("\t-b = Dump btree\n"));
        printf(_("\t-k = Dump key files\n"));
        printf(_("\t-x = Dump xref file\n"));
        printf(_("\t-r = Dump records\n"));
	printf("\n");
	printf(_("\t--help\tdisplay this help and exit"));
	printf("\n");
	printf(_("\t--version\toutput version information and exit"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n");
	printf(_("\tlldump %s"), fname);
	printf("\n\n");
	printf(_("Report bugs to https://github.com/MarcNo/lifelines/issues"));
	printf("\n");
}
/*===============================================
 * dump_btree -- open and print btree to stdout
 *=============================================*/
void dump_btree(BTREE btree)
{
        INDEX index;
        INT i, n, lo, hi;
        FKEY nfkey;
        BLOCK block;
        BOOLEAN found = FALSE;
        RAWRECORD rawrec;

        ASSERT(index = bmaster(btree));

// loop over nodes
	if (ixtype(index) == BTINDEXTYPE) {
	}
	if (ixtype(index) == BTBLOCKTYPE) {
	}
}
/*===============================
 * print_index -- print INDEX to stdout
 *=============================*/
void print_index(INDEX index)
{
	INT n;

	printf("INDEX\n");
	printf("fkey: %d type: %d parent: %d nkeys: %d\n", index->ix_self, index->ix_type, index->ix_parent, index->ix_nkeys);
	for (n=0; n<NOENTS; n++) {
		printf("  %d: rkey: '%s' fkey: %d\n", n, index->ix_rkeys[n], index->ix_fkeys[n]);
	}
	printf("\n");
}
/*===============================
 * print_block -- print BLOCK to stdout
 *=============================*/
void print_block(BLOCK block)
{
	INT n;

	printf("BLOCK\n");
	printf("fkey: %d type: %d parent: %d nkeys: %d\n", block->ix_self, block->ix_type, block->ix_parent, block->ix_nkeys);
	for (n=0; n<NORECS; n++) {
		printf("  %d: rkey: '%s' offs: %d lens: %d\n", n, block->ix_rkeys[n], block->ix_offs[n], block->ix_lens[n]);
	}
	printf("\n");
}
/*===============================
 * dump_keyfile -- open and print KEYFILE1 and KEYFILE2 to stdout
 *=============================*/
void dump_keyfile(STRING dir)
{
	char scratch[200];
	struct stat sbuf;
	KEYFILE1_n32 kfile1_n32;
	KEYFILE1_n64 kfile1_n64;
	KEYFILE2_n32 kfile2_n32;
	KEYFILE2_n64 kfile2_n64;
	void *kf1 = NULL;
	void *kf2 = NULL;
	FILE *fk;
	long size;
	int kf1size = 0;
	int kf2size = 0;

	sprintf(scratch, "%s/key", dir);
        if (stat(scratch, &sbuf) || !S_ISREG(sbuf.st_mode)) {
		printf("Error opening keyfile\n");
		goto error2;
	}

        if (!(fk = fopen(scratch, LLREADBINARY))) {
		printf("Error opening keyfile\n");
		goto error2;
	}

	if (fseek(fk, 0, SEEK_END)) {
		printf("Error seeking to end of keyfile\n");
		goto error1;
	}

        size = ftell(fk);

	if (fseek(fk, 0, SEEK_SET)) {
		printf("Error seeking to start of keyfile\n");
		goto error1;
	}

	/*
	 * Valid keyfile sizes:
	 * 12		(32-bit KEYFILE1 only)
	 * 12 + 28 = 40	(32-bit KEYFILE1 + KEYFILE2)
	 * 16		(64-bit KEYFILE1 only)
	 * 16 + 42 = 56 (64-bit KEYFILE1 + KEYFILE2)
	 */

	switch (size)
	{
		case 12:
			kf1size = 12;
			kf1 = &kfile1_n32;
			break;
		case 40:
			kf1size = 12;
			kf1 = &kfile1_n32;
			kf2size = 28;
			kf2 = &kfile2_n32;
			break;
		case 16:
			kf1size = 16;
			kf1 = &kfile1_n64;
			break;
		case 56:
			kf1size = 16;
			kf1 = &kfile1_n64;
			kf2size = 40;
			kf2 = &kfile2_n64;
			break;
		default:
			break;
	}

        if (fread(kf1, kf1size, 1, fk) != 1) {
		printf("Error reading keyfile\n");
		goto error1;
        }

	print_keyfile1(kf1, kf1size);

        if (fread(kf2, kf2size, 1, fk) != 1) {
		printf("Error reading keyfile2\n");
		goto error1;
        }

	print_keyfile2(kf2, kf2size);

error1:
	fclose(fk);
error2:
	return;
}
/*===============================
 * print_keyfile1 -- print KEYFILE1 to stdout
 *=============================*/
void print_keyfile1(void *kf1, int kf1size)
{
	printf("KEYFILE1\n");
	printf("========\n");
	printf("length: %d\n", kf1size);
	if (kf1size == 12)
	{
		KEYFILE1_n32 *kfile1 = (KEYFILE1_n32 *) kf1;
		printf("mkey: 0x%08x (%d) fkey: 0x%08x (%d) ostat: 0x%08x (%d)\n",
		       kfile1->k_mkey, kfile1->k_mkey,
		       kfile1->k_fkey, kfile1->k_fkey,
		       kfile1->k_ostat, kfile1->k_ostat);
	} else if (kf1size == 16) {
		KEYFILE1_n64 *kfile1 = (KEYFILE1_n64 *) kf1;
		printf("mkey: 0x%08x (%d) fkey: 0x%08x (%d) ostat: 0x%016llx (%lld)\n",
		       kfile1->k_mkey, kfile1->k_mkey,
		       kfile1->k_fkey, kfile1->k_fkey,
		       kfile1->k_ostat, kfile1->k_ostat);
	} else {
		printf("Error printing keyfile1; invalid size (%d)\n",kf1size);
	}
	printf("\n");
}
/*===============================
 * print_keyfile2 -- print KEYFILE2 to stdout
 *=============================*/
void print_keyfile2(void *kf2, int kf2size)
{
	printf("KEYFILE2\n");
	printf("========\n");
	printf("length: %d\n", kf2size);
	if (kf2size == 28) {
		KEYFILE2_n32 *kfile2 = (KEYFILE2_n32 *) kf2;
		printf("name: '%-18s' pad: 0x%02x 0x%02x magic: 0x%08x version: 0x%08x (%d)\n",
 		       kfile2->name,
		       kfile2->pad[0], kfile2->pad[1],
		       kfile2->magic,
		       kfile2->version, kfile2->version);
	} else if (kf2size == 40) {
		KEYFILE2_n64 *kfile2 = (KEYFILE2_n64 *) kf2;
		printf("name: '%-18s' pad: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x magic: 0x%016llx version: 0x%016llx (%d)\n",
 		       kfile2->name,
		       kfile2->pad[0], kfile2->pad[1], kfile2->pad[2],
		       kfile2->pad[3], kfile2->pad[4], kfile2->pad[5],
		       kfile2->magic,
		       kfile2->version, kfile2->version);
	} else {
		printf("Error printing keyfile2; invalid size (%d)\n",kf2size);
	}
	printf("\n");
}
/*===============================
 * dump_xref -- open and print xrefs to stdout
 *=============================*/
void dump_xref(STRING dir)
{
	char scratch[200];
	struct stat sbuf;
	FILE *xfp;
	INT xrecs[5];

	if (!openxref(FALSE))
	{
		printf("Error opening/reading xrefs\n");
		goto error1;
	}

	print_xrefs();

error1:
	closexref();
error2:
	return;
}
/*===============================
 * print_xrefs -- print xrefs to stdout
 *=============================*/
void print_xrefs(void)
{
	printf("XREFFILE\n");
	printf("========\n");
	dumpxrefs();
}
/*===============================
 * vcrashlog -- Send crash info to screen
 *  internal implementation
 *=============================*/
static void
vcrashlog (int newline, const char * fmt, va_list args)
{
        vprintf(fmt, args);
        if (newline) {
                printf("\n");
        }
}
/*===============================
 * crashlog -- Send string to crash log and screen
 *=============================*/
void
crashlog (STRING fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        vcrashlog(0, fmt, args);
        va_end(args);
}
/*===============================
 * crashlogn -- Send string to crash log and screen
 *  add carriage return to end line
 *=============================*/
void
crashlogn (STRING fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        vcrashlog(1, fmt, args);
        va_end(args);
}

