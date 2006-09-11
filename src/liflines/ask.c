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
 * ask.c -- Interact with user for various reasons
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 24 Aug 93
 *   2.3.6 - 30 Oct 93    3.0.0 - 19 Aug 94
 *   3.0.2 - 02 Dec 94
 *===========================================================*/
/* modified 2000-01-26 J.F.Chandler */
/* modified 2000-04-25 J.F.Chandler */

#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "llinesi.h"
#include "feedback.h"


/*********************************************
 * external/imported variables
 *********************************************/

extern INT listbadkeys;
extern char badkeylist[];

extern STRING qSntchld,qSntprnt,qSidfbrs,qSentnam;
extern STRING qSnotonei,qSnotonex,qSifonei,qSifonex;
extern STRING qSnofopn,qSidbrws,qSwhtfname,qSwhtfnameext;
extern STRING qSnonamky,qSparadox,qSaskint,qSmisskeys,qSbadkeyptr;
extern STRING qSfn2long,qSidkyrfn,qSduprfn;

/*********************************************
 * local function prototypes
 *********************************************/

static RECORD ask_for_any_once(STRING ttl, char ctype, ASK1Q ask1, INT *prc);
static void make_fname_prompt(STRING fnamebuf, INT len, STRING ext);

/*=====================================================
 * ask_for_fam_by_key -- Ask user to identify family by 
 *  key (or REFN)
 *  (if they enter nothing, it will fall thru to ask_for_fam)
 *  fttl: [IN]  title for prompt
 *  pttl: [IN]  title for prompt to identify spouse
 *  sttl: [IN]  title for prompt to identify sibling
 *========================================================*/
RECORD
ask_for_fam_by_key (STRING fttl, STRING pttl, STRING sttl)
{
	RECORD fam = ask_for_record(fttl, 'F');
	return fam ? fam : ask_for_fam(pttl, sttl);
}
/*===========================================
 * ask_for_fam -- Ask user to identify family by spouses
 *  pttl: [IN]  title for prompt to identify spouse
 *  sttl: [IN]  title for prompt to identify sibling
 *=========================================*/
RECORD
ask_for_fam (STRING pttl, STRING sttl)
{
	RECORD sib=0, prn=0;
	prn = ask_for_indi(pttl, DOASK1);
	if (!prn)  {
		NODE fam=0;
		RECORD frec=0;
		sib = ask_for_indi(sttl, DOASK1);
		if (!sib) return NULL;
		fam = FAMC(nztop(sib));
		if (!fam) {
			message(_(qSntchld));
			return NULL;
		}
		frec = key_to_frecord(rmvat(nval(fam)));
		return frec;
	}
	if (!FAMS(nztop(prn))) {
		message(_(qSntprnt));
		return NULL;
	}
	return choose_family(prn, _(qSparadox), _(qSidfbrs), TRUE);
}
/*===========================================
 * ask_for_int -- Ask user to provide integer
 * titl: [IN]  prompt title
 * TODO: change to BOOLEAN return for failure
 *=========================================*/
BOOLEAN
ask_for_int (STRING ttl, INT * prtn)
{
	INT ival, c, neg;
	char buffer[MAXPATHLEN];
	while (TRUE) {
		STRING p = buffer;
		if (!ask_for_string(ttl, _(qSaskint), buffer, sizeof(buffer)))
			return FALSE;
		neg = 1;
		while (iswhite(*p++))
			;
		--p;
		if (*p == '-') {
			neg = -1;
			p++;
			while (iswhite(*p++))
				;
			--p;
		}
		if (chartype(*p) == DIGIT) {
			ival = *p++ - '0';
			while (chartype(c = *p++) == DIGIT)
				ival = ival*10 + c - '0';
			--p;
			while (iswhite(*p++))
				;
			--p;
			if (*p == 0) {
				*prtn = ival*neg;
				return TRUE;
			}
		}
	}
}
/*======================================
 * ask_for_file_worker -- Ask for and open file
 *  ttl:       [IN]  title of question (1rst line)
 *  pfname     [OUT] file as user entered it (optional param)
 *  pfullpath  [OUT] file as found (optional param)
 * pfname & pfulllpath are heap-allocated
 *====================================*/
