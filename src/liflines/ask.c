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
 *   2.3.4 - 24 Jun 93    2.3.5 - 24 Aug 93
 *   2.3.6 - 30 Oct 93    3.0.0 - 19 Aug 94
 *   3.0.2 - 02 Dec 94
 *===========================================================*/
/* modified 2000-01-26 J.F.Chandler */

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "indiseq.h"

extern STRING ntchld, ntprnt, idfbrs, entnam, unknam, notone, ifone;
extern STRING nofopn;
extern INDISEQ str_to_indiseq();
extern INDISEQ choose_list_from_indiseq();

/*===========================================
 * ask_for_fam -- Ask user to identify family
 *=========================================*/
NODE ask_for_fam (pttl, sttl)
STRING pttl, sttl;
{
	NODE sib, fam, prn = ask_for_indi(pttl, FALSE, TRUE);
	if (!prn)  {
		sib = ask_for_indi(sttl, FALSE, TRUE);
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
INT ask_for_int (ttl)
STRING ttl;
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
/*========================================
 * ask_yes_or_no -- Ask yes or no question
 *======================================*/
BOOLEAN ask_yes_or_no (ttl)
STRING ttl;
{
	INT c = ask_for_char(ttl, "enter y (yes) or n (no): ",
	    "yYnN");
	return c == 'y' || c == 'Y';
}
/*=========================================================
 * ask_yes_or_no_msg -- Ask yes or no question with message
 *=======================================================*/
BOOLEAN ask_yes_or_no_msg (msg, ttl)
STRING msg, ttl;
{
	INT c = ask_for_char_msg(msg, ttl, "enter y (yes) or n (no): ",
	    "yYnN");
	return c == 'y' || c == 'Y';
}
/*======================================
 * ask_for_file -- Ask for and open file
 *====================================*/
FILE *ask_for_file (mode, ttl, pfname, path, ext)
STRING mode;
STRING ttl;
STRING *pfname;
STRING path;
STRING ext;
{
	FILE *fp, *fopenpath();
	STRING fname;
	char fnamebuf[512];
	int elen, flen;
	
	if(ext && *ext)
	  sprintf(fnamebuf, "enter file name (*%s)", ext);
	else {
	  ext = NULL;	/* a null extension is the same as no extension */
	  strcpy(fnamebuf, "enter file name: ");
	}

	fname = ask_for_string(ttl, fnamebuf);

	if (pfname) *pfname = fname;

	if(ext) {
	    elen = strlen(ext);
	    flen = strlen(fname);
	    if((elen < flen) && (strcmp(fname+flen-elen, ext) == 0))
		ext = NULL;	/* the file name has the extension already */
	}

	if (!fname || *fname == 0) return NULL;
	if (!path || *path == 0) {
	    	fp = NULL;
		if(ext) {
		  sprintf(fnamebuf, "%s%s", fname, ext);
		  fp = fopen(fnamebuf, mode);
		  if(pfname) *pfname = strsave(fnamebuf);
		}
		if((fp == NULL) && ((fp = fopen(fname, mode)) == NULL)) {
			mprintf(nofopn, fname);
			return NULL;
		}
		return fp;
	}
	if (!(fp = fopenpath(fname, mode, path, ext, pfname))) {
	    	if(pfname && (*pfname == NULL)) *pfname = fname;
		mprintf(nofopn, fname);
		return NULL;
	}
	return fp;
}
#define RC_DONE     0
#define RC_NOSELECT 1
#define RC_SELECT   2
/*=================================================
 * ask_for_indiseq -- Ask user to identify sequence
 *===============================================*/
INDISEQ ask_for_indiseq (ttl, prc)
STRING ttl;
INT *prc;
{
	INDISEQ seq;
	NODE indi;
	STRING name = ask_for_string(ttl, "enter name, key, refn or list:");
	*prc = RC_DONE;
	if (!name || *name == 0) return NULL;
	*prc = RC_NOSELECT;
	seq = str_to_indiseq(name);
	if (!seq) {
		message(unknam);
		return NULL;
	}
	*prc = RC_SELECT;
	return seq;
}
/*============================================================
 * ask_for_indi_once -- Have user identify sequence and select
 *   person
 *==========================================================*/
NODE ask_for_indi_once (ttl, ask1, prc)
STRING ttl;
BOOLEAN ask1;
INT *prc;
{
	NODE indi;
	INDISEQ seq = ask_for_indiseq(ttl, prc);
	if (*prc == RC_DONE || *prc == RC_NOSELECT) return NULL;
	indi = format_and_choose_indi(seq, FALSE, FALSE, ask1, ifone, notone);
	remove_indiseq(seq, FALSE);
	*prc = indi ? RC_SELECT : RC_NOSELECT;
	return indi;
}
/*=================================================================
 * ask_for_indi -- Ask user to identify sequence and select person;
 *   reask protocol used
 *===============================================================*/
NODE ask_for_indi (ttl, reask, ask1)
STRING ttl;
BOOLEAN reask, ask1;
{
	while (TRUE) {
		INT rc;
		NODE indi = ask_for_indi_once(ttl, ask1, &rc);
		if (rc == RC_DONE || rc == RC_SELECT) return indi;
		if (!reask || !ask_yes_or_no(entnam)) return NULL;
	}
}
/*===============================================================
 * ask_for_indi_list_once -- Ask user to identify person sequence
 *   and then select sub-sequence of them
 *=============================================================*/
INDISEQ ask_for_indi_list_once (ttl, prc)
STRING ttl;
INT *prc;
{
	STRING name;
	NODE indi;
	INT i, len;
	INDISEQ seq = ask_for_indiseq(ttl, prc);
	if (*prc == RC_DONE || *prc == RC_NOSELECT) return NULL;
	format_indiseq(seq, FALSE, FALSE);
	seq = choose_list_from_indiseq(notone, seq);
	*prc = seq ? RC_SELECT : RC_NOSELECT;
	return seq;
}
/*===================================================================
 * ask_for_indi_list -- Ask user to identify person sequence and then
 *   select one person from it; use reask protocol
 *=================================================================*/
INDISEQ ask_for_indi_list (ttl, reask)
STRING ttl;
BOOLEAN reask;
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
STRING ask_for_indi_key (ttl, reask, ask1)
STRING ttl;
BOOLEAN reask;
BOOLEAN ask1;
{
	NODE indi = ask_for_indi(ttl, reask, ask1);
	if (!indi) return NULL;
	return rmvat(nxref(indi));
}
/*===============================================================
 * format_and_choose_indi -- Format sequence and have user choose
 *   person from it
 *=============================================================*/
NODE format_and_choose_indi (seq, fams, marr, ask1, titl1, titln)
INDISEQ seq;	/* sequence */
BOOLEAN fams;	/* seq of families? */
BOOLEAN marr;	/* show marriage info? */
BOOLEAN ask1;	/* choose if len one? */
STRING titl1;	/* title if len = one */
STRING titln;	/* title if len > one */
{
	INT i = 0;
	format_indiseq(seq, fams, marr);
	if (length_indiseq(seq) > 1)
		i = choose_one_from_indiseq(titln, seq);
	else if (ask1 && titl1)
		i = choose_one_from_indiseq(titl1, seq);
	else
		i = 0;
	if (i == -1) return NULL;
	if (fams) return key_to_fam(skey(IData(seq)[i]));
	return key_to_indi(skey(IData(seq)[i]));
}
