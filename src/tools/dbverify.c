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

extern BTREE BTR;
BOOLEAN selftest = FALSE; /* selftest rules (ignore paths) */


int opt_finnish = 0;

#define ERR_ORPHANNAME 0
#define ERR_GHOSTNAME 1
#define ERR_NUM 2

static INT err_count[ERR_NUM];
static LIST tofix;

static BOOLEAN noisy=FALSE;

/*=================================
 * print_usage -- Explain arguments
 *===============================*/
static void
print_usage ()
{
	printf(
		"usage: btedit -(flags) <btree>\n"
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
	err_count[err]++;
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
} NAMEREFN_REC;
/*===============================================
 * alloc_namerefn -- allocates a new NAMEREFN_REC
 *=============================================*/
static NAMEREFN_REC *
alloc_namerefn (STRING namerefn, STRING key)
{
	NAMEREFN_REC * rec = (NAMEREFN_REC *)stdalloc(sizeof(*rec));
	rec->namerefn = strsave(namerefn);
	rec->key = strsave(key);
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
/*============================================
 * cgn_callback -- callback for name traversal
 *  for checking for ghost names
 *==========================================*/
static BOOLEAN
cgn_callback (STRING key, STRING name, void *param)
{
	/* a name record which points at indi=key */
	INT fixGhosts = (INT)param;
	NODE indi = qkey_to_indi(key);
	if (!indi) {
		report_error(ERR_ORPHANNAME, "Orphaned name: %s", name);
		if (fixGhosts)
			enqueue_list(tofix, (WORD)alloc_namerefn(name, key));
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
			report_error(ERR_GHOSTNAME, "Ghost name: %s", name);
			if (fixGhosts)
				enqueue_list(tofix, (WORD)alloc_namerefn(name, key));
		}
	}

	if (noisy)
		report_progress("Name: %s", name);

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

	traverse_names(cgn_callback, (void *)fixGhosts);
	/*
	call traverse_refns here, except that 
	cgr_callback needs writing, and is a bit trickier
	as refns point to any type of record
	*/

	if (fixGhosts) {
		NAMEREFN_REC * rec;
		while (!empty_list(tofix)) {
			rec = (NAMEREFN_REC *) dequeue_list(tofix);
			remove_name(rec->namerefn, rec->key);
			free_namerefn(rec);
		}
	}
	
	ASSERT(empty_list(tofix));
	remove_list(tofix, NULL); 
	tofix=0;
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
