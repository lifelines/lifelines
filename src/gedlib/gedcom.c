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
/*================================================================
 * gedcom.c -- Read and convert GEDCOM records to LifeLines format
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 08 May 94    3.0.2 - 01 Dec 94
 *==============================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "sequence.h"
#include "gedcheck.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

static NODE indi_to_indi();
static NODE fam_to_fam();
static NODE even_to_even();
static NODE sour_to_sour();
static NODE othr_to_othr();

STRING misnam = (STRING) "Missing NAME line in INDI record; record ignored.\n";
STRING noiref = (STRING) "FAM record has no INDI references; record ignored.\n";

/*========================================================
 * node_to_node -- Convert GEDCOM record to LifeLines form
 *======================================================*/
NODE
node_to_node (NODE node,
              INT *ptype)
{
	*ptype = 0;
	if (eqstr("HEAD", ntag(node)) ||
	    eqstr("TRLR", ntag(node))) return NULL;
	if (eqstr("INDI", ntag(node)))      *ptype = INDI_REC;
	else if (eqstr("FAM",  ntag(node))) *ptype = FAM_REC;
	else if (eqstr("EVEN", ntag(node))) *ptype = EVEN_REC;
	else if (eqstr("SOUR", ntag(node))) *ptype = SOUR_REC;
	else                                *ptype = OTHR_REC;
	switch (*ptype) {
	case INDI_REC: return indi_to_indi(node);
	case FAM_REC:  return fam_to_fam(node);
	case EVEN_REC: return even_to_even(node);
	case SOUR_REC: return sour_to_sour(node);
	case OTHR_REC: return othr_to_othr(node);
	default: FATAL();
	}
	return NULL;		/* keep compiler happy */
}
/*=======================================================
 * indi_to_indi - Convert person record to LifeLines form
 *=====================================================*/
static NODE
indi_to_indi (NODE indi)
{
	NODE node, last, name, famc, lfmc, fams, lfms, frst, prev = 0;
	last = name = famc = fams = frst = lfmc =lfms = NULL;
	node = nchild(indi);
	while (node) {
		STRING tag = ntag(node);
		if (eqstr("NAME", tag) && !name)
			name = node;
		else if (eqstr("FAMS", tag)) {
			if (!fams)
				lfms = fams = node;
			else
				lfms = nsibling(lfms) = node;
		} else if (eqstr("FAMC", tag)) {
			if (!famc)
				lfmc = famc = node;
			else
				lfmc = nsibling(lfmc) = node;
		} else {
			if (!frst)
				frst = last = node;
			else
				last = nsibling(last) = node;
		}
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	if (!name) {
		llwprintf(misnam);
		return NULL;
	}
	nchild(indi) = node = name;
	if (frst) {
		nsibling(node) = frst;
		node = last;
	}
	if (famc) {
		nsibling(node) = famc;
		node = lfmc;
	}
	nsibling(node) = fams;
	return indi;
}
/*======================================================
 * fam_to_fam -- Convert family record to LifeLines form
 *====================================================*/
static NODE
fam_to_fam (NODE fam)
{
	NODE node, frst, last, husb, wife, chil, lchl, prev;
	frst = last = husb = wife = chil = lchl = NULL;
	node = nchild(fam);
	while (node) {
		STRING tag = ntag(node);
		if (eqstr("HUSB", tag) && !husb)
			husb = node;
		else if (eqstr("WIFE", tag) && !wife)
			wife = node;
		else if (eqstr("CHIL", tag)) {
			if (!chil)
				chil = lchl = node;
			else
				lchl = nsibling(lchl) = node;
		} else {
			if (!frst)
				last = frst = node;
			else
				last = nsibling(last) = node;
		}
		prev = node;
		node = nsibling(node);
		nsibling(prev) = NULL;
	}
	node = NULL;
	if (!husb && !wife && !chil) {
		llwprintf(noiref);
		return NULL;
	}
	if (husb) nchild(fam) = node = husb;
	if (wife) {
		if (!node)
			nchild(fam) = node = wife;
		else
			node = nsibling(node) = wife;
	}
	if (frst) {
		if (!node)
			nchild(fam) = frst;
		else
			nsibling(node) = frst;
		node = last;
	}
	if (chil) {
		if (!node)
			nchild(fam) = chil;
		else
			nsibling(node) = chil;
	}
	return fam;
}
/*=======================================================
 * even_to_even -- Convert event record to LifeLines form
 *=====================================================*/
static NODE
even_to_even (NODE evn)
{
	return evn;
}
/*========================================================
 * sour_to_sour -- Convert source record to LifeLines form
 *======================================================*/
static NODE
sour_to_sour (NODE src)
{
	return src;
}
/*=======================================================
 * othr_to_othr -- Convert other record to LifeLines form
 *=====================================================*/
static NODE
othr_to_othr (NODE oth)
{
	return oth;
}
