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
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"

#include "llinesi.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern INT listbadkeys;
extern char badkeylist[];

extern STRING ntchld,ntprnt,idfbrs,entnam,notone,ifone;
extern STRING nofopn,idbrws,whtfname,whtfnameext;

/*********************************************
 * local function prototypes
 *********************************************/

static INDISEQ ask_for_indi_list_once(STRING, INT*);
static RECORD ask_for_any_once(STRING ttl, char ctype, ASK1Q ask1, INT *prc);

/*=====================================================
 * ask_for_fam_by_key -- Ask user to identify family by
 *  key (or REFN)
 *  (if they enter nothing, it will fall thru to ask_for_fam)
 *========================================================*/
NODE
ask_for_fam_by_key (STRING fttl, STRING pttl, STRING sttl)
{
	RECORD fam = ask_for_record(fttl, 'F');
	return fam ? nztop(fam) : ask_for_fam(pttl, sttl);
}
/*===========================================
 * ask_for_fam -- Ask user to identify family
 *=========================================*/
NODE
ask_for_fam (STRING pttl,
             STRING sttl)
{
	NODE sib, fam, prn;
	prn = ask_for_indi_old(pttl, NOCONFIRM, DOASK1);
	if (!prn)  {
		sib = ask_for_indi_old(sttl, NOCONFIRM, DOASK1);
		if (!sib) return NULL;
		if (!(fam = FAMC(sib))) {
			message(ntchld);
			return NULL;
		}
		fam = key_to_fam(rmvat(nval(fam)));
		return fam;
	}
	if (!FAMS(prn)) {
		message(ntprnt);
		return NULL;
	}
	return choose_family(prn, "e", idfbrs, TRUE);
}
/*===========================================
 * ask_for_int -- Ask user to provide integer
 *=========================================*/
INT
ask_for_int (STRING ttl)
{
	INT ival, c, neg;
	STRING p = ask_for_string(ttl, "enter integer:");
	while (TRUE) {
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
			if (*p == 0) return ival*neg;
		}
		p = ask_for_string(ttl, "enter integer:");
	}
}
/*============================================
 * expand_special_chars -- Replace ~ with home
 *==========================================*/
static int
expand_special_chars (STRING fname, STRING buffer, INT buflen)
{
#ifdef WIN32
	if (fname[0] == '~')	{
		/* replace ~ with user's home directory, if present */
		char * home = (STRING)getenv("USERPROFILE");
		if (home && home[0]
			&& (INT)(strlen(home)+strlen(fname)+1) < buflen) {
			strncpy(buffer, home, sizeof(buffer));
			strcat(buffer, fname+1);
			return 1;
		}
	}
#else
	fname=fname; /* unused */
	buffer=buffer; /* unused */
	buflen=buflen; /* unused */
#endif
	return 0;
}
/*======================================
 * ask_for_file_worker -- Ask for and open file
 * pfname - optional output parameter (pass NULL if undesired)
 *====================================*/
typedef enum { INPUT, OUTPUT } DIRECTION;
static FILE *
ask_for_file_worker (STRING mode,
                     STRING ttl,
                     STRING *pfname,
                     STRING path,
                     STRING ext,
                     DIRECTION direction)
{
	FILE *fp;
	STRING fname;
	char fnamebuf[512];
	int elen, flen;
	char pathtemp[MAXPATHLEN];

	make_fname_prompt(fnamebuf, sizeof(fnamebuf), ext);

	if (direction==INPUT)
		fname = ask_for_input_filename(ttl, path, fnamebuf);
	else
		fname = ask_for_output_filename(ttl, path, fnamebuf);

	if (pfname) *pfname = 0; /* 0 indicates we didn't try to open */

	if (ISNULL(fname)) return NULL;

	if (pfname) *pfname = fname;

	if(ext) {
		elen = strlen(ext);
		flen = strlen(fname);
		if((elen < flen) && (strcmp(fname+flen-elen, ext) == 0))
			ext = NULL;	/* the file name has the extension already */
	}

	if (expand_special_chars(fname, pathtemp, sizeof(pathtemp)))
	{
		/* TO DO - use pathtemp - but what about space in fname ? */
	}

	if (ISNULL(path)) {
		fp = NULL;
		if(ext) {
			sprintf(fnamebuf, "%s%s", fname, ext);
			fp = fopen(fnamebuf, mode);
			if(fp && pfname) *pfname = strsave(fnamebuf);
		} else {
			fp = fopen(fname, mode);
			if(fp && pfname) *pfname = fname;
		}
		if (fp == NULL) {
			msg_error(nofopn, fname);
			return NULL;
		}
		return fp;
	}

	if (!(fp = fopenpath(fname, mode, path, ext, pfname))) {
		if(pfname && (*pfname == NULL)) *pfname = fname;
		msg_error(nofopn, fname);
		return NULL;
	}
	return fp;
}
/*======================================
 * make_fname_prompt -- Create prompt line
 *  for filename, depending on extension
 * Created: 2001/12/24, Perry Rapp
 *====================================*/
