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
#include "menuitem.h"

#include "llinesi.h"

/*********************************************
 * global/exported variables
 *********************************************/

NODE jumpnode; /* used by Ethel for direct navigation */

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING nochil, nopers, nofam, nosour, idsour, norec;
extern STRING nosour, idsour, noeven, ideven, noothe, idothe;
extern STRING nonote, idnote, noptr, idptr;
extern STRING idsbrs, idsrmv, idfbrs, idcbrs, idcrmv, iscnew, issnew;
extern STRING idfcop, ntprnt, nofath, nomoth, nospse, noysib, noosib;
extern STRING noprnt, nohusb, nowife, hasbth, hasnei, nocinf, nocofp;
extern STRING idpnxt, ids2fm, idc2fm, idplst, idp2br, crtcfm, crtsfm;
extern STRING ronlye, ronlya, idhbrs, idwbrs;
extern STRING id1sbr, id2sbr, id1fbr, id2fbr, id1cbr, id2cbr;
extern STRING id1hbr, id2hbr, id1wbr, id2wbr;
extern STRING spover, idfamk, nohist;
/*
	The history list is a circular buffer in hist_list.
	hist_start points at the earliest entry, and 
	hist_past_end points just past the latest entry.
	If hist_start==hist_past_end the the buffer is full.
	(If hist_start==-1, then there are no entries.)
	(NB: CMD_HISTORY_FWD will go ahead to unused entries.)
	-1 <= hist_start < ARRSIZE(hist_list)
	0 <= hist_past_end < ARRSIZE(hist_list)
*/
static INT hist_start=-1, hist_past_end=0;
static NKEY hist_list[20];

/*********************************************
 * local enums & defines
 *********************************************/

