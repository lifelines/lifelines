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

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

#include "llinesi.h"

extern STRING nochil, nopers, nofam, nosour, idsour;
extern STRING nosour, idsour, noeven, ideven, noothe, idothe;
extern STRING idsbrs, idsrmv, idfbrs, idcbrs, idcrmv, iscnew, issnew;
extern STRING idfcop, ntprnt, nofath, nomoth, nospse, noysib, noosib;
extern STRING noprnt, nohusb, nowife, hasbth, hasnei, nocinf, nocofp;
extern STRING idpnxt, ids2fm, idc2fm, idplst, idp2br, crtcfm, crtsfm;
extern STRING ronlye, ronlya, idhbrs, idwbrs;
extern STRING id1sbr, id2sbr, id1fbr, id2fbr, id1cbr, id2cbr;
extern STRING id1hbr, id2hbr, id1wbr, id2wbr;

static INT browse_indi(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static INT browse_fam(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static INT browse_pedigree(NODE*, NODE*, NODE*, NODE*, INDISEQ*);

#define ALLPARMS &indi1, &indi2, &fam1, &fam2, &seq

/*=========================================
 * browse -- Main loop of browse operation.
 *=======================================*/
void
browse (NODE indi1)
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
 * goto_indi_child - jump to child by number
 *==============================================*/
NODE
goto_indi_child(NODE indi, int childno)
{
	INT num1, num2, i = 0;
	NODE answer = 0;
	if (!indi) return NULL;
	FORFAMSS(indi, fam, spouse, num1)
		FORCHILDREN(fam, chil, num2)
			i++;
			if (i == childno) answer = chil;
		ENDCHILDREN
	ENDFAMSS
	return answer;
}

/*================================================
 * goto_fam_child - jump to child by number
 *==============================================*/
NODE
goto_fam_child(NODE fam, int childno)
{
	INT num, i = 0;
	NODE answer = 0;
	if (!fam) return NULL;
	FORCHILDREN(fam, chil, num)
		i++;
		if (i == childno) answer = chil;
	ENDCHILDREN
	return answer;
}
/*================================================
 * browse_indi -- Handle person browse operations.
 *==============================================*/
static INT
browse_indi (NODE *pindi1,
             NODE *pindi2,
             NODE *pfam1,
             NODE *pfam2,
             INDISEQ *pseq)
{
	STRING key, name, addstrings[2];
	INT i, c, rc;
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
			if ((*pfam1 = choose_family(indi, ntprnt,
				idfbrs, TRUE)))
				return BROWSE_FAM;
			break;
		case 'G':
			if ((*pfam1 = choose_family(indi, ntprnt,
				id1fbr, TRUE)))
			  if ((*pfam2 = choose_family(indi, ntprnt,
				id2fbr, TRUE)))
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
			node = ask_for_indi(idpnxt, NOCONFIRM, FALSE);
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
		case '(':       /* scroll children (& spouses) up */
			show_scroll(-1);
			break;
		case ')':       /* scroll children (& spouses) down */
			show_scroll(+1);
			break;
		case '#':       /* toggle children numbers */
			show_childnumbers();
			break;
		case '1':	/* Go to children by number */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			node = goto_indi_child(indi, c-'0');
			if (node) indi = node;
			else message(nochil);
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
			if ((*pfam1 = choose_family(indi, noprnt,
				idfbrs, FALSE)))
				return BROWSE_FAM;
			break;
		case 'U':	/* tandem browse to two parents families*/
			if ((*pfam1 = choose_family(indi, noprnt,
				id1fbr, FALSE)))
			  if ((*pfam2 = choose_family(indi, noprnt,
				id2fbr, FALSE)))
				return BROWSE_2FAM;
			break;
		case 'b': 	/* Browse new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
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
			node = ask_for_indi(idp2br, NOCONFIRM, FALSE);
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
		case '+':	/* Go to next indi in db */
			{
				i = atoi(key_of_record(indi));
				i = xref_nexti(i);
				if (i)
					indi = rkeynum_to_indi(i);
				else message(nopers);
				break;
			}
		case '-':	/* Go to prev indi in db */
			{
				i = atoi(key_of_record(indi));
				i = xref_previ(i);
				if (i)
					indi = rkeynum_to_indi(i);
				else message(nopers);
				break;
			}
		case '$':	/* Browse to sources */
			node = choose_source(indi, nosour, idsour);
			if (node)
				edit_source(node);
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
static INT
browse_fam (NODE *pindi,
            NODE *pdum,
            NODE *pfam1,
            NODE *pfam2,
            INDISEQ *pseq)
{
	INT i, c, rc;
	NODE save = NULL, fam = *pfam1, node, husb, wife, chil, rest;
	NODE root, fref, spnodes[30];
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
		case 'B':
			i = ask_for_int("Enter Family Number to Browse to");
			if(i > 0) {
			    sprintf(scratch, "F%d", i);
			    if((node = key_to_fam(scratch))) {
				fam = node;
			    }
			}
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
			if (length_indiseq(seq) == 1) {
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
			*pindi = ask_for_indi(idpnxt, NOCONFIRM, FALSE);
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
		case '(':       /* scroll children up */
			show_scroll(-1);
			break;
		case ')':       /* scroll children down */
			show_scroll(+1);
			break;
		case '#':       /* toggle children numbers */
			show_childnumbers();
			break;
		case '1':	/* Go to children by number */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			*pindi = goto_fam_child(fam, c-'0');
			if (*pindi) return BROWSE_INDI;
			message(nochil);
			break;
		case '+':	/* Go to next fam in db */
			{
				i = atoi(key_of_record(fam));
				i = xref_nextf(i);
				if (i)
					fam = rkeynum_to_fam(i);
				else message(nofam);
				break;
			}
		case '-':	/* Go to prev indi in db */
			{
				i = atoi(key_of_record(fam));
				i = xref_prevf(i);
				if (i)
					fam = rkeynum_to_fam(i);
				else message(nofam);
				break;
			}
		case '$':	/* Browse to sources */
			node = choose_source(fam, nosour, idsour);
			if (node)
				edit_source(node);
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
static INT
browse_pedigree (NODE *pindi,
                 NODE *pdum1,
                 NODE *pfam,
                 NODE *pdum2,
                 INDISEQ *pseq)
{
	NODE node, indi = *pindi;
	INT c, i, rc;
	STRING key, name;
	INDISEQ seq = NULL;
	if (!indi) return BROWSE_QUIT;
	pedigree_reset_scroll();
	while (TRUE) {
		switch (c=ped_browse(indi)) {
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
			if ((node = choose_child(indi, NULL, nocofp,
			    idcbrs, FALSE)))
				indi = node;
			break;
		case 'g':	/* Switch to family mode */
			if ((*pfam = choose_family(indi, ntprnt, idfbrs, TRUE)))
				return BROWSE_FAM;
			break;
		case 'b': 	/* Browse new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				indi = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case '&':       /* toggle pedigree mode (ancestors/descendants) */
			pedigree_toggle_mode();
			break;
		case '(':       /* scroll pedigree up */
			pedigree_scroll(-1);
			break;
		case ')':       /* scroll pedigree down */
			pedigree_scroll(+1);
			break;
		case '[':       /* decrease pedigree depth */
			pedigree_increase_generations(-1);
			break;
		case ']':       /* increase pedigree depth */
			pedigree_increase_generations(+1);
			break;
		case '1':	/* Go to children by number */
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			node = goto_indi_child(indi, c-'0');
			if (node) indi = node;
			else message(nochil);
			break;
		case '+':	/* Go to next indi in db */
			{
				i = atoi(key_of_record(indi));
				i = xref_nexti(i);
				if (i)
					indi = rkeynum_to_indi(i);
				else message(nopers);
				break;
			}
		case '-':	/* Go to prev indi in db */
			{
				i = atoi(key_of_record(indi));
				i = xref_previ(i);
				if (i)
					indi = rkeynum_to_indi(i);
				else message(nopers);
				break;
			}
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
		case '$':	/* Browse to sources */
			node = choose_source(indi, nosour, idsour);
			if (node)
				edit_source(node);
			break;
		case 'q':
		default:
			return BROWSE_QUIT;
		}
	}
}
/*==================================================
 * choose_any_source -- choose from list of all sources
 *================================================*/
NODE choose_any_source()
{
	INDISEQ seq;
	NODE node;
	seq = get_all_sour();
	if (!seq)
	{
		message(nosour);
		return 0;
	}
	node = format_and_choose_generic(seq, TRUE, idsour, idsour);
	remove_indiseq(seq, FALSE);
	return node;
}
/*==================================================
 * browse_sources -- browse list of all sources
 *================================================*/
void browse_sources ()
{
	NODE node = choose_any_source();
	if (node)
		edit_source(node);
}
/*==================================================
 * browse_events -- browse list of all events
 *================================================*/
void browse_events ()
{
	INDISEQ seq = get_all_even();
	NODE node;
	if (!seq)
	{
		message(noeven);
		return;
	}
	node = format_and_choose_generic(seq, TRUE, ideven, ideven);
	remove_indiseq(seq, FALSE);
	if (node)
		edit_event(node);
}
/*==================================================
 * browse_others -- browse list of all sources
 *================================================*/
void browse_others ()
{
	INDISEQ seq = get_all_othe();
	NODE node;
	if (!seq)
	{
		message(noothe);
		return;
	}
	node = format_and_choose_generic(seq, TRUE, idothe, idothe);
	remove_indiseq(seq, FALSE);
	if (node)
		edit_other(node);
}
