/* 
   Copyright (c) 2000 Perry Rapp

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
 * dbverify.c -- Database check & repair facility for LifeLines
 * Copyright(c) 2000 by Perry Rapp; all rights reserved
 *
 * General notes:
 *  classify_nodes ignores duplicates, so we probably ? shouldn't
 *   see any duplicates
 *===============================================================*/

#include <stdarg.h>
#include "sys_inc.h"
#include "standard.h"
#include "llstdlib.h"
#include "gedcom.h"
#include "btree.h"
#include "indiseq.h"


/*********************************************
 * external variables (no header)
 *********************************************/

extern BTREE BTR;

/*********************************************
 * required global variables
 *********************************************/

BOOLEAN selftest = FALSE; /* selftest rules (ignore paths) */
int opt_finnish = 0;

/*********************************************
 * local types
 *********************************************/

/*==========================================
 * errinfo -- holds stats on a type of error
 *========================================*/
struct errinfo {
	INT err;
	INT err_count;
	INT fix_count;
	STRING desc;
};
/*==================================
 * work -- what the user wants to do
 *================================*/
struct work {
	INT find_ghosts;
	INT fix_ghosts;
	INT check_indis;
	INT fix_indis;
	INT check_fams;
	INT fix_fams;
	INT check_sours;
	INT fix_sours;
	INT check_evens;
	INT fix_evens;
	INT check_othes;
	INT fix_othes;
};
/*=======================================
 * NAMEREFN_REC -- holds one name or refn
 *  key is the target pointed to
 *=====================================*/
typedef struct
{
	STRING namerefn; 
	STRING key;
	INT err;
} NAMEREFN_REC;

/*********************************************
 * local function prototypes
 *********************************************/

static void print_usage(void);
static void report_error(INT err, STRING fmt, ...);
static void report_progress(STRING fmt, ...);
static NAMEREFN_REC * alloc_namerefn(STRING namerefn, STRING key, INT err);
static void free_namerefn(NAMEREFN_REC * rec);
static void check_ghosts(void);
static BOOLEAN cgn_callback(STRING key, STRING name, BOOLEAN newset, void *param);
static BOOLEAN cgr_callback(STRING key, STRING refn, BOOLEAN newset, void *param);
static void finish_and_delete_nameset(INDISEQ seq);
static void finish_and_delete_refnset(INDISEQ seq);
static void check_nodes(void);
static BOOLEAN nodes_callback(STRING key, NOD0 nod0, void *param);
static void check_set(INDISEQ seq, char ctype);
static BOOLEAN check_indi(STRING key, NOD0 nod0);
static BOOLEAN check_fam(STRING key, NOD0 nod0);
static BOOLEAN check_sour(STRING key, NOD0 nod0);
static BOOLEAN check_even(STRING key, NOD0 nod0);
static BOOLEAN check_othe(STRING key, NOD0 nod0);
static void validate_errs(void);
static void report_results(void);

/*********************************************
 * local variables
 *********************************************/

#define ERR_ORPHANNAME 0
#define ERR_GHOSTNAME 1
#define ERR_DUPNAME 2
#define ERR_NONINDINAME 3
#define ERR_DUPINDI 4
#define ERR_DUPFAM 5
#define ERR_DUPSOUR 6
#define ERR_DUPEVEN 7
#define ERR_DUPOTHE 8
#define ERR_MISSING 9
#define ERR_DELETED 10
#define ERR_BADNAME 11
#define ERR_BADFAMREF 12

