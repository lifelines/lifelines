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
 * browse.c -- Implements the browse command
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 24 Sep 94
 *   3.0.2 - 16 Oct 94    3.0.2 - 30 Dec 94
 *   3.0.3 - 04 May 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "indiseq.h"
#include "gedcom.h"
#include "translat.h"

extern STRING idsbrs, idsrmv, idfbrs, idcbrs, idcrmv, iscnew, issnew;
extern STRING idfcop, ntprnt, nofath, nomoth, nospse, noysib, noosib;
extern STRING noprnt, nohusb, nowife, hasbth, hasnei, nocinf, nocofp;
extern STRING idpnxt, ids2fm, idc2fm, idplst, idp2br, crtcfm, crtsfm;
extern STRING ronlye, ronlya, idhbrs, idwbrs;
extern STRING id1sbr, id2sbr, id1fbr, id2fbr, id1cbr, id2cbr;
extern STRING id1hbr, id2hbr, id1wbr, id2wbr;

NODE family_to_browse_to();
INDISEQ ask_for_indiseq();

#define ALLPARMS &indi1, &indi2, &fam1, &fam2, &seq

/*=========================================
 * browse -- Main loop of browse operation.
 *=======================================*/
browse (indi1)
NODE indi1;
{
	INT code, len, rc;
	NODE indi2, fam1, fam2;
	STRING key, name;
	INDISEQ seq = NULL;

	if (!indi1 && (seq = ask_for_indiseq(idplst, &rc))) {
		if ((len = length_indiseq(seq)) < 1) return;
		if (len == 1) {
			element_indiseq(seq, 0, &key, &name);
			indi1 = key_to_indi(key);
			remove_indiseq(seq, FALSE);
			seq = NULL;
		}
	}
	if (!indi1 && !seq) return;
	code = indi1 ? BROWSE_INDI : BROWSE_LIST;
	while (code != BROWSE_QUIT) {
		switch (code) {
		case BROWSE_INDI:
			code = browse_indi(ALLPARMS); break;
		case BROWSE_FAM:
			code = browse_fam(ALLPARMS); break;
		case BROWSE_PED:
			code = browse_pedigree(ALLPARMS); break;
		case BROWSE_TAND:
			code = browse_tandem(ALLPARMS); break;
		case BROWSE_2FAM:
			code = browse_2fam(ALLPARMS); break;
		case BROWSE_LIST:
			code = browse_list(ALLPARMS); break;
		}
	}
}
/*================================================
 * browse_indi -- Handle person browse operations.
 *==============================================*/