#define ALLPARMS &indi1, &indi2, &fam1, &fam2, &seq
#define MAX_SPOUSES 30

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static INT browse_aux(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT browse_indi(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT browse_fam(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT browse_indi_modes(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq, INT indimode);
static INT browse_pedigree(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static INT display_aux(NODE node, INT mode);
static INT display_fam(NODE fam, INT fammode);
static INT display_indi(NODE indi, INT mode);
static NODE goto_fam_child(NODE fam, int childno);
static NODE goto_indi_child(NODE indi, int childno);
static BOOLEAN handle_aux_mode_cmds(INT c, INT * mode);
static BOOLEAN handle_menu_commands_old(INT c);
static NODE history_back(void);
static void history_record(NODE node);
static NODE history_fwd(void);
static void pick_add_child_to_fam(NODE fam, NODE save);
static void pick_add_spouse_to_family(NODE fam, NODE save);
static NODE pick_create_new_family(NODE indi, NODE save, STRING * addstrings);
static void pick_remove_spouse_from_family(NODE fam);

/*********************************************
 * local variables
 *********************************************/


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=========================================
 * prompt_for_browse -- prompt for browse target
 *  when only type of browse is known
 * Created: 2001/02/25, Perry Rapp
 *=======================================*/
static void
prompt_for_browse (NODE * node, INT * code, INDISEQ * seq)
{
	NOD0 nod0;
	INT len, rc;
	STRING key, name;

	if (*code == BROWSE_INDI) {
		*seq = ask_for_indiseq(idplst, &rc);
		if (!*seq) return;
		if ((len = length_indiseq(*seq)) < 1) return;
		if (len == 1) {
			element_indiseq(*seq, 0, &key, &name);
			*node = key_to_indi(key);
			remove_indiseq(*seq, FALSE);
			*seq = NULL;
		} else {
			*code = BROWSE_LIST;
		}
		return;
	}
	if (*code == BROWSE_EVEN) {
		nod0 = choose_any_event();
		*node = nztop(nod0);
		return;
	}
	if (*code == BROWSE_SOUR) {
		nod0 = choose_any_source();
		*node = nztop(nod0);
		return;
	}
	nod0 = choose_any_other();
	*node = nztop(nod0);
	return;
}
/*=========================================
 * browse -- Main loop of browse operation.
 *  node may be NULL (then prompt for indi)
 *=======================================*/
void
browse (NODE node, INT code)
{
	NODE indi1, indi2, fam1, fam2;
	INDISEQ seq = NULL;
	STRING key;

	if (!node)
		prompt_for_browse(&node, &code, &seq);

	if (!node && !seq) return;


	/* set up ALLPARMS if not list */
	if (node) {
		fam1 = indi1 = node;
	}

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
		case BROWSE_EVEN:
		case BROWSE_SOUR:
		case BROWSE_AUX:
			code = browse_aux(ALLPARMS); break;
		case BROWSE_UNK:
			key = rmvat(nxref(indi1));
			switch(key[0]) {
			case 'I': code=BROWSE_INDI; break;
			case 'F': code=BROWSE_FAM; break;
			default: code=BROWSE_AUX; break;
			}
		}
	}
}
/*================================================
 * goto_indi_child - jump to child by number
 *==============================================*/
static NODE
goto_indi_child (NODE indi, int childno)
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
static NODE
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
/*===============================================
 * pick_create_new_family -- 
 *  pulled out of browse_indi, 2001/02/04, Perry Rapp
 *=============================================*/
static NODE
pick_create_new_family (NODE indi, NODE save, STRING * addstrings)
{
	INT i;
	NODE node=0;
	TRANTABLE ttd = tran_tables[MINDS];
	char scratch[100];
	if (readonly) {
		message(ronlya);
		return NULL;
	}
	i = choose_from_list(idfcop, 2, addstrings);
	if (i == -1) return NULL;
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
	return node;
}
/*==========================================
 * display_indi -- Show indi in current mode
 *========================================*/
static INT
display_indi (NODE indi, INT mode)
{
	CACHEEL icel;
	INT c;
	icel = indi_to_cacheel(indi);
	lock_cache(icel);
	c = indi_browse(indi, mode);
	unlock_cache(icel);
	return c;
}
/*==========================================
 * display_2indi -- Show two indi in current mode
 *========================================*/
INT
display_2indi (NODE indi1, NODE indi2, INT mode)
{
	CACHEEL icel1, icel2;
	INT c;
	icel1 = indi_to_cacheel(indi1);
	icel2 = indi_to_cacheel(indi2);
	lock_cache(icel1);
	lock_cache(icel2);
	c = twoindi_browse(indi1, indi2, mode);
	unlock_cache(icel1);
	unlock_cache(icel2);
	return c;
}
/*====================================================
 * browse_indi_modes -- Handle person/pedigree browse.
 *==================================================*/
static INT
browse_indi_modes (NODE *pindi1,
                   NODE *pindi2,
                   NODE *pfam1,
                   NODE *pfam2,
                   INDISEQ *pseq,
                   INT indimode
                   )
{
	STRING key, name, addstrings[2];
	INT i, c, rc;
	INT nkeyp, indimodep;
	NODE node, save = NULL, indi = *pindi1;
	NODE node2;
	INDISEQ seq = NULL;
	TRANTABLE ttd = tran_tables[MINDS];

	addstrings[0] = crtcfm;
	addstrings[1] = crtsfm;
	if (!indi) return BROWSE_QUIT;
	show_reset_scroll();
	nkeyp = 0;
	indimodep = indimode;

	while (TRUE) {
		if (indi_to_keynum(indi) != nkeyp 
			|| indimode != indimodep) {
			show_reset_scroll();
		}
		history_record(indi);
		c = display_indi(indi, indimode);
		/* last keynum & mode, so can tell if changed */
		nkeyp = indi_to_keynum(indi);
		indimodep = indimode;
		if (c != CMD_NEWFAMILY) save = NULL;
		if (!handle_menu_cmds(c)
			&& !handle_scroll_cmds(c)
			&& !handle_indi_mode_cmds(c, &indimode))
			switch (c)
		{
		case CMD_EDIT:	/* Edit this person */
			node = edit_indi(indi);
			if (node)
				indi = node;
			break;
		case CMD_FAMILY: 	/* Browse to person's family */
			if ((*pfam1 = choose_family(indi, ntprnt, 
				idfbrs, TRUE)))
				return BROWSE_FAM;
			break;
		case CMD_TANDEM_FAMILIES:
			if ((*pfam1 = choose_family(indi, ntprnt,
				id1fbr, TRUE)))
			  if ((*pfam2 = choose_family(indi, ntprnt,
				id2fbr, TRUE)))
				return BROWSE_2FAM;
			break;
		case CMD_FATHER: 	/* Browse to person's father */
			node = choose_father(indi, NULL, nofath,
			    idhbrs, NOASK1);
			if (node) indi = node;
			break;
		case CMD_TANDEM_FATHERS:	/* Tandem Browse to person's fathers */
			node = choose_father(indi, NULL, nofath,
			    id1hbr, NOASK1);
			if (node) {
			  node2 = choose_father(indi, NULL, nofath,
			    id2hbr, NOASK1);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case CMD_MOTHER:	/* Browse to person's mother */
			node = choose_mother(indi, NULL, nomoth,
			    idwbrs, NOASK1);
			if (node) indi = node;
			break;
		case CMD_TANDEM_MOTHERS:	/* Tandem Browse to person's mothers */
			node = choose_mother(indi, NULL, nomoth,
			    id1wbr, NOASK1);
			if (node) {
			  node2 = choose_mother(indi, NULL, nomoth,
			    id2wbr, NOASK1);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case CMD_BROWSE_ZIP:	/* Zip browse another person */
			node = ask_for_indi(idpnxt, NOCONFIRM, NOASK1);
			if (node) indi = node;
			break;
		case CMD_SPOUSE:	/* Browse to person's spouse */
			node = choose_spouse(indi, nospse, idsbrs);
			if (node) indi = node;
			break;
		case CMD_TANDEM_SPOUSES:	/* browse to tandem spouses */
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
		case CMD_CHILDREN:	/* Browse to person's child */
			node = choose_child(indi, NULL, nocofp,
			    idcbrs, NOASK1);
			if (node) indi = node;
			break;
		case CMD_TOGGLE_PEDTYPE:       /* toggle pedigree mode (ancestors/descendants) */
			pedigree_toggle_mode();
			break;
		case CMD_DEPTH_DOWN:       /* decrease pedigree depth */
			pedigree_increase_generations(-1);
			break;
		case CMD_DEPTH_UP:      /* increase pedigree depth */
			pedigree_increase_generations(+1);
			break;
		case CMD_TOGGLE_CHILDNUMS:       /* toggle children numbers */
			show_childnumbers();
			break;
		case CMD_CHILD_DIRECT0+1:	/* Go to children by number */
		case CMD_CHILD_DIRECT0+2:
		case CMD_CHILD_DIRECT0+3:
		case CMD_CHILD_DIRECT0+4:
		case CMD_CHILD_DIRECT0+5:
		case CMD_CHILD_DIRECT0+6:
		case CMD_CHILD_DIRECT0+7:
		case CMD_CHILD_DIRECT0+8:
		case CMD_CHILD_DIRECT0+9:
			node = goto_indi_child(indi, c-CMD_CHILD_DIRECT0);
			if (node) indi = node;
			else message(nochil);
			break;
		case CMD_TANDEM_CHILDREN:	/* browse to tandem children */
			node = choose_child(indi, NULL, nocofp,
			    id1cbr, NOASK1);
			if (node) {
			  node2 = choose_child(indi, NULL, nocofp,
			    id2cbr, NOASK1);
			  if (node2) {
				*pindi1 = node;
				*pindi2 = node2;
				return BROWSE_TAND;
			  }
			}
			break;
		case CMD_PEDIGREE:	/* Switch to pedigree mode */
			*pindi1 = indi;
			if (indimode == 'i')
				indimode = 'p';
			else
				indimode = 'i';
			break;
		case CMD_UPSIB:	/* Browse to older sib */
			if (!(node = indi_to_prev_sib(indi)))
				message(noosib);
			else
				 indi = node;
			break;
		case CMD_DOWNSIB:	/* Browse to younger sib */
			if (!(node = indi_to_next_sib(indi)))
				message(noysib);
			else 
				 indi = node;
			break;
		case CMD_PARENTS:	/* Browse to parents' family */
			if ((*pfam1 = choose_family(indi, noprnt,
				idfbrs, FALSE)))
				return BROWSE_FAM;
			break;
		case CMD_TANDEM_PARENTS:	/* tandem browse to two parents families*/
			if ((*pfam1 = choose_family(indi, noprnt,
				id1fbr, FALSE)))
			  if ((*pfam2 = choose_family(indi, noprnt,
				id2fbr, FALSE)))
				return BROWSE_2FAM;
			break;
		case CMD_BROWSE: 	/* Browse new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				indi = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				seq=NULL;
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case CMD_NEWPERSON:	/* Add new person */
			if (!(node = nztop(add_indi_by_edit()))) break;
			save = indi;
			indi = node;
			break;
		case CMD_NEWFAMILY:	/* Add family for current person */
			node = pick_create_new_family(indi, save, addstrings);
			save = NULL;
			if (!node) break;
			*pfam1 = node;
			return BROWSE_FAM;
		case CMD_TANDEM:	/* Switch to tandem browsing */
			node = ask_for_indi(idp2br, NOCONFIRM, NOASK1);
			if (node) {
				*pindi1 = indi;
				*pindi2 = node;
				return BROWSE_TAND;
			}
			break;
		case CMD_SWAPFAMILIES: 	/* Swap families of current person */
			swap_families(indi);
			break;
		case CMD_ADDASSPOUSE:	/* Add person as spouse */
			add_spouse(indi, NULL, TRUE);
			break;
		case CMD_ADDASCHILD:    /* Add person as child */
			add_child(indi, NULL);
			break;
		case CMD_PERSON:   /* switch to person browse */
			indimode='i';
			break;
		case CMD_REMOVEASSPOUSE:	/* Remove person as spouse */
			choose_and_remove_spouse(indi, NULL, FALSE);
			break;
		case CMD_REMOVEASCHILD:	/* Remove person as child */
			choose_and_remove_child(indi, NULL, FALSE);
			break;
		case CMD_ADVANCED:	/* Advanced person edit */
			advanced_person_edit(indi);
			break;
		case CMD_JUMP_HOOK:   /* GUI direct navigation */
			indi = jumpnode;
			break;
		case CMD_NEXT:	/* Go to next indi in db */
			{
				i = xref_nexti(nkeyp);
				if (i)
					indi = keynum_to_indi(i);
				else message(nopers);
				break;
			}
		case CMD_PREV:	/* Go to prev indi in db */
			{
				i = xref_previ(nkeyp);
				if (i)
					indi = keynum_to_indi(i);
				else message(nopers);
				break;
			}
		case CMD_SOURCES:	/* Browse to sources */
			node = choose_source(indi, nosour, idsour);
			if (node) {
				*pindi1 = node;
				return BROWSE_AUX;
			}
			break;
		case CMD_NOTES:	/* Browse to notes */
			node = choose_note(indi, nonote, idnote);
			if (node) {
				*pindi1 = node;
				return BROWSE_AUX;
			}
			break;
		case CMD_POINTERS:	/* Browse to references */
			node = choose_pointer(indi, noptr, idptr);
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			break;
		case CMD_HISTORY_BACK:
			node = history_back();
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			message(nohist);
			break;
		case CMD_HISTORY_FWD:
			node = history_fwd();
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			message(nohist);
			break;
		case CMD_QUIT:
		default:
			return BROWSE_QUIT;
		}
	}
}
/*==========================================
 * display_aux -- Show aux node in current mode
 * Created: 2001/01/27, Perry Rapp
 *========================================*/
static INT
display_aux (NODE node, INT mode)
{
	CACHEEL cel;
	INT c;
	cel = node_to_cacheel(node);
	lock_cache(cel);
	c = aux_browse(node, mode);
	unlock_cache(cel);
	return c;
}
/*====================================================
 * browse_aux -- Handle aux node browse.
 * Created: 2001/01/27, Perry Rapp
 *==================================================*/
static INT
browse_aux (NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq)
{
	NODE node = *pindi1;
	STRING key = rmvat(nxref(node));
	char ntype = key[0];
	INT i, c;
	INT nkeyp, auxmode, auxmodep;
	NODE node2;
	auxmode = 'x';

	if (!node) return BROWSE_QUIT;
	show_reset_scroll();
	nkeyp = 0;
	auxmodep = auxmode;

	while (TRUE) {
		if (node_to_keynum(ntype, node) != nkeyp 
			|| auxmode != auxmodep) {
			show_reset_scroll();
		}
		history_record(node);
		c = display_aux(node, auxmode);
		/* last keynum & mode, so can tell if changed */
		nkeyp = node_to_keynum(ntype, node);
		auxmodep = auxmode;
		if (!handle_menu_cmds(c)
			&& !handle_scroll_cmds(c)
			&& !handle_aux_mode_cmds(c, &auxmode))
			switch (c)
		{
		case CMD_EDIT:
			switch(ntype) {
			case 'S': edit_source(node); break;
			case 'E': edit_event(node); break;
			case 'X': edit_other(node); break;
			}
			break;
		case CMD_NOTES:	/* Browse to notes */
			node2 = choose_note(node, nonote, idnote);
			if (node2)
				node = node2;
			break;
		case CMD_POINTERS:	/* Browse to references */
			node2 = choose_pointer(node, noptr, idptr);
			if (node2)
				node = node2;
			break;
		case CMD_NEXT:	/* Go to next in db */
			{
				i = xref_next(ntype, nkeyp);
				if (i)
					node = keynum_to_node(ntype, i);
				else message(norec);
				break;
			}
		case CMD_PREV:	/* Go to prev in db */
			{
				i = xref_prev(ntype, nkeyp);
				if (i)
					node = keynum_to_node(ntype, i);
				else message(norec);
				break;
			}
		case CMD_HISTORY_BACK:
			node = history_back();
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			message(nohist);
			break;
		case CMD_HISTORY_FWD:
			node = history_fwd();
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			message(nohist);
			break;
		case CMD_QUIT:
		default:
			return BROWSE_QUIT;
		}
	}
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
	return browse_indi_modes(pindi1, pindi2, pfam1, pfam2, pseq, 'n');
}
/*===============================================
 * pick_remove_spouse_from_family -- 
 *  pulled out of browse_fam, 2001/02/03, Perry Rapp
 *=============================================*/
static void
pick_remove_spouse_from_family (NODE fam)
{
	NODE fref, husb, wife, chil, rest;
	NODE root, node, spnodes[MAX_SPOUSES];
	STRING spstrings[MAX_SPOUSES];
	INT i;
	if (readonly) {
		message(ronlye);
		return;
	}
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	if (!husb && !wife) {
		message(hasnei);
		return;
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
		if (i == MAX_SPOUSES) {
			message(spover);
			break;
		}
	}
	join_fam(fam, fref, husb, wife, chil, rest);
	i = choose_from_list(idsrmv, i, spstrings);
	if (i == -1) return;
	choose_and_remove_spouse(spnodes[i], fam, TRUE);
}
/*===============================================
 * pick_add_spouse_to_family -- 
 *  pulled out of browse_fam, 2001/02/03, Perry Rapp
 *=============================================*/
static void
pick_add_spouse_to_family (NODE fam, NODE save)
{
	NODE fref, husb, wife, chil, rest;
	char scratch[100];
	TRANTABLE ttd = tran_tables[MINDS];
	if (readonly) {
		message(ronlye);
		return;
	}
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	join_fam(fam, fref, husb, wife, chil, rest);
#if 0
	if (husb && wife) {
		message(hasbth);
		return;
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
			return;
		}
	}
	add_spouse(NULL, fam, TRUE);
}
/*===============================================
 * pick_add_child_to_fam -- 
 *  pulled out of browse_fam, 2001/02/03, Perry Rapp
 *=============================================*/
static void
pick_add_child_to_fam (NODE fam, NODE save)
{
	char scratch[100];
	TRANTABLE ttd = tran_tables[MINDS];
	if (readonly) {
		message(ronlye);
		return;
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
			return;
		}
	}
	add_child(NULL, fam);
}
/*===========================================
 * display_fam -- Show family in current mode
 *=========================================*/
static INT
display_fam (NODE fam, INT mode)
{
	CACHEEL icel;
	INT c=0;
	icel = fam_to_cacheel(fam);
	lock_cache(icel);
	c = fam_browse(fam, mode);
	unlock_cache(icel);
	return c;
}
/*==========================================
 * display_2fam -- Show two fam in current mode
 *========================================*/
INT
display_2fam (NODE fam1, NODE fam2, INT mode)
{
	CACHEEL icel1, icel2;
	INT c;
	icel1 = fam_to_cacheel(fam1);
	icel2 = fam_to_cacheel(fam2);
	lock_cache(icel1);
	lock_cache(icel2);
	c = twofam_browse(fam1, fam2, mode);
	unlock_cache(icel1);
	unlock_cache(icel2);
	return c;
}
/*===============================================
 * browse_fam -- Handle family browse selections.
 *=============================================*/
static INT
browse_fam (NODE *pindi1,
            NODE *pindi2,
            NODE *pfam1,
            NODE *pfam2,
            INDISEQ *pseq)
{
	INT i, c, rc;
	static INT fammode='n';
	INT nkeyp, fammodep;
	NODE save = NULL, fam = *pfam1, node;
	INDISEQ seq;
	STRING key, name;
	char scratch[100];
	TRANTABLE ttd = tran_tables[MINDS];

	if (!fam) return BROWSE_QUIT;
	show_reset_scroll();
	nkeyp = 0;
	fammodep = fammode;

	while (TRUE) {
		if (fam_to_keynum(fam) != nkeyp
			|| fammode != fammodep) {
			show_reset_scroll();
		}
		history_record(fam);
		c = display_fam(fam, fammode);
		/* last keynum & mode, so can tell if changed */
		nkeyp = fam_to_keynum(fam);
		fammodep = fammode;
		if (c != CMD_ADDCHILD && c != CMD_ADDSPOUSE)
			save = NULL;
		if (!handle_menu_cmds(c)
			&& !handle_scroll_cmds(c)
			&& !handle_fam_mode_cmds(c, &fammode))
			switch (c) 
		{
		case CMD_ADVANCED:	/* Advanced family edit */
			advanced_family_edit(fam);
			break;
		case CMD_BROWSE_FAM:
			i = ask_for_int(idfamk);
			if(i > 0) {
				sprintf(scratch, "F%d", i);
				if((node = key_to_fam(scratch))) {
					fam = node;
				}
			}
			break;
		case CMD_EDIT:	/* Edit family's record */
			node = edit_family(fam);
			if (node)
				fam = node;
			break;
		case CMD_FATHER:	/* Browse to family's father */
			*pindi1 = choose_father(NULL, fam, nohusb,
			    idhbrs, NOASK1);
			if (*pindi1) return BROWSE_INDI;
			break;
		case CMD_TANDEM_FATHERS:	/* Tandem Browse to family's fathers */
			*pindi1 = choose_father(NULL, fam, nohusb,
			    id1hbr, NOASK1);
			if (*pindi1) {
			  *pindi2 = choose_father(NULL, fam, nohusb,
			    id2hbr, NOASK1);
			  if (*pindi2) 
				return BROWSE_TAND;
			}
			break;
		case CMD_MOTHER:	/* Browse to family's mother */
			*pindi1 = choose_mother(NULL, fam, nowife,
			    idwbrs, NOASK1);
			if (*pindi1) return BROWSE_INDI;
			break;
		case CMD_TANDEM_MOTHERS:	/* Tandem Browse to family's mother */
			*pindi1 = choose_mother(NULL, fam, nowife,
			    id1wbr, NOASK1);
			if (*pindi1) {
				*pindi2 = choose_mother(NULL, fam, nowife, 
					id2wbr, NOASK1);
				if (*pindi2) 
					return BROWSE_TAND;
			}
			break;
		case CMD_CHILDREN:	/* Browse to a child */
			*pindi1 = choose_child(NULL, fam, nocinf,
				idcbrs, NOASK1);
			if (*pindi1) return BROWSE_INDI;
			break;
		case CMD_TANDEM_CHILDREN:	/* browse to tandem children */
			*pindi1 = choose_child(NULL, fam, nocinf,
			    id1cbr, NOASK1);
			if (*pindi1) {
				*pindi2 = choose_child(NULL, fam, nocinf,
					id2cbr, NOASK1);
				if (*pindi2) 
					return BROWSE_TAND;
			}
			break;
		case CMD_REMOVECHILD:	/* Remove a child */
			if (readonly) {
				message(ronlye);
				break;
			}
			*pindi1 = choose_child(NULL, fam, nocinf,
			    idcrmv, DOASK1);
			if (*pindi1) choose_and_remove_child(*pindi1, fam, TRUE);
			break;
		case CMD_ADDSPOUSE:	/* Add spouse to family */
			pick_add_spouse_to_family(fam, save);
			save = NULL;
			break;
		case CMD_REMOVESPOUSE:	/* Remove spouse from family */
			pick_remove_spouse_from_family(fam);
			break;
		case CMD_NEWPERSON:	/* Add person to database */
			save = nztop(add_indi_by_edit());
			break;
		case CMD_ADDCHILD:	/* Add child to family */
			pick_add_child_to_fam(fam, save);
			save = NULL;
			break;
		case CMD_BROWSE: 	/* Browse to new list of persons */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				*pindi1 = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				seq=NULL;
				return BROWSE_INDI;
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case CMD_BROWSE_ZIP:	/* Zip browse to new person */
			*pindi1 = ask_for_indi(idpnxt, NOCONFIRM, NOASK1);
			if (*pindi1) return BROWSE_INDI;
			break;
		case CMD_TANDEM:	/* Enter family tandem mode */
			node = ask_for_fam(ids2fm, idc2fm);
			if (node) {
				*pfam1 = fam;
				*pfam2 = node;
				return BROWSE_2FAM;
			}
			break;
		case CMD_SWAPCHILDREN:	/* Swap two children */
			swap_children(NULL, fam);
			break;
		case CMD_TOGGLE_CHILDNUMS:       /* toggle children numbers */
			show_childnumbers();
			break;
		case CMD_CHILD_DIRECT0+1:	/* Go to children by number */
		case CMD_CHILD_DIRECT0+2:
		case CMD_CHILD_DIRECT0+3:
		case CMD_CHILD_DIRECT0+4:
		case CMD_CHILD_DIRECT0+5:
		case CMD_CHILD_DIRECT0+6:
		case CMD_CHILD_DIRECT0+7:
		case CMD_CHILD_DIRECT0+8:
		case CMD_CHILD_DIRECT0+9:
			*pindi1 = goto_fam_child(fam, c-CMD_CHILD_DIRECT0);
			if (*pindi1) return BROWSE_INDI;
			message(nochil);
			break;
		case CMD_NEXT:	/* Go to next fam in db */
			{
				i = xref_nextf(nkeyp);
				if (i)
					fam = keynum_to_fam(i);
				else
					message(nofam);
				break;
			}
		case CMD_PREV:	/* Go to prev fam in db */
			{
				i = xref_prevf(nkeyp);
				if (i)
					fam = keynum_to_fam(i);
				else
					message(nofam);
				break;
			}
		case CMD_SOURCES:	/* Browse to sources */
			node = choose_source(fam, nosour, idsour);
			if (node) {
				*pindi1 = node;
				return BROWSE_AUX;
			}
			break;
		case CMD_HISTORY_BACK:
			node = history_back();
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			message(nohist);
			break;
		case CMD_HISTORY_FWD:
			node = history_fwd();
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			message(nohist);
			break;
		case CMD_QUIT:
		default:
			return BROWSE_QUIT;
		}
	}
}
/*======================================================
 * handle_menu_cmds -- Handle menuing commands
 * Created: 2001/01/31, Perry Rapp
 *====================================================*/
BOOLEAN
handle_menu_cmds (INT c)
{
	switch(c) {
		case CMD_MENU_GROW: adjust_menu_height(+1); return TRUE;
		case CMD_MENU_SHRINK: adjust_menu_height(-1); return TRUE;
		case CMD_MENU_MORE: cycle_menu(); return TRUE;
		case CMD_MENU_TOGGLE: toggle_menu(); return TRUE;
	}
	return FALSE;
}
/*======================================================
 * handle_scroll_cmds -- Handle detail scrolling
 * Created: 2001/02/01, Perry Rapp
 *====================================================*/
BOOLEAN
handle_scroll_cmds (INT c)
{
	switch(c) {
		case CMD_SCROLL_UP: show_scroll(-1); return TRUE;
		case CMD_SCROLL_DOWN: show_scroll(+1); return TRUE;
	}
	return FALSE;
}
/*======================================================
 * handle_indi_mode_cmds -- Handle indi modes
 * Created: 2001/02/04, Perry Rapp
 *====================================================*/
BOOLEAN
handle_indi_mode_cmds (INT c, INT * mode)
{
	switch(c) {
		case CMD_MODE_GEDCOM: *mode = 'g'; return TRUE;
		case CMD_MODE_GEDCOMX: *mode = 'x'; return TRUE;
		case CMD_MODE_PEDIGREE:
			*mode = (*mode=='a')?'d':'a';
			return TRUE;
		case CMD_MODE_ANCESTORS: *mode = 'a'; return TRUE;
		case CMD_MODE_DESCENDANTS: *mode = 'd'; return TRUE;
		case CMD_MODE_NORMAL: *mode = 'n'; return TRUE;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'n': *mode = 'a'; break;
			case 'a': *mode = 'd'; break;
			case 'd': *mode = 'g'; break;
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 'n'; break;
			}
			return TRUE;
	}
	return FALSE;
}
/*======================================================
 * handle_fam_mode_cmds -- Handle indi modes
 * Created: 2001/02/04, Perry Rapp
 *====================================================*/
BOOLEAN
handle_fam_mode_cmds (INT c, INT * mode)
{
	switch(c) {
		case CMD_MODE_GEDCOM: *mode = 'g'; return TRUE;
		case CMD_MODE_GEDCOMX: *mode = 'x'; return TRUE;
		case CMD_MODE_NORMAL: *mode = 'n'; return TRUE;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'n': *mode = 'g'; break;
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 'n'; break;
			}
			return TRUE;
	}
	return FALSE;
}
/*======================================================
 * handle_aux_mode_cmds -- Handle aux modes
 * Created: 2001/02/11, Perry Rapp
 *====================================================*/
