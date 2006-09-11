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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * delete.c -- Removes person and family records from database 
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 15 Aug 93
 *   3.0.0 - 30 Jun 94    3.0.2 - 10 Dec 94
 *   3.0.3 - 21 Jan 96
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "llinesi.h"
#include "feedback.h"


extern STRING qSidpdel, qSidodel, qScfpdel, qScfodel, qScffdel, qScffdeld;
extern STRING qSidfrmv, qSidfrsp, qSidfrch;
extern STRING qSidcrmv, qSntchld, qSntprnt, qSidsrmv, qSidsrmf, qSnormls, qScfcrmv;
extern STRING qSokcrmv, qSntsinf, qSntcinf, qScfsrmv, qSoksrmv, qSronlye, qSidcrmf;
extern STRING qSparadox;

/*=====================================================
 * choose_and_remove_family -- Choose & delete a family
 *  (remove all members, and delete F record)
 *===================================================*/
void
choose_and_remove_family (void)
{
	NODE fam, node, indi;
	INDISEQ spseq, chseq;
	STRING tag, key;
	char confirm[512]="", members[64];
	char spouses[32], children[32];
	INT n;

	fam = nztop(ask_for_fam_by_key(_(qSidfrmv), _(qSidfrsp), _(qSidfrch)));
	if (!fam)
		return;

	/* get list of spouses & children */
	spseq = create_indiseq_null(); /* spouses */
	chseq = create_indiseq_null(); /* children */
	for (node=nchild(fam); node; node = nsibling(node)) {
		tag = ntag(node);
		if (eqstr("HUSB", tag) || eqstr("WIFE", tag)) {
			key = strsave(rmvat(nval(node)));
			append_indiseq_null(spseq, key, NULL, TRUE, TRUE);
		} else if (eqstr("CHIL", tag)) {
			key = strsave(rmvat(nval(node)));
			append_indiseq_null(chseq, key, NULL, TRUE, TRUE);
		}
	}

	/* build confirm string */
	n = ISize(spseq);
	llstrsetf(spouses, sizeof(spouses), uu8
		, _pl("%d spouse", "%d spouses", n), n);
	n = ISize(chseq);
	llstrsetf(children, sizeof(children), uu8
		, _pl("%d child", "%d children", n), n);
	llstrsetf(members, sizeof(members), uu8
		, _(qScffdeld), fam_to_key(fam), spouses, children);
	llstrapps(confirm, sizeof(confirm), uu8, _(qScffdel));
	llstrapps(confirm, sizeof(confirm), uu8, members);

	if (ask_yes_or_no(confirm)) {

		if (ISize(spseq)+ISize(chseq) == 0) {
			/* handle empty family */
			remove_empty_fam(fam);
		}
		else {
			/* the last remove command will delete the family */
			FORINDISEQ(spseq, el, num)
				indi = key_to_indi(element_skey(el));
				remove_spouse(indi, fam);
			ENDINDISEQ

			FORINDISEQ(chseq, el, num)
				indi = key_to_indi(element_skey(el));
				remove_child(indi, fam);
			ENDINDISEQ
		}
	}
	
	remove_indiseq(spseq);
	remove_indiseq(chseq);
}
/*================================================================
 * choose_and_remove_indi -- Prompt & delete person and links; if this leaves families
 *   with no links, remove them
 *  indi:  [in]  person to remove - (if null, will ask for person)
 *  conf:  [in]  have user confirm ?
 *==============================================================*/
void
choose_and_remove_indi (NODE indi, CONFIRMQ confirmq)
{
	/* prompt if needed */
	if (!indi && !(indi = nztop(ask_for_indi(_(qSidpdel), DOASK1))))
		return;
	/* confirm if caller desired */
	if (confirmq==DOCONFIRM && !ask_yes_or_no(_(qScfpdel))) return;

	/* alright, we finished the UI, so delegate to the internal workhorse */
	remove_indi_by_root(indi);
}
/*================================================================
 * choose_and_remove_any_record -- Prompt & delete any record
 *   (delete any empty families produced)
 *  record:  [in]  record to remove (if null, will ask for record)
 *  conf:  [in]  have user confirm ?
 *==============================================================*/
BOOLEAN
choose_and_remove_any_record (RECORD record, CONFIRMQ confirmq)
{
	/* prompt if needed */
	if (!record && !(record = ask_for_any(_(qSidodel), DOASK1)))
		return FALSE;
	/* confirm if caller desired */
	if (confirmq==DOCONFIRM && !ask_yes_or_no(_(qScfodel)))
		return FALSE;

	/* alright, we finished the UI, so delegate to the internal workhorse */
	return remove_any_record(record);
}
/*===========================================
 * choose_and_remove_spouse -- Remove spouse 
 *  from family (prompting for spouse and/or family
 *  if NULL passed)
 *  nolast: don't remove last member of family?
 *=========================================*/
BOOLEAN
choose_and_remove_spouse (RECORD irec, RECORD frec, BOOLEAN nolast)
{
	NODE fam;

	if (readonly) {
		message(_(qSronlye));
		return FALSE;
	}

/* Identify spouse to remove */
	if (!irec) irec = ask_for_indi(_(qSidsrmv), NOASK1);
	if (!irec) return FALSE;
	if (!FAMS(nztop(irec))) {
		message(_(qSntprnt));
		return FALSE;
	}

/* Identify family to remove spouse from */
	if (!frec) frec = choose_family(irec, _(qSparadox), _(qSidsrmf), TRUE);
	if (!frec) return FALSE;
	fam = nztop(frec);
	if (nolast && num_fam_xrefs(fam) < 2) {
		message(_(qSnormls));
		return FALSE;
	}
	if (!ask_yes_or_no(_(qScfsrmv))) return FALSE;

	/* call internal workhorse remove_spouse() to do the actual removal */
	if (!remove_spouse(nztop(irec), fam)) {
		message(_(qSntsinf));
		return FALSE;
	}
	message(_(qSoksrmv));
	return TRUE;
}
/*===========================================
 * choose_and_remove_child -- Remove child
 *  from family (prompting for child and/or family
 *  if NULL passed)
 *  nolast: don't remove last member of family?
 *=========================================*/
BOOLEAN
choose_and_remove_child (RECORD irec, RECORD frec, BOOLEAN nolast)
{
	NODE fam;

	if (readonly) {
		message(_(qSronlye));
		return FALSE;
	}
		
/* Identify child and check for FAMC nodes */
	if (!irec) irec = ask_for_indi(_(qSidcrmv), NOASK1);
	if (!irec) return FALSE;
	if (!FAMC(nztop(irec))) {
		message(_(qSntchld));
		return FALSE;
	}

/* Identify family to remove child from */
	if (!frec) frec = choose_family(irec, _(qSparadox), _(qSidcrmf), FALSE);
	if (!frec) return FALSE;
	fam = nztop(frec);
	if (nolast && num_fam_xrefs(fam) < 2) {
		message(_(qSnormls));
		return FALSE;
	}
	if (!ask_yes_or_no(_(qScfcrmv))) return TRUE;

	if (!remove_child(nztop(irec), fam)) {
		message(_(qSntcinf));
		return FALSE;
	}

	message(_(qSokcrmv));
	return TRUE;
}
