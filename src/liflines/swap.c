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
 *   2.3.4 - 24 Jun 93    2.3.5 - 27 Sep 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 23 Sep 94
 *   3.0.2 - 02 Dec 94
 *===============================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

extern STRING idcswp, id1csw, id2csw, id1fsw, id2fsw, idfbys, ntprnt;
extern STRING less2c, okcswp, less2f, okfswp, idfswp, ronlye;

/*=============================================
 * swap_children -- Swap two children in family
 *===========================================*/
BOOLEAN swap_children (NODE prnt,	/* parent - poss NULL */
		       NODE fam)	/* family - poss NULL */
{
	NODE chil1, chil2, chil, one, two, tmp;
	STRING key1, key2, str;
	INT nfam, nchil;

	if (readonly) {
		message(ronlye);
		return FALSE;
	}

/* Identify parent if need be */
	if (fam) goto gotfam;
	if (!prnt) prnt = ask_for_indi(idfswp, FALSE, FALSE);
	if (!prnt) return FALSE;
	nfam = num_families(prnt);
	if (nfam <= 0) {
		message(ntprnt);
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

/* Identify children to swap */
	chil1 = choose_child(NULL, fam, "e", id1csw, FALSE);
	if (!chil1) return FALSE;
	chil2 = choose_child(NULL, fam, "e", id2csw, FALSE);
	if (!chil2) return FALSE;
	if (chil1 == chil2) return FALSE;
	key1 = nxref(chil1);
	key2 = nxref(chil2);

   /* Swap CHIL nodes and update database */
	ASSERT(chil = CHIL(fam));
	one = two = NULL;
	for (;  chil;  chil = nsibling(chil)) {
		if (eqstr(key1, nval(chil))) one = chil;
		if (eqstr(key2, nval(chil))) two = chil;
	}
	ASSERT(one && two);
	str = nval(one);
	nval(one) = nval(two);
	nval(two) = str;
	tmp = nchild(one);
	nchild(one) = nchild(two);
	nchild(two) = tmp;
	fam_to_dbase(fam);
	message(okcswp);
	return TRUE;
}
/*=============================================
 * swap_families -- Swap two families of person
 *===========================================*/
BOOLEAN swap_families (NODE indi)
{
	NODE fam1, fam2, fams, one, two, tmp;
	INT nfam;
	STRING str, key1, key2;

	if (readonly) {
		message(ronlye);
		return FALSE;
	}

/* Find person and assure has >= 2 families */
	if (!indi) indi = ask_for_indi(idfswp, FALSE, FALSE);
	if (!indi) return FALSE;
	if (!(fams = FAMS(indi))) {
		message(ntprnt);
		return FALSE;
	}
	nfam = num_families(indi);
	if (nfam < 2) {
		message(less2f);
		return FALSE;
	}

/* Find families to swap */
	fam1 = choose_family(indi, "e", id1fsw, TRUE);
	if (!fam1) return FALSE;
	fam2 = choose_family(indi, "e", id2fsw, TRUE);
	if (!fam2) return FALSE;
	if (fam1 == fam2) return FALSE;
	key1 = nxref(fam1);
	key2 = nxref(fam2);

/* Swap FAMS nodes and update database */
	ASSERT(fams = FAMS(indi));
	one = two = NULL;
	for (;  fams;  fams = nsibling(fams)) {
		if (eqstr(key1, nval(fams))) one = fams;
		if (eqstr(key2, nval(fams))) two = fams;
	}
	ASSERT(one && two);
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
