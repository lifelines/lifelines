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
 *===============================================================*/

#include <stdarg.h>
#include "sys_inc.h"
#include "standard.h"
#include "llstdlib.h"
#include "gedcom.h"
#include "btree.h"
#include "indiseq.h"

extern BTREE BTR;
BOOLEAN selftest = FALSE; /* selftest rules (ignore paths) */


int opt_finnish = 0;


struct errinfo {
	INT err;
	INT err_count;
	INT fix_count;
	STRING desc;
};

#define ERR_ORPHANNAME 0
#define ERR_GHOSTNAME 1
#define ERR_DUPNAME 2
#define ERR_NONINDINAME 3

static struct errinfo errs[] = {
	{ ERR_ORPHANNAME, 0, 0, "Orphan names" }
	, { ERR_GHOSTNAME, 0, 0, "Ghost names" }
	, { ERR_DUPNAME, 0, 0, "Duplicate names" }
	, { ERR_NONINDINAME, 0, 0, "Non-indi names" }
};


static LIST tofix;
static INDISEQ dupseq;

static BOOLEAN noisy=FALSE;

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
/*=================================================================
 * finish_and_delete_nameset -- check for dups in a set of one name
 *===============================================================*/
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
/*============================================
 * cgn_callback -- callback for name traversal
 *  for checking for ghost names
 *==========================================*/
static BOOLEAN
cgn_callback (STRING key, STRING name, BOOLEAN newset, void *param)
{
	/* a name record which points at indi=key */
	INT fixGhosts = (INT)param;
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
	append_indiseq_null(dupseq, strsave(key), NULL, TRUE, TRUE);

	if (!indi) {
		report_error(ERR_ORPHANNAME, "Orphaned name: %s", name);
		if (fixGhosts)
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
			if (fixGhosts)
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
	INT fixGhosts = (INT)param;
	NOD0 nod0 = key_to_typ0(key, TRUE);
	NODE node = nztop(nod0);

	if (newset) {
		finish_and_delete_refnset(dupseq);
		dupseq = create_indiseq_null();
	}
	append_indiseq_null(dupseq, strsave(key), NULL, TRUE, TRUE);
	
	if (!node) {
		report_error(ERR_ORPHANNAME, "Orphaned refn: %s", refn);
		if (fixGhosts)
			enqueue_list(tofix, (VPTR)alloc_namerefn(refn, key, ERR_ORPHANNAME));
	} else {
	}
	if (noisy)
		report_progress("Refn: %s", refn);

	return 1; /* continue traversal */
}
/*==========================================
 * check_ghosts -- Process all names & refns
 *  checking (& optionally fixing) ghosts
 *=========================================*/
static void
check_ghosts (INT fixGhosts)
{
	tofix = create_list();
	dupseq = create_indiseq_null();
	/* dupseq is used inside cgn_callback, across calls */
	traverse_names(cgn_callback, (void *)fixGhosts);
	finish_and_delete_nameset(dupseq);
	dupseq = NULL;

	if (fixGhosts) {
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
	traverse_refns(cgr_callback, (void *)fixGhosts);
	finish_and_delete_refnset(dupseq);
	dupseq = NULL;

	if (fixGhosts) {
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
/*=========================================
 * validate_errs -- Validate the errs array
 *=======================================*/
static void
validate_errs ()
{
	INT i;
	for (i=0; i<sizeof(errs)/sizeof(errs[0]); i++) {
		if (errs[i].err != i) {
			fprintf(stderr, "Invalid errs array[%d] in dbverify - fix program\n", i);
			FATAL();
		}
	}
}
/*===============================================
 * report_results -- Print out error & fix counts
 *=============================================*/
static void
report_results ()
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
/*===========================================
 * main -- Main procedure of dbverify command
 *=========================================*/
int
main (int argc,
      char **argv)
{
	char *flags, *dbname;
	char *ptr;
	INT findGhosts=FALSE, fixGhosts=FALSE;

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
		case 'g': findGhosts=TRUE; break;
		case 'G': fixGhosts=TRUE; break;
		case 'n': noisy=TRUE; break;
		default: print_usage(); return (1); 
		}
	}
	if (!(BTR = openbtree(dbname, FALSE, TRUE))) {
		printf("could not open database: %s\n", dbname);
		return (1);
	}
	init_lifelines();

	if (findGhosts || fixGhosts)
		check_ghosts(fixGhosts);


	/*
	TO DO:
	bad pointers
	broken lineage links
	empty families
	*/

	report_results();

	closebtree(BTR);
	return TRUE;
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
