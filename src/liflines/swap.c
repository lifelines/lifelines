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
/*=================================================================
 * swap.c -- Swaps two children of family or two families of person
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 27 Sep 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 23 Sep 94
 *   3.0.2 - 02 Dec 94
 *===============================================================*/

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
extern STRING idcswp, id1csw, id2csw, id1fsw, id2fsw, idfbys, ntprnt;
extern STRING less2c, okcswp, less2f, okfswp, idfswp, ronlye;
extern STRING idcrdr, ntchld, paradox;

/*********************************************
 * local function prototypes
 *********************************************/
static INT child_index(NODE child, NODE fam);
static void swap_children_impl(NODE fam, NODE one, NODE two);

/*=============================================
 * swap_children -- Swap two children in family
 * NODE prnt:   [in] parent (may be NULL)
 * NODE fam:    [in] family (may be NULL)
 *===========================================*/
BOOLEAN
swap_children (NODE prnt, NODE fam)
{
	NODE chil, one, two;
	INT nfam, nchil;

	if (readonly) {
		message(_(ronlye));
		return FALSE;
	}

/* Identify parent if need be */
	if (fam) goto gotfam;
	if (!prnt) prnt = ask_for_indi_old(idcswp, NOCONFIRM, NOASK1);
	if (!prnt) return FALSE;
	nfam = num_families(prnt);
	if (nfam <= 0) {
		message(_(ntchld));
		return FALSE;
	}

/* Identify family if need be */
	if (nfam == 1) {
		fam = key_to_fam(rmvat(nval(FAMS(prnt))));
		goto gotfam;
	}
	if (!(fam = choose_family(prnt, ntprnt, idfbys, TRUE))) return FALSE;
gotfam:
	nchil = num_children(fam);
	if (nchil < 2) {
		message(less2c);
		return FALSE;
	}

	if (nchil == 2) {
		/* swap the two existing ones */
		one = CHIL(fam);
		two = nsibling(one);
	} else {
		NODE chil1, chil2;
		STRING key1, key2;
		/* Identify children to swap */
		chil1 = choose_child(NULL, fam, "e", id1csw, NOASK1);
		if (!chil1) return FALSE;
		chil2 = choose_child(NULL, fam, "e", id2csw, NOASK1);
		if (!chil2) return FALSE;
		if (chil1 == chil2) return FALSE;
		key1 = nxref(chil1);
		key2 = nxref(chil2);

		/* loop through children & find the ones chosen */
		ASSERT(chil = CHIL(fam));
		one = two = NULL;
		for (;  chil;  chil = nsibling(chil)) {
			if (eqstr(key1, nval(chil))) one = chil;
			if (eqstr(key2, nval(chil))) two = chil;
		}
	}

	swap_children_impl(fam, one, two);
	message(okcswp);
	return TRUE;
}
/*=============================================
 * swap_children_impl -- Swap input children in input family
 *  fam:     [in] family of interest
 *  one,two: [in] children to swap
 * all inputs assumed valid - no user feedback here
 *===========================================*/
static void
swap_children_impl (NODE fam, NODE one, NODE two)
{
	STRING str;
	NODE tmp;
	ASSERT(one && two);
   /* Swap CHIL nodes and update database */
	str = nval(one);
	nval(one) = nval(two);
	nval(two) = str;
	tmp = nchild(one);
	nchild(one) = nchild(two);
	nchild(two) = tmp;
	fam_to_dbase(fam);
}
/*=============================================
 * reorder_child -- Reorder one child in family
 * NODE prnt:   [in] parent (may be NULL)
 * NODE fam:    [in] family (may be NULL)
 *===========================================*/