void
make_fname_prompt (STRING fnamebuf, INT len, STRING ext)
{
	if (ISNULL(ext)) {
		ext = NULL;	/* a null extension is the same as no extension */
		snprintf(fnamebuf, len, "%s: ", whtfname);
	}
	else {
		snprintf(fnamebuf, len, whtfnameext, ext);
	}
}
/*======================================
 * ask_for_input_file -- Ask for and open file for input
 * pfname - optional output parameter (pass NULL if undesired)
 *====================================*/
FILE *
ask_for_input_file (STRING mode,
                    STRING ttl,
                    STRING *pfname,
                    STRING path,
                    STRING ext)
{
	return ask_for_file_worker(mode, ttl, pfname, path, ext,
		INPUT);
}

/*======================================
 * ask_for_output_file -- Ask for and open file for output
 * pfname - optional output parameter (pass NULL if undesired)
 *====================================*/
FILE *
ask_for_output_file (STRING mode,
                     STRING ttl,
                     STRING *pfname,
                     STRING path,
                     STRING ext)
{
	return ask_for_file_worker(mode, ttl, pfname, path, ext,
		OUTPUT);
}
	/* RC_DONE means user just hit enter -- interpret as a cancel */
#define RC_DONE     0
	/* RC_NOSELECT means user's choice couldn't be found & we gave up (& told them) */
#define RC_NOSELECT 1
	/* RC_SELECT means user chose something valid */
#define RC_SELECT   2
/*=================================================
 * ask_for_indiseq -- Ask user to identify sequence
 *  ttl:   [IN]  prompt (title) to display
 *  ctype: [IN]  type of record (eg, 'I') (0 for any)
 *  prc:   [OUT] result code (RC_DONE, RC_SELECT, RC_NOSELECT)
 *===============================================*/
INDISEQ
ask_for_indiseq (STRING ttl, char ctype, INT *prc)
{
	INDISEQ seq;
	STRING name = ask_for_string(ttl, N_(idbrws));
	*prc = RC_DONE;
	if (!name || *name == 0) return NULL;
	*prc = RC_NOSELECT;
	seq = str_to_indiseq(name, ctype);
	if (!seq) {
		STRING unknam = _("There is no one in the database with that name or key.");
		message(unknam);
		return NULL;
	}
	*prc = RC_SELECT;
	return seq;
}
/*============================================================
 * ask_for_any_once -- Have user identify sequence and select
 *   person
 *  ttl:   [IN]  title to present
 *  ctype: [IN]  type of record (eg, 'I') (0 for any)
 *  ask1:  [IN]  whether to present list if only one matches their desc.
 *  prc:   [OUT] result (RC_DONE, RC_SELECT, RC_NOSELECT)
 *==========================================================*/
static RECORD
ask_for_any_once (STRING ttl, char ctype, ASK1Q ask1, INT *prc)
{
	RECORD indi = 0;
	INDISEQ seq = ask_for_indiseq(ttl, ctype, prc);
	if (*prc == RC_DONE || *prc == RC_NOSELECT) return NULL;
	indi = choose_from_indiseq(seq, ask1, ifone, notone);
	remove_indiseq(seq);
	*prc = indi ? RC_SELECT : RC_NOSELECT;
	return indi;
}
/*=================================================================
 * ask_for_indi_old -- old interface to ask_for_indi (q.v.)
 *===============================================================*/
NODE
ask_for_indi_old (STRING ttl, CONFIRMQ confirmq, ASK1Q ask1)
{
	return nztop(ask_for_indi(ttl, confirmq, ask1));
}
/*=================================================================
 * ask_for_indi -- Ask user to identify sequence and select person;
 *   reask protocol used
 * ttl:      [in] title for question
 * confirmq: [in] whether to confirm after choice
 * ask1:     [in] whether to present list if only one matches
 *===============================================================*/
