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
extern STRING mtitle;

/*********************************************
 * required global variables
 *********************************************/

BOOLEAN selftest = FALSE; /* selftest rules (ignore paths) */
STRING btreepath = NULL;	/* normally defined in liflines/main.c */
STRING readpath = NULL;		/* normally defined in liflines/main.c */
BOOLEAN readonly = FALSE;	/* normally defined in liflines/main.c */
BOOLEAN writeable = FALSE;	/* normally defined in liflines/main.c */
BOOLEAN immutable = FALSE;  /* normally defined in liflines/main.c */
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

/* alphabetical */
static NAMEREFN_REC * alloc_namerefn(STRING namerefn, STRING key, INT err);
static BOOLEAN cgn_callback(STRING key, STRING name, BOOLEAN newset, void *param);
static BOOLEAN cgr_callback(STRING key, STRING refn, BOOLEAN newset, void *param);
static BOOLEAN check_even(STRING key, NOD0 nod0);
static BOOLEAN check_fam(STRING key, NOD0 nod0);
static void check_ghosts(void);
static BOOLEAN check_indi(STRING key, NOD0 nod0);
static void check_node(STRING key, NODE node, INT level);
static void check_nodes(void);
static void check_pointers(STRING key, NOD0 nod0);
static void check_set(INDISEQ seq, char ctype);
static BOOLEAN check_sour(STRING key, NOD0 nod0);
static BOOLEAN check_othe(STRING key, NOD0 nod0);
static BOOLEAN find_xref(STRING key, NODE node, STRING tag1, STRING tag2);
static void finish_and_delete_nameset(void);
static void finish_and_delete_refnset(void);
static void free_namerefn(NAMEREFN_REC * rec);
static BOOLEAN nodes_callback(STRING key, NOD0 nod0, void *param);
static void print_usage(void);
static void report_error(INT err, STRING fmt, ...);
static void report_progress(STRING fmt, ...);
static void report_results(void);
static void validate_errs(void);

/*********************************************
 * local variables
 *********************************************/

enum {
	ERR_ORPHANNAME,  ERR_GHOSTNAME, ERR_DUPNAME, ERR_DUPREFN
	, ERR_NONINDINAME, ERR_DUPINDI, ERR_DUPFAM
	, ERR_DUPSOUR, ERR_DUPEVEN, ERR_DUPOTHE
	, ERR_MISSING, ERR_DELETED, ERR_BADNAME
	, ERR_BADFAMREF, ERR_MISSINGCHILD, ERR_MISSINGSPOUSE
	, ERR_BADHUSBREF, ERR_BADWIFEREF, ERR_BADCHILDREF
	, ERR_EXTRAHUSB, ERR_EXTRAWIFE, ERR_EXTRACHILD
	, ERR_EMPTYFAM, ERR_SOLOFAM, ERR_BADPOINTER
};

static struct errinfo errs[] = {
	{ ERR_ORPHANNAME, 0, 0, "Orphan names" }
	, { ERR_GHOSTNAME, 0, 0, "Ghost names" }
	, { ERR_DUPNAME, 0, 0, "Duplicate names" }
	, { ERR_DUPREFN, 0, 0, "Duplicate names" }
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
	, { ERR_MISSINGCHILD, 0, 0, "Missing child" }
	, { ERR_MISSINGSPOUSE, 0, 0, "Missing spouse" }
	, { ERR_BADHUSBREF, 0, 0, "Bad husb reference" }
	, { ERR_BADWIFEREF, 0, 0, "Bad wife reference" }
	, { ERR_BADCHILDREF, 0, 0, "Bad child reference" }
	, { ERR_EXTRAHUSB, 0, 0, "Improper husb" }
	, { ERR_EXTRAWIFE, 0, 0, "Improper wife" }
	, { ERR_EXTRACHILD, 0, 0, "Improper child" }
	, { ERR_EMPTYFAM, 0, 0, "Empty family" }
	, { ERR_SOLOFAM, 0, 0, "Single person family" }
	, { ERR_BADPOINTER, 0, 0, "Bad pointer" }
};
static struct work todo;
static LIST tofix;
/* sequence of NAMEs or REFNs in the same block */
static INDISEQ soundexseq;
static BOOLEAN noisy=FALSE;
static INDISEQ seq_indis, seq_fams, seq_sours, seq_evens, seq_othes;
static STRING lineage_tags[] = {
	"FAMC"
	, "FAMS"
	, "WIFE"
	, "HUSB"
};
/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================
 * print_usage -- Explain arguments
 * Created: 2001/01/01, Perry Rapp
 *===============================*/
