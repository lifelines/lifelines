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
/*=============================================================
 * xreffile.c -- Handle the xref file
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 02 May 94    3.0.2 - 10 Nov 94
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btree.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "table.h"
#include "translat.h"

extern BTREE BTR;


/*===================================================================
 * First five words in xrefs file are number of INDI, FAM, EVEN, SOUR
 *   and other keys in file; remaining words are keys, in respective
 *   order, for the records; first in each group is next unused key;
 *   rest are keys of deleted records
 * nixrefs==1 means there are no deleted INDI keys
 * nixrefs==2 means there is one deleted INDI key (ixrefs[1])
 *=================================================================*/

/*********************************************
 * local types
 *********************************************/

 /*==================================== 
 * deleteset -- set of deleted records
 *  NB: storage order is IFESX
 *  whereas canonical order is IFSEX
 *==================================*/
struct deleteset_s
{
	INT n; /* num keys + 1, ie, starts at 1 */
	INT * recs;
	INT max;
	char ctype;
};
typedef struct deleteset_s *DELETESET;


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_xref_to_set(INT key, DELETESET set);
static void freexref(DELETESET set);
static STRING getxref(DELETESET set);
static void growxrefs(DELETESET set);
static STRING newxref(STRING xrefp, BOOLEAN flag, DELETESET set);
static INT num_set(DELETESET set);
static void readrecs(DELETESET set);
static BOOLEAN readxrefs(void);
static INT xref_last(DELETESET set);

/*********************************************
 * local variables
 *********************************************/

/* INDI, FAM, EVEN, SOUR, other sets */
static struct deleteset_s irecs, frecs, srecs, erecs, xrecs;

static FILE *xreffp=0;	/* open xref file pointer */
static BOOLEAN xrefReadonly = FALSE;

static INT maxkeynum=-1; /* cache value of largest key extant (-1 means not sure) */

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==================================== 
 * initdset -- Initialize a delete set
 *==================================*/
static void
initdset (DELETESET set, char ctype)
{
	set->ctype = ctype;
	set->max = 0;
	set->n = 1;
	set->recs = 0;
}
/*=================================== 
 * initdsets -- Initialize delete sets
 *=================================*/
static void
initdsets (void)
{
	initdset(&irecs, 'I');
	initdset(&frecs, 'F');
	initdset(&srecs, 'S');
	initdset(&erecs, 'E');
	initdset(&xrecs, 'X');
}
/*============================== 
 * initxref -- Create xrefs file
 *============================*/
void
initxref (void)
{
	char scratch[100];
	INT i = 1, j;
	ASSERT(!xrefReadonly);
	initdsets();
	ASSERT(!xreffp);
	sprintf(scratch, "%s/xrefs", BTR->b_basedir);
	ASSERT(xreffp = fopen(scratch, LLWRITEBINARY));
	for (j = 0; j < 10; j++) {
		ASSERT(fwrite(&i, sizeof(INT), 1, xreffp) == 1);
	}
	fclose(xreffp); xreffp=0;
}
/*============================
 * openxref -- Open xrefs file
 *==========================*/
BOOLEAN
openxref (BOOLEAN readonly)
{
	char scratch[100];
	STRING fmode;

	initdsets();
	ASSERT(!xreffp);
	sprintf(scratch, "%s/xrefs", BTR->b_basedir);
	xrefReadonly = readonly;
	fmode = xrefReadonly ? LLREADBINARY : LLREADBINARYUPDATE;
	if (!(xreffp = fopen(scratch, fmode))) {
		return FALSE;
	}
	return readxrefs();
}
/*==============================
 * closexref -- Close xrefs file
 *  (Safe to call if not open)
 *============================*/
void
closexref (void)
{
	if (xreffp) {
		fclose(xreffp); xreffp = 0;
	}
	freexref(&irecs);
	freexref(&frecs);
	freexref(&srecs);
	freexref(&erecs);
	freexref(&xrecs);
}
/*=========================================
 * getxrefnum -- Return new keynum for type
 *  from deleted list if available, or else
 *  a new highnumber
 *  generic for all 5 types
 * Created: 2001/02/04, Perry Rapp
 *=======================================*/