typedef enum { INPUT, OUTPUT } DIRECTION;
static FILE *
ask_for_file_worker (STRING mode,
                     STRING ttl,
                     STRING *pfname,
                     STRING *pfullpath,
                     STRING path,
                     STRING ext,
                     DIRECTION direction)
{
	FILE *fp;
	char prompt[MAXPATHLEN];
	char fname[MAXPATHLEN];
	int elen, flen;
	BOOLEAN rtn;

	make_fname_prompt(prompt, sizeof(prompt), ext);

	if (direction==INPUT)
		rtn = ask_for_input_filename(ttl, path, prompt, fname, sizeof(fname));
	else
		rtn = ask_for_output_filename(ttl, path, prompt, fname, sizeof(fname));
	
	if (pfname) {
		if (fname && fname[0])
			*pfname = strdup(fname);
		else
			*pfname = 0;
	}
	if (pfullpath) *pfullpath = 0; /* 0 indicates we didn't try to open */

	if (!rtn || !fname[0]) return NULL;

	if (!expand_special_fname_chars(fname, sizeof(fname), uu8)) {
		msg_error(_(qSfn2long));
		return NULL;
	}

ask_for_file_try:

	/* try name as given */
	if (ISNULL(path)) {
		/* bare filename was given */
		if ((fp = fopen(fname, mode)) != NULL) {
			if (pfname)
				strupdate(pfname, fname);
			return fp;
		}
	} else {
		/* fully qualified path was given */
		if ((fp = fopenpath(fname, mode, path, ext, uu8, pfullpath)) != NULL) {
			return fp;
		}
	}

	/* try default extension */
	if (ext) {
		elen = strlen(ext);
		flen = strlen(fname);
		if (elen<flen && path_match(fname+flen-elen, ext)) {
			ext = NULL;	/* the file name has the extension already */
		} else {
			/* add extension and go back and retry */
			llstrapps(fname, sizeof(fname), uu8, ext);
			ext = NULL; /* only append extension once! */
			goto ask_for_file_try;
		}
	}

	/* failed to open it, give up */
	msg_error(_(qSnofopn), fname);
	return NULL;
}
/*======================================
 * make_fname_prompt -- Create prompt line
 *  for filename, depending on extension
 * Created: 2001/12/24, Perry Rapp
 *====================================*/
static void
make_fname_prompt (STRING fnamebuf, INT len, STRING ext)
{
	if (ISNULL(ext)) {
		ext = NULL;	/* a null extension is the same as no extension */
		llstrncpyf(fnamebuf, len, uu8, "%s: ", _(qSwhtfname));
	}
	else {
		llstrncpyf(fnamebuf, len, uu8, _(qSwhtfnameext), ext);
	}
}
/*======================================
 * ask_for_input_file -- Ask for and open file for input
 *  ttl:       [IN]  title of question (1rst line)
 *  pfname     [OUT] file as user entered it (optional param)
 *  pfullpath  [OUT] file as found (optional param)
 *====================================*/
FILE *
ask_for_input_file (STRING mode,
                    STRING ttl,
                    STRING *pfname,
                    STRING *pfullpath,
                    STRING path,
                    STRING ext)
{
	return ask_for_file_worker(mode, ttl, pfname, pfullpath, path, ext, INPUT);
}

/*======================================
 * ask_for_output_file -- Ask for and open file for output
 *  ttl:   [IN]  title of question (1rst line)
 *  pfname [OUT] optional output parameter (pass NULL if undesired)
 *====================================*/
FILE *
ask_for_output_file (STRING mode,
                     STRING ttl,
                     STRING *pfname,
                     STRING *pfullpath,
                     STRING path,
                     STRING ext)
{
	return ask_for_file_worker(mode, ttl, pfname, pfullpath, path, ext, OUTPUT);
}
	/* RC_DONE means user just hit enter -- interpret as a cancel */
#define RC_DONE       0
	/* RC_NOSELECT means user's choice couldn't be found & we gave up (& told them) */
#define RC_NOSELECT   1
	/* RC_SELECT means user chose a valid list (may have only one entry) */