BOOLEAN
reorder_child (NODE prnt, NODE fam)
{
	INT nfam, nchil;
	INT prevorder, i;
	NODE child;

	if (readonly) {
		message(_(ronlye));
		return FALSE;
	}

/* Identify parent if need be */
	if (fam) goto gotfam;
	if (!prnt) prnt = ask_for_indi_old(idcswp, NOCONFIRM, NOASK1);
	if (!prnt) return FALSE;
	nfam = num_families(prnt);
	if (nfam <= 0) {
		message(_(ntchld));
		return FALSE;
	}

/* Identify family if need be */
	if (nfam == 1) {
		fam = key_to_fam(rmvat(nval(FAMS(prnt))));
		goto gotfam;
	}
	if (!(fam = choose_family(prnt, ntprnt, idfbys, TRUE))) return FALSE;
gotfam:
	nchil = num_children(fam);
	if (nchil < 2) {
		message(less2c);
		return FALSE;
	}

	if (nchil == 2) {
		/* swap the two existing ones */
		NODE one = CHIL(fam);
		NODE two = nsibling(one);
		swap_children_impl(fam, one, two);
		message(okcswp);
		return TRUE;
	}

	/* Identify children to swap */
	child = choose_child(NULL, fam, "e", idcrdr, NOASK1);
	if (!child) return FALSE;

	prevorder = child_index(child, fam);

	/* first remove child, so can list others & add back */
	remove_child(child, fam);

	i = ask_child_order(fam, ALWAYS_PROMPT, &disp_shrt_rfmt);
	if (i == -1) {
		/* must put child back if cancel */
		add_child_to_fam(child, fam, prevorder);
		return FALSE;
	}

/* Add FAMC node to child */

	add_child_to_fam(child, fam, i);

	fam_to_dbase(fam);
	return TRUE;
}
/*=============================================
 * child_index -- What is child's birth index in this family ?
 *  child:  [in] child of interest
 *  fam:    [in] family of interest
 *===========================================*/
static INT
child_index (NODE child, NODE fam)
{
	INT j;
	NODE husb, wife, chil, rest, fref;
	NODE node;
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	node = chil;
	j = 0;
	while (node && !eqstr(nval(node), nxref(child))) {
		++j;
		node = nsibling(node);
	}
	join_fam(fam, fref, husb, wife, chil, rest);
	return child ? j : -1;
}
/*=============================================
 * swap_families -- Swap two families of person
 *  indi:  [in] person
 *  prompt for indi if NULL
 *  prompt for families if person chosen has >2
 *===========================================*/
BOOLEAN
swap_families (NODE indi)
{
	NODE fams, one, two, tmp;
	INT nfam;
	STRING str;

	if (readonly) {
		message(_(ronlye));
		return FALSE;
	}

/* Find person and assure has >= 2 families */
	if (!indi) indi = ask_for_indi_old(idfswp, NOCONFIRM, NOASK1);
	if (!indi) return FALSE;
	if (!(fams = FAMS(indi))) {
		message(ntprnt);
		return FALSE;
	}
	nfam = num_families(indi);
	if (nfam < 2) {
		msg_error(_(less2f));
		return FALSE;
	}

/* Find families to swap */
	ASSERT(fams = FAMS(indi));
	if (nfam == 2) {
		/* swap the two existing ones */
		one = fams;
		two = nsibling(fams);
	} else {
		NODE fam1, fam2;
		STRING key1, key2;
		/* prompt for families */
		fam1 = choose_family(indi, paradox, id1fsw, TRUE);
		if (!fam1) return FALSE;
		fam2 = choose_family(indi, paradox, id2fsw, TRUE);
		if (!fam2) return FALSE;
		if (fam1 == fam2) return FALSE;
		key1 = nxref(fam1);
		key2 = nxref(fam2);
		/* loop through families & find the ones chosen */
		one = two = NULL;
		for (;  fams;  fams = nsibling(fams)) {
			if (eqstr(key1, nval(fams))) one = fams;
			if (eqstr(key2, nval(fams))) two = fams;
		}
	}

	ASSERT(one && two);
/* Swap FAMS nodes and update database */
	str = nval(one);
	nval(one) = nval(two);
	nval(two) = str;
	tmp = nchild(one);
	nchild(one) = nchild(two);
	nchild(two) = tmp;
	indi_to_dbase(indi);
	message(okfswp);
	return TRUE;
}