static INT
getxrefnum (DELETESET set)
{
	INT keynum;
	ASSERT(xreffp && set->n >= 1);
	keynum = (set->n == 1) ? set->recs[0]++ : set->recs[--(set->n)];
	ASSERT(writexrefs());
	maxkeynum=-1;
	return keynum;
}
/*=========================================
 * getxref -- Return new key pointer for type
 *  from deleted list if available, or else
 *  a new highnumber
 *  generic for all 5 types
 *=======================================*/
static STRING
getxref (DELETESET set)
{
	INT keynum = getxrefnum(set);
	static char scratch[12];
	sprintf(scratch, "@%c%d@", set->ctype, keynum);
	return scratch;
}
/*===================================================
 * get?xref -- Wrappers for each type to getxref (qv)
 *  symmetric versions
 *=================================================*/
STRING getfxref (void) { return getxref(&frecs); }
STRING getsxref (void) { return getxref(&srecs); }
STRING getexref (void) { return getxref(&erecs); }
STRING getxxref (void) { return getxref(&xrecs); }
/*===================================================
 * get?xrefnum -- Wrappers for each type to getxrefnum (qv)
 * Created: 2001/02/04, Perry Rapp
 *=================================================*/
INT getixrefnum (void) { return getxrefnum(&irecs); }
/*======================================
 * sortxref -- Sort xrefs after reading
 *====================================*/
static void
sortxref (DELETESET set)
{
	/*
	TO DO - call qsort instead
	and also flag sorted file by changing file structure
	*/
	/*
	sort from high to low, so lowest at top of
	array, ready to be handed out

	they should normally already be sorted, 
	so use watchful bubble-sort for O(n)
	*/
	INT i,j, temp, ct;
	for (i=1; i<set->n; i++) {
		ct=0;
		for (j=i+1; j<set->n; j++) {
			if (set->recs[i] < set->recs[j]) {
				ct++;
				temp = set->recs[j];
				set->recs[j] = set->recs[i];
				set->recs[i] = temp;
			}
			if (i==1 && !ct) return; /* already sorted */
		}
	}
}
/*======================================
 * sortxrefs -- Sort xrefs after reading
 *====================================*/
static void
sortxrefs (void)
{
	sortxref(&irecs);
	sortxref(&frecs);
	sortxref(&srecs);
	sortxref(&erecs);
	sortxref(&xrecs);
}
/*=============================
 * readxrefs -- Read xrefs file
 *  storage order: IFESX
 *===========================*/