static struct errinfo errs[] = {
	{ ERR_ORPHANNAME, 0, 0, "Orphan names" }
	, { ERR_GHOSTNAME, 0, 0, "Ghost names" }
	, { ERR_DUPNAME, 0, 0, "Duplicate names" }
	, { ERR_NONINDINAME, 0, 0, "Non-indi names" }
	, { ERR_DUPINDI, 0, 0, "Duplicate individuals" }
	, { ERR_DUPFAM, 0, 0, "Duplicate families" }
	, { ERR_DUPSOUR, 0, 0, "Duplicate sources" }
	, { ERR_DUPEVEN, 0, 0, "Duplicate events" }
	, { ERR_DUPOTHE, 0, 0, "Duplicate others" }
	, { ERR_MISSING, 0, 0, "Missing records" }
	, { ERR_DELETED, 0, 0, "Deleted records" }
	, { ERR_BADNAME, 0, 0, "Bad name" }
	, { ERR_BADFAMREF, 0, 0, "Bad family reference" }
};
static struct work todo;
static LIST tofix;
static INDISEQ dupseq;
static BOOLEAN noisy=FALSE;
static INDISEQ seq_indis, seq_fams, seq_sours, seq_evens, seq_othes;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=================================
 * print_usage -- Explain arguments
 *===============================*/
static void
print_usage (void)
{
	printf(
		"usage: dbverify -(flags) <btree>\n"
		"flags:\n"
		"\t-g = Check for ghosts (names/refns)\n"
		"\t-G = Check for & fix ghosts (names/refns)\n"
		"\t-i = Check individuals\n"
		"\t-f = Check families\n"
		"\t-s = Check sours\n"
		"\t-e = Check events\n"
		"\t-x = Check others\n"
		"\t-n = Noisy (echo every record processed)\n"
		);
}
/*========================================
 * report_error -- report some error found
 *======================================*/
