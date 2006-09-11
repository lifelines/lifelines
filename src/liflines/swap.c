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
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
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
extern STRING qSidcswp, qSid1csw, qSid2csw, qSid1fsw, qSid2fsw, qSidfbys, qSntprnt;
extern STRING qSless2c, qSokcswp, qSless2f, qSokfswp, qSidfswp, qSronlye;
extern STRING qSidcrdr, qSntchld, qSparadox;
extern STRING qScffswp, qScfchswp;

/*********************************************
 * local function prototypes
 *********************************************/
static INT child_index(NODE child, NODE fam);
static BOOLEAN confirm_and_swap_children_impl(NODE fam, NODE one, NODE two);
static void swap_children_impl(NODE fam, NODE one, NODE two);

/*=============================================
 * swap_children -- Swap two children in family
 *  prnt: [IN]  parent (may be NULL)
 *  fam:  [IN]  family (may be NULL)
 *===========================================*/
BOOLEAN
swap_children (RECORD prnt, RECORD frec)
{
	NODE fam, chil, one, two;
	INT nfam, nchil;

	if (readonly) {
		message(_(qSronlye));
		return FALSE;
	}

/* Identify parent if need be */
	if (frec) goto gotfam;
	if (!prnt) prnt = ask_for_indi(_(qSidcswp), NOASK1);
	if (!prnt) return FALSE;
	nfam = num_families(nztop(prnt));
	if (nfam <= 0) {
		message(_(qSntchld));
		return FALSE;
	}

/* Identify family if need be */
	if (nfam == 1) {
		frec = key_to_frecord(rmvat(nval(FAMS(nztop(prnt)))));
		goto gotfam;
	}
	if (!(frec = choose_family(prnt, _(qSntprnt), _(qSidfbys), TRUE)))
		return FALSE;
gotfam:
	fam = nztop(frec);
	nchil = num_children(fam);
	if (nchil < 2) {
		message(_(qSless2c));
		return FALSE;
	}

	if (nchil == 2) {
		/* swap the two existing ones */
		one = CHIL(fam);
		two = nsibling(one);
	} else {
		RECORD chil1, chil2;
		STRING key1, key2;
		/* Identify children to swap */
		chil1 = choose_child(NULL, frec, "e", _(qSid1csw), NOASK1);
		if (!chil1) return FALSE;
		chil2 = choose_child(NULL, frec, "e", _(qSid2csw), NOASK1);
		if (!chil2) return FALSE;
		if (chil1 == chil2) return FALSE;
		key1 = nxref(nztop(chil1));
		key2 = nxref(nztop(chil2));

		/* loop through children & find the ones chosen */
		ASSERT(chil = CHIL(fam));
		one = two = NULL;
		for (;  chil;  chil = nsibling(chil)) {
			if (eqstr(ntag(chil), "CHIL")) {
				if (eqstr(key1, nval(chil))) one = chil;
				if (eqstr(key2, nval(chil))) two = chil;
			}
		}
	}

	if (!confirm_and_swap_children_impl(fam, one, two))
		return FALSE;
	message(_(qSokcswp));
	return TRUE;
}
/*=============================================
 * confirm_and_swap_children_impl -- Swap input children in input family
 *  fam:     [in] family of interest
 *  one,two: [in] children to swap
 * inputs assumed valie
 * confirm then call worker
 *===========================================*/
static BOOLEAN
confirm_and_swap_children_impl (NODE fam, NODE one, NODE two)
{
	if (!ask_yes_or_no(_(qScfchswp)))
		return FALSE;

	swap_children_impl(fam, one, two);
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
	ASSERT(one);
	ASSERT(two);
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
 *  prnt: [IN]  parent (may be NULL)
 *  fam:  [IN]  family (may be NULL)
 *  rftm: [IN]  person formatting for prompts
 *===========================================*/
BOOLEAN
reorder_child (RECORD prnt, RECORD frec, RFMT rfmt)
{
	INT nfam, nchil;
	INT prevorder, i;
	NODE fam, child;

	if (readonly) {
		message(_(qSronlye));
		return FALSE;
	}

/* Identify parent if need be */
	if (frec) goto gotfam;
	if (!prnt) prnt = ask_for_indi(_(qSidcswp), NOASK1);
	if (!prnt) return FALSE;
	nfam = num_families(nztop(prnt));
	if (nfam <= 0) {
		message(_(qSntchld));
		return FALSE;
	}

/* Identify family if need be */
	if (nfam == 1) {
		frec = key_to_frecord(rmvat(nval(FAMS(nztop(prnt)))));
		goto gotfam;
	}
	if (!(frec = choose_family(prnt, _(qSntprnt), _(qSidfbys), TRUE))) 
		return FALSE;
gotfam:
	fam = nztop(frec);
	nchil = num_children(fam);
	if (nchil < 2) {
		message(_(qSless2c));
		return FALSE;
	}

	if (nchil == 2) {
		/* swap the two existing ones */
		NODE one = CHIL(fam);
		NODE two = nsibling(one);
		if (!confirm_and_swap_children_impl(fam, one, two))
			return FALSE;
		message(_(qSokcswp));
		return TRUE;
	}

	/* Identify children to swap */
	child = nztop(choose_child(NULL, frec, "e", _(qSidcrdr), NOASK1));
	if (!child) return FALSE;

	prevorder = child_index(child, fam);

	/* first remove child, so can list others & add back */
	remove_child(child, fam);

	i = ask_child_order(fam, ALWAYS_PROMPT, rfmt);
	if (i == -1 || !ask_yes_or_no(_(qScfchswp))) {
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
 *  Ask for yes/no confirm
 *===========================================*/
BOOLEAN
swap_families (RECORD irec)
{
	NODE indi, fams, one, two, tmp;
	INT nfam;
	STRING str;

	if (readonly) {
		message(_(qSronlye));
		return FALSE;
	}

/* Find person and assure has >= 2 families */
	if (!irec) irec = ask_for_indi(_(qSidfswp), NOASK1);
	if (!irec) return FALSE;
	indi = nztop(irec);
	if (!(fams = FAMS(indi))) {
		message(_(qSntprnt));
		return FALSE;
	}
	nfam = num_families(indi);
	if (nfam < 2) {
		msg_error(_(qSless2f));
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
		fam1 = nztop(choose_family(irec, _(qSparadox), _(qSid1fsw), TRUE));
		if (!fam1) return FALSE;
		fam2 = nztop(choose_family(irec, _(qSparadox), _(qSid2fsw), TRUE));
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

	ASSERT(one);
	ASSERT(two);
	ASSERT(eqstr(ntag(one), "FAMS"));
	ASSERT(eqstr(ntag(two), "FAMS"));

/* Ask user to confirm */
	if (!ask_yes_or_no(_(qScffswp)))
		return FALSE;

/* Swap FAMS nodes and update database */
	str = nval(one);
	nval(one) = nval(two);
	nval(two) = str;
	tmp = nchild(one);
	nchild(one) = nchild(two);
	nchild(two) = tmp;
	indi_to_dbase(indi);
	message(_(qSokfswp));
	return TRUE;
}
