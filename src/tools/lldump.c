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
 *
 * July 2020 added code to print all data found in lifelines data files
 * Especially Name, REFN, VUOPT data. lldump should now dump everything.
 * (as in you could reconstruct a database from the output of lldump
 * I haven't tried it, but I believe everything is now accounted for.)
 *===============================================================*/

#include "llstdlib.h"
#include "btree.h"
#include "../btree/btreei.h"	/* path2fkey */
#include "gedcom.h"
#include "version.h"
#include "toolsi.h"
#include "errno.h"

extern STRING qSgen_bugreport;

/*********************************************
 * required global variables
 *********************************************/
/* defined in liflines/main.c */
STRING readpath_file = NULL;   /* normally defined in liflines/main.c */
STRING readpath = NULL;        /* normally defined in liflines/main.c */
BOOLEAN readonly = FALSE;      /* normally defined in liflines/main.c */
BOOLEAN writeable = FALSE;     /* normally defined in liflines/main.c */
BOOLEAN immutable = FALSE;     /* normally defined in liflines/main.c */
STRING dbname = NULL;
int opt_finnish = 0;
int opt_mychar = 0;
extern BTREE BTR;

/*==================================
 * work -- what the user wants to do
 *================================*/
struct work {
	INT dump_btree;
	INT dump_key;
	INT dump_record;
	INT dump_xref;
};
int summarize = 0;	// 0 print unused directory entries
                        // 1 print summary of unused entries
                        // they are typically all 0, but when a INDEX is 
                        // split the 'deleted' entries remain
static struct work todo;
static INT lead_char[26]; 

typedef struct {
	char rkey[9];	// XREF value as found in database (usually 
					//      right justified, no null in field of 8
					//      but here it's null terminated.
	char rname[6];	// Type of XREF, INDI, FAM, Name, ...
	char *rkeyfirst;// pointer to 1st letter of XREF
	} keytype;
keytype akey;		// struct to store RKEY's validated with check_rkey()

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
void crashlog (STRING fmt, ...);
void crashlogn (STRING fmt, ...);
void dump_block(STRING dir);
void dump_index(STRING dir);
void dump_keyfile(STRING dir);
void dump_xref(STRING dir);
BOOLEAN tf_print_block(BTREE btree, BLOCK block, void *param);
void print_block(BTREE btree, BLOCK block, INT32 *offset);
BOOLEAN tf_print_index(BTREE btree, INDEX index, void *param);
void print_index(INDEX index, INT32 *offset);
void print_keyfile(KEYFILE1* kfile1, KEYFILE2* kfile2, INT32 size);
static void print_usage(void);
void print_xrefs(void);
static void vcrashlog (int newline, const char * fmt, va_list args);
static size_t getfilesize(STRING dir, STRING filename);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

// Direct assignment can trigger cast-align warnings.
// Only use these when the input is guaranteed to be aligned properly!
#define CASTPTR_INT32(in) (INT32*)((void*)(in))
#define EXTRACT_INT32(in) *(CASTPTR_INT32(in))
#define CASTPTR_INT64(in) (INT64*)((void*)(in))
#define EXTRACT_INT64(in) *(CASTPTR_INT64(in))

/*=========================================
 * main -- Main procedure of lldump command
 *=======================================*/
int
main (int argc,
      char **argv)
{
	char *ptr, *flags;
	BOOLEAN cflag=FALSE; /* create new db if not found */
	BOOLEAN writ=0; /* request write access to database */
	BOOLEAN immut=TRUE; /* immutable access to database */
	INT lldberrnum=0;
	int rtn=0;
	int i=0;

	set_signals(sighand_cmdline);

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
		case 's': summarize=1; break;
		case 'x': todo.dump_xref=TRUE; break;
		default: print_usage(); goto finish;
		}
	}

	if (!(BTR = bt_openbtree(dbname, cflag, writ, immut, &lldberrnum))) {
		printf(_("Failed to open btree: %s (" FMT_INT ": %s)."), dbname, lldberrnum, getlldberrstr(lldberrnum));
		puts("");
		return 20;
	}

	rtn = 0;

	if (todo.dump_key)    { dump_keyfile(dbname); }
	if (todo.dump_xref)   { dump_xref(dbname);    }
	if (todo.dump_btree)  { dump_index(dbname);   }
	if (todo.dump_record) { dump_block(dbname);   }

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

	printf(_("lifelines `lldump' dumps a lifelines database file.\n"));
	printf("\n\n");
	printf(_("Usage lldump [options] <database>"));
	printf("\n\n");
	printf(_("options:\n"));
	printf(_("\t-a = Dump ALL records\n"));
	printf(_("\t-b = Dump btree (INDEX)\n"));
	printf(_("\t-k = Dump key files (KEYFILE1, KEYFILE2)\n"));
	printf(_("\t-x = Dump xref file (DELETESET)\n"));
	printf(_("\t-r = Dump records (BLOCK)\n"));
	printf(_("\t-s = Summarize unused INDEX and BLOCK directory entries\n"));
	printf(_("\t     default dump all entries\n"));
	printf("\n");
	printf(_("\t--help\tdisplay this help and exit"));
	printf("\n");
	printf(_("\t--version\toutput version information and exit"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n");
	printf(_("\tlldump %s"), fname);
	printf("\n\n");
	printf("%s", _(qSgen_bugreport));
	printf("\n");
}

/*===============================================
 * check_rkey -- perform basic checks on RKEY
 * arguments are a pointer to a keytype to fill in, and pointer to RKEY to check
 * Valid start chars EFINRSVX
 * 1. it's 8 chars long
 * 2. no nulls in the 8 chars.
 * 3. check for valid characters - leading spaces, key letter, ...
 * 3a. EFISX  are letter followed by number
 *     represent XREF's for EVEN, FAM, INDI, SOUR, Other
 * 3b. R
 *     REFN 
 *     R followed by A-Za-z0-9 
 *     R's value is 2 or 3 chars, including the R
 * 3c. N  6 chars long, N[A-Z$][A-Z][0-9]{3,3}
 *     Name
 * 3d. V is VUOPT (only V key I am aware of)
 * 3e. H is HISTV or HISTC
 * messages to stdout - errors, warnings are printed and ignored
 * 
 *WARNING: item 2 is broken for Keys starting with a R, ie REFN data.
 *   if REFN entry has length 2, e.g. R7 then it's found as '     R7\0'
 *   for now if it isn't broke don't fix it.
 *   well it is broke, but there seem to be no side effects, so nothing is
 *   broke, and fixing it means finding all the places it's dealt with.
 *=============================================*/

static void
check_rkey(keytype *kt, RKEY *key, BOOLEAN in_data) {
	if (RKEY_IS_NULL(*key)) {
		// special case zero key
		strncpy(kt->rkey,"0x00 x 8",9);  // null key in 8 chars
		strncpy(kt->rname,"Zero",6);
		kt->rkeyfirst = kt->rkey;
		return;
	}
	memcpy(kt->rkey,key,9);  // grab string and 1 more char.
	kt->rkey[8] = 0;         // RKEY is 1st 8 char of key, force 9th char to null
	char *p = kt->rkey;
	while (*p == ' ') p++;   // p points to 1st non-space char
	kt->rkeyfirst = p;
	if (strlen(kt->rkey) != 8) {
		printf("Warning, embedded null in RKEY %s (0x",kt->rkeyfirst);
		for (int i = 0; i <= 7; i++) {
			printf("%2.2x",kt->rkey[i]);
		}
		printf(")\n");
	}
	if (in_data && *p >= 'A' && *p <= 'Z') {
	    lead_char[*p - 'A']++;
	}
	switch (*p) {
	case 'E': strncpy(kt->rname,"EVEN",6);  break;
	case 'F': strncpy(kt->rname,"FAM",6);   break;
	case 'I': strncpy(kt->rname,"INDI",6);  break;
	case 'N': strncpy(kt->rname,"Name",6);  break;
	case 'R': strncpy(kt->rname,"REFN",6);  break;
	case 'S': strncpy(kt->rname,"SOUR",6);  break;
	case 'V': strncpy(kt->rname,"VUOPT",6); break;
	case 'X': strncpy(kt->rname,"Other",6); break;
	case 'H': strncpy(kt->rname,"HIST",6);  break;
	default:
		printf("Error, unrecognized RKEY '%s'\n",p);
		strncpy(kt->rname,"Bad",6); 
	}
	//validity checks
// 2. EFISX  are letter followed by number
//    EVEN, FAM, INDI, SOUR, Other
//    R,N are letter followed by A-Za-z0-9
//       well really N is [A-Z]{3}[0-9]{3}
//    REFN Name
//    V is VUOPT
//    H is HIST[VC]
	switch (*p) {
	case 'E': 
	case 'F': 
	case 'I':
	case 'S':
	case 'X': 
		while (*++p && isdigit(*p));
		if (*p != 0 ) {
			printf("Error, unexpected char in RKEY(%s) \n", kt->rkey);
		}
		break;
	case 'N': // // 3c. N  6 chars long, N[A-Z$][A-Z][0-9]{3,3}
		if (strlen(p) != 6) {
			printf("Error, Name RKEY isn't 6 characters long, %s\n",kt->rkey);
			break;
		}
		if (!(*++p && (isupper(*p) || *p == '$'))) {
			printf("Error, Name RKEY has invalid 2nd Character, %s\n",kt->rkey);
			break;
		}
		if (!(*++p && (isupper(*p)))) {
			printf("Error, Name RKEY has invalid 3nd Character, %s\n",kt->rkey);
			break;
		}
		while (*++p) {
			if (!isdigit(*p)) {
				printf("Error, Name RKEY has invalid Character, %s\n",kt->rkey);
			}
		}
		break;
	case 'R': 
		// see refns.c R's are R char char or R char  2/3 char only
		// see refn2rkey in refns.c
		// each Rxx key is a table of all the REFN values starting
		// with xx. (so '1 REFN German Born' is under key RGe.)
		// Note '1 REFN Q Royal92-I1' results in key 'RQ '
		// so 2nd char can be a space for R keys
		if (strlen(p) > 3) {
			printf("Error, REFN RKEY more than 3 characters long, %s\n",kt->rkey);
		}
		while (*++p && isalnum(*p)) ;
		if (p < &kt->rkey[8] && *p != 0) {
			// we already flagged null in 1st 8 chars so don't report it here
			if (*p != ' ') {
				printf("Error, REFN RKEY has invalid character, %s\n",kt->rkey);
                        }
		}
		break;
	case 'V': 
		if (strcmp(p,"VUOPT") != 0) {
			printf("Error, invalid letter in RKEY(%s)\n",kt->rkey);
		}
		break;
	case 'H':
		if (strcmp(p,"HISTV") != 0 && strcmp(p,"HISTC") != 0) {
			printf("Error, invalid letter in RKEY(%s)\n",kt->rkey);
		}
		break;
	default:
		printf("Error, unrecognized RKEY %c\n",*p);
		strncpy(kt->rname,"Bad",6); 
	}
}
/*===============================================
 * dump_index -- open and print index to stdout
 *=============================================*/
void dump_index(STRING dir)
{
	if (strcmp(dir, bbasedir(BTR)) != 0) {
		printf("Error, mismatch in btree file names, %s and %s\n",
			dir, bbasedir(BTR));
	}

	traverse_index_blocks(BTR, bmaster(BTR), NULL, tf_print_index, NULL);

	return;
}
/*===============================
 * tf_print_index -- traversal function wrapper for print_index
 *=============================*/
BOOLEAN tf_print_index(HINT_PARAM_UNUSED BTREE btree, INDEX index, HINT_PARAM_UNUSED void *param)
{
	INT32 offset = 0;

	print_index(index, &offset);
	return TRUE;
}
/*===============================
 * print_index -- print INDEX to stdout
 *=============================*/
void print_index(INDEX index, INT32 *offset)
{
	INT n;
	INT32 size = 0;
	INT NRcount = 0;

	printf("\n");
	/* Step 1: Get length of file */
	size = (INT32)getfilesize(dbname, fkey2path(ixself(index)));

	/* Step 2: Print INDEX directory */
	printf("INDEX - DIRECTORY %s\n",fkey2path(ixself(index)));
	printf(FMT_INT32_HEX ":ix_self:   " FMT_INT32_HEX " (%s)\n", *offset,
		ixself(index), fkey2path(ixself(index)));
	*offset += sizeof(ixself(index));

	printf(FMT_INT32_HEX ":ix_type:   " FMT_INT32 " (%s)\n", *offset,
		ixtype(index),
		(ixtype(index) == 1 ? "INDEX" : (ixtype(index) == 2 ? "BLOCK" : "UNKNOWN")));
	*offset += sizeof(ixtype(index));

#if __WORDSIZE != 16
	printf(FMT_INT32_HEX ":ix_pad1:   " FMT_INT16_HEX "\n", *offset, index->ix_pad1);
	*offset += sizeof(index->ix_pad1);
#endif

	printf(FMT_INT32_HEX ":ix_parent: " FMT_INT32_HEX " (%s)\n", *offset,
		ixparent(index), fkey2path(ixparent(index)));
	*offset += sizeof(ixparent(index));

	printf(FMT_INT32_HEX ":ix_nkeys:  " FMT_INT16 "\n", *offset, nkeys(index));
	*offset += sizeof(nkeys(index));
	NRcount = nkeys(index);

	//print data in 2 column  rkey,pad2 & fkey  
	// the 0th entries are a backstop for btree searching
	// regular key entries are 1..ix_nkeys
	// There are a total of NOENTS (340) entries.
	// INDEX Directory is 4096 bytes in size
	INT32 offfkey = *offset + sizeof(index->ix_rkeys)+2;
	for (n=0; n<NOENTS; n++) {
		if (n == NRcount+1) {
			if (n < NOENTS - 1) {
				printf("\ndeleted/unused entries\n");
			}
			if (summarize) break;
		}
		check_rkey(&akey,&rkeys(index,n),FALSE);
		printf(FMT_INT32_HEX_06 ":ix_rkeys[" FMT_INT_04 "]:'%-8.8s'  ", 
				*offset, n, akey.rkey);
		*offset += sizeof(rkeys(index,n));
		printf(FMT_INT32_HEX_06 ":ix_fkeys[" FMT_INT_04 "]:" 
			FMT_INT32_HEX "(%s)\n", 
			offfkey, n, fkeys(index,n), fkey2path(fkeys(index,n)));
		offfkey += sizeof(fkeys(index,n));
	}
	// process unused entries
	if (summarize) {
		printf(FMT_INT32_HEX "-" FMT_INT32_HEX 
			":ix_rkeys[" FMT_INT_04 "-" FMT_INT_04 
			"] default value 0x0000000000000000\n",
			*offset,
			*offset+(INT32)(sizeof(rkeys(index,n))*(NOENTS-NRcount-1)- 1),
			NRcount+1,(INT)NOENTS-1);
		printf(FMT_INT32_HEX "-" FMT_INT32_HEX 
			":ix_fkeys[" FMT_INT_04 "-" FMT_INT_04 
			"] default value 0x00000000\n",
			offfkey,
			offfkey+(INT32)(sizeof(fkeys(index,0))*(NOENTS-NRcount-1) - 1),
			NRcount+1,(INT)NOENTS-1);
		// now check that they really are zero'ed
		int found_deleted = 0;
		for (n=NRcount+1; n<NOENTS; n++) {
			if (!RKEY_IS_NULL(rkeys(index,n)) || fkeys(index,n) != 0) {
				if (found_deleted++ == 0) { 
					printf("\ndeleted index rkey/fkey pairs\n");
				}
				if (!RKEY_IS_NULL(rkeys(index,n))) {
					check_rkey(&akey,&rkeys(index,n),FALSE);
					printf(FMT_INT32_HEX_06 ":ix_rkeys[" FMT_INT_04 "]:'%-8.8s'  ", 
						*offset, n, akey.rkey);
				} else {
					// hack a value to represent a zero filled key
					printf(FMT_INT32_HEX_06 ":ix_rkeys[" FMT_INT_04 "]:'%-8.8s'  ", 
						*offset, n, "8 x 0x00");
				}
				printf(FMT_INT32_HEX_06 ":ix_fkeys[" FMT_INT_04 "]:" 
					FMT_INT32_HEX "(%s)\n", 
					offfkey, n, fkeys(index,n), fkey2path(fkeys(index,n)));
			}
			*offset += sizeof(rkeys(index,0));
			offfkey += sizeof(fkeys(index,0));
		}
	}

#if __WORDSIZE != 16
	printf(FMT_INT32_HEX ":ix_pad2: " FMT_INT16_HEX "\n", *offset, index->ix_pad2);
	*offset += sizeof(index->ix_pad2);
#endif
	*offset += sizeof(index->ix_fkeys);
	printf(FMT_INT32_HEX ": EOF (" FMT_INT32_HEX ") %s\n", *offset, size, (*offset == size) ? "GOOD" : "BAD");
}
/*===============================================
 * dump_block -- open and print block to stdout
 *=============================================*/
void dump_block(STRING dir)
{
	if (strcmp(dir, bbasedir(BTR)) != 0) {
		printf("Error, mismatch in btree file names, %s and %s\n",
			dir, bbasedir(BTR));
	}

	traverse_index_blocks(BTR, bmaster(BTR), NULL, NULL, tf_print_block);

	printf("\n\nSummary of data entries by key type (includes deleted entries)\n");
	printf("Deleted Items remain in the database. ISEXF records are marked\n");
	printf("as deleted in xrefs file, soundex entries (N) has a record\n");
	printf("but the count of INDI's with this soundex value is 0.\n");
	char c;
	for (c = 'A'; c <= 'Z'; c++) {
	    if (lead_char[c - 'A'] != 0) {
		printf( "   %c " FMT_INT_6 "\n",c,lead_char[c - 'A']);
	    }
	}
	return;
}
/*===============================
 * tf_print_block -- traversal function wrapper for print_block
 *=============================*/
BOOLEAN tf_print_block(HINT_PARAM_UNUSED BTREE btree, BLOCK block, HINT_PARAM_UNUSED void *param)
{
	INT32 offset = 0;

	print_block(btree, block, &offset);
	return TRUE;
}

/*===============================
 * print_block -- print BLOCK to stdout
 *=============================*/
void print_block(BTREE btree, BLOCK block, INT32 *offset)
{
	INT n;
	INT32 size=0;
	INT NRcount = 0;

	printf("\n");
	/* Step 1: Get length of file */
	size = (INT32)getfilesize(dbname, fkey2path(ixself(block)));

	/* Step 2: Dump BLOCK structure */
	printf("BLOCK - DIRECTORY %s\n",fkey2path(ixself(block)));
	// BLOCK header
	printf(FMT_INT32_HEX ":ix_self:   " FMT_INT32_HEX " (%s)\n", *offset, 
		ixself(block), fkey2path(ixself(block)));
	*offset += sizeof(ixself(block));

	printf(FMT_INT32_HEX ":ix_type:   " FMT_INT32 " (%s)\n", *offset, 
		ixtype(block), (ixtype(block) == 1 ? "INDEX" : 
		(ixtype(block) == 2 ? "BLOCK" : "UNKNOWN")));
	*offset += sizeof(ixtype(block));

#if __WORDSIZE != 16
	printf(FMT_INT32_HEX ":ix_pad1:   " FMT_INT16_HEX "\n",
		*offset, block->ix_pad1);
	*offset += sizeof(block->ix_pad1);
#endif
	printf(FMT_INT32_HEX ":ix_parent: " FMT_INT32_HEX " (%s)\n",
		*offset, ixparent(block), fkey2path(ixparent(block)));
	*offset += sizeof(ixparent(block));

	NRcount = nkeys(block);
	printf(FMT_INT32_HEX ":ix_nkeys:  " FMT_INT "\n", *offset, NRcount);
	*offset += sizeof(nkeys(block));

	// print out block Directory Keys
	// as opposed to index Directory for block Directory 
	// there are ix_nkeys entries, indexed 0..ix_nkeys-1; total of 
	// There are a total of NORECS (255) entries.
	// BLOCK Directory is 4096 bytes in size 
	for (n=0; n<NORECS; n++) {
		if (n == NRcount) {
			if (n < NORECS - 1) {
				printf("\ndeleted/unused entries\n");
			}
			if (summarize) break;
		}
		check_rkey(&akey,&rkeys(block,n),FALSE);
		printf(FMT_INT32_HEX ":ix_rkey[" FMT_INT_04 "]: '%-8.8s'\n",
			*offset, n, akey.rkey);
		*offset += sizeof(rkeys(block,n));
	}
	if (summarize) {
		if (NRcount != NORECS) {
			printf(FMT_INT32_HEX "-" FMT_INT32_HEX 
				":ix_rkey[" FMT_INT_04 "-" FMT_INT_04 
				"] default value 0x0000000000000000\n",
				*offset,
				*offset + (INT32)((NORECS-NRcount)*sizeof(rkeys(block,0)) - 1),
				NRcount,(INT)NORECS-1);
			for (n=NRcount; n<NORECS; n++) {
				if (!RKEY_IS_NULL(rkeys(block,n))) {
					INT64 v;
					RKEY_AS_INT64(rkeys(block,n), v);
					printf(FMT_INT32_HEX ":ix_rkey[" FMT_INT_04 "]:"
						FMT_INT64_HEX " value not zero\n",
						*offset, n, v);
				}
				*offset += sizeof(rkeys(block,0));
			}
		}
	}

#if __WORDSIZE != 16
	printf(FMT_INT32_HEX ":ix_pad2: " FMT_INT16_HEX "\n",
		*offset, block->ix_pad2);
	*offset += sizeof(block->ix_pad2);
#endif

	// print out ix_offs and ix_lens values in directory
	// technically size of ix_offs and ix_lens are same but 
	// for clarity treat them as not same

	// first, print the values for valid keys (0 to NRcount) 
	INT32 offlens = *offset + sizeof(block->ix_offs);
	for (n=0; n<NRcount; n++) {
		printf(FMT_INT32_HEX ":ix_offs[" FMT_INT_04 "]: " 
			FMT_INT32_HEX , *offset, n, offs(block,n));
		printf("  " FMT_INT32_HEX ":ix_lens[" FMT_INT_04 "]: " 
			FMT_INT32_HEX "\n", offlens,
			n, lens(block,n));
		*offset += sizeof(offs(block,n));
		offlens += sizeof(lens(block,n));
	}
	// next skip the unused value (NRcount to NORECS)
	// print a summary message that all values should be 0
	// then check to make sure and print exceptions
	printf("\n");
	printf(FMT_INT32_HEX "-" FMT_INT32_HEX 
		":ix_offs[" FMT_INT_04 "-" FMT_INT_04 
		"] default value 0x00000000\n",
		*offset,
		*offset+ (INT32)(sizeof(offs(block,n))*(NORECS-NRcount)- 1),
		NRcount,(INT)NORECS-1);
	printf(FMT_INT32_HEX "-" FMT_INT32_HEX 
		":ix_lens[" FMT_INT_04 "-" FMT_INT_04 
		"] default value 0x00000000\n",
		offlens,
		offlens + (INT32)(sizeof(lens(block,0))*(NORECS-NRcount) - 1),
		NRcount,(INT)NORECS-1);
	// check for non=zero data
	for (n=NRcount; n<NORECS; n++) {
		if (offs(block,n) != 0) {
			printf(FMT_INT32_HEX ":ix_offs[" FMT_INT_04 "]: "
				FMT_INT32_HEX " value not zero\n",
				*offset, n, offs(block,n));
		}
		if (lens(block,n) != 0) {
			printf(FMT_INT32_HEX ":ix_lens[" FMT_INT_04 "]: "
				FMT_INT32_HEX " value not zero\n",
				offlens, n, lens(block,n));
		}
		*offset += sizeof(offs(block,0));
		offlens += sizeof(lens(block,0));
	}
	*offset = offlens;
	printf("\n");

	/* Step 3: Dump BLOCK data */
	/* Note that this mechanism is horribly inefficient from an I/O perspective. */
	printf("BLOCK - DATA\n");
	for (n=0; n<NRcount; n++) {
		INT len;
		INT32 roff = offs(block, n);
		INT32 rlen = lens(block,n);
		if (roff == 0 && rlen == 0 && RKEY_IS_NULL(rkeys(block,n))) {
			printf("[" FMT_INT "], found unexpected uninitialized key\n",
				n );
			continue;  // blank entry skip it.
		}
		RAWRECORD rec = readrec(btree, block, n, &len);
		INT32 roffnxt = (n < NORECS - 1 && offs(block,n+1)!=0) ?
			offs(block,n+1)+BUFLEN : size ;
		if (*offset + rlen > roffnxt) {
			printf("Error, record " FMT_INT " longer than available data\n",n);
			printf("offset=" FMT_INT32_HEX
				", +rlen(" FMT_INT32_HEX ")n=" 
				FMT_INT32_HEX ", roffnxt=" FMT_INT32_HEX "\n",
				*offset, rlen, *offset+rlen,roffnxt);
		} else if (*offset + rlen < roffnxt) {
			if (offs(block,n+1) == 0) {
				printf("Warning, last record record " FMT_INT
					" does not fill rest of block\n",n);
				printf("cur offset = " FMT_INT32_HEX
					", remaining = " FMT_INT32_HEX "\n",
					*offset, size - *offset-rlen);
			} else {
				printf("Error, record " FMT_INT 
					" shorter than available data\n",n);
				printf("cur offset = " FMT_INT32_HEX
					", remaining = " FMT_INT32_HEX "\n",
					*offset, size - *offset-rlen);
			}
		}
		check_rkey(&akey,&rkeys(block,n),TRUE);
		/* keys start with I,F,S,E,X and N R V H */
		printf("[" FMT_INT_04 "] %s rkey: %s offs: " FMT_INT32_HEX
			" lens: " FMT_INT32_HEX "\n",
			n,akey.rname, akey.rkeyfirst,
			(INT32)offs(block,n), (INT32)lens(block,n));
		if (*akey.rkeyfirst ==  'N' || *akey.rkeyfirst == 'R') {
			// name / REFN data consists of
			// INT16 nnames : number of names
			// RKEY[nnames] : INDI keys of the name/REFN
			// offset[nnames] : offset to the name
			//
			// col1 and col2 are offsets into rec
			// so *offset is kept frozen till end
			// we reuse akey buffer to check rkeys in sublists
			//    so save first letter of this record's key
			char first = *akey.rkeyfirst;
			INT32 Ncount = EXTRACT_INT32(&rec[0]);
			INT32 col1 = sizeof(INT32);
			INT32 col2 = col1 + Ncount * sizeof(RKEY);
			INT32 lennames = 0;
			INT m;
			// print out Keys/string offsets
			printf("   " FMT_INT32_HEX ": Ncount " FMT_INT32 "\n",
				*offset,Ncount);
			printf("      Keys and string offsets, strings\n");
			INT strbase = sizeof(INT32) + (sizeof(RKEY) +
				sizeof(INT32))*Ncount;
			for(m = 0; m < Ncount; m++) {
				INT32 recoff = EXTRACT_INT32(&rec[col2]);
				INT32 stroff = strbase + recoff;
				printf("    " FMT_INT_3 ". " FMT_INT32_HEX ":RKEY %8.8s "
				    FMT_INT32_HEX ":offset " FMT_INT32_HEX "\n",
					m+1, *offset + col1, &rec[col1],
					     *offset + col2, recoff);
				printf("         " FMT_INT32_HEX ":string '%s'\n",
					*offset+stroff, &rec[stroff]);
				col1 += sizeof(RKEY);
				col2 += sizeof(INT32);
			}
			// Now rescan data and assemble the data
			col1 = sizeof(INT32);
			col2 = col1 + Ncount*sizeof(RKEY); 
			printf("      Assembled data for %s\n",akey.rkeyfirst);
			int keylen = 0;
			for(m = 0; m < Ncount; m++) {
				check_rkey(&akey,(RKEY *)&rec[col1],FALSE);
				INT l = strlen(akey.rkeyfirst);
				if (keylen < l) keylen = l;
				col1 += sizeof(RKEY);
			}
			col1 = sizeof(INT32);
			for(m = 0; m < Ncount; m++) {
				INT stroff = strbase + EXTRACT_INT32(&rec[col2]);

				//  print keys,strings
				char *p = &rec[col1];
				while (*p == ' ') ++p;
				check_rkey(&akey,(RKEY *)&rec[col1],FALSE);
				printf("    " FMT_INT_3 ". %-*s %s %s%s\n" ,
					m+1, keylen, akey.rkeyfirst,
					(first == 'N' ? " name " :
					" identified by 'REFN "), &rec[stroff],
					(first == 'N' ? "" : "'"));
				lennames += strlen(&rec[stroff])+1;
				col1 += sizeof(RKEY);
				col2 += sizeof(INT32);
			}
		} else if (*akey.rkeyfirst == 'H') {
			// HIST data consists of
			// INT16 count : number of history records
			// INT16 count : number of history records (again)
			// .. foreach ..
			// INT16 ntype : node type (from char)
			// INT16 nkey  : node key (from INT)
			//
			// See save_nkey_list and load_nkey_list
			//
			INT32 *ptr = CASTPTR_INT32(&rec[0]);
			INT32 count1 = *ptr++;
			INT32 count2 = *ptr++;
			INT32 count = ((count1>count2) ? count2 : count1);
			INT i;

			// print out count
			printf("   " FMT_INT32_HEX ": count1 " FMT_INT32 "\n",
				*offset,count1);
			printf("   " FMT_INT32_HEX ": count1 " FMT_INT32 "\n",
				*offset+(INT32)sizeof(INT32),count2);

			// print out entries
			for (i=0; i<count; i++)
			{
				INT32 type = *ptr++;
				INT32 offset1 = (i+1)*8;
				INT32 num = *ptr++;
				INT32 offset2 = (i+1)*8 + 4;
				printf("    " FMT_INT_3 ". " FMT_INT32_HEX ":type '%c' "
				                             FMT_INT32_HEX ":keynum " FMT_INT "\n",
					i, *offset + offset1, (char)type,
					   *offset + offset2, (INT)num);
			}
		} else {
			// handle all but N,R,H - E F I P S V X
			// these have just text data
			if (rec != NULL)
				printf(FMT_INT32_HEX "-" FMT_INT32_HEX 
					":\n>>%s<<\n", 
					BUFLEN+roff,BUFLEN+roff+lens(block,n)-1, rec);
			else
				printf(">> ??not text?? <<\n");
		} 
		*offset += lens(block,n);
		stdfree(rec);
	}
	printf("\n");

	printf(FMT_INT32_HEX ": EOF (" FMT_INT32_HEX ") %s\n",
		*offset, size, (*offset == size) ? "GOOD" : "BAD");
}
/*===============================
 * dump_keyfile -- open and print KEYFILE1 and KEYFILE2 to stdout
 *=============================*/
void dump_keyfile(STRING dir)
{
	char scratch[MAXPATHLEN];
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	FILE *fk;
	size_t size;

	snprintf(scratch, sizeof(scratch), "%s/key", dir);

	size = getfilesize(dir, "key");

	if (!(fk = fopen(scratch, LLREADBINARY))) {
		printf("Error opening keyfile (%d: %s)\n", errno, scratch);
		goto error2;
	}

	printf("KEYFILE key\n");
	if (size != sizeof(kfile1) &&
		size != (sizeof(kfile1) + sizeof(kfile2))) {
		printf("Error: keyfile size invalid (" FMT_SIZET 
			"), valid sizes are " FMT_SIZET " and " FMT_SIZET "\n",
			size, sizeof(kfile1), sizeof(kfile1)+sizeof(kfile2));
		goto error1;
	}

	if (fread(&kfile1, sizeof(kfile1), 1, fk) != 1) {
		printf("Error reading keyfile (%d: %s)\n", errno, scratch);
		goto error1;
	}

	/* only attempt to read kfile2 if the file size indicates that it is present */
	if (size != sizeof(kfile1))
	{
		if (fread(&kfile2, sizeof(kfile2), 1, fk) != 1) {
			printf("Error reading keyfile (%d: %s)\n", errno, scratch);
			goto error1;
		}

		print_keyfile(&kfile1, &kfile2, size);
	}
	else
	{
		print_keyfile(&kfile1, NULL, size);
	}

error1:
	fclose(fk);
error2:
	return;
}
/*===============================
 * print_keyfile -- print KEYFILE1 and KEYFILE2 to stdout
 *=============================*/
void print_keyfile(KEYFILE1* kfile1, KEYFILE2* kfile2, INT32 size)
{
	INT32 offset = 0;

	printf("KEYFILE1\n");
	printf("========\n");

	printf(FMT_INT16_HEX ":mkey:  " FMT_INT32_HEX " (%s)\n", offset, 
		kfile1->k_mkey, fkey2path(kfile1->k_mkey));
	offset += sizeof(kfile1->k_mkey);

	printf(FMT_INT16_HEX ":fkey:  " FMT_INT32_HEX " (%s)\n", offset,
		kfile1->k_fkey, fkey2path(kfile1->k_fkey));
	offset += sizeof(kfile1->k_fkey);

	printf(FMT_INT16_HEX ":ostat: " FMT_INT32_HEX " (%d)\n", offset,
		kfile1->k_ostat, kfile1->k_ostat);
	offset += sizeof(kfile1->k_ostat);

	printf("\n");

	if (kfile2)
	{
		printf("KEYFILE2\n");
		printf("========\n");

		printf(FMT_INT16_HEX ":name:	 '%-18.18s'\n", offset,
			kfile2->name);
		offset += sizeof(kfile2->name);
#if WORDSIZE != 16
		printf(FMT_INT16_HEX ":pad1:	 " FMT_INT16_HEX "\n", offset,
			kfile2->pad1);
		offset += sizeof(kfile2->pad1);
#endif
		printf(FMT_INT16_HEX ":magic:	 " FMT_INT32_HEX "\n", offset, kfile2->magic);
		offset += sizeof(kfile2->magic);

		printf(FMT_INT16_HEX ":version:  " FMT_INT32_HEX " (%d)\n", 
			offset, kfile2->version, kfile2->version);
		offset += sizeof(kfile2->version);
	}

	printf(FMT_INT32_HEX ": EOF (" FMT_INT32_HEX ") %s\n", offset,
		size, (offset == size) ? "GOOD" : "BAD");
	printf("\n");
}
/*===============================
 * dump_xref -- open and print xrefs to stdout
 *=============================*/
void dump_xref(STRING dir)
{
	if (strcmp(dir, bbasedir(BTR)) != 0) {
		printf("Error, mismatch in btree file names, %s and %s\n",
			dir, bbasedir(BTR));
	}

	if (!openxref(FALSE))
	{
		printf("Error opening/reading xrefs\n");
		goto error1;
	}

	print_xrefs();

error1:
	closexref();
	return;
}
/*===============================
 * print_xrefs -- print xrefs to stdout
 *=============================*/
void print_xrefs(void)
{
	printf("XREFFILE xrefs\n");
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
/*===============================
 * getfilesize -- Get length of file
 *=============================*/
size_t
getfilesize (STRING dir, STRING filename)
{
	FILE *fp;
	size_t size = 0;
	char scratch[MAXPATHLEN];
	struct stat sbuf;

	snprintf(scratch, sizeof(scratch), "%s/%s", dir, filename);
	if (stat(scratch, &sbuf) || !S_ISREG(sbuf.st_mode)) {
		printf("Invalid filename (%d: %s)\n", errno, scratch);
		goto error2;
	}

	if (!(fp = fopen(scratch, LLREADBINARY))) {
		printf("Error opening file (%d: %s)\n", errno, scratch);
		goto error2;
	}

	if (fseek(fp, 0, SEEK_END)) {
		printf("Error seeking to end of file\n");
		goto error1;
	}

	size = ftell(fp);

error1:
	fclose(fp);

error2:
	return size;
}