static void
report_error (INT err, STRING fmt, ...)
{
	va_list args;
	errs[err].err_count++;
	va_start(args, fmt);
	printf("! ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}
/*=========================================
 * report_progress -- report current record
 *=======================================*/
static void
report_progress (STRING fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf(" [");
	vprintf(fmt, args);
	printf("]");
	printf("\n");
	va_end(args);
}
/*===============================================
 * alloc_namerefn -- allocates a new NAMEREFN_REC
 *=============================================*/
static NAMEREFN_REC *
alloc_namerefn (STRING namerefn, STRING key, INT err)
{
	NAMEREFN_REC * rec = (NAMEREFN_REC *)stdalloc(sizeof(*rec));
	rec->namerefn = strsave(namerefn);
	rec->key = strsave(key);
	rec->err = err;
	return rec;
}
/*==========================================
 * free_namerefn -- frees a new NAMEREFN_REC
 *========================================*/
static void
free_namerefn (NAMEREFN_REC * rec)
{
	stdfree(rec->namerefn);
	stdfree(rec->key);
	stdfree(rec);
}
/*==========================================
 * check_ghosts -- Process all names & refns
 *  checking (& optionally fixing) ghosts
 *=========================================*/
static void
check_ghosts (void)
{
	tofix = create_list();
	dupseq = create_indiseq_null();
	/* dupseq is used inside cgn_callback, across calls */
	traverse_names(cgn_callback, NULL);
	finish_and_delete_nameset(dupseq);
	dupseq = NULL;

	if (todo.fix_ghosts) {
		NAMEREFN_REC * rec;
		while (!empty_list(tofix)) {
			rec = (NAMEREFN_REC *) dequeue_list(tofix);
			remove_name(rec->namerefn, rec->key);
			errs[rec->err].fix_count++;
			free_namerefn(rec);
		}
	}

	ASSERT(empty_list(tofix) && !dupseq);

	dupseq = create_indiseq_null();
	/* dupseq is used inside cgr_callback, across calls */
	traverse_refns(cgr_callback, NULL);
	finish_and_delete_refnset(dupseq);
	dupseq = NULL;

	if (todo.fix_ghosts) {
		NAMEREFN_REC * rec;
		while (!empty_list(tofix)) {
			rec = (NAMEREFN_REC *) dequeue_list(tofix);
			remove_refn(rec->namerefn, rec->key);
			free_namerefn(rec);
		}
	}
	
	remove_list(tofix, NULL); 
	tofix=0;
}
/*============================================
 * cgn_callback -- callback for name traversal
 *  for checking for ghost names
 *==========================================*/
static BOOLEAN
cgn_callback (STRING key, STRING name, BOOLEAN newset, void *param)
{
	/* a name record which points at indi=key */
	NOD0 indi0 = qkey_to_indi0(key);
	NODE indi = nztop(indi0);

	/* bail out immediately if not INDI */
	if (key[0] != 'I') {
		report_error(ERR_NONINDINAME, "Non-indi name, key=%s, name=%s", key, name);
		return 1; /* continue traversal */
	}

	if (newset) {
		finish_and_delete_nameset(dupseq);
		dupseq = create_indiseq_null();
	}

	append_indiseq_null(dupseq, strsave(key), name, TRUE, TRUE);

	if (!indi) {
		report_error(ERR_ORPHANNAME, "Orphaned name: %s", name);
		if (todo.fix_ghosts)
			enqueue_list(tofix, (VPTR)alloc_namerefn(name, key, ERR_ORPHANNAME));
	} else {
		NODE node;
		BOOLEAN found=FALSE;
		NODE nam, refn, sex, body, famc, fams;
		split_indi(indi, &nam, &refn, &sex, &body, &famc, &fams);
		for (node = nam; node; node = nsibling(node)) {
			if (!strcmp(nval(node), name))
				found=TRUE;
		}
		join_indi(indi, nam, refn, sex, body, famc, fams);
		if (!found) {
			report_error(ERR_GHOSTNAME, "Ghost name: %s -> %s", name, key);
			if (todo.fix_ghosts)
				enqueue_list(tofix, (VPTR)alloc_namerefn(name, key, ERR_GHOSTNAME));
		}
	}

	if (noisy)
		report_progress("Name: %s", name);

	return 1; /* continue traversal */
}
/*============================================
 * cgr_callback -- callback for refn traversal
 *  for checking for ghost refns
 *==========================================*/
static BOOLEAN
cgr_callback (STRING key, STRING refn, BOOLEAN newset, void *param)
{
	/* a refn record which points at nod0=key */
	NOD0 nod0 = key_to_typ0(key, TRUE);
	NODE node = nztop(nod0);

	if (newset) {
		finish_and_delete_refnset(dupseq);
		dupseq = create_indiseq_null();
	}
	append_indiseq_null(dupseq, strsave(key), refn, TRUE, TRUE);
	
	if (!node) {
		report_error(ERR_ORPHANNAME, "Orphaned refn: %s", refn);
		if (todo.fix_ghosts)
			enqueue_list(tofix, (VPTR)alloc_namerefn(refn, key, ERR_ORPHANNAME));
	} else {
	}
	if (noisy)
		report_progress("Refn: %s", refn);

	return 1; /* continue traversal */
}
/*=================================================================
 * finish_and_delete_nameset -- check for dups in a set of one name
 *===============================================================*/
static void
finish_and_delete_nameset (INDISEQ seq)
{
	char prevkey[8];
	prevkey[0]=0;
	keysort_indiseq(seq);
	FORINDISEQ(seq, el, num)
		if (!strcmp(skey(el), prevkey)) {
			report_error(ERR_DUPNAME, "Duplicate name for %s (%s)", prevkey, snam(el));
		}
		strcpy(prevkey, skey(el));
	ENDINDISEQ
	remove_indiseq(seq, FALSE);
}
/*=================================================================
 * finish_and_delete_refnset -- check for dups in a set of one refn
 *===============================================================*/
static void
finish_and_delete_refnset (INDISEQ seq)
{
	char prevkey[8];
	prevkey[0]=0;
	canonkeysort_indiseq(seq);
	FORINDISEQ(seq, el, num)
		if (!strcmp(skey(el), prevkey)) {
			report_error(ERR_DUPNAME, "Duplicate refn for %s (%s)", prevkey, snam(el));
		}
		strcpy(prevkey, skey(el));
	ENDINDISEQ
	remove_indiseq(seq, FALSE);
}
/*=================================
 * check_nodes -- Process all nodes
 *  checking and/or fixing as requested
 *================================*/
static void
check_nodes (void)
{
	seq_indis = create_indiseq_null();
	seq_fams = create_indiseq_null();
	seq_sours = create_indiseq_null();
	seq_evens = create_indiseq_null();
	seq_othes = create_indiseq_null();
	traverse_db_key_nod0s(nodes_callback, NULL);
	/* check what we saw against delete sets */
	check_set(seq_indis, 'I');
	check_set(seq_fams, 'F');
	check_set(seq_sours, 'S');
	check_set(seq_evens, 'E');
	check_set(seq_othes, 'X');
	remove_indiseq(seq_indis, FALSE);
	remove_indiseq(seq_fams, FALSE);
	remove_indiseq(seq_sours, FALSE);
	remove_indiseq(seq_evens, FALSE);
	remove_indiseq(seq_othes, FALSE);
}
/*=============================================
 * nodes_callback -- callback for node traversal
 *  for checking indis, fams, sours, evens, othes
 *===========================================*/
static BOOLEAN
nodes_callback (STRING key, NOD0 nod0, void *param)
{
	if (noisy)
		report_progress("Node: %s", key);
	switch (key[0]) {
	case 'I': return todo.check_indis ? check_indi(key, nod0) : TRUE;
	case 'F': return todo.check_fams ? check_fam(key, nod0) : TRUE;
	case 'S': return todo.check_sours ? check_sour(key, nod0) : TRUE;
	case 'E': return todo.check_evens ? check_even(key, nod0) : TRUE;
	case 'X': return todo.check_othes ? check_othe(key, nod0) : TRUE;
	}
	ASSERT(0); /* traverse_db_key_nod0s is broken */
	return TRUE;
}
/*=====================================
 * check_indi -- process indi record
 *  checking and/or fixing as requested
 *===================================*/
static BOOLEAN
check_indi (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	NODE indi1, name1, refn1, sex1, body1, famc1, fams1;
	NODE node;
	INT keynum = atoi(&key[1]);
	CACHEEL icel;
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPINDI, "Duplicate individual for %s", key);
	}
	indi1 = nztop(nod0);
	icel = indi_to_cacheel(indi1);
	lock_cache(icel);
	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	for (node = name1; node; node = nsibling(node)) {
		STRING name=nval(node);
		if (!valid_name(name)) {
			report_error(ERR_BADNAME, "Bad name for individual %s: %s", key, name);
		}
		/* TO DO: verify that name is in db */
	}
	for (node = refn1; node; node = nsibling(node)) {
		STRING refn=nval(node);
		/* TO DO: verify that refn is in db */
	}
	for (node = famc1; node; node = nsibling(node)) {
		STRING famkey=rmvat(nval(node));
		NODE fam = qkey_to_fam(famkey);
		if (!fam) {
			report_error(ERR_BADFAMREF, "Bad family reference (%s) individual %s", famkey, key);
		}
		/* TO DO: verify that family lists indi */
	}
	/* TO DO: check lineage links */
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	/*
	TO DO: check pointers ?
	*/
	unlock_cache(icel);
	append_indiseq_null(seq_indis, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_fam -- process fam record
 *  checking and/or fixing as requested
 *===================================*/
static BOOLEAN
check_fam (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPFAM, "Duplicate family for %s", key);
	}
	/*
	split & check lineage links
	check for empty family
	check pointers ?
	*/
	append_indiseq_null(seq_fams, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_sour -- process sour record
 *  checking and/or fixing as requested
 *===================================*/
static BOOLEAN
check_sour (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPSOUR, "Duplicate source for %s", key);
	}
	/*
	check pointers ?
	*/
	append_indiseq_null(seq_sours, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_even -- process even record
 *  checking and/or fixing as requested
 *===================================*/
static BOOLEAN
check_even (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPEVEN, "Duplicate event for %s", key);
	}
	/*
	check pointers ?
	*/
	append_indiseq_null(seq_evens, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_othe -- process othe record
 *  checking and/or fixing as requested
 *===================================*/
static BOOLEAN
check_othe (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPOTHE, "Duplicate record for %s", key);
	}
	/*
	check pointers ?
	*/
	append_indiseq_null(seq_othes, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*===================================
 * check_set -- Validate set of nodes
 *=================================*/
static void
check_set (INDISEQ seq, char ctype)
{
	INT i=0;
	keysort_indiseq(seq); /* spri now valid */
	i = xref_next(ctype, i);
	FORINDISEQ(seq, el, num)
		while (i<spri(el)) {
			report_error(ERR_MISSING, "Missing undeleted record %c%d", ctype, i);
			i = xref_next(ctype, i);
		}
		if (i == spri(el)) {
			/* in synch */
			i = xref_next(ctype, i);
		} else { /* spri(el) < i */
			report_error(ERR_DELETED, "Delete set contains valid record %s", skey(el));
			/* fall thru and only advance seq */
		}
	ENDINDISEQ
}
/*=========================================
 * validate_errs -- Validate the errs array
 *=======================================*/
static void
validate_errs (void)
{
	INT i;
	for (i=0; i<sizeof(errs)/sizeof(errs[0]); i++) {
		if (errs[i].err != i) {
			fprintf(stderr, "Invalid errs array[%d] in dbverify - fix program\n", i);
			FATAL();
		}
	}
}
/*===========================================
 * main -- Main procedure of dbverify command
 *=========================================*/
int
main (int argc,
      char **argv)
{
	char *flags, *dbname;
	char *ptr;

	validate_errs();

#ifdef WIN32
	/* TO DO - research if this is necessary */
	_fmode = O_BINARY;	/* default to binary rather than TEXT mode */
#endif

	if (argc != 3 || argv[1][0] != '-' || argv[1][1] == '\0') {
		print_usage();
		return (1);
	}
	flags = argv[1];
	dbname = argv[2];
	for (ptr=&flags[1]; *ptr; ptr++) {
		switch(*ptr) {
		case 'g': todo.find_ghosts=TRUE; break;
		case 'G': todo.fix_ghosts=TRUE; break;
		case 'i': todo.check_indis=TRUE; break;
		case 'f': todo.check_fams=TRUE; break;
		case 's': todo.check_sours=TRUE; break;
		case 'e': todo.check_evens=TRUE; break;
		case 'x': todo.check_othes=TRUE; break;
		case 'n': noisy=TRUE; break;
		default: print_usage(); return (1); 
		}
	}
	if (!(BTR = openbtree(dbname, FALSE, TRUE))) {
		printf("could not open database: %s\n", dbname);
		return (1);
	}
	init_lifelines();
	printf("Checking %s\n", dbname);

	if (todo.find_ghosts || todo.fix_ghosts)
		check_ghosts();

	/* simplify successive logic */
	if (todo.fix_indis) todo.check_indis=TRUE;
	if (todo.fix_fams) todo.check_fams=TRUE;
	if (todo.fix_sours) todo.check_sours=TRUE;
	if (todo.fix_evens) todo.check_evens=TRUE;
	if (todo.fix_othes) todo.check_othes=TRUE;

	if (todo.check_indis
		|| todo.check_fams
		|| todo.check_sours
		|| todo.check_evens
		|| todo.check_othes)
		check_nodes();

	report_results();

	closebtree(BTR);
	return TRUE;
}
/*===============================================
 * report_results -- Print out error & fix counts
 *=============================================*/
static void
report_results (void)
{
	INT i, ct=0;
	for (i=0; i<sizeof(errs)/sizeof(errs[0]); i++) {
		if (errs[i].err_count || errs[i].fix_count) {
			ct++;
			printf("%s: %d errors, %d fixed\n", 
				errs[i].desc, errs[i].err_count, errs[i].fix_count);
		}
	}
	if (!ct) {
		printf("No errors found\n");
	}
}
/*=========================================================
 * __allocate -- Allocate memory - called by stdalloc macro
 *========================================================*/
void *
__allocate (int len,     /* number of bytes to allocate */
            STRING file, /* not used */
            int line)    /* not used */
{
	char *p;
	if ((p = malloc(len)) == NULL)  FATAL();
	return p;
}
/*=======================================================
 * __deallocate - Return memory - called by stdfree macro
 *=====================================================*/
void
__deallocate (void *ptr,  /* memory being returned */
              STRING file, /* not used */
              int line)   /* not used */
{
	free(ptr);
}
/*=============================
 * fatal -- Fatal error routine
 *===========================*/
void
__fatal (STRING file,
         int line)
{
	printf("FATAL ERROR: %s: line %d\n", file, line);
	exit(1);
}