BOOLEAN
handle_aux_mode_cmds (INT c, INT * mode)
{
	switch(c) {
		case CMD_MODE_GEDCOM: *mode = 'g'; return TRUE;
		case CMD_MODE_GEDCOMX: *mode = 'x'; return TRUE;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 'g'; break;
			}
			return TRUE;
	}
	return FALSE;
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
	return browse_indi_modes(pindi, pdum1, pfam, pdum2, pseq, 'p');
}
/*==================================================
 * choose_any_source -- choose from list of all sources
 *================================================*/
NOD0
choose_any_source (void)
{
	INDISEQ seq;
	NOD0 nod0;
	seq = get_all_sour();
	if (!seq)
	{
		message(nosour);
		return 0;
	}
	nod0 = choose_from_indiseq(seq, DOASK1, idsour, idsour);
	remove_indiseq(seq, FALSE);
	return nod0;
}
/*==================================================
 * choose_any_event -- choose from list of all events
 *================================================*/
NOD0
choose_any_event (void)
{
	INDISEQ seq;
	NOD0 nod0;
	seq = get_all_even();
	if (!seq)
	{
		message(noeven);
		return NULL;
	}
	nod0 = choose_from_indiseq(seq, DOASK1, ideven, ideven);
	remove_indiseq(seq, FALSE);
	return nod0;
}
/*==================================================
 * choose_any_other -- choose from list of all others
 *================================================*/
