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
#include "lloptions.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
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
extern STRING idpnxt, idnxt;
extern STRING ids2fm, idc2fm, idplst, idp2br, crtcfm, crtsfm;
extern STRING ronlye, ronlya, idhbrs, idwbrs;
extern STRING id1sbr, id2sbr, id1fbr, id2fbr, id1cbr, id2cbr;
extern STRING id1hbr, id2hbr, id1wbr, id2wbr;
extern STRING spover, idfamk, nohist, idhist, histclr;
extern STRING tag2long2cnc,newrecis,autoxref,editcur,gotonew,staycur;
extern STRING badhistcnt,badhistcnt2,badhistlen;

/*********************************************
 * local enums & defines
 *********************************************/

#define ALLPARMS &indi1, &indi2, &fam1, &fam2, &seq
#define MAX_SPOUSES 30
struct hist;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static NODE add_new_rec_maybe_ref(NODE node, char ntype);
static void ask_clear_history(struct hist * histp);
static void autoadd_xref(NODE node, NODE newnode);
static INT browse_aux(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT browse_indi(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT browse_fam(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT browse_indi_modes(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq, INT indimode);
static INT browse_pedigree(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static INT display_aux(NODE node, INT mode, BOOLEAN reuse);
static INT get_hist_count(struct hist * histp);
static NODE goto_fam_child(NODE fam, int childno);
static NODE goto_indi_child(NODE indi, int childno);
static BOOLEAN handle_aux_mode_cmds(INT c, INT * mode);
static INT handle_history_cmds(INT c, NODE * pindi1);
static NODE history_back(struct hist * histp);
static NODE history_list(struct hist * histp);
static void history_record(NODE node, struct hist * histp);
static NODE history_fwd(struct hist * histp);
static void init_hist(struct hist * histp, INT count);
static void load_hist_lists(void);
static void load_nkey_list(STRING key, struct hist * histp);
static void pick_add_child_to_fam(NODE fam, NODE save);
static void pick_add_spouse_to_family(NODE fam, NODE save);
static NODE pick_create_new_family(NODE indi, NODE save, STRING * addstrings);
static void pick_remove_spouse_from_family(NODE fam);
static void save_hist_lists(void);
static void save_nkey_list(STRING key, struct hist * histp);

/*********************************************
 * local variables
 *********************************************/

/*
	A history structure is a circular buffer holding
	a list. start is the earliest entry, and the 
	past_end points just beyond the latest entry.
	If start==past_end the the buffer is full.
	(If start==-1, then there are no entries.)
	list is a dynamically allocated array, with #entries==size.
	-1 <= start < size
	0 <= past_end < size
	(NB: CMD_HISTORY_FWD will go ahead to unused entries.)
*/
struct hist {
	INT start;
	INT past_end;
	INT size;
	NKEY * list;
};
static struct hist vhist;


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
	RECORD rec;
	INT len, rc;
	STRING key, name;

	if (*code == BROWSE_INDI) {
		*seq = ask_for_indiseq(idplst, 'I', &rc);
		if (!*seq) return;
		if ((len = length_indiseq(*seq)) < 1) return;
		if (len == 1) {
			element_indiseq(*seq, 0, &key, &name);
			*node = key_to_indi(key);
			remove_indiseq(*seq);
			*seq = NULL;
		} else {
			*code = BROWSE_LIST;
		}
		return;
	}
	if (*code == BROWSE_EVEN) {
		rec = choose_any_event();
		*node = nztop(rec);
		return;
	}
	if (*code == BROWSE_SOUR) {
		rec = choose_any_source();
		*node = nztop(rec);
		return;
	}
	rec = choose_any_other();
	*node = nztop(rec);
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
			case 'F': code=BROWSE_FAM; fam1 = indi1; break;
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
	i = choose_from_array(idfcop, 2, addstrings);
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
	BOOLEAN reuse=FALSE; /* flag to reuse same display strings */
	INT nkeyp, indimodep;
	NODE node, save = NULL, indi = *pindi1;
	NODE node2;
	INDISEQ seq = NULL;
	char c2;

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
		history_record(indi, &vhist);
			/* display & get input, preserving INDI in cache */
		display_indi(indi, indimode, reuse);
		c = interact_indi();
		/* last keynum & mode, so can tell if changed */
		nkeyp = indi_to_keynum(indi);
		indimodep = indimode;
reprocess_indi_cmd: /* so one command can forward to another */
		reuse = FALSE; /* don't reuse display unless specifically set */
		if (c != CMD_NEWFAMILY) save = NULL;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_scroll_cmds(c, &reuse))
			continue;
		if (handle_indi_mode_cmds(c, &indimode))
			continue;
		i = handle_history_cmds(c, pindi1);
		if (i == 1)
			continue; /* history cmd handled, stay here */
		if (i == -1)
			return BROWSE_UNK; /* history cmd handled, leave page */
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
		case CMD_BROWSE_ZIP_INDI:	/* Zip browse another person */
			node = ask_for_indi_old(idpnxt, NOCONFIRM, NOASK1);
			if (node) indi = node;
			break;
		case CMD_BROWSE_ZIP_ANY:	/* Zip browse any record */
			*pindi1 = ask_for_any_old(idnxt, NOCONFIRM, NOASK1);
			if (*pindi1) return BROWSE_UNK;
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
			seq = ask_for_indiseq(idplst, 'I', &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				indi = key_to_indi(key);
				remove_indiseq(seq);
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
		case CMD_ADD_SOUR: /* add source */
		case CMD_ADD_EVEN: /* add event */
		case CMD_ADD_OTHR: /* add other */
			c2 = (c==CMD_ADD_SOUR ? 'S' : (c==CMD_ADD_EVEN ? 'E' : 'X'));
			node = add_new_rec_maybe_ref(indi, c2);
			if (node == indi) {
				c = CMD_EDIT;
				goto reprocess_indi_cmd; /* forward to edit */
			}
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			break;
		case CMD_TANDEM:	/* Switch to tandem browsing */
			node = ask_for_indi_old(idp2br, NOCONFIRM, NOASK1);
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
display_aux (NODE node, INT mode, BOOLEAN reuse)
{
	CACHEEL cel;
	INT c;
	cel = node_to_cacheel(node);
	lock_cache(cel);
	c = aux_browse(node, mode, reuse);
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
	NODE aux = *pindi1;
	STRING key = rmvat(nxref(aux));
	char ntype = key[0];
	INT i, c;
	BOOLEAN reuse=FALSE; /* flag to reuse same display strings */
	INT nkeyp, auxmode, auxmodep;
	NODE node;
	char c2;
	pindi2=pindi2; /* unused */
	pfam1=pfam1; /* unused */
	pfam2=pfam2; /* unused */
	pseq=pseq; /* unused */

	auxmode = 'x';

	if (!aux) return BROWSE_QUIT;
	show_reset_scroll();
	nkeyp = 0;
	auxmodep = auxmode;

	while (TRUE) {
		if (node_to_keynum(ntype, aux) != nkeyp 
			|| auxmode != auxmodep) {
			show_reset_scroll();
		}
		history_record(aux, &vhist);
		c = display_aux(aux, auxmode, reuse);
		/* last keynum & mode, so can tell if changed */
		nkeyp = node_to_keynum(ntype, aux);
		auxmodep = auxmode;
reprocess_aux_cmd:
		reuse = FALSE; /* don't reuse display unless specifically set */
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_scroll_cmds(c, &reuse))
			continue;
		if (handle_aux_mode_cmds(c, &auxmode))
			continue;
		i = handle_history_cmds(c, pindi1);
		if (i == 1)
			continue; /* history cmd handled, stay here */
		if (i == -1)
			return BROWSE_UNK; /* history cmd handled, leave page */
		switch (c)
		{
		case CMD_EDIT:
			switch(ntype) {
			case 'S': edit_source(aux); break;
			case 'E': edit_event(aux); break;
			case 'X': edit_other(aux); break;
			}
			break;
		case CMD_ADD_SOUR: /* add source */
		case CMD_ADD_EVEN: /* add event */
		case CMD_ADD_OTHR: /* add other */
			c2 = (c==CMD_ADD_SOUR ? 'S' : (c==CMD_ADD_EVEN ? 'E' : 'X'));
			node = add_new_rec_maybe_ref(aux, c2);
			if (node == aux) {
				c = CMD_EDIT;
				goto reprocess_aux_cmd; /* forward to edit */
			}
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			break;
		case CMD_BROWSE_ZIP_INDI:	/* Zip browse to new person */
			*pindi1 = ask_for_indi_old(idpnxt, NOCONFIRM, NOASK1);
			if (*pindi1) return BROWSE_INDI;
			break;
		case CMD_BROWSE_ZIP_ANY:	/* Zip browse any record */
			*pindi1 = ask_for_any_old(idnxt, NOCONFIRM, NOASK1);
			if (*pindi1) return BROWSE_UNK;
			break;
		case CMD_NOTES:	/* Browse to notes */
			node = choose_note(aux, nonote, idnote);
			if (node)
				aux = node;
			break;
		case CMD_POINTERS:	/* Browse to references */
			node = choose_pointer(aux, noptr, idptr);
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			break;
		case CMD_NEXT:	/* Go to next in db */
			{
				i = xref_next(ntype, nkeyp);
				if (i)
					aux = keynum_to_node(ntype, i);
				else message(norec);
				break;
			}
		case CMD_PREV:	/* Go to prev in db */
			{
				i = xref_prev(ntype, nkeyp);
				if (i)
					aux = keynum_to_node(ntype, i);
				else message(norec);
				break;
			}
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
 *  construct list of spouses, prompt for choice, & remove
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
			 NULL, 66, &disp_shrt_rfmt);
		spnodes[i++] = root;
	}
	for (node = wife; node; node = nsibling(node)) {
		root = key_to_indi(rmvat(nval(node)));
		spstrings[i] = indi_to_list_string(root,
			 NULL, 66, &disp_shrt_rfmt);
		spnodes[i++] = root;
		if (i == MAX_SPOUSES) {
			message(spover);
			break;
		}
	}
	join_fam(fam, fref, husb, wife, chil, rest);
	i = choose_from_array(idsrmv, i, spstrings);
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
/*===============================================
 * browse_fam -- Handle family browse selections.
 *=============================================*/
static INT
browse_fam (NODE *pindi1, NODE *pindi2, NODE *pfam1, NODE *pfam2
	, INDISEQ *pseq)
{
	INT i, c, rc;
	BOOLEAN reuse=FALSE; /* flag to reuse same display strings */
	static INT fammode='n';
	INT nkeyp, fammodep;
	NODE save = NULL, fam = *pfam1, node;
	INDISEQ seq;
	STRING key, name;
	char c2;

	if (!fam) return BROWSE_QUIT;
	show_reset_scroll();
	nkeyp = 0;
	fammodep = fammode;

	while (TRUE) {
		if (fam_to_keynum(fam) != nkeyp
			|| fammode != fammodep) {
			show_reset_scroll();
		}
		history_record(fam, &vhist);
		display_fam(fam, fammode, reuse);
		c = interact_fam();
		/* last keynum & mode, so can tell if changed */
		nkeyp = fam_to_keynum(fam);
		fammodep = fammode;
reprocess_fam_cmd: /* so one command can forward to another */
		reuse = FALSE; /* don't reuse display unless specifically set */
		if (c != CMD_ADDCHILD && c != CMD_ADDSPOUSE)
			save = NULL;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_scroll_cmds(c, &reuse))
			continue;
		if (handle_fam_mode_cmds(c, &fammode))
			continue;
		i = handle_history_cmds(c, pindi1);
		if (i == 1)
			continue; /* history cmd handled, stay here */
		if (i == -1)
			return BROWSE_UNK; /* history cmd handled, leave page */
		switch (c) 
		{
		case CMD_ADVANCED:	/* Advanced family edit */
			advanced_family_edit(fam);
			break;
		case CMD_BROWSE_FAM:
			i = ask_for_int(idfamk);
			if(i > 0) {
				if((node = qkeynum_to_fam(i))) {
					fam = node;
				} else {
					STRING unknfam = _("No such family.");
					message(unknfam);
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
		case CMD_ADD_SOUR: /* add source */
		case CMD_ADD_EVEN: /* add event */
		case CMD_ADD_OTHR: /* add other */
			c2 = (c==CMD_ADD_SOUR ? 'S' : (c==CMD_ADD_EVEN ? 'E' : 'X'));
			node = add_new_rec_maybe_ref(fam, c2);
			if (node == fam) {
				c = CMD_EDIT;
				goto reprocess_fam_cmd; /* forward to edit */
			}
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			break;
		case CMD_ADDCHILD:	/* Add child to family */
			pick_add_child_to_fam(fam, save);
			save = NULL;
			break;
		case CMD_BROWSE: 	/* Browse to new list of persons */
			seq = ask_for_indiseq(idplst, 'I', &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				*pindi1 = key_to_indi(key);
				remove_indiseq(seq);
				seq=NULL;
				return BROWSE_INDI;
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case CMD_BROWSE_ZIP_INDI:	/* Zip browse to new person */
			*pindi1 = ask_for_indi_old(idpnxt, NOCONFIRM, NOASK1);
			if (*pindi1) return BROWSE_INDI;
			break;
		case CMD_BROWSE_ZIP_ANY:	/* Zip browse any record */
			*pindi1 = ask_for_any_old(idnxt, NOCONFIRM, NOASK1);
			if (*pindi1) return BROWSE_UNK;
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
		case CMD_REORDERCHILD: /* Move a child in order */
			reorder_child(NULL, fam);
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
		case CMD_NOTES:	/* Browse to notes */
			node = choose_note(fam, nonote, idnote);
			if (node) {
				*pindi1 = node;
				return BROWSE_AUX;
			}
			break;
		case CMD_POINTERS:	/* Browse to references */
			node = choose_pointer(fam, noptr, idptr);
			if (node) {
				*pindi1 = node;
				return BROWSE_UNK;
			}
			break;
		case CMD_QUIT:
		default:
			return BROWSE_QUIT;
		}
	}
}
/*======================================================
 * handle_menu_cmds -- Handle menuing commands
 * That is, handle commands that are internal to the menuing 
 * system (eg, paging, changing current menu size).
 * Created: 2001/01/31, Perry Rapp
 *====================================================*/
BOOLEAN
handle_menu_cmds (INT c, BOOLEAN * reuse)
{
	BOOLEAN old = *reuse;
	/* if a menu command, then we CAN reuse the previous display strings */
	*reuse = TRUE;
	switch(c) {
		case CMD_MENU_GROW: adjust_menu_height(+1); return TRUE;
		case CMD_MENU_SHRINK: adjust_menu_height(-1); return TRUE;
		case CMD_MENU_MORECOLS: adjust_menu_cols(+1); return TRUE;
		case CMD_MENU_LESSCOLS: adjust_menu_cols(-1); return TRUE;
		case CMD_MENU_MORE: cycle_menu(); return TRUE;
		case CMD_MENU_TOGGLE: toggle_menu(); return TRUE;
	}
	*reuse = old;
	return FALSE;
}
/*======================================================
 * handle_scroll_cmds -- Handle detail scrolling
 * Created: 2001/02/01, Perry Rapp
 *====================================================*/
BOOLEAN
handle_scroll_cmds (INT c, BOOLEAN * reuse)
{
	BOOLEAN old = *reuse;
	/* if a menu command, then we CAN reuse the previous display strings */
	*reuse = TRUE;
	switch(c) {
		case CMD_SCROLL_UP: show_scroll(-1); return TRUE;
		case CMD_SCROLL_DOWN: show_scroll(+1); return TRUE;
	}
	*reuse = old;
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
		case CMD_MODE_GEDCOMT: *mode = 't'; return TRUE;
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
			case 'x': *mode = 't'; break;
			case 't': *mode = 'n'; break;
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
		case CMD_MODE_GEDCOMT: *mode = 't'; return TRUE;
		case CMD_MODE_NORMAL: *mode = 'n'; return TRUE;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'n': *mode = 'g'; break;
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 't'; break;
			case 't': *mode = 'n'; break;
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
		case CMD_MODE_GEDCOMT: *mode = 't'; return TRUE;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 't'; break;
			case 't': *mode = 'g'; break;
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
RECORD
choose_any_source (void)
{
	INDISEQ seq;
	RECORD rec;
	seq = get_all_sour();
	if (!seq)
	{
		message(nosour);
		return 0;
	}
	rec = choose_from_indiseq(seq, DOASK1, idsour, idsour);
	remove_indiseq(seq);
	return rec;
}
/*==================================================
 * choose_any_event -- choose from list of all events
 *================================================*/
RECORD
choose_any_event (void)
{
	INDISEQ seq;
	RECORD rec;
	seq = get_all_even();
	if (!seq)
	{
		message(noeven);
		return NULL;
	}
	rec = choose_from_indiseq(seq, DOASK1, ideven, ideven);
	remove_indiseq(seq);
	return rec;
}
/*==================================================
 * choose_any_other -- choose from list of all others
 *================================================*/
RECORD
choose_any_other (void)
{
	INDISEQ seq;
	RECORD rec;
	seq = get_all_othe();
	if (!seq)
	{
		message(noothe);
		return NULL;
	}
	rec = choose_from_indiseq(seq, DOASK1, idothe, idothe);
	remove_indiseq(seq);
	return rec;
}
/*==================================================
 * load_hist_lists -- Load previous history from database
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static void
load_hist_lists (void)
{
	/* V for visit history, planning to also have a change history */
	INT count = getoptint("HistorySize", 20);
	if (count<0 || count > 9999)
		count = 20;
	init_hist(&vhist, count);
	if (getoptint("SaveHistory", 0))
		load_nkey_list("VHIST", &vhist);
}
/*==================================================
 * save_hist_lists -- Save history into database
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static void
save_hist_lists (void)
{
	if (!getoptint("SaveHistory", 0)) return;
	save_nkey_list("VHIST", &vhist);
}
/*==================================================
 * init_hist -- create & initialize a history list
 * Created: 2001/12/23, Perry Rapp
 *=================================================*/
static void
init_hist (struct hist * histp, INT count)
{
	INT size = count * sizeof(histp->list[0]);
	memset(histp, 0, sizeof(*histp));
	histp->size = count;
	histp->list = (NKEY *)stdalloc(size);
	memset(histp->list, 0, size);
	histp->start = -1;
	histp->past_end = 0;
}
/*==================================================
 * load_nkey_list -- Load node list from record into NKEY array
 *  key:   [IN]  key used to store list in database
 *  histp: [IN]  history list to save
 * Fills in ndarray with data from database.
 * Created: 2001/12/23, Perry Rapp
 * TODO: should be moved to gedlib
 *================================================*/
static void
load_nkey_list (STRING key, struct hist * histp)
{
	STRING rawrec;
	INT * ptr;
	INT count, len, i, temp;

	count = 0;
	if (!(rawrec = retrieve_raw_record(key, &len)))
		return;
	if (len < 8 || (len % 8) != 0)
		return;
	ptr = (INT *)rawrec;
	temp = *ptr++;
	if (temp<1 || temp > 9999) {
		/* #records failed sanity check */
		msg_error(badhistcnt);
		goto end;
	}
	if (temp != *ptr++) {
		/* 2nd copy of #records failed to match */
		msg_error(badhistcnt2);
		goto end;
	}
	if (len != (temp+1)*8) {
		/* length should be 8 bytes per record + 8 byte header */
		msg_error(badhistlen);
	}
	count = temp;
	if (count > histp->size) count = histp->size;
	for (i=0,temp=0; i<count; ++i) {
		char key[12];
		char ntype = *ptr++;
		INT keynum = *ptr++;
		if (!ntype || !keynum)
			continue;
		histp->list[temp].ntype = ntype;
		histp->list[temp].keynum = keynum;
		/* We could sanity check these */
		snprintf(key, sizeof(key), "%c%d", ntype, keynum);
		histp->list[temp].key = strdup(key);
		++temp;
	}
	count = temp;
	if (count) {
		histp->start = 0;
		histp->past_end = count % histp->size;
	}

end:
	stdfree(rawrec);
}
/*==================================================
 * get_hist_count -- Calculate current size of history
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static INT
get_hist_count (struct hist * histp)
{
	if (!histp->size || histp->start==-1)
		return 0;
	else if (histp->past_end > histp->start)
		return histp->past_end - histp->start;
	else
		return histp->size - histp->start + histp->past_end;
}
/*==================================================
 * save_nkey_list -- Save nkey list from circular array
 *  key:     [IN]  key used to store list in database
 *  hist:    [IN]  history list to save
 * Created: 2001/12/23, Perry Rapp
 * TODO: should be moved to gedlib
 *================================================*/
static void
save_nkey_list (STRING key, struct hist * histp)
{
	FILE * fp=0;
	INT next, prev, count, temp;
	size_t rtn;

	count = get_hist_count(histp);

	unlink(editfile);

	fp = fopen(editfile, "w");
	if (!fp) return;

	/* write count first -- twice just to take up 8 bytes same as records */
	rtn = fwrite(&count, 4, 1, fp); ASSERT(rtn==1);
	rtn = fwrite(&count, 4, 1, fp); ASSERT(rtn==1);

	prev = -1;
	next = histp->start;
	while (1) {
		/* write type & number, 4 bytes each */
		temp = histp->list[next].ntype;
		rtn = fwrite(&temp, 4, 1, fp); ASSERT(rtn==1);
		temp = histp->list[next].keynum;
		rtn = fwrite(&temp, 4, 1, fp); ASSERT(rtn==1);
		prev = next;
		next = (next+1) % histp->size;
		if (next == histp->past_end)
			break; /* finished them all */
	}
	fclose(fp);

	store_text_file_to_db(key, editfile, 0);
}
/*==================================================
 * history_record -- add node to history if different from top of history
 * Created: 2001/03?, Perry Rapp
 *================================================*/
static void
history_record (NODE node, struct hist * histp)
{
	NKEY nkey = nkey_zero();
	INT prev, next, i;
	INT count = get_hist_count(histp);
	INT protect = getoptint("HistoryBounceSuppress", 0);
	if (!histp->size) return;
	if (histp->start==-1) {
		histp->start = histp->past_end;
		node_to_nkey(node, &histp->list[histp->start]);
		histp->past_end = (histp->start+1) % histp->size;
		return;
	}
	/*
	copy new node into nkey variable so we can check
	if this is the same as our most recent (histp->list[last])
	*/
	node_to_nkey(node, &nkey);
	if (protect<1 || protect>99)
		protect=1;
	if (protect>count)
		protect=count;
	/* traverse from most recent back (bounce suppression) */
	prev = -1;
	next = (histp->past_end-1);
	if (next < 0) next += histp->size;
	for (i=0; i<protect; ++i) {
		if (nkey_eq(&nkey, &histp->list[next]))
			return;
		prev = next;
		if (--next < 0) next += histp->size;
	}
	/* it is a new one so add it to circular list */
	nkey_copy(&nkey, &histp->list[histp->past_end]);
	if (histp->start == histp->past_end) {
		/* full buffer & we just overwrote the oldest */
		histp->start = (histp->start+1) % histp->size;
	}
	/* advance pointer to account for what we just added */
	histp->past_end = (histp->past_end+1) % histp->size;
}
/*==================================================
 * history_back -- return prev NODE in history, if exists
 * Created: 2001/03?, Perry Rapp
 *================================================*/
static NODE
history_back (struct hist * histp)
{
	INT last;
	NODE node;
	if (!histp->size || histp->start==-1)
		return NULL;
	/* back up from histp->past_end to current item */
	last = histp->past_end-1;
	if (last < 0) last += histp->size;
	while (last != histp->start) {
		/* loop is to keep going over deleted ones */
		/* now back up before current item */
		if (--last < 0) last += histp->size;
		nkey_to_node(&histp->list[last], &node);
		if (node) {
			histp->past_end = (last+1) % histp->size;
			return node;
		}
	}
	return NULL;
}
/*==================================================
 * history_fwd -- return later NODE in history, if exists
 * Created: 2001/03?, Perry Rapp
 *================================================*/
static NODE
history_fwd (struct hist * histp)
{
	INT next;
	NODE node;
	if (!histp->size || histp->past_end == histp->start)
		return NULL; /* at end of full history */
	next = histp->past_end;
	nkey_to_node(&histp->list[next], &node);
	return node;
}
/*==================================================
 * history_list -- let user choose from history list
 *  calls message(nohist) if none found
 *  returns NULL if no history or if user cancels
 * Created: 2001/04/12, Perry Rapp
 *================================================*/
static NODE
history_list (struct hist * histp)
{
	INDISEQ seq;
	NODE node;
	INT next, prev;
	if (!histp->size || histp->start==-1) {
		message(nohist);
		return NULL;
	}
	/* add all items of history to seq */
	seq = create_indiseq_null();
	prev = -1;
	next = histp->start;
	while (1) {
		nkey_to_node(&histp->list[next], &node);
		if (node) {
			STRING key = node_to_key(node);
			append_indiseq_null(seq, key, NULL, TRUE, FALSE);
		}
		prev = next;
		next = (next+1) % histp->size;
		if (next == histp->past_end)
			break; /* finished them all */
	}
	node = nztop(choose_from_indiseq(seq, DOASK1, idhist, idhist));
	remove_indiseq(seq);
	return node;
}
/*==================================================
 * ask_clear_history -- delete vist history
 *  (first verify via y/n prompt)
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static void
ask_clear_history (struct hist * histp)
{
	char buffer[120];
	INT count;

	if (!histp->size || histp->start==-1) {
		message(nohist);
		return;
	}
	count = get_hist_count(histp);
	sprintf(buffer, histclr, count);
	if (ask_yes_or_no(buffer))
		histp->start = -1;
}
/*==================================================
 * handle_history_cmds -- handle the history commands
 *  returns 0 for not a history command
 *  returns 1 if handled but continue on same page
 *  returns -1 if handled & set *pindi1 for switching pages
 * Created: 2001/04/12, Perry Rapp
 *================================================*/
static INT
handle_history_cmds (INT c, NODE * pindi1)
{
	NODE node;
	if (c == CMD_HISTORY_BACK) {
		node = history_back(&vhist);
		if (node) {
			*pindi1 = node;
			return -1; /* handled, change pages */
		}
		message(nohist);
		return 1; /* handled, stay here */
	}
	if (c == CMD_HISTORY_FWD) {
		node = history_fwd(&vhist);
		if (node) {
			*pindi1 = node;
			return -1; /* handled, change pages */
		}
		message(nohist);
		return 1; /* handled, stay here */
	}
	if (c == CMD_HISTORY_LIST) {
		node = history_list(&vhist);
		if (node) {
			*pindi1 = node;
			return -1; /* handled, change pages */
		}
		return 1;
	}
	if (c == CMD_HISTORY_CLEAR) {
		ask_clear_history(&vhist);
		return 1;
	}
	return 0; /* unhandled */
}
/*==================================================
 * add_new_rec_maybe_ref -- add a new record
 *  and optionally create a reference to it from
 *  current record
 * rtns: returns new node if user wants to browse to it
 *       current node to edit it (current node)
 *       NULL to just stay where we are
 * Created: 2001/04/06, Perry Rapp
 *================================================*/
static NODE
add_new_rec_maybe_ref (NODE node, char ntype)
{
	NODE newnode=NULL;
	STRING choices[4];
	char title[60];
	INT rtn;

	/* create new node of requested type */
	if (ntype=='E') 
		newnode=add_event();
	else if (ntype=='S')
		newnode=add_source();
	else
		newnode=add_other();
	/* bail if user cancelled creation */
	if (!newnode)
		return NULL;
	/* sanity check for long tags in others */
	if (strlen(ntag(newnode))>40) {
		msg_info(tag2long2cnc);
		return newnode;
	}
	/* now ask the user how to connect the new node */
	sprintf(title, newrecis, nxref(newnode));
	msg_info(title);
	/* keep new node # in status so it will be visible during edit */
	lock_status_msg(TRUE);
	choices[0] = autoxref;
	choices[1] = editcur;
	choices[2] = gotonew;
	choices[3] = staycur;
	rtn = choose_from_array(NULL, 4, choices);
	lock_status_msg(FALSE);
	switch(rtn) {
	case 0: 
		autoadd_xref(node, newnode);
		return NULL;
	case 1:
		return node; /* convention - return current node for edit */
	case 2:
		return newnode;
	default:
		return NULL;
	}
}
/*==================================================
 * autoadd_xref -- add trailing xref from existing node
 *  to new node
 * Created: 2001/11/11, Perry Rapp
 *================================================*/
static void
autoadd_xref (NODE node, NODE newnode)
{
	NODE xref; /* new xref added to end of node */
	NODE find, prev; /* used finding last child of node */
	xref = create_node(NULL, ntag(newnode), nxref(newnode), node);
	if (!(find = nchild(node))) {
		nchild(node) = xref;
	} else {
		/* find last child of node */
		while(find) {
			prev = find;
			find = nsibling(find);
		}
		nsibling(prev) = xref;
	}
	unknown_node_to_dbase(node);
}
/*==================================================
 * init_browse_module -- Do any initialization
 *  This is after database is determined, and before
 *  main_menu begins processing.
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
void
init_browse_module (void)
{
	load_hist_lists();
}
/*==================================================
 * term_browse_module -- Cleanup for browse module
 *  Primarily to persist history
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
void
term_browse_module (void)
{
	save_hist_lists();
}
