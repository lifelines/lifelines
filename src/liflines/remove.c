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
 * remove.c -- Remove child or spouse from family
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 05 Dec 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"

extern STRING idcrmv, ntchld, ntprnt, idsrmv, idsrmf, normls, cfcrmv;
extern STRING okcrmv, ntsinf, ntcinf, cfsrmv, oksrmv, ronlye, idcrmf;

/*=========================================
 * remove_child -- Remove child from family
 *=======================================*/
BOOLEAN remove_child (indi, fam, nolast)
NODE indi, fam;
BOOLEAN nolast;	/* don't remove last in family? */
{
	NODE node, last;

	if (readonly) {
		message(ronlye);
		return FALSE;
	}
		
/* Identify child and check for FAMC nodes */
	if (!indi) indi = ask_for_indi(idcrmv, FALSE, FALSE);
	if (!indi) return FALSE;
	if (!FAMC(indi)) {
		message(ntchld);
		return FALSE;
	}

/* Identify family to remove child from */
	if (!fam) fam = choose_family(indi, "e", idcrmf, FALSE);
	if (!fam) return FALSE;
	if (nolast && num_fam_xrefs(fam) < 2) {
		message(normls);
		return FALSE;
	}
	if (!ask_yes_or_no(cfcrmv)) return TRUE;

/* Make sure child is in family and remove his/her CHIL line */
	if (!(node = find_node(fam, "CHIL", nxref(indi), &last))) {
		message(ntcinf);
		return FALSE;
	}
	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(fam) = nsibling(node);
	free_node(node);

/* Remove FAMC line from child */
	node = find_node(indi, "FAMC", nxref(fam), &last);
	ASSERT(node && last);
	nsibling(last) = nsibling(node);
	free_node(node);

/* Update database with changed records */
	indi_to_dbase(indi);
	if (num_fam_xrefs(fam) == 0)
		delete_fam(fam);
	else
		fam_to_dbase(fam);
	message(okcrmv);
	return TRUE;
}
/*===========================================
 * remove_spouse -- Remove spouse from family
 *=========================================*/
BOOLEAN remove_spouse (indi, fam, nolast)
NODE indi, fam;
BOOLEAN nolast;	/* don't remove last member of family? */
{
	NODE node, last;
	INT i, sex;
	STRING stag;

	if (readonly) {
		message(ronlye);
		return FALSE;
	}

/* Identify spouse to remove */
	if (!indi) indi = ask_for_indi(idsrmv, FALSE, FALSE);
	if (!indi) return FALSE;
	if (!FAMS(indi)) {
		message(ntprnt);
		return FALSE;
	}
	sex = SEX(indi);
	ASSERT(sex == SEX_MALE || sex == SEX_FEMALE);

/* Identify family to remove spouse from */
	if (!fam) fam = choose_family(indi, "e", idsrmf, TRUE);
	if (!fam) return FALSE;
	if (nolast && num_fam_xrefs(fam) < 2) {
		message(normls);
		return FALSE;
	}
	if (!ask_yes_or_no(cfsrmv)) return FALSE;

/* Make sure spouse is in family and remove his/her HUSB/WIFE line */
	stag = (STRING) ((sex == SEX_MALE) ? "HUSB" : "WIFE");
	if (!(node = find_node(fam, stag, nxref(indi), &last))) {
		message(ntsinf);
		return FALSE;
	}
	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(fam) = nsibling(node);
	free_node(node);

/* Remove FAMS line from spouse */
	node = find_node(indi, "FAMS", nxref(fam), &last);
	ASSERT(node && last);
	nsibling(last) = nsibling(node);
	free_node(node);

/* Update database with change records */
	indi_to_dbase(indi);
	if (num_fam_xrefs(fam) == 0)
		delete_fam(fam);
	else
		fam_to_dbase(fam);
	message(oksrmv);
	return TRUE;
}
/*=======================================================
 * num_fam_xrefs -- Find number of person links in family
 *   LOOSEEND -- How about other links in the future???
 *=====================================================*/
INT num_fam_xrefs (fam)
NODE fam;
{
	INT num;
	NODE fref, husb, wife, chil, rest;

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	num = length_nodes(husb) + length_nodes(wife) + length_nodes(chil);
	join_fam(fam, fref, husb, wife, chil, rest);
	return num;
}