NOD0
choose_any_other (void)
{
	INDISEQ seq;
	NOD0 nod0;
	seq = get_all_othe();
	if (!seq)
	{
		message(noothe);
		return NULL;
	}
	nod0 = choose_from_indiseq(seq, DOASK1, idothe, idothe);
	remove_indiseq(seq, FALSE);
	return nod0;
}
/*==================================================
 * history_record -- add node to history if different from top of history
 *================================================*/
static void
history_record (NODE node)
{
	NKEY nkey = nkey_zero();
	INT last;
	if (hist_start==-1) {
		hist_start=hist_past_end;
		node_to_nkey(node, &hist_list[hist_start]);
		hist_past_end = (hist_start+1) % ARRSIZE(hist_list);
		/* hist_list[1+] are all zero because of static data initialization */
		return;
	}
	/*
	copy new node into nkey variable so we can check
	if this is the same as our most recent (hist_list[last])
	*/
	node_to_nkey(node, &nkey);
	last = (hist_past_end-1) % ARRSIZE(hist_list);
	if (nkey_eq(&nkey, &hist_list[last]))
		return;
	/* it is a new one so add it to circular list */
	nkey_copy(&nkey, &hist_list[hist_past_end]);
	if (hist_start == hist_past_end) {
		/* full buffer & we just overwrote the oldest */
		hist_start = (hist_start+1) % ARRSIZE(hist_list);
	}
	/* advance pointer to account for what we just added */
	hist_past_end = (hist_past_end+1) % ARRSIZE(hist_list);
}
/*==================================================
 * history_back -- return prev NODE in history, if exists
 *================================================*/
static NODE
history_back (void)
{
	INT last;
	NODE node;
	if (hist_start==-1)
		return NULL;
	/* back up from hist_past_end to current item */
	last = (hist_past_end-1) % ARRSIZE(hist_list);
	while (last != hist_start) {
		/* loop is to keep going over deleted ones */
		/* now back up before current item */
		last = (last-1) % ARRSIZE(hist_list);
		nkey_to_node(&hist_list[last], &node);
		if (node) {
			hist_past_end = (last+1) % ARRSIZE(hist_list);
			return node;
		}
	}
	return NULL;
}
/*==================================================
 * history_fwd -- return later NODE in history, if exists
 *================================================*/
static NODE
history_fwd (void)
{
	INT next;
	NODE node;
	if (hist_past_end == hist_start)
		return NULL; /* at end of full history */
	next = hist_past_end;
	nkey_to_node(&hist_list[next], &node);
	return node;
}