#define RC_SELECT     2
/*=================================================
 * ask_for_indiseq -- Ask user to identify sequence
 *  ttl:   [IN]  prompt (title) to display
 *  ctype: [IN]  type of record (eg, 'I') (0 for any)
 *  prc:   [OUT] result code (RC_DONE, RC_SELECT, RC_NOSELECT)
 *===============================================*/
INDISEQ
ask_for_indiseq (CNSTRING ttl, char ctype, INT *prc)
{
	while (1)
	{
		INDISEQ seq=0;
		char name[MAXPATHLEN];
		*prc = RC_DONE;
		if (!ask_for_string(ttl, _(qSidbrws), name, sizeof(name)))
			return NULL;
		if (!name || *name == 0) return NULL;
		*prc = RC_NOSELECT;
		if (eqstr(name, "@")) {
			seq = invoke_search_menu();
			if (!seq)
				continue; /* fallback to main question above */
			*prc = RC_SELECT;
		} else {
			seq = str_to_indiseq(name, ctype);
			if (seq) {
				*prc = RC_SELECT;
			} else {
				msg_error(_(qSnonamky));
				continue;
			}
		}
		return seq;
	}
}
/*============================================================
 * ask_for_any_once -- Have user identify sequence and select record
 *  ttl:   [IN]  title to present
 *  ctype: [IN]  type of record (eg, 'I') (0 for any, 'B' for any preferring INDI)
 *  ask1:  [IN]  whether to present list if only one matches their desc.
 *  prc:   [OUT] result (RC_DONE, RC_SELECT, RC_NOSELECT)
 *==========================================================*/
static RECORD
ask_for_any_once (STRING ttl, char ctype, ASK1Q ask1, INT *prc)
{
	RECORD indi = 0;
	INDISEQ seq = ask_for_indiseq(ttl, ctype, prc);
	if (*prc == RC_DONE || *prc == RC_NOSELECT) return NULL;
	ASSERT(*prc == RC_SELECT);
	/* user chose a set of possible answers */
	/* might be a single-entry indiseq, but if so still need to confirm */
	ASSERT(*prc == RC_SELECT);
	if (ctype == 'I') {
		indi = choose_from_indiseq(seq, ask1, _(qSifonei), _(qSnotonei));
	} else {
		indi = choose_from_indiseq(seq, ask1, _(qSifonex), _(qSnotonex));
	}
	remove_indiseq(seq);
	*prc = indi ? RC_SELECT : RC_NOSELECT;
	return indi;
}
/*=================================================================
 * ask_for_indi -- Ask user to identify sequence and select person;
 *   reask protocol used
 * ttl:      [in] title for question
 * ask1:     [in] whether to present list if only one matches
 *===============================================================*/
RECORD
ask_for_indi (STRING ttl, ASK1Q ask1)
{
	INT rc = 0;
	RECORD indi = ask_for_any_once(ttl, 'I', ask1, &rc);
	return indi;
}
/*=================================================================
 * ask_for_any -- Ask user to identify sequence and select record
 *   reask protocol used
 * ttl:      [in] title for question
 * confirmq: [in] whether to confirm after choice
 * ask1:     [in] whether to present list if only one matches
 *===============================================================*/
RECORD
ask_for_any (STRING ttl, ASK1Q ask1)
{
	char ctype = 0; /* code for any type */
	while (TRUE) {
		INT rc;
		RECORD record = ask_for_any_once(ttl, ctype, ask1, &rc);
		if (rc == RC_DONE || rc == RC_SELECT)
			return record;
		return NULL;
	}
}
/*===================================================================
 * ask_for_indi_list -- Ask user to identify person sequence
 * reask if true if we should give them another chance if their search hits nothing
 * returns null value indiseq
 * used by both reports & interactive use
 *=================================================================*/
INDISEQ
ask_for_indi_list (STRING ttl, BOOLEAN reask)
{
	while (TRUE) {
		INT rc = RC_DONE;
		INDISEQ seq = ask_for_indiseq(ttl, 'I', &rc);
		if (rc == RC_DONE)
			return NULL;
		if (rc == RC_NOSELECT) {
			if (!reask || !ask_yes_or_no(_(qSentnam)))
				return NULL;
			continue;
		}
		ASSERT(seq);
		rc = choose_list_from_indiseq(_(qSnotonei), seq);
		if (rc == -1) {
			remove_indiseq(seq);
			seq = NULL;
			if (!reask || !ask_yes_or_no(_(qSentnam)))
				return NULL;
		}
		return seq;
	}
}
/*==========================================================
 * ask_for_indi_key -- Have user identify person; return key
 *========================================================*/