RECORD
ask_for_indi (STRING ttl, CONFIRMQ confirmq, ASK1Q ask1)
{
	while (TRUE) {
		INT rc;
		RECORD indi = ask_for_any_once(ttl, 'I', ask1, &rc);
		if (rc == RC_DONE || rc == RC_SELECT) return indi;
		if (confirmq != DOCONFIRM || !ask_yes_or_no(entnam)) return NULL;
	}
}
/*=================================================================
 * ask_for_any_old -- old interface to ask_for_any (q.v.)
 *===============================================================*/
NODE
ask_for_any_old (STRING ttl, CONFIRMQ confirmq, ASK1Q ask1)
{
	return nztop(ask_for_any(ttl, confirmq, ask1));
}
/*=================================================================
 * ask_for_any -- Ask user to identify sequence and select record
 *   reask protocol used
 * ttl:      [in] title for question
 * confirmq: [in] whether to confirm after choice
 * ask1:     [in] whether to present list if only one matches
 *===============================================================*/
RECORD
ask_for_any (STRING ttl, CONFIRMQ confirmq, ASK1Q ask1)
{
	char ctype = 0; /* code for any type */
	while (TRUE) {
		INT rc;
		RECORD record = ask_for_any_once(ttl, ctype, ask1, &rc);
		if (rc == RC_DONE || rc == RC_SELECT) return record;
		if (confirmq != DOCONFIRM || !ask_yes_or_no(entnam)) return NULL;
	}
}
/*===============================================================
 * ask_for_indi_list_once -- Ask user to identify person sequence
 *   and then select sub-sequence of them
 * returns null value indiseq
 * used by both reports & interactive use
 *=============================================================*/
static INDISEQ
ask_for_indi_list_once (STRING ttl,
                        INT *prc)
{
	INDISEQ seq = ask_for_indiseq(ttl, 'I', prc);
	INT rv;
	if (*prc == RC_DONE || *prc == RC_NOSELECT) return NULL;
	rv = choose_list_from_indiseq(notone, seq);
	if (rv == -1) {
		remove_indiseq(seq);
		seq = NULL;
	}
	*prc = seq ? RC_SELECT : RC_NOSELECT;
	return seq;
}
/*===================================================================
 * ask_for_indi_list -- Ask user to identify person sequence and then
 *   select one person from it; use reask protocol
 * returns null value indiseq
 * used by both reports & interactive use
 *=================================================================*/
INDISEQ
ask_for_indi_list (STRING ttl,
                   BOOLEAN reask)
{
	while (TRUE) {
		INT rc;
		INDISEQ seq = ask_for_indi_list_once(ttl, &rc);
		if (rc == RC_DONE || rc == RC_SELECT) return seq;
		if (!reask || !ask_yes_or_no(entnam)) return NULL;
	}
}
/*==========================================================
 * ask_for_indi_key -- Have user identify person; return key
 *========================================================*/
STRING
ask_for_indi_key (STRING ttl,
                  CONFIRMQ confirmq,
                  ASK1Q ask1)
{
	NODE indi = ask_for_indi_old(ttl, confirmq, ask1);
	if (!indi) return NULL;
	return rmvat(nxref(indi));
}
/*===============================================================
 * choose_one_from_indiseq_if_needed  -- handle ask1 cases
 *=============================================================*/
static INT
choose_one_from_indiseq_if_needed (INDISEQ seq,
                                   ASK1Q ask1,
                                   STRING titl1,
                                   STRING titln)
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
 *=====================================================*/
RECORD
choose_from_indiseq (
	INDISEQ seq,    /* sequence */
	ASK1Q ask1,   /* choose if len one? */
	STRING titl1,   /* title if len = one */
	STRING titln)   /* title if len > one */
{
	INT i = 0;
	RECORD rec=0;
	STRING skey;
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
		skey = skey(IData(seq)[i]);
		rec = key_to_record(skey, TRUE);
	}
	listbadkeys = 0;
	if(!rec) {
		char buf[132];
		if (badkeylist[0])
			sprintf(buf, "WARNING: missing keys: %.40s", badkeylist);
		else
			sprintf(buf, "WARNING: invalid pointer");
		message(buf);
	}
	return rec;
}
