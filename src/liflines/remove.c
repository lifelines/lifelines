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
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 05 Dec 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

#include "llinesi.h"

extern STRING idcrmv, ntchld, ntprnt, idsrmv, idsrmf, normls, cfcrmv;
extern STRING okcrmv, ntsinf, ntcinf, cfsrmv, oksrmv, ronlye, idcrmf;

static INT num_fam_xrefs(NODE);

/*===========================================
 * choose_and_remove_child -- Remove child
 *  from family (prompting for child and/or family
 *  if NULL passed)
 *  nolast: don't remove last member of family?
 *=========================================*/
BOOLEAN
choose_and_remove_child (NODE indi, NODE fam, BOOLEAN nolast)
{
	if (readonly) {
		message(ronlye);
		return FALSE;
	}
		
/* Identify child and check for FAMC nodes */
	if (!indi) indi = ask_for_indi(idcrmv, NOCONFIRM, NOASK1);
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

	if (!remove_child(indi, fam)) {
		message(ntcinf);
		return FALSE;
	}

	message(okcrmv);
	return TRUE;
}
/*=========================================
 * remove_child -- Remove child from family
 *  silent function
 *=======================================*/
BOOLEAN
remove_child (NODE indi, NODE fam)
{
	NODE node, last;

/* Make sure child is in family and remove his/her CHIL line */
	if (!(node = find_node(fam, "CHIL", nxref(indi), &last))) {
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
	return TRUE;
}
/*===========================================
 * choose_and_remove_spouse -- Remove spouse 
 *  from family (prompting for spouse and/or family
 *  if NULL passed)
 *  nolast: don't remove last member of family?
 *=========================================*/
BOOLEAN
choose_and_remove_spouse (NODE indi, NODE fam, BOOLEAN nolast)
{
	if (readonly) {
		message(ronlye);
		return FALSE;
	}

/* Identify spouse to remove */
	if (!indi) indi = ask_for_indi(idsrmv, NOCONFIRM, NOASK1);
	if (!indi) return FALSE;
	if (!FAMS(indi)) {
		message(ntprnt);
		return FALSE;
	}

/* Identify family to remove spouse from */
	if (!fam) fam = choose_family(indi, "e", idsrmf, TRUE);
	if (!fam) return FALSE;
	if (nolast && num_fam_xrefs(fam) < 2) {
		message(normls);
		return FALSE;
	}
	if (!ask_yes_or_no(cfsrmv)) return FALSE;

	if (!remove_spouse(indi, fam)) {
		message(ntsinf);
		return FALSE;
	}
	message(oksrmv);
	return TRUE;
}
/*===========================================
 * remove_spouse -- Remove spouse from family
 *  both arguments required
 *  silent function
 *=========================================*/
BOOLEAN
remove_spouse (NODE indi, NODE fam)
{
	NODE node, last;
	INT sex;
	STRING stag;

	sex = SEX(indi);
	ASSERT(sex == SEX_MALE || sex == SEX_FEMALE);
/* Make sure spouse is in family and remove his/her HUSB/WIFE line */
	stag = (STRING) ((sex == SEX_MALE) ? "HUSB" : "WIFE");
	if (!(node = find_node(fam, stag, nxref(indi), &last)))
		return FALSE;
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
	return TRUE;
}
/*=======================================================
 * num_fam_xrefs -- Find number of person links in family
 *   LOOSEEND -- How about other links in the future???
 *=====================================================*/
static INT
num_fam_xrefs (NODE fam)
{
	INT num;
	NODE fref, husb, wife, chil, rest;

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	num = length_nodes(husb) + length_nodes(wife) + length_nodes(chil);
	join_fam(fam, fref, husb, wife, chil, rest);
	return num;
}