INT browse_indi (pindi1, pindi2, pfam1, pfam2, pseq)
NODE *pindi1, *pindi2, *pfam1, *pfam2;
INDISEQ *pseq;
{
	STRING key, name, addstrings[2];
	INT i, c, len, rc;
	NODE node, save = NULL, indi = *pindi1;
	NODE node2;
	INDISEQ seq = NULL;
	TRANTABLE ttd = tran_tables[MINDS];
	char scratch[100];

	addstrings[0] = crtcfm;
	addstrings[1] = crtsfm;
	if (!indi) return BROWSE_QUIT;
	while (TRUE) {
		c = indi_browse(indi);
		if (c != 'a') save = NULL;
		switch (c) {
		case 'e':	/* Edit this person */
			indi = edit_indi(indi);
			break;
		case 'g': 	/* Browse to person's family */
			if (*pfam1 = choose_family(indi, ntprnt, idfbrs, TRUE))
				return BROWSE_FAM;
			break;
		case 'G':
			if (*pfam1 = choose_family(indi, ntprnt,
				id1fbr, TRUE))
			  if (*pfam2 = choose_family(indi, ntprnt,
				id2fbr, TRUE))
				return BROWSE_2FAM;
			break;
		case 'f': 	/* Browse to person's father */
			node = choose_father(indi, NULL, nofath,
			    idhbrs, FALSE);
			if (node) indi = node;
			break;
		case 'F':	/* Tandem Browse to person's fathers */
			node = choose_father(indi, NULL, nofath,
			    id1hbr, FALSE);
			if (node) {
			  node2 = choose_father(indi, NULL, nofath,
			    id2hbr, FALSE);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case 'm':	/* Browse to person's mother */
			node = choose_mother(indi, NULL, nomoth,
			    idwbrs, FALSE);
			if (node) indi = node;
			break;
		case 'M':	/* Tandem Browse to person's mothers */
			node = choose_mother(indi, NULL, nomoth,
			    id1wbr, FALSE);
			if (node) {
			  node2 = choose_mother(indi, NULL, nomoth,
			    id2wbr, FALSE);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case 'z':	/* Zip browse another person */
			node = ask_for_indi(idpnxt, FALSE, FALSE);
			if (node) indi = node;
			break;
		case 's':	/* Browse to person's spouse */
			node = choose_spouse(indi, nospse, idsbrs);
			if (node) indi = node;
			break;
		case 'S':	/* browse to tandem spouses */
			node = choose_spouse(indi, nospse, id1sbr);
			if (node) {
			  node2 = choose_spouse(indi, nospse, id2sbr);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case 'c':	/* Browse to person's child */
			node = choose_child(indi, NULL, nocofp,
			    idcbrs, FALSE);
			if (node) indi = node;
			break;
		case 'C':	/* browse to tandem children */
			node = choose_child(indi, NULL, nocofp,
			    id1cbr, FALSE);
			if (node) {
			  node2 = choose_child(indi, NULL, nocofp,
			    id2cbr, FALSE);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case 'p':	/* Switch to pedigree mode */
			*pindi1 = indi;
			return BROWSE_PED;
		case 'o':	/* Browse to older sib */
			if (!(node = indi_to_prev_sib(indi)))
				message(noosib);
			else
				indi = node;
			break;
		case 'y':	/* Browse to younger sib */
			if (!(node = indi_to_next_sib(indi)))
				message(noysib);
			else 
				indi = node;
			break;
		case 'u':	/* Browse to parents' family */
			if (*pfam1 = choose_family(indi, noprnt, idfbrs, FALSE))
				return BROWSE_FAM;
			break;
		case 'U':	/* tandem browse to two parents families*/
			if (*pfam1 = choose_family(indi, noprnt,
				id1fbr, FALSE))
			  if (*pfam2 = choose_family(indi, noprnt,
				id2fbr, FALSE))
				return BROWSE_2FAM;
			break;
		case 'b': 	/* Browse new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if ((len = length_indiseq(seq)) == 1) {
				element_indiseq(seq, 0, &key, &name);
				indi = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case 'n':	/* Add new person */
			if (!(node = add_indi_by_edit())) break;
			save = indi;
			indi = node;
			break;
		case 'a':	/* Add family for current person */
			if (readonly) {
				message(ronlya);
				break;
			}
			i = choose_from_list(idfcop, 2, addstrings);
			if (i == -1) break;
			if (i == 0) node = add_family(NULL, NULL, indi);
			else if (save) {
				if (keyflag)
					sprintf(scratch, "%s%s (%s)", issnew,
				    	    indi_to_name(save, ttd, 55),
					    rmvat(nxref(save))+1);
				else
					sprintf(scratch, "%s%s", issnew,
					    indi_to_name(save, ttd, 55));
				if (ask_yes_or_no(scratch))
					node = add_family(indi, save, NULL);
				else
					node = add_family(indi, NULL, NULL);
			} else
				node = add_family(indi, NULL, NULL);
			save = NULL;
			if (!node) break;
			*pfam1 = node;
			return BROWSE_FAM;
		case 't':	/* Switch to tandem browsing */
			node = ask_for_indi(idp2br, FALSE, FALSE);
			if (node) {
				*pindi1 = indi;
				*pindi2 = node;
				return BROWSE_TAND;
			}
			break;
		case 'x': 	/* Swap families of current person */
			swap_families(indi);
			break;
		case 'h':	/* Add person as spouse */
			add_spouse(indi, NULL, TRUE);
			break;
		case 'i':	/* Add person as child */
			add_child(indi, NULL);
			break;
		case 'r':	/* Remove person as spouse */
			remove_spouse(indi, NULL, FALSE);
			break;
		case 'd':	/* Remove person as child */
			remove_child(indi, NULL, FALSE);
			break;
		case 'A':	/* Advanced person edit */
			advanced_person_edit(indi);
			break;
		case 'q':
		default:
			return BROWSE_QUIT;
		}
	}
}
/*===============================================
 * browse_fam -- Handle family browse selections.
 *=============================================*/
INT browse_fam (pindi, pdum, pfam1, pfam2, pseq)
NODE *pfam1, *pindi, *pdum, *pfam2;
INDISEQ *pseq;
{
	INT i, c, len, rc;
	NODE save = NULL, fam = *pfam1, node, husb, wife, chil, rest;
	NODE root, this, fref, spnodes[30];
	INDISEQ seq;
	STRING key, name, spstrings[2];
	char scratch[100];
	TRANTABLE ttd = tran_tables[MINDS];

	if (!fam) return BROWSE_QUIT;
	while (TRUE) {
		c = fam_browse(fam);
		if (c != 'a' && c != 's') save = NULL;
		switch (c) {
		case 'A':	/* Advanced family edit */
			advanced_family_edit(fam);
			break;
		case 'e':	/* Edit family's record */
			fam = edit_family(fam);
			break;
		case 'f':	/* Browse to family's father */
			*pindi = choose_father(NULL, fam, nohusb,
			    idhbrs, FALSE);
			if (*pindi) return BROWSE_INDI;
			break;
		case 'F':	/* Tandem Browse to family's fathers */
			*pindi = choose_father(NULL, fam, nohusb,
			    id1hbr, FALSE);
			if (*pindi) {
			  *pdum = choose_father(NULL, fam, nohusb,
			    id2hbr, FALSE);
			  if (*pdum) 
				return BROWSE_TAND;
			}
			break;
		case 'm':	/* Browse to family's mother */
			*pindi = choose_mother(NULL, fam, nowife,
			    idwbrs, FALSE);
			if (*pindi) return BROWSE_INDI;
			break;
		case 'M':	/* Tandem Browse to family's mother */
			*pindi = choose_mother(NULL, fam, nowife,
			    id1wbr, FALSE);
			if (*pindi) {
			  *pdum = choose_mother(NULL, fam, nowife,
			    id2wbr, FALSE);
			  if (*pdum) 
				return BROWSE_TAND;
			}
			break;
		case 'c':	/* Browse to a child */
			*pindi = choose_child(NULL, fam, nocinf,
			    idcbrs, FALSE);
			if (*pindi) return BROWSE_INDI;
			break;
		case 'C':	/* browse to tandem children */
			*pindi = choose_child(NULL, fam, nocinf,
			    id1cbr, FALSE);
			if (*pindi) {
			  *pdum = choose_child(NULL, fam, nocinf,
			    id2cbr, FALSE);
			  if (*pdum) 
				return BROWSE_TAND;
			}
			break;
		case 'd':	/* Remove a child */
			if (readonly) {
				message(ronlye);
				break;
			}
			*pindi = choose_child(NULL, fam, nocinf,
			    idcrmv, TRUE);
			if (*pindi) remove_child(*pindi, NULL, TRUE);
			break;
		case 's':	/* Add spouse to family */
			if (readonly) {
				message(ronlye);
				break;
			}
			split_fam(fam, &fref, &husb, &wife, &chil, &rest);
			join_fam(fam, fref, husb, wife, chil, rest);
#if 0
			if (husb && wife) {
				message(hasbth);
				break;
			}
#endif
			if (save) {
				if (keyflag)
					sprintf(scratch, "%s%s (%s)", issnew,
					    indi_to_name(save, ttd, 56),
					    rmvat(nxref(save))+1);
				else
					sprintf(scratch, "%s%s", issnew,
					    indi_to_name(save, ttd, 56));
				if (ask_yes_or_no(scratch)) {
					add_spouse(save, fam, FALSE);
					save = NULL;
					break;
				}
			}
			add_spouse(NULL, fam, TRUE);
			save = NULL;
			break;
		case 'r':	/* Remove spouse from family */
			if (readonly) {
				message(ronlye);
				break;
			}
			split_fam(fam, &fref, &husb, &wife, &chil, &rest);
			if (!husb && !wife) {
				message(hasnei);
				break;
			}
			i = 0;
			for (node = husb; node; node = nsibling(node)) {
				root = key_to_indi(rmvat(nval(node)));
				spstrings[i] = indi_to_list_string(root,
				    NULL, 66);
				spnodes[i++] = root;
			}
			for (node = wife; node; node = nsibling(node)) {
				root = key_to_indi(rmvat(nval(node)));
				spstrings[i] = indi_to_list_string(root,
				    NULL, 66);
				spnodes[i++] = root;
			}
			join_fam(fam, fref, husb, wife, chil, rest);
			i = choose_from_list(idsrmv, i, spstrings);
			if (i == -1) break;
			remove_spouse(spnodes[i], fam, TRUE);
			break;
		case 'n':	/* Add person to database */
			save = add_indi_by_edit();
			break;
		case 'a':	/* Add child to family */
			if (readonly) {
				message(ronlye);
				break;
			}
			if (save) {
				if (keyflag)
					sprintf(scratch, "%s%s (%s)", iscnew,
					    indi_to_name(save, ttd, 56),
					    rmvat(nxref(save))+1);
				else
					sprintf(scratch, "%s%s", iscnew,
					    indi_to_name(save, ttd, 56));
				if (ask_yes_or_no(scratch)) {
					add_child(save, fam);
					save = NULL;
					break;
				}
			}
			add_child(NULL, fam);
			save = NULL;
			break;
		case 'b': 	/* Browse to new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if ((len = length_indiseq(seq)) == 1) {
				element_indiseq(seq, 0, &key, &name);
				*pindi = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				return BROWSE_INDI;
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case 'z':	/* Zip browse to new person */
			*pindi = ask_for_indi(idpnxt, FALSE, FALSE);
			if (*pindi) return BROWSE_INDI;
			break;
		case 't':	/* Enter family tandem mode */
			node = ask_for_fam(ids2fm, idc2fm);
			if (node) {
				*pfam1 = fam;
				*pfam2 = node;
				return BROWSE_2FAM;
			}
			break;
		case 'x':	/* Swap two children */
			swap_children(NULL, fam);
			break;
		case 'q':
		default:
			return BROWSE_QUIT;
		}
	}
}
/*======================================================
 * browse_pedigree -- Handle pedigree browse selections.
 *====================================================*/
INT browse_pedigree (pindi, pdum1, pfam, pdum2, pseq)
NODE *pindi, *pfam, *pdum1, *pdum2;
INDISEQ *pseq;
{
	NODE node, indi = *pindi;
	INT rc, len;
	STRING key, name;
	INDISEQ seq = NULL;
	if (!indi) return BROWSE_QUIT;
	while (TRUE) {
		switch (ped_browse(indi)) {
		case 'e':	/* Edit person */
			indi = edit_indi(indi);
			break;
		case 'i':	/* Switch to person browse mode */
			*pindi = indi;
			return BROWSE_INDI;
		case 'f':	/* Browse to father */
			node = choose_father(indi, NULL, nofath,
			    idhbrs, FALSE);
			if (node) indi = node;
			break;
		case 'm':	/* Browse to mother */
			node = choose_mother(indi, NULL, nomoth,
			    idwbrs, FALSE);
			if (node) indi = node;
			break;
		case 's':	/* Browse to spouse */
			node = choose_spouse(indi, nospse, idsbrs);
			if (node) indi = node;
			break;
		case 'c':	/* Browse to children */
			if (node = choose_child(indi, NULL, nocofp,
			    idcbrs, FALSE))
				indi = node;
			break;
		case 'g':	/* Switch to family mode */
			if (*pfam = choose_family(indi, ntprnt, idfbrs, TRUE))
				return BROWSE_FAM;
			break;
		case 'b': 	/* Browse new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if ((len = length_indiseq(seq)) == 1) {
				element_indiseq(seq, 0, &key, &name);
				indi = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case 'q':
		default:
			return BROWSE_QUIT;
		}
	}
}
