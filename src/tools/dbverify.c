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

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "gedcom.h"
#include "btree.h"
#include "indiseq.h"
#include "lloptions.h"
#include "cache.h"
#include "arch.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif


/*********************************************
 * external variables (no header)
 *********************************************/

extern BTREE BTR;
extern STRING qSmtitle, qSbaddb;

/*********************************************
 * required global variables
 *********************************************/

BOOLEAN selftest = FALSE; /* selftest rules (ignore paths) */
STRING readpath_file = NULL;	/* normally defined in liflines/main.c */
STRING readpath = NULL;		/* normally defined in liflines/main.c */
BOOLEAN readonly = FALSE;	/* normally defined in liflines/main.c */
BOOLEAN writeable = FALSE;	/* normally defined in liflines/main.c */
BOOLEAN immutable = FALSE;  /* normally defined in liflines/main.c */
int opt_finnish = 0;
int opt_mychar = 0;

/*********************************************
 * local types
 *********************************************/

/*==========================================
 * errinfo -- holds stats on a type of error
 *========================================*/
struct errinfo {
	int err;
	int err_count;
	int fix_count;
	STRING desc;
};
/*==================================
 * work -- what the user wants to do
 *================================*/
struct work {
	INT check_dbstructure;
	INT find_ghosts;
	INT fix_ghosts;
	INT check_indis;
	INT check_fams;
	INT check_sours;
	INT check_evens;
	INT check_othes;
	INT fix_deletes;
	INT fix_alter_pointers;
	INT check_missing_data_records; /* record in index, but no data */
	INT fix_missing_data_records;
	INT pass; /* =1 is checking, =2 is fixing */
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
static NAMEREFN_REC * alloc_namerefn(CNSTRING namerefn, CNSTRING key, INT err);
static BOOLEAN cgn_callback(CNSTRING key, CNSTRING name, BOOLEAN newset, void *param);
static BOOLEAN cgr_callback(CNSTRING key, CNSTRING refn, BOOLEAN newset, void *param);
static void check_and_fix_records(void);
static BOOLEAN check_block(BLOCK block, RKEY * lo, RKEY * hi);
static BOOLEAN check_btree(BTREE btr);
static BOOLEAN check_even(CNSTRING key, RECORD rec);
static BOOLEAN check_fam(CNSTRING key, RECORD rec);
static void check_ghosts(void);
static BOOLEAN check_keys(BLOCK block, RKEY * lo, RKEY * hi);
static BOOLEAN check_index(BTREE btr, INDEX index, TABLE fkeytab, RKEY * lo, RKEY * hi);
static BOOLEAN check_indi(CNSTRING key, RECORD rec);
static void check_node(CNSTRING key, NODE node, INT level);
static void check_nodes(void);
static void check_missing_data_records(void);
static void check_pointers(CNSTRING key, RECORD rec);
static void check_set(INDISEQ seq, char ctype);
static BOOLEAN check_sour(CNSTRING key, RECORD rec);
static void check_typed_missing_data_records(char ntype);
static BOOLEAN check_othe(CNSTRING key, RECORD rec);
static BOOLEAN find_xref(CNSTRING key, NODE node, CNSTRING tag1, CNSTRING tag2);
static void finish_and_delete_nameset(void);
static void finish_and_delete_refnset(void);
static void fix_nodes(void);
static void free_namerefn(NAMEREFN_REC * rec);
static BOOLEAN fix_bad_pointer(CNSTRING key, RECORD rec, NODE node);
static BOOLEAN nodes_callback(CNSTRING key, RECORD rec, void *param);
static void printblock(BLOCK block);
static CNSTRING printkey(CNSTRING key);
static void print_usage(void);
static void process_fam(RECORD rec);
static void process_indi(RECORD rec);
static void process_record(RECORD rec);
static void report_error(INT err, STRING fmt, ...);
static void report_fix(INT err, STRING fmt, ...);
static void report_progress(STRING fmt, ...);
static void report_results(void);
static void validate_errs(void);
static void vcrashlog(int newline, const char * fmt, va_list args);

/*********************************************
 * local variables
 *********************************************/

enum {
	ERR_ORPHANNAME,  ERR_GHOSTNAME, ERR_DUPNAME, ERR_DUPREFN
	, ERR_NONINDINAME, ERR_DUPINDI, ERR_DUPFAM
	, ERR_DUPSOUR, ERR_DUPEVEN, ERR_DUPOTHE
	, ERR_UNDELETED, ERR_DELETED, ERR_BADNAME
	, ERR_BADFAMREF, ERR_MISSINGCHILD, ERR_MISSINGSPOUSE
	, ERR_BADHUSBREF, ERR_BADWIFEREF, ERR_BADCHILDREF
	, ERR_EXTRAHUSB, ERR_EXTRAWIFE, ERR_EXTRACHILD
	, ERR_EMPTYFAM, ERR_SOLOFAM, ERR_BADPOINTER
	, ERR_MISSINGREC
};

static struct errinfo errs[] = {
	{ ERR_ORPHANNAME, 0, 0, N_("Orphan names") }
	, { ERR_GHOSTNAME, 0, 0, N_("Ghost names") }
	, { ERR_DUPNAME, 0, 0, N_("Duplicate names") }
	, { ERR_DUPREFN, 0, 0, N_("Duplicate names") }
	, { ERR_NONINDINAME, 0, 0, N_("Non-indi names") }
	, { ERR_DUPINDI, 0, 0, N_("Duplicate individuals") }
	, { ERR_DUPFAM, 0, 0, N_("Duplicate families") }
	, { ERR_DUPSOUR, 0, 0, N_("Duplicate sources") }
	, { ERR_DUPEVEN, 0, 0, N_("Duplicate events") }
	, { ERR_DUPOTHE, 0, 0, N_("Duplicate others") }
	, { ERR_UNDELETED, 0, 0, N_("Missing records (from deleteset)") }
	, { ERR_DELETED, 0, 0, N_("Deleted records") }
	, { ERR_BADNAME, 0, 0, N_("Bad name") }
	, { ERR_BADFAMREF, 0, 0, N_("Bad family reference") }
	, { ERR_MISSINGCHILD, 0, 0, N_("Missing child") }
	, { ERR_MISSINGSPOUSE, 0, 0, N_("Missing spouse") }
	, { ERR_BADHUSBREF, 0, 0, N_("Bad HUSB reference") }
	, { ERR_BADWIFEREF, 0, 0, N_("Bad WIFE reference") }
	, { ERR_BADCHILDREF, 0, 0, N_("Bad CHIL reference") }
	, { ERR_EXTRAHUSB, 0, 0, N_("Improper HUSB") }
	, { ERR_EXTRAWIFE, 0, 0, N_("Improper WIFE") }
	, { ERR_EXTRACHILD, 0, 0, N_("Improper child") }
	, { ERR_EMPTYFAM, 0, 0, N_("Empty family") }
	, { ERR_SOLOFAM, 0, 0, N_("Single person family") }
	, { ERR_BADPOINTER, 0, 0, N_("Bad pointer") }
	, { ERR_MISSINGREC, 0, 0, N_("Missing data records") }
};
static struct work todo;
static LIST tofix=0;
/* sequence of NAMEs or REFNs in the same block */
static INDISEQ soundexseq=0;
static BOOLEAN noisy=FALSE;
static INDISEQ seq_indis, seq_fams, seq_sours, seq_evens, seq_othes;
static STRING lineage_tags[] = {
	"FAMC"
	, "FAMS"
	, "WIFE"
	, "HUSB"
	, "CHIL"
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
	STRING title = _(qSmtitle);
#ifdef WIN32
	char * fname = _("\\My Documents\\LifeLines\\Databases\\MyFamily");
#else
	char * fname = _("/home/users/myname/lifelines/databases/myfamily");
#endif
	llstrncpyf(verstr, sizeof(verstr), uu8, title
		, get_lifelines_version(sizeof(verstr)-1-strlen(title)));
	printf(_("usage: dbverify -(flags) <btree>\n"));
	printf(_("flags:\n"));
	printf(_("\t-a = Perform all checks (does not include fixes)\n"));
	printf(_("\t-g = Check for ghosts (names/refns)\n"));
	printf(_("\t-G = Check for & fix ghosts (names/refns)\n"));
	printf(_("\t-i = Check individuals\n"));
	printf(_("\t-f = Check families\n"));
	printf(_("\t-F = Alter any bad family lineage pointers (to _badptr)\n"));
	printf(_("\t-h = Display help text (this text)\n"));
	printf(_("\t-s = Check sours\n"));
	printf(_("\t-e = Check events\n"));
	printf(_("\t-x = Check others\n"));
	printf(_("\t-l = Check database structure\n"));
	printf(_("\t-m = Check for records missing data entries\n"));
	printf(_("\t-M = Fix records missing data entries\n"));
	printf(_("\t-D = Fix bad delete entries\n"));
	printf(_("\t-n = Noisy (echo every record processed)\n"));
	printf(_("example: dbverify -ifsex \"%s\"\n"), fname);
	printf("%s\n", verstr);
}
/*========================================
 * report_error -- report some error found
 *  increment error count
 *======================================*/
static void
report_error (INT err, STRING fmt, ...)
{
	va_list args;
	if (err>=0)
		++errs[err].err_count;
	va_start(args, fmt);
	printf("! ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}
/*========================================
 * report_fix -- report an error fixed
 *  increment fixed count
 *======================================*/
static void
report_fix (INT err, STRING fmt, ...)
{
	va_list args;
	++errs[err].fix_count;
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
alloc_namerefn (CNSTRING namerefn, CNSTRING key, INT err)
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
		while (!is_empty_list(tofix)) {
			rec = (NAMEREFN_REC *) dequeue_list(tofix);
			remove_name(rec->namerefn, rec->key);
			++errs[rec->err].fix_count;
			free_namerefn(rec);
		}
	}

	ASSERT(is_empty_list(tofix));
	ASSERT(!soundexseq);

	soundexseq = create_indiseq_sval();
	/* soundexseq is used inside cgr_callback, across calls */
	traverse_refns(cgr_callback, NULL);
	finish_and_delete_refnset();

	if (todo.fix_ghosts) {
		NAMEREFN_REC * rec;
		while (!is_empty_list(tofix)) {
			rec = (NAMEREFN_REC *) dequeue_list(tofix);
			remove_refn(rec->namerefn, rec->key);
			free_namerefn(rec);
		}
	}
	
	destroy_list(tofix);
	tofix=0;
}
/*============================================
 * cgn_callback -- callback for name traversal
 *  for checking for ghost names
 * Created: 2001/01/01, Perry Rapp
 *==========================================*/
static BOOLEAN
cgn_callback (CNSTRING key, CNSTRING name, BOOLEAN newset, void *param)
{
	/* a name record which points at indi=key */
	RECORD indi0 = NULL;
	NODE indi = NULL;
	param=param; /* unused */

	/* bail out immediately if not INDI */
	if (key[0] != 'I') {
		report_error(ERR_NONINDINAME
			, _("Non-indi name, key=%s, name=%s")
			, key, name);
		return 1; /* continue traversal */
	}
	
	indi0 = qkey_to_irecord(key);
	indi = nztop(indi0);

	if (newset) {
		finish_and_delete_nameset();
		soundexseq = create_indiseq_sval();
	}

	append_indiseq_sval(soundexseq, strsave(key), (STRING)name, strsave(name)
		, TRUE, TRUE); /* sure, alloc */

	if (!indi) {
		report_error(ERR_ORPHANNAME, _("Orphaned name: %s"), name);
		if (todo.fix_ghosts)
			enqueue_list(tofix, (VPTR)alloc_namerefn(name, key, ERR_ORPHANNAME));
	} else {
		NODE node;
		BOOLEAN found=FALSE;
		NODE nam, refn, sex, body, famc, fams;
		split_indi_old(indi, &nam, &refn, &sex, &body, &famc, &fams);
		for (node = nam; node; node = nsibling(node)) {
			if (!strcmp(nval(node), name))
				found=TRUE;
		}
		join_indi(indi, nam, refn, sex, body, famc, fams);
		if (!found) {
			report_error(ERR_GHOSTNAME, _("Ghost name: %s -> %s"), name, key);
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
cgr_callback (CNSTRING key, CNSTRING refn, BOOLEAN newset, void *param)
{
	/* a refn record which points at record=key */
	RECORD rec = key_to_record(key);
	NODE node = nztop(rec);
	param = param; /* unused */

	if (newset) {
		finish_and_delete_refnset();
		soundexseq = create_indiseq_sval();
	}
	append_indiseq_sval(soundexseq, strsave(key), NULL, strsave(refn)
		, TRUE, TRUE); /* sure, alloc */
	
	if (!node) {
		report_error(ERR_ORPHANNAME, _("Orphaned refn: %s"), refn);
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
	CNSTRING name="";
	CNSTRING skey="";
	TABLE table = create_table_int();
	prevkey[0]=0;
	calc_indiseq_names(soundexseq);
	keysort_indiseq(soundexseq);
	/*
	We go thru the list of equivalent names, sorted by person,
	and for each person, table all their names, watching for
	duplicates
	*/
	FORINDISEQ(soundexseq, el, num)
		name = element_sval(el);
		skey = element_skey(el);
		if (!eqstr(skey, prevkey)) {
			/* new person, start over */
			destroy_table(table);
			table = create_table_int();
		}
		if (in_table(table, name)) {
			report_error(ERR_DUPNAME, _("Duplicate name for %s (%s)")
				, skey, name);
		} else {
			insert_table_int(table, name, 1);
		}
		strcpy(prevkey, skey);
	ENDINDISEQ
	remove_indiseq(soundexseq);
	soundexseq = NULL;
	destroy_table(table);
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
	CNSTRING refn="";
	CNSTRING skey="";
	TABLE table = create_table_int();
	prevkey[0]=0;
	canonkeysort_indiseq(soundexseq);
	FORINDISEQ(soundexseq, el, num)
		refn = element_sval(el);
		skey = element_skey(el);
		if (!eqstr(skey, prevkey)) {
			/* new person, start over */
			destroy_table(table);
			table = create_table_int();
		}
		if (in_table(table, refn)) {
			report_error(ERR_DUPREFN, _("Duplicate refn for %s (%s)")
				, skey, refn);
		} else {
			insert_table_int(table, refn, 1);
		}
		strcpy(prevkey, skey);
	ENDINDISEQ
	remove_indiseq(soundexseq);
	soundexseq = NULL;
	destroy_table(table);
}
/*=================================
 * check_nodes -- Check all nodes in database
 * (as selected in struct work)
 *================================*/
static void
check_nodes (void)
{
	seq_indis = create_indiseq_null();
	seq_fams = create_indiseq_null();
	seq_sours = create_indiseq_null();
	seq_evens = create_indiseq_null();
	seq_othes = create_indiseq_null();
	traverse_db_key_recs(nodes_callback, NULL);
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
/*=================================
 * check_and_fix_records -- 
 *================================*/
static void
check_and_fix_records (void)
{
	tofix = create_list();
	todo.pass = 1;
	check_nodes();
	todo.pass = 2;
	fix_nodes();
	destroy_empty_list(tofix);
}
/*=================================
 * fix_nodes -- Fix all nodes on fix list
 *  (uses list from check_nodes)
 *================================*/
static void
fix_nodes (void)
{
	if (!tofix) return;
	while (!is_empty_list(tofix)) {
		STRING key = dequeue_list(tofix);
		RECORD rec = key_to_record(key);
		strfree(&key);
		lock_record_in_cache(rec);
		process_record(rec);
		unlock_record_from_cache(rec);
	}
}
/*=============================================
 * nodes_callback -- callback for node traversal
 *  for checking indis, fams, sours, evens, othes
 * Created: 2001/01/14, Perry Rapp
 *===========================================*/
static BOOLEAN
nodes_callback (CNSTRING key, RECORD rec, void *param)
{
	param=param;	/* NOTUSED */
	if (noisy)
		report_progress("Node: %s", key);
	switch (key[0]) {
	case 'I': return todo.check_indis ? check_indi((CNSTRING)key, (RECORD)rec) : TRUE;
	case 'F': return todo.check_fams ? check_fam((CNSTRING)key, (RECORD)rec) : TRUE;
	case 'S': return todo.check_sours ? check_sour((CNSTRING)key, (RECORD)rec) : TRUE;
	case 'E': return todo.check_evens ? check_even((CNSTRING)key, (RECORD)rec) : TRUE;
	case 'X': return todo.check_othes ? check_othe((CNSTRING)key, (RECORD)rec) : TRUE;
	}
	ASSERT(0); /* traverse_db_key_recs is broken */
	return TRUE;
}
/*=====================================
 * process_record -- process record
 *  Called during pass2 (fix)
 *===================================*/
static void
process_record (RECORD rec)
{
	switch(nztype(rec)) {
	case 'I': process_indi(rec); break;
	case 'F': process_fam(rec); break;
		/* TODO */
	}
}
/*=====================================
 * check_indi -- check indi record
 *  and record any records needing fixing
 *  in tofix list
 *===================================*/
static BOOLEAN
check_indi (CNSTRING key, RECORD rec)
{
	static char prevkey[MAXKEYWIDTH+1];
	CACHEEL icel1=0;
	RECORD recx=0;

	if (eqstr(key, prevkey)) {
		report_error(ERR_DUPINDI, _("Duplicate individual for %s"), key);
	}
	icel1 = indi_to_cacheel(rec);
	lock_cache(icel1);

	recx = get_record_for_cel(icel1);
	ASSERT(todo.pass == 1);
	process_indi(recx);

	unlock_cache(icel1);
	check_pointers(key, rec);
	append_indiseq_null(seq_indis, strsave(key), NULL, TRUE, TRUE);
	release_record(recx);
	return TRUE;
}
/*=====================================
 * process_indi -- process indi record
 *  checking in pass 1, fixing in pass 2
 *===================================*/
static void
process_indi (RECORD rec)
{
	NODE indi0, indi1;
	NODE name1, refn1, sex1, body1, famc1, fams1;
	NODE node1;
	BOOLEAN altered=FALSE;
	BOOLEAN needfix=FALSE;
	CNSTRING key = nzkey(rec);

	indi0 = nztop(rec);
	if (todo.pass==1) {
		indi1 = indi0;
	} else {
		indi1 = copy_node_subtree(indi0);
	}
	split_indi_old(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);

	if (todo.pass == 1) {
		/* check names */
		for (node1 = name1; node1; node1 = nsibling(node1)) {
			STRING name=nval(node1);
			if (!valid_name(name)) {
				report_error(ERR_BADNAME, _("Bad name for individual %s: %s"), key, name);
			} else {
			/* TO DO: verify that name is in db */
			}
		}
		/* check refns */
		for (node1 = refn1; node1; node1 = nsibling(node1)) {
			/* STRING refn=nval(node1); */
			/* TO DO: verify that refn is in db */
		}
	}

	/* check parents */
	for (node1 = famc1; node1; node1 = nsibling(node1)) {
		STRING famkey=rmvat(nval(node1));
		NODE fam2 = qkey_to_fam(famkey);
		if (!fam2) {
			if (todo.pass == 1) {
				report_error(ERR_BADFAMREF, _("Bad family reference (%s) individual %s"), famkey, key);
			}
		} else {
			/* look for indi1 (key) in fam2's children */
			if (!find_xref(key, fam2, "CHIL", NULL)) {
				if (todo.pass == 1) {
					report_error(ERR_MISSINGCHILD, _("Missing child (%s) in family (%s)"), key, famkey);
					needfix=TRUE;
				} else {
					if (fix_bad_pointer(key, rec, node1)) {
						report_fix(ERR_MISSINGCHILD, _("Fixed missing child (%s) in family (%s)"), key, famkey);
						altered=TRUE;
					}
				}
			}
		}
	}
	
	/* check spouses */
	for (node1 = fams1; node1; node1 = nsibling(node1)) {
		STRING famkey=rmvat(nval(node1));
		NODE fam2 = qkey_to_fam(famkey);
		if (!fam2) {
			if (todo.pass == 1) {
				report_error(ERR_BADFAMREF, _("Bad family reference (%s) individual %s"), famkey, key);
			}
		} else {
			/* look for indi1 (key) in fam2's spouses */
			if (!find_xref(key, fam2, "HUSB", "WIFE")) {
				if (todo.pass == 1) {
					report_error(ERR_MISSINGSPOUSE, _("Missing spouse (%s) in family (%s)"), key, famkey);
					needfix=TRUE;
				} else {
					if (fix_bad_pointer(key, rec, node1)) {
						report_fix(ERR_MISSINGSPOUSE, _("Fixed missing spouse (%s) in family (%s)"), key, famkey);
						altered=TRUE;
					}
				}
			}
		}
	}

	join_indi(indi1, name1, refn1, sex1, body1, famc1, fams1);
	if (altered) {
		/* must normalize, as some lineage references may have been 
		altered to non-lineage tags to fix broken pointers */
		normalize_indi(indi1);

		/* write to database */
		replace_indi(indi0, indi1);

	} else if (needfix) {
		enqueue_list(tofix, strsave(key));
	}
}
/*=====================================
 * check_fam -- check family record
 *  and record any records needing fixing
 *  in tofix list
 *===================================*/
static BOOLEAN
check_fam (CNSTRING key, RECORD rec)
{
	static char prevkey[MAXKEYWIDTH+1]="";
	CACHEEL fcel1=0;
	RECORD recx=0;

	if (eqstr(key, prevkey)) {
		report_error(ERR_DUPFAM, _("Duplicate family for %s"), key);
	}
	
	fcel1 = fam_to_cacheel(rec);
	lock_cache(fcel1);

	recx = get_record_for_cel(fcel1);
	ASSERT(todo.pass == 1);
	process_fam(recx);

	unlock_cache(fcel1);
	check_pointers(key, rec);
	append_indiseq_null(seq_fams, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * process_fam -- process indi record
 *  checking in pass 1, fixing in pass 2
 *===================================*/
static void
process_fam (RECORD rec)
{
	NODE fam0, fam1;
	NODE fref1, husb1, wife1, chil1, rest1;
	NODE node1;
	INT members = 0;
	BOOLEAN altered=FALSE;
	BOOLEAN needfix=FALSE;
	CNSTRING key = nzkey(rec);

	fam0 = nztop(rec);
	if (todo.pass==1) {
		fam1 = fam0;
	} else {
		fam1 = copy_node_subtree(fam0);
	}
	split_fam(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);

	/* check refns */
	for (node1 = fref1; node1; node1 = nsibling(node1)) {
		/* STRING refn=nval(node1); */
		/* TO DO: verify that refn is in db */
	}
	/* check husbs */
	for (node1 = husb1; node1; node1 = nsibling(node1)) {
		STRING husbkey=rmvat(nval(node1));
		NODE husb = qkey_to_indi(husbkey);
		members++;
		if (!husb) {
			if (todo.pass == 1) {
				report_error(ERR_BADHUSBREF
					, _("Bad HUSB reference (%s) in family %s")
					, husbkey, key);
				needfix=TRUE;
			} else {
				if (fix_bad_pointer(key, rec, node1)) {
					report_fix(ERR_BADHUSBREF
						, _("Fixed Bad HUSB reference (%s) in family %s")
						, husbkey, key);
					altered=TRUE;
				}
			}
		} else {
			/* look for family (key) in husb */
			if (!find_xref(key, husb, "FAMS", NULL)) {
				report_error(ERR_EXTRAHUSB
					, _("Improper HUSB (%s) in family (%s)")
					, husbkey, key);
			}
		}
	}
	/* check wives */
	for (node1 = wife1; node1; node1 = nsibling(node1)) {
		STRING wifekey=rmvat(nval(node1));
		NODE wife = qkey_to_indi(wifekey);
		members++;
		if (!wife) {
			if (todo.pass == 1) {
				report_error(ERR_BADWIFEREF
					, _("Bad wife reference (%s) in family %s")
					, wifekey, key);
				needfix=TRUE;
			} else {
				if (fix_bad_pointer(key, rec, node1)) {
					report_fix(ERR_BADWIFEREF
						, _("Fixed Bad wife reference (%s) in family %s")
						, printkey(wifekey), key);
					altered=TRUE;
				}
			}
		} else {
			/* look for family (key) in wife */
			if (!find_xref(key, wife, "FAMS", NULL)) {
				report_error(ERR_EXTRAWIFE
					, _("Improper wife (%s) in family (%s)")
					, printkey(wifekey), key);
			}
		}
	}
	/* check children */
	for (node1 = chil1; node1; node1 = nsibling(node1)) {
		STRING chilkey=rmvat(nval(node1));
		NODE child = qkey_to_indi(chilkey);
		members++;
		if (!child) {
			if (todo.pass == 1) {
				report_error(ERR_BADCHILDREF
					, _("Bad child reference (%s) in family %s")
					, printkey(chilkey), key);
				needfix=TRUE;
			} else {
				if (fix_bad_pointer(key, rec, node1)) {
					report_fix(ERR_BADCHILDREF
						, _("Fixed bad child reference (%s) in family %s")
						, printkey(chilkey), key);
					altered=TRUE;
				}
			}
		} else {
			/* look for family (key) in child */
			if (!find_xref(key, child, "FAMC", NULL)) {
				report_error(ERR_EXTRACHILD
					, _("Improper child: Child (%s) without FAMC reference to family (%s)")
					, printkey(chilkey), key);
			}
		}
	}
	join_fam(fam1, fref1, husb1, wife1, chil1, rest1);
	/* check for undersized family */
	if (!members) {
		report_error(ERR_EMPTYFAM, _("Empty family (%s)"), key);
	} else if (members == 1) {
		report_error(ERR_SOLOFAM, _("Single person family (%s)"), key);
	}


	if (altered) {
		/* must normalize, as some lineage references may have been 
		altered to non-lineage tags to fix broken pointers */
		normalize_fam(fam1);

		/* write to database */
		replace_fam(fam0, fam1);

	} else if (needfix) {
		enqueue_list(tofix, strsave(key));
	}
}
/*=====================================
 * check_sour -- process sour record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_sour (CNSTRING key, RECORD rec)
{
	static char prevkey[MAXKEYWIDTH+1];
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPSOUR, _("Duplicate source for %s"), key);
	}
	check_pointers(key, rec);
	append_indiseq_null(seq_sours, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_even -- process even record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_even (CNSTRING key, RECORD rec)
{
	static char prevkey[MAXKEYWIDTH+1];
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPEVEN, _("Duplicate event for %s"), key);
	}
	check_pointers(key, rec);
	append_indiseq_null(seq_evens, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*=====================================
 * check_othe -- process othe record
 *  checking and/or fixing as requested
 * Created: 2001/01/14, Perry Rapp
 *===================================*/
static BOOLEAN
check_othe (CNSTRING key, RECORD rec)
{
	static char prevkey[MAXKEYWIDTH+1];
	if (!strcmp(key, prevkey)) {
		report_error(ERR_DUPOTHE, _("Duplicate record for %s"), key);
	}
	check_pointers(key, rec);
	append_indiseq_null(seq_othes, strsave(key), NULL, TRUE, TRUE);
	return TRUE;
}
/*===================================
 * find_xref -- Search node for a cross-reference key
 * 2001/01/21, Perry Rapp
 *=================================*/
static BOOLEAN
find_xref (CNSTRING key, NODE node, CNSTRING tag1, CNSTRING tag2)
{
	NODE node2;
	CACHEEL ncel2;
	BOOLEAN found=FALSE;
	ncel2 = node_to_cacheel_old(node);
	lock_cache(ncel2);
	for (node2 = nchild(node); node2; node2 = nsibling(node2)) {
		if (eqstr(tag1, ntag(node2))
			|| (tag2 && eqstr(tag2, ntag(node2)))) {
			STRING key2 = rmvat(nval(node2));
			if (key2 && eqstr(key, key2)) {
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
 * check_pointers -- check record for bad pointers
 *  and bad levels
 * 2001/01/21, Perry Rapp
 *===================================*/
static void
check_pointers (CNSTRING key, RECORD rec)
{
	check_node(key, nztop(rec), 0);
}
/*=====================================
 * check_node -- check node for bad pointers
 *  and bad levels, and continue traverse
 * 2001/02/18, Perry Rapp
 *===================================*/
static void
check_node (CNSTRING n0key, NODE node, INT level)
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
				report_error(ERR_BADPOINTER
					, _("Bad pointer (in %s): %s")
					, n0key, nval(node));
			}
		}
	}
	if (nchild(node))
		check_node(n0key, nchild(node), level+1);
	if (nsibling(node))
		check_node(n0key, nsibling(node), level);
}
/*===================================
 * fix_bad_pointer -- Fix bad pointer if requested
 *=================================*/
static BOOLEAN
fix_bad_pointer (CNSTRING key, RECORD rec, NODE node)
{
	key=key; /* unused */
	rec=rec; /* unused */

	if (todo.fix_alter_pointers) {
		change_node_tag(node, "_badptr");
		return TRUE;
	}
	return FALSE;
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
		while (i<element_ikey(el)) {
			report_error(ERR_UNDELETED
				, _("Missing undeleted record %c%d")
				, ctype, i);
			if (todo.fix_deletes) {
				char key[33];
				sprintf(key, "%c%ld", ctype, i);
				if (mark_deleted_record_as_deleted(key)) {
					report_fix(ERR_UNDELETED
						, _("Fixed missing undeleted record %c%d")
						, ctype, i);
				} else {
					report_error(-1
						, _("Failed to fix missing undeleted record %c%d")
						, ctype, i);
				}
			}
			i = xref_next(ctype, i);
		}
		if (i == element_ikey(el)) {
			/* in synch */
			i = xref_next(ctype, i);
		} else { /* spri(el) < i */
			CNSTRING key = element_skey(el);
			report_error(ERR_DELETED
				, _("Delete set contains valid record %s")
				, key);
			if (todo.fix_deletes) {
				if (mark_live_record_as_live(key)) {
					report_fix(ERR_DELETED
						, _("Fixed delete set contains valid record %s")
						, key);
				} else {
					report_error(-1
						, _("Failed to fix missing undeleted record %s")
						, key);
				}
			}
			/* fall thru and only advance seq */
		}
	ENDINDISEQ
}
/*=========================================
 * check_btree -- Validate btree itself
 * Created: 2003/09/05, Perry Rapp
 *=======================================*/
static BOOLEAN
check_btree (BTREE btr)
{
	/* keep track of which files we've visited */
	TABLE fkeytab = create_table_int();
	/* check that master index has its fkey correct */
	INDEX index = bmaster(BTR);
	INT mk1 = bkfile(btr).k_mkey;
	if (ixself(index) != mk1) {
		printf(_("Master fkey misaligned"));
		printf("(%ld != %ld\n", ixself(index), mk1);
		return FALSE;
	}
	check_index(btr, index, fkeytab, NULL, NULL);
	destroy_table(fkeytab);
	return TRUE;
}
/*=========================================
 * check_index -- Validate one index node of btree
 * Created: 2003/09/05, Perry Rapp
 *=======================================*/
static BOOLEAN
check_index (BTREE btr, INDEX index, TABLE fkeytab, RKEY * lo, RKEY * hi)
{
	INT n = nkeys(index);
	INT i;
	if (!check_keys((BLOCK)index, lo, hi))
		return FALSE;
	for (i = 0; i <= n; i++) {
		INDEX newix=0;
		char scratch[200];
		FKEY fkey = fkeys(index, i);
		RKEY *lox, *hix;

		get_index_file(scratch, btr, fkey);
		if (in_table(fkeytab, scratch)) {
			printf(_("Cycle in indexes, file %s found again!\n"), scratch);
			return FALSE;
		} else {
			insert_table_int(fkeytab, scratch, 1);
		}
		newix = readindex(btr, fkey, TRUE);
		if (!newix) {
			printf(_("Error loading index at key"));
			printf("%ld\n", i);
			printblock((BLOCK)index);
		}
		/* figure upper & lower bounds of what keys should be in the child */
		lox = (i==0 ? lo : &rkeys(index, i));
		hix = (i==n ? hi : &rkeys(index, i+1));
		if (ixtype(newix) == BTINDEXTYPE) {
			if (!check_index(btr, newix, fkeytab, lox, hix))
				return FALSE;
		} else {
			if (!check_block((BLOCK)newix, lox, hix))
				return FALSE;
		}
	}
	/* TODO: use fkeytab */
	return TRUE;
}
/*=========================================
 * check_block -- Validate one data block node of btree
 * Created: 2003/09/05, Perry Rapp
 *=======================================*/
static BOOLEAN
check_block (BLOCK block, RKEY * lo, RKEY * hi)
{
	if (!check_keys(block, lo, hi))
		return FALSE;
	return TRUE;
}
/*=========================================
 * check_keys -- Validate keys of index or block
 * Created: 2003/09/05, Perry Rapp
 *=======================================*/
static BOOLEAN
check_keys (BLOCK block, RKEY * lo, RKEY * hi)
{
	INT n = nkeys(block);
	INT i = 0;
	INT start = 0;
	BOOLEAN ok = TRUE;
	if (ixtype(block) == BTINDEXTYPE)
		++start; /* keys are 1..n for index */
	else
		--n; /* keys are 0..n-1 for block */
	for (i=start ; i <= n; i++) {
		if (i==start && lo) {
			INT rel = cmpkeys(lo, &rkeys(block, i));
			if (rel < 0) {
				printf(_("First key in block below parent's limit\n"));
				printblock(block);
				ok = FALSE;
			}
		}
		if (i==n && hi) {
			INT rel = cmpkeys(&rkeys(block, i), hi);
			if (rel > 0) {
				printf(_("Last key in block above parent's limit\n"));
				printblock(block);
				ok = FALSE;
			}
		}
		if (i<n) {
			INT rel = cmpkeys(&rkeys(block, i), &rkeys(block, i+1));
			if (rel >= 0) {
				printf(_("Key not below next key"));
				printf(": %ld\n", i);
				printblock(block);
				ok = FALSE;
			}
		}
	}
	return ok;
}
/*=========================================
 * printblock -- Describe index file
 * Created: 2003/09/05, Perry Rapp
 *=======================================*/
static void
printblock (BLOCK block)
{
	FKEY fkey = ixself(block);
	STRING tname = _("data block");
	if (ixtype(block) == BTINDEXTYPE)
		tname = _("data block");
	printf(_("%s fkey=%d, file=%s"), tname, fkey, fkey2path(fkey));
}
/*=========================================
 * validate_errs -- Validate the errs array
 * (fatal assert if finds problem)
 *=======================================*/
static void
validate_errs (void)
{
	int i;
	for (i=0; i<ARRSIZE(errs); i++) {
		if (errs[i].err != i) {
			fprintf(stderr
				, _("Invalid errs array[%d] in dbverify - fix program\n")
				, i);
			print_version("dbverify");
			FATAL();
		}
	}
}
/*=========================================
 * check_missing_data_records -- Check for records
 *  present in index but not in data
 *=======================================*/
static void
check_missing_data_records (void)
{
	check_typed_missing_data_records('I');
	check_typed_missing_data_records('F');
	check_typed_missing_data_records('S');
	check_typed_missing_data_records('E');
	check_typed_missing_data_records('X');
}
/*=========================================
 * check_typed_missing_data_records -- Check all records 
 *  of specified type for orphans
 *=======================================*/
static void
check_typed_missing_data_records (char ntype)
{
	int keynum=0;
	while (TRUE) {
		char key[33];
		keynum = xref_next(ntype, keynum);
		if (!keynum) return;
		sprintf(key, "%c%d", ntype, keynum);
		if (noisy)
			report_progress("Check data record presence: %s", key);
		if (is_record_missing_data_entry(key)) {
			if (todo.pass == 1) {
				report_error(ERR_MISSINGREC, _("Missing data record (%s)"), key);
			} else {
				if (todo.fix_missing_data_records) {
					delete_record_missing_data_entry(key);
					report_fix(ERR_MISSINGREC, _("Fixed missing data record (%s)"), key);
				}
			}
		}
	}
}
/*===========================================
 * main -- Main procedure of dbverify command
 *=========================================*/
int
main (int argc, char **argv)
{
	char *flags, *dbname;
	char *ptr;
	char * msg;
	BOOLEAN cflag=FALSE; /* create new db if not found */
	INT writ=1; /* request write access to database */
	BOOLEAN immut=FALSE; /* immutable access to database */
	BOOLEAN allchecks=FALSE; /* if user requested all checks */
	INT returnvalue=1;
	STRING crashlog=NULL;
	INT lldberrnum=0;
	int i=0;

	/* initialize all the low-level library code */
	init_stdlib();

	validate_errs();

#if HAVE_SETLOCALE
	/* initialize locales */
	setlocale(LC_ALL, "");
#endif /* HAVE_SETLOCALE */

#if ENABLE_NLS
	ll_bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	save_original_locales();

#ifdef WIN32
	/* TO DO - research if this is necessary */
	_fmode = O_BINARY;	/* default to binary rather than TEXT mode */
#endif

	/* handle conventional arguments --version and --help */
	/* needed for help2man to synthesize manual pages */
	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "--version")) {
			print_version("dbverify");
			return 0;
		}
		if (!strcmp(argv[i], "--help")
			|| !strcmp(argv[i], "-?")) {
			print_usage();
			return 0;
		}
	}

	if (argc != 3 || argv[1][0] != '-' || argv[1][1] == '\0') {
		print_usage();
		goto done;
	}
	flags = argv[1];
	dbname = argv[2];
	for (ptr=&flags[1]; *ptr; ++ptr) {
		switch(*ptr) {
		case 'l': todo.check_dbstructure=TRUE; break;
		case 'g': todo.find_ghosts=TRUE; break;
		case 'G': todo.fix_ghosts=TRUE; break;
		case 'i': todo.check_indis=TRUE; break;
		case 'f': todo.check_fams=TRUE; break;
		case 's': todo.check_sours=TRUE; break;
		case 'e': todo.check_evens=TRUE; break;
		case 'x': todo.check_othes=TRUE; break;
		case 'n': noisy=TRUE; break;
		case 'a': allchecks=TRUE; break;
		case 'F': todo.fix_alter_pointers=TRUE; break;
		case 'm': todo.check_missing_data_records=TRUE; break;
		case 'M': todo.fix_missing_data_records=TRUE; break;
		case 'D': todo.fix_deletes=TRUE; break;
		case 'v': print_version("llexec"); goto done;
		case 'h':
		default: print_usage(); goto done;
		}
	}

	/* Turn off Memory Debugging */
	/* This is unnecessary -- it is off anyway, Perry, 2001/10/28 */
	alloclog  = FALSE;

	/* Enable any checks needed for fixes selected */
	if (todo.fix_missing_data_records)
		todo.check_missing_data_records = 1;

	/* initialize options & misc. stuff */
	llgettext_set_default_localedir(LOCALEDIR);
	if (!init_lifelines_global(0, &msg, 0)) {
		printf("%s\n", msg);
		goto done;
	}

	/* setup crashlog in case init_screen fails (eg, bad menu shortcuts) */
	crashlog = getlloptstr("CrashLog_dbverify", NULL);
	if (!crashlog) { crashlog = "Crashlog_dbverify.log"; }
	crash_setcrashlog(crashlog);

	/* NB: This assumes btree database */
	if (!(BTR = bt_openbtree(dbname, cflag, writ, immut, &lldberrnum))) {
		char buffer[256];
		describe_dberror(lldberrnum, buffer, ARRSIZE(buffer));
		puts(buffer);
		goto done;
	}

	/* 
	Do low-level checks before initializing lifelines database layer,
	because lifelines database init traverses btree for user options 
	and translation tables, and here we check the infrastructure of
	the btree itself.
	*/
	if (todo.check_dbstructure || allchecks) {
		if (!check_btree(BTR))
			goto done;
	}

	if (!init_lifelines_postdb()) {
		printf(_(qSbaddb));
		goto done;
	}
	printf(_("Checking %s"), dbname);
	puts("");

	/* all checks - if any new ones, have to update this */
	if (allchecks) {
		todo.check_indis=todo.check_fams=todo.check_sours=TRUE;
		todo.check_evens=todo.check_othes=TRUE;
		todo.find_ghosts=TRUE;
	}

	if (todo.find_ghosts || todo.fix_ghosts)
		check_ghosts();


	if (!(bwrite(BTR))) {
		todo.fix_alter_pointers = FALSE;
		todo.fix_ghosts = FALSE;
	}

	if (todo.check_indis
		|| todo.check_fams
		|| todo.check_sours
		|| todo.check_evens
		|| todo.check_othes) {
		check_and_fix_records();
	}

	if (todo.check_missing_data_records) {
		check_missing_data_records();
	}

	report_results();

	closebtree(BTR);

	/* TODO: probably should call lldb_close, Perry 2005-10-07 */

	BTR = 0;
	returnvalue = 0;

done:
	return returnvalue;
}
/*===============================================
 * report_results -- Print out error & fix counts
 * Created: 2001/01/13, Perry Rapp
 *=============================================*/
static void
report_results (void)
{
	INT i, ct=0;
	for (i=0; i<ARRSIZE(errs); i++) {
		if (errs[i].err_count || errs[i].fix_count) {
			char buffer[64];
			printf("%s: ", _(errs[i].desc));
			snprintf(buffer, sizeof(buffer)
				, _pl("%d error", "%d errors", errs[i].err_count)
				, errs[i].err_count);
			printf("%s, ", buffer);
			snprintf(buffer, sizeof(buffer)
				, _pl("%d fixed", "%d fixed", errs[i].fix_count)
				, errs[i].fix_count);
			printf("%s\n", buffer);
			ct++;
		}
	}
	if (!ct) {
		printf("%s\n", _("No errors found"));
	}
}
/*=============================
 * fatal -- Fatal error routine
 * Created: 2001/01/01, Perry Rapp
 *  handles null or empty details input
 *===========================*/
void
__fatal (STRING file, int line, CNSTRING details)
{
	/* avoid reentrancy */
	static BOOLEAN failing=FALSE;
	if (failing) return;
	failing=TRUE;

	/* send to error log if one is specified */
	errlog_out(_("Fatal Error"), details, file, line);

	printf(_("FATAL ERROR: "));
	if (details && details[0]) {
		printf(details);
	}
	printf("\n");
	printf(_("In file <%s> at line %d"), file, line);
	printf("\n");

	/* offer crash dump before closing database */
	ll_optional_abort(_("ASSERT failure"));

	failing=FALSE;
	exit(1);
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
 * printkey -- Displayable version of key
 *  same as key, except for NULL keys
 *=============================*/
static CNSTRING
printkey (CNSTRING key)
{
	return key ? key : "NULL";
}