static BOOLEAN
readxrefs (void)
{
	ASSERT(xreffp);
	ASSERT(fread(&irecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&frecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&erecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&srecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fread(&xrecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(irecs.n > 0);
	ASSERT(frecs.n > 0);
	ASSERT(erecs.n > 0);
	ASSERT(srecs.n > 0);
	ASSERT(xrecs.n > 0);
	if (irecs.n > irecs.max) growxrefs(&irecs);
	if (frecs.n > frecs.max) growxrefs(&frecs);
	if (srecs.n > srecs.max) growxrefs(&srecs);
	if (erecs.n > erecs.max) growxrefs(&erecs);
	if (xrecs.n > xrecs.max) growxrefs(&xrecs);
	readrecs(&irecs);
	readrecs(&frecs);
	readrecs(&erecs);
	readrecs(&srecs);
	readrecs(&xrecs);
	sortxrefs();
	return TRUE;
}
/*=========================================
 * readrecs -- Read in one array of records
 *=======================================*/
static void
readrecs (DELETESET set)
{
	ASSERT((INT)fread(set->recs, sizeof(INT), set->n, xreffp) == set->n);
}
/*================================
 * writexrefs -- Write xrefs file.
 *  storage order: IFESX
 *==============================*/
BOOLEAN
writexrefs (void)
{
	ASSERT(!xrefReadonly);
	ASSERT(xreffp);
	rewind(xreffp);
	ASSERT(fwrite(&irecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&frecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&erecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&srecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT(fwrite(&xrecs.n, sizeof(INT), 1, xreffp) == 1);
	ASSERT((INT)fwrite(irecs.recs, sizeof(INT), irecs.n, xreffp) == irecs.n);
	ASSERT((INT)fwrite(frecs.recs, sizeof(INT), frecs.n, xreffp) == frecs.n);
	ASSERT((INT)fwrite(erecs.recs, sizeof(INT), erecs.n, xreffp) == erecs.n);
	ASSERT((INT)fwrite(srecs.recs, sizeof(INT), srecs.n, xreffp) == srecs.n);
	ASSERT((INT)fwrite(xrecs.recs, sizeof(INT), xrecs.n, xreffp) == xrecs.n);
	fflush(xreffp);
	return TRUE;
}
/*=====================================
 * add_xref_to_set -- Add deleted key to xrefs.
 *  generic for all types
 *===================================*/
static void
add_xref_to_set (INT key, DELETESET set)
{
	INT lo,hi,md, i;
	if (key <= 0 || !xreffp || (set->n) < 1) FATAL();
	if (set->n >= set->max)
		growxrefs(set);
	ASSERT(set->n < set->max);
	lo=1;
	hi=(set->n)-1;
	/* binary search to find where to insert key */
	while (lo<=hi) {
		md = (lo + hi)/2;
		if (key>(set->recs)[md])
			hi=--md;
		else if (key<(set->recs)[md])
			lo=++md;
		else {
			char msg[64];
			sprintf(msg, "Tried to add already-deleted record (%d) to xref (%c)!"
				, key, set->ctype);
			FATAL2(msg); /* deleting a deleted record! */
		}
	}
	/* key replaces xrefs[lo] - push lo+ up */
	for (i=set->n-1; i>=lo; --i)
		(set->recs)[i+1] = (set->recs)[i];
	(set->recs)[lo] = key;
	(set->n)++;
	ASSERT(writexrefs());
	maxkeynum=-1;
}
/*===================================================
 * add?xref -- Wrappers for each type to add_xref_to_set (qv)
 *  5 symmetric versions
 *=================================================*/
void addixref (INT key) { add_xref_to_set(key, &irecs); }
void addfxref (INT key) { add_xref_to_set(key, &frecs); }
void addsxref (INT key) { add_xref_to_set(key, &srecs); }
void addexref (INT key) { add_xref_to_set(key, &erecs); }
void addxxref (INT key) { add_xref_to_set(key, &xrecs); }
/*===================================================
 * addxref -- Mark key free (accepts string key, any type)
 *=================================================*/
void addxref (CNSTRING key)
{
	INT keyint = atoi(key + 1);
	switch(key[0]) {
	case 'I': addixref(keyint); break;
	case 'F': addfxref(keyint); break;
	case 'S': addsxref(keyint); break;
	case 'E': addexref(keyint); break;
	case 'X': addxxref(keyint); break;
	default: ASSERT(0); break;
	}
}
/*==========================================
 * growxrefs -- Grow memory for xrefs array.
 *  generic for all types
 *========================================*/
static void
growxrefs (DELETESET set)
{
	INT i, m = set->max, *newp;
	if (set->max == 0)
		set->max = 64;
	while (set->max <= set->n)
		set->max = set->max << 1;
	newp = (INT *) stdalloc((set->max)*sizeof(INT));
	if (m) {
		for (i = 0; i < set->n; i++)
			newp[i] = set->recs[i];
		stdfree(set->recs);
	}
	set->recs = newp;
}
/*==========================================
 * freexref -- Free memory & clear xrefs array
 *  Called when database is closed
 *========================================*/
static void
freexref (DELETESET set)
{
	ASSERT(set);
	if (set->recs) {
		stdfree(set->recs);
		set->recs = 0;
		set->max = 0;
		set->n = 0;
	} else {
		ASSERT(set->max == 0);
		ASSERT(set->n == 0);
	}
}
/*==========================================================
 * num_????s -- Return number of type of things in database.
 *  5 symmetric versions
 *========================================================*/
static INT num_set (DELETESET set)
{
	ASSERT(set);
	return set->recs[0] - set->n;
}
INT num_indis (void) { return num_set(&irecs); }
INT num_fams (void) { return num_set(&frecs); }
INT num_sours (void) { return num_set(&srecs); }
INT num_evens (void) { return num_set(&erecs); }
INT num_othrs (void) { return num_set(&xrecs); }
/*========================================================
 * max_????s -- Return max key number of object type in db
 * 5 symmetric versions
 *======================================================*/
static INT max_set (DELETESET set)
{
	return set->recs[0];
}
INT xref_max_indis (void) { return max_set(&irecs); }
INT xref_max_fams (void) { return max_set(&frecs); }
INT xref_max_sours (void) { return max_set(&srecs); }
INT xref_max_evens (void) { return max_set(&erecs); }
INT xref_max_othrs (void) { return max_set(&xrecs); }
/*======================================================
 * xref_max_any -- Return largest key number of any type
 *====================================================*/
INT
xref_max_any (void)
{
	if (maxkeynum>=0)
		return maxkeynum;
	if (xref_max_indis() > maxkeynum)
		maxkeynum = xref_max_indis();
	if (xref_max_fams() > maxkeynum)
		maxkeynum = xref_max_fams();
	if (xref_max_sours() > maxkeynum)
		maxkeynum = xref_max_sours();
	if (xref_max_evens() > maxkeynum)
		maxkeynum = xref_max_evens();
	if (xref_max_othrs() > maxkeynum)
		maxkeynum = xref_max_othrs();
	return maxkeynum;
}
/*================================================
 * newxref -- Return original or next xref value
 * xrefp = key of the individual
 * flag = use the current key
 *  returns static buffer
 *==============================================*/
STRING
newxref (STRING xrefp, BOOLEAN flag, DELETESET set)
{
	INT keynum;
	BOOLEAN changed;
	static char scratch[12];
	if(flag) {
		keynum = atoi(xrefp+1);
		changed = ((set->n != 1) || (keynum >= set->recs[0]));
		if(set->n != 1)
			set->n = 1;	/* forget about deleted entries */
		if(keynum >= set->recs[0])
			set->recs[0] = keynum+1;	/* next available */
		if(changed)
			ASSERT(writexrefs());
		sprintf(scratch, "@%s@", xrefp);
		return(scratch);
	}
	return(getxref(set));
}
/*================================================
 * newixref -- Return original or next ixref value
 * xrefp = key of the individual
 * flag = use the current key
 *==============================================*/
STRING
newixref (STRING xrefp, BOOLEAN flag)
{
	return newxref(xrefp, flag, &irecs);
}
/*================================================
 * newfxref -- Return original or next fxref value
 * xrefp = key of the individual
 * flag = use the current key
 *==============================================*/
STRING
newfxref (STRING xrefp, BOOLEAN flag)
{
	return newxref(xrefp, flag, &frecs);
}
/*================================================
 * newsxref -- Return original or next sxref value
 * xrefp = key of the individual
 * flag = use the current key
 *==============================================*/
STRING
newsxref (STRING xrefp, BOOLEAN flag)
{
	return newxref(xrefp, flag, &srecs);
}
/*================================================
 * newexref -- Return original or next exref value
 * xrefp = key of the individual
 * flag = use the current key
 *==============================================*/
STRING
newexref (STRING xrefp, BOOLEAN flag)
{
	return newxref(xrefp, flag, &erecs);
}
/*================================================
 * newxxref -- Return original or next xxref value
 * xrefp = key of the individual
 * flag = use the current key
 *==============================================*/
STRING
newxxref (STRING xrefp, BOOLEAN flag)
{
	return newxref(xrefp, flag, &xrecs);
}
/*================================================
 * xref_isvalid_impl -- is this a valid whatever ?
 *  generic for all 5 types
 * (internal use)
 *==============================================*/
static BOOLEAN
xref_isvalid_impl (DELETESET set, INT keynum)
{
	INT lo,hi,md;
	if (set->n == set->recs[0]) return FALSE; /* no valids */
	if (set->n == 1) return TRUE; /* all valid */
	/* binary search deleteds */
	lo=1;
	hi=(set->n)-1;
	while (lo<=hi) {
		md = (lo + hi)/2;
		if (keynum>(set->recs)[md])
			hi=--md;
		else if (keynum<(set->recs)[md])
			lo=++md;
		else
			return FALSE;
	}
	return TRUE;
}
/*=========================================================
 * xref_next_impl -- Return next valid of some type after i
 *  returns 0 if none found
 *  generic for all 5 types
 *  this could be more efficient (after first one work
 *  thru tree)
 *=======================================================*/
static INT
xref_next_impl (DELETESET set, INT i)
{
	if (set->n == set->recs[0]) return 0; /* no valids */
	while (++i < set->recs[0])
	{
		if (xref_isvalid_impl(set, i)) return i;
	}
	return 0;
}
/*==========================================================
 * xref_prev_impl -- Return prev valid of some type before i
 *  returns 0 if none found
 *  generic for all 5 types
 *========================================================*/
static INT
xref_prev_impl (DELETESET set, INT i)
{
	if (set->n == set->recs[0]) return 0; /* no valids */
	while (--i)
	{
		if (xref_isvalid_impl(set, i)) return i;
	}
	return 0;
}
/*===============================================
 * xref_next? -- Return next valid indi/? after i
 *  returns 0 if none found
 *  5 symmetric versions
 *=============================================*/
INT xref_nexti (INT i) { return xref_next_impl(&irecs, i); }
INT xref_nextf (INT i) { return xref_next_impl(&frecs, i); }
INT xref_nexts (INT i) { return xref_next_impl(&srecs, i); }
INT xref_nexte (INT i) { return xref_next_impl(&erecs, i); }
INT xref_nextx (INT i) { return xref_next_impl(&xrecs, i); }
INT xref_next (char ntype, INT i)
{
	switch(ntype) {
	case 'I': return xref_nexti(i);
	case 'F': return xref_nextf(i);
	case 'S': return xref_nexts(i);
	case 'E': return xref_nexte(i);
	case 'X': return xref_nextx(i);
	}
	ASSERT(0); return 0;
}
/*================================================
 * xref_prev? -- Return prev valid indi/? before i
 *  returns 0 if none found
 *  5 symmetric versions
 *==============================================*/
INT xref_previ (INT i) { return xref_prev_impl(&irecs, i); }
INT xref_prevf (INT i) { return xref_prev_impl(&frecs, i); }
INT xref_prevs (INT i) { return xref_prev_impl(&srecs, i); }
INT xref_preve (INT i) { return xref_prev_impl(&erecs, i); }
INT xref_prevx (INT i) { return xref_prev_impl(&xrecs, i); }
INT xref_prev (char ntype, INT i)
{
	switch(ntype) {
	case 'I': return xref_previ(i);
	case 'F': return xref_prevf(i);
	case 'S': return xref_prevs(i);
	case 'E': return xref_preve(i);
	case 'X': return xref_prevx(i);
	}
	ASSERT(0); return 0;
}
/*=========================================
 * xref_first? -- Return first valid indi/?
 *  returns 0 if none found
 *  5 symmetric versions
 *=======================================*/
INT xref_firsti (void) { return xref_nexti(0); }
INT xref_firstf (void) { return xref_nextf(0); }
INT xref_firsts (void) { return xref_nexts(0); }
INT xref_firste (void) { return xref_nexte(0); }
INT xref_firstx (void) { return xref_nextx(0); }
/*=======================================
 * xref_last? -- Return last valid indi/?
 *  returns 0 if none found
 *  5 symmetric versions
 *=====================================*/
static INT xref_last (DELETESET set)
{
	return xref_prev_impl(set, set->recs[0]);
}
INT xref_lasti (void) { return xref_last(&irecs); }
INT xref_lastf (void) { return xref_last(&frecs); }
INT xref_lasts (void) { return xref_last(&srecs); }
INT xref_laste (void) { return xref_last(&erecs); }
INT xref_lastx (void) { return xref_last(&xrecs); }
/*=======================================
 * xrefs_get_counts_from_unopened_db --
 *  read record counts out of file on disk
 * returns FALSE if specified path is not the root of a traditional lifelines database
 *=====================================*/
BOOLEAN
xrefs_get_counts_from_unopened_db (CNSTRING path, INT *nindis, INT *nfams
	, INT *nsours, INT *nevens, INT *nothrs)
{
	char scratch[100];
	STRING fmode;
	FILE * fp = 0;
	INT i;
	INT ndels[5], nmax[5];

	ASSERT(!xreffp);
	sprintf(scratch, "%s/xrefs", path);
	fmode = LLREADBINARY;
	if (!(fp = fopen(scratch, fmode))) {
		return FALSE;
	}
	for (i=0; i<5; ++i) {
		ASSERT(fread(&ndels[i], sizeof(INT), 1, fp) == 1);
	}
	for (i=0; i<5; ++i) {
		INT j;
		for (j=0; j<ndels[i]; ++j) {
			INT k;
			ASSERT(fread(&k, sizeof(INT), 1, fp) == 1);
			if (!j)
				nmax[i] = k;
		}
	}
	*nindis = nmax[0] - ndels[0];
	*nfams = nmax[1] - ndels[1];
	*nevens = nmax[2] - ndels[2];
	*nsours = nmax[3] - ndels[3];
	*nothrs = nmax[4] - ndels[4];
	fclose(fp);
	return TRUE;
}