static void
print_usage (void)
{
	char verstr[80];
	snprintf(verstr, sizeof(verstr), mtitle
		, get_lifelines_version(sizeof(verstr)-1-strlen(mtitle)));
	printf(
		"usage: dbverify -(flags) <btree>\n"
		"flags:\n"
		"\t-a = Perform all checks (does not include fixes)\n"
		"\t-g = Check for ghosts (names/refns)\n"
		"\t-G = Check for & fix ghosts (names/refns)\n"
		"\t-i = Check individuals\n"
		"\t-f = Check families\n"
		"\t-s = Check sours\n"
		"\t-e = Check events\n"
		"\t-x = Check others\n"
		"\t-n = Noisy (echo every record processed)\n"
#ifdef WIN32
		"example: dbverify -ifsex \"\\My Documents\\LifeLines\\Databases\\MyFamily\"\n"
#endif
		);
		printf("%s\n", verstr);
}
/*========================================
 * report_error -- report some error found
 * Created: 2001/01/01, Perry Rapp
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
 * Created: 2001/01/01, Perry Rapp
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
 * Created: 2001/01/01, Perry Rapp
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
 * Created: 2001/01/01, Perry Rapp
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
 * Created: 2001/01/01, Perry Rapp
 *=========================================*/
static void
check_ghosts (void)
{
	tofix = create_list();
	soundexseq = create_indiseq_sval();
	/* soundexseq is used inside cgn_callback, across calls */
	traverse_names(cgn_callback, NULL);
	finish_and_delete_nameset();

	if (todo.fix_ghosts) {
		NAMEREFN_REC * rec;
		while (!empty_list(tofix)) {
			rec = (NAMEREFN_REC *) dequeue_list(tofix);
			remove_name(rec->namerefn, rec->key);
			errs[rec->err].fix_count++;
			free_namerefn(rec);
		}
	}

	ASSERT(empty_list(tofix) && !soundexseq);

	soundexseq = create_indiseq_sval();
	/* soundexseq is used inside cgr_callback, across calls */
	traverse_refns(cgr_callback, NULL);
	finish_and_delete_refnset();

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
 * Created: 2001/01/01, Perry Rapp
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
		finish_and_delete_nameset();
		soundexseq = create_indiseq_sval();
	}

	append_indiseq_sval(soundexseq, strsave(key), name, strsave(name)
		, TRUE, TRUE); /* sure, alloc */

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
 * Created: 2001/01/13, Perry Rapp
 *==========================================*/