STRING
ask_for_indi_key (STRING ttl, ASK1Q ask1)
{
	RECORD indi = ask_for_indi(ttl, ask1);
	if (!indi) return NULL;
	return rmvat(nxref(nztop(indi)));
}
/*===============================================================
 * choose_one_from_indiseq_if_needed  -- handle ask1 cases
 *  seq:   [IN]  sequence from which to choose
 *  ask1:  [IN]  whether to prompt if only one element in sequence
 *  titl1: [IN]  title if sequence has one element
 *  titln: [IN]  title if sequence has multiple elements
 *=============================================================*/
static INT
choose_one_from_indiseq_if_needed (INDISEQ seq, ASK1Q ask1, STRING titl1
	, STRING titln)
{
	if (length_indiseq(seq) > 1)
		return choose_one_from_indiseq(titln, seq);
	else if (ask1==DOASK1 && titl1)
		return choose_one_from_indiseq(titl1, seq);
	return 0;
}
/*======================================================
 * choose_from_indiseq -- Format sequence and have user
 *  choose from it (any type)
 * This handles bad pointers, which can get into the data
 *  several ways.
 *  seq:   [IN]  sequence from which to choose
 *  ask1:  [IN]  whether to prompt if only one element in sequence
 *  titl1: [IN]  title if sequence has one element
 *  titln: [IN]  title if sequence has multiple elements
 *=====================================================*/
RECORD
choose_from_indiseq (INDISEQ seq, ASK1Q ask1, STRING titl1, STRING titln)
{
	INT i = 0;
	RECORD rec=0;

	i = choose_one_from_indiseq_if_needed(seq, ask1, titl1, titln);
	if (i == -1) return NULL;
	listbadkeys=1;
	/* which typed value indiseq is this ? */
	if (!indiseq_is_valtype_ival(seq) && !indiseq_is_valtype_null(seq))
	{
		/* int debug=1; */ /* Can this happen ? */
	}
	if (-1 == get_indiseq_ival(seq, i)) /* invalid pointer */
		badkeylist[0] = 0;
	else {
		CNSTRING skey = element_key_indiseq(seq, i);
		rec = key_to_record(skey);
	}
	listbadkeys = 0;
	if(!rec) {
		char buf[132];
		if (badkeylist[0])
			llstrncpyf(buf, sizeof(buf), uu8, "%s: %.40s", _(qSmisskeys), badkeylist);
		else
			llstrncpyf(buf, sizeof(buf), uu8, _(qSbadkeyptr));
		message(buf);
	}
	return rec;
}
/*===============================================
 * ask_for_record -- Ask user to identify record
 *  lookup by key or by refn (& handle dup refns)
 *  idstr: [IN]  question prompt
 *  letr:  [IN]  letter to possibly prepend to key (ie, I/F/S/E/X)
 *=============================================*/
RECORD
ask_for_record (STRING idstr, INT letr)
{
	RECORD rec;
	char answer[MAXPATHLEN];
	if (!ask_for_string(idstr, _(qSidkyrfn), answer, sizeof(answer))
		|| !answer[0])
		return NULL;

	rec = key_possible_to_record(answer, letr);
	if (!rec) {
		INDISEQ seq;
		seq = refn_to_indiseq(answer, letr, KEYSORT);
		if (!seq) return NULL;
		rec = choose_from_indiseq(seq, NOASK1, _(qSduprfn), _(qSduprfn));
		remove_indiseq(seq);
	}
	return rec;
}
/*===============================================
 * ask_for_record_key -- Ask user to enter record key
 * returns NULL or strsave'd answer
 *=============================================*/
STRING
ask_for_record_key (STRING title, STRING prompt)
{
	char answer[MAXPATHLEN];
	if (!ask_for_string(title, prompt, answer, sizeof(answer)))
		return NULL;
	if (!answer) return NULL;
	return strsave(answer);
}