static BOOLEAN
cgr_callback (STRING key, STRING refn, BOOLEAN newset, void *param)
{
	/* a refn record which points at nod0=key */
	NOD0 nod0 = key_to_typ0(key, TRUE);
	NODE node = nztop(nod0);

	if (newset) {
		finish_and_delete_refnset();
		soundexseq = create_indiseq_sval();
	}
	append_indiseq_sval(soundexseq, strsave(key), NULL, strsave(refn)
		, TRUE, TRUE); /* sure, alloc */
	
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
 * soundexseq is sequence of NAMEs all in the same block (equivalent
 *       names as far as NAME storage goes)
 * Created: 2001/01/13, Perry Rapp
 *===============================================================*/
static void
finish_and_delete_nameset (void)
{
	char prevkey[8];
	STRING name="";
	TABLE table = create_table();
	prevkey[0]=0;
	keysort_indiseq(soundexseq);
	/*
	We go thru the list of equivalent names, sorted by person,
	and for each person, table all their names, watching for
	duplicates
	*/
	FORINDISEQ(soundexseq, el, num)
		name = sval(el).w;
		if (!eqstr(skey(el), prevkey)) {
			/* new person, start over */
			remove_table(table, DONTFREE);
			table = create_table();
		}
		if (in_table(table, name)) {
			report_error(ERR_DUPNAME, "Duplicate name for %s (%s)"
				, skey(el), name);
		} else {
			insert_table_int(table, name, 1);
		}
		strcpy(prevkey, skey(el));
	ENDINDISEQ
	remove_indiseq(soundexseq);
	soundexseq = NULL;
	remove_table(table, DONTFREE);
}
/*=================================================================
 * finish_and_delete_refnset -- check for dups in a set of one refn
 * soundexseq is sequence of REFNs all in the same block (equivalent
 *       names as far as REFN storage goes)
 * Created: 2001/01/13, Perry Rapp
 *===============================================================*/
static void
finish_and_delete_refnset (void)
{
	char prevkey[8];
	STRING refn="";
	TABLE table = create_table();
	prevkey[0]=0;
	canonkeysort_indiseq(soundexseq);
	FORINDISEQ(soundexseq, el, num)
		refn = sval(el).w;
		if (!eqstr(skey(el), prevkey)) {
			/* new person, start over */
			remove_table(table, DONTFREE);
			table = create_table();
		}
		if (in_table(table, refn)) {
			report_error(ERR_DUPREFN, "Duplicate refn for %s (%s)"
				, skey(el), refn);
		} else {
			insert_table_int(table, refn, 1);
		}
		strcpy(prevkey, skey(el));
	ENDINDISEQ
	remove_indiseq(soundexseq);
	soundexseq = NULL;
}
/*=================================
 * check_nodes -- Process all nodes
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
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
	remove_indiseq(seq_indis);
	remove_indiseq(seq_fams);
	remove_indiseq(seq_sours);
	remove_indiseq(seq_evens);
	remove_indiseq(seq_othes);
}
/*=============================================
 * nodes_callback -- callback for node traversal
 *  for checking indis, fams, sours, evens, othes
 * Created: 2001/01/14, Perry Rapp
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
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_indi (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	NODE indi1, name1, refn1, sex1, body1, famc1, fams1;
	NODE node1;
	CACHEEL icel1;
	INT keynum = atoi(&key[1]);
	if (eqstr(key, prevkey)) {
		report_error(ERR_DUPINDI, "Duplicate individual for %s", key);
	}
	indi1 = nztop(nod0);
	icel1 = indi_to_cacheel(indi1);
	lock_cache(icel1);
	split_indi(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	/* check names */
	for (node1 = name1; node1; node1 = nsibling(node1)) {
		STRING name=nval(node1);
		if (!valid_name(name)) {
			report_error(ERR_BADNAME, "Bad name for individual %s: %s", key, name);
		} else {
		/* TO DO: verify that name is in db */
		}
	}
	/* check refns */
	for (node1 = refn1; node1; node1 = nsibling(node1)) {
		STRING refn=nval(node1);
		/* TO DO: verify that refn is in db */
	}
	/* check parents */
	for (node1 = famc1; node1; node1 = nsibling(node1)) {
		STRING famkey=rmvat(nval(node1));
		NODE fam2 = qkey_to_fam(famkey);
		if (!fam2) {
			report_error(ERR_BADFAMREF, "Bad family reference (%s) individual %s", famkey, key);
		} else {
			/* look for indi1 (key) in fam2's children */
			if (!find_xref(key, fam2, "CHIL", NULL)) {
				report_error(ERR_MISSINGCHILD, "Missing child (%s) in family (%s)", key, famkey);
			}
		}
	}
	/* check spouses */
	for (node1 = fams1; node1; node1 = nsibling(node1)) {
		STRING famkey=rmvat(nval(node1));
		NODE fam2 = qkey_to_fam(famkey);
		if (!fam2) {
			report_error(ERR_BADFAMREF, "Bad family reference (%s) individual %s", famkey, key);
		} else {
			/* look for indi1 (key) in fam2's spouses */
			if (!find_xref(key, fam2, "HUSB", "WIFE")) {
				report_error(ERR_MISSINGSPOUSE, "Missing spouse (%s) in family (%s)", key, famkey);
			}
		}
	}
	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	unlock_cache(icel1);
	check_pointers(key, nod0);
	append_indiseq_null(seq_indis, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_fam -- process fam record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_fam (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	NODE fam1, fref1, husb1, wife1, chil1, rest1;
	NODE node1;
	CACHEEL fcel1;
	INT keynum = atoi(&key[1]);
	INT members = 0;
	if (eqstr(key, prevkey)) {
		report_error(ERR_DUPFAM, "Duplicate family for %s", key);
	}
	fam1 = nztop(nod0);
	fcel1 = fam_to_cacheel(fam1);
	lock_cache(fcel1);
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	/* check refns */
	for (node1 = fref1; node1; node1 = nsibling(node1)) {
		STRING refn=nval(node1);
		/* TO DO: verify that refn is in db */
	}
	/* check husbs */
	for (node1 = husb1; node1; node1 = nsibling(node1)) {
		STRING husbkey=rmvat(nval(node1));
		NODE husb = qkey_to_indi(husbkey);
		members++;
		if (!husb) {
			report_error(ERR_BADHUSBREF, "Bad husb reference (%s) in family %s", husbkey, key);
		} else {
			/* look for family (key) in husb */
			if (!find_xref(key, husb, "FAMS", NULL)) {
				report_error(ERR_EXTRAHUSB, "Improper husb (%s) in family (%s)", husbkey, key);
			}
		}
	}
	/* check wives */
	for (node1 = wife1; node1; node1 = nsibling(node1)) {
		STRING wifekey=rmvat(nval(node1));
		NODE wife = qkey_to_indi(wifekey);
		members++;
		if (!wife) {
			report_error(ERR_BADWIFEREF, "Bad wife reference (%s) in family %s", wifekey, key);
		} else {
			/* look for family (key) in wife */
			if (!find_xref(key, wife, "FAMS", NULL)) {
				report_error(ERR_EXTRAWIFE, "Improper wife (%s) in family (%s)", wifekey, key);
			}
		}
	}
	/* check children */
	for (node1 = chil1; node1; node1 = nsibling(node1)) {
		STRING chilkey=rmvat(nval(node1));
		NODE child = qkey_to_indi(chilkey);
		members++;
		if (!child) {
			report_error(ERR_BADCHILDREF, "Bad child reference (%s) in family %s", chilkey, key);
		} else {
			/* look for family (key) in child */
			if (!find_xref(key, child, "FAMC", NULL)) {
				report_error(ERR_EXTRACHILD, "Improper child (%s) in family (%s)", chilkey, key);
			}
		}
	}
	join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
	/* check for undersized family */
	if (!members) {
		report_error(ERR_EMPTYFAM, "Empty family (%s)", key);
	} else if (members == 1) {
		report_error(ERR_SOLOFAM, "Single person family (%s)", key);
	}
	unlock_cache(fcel1);
	check_pointers(key, nod0);
	append_indiseq_null(seq_fams, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_sour -- process sour record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_sour (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPSOUR, "Duplicate source for %s", key);
	}
	check_pointers(key, nod0);
	append_indiseq_null(seq_sours, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_even -- process even record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_even (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPEVEN, "Duplicate event for %s", key);
	}
	check_pointers(key, nod0);
	append_indiseq_null(seq_evens, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_othe -- process othe record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_othe (STRING key, NOD0 nod0)
{
	static char prevkey[9];
	INT keynum = atoi(&key[1]);
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPOTHE, "Duplicate record for %s", key);
	}
	check_pointers(key, nod0);
	append_indiseq_null(seq_othes, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*===================================
 * find_xref -- Search node for a cross-reference key
 * 2001/01/21, Perry Rapp
 *=================================*/
static BOOLEAN
find_xref (STRING key, NODE node, STRING tag1, STRING tag2)
{
	NODE node2;
	CACHEEL ncel2;
	BOOLEAN found=FALSE;
	ncel2 = node_to_cacheel(node);
	lock_cache(ncel2);
	for (node2 = nchild(node); node2; node2 = nsibling(node2)) {
		if (eqstr(tag1, ntag(node2))
			|| (tag2 && eqstr(tag2, ntag(node2)))) {
			STRING key2 = rmvat(nval(node2));
			if (eqstr(key, key2)) {
				found = TRUE;
				goto exit_find;
			}
		}
	}
exit_find:
	unlock_cache(ncel2);
	return found;
}
/*=====================================
 * check_pointers -- check nod0 for bad pointers
 *  and bad levels
 * 2001/01/21, Perry Rapp
 *===================================*/
static void
check_pointers (STRING key, NOD0 nod0)
{
	check_node(key, nztop(nod0), 0);
}
/*=====================================
 * check_node -- check node for bad pointers
 *  and bad levels, and continue traverse
 * 2001/02/18, Perry Rapp
 *===================================*/
static void
check_node (STRING n0key, NODE node, INT level)
{
	BOOLEAN lineage=FALSE;
	/* ignore lineage links - they are checked elsewhere */
	if (level==1) {
		INT i;
		for (i=0; i<ARRSIZE(lineage_tags); i++) {
			if (eqstr(ntag(node), lineage_tags[i])) {
				lineage=TRUE;
				break;
			}
		}
	}
	/*
	TO DO: How do we tell non-pointers that *ought* to
	be pointers, eg "1 SOUR <FamilyHistory>" ?
	*/
	if (!lineage) {
		STRING skey = rmvat(nval(node));
		if (skey) {
			NODE xnode = qkey_to_type(skey);
			if (!xnode) {
				report_error(ERR_BADPOINTER, "Bad pointer (in %s): %s", n0key, nval(node));
			}
		}
	}
	if (nchild(node))
		check_node(n0key, nchild(node), level+1);
	if (nsibling(node))
		check_node(n0key, nsibling(node), level);
}
/*===================================
 * check_set -- Validate set of nodes
 * Created: 2001/01/14, Perry Rapp
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
 * Created: 2001/01/13, Perry Rapp
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
 * Created: 2001/01/01, Perry Rapp
 *=========================================*/
int
main (int argc,
      char **argv)
{
	char *flags, *dbname;
	char *ptr;
	char * msg;
	BOOLEAN cflag=FALSE; /* create new db if not found */
	INT writ=1; /* request write access to database */
	BOOLEAN immut=FALSE; /* immutable access to database */
	BOOLEAN allchecks=FALSE; /* if user requested all checks */

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
		case 'a': allchecks=TRUE; break;
		default: print_usage(); return (1); 
		}
	}

	/* Turn off Memory Debugging */
	alloclog  = FALSE;

	if (!init_lifelines_global(&msg)) {
		printf("%s\n", msg);
		return (1);
	}
	if (!(BTR = openbtree(dbname, cflag, writ, immut))) {
		char buffer[256];
		describe_dberror(bterrno, buffer, ARRSIZE(buffer));
		puts(buffer);
		return (1);
	}
	init_lifelines_db();
	printf("Checking %s\n", dbname);

	if (todo.find_ghosts || todo.fix_ghosts)
		check_ghosts();

	/* this simplifies later logic */
	if (todo.fix_indis) todo.check_indis=TRUE;
	if (todo.fix_fams) todo.check_fams=TRUE;
	if (todo.fix_sours) todo.check_sours=TRUE;
	if (todo.fix_evens) todo.check_evens=TRUE;
	if (todo.fix_othes) todo.check_othes=TRUE;

	/* all fixes - if any new ones, have to update this */
	if (allchecks) {
		todo.check_indis=todo.check_fams=todo.check_sours=TRUE;
		todo.check_evens=todo.check_othes=TRUE;
		todo.find_ghosts=TRUE;
	}

	if (!(bwrite(BTR))) {
		todo.fix_indis = todo.fix_fams = todo.fix_sours = FALSE;
		todo.fix_evens = todo.fix_othes = FALSE;
	}


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
 * Created: 2001/01/13, Perry Rapp
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
/*=============================
 * fatal -- Fatal error routine
 * Created: 2001/01/01, Perry Rapp
 *  handles null or empty details input
 *===========================*/
void
__fatal (STRING file, int line, STRING details)
{
	printf("FATAL ERROR: ");
	if (details && details[0]) {
		printf(details);
		printf("\nAT: ");
	}
	printf("%s: line %d\n", file, line);
	exit(1);
}
