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
extern STRING idsbrs, idsrmv, idfbrs, idcbrs, idcrmv, iscnew, issnew;
extern STRING idfcop, ntprnt, nofath, nomoth, nospse, noysib, noosib;
extern STRING noprnt, nohusb, nowife, hasbth, hasnei, nocinf, nocofp;
extern STRING idpnxt, ids2fm, idc2fm, idplst, idp2br, crtcfm, crtsfm;
extern STRING ronlye, ronlya, idhbrs, idwbrs;
extern STRING id1sbr, id2sbr, id1fbr, id2fbr, id1cbr, id2cbr;
extern STRING id1hbr, id2hbr, id1wbr, id2wbr;

/*********************************************
 * local enums & defines
 *********************************************/

#define ALLPARMS &indi1, &indi2, &fam1, &fam2, &seq

/*********************************************
 * local function prototypes
 *********************************************/

static INT browse_indi(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static INT browse_fam(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static INT browse_pedigree(NODE*, NODE*, NODE*, NODE*, INDISEQ*);
static BOOLEAN handle_menu_commands(INT c);
static BOOLEAN handle_menu_commands_old(INT c);
static NODE goto_indi_child(NODE indi, int childno);
static NODE goto_fam_child(NODE fam, int childno);
static INT display_indi(NODE indi, INT mode);
static INT browse_indi_modes(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq, INT indimode);
static INT display_aux(NODE node, INT mode);
static INT browse_aux(NODE node);
static INT browse_indi(NODE *pindi1, NODE *pindi2, NODE *pfam1,
	NODE *pfam2, INDISEQ *pseq);
static INT display_fam(NODE fam, INT fammode);

/*********************************************
 * local variables
 *********************************************/

static BOOLEAN gedcom_mode = FALSE;


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

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
	if (mode == 'i') {
		if (gedcom_mode)
			c = indi_ged_browse(indi);
		else
			c = indi_browse(indi);
	}
	else
		c = ped_browse(indi);
	unlock_cache(icel);
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
	char scratch[100];

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
		c = display_indi(indi, indimode);
		/* last keynum & mode, so can tell if changed */
		nkeyp = indi_to_keynum(indi);
		indimodep = indimode;
		if (c != 'a') save = NULL;
		if (!handle_menu_commands(c))
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
		case CMD_2FAM:
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
		case CMD_SCROLL_UP:       /* scroll details/pedigree up */
			show_scroll(-1);
			break;
		case CMD_SCROLL_DOWN:       /* scroll details/pedigree down */
			show_scroll(+1);
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
		case CMD_GEDCOM_MODE: /* Switch to gedcom mode */
			gedcom_mode = !gedcom_mode;
			indimodep = 0; /* force redraw */
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
		case CMD_2PAR:	/* tandem browse to two parents families*/
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
			if (!(node = add_indi_by_edit())) break;
			save = indi;
			indi = node;
			break;
		case CMD_NEWFAMILY:	/* Add family for current person */
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
			if (node)
				browse_source_node(node);
			break;
		case CMD_QUIT:
		default:
			return BROWSE_QUIT;
		}
	}
}
/*================================================
 * browse_source_node -- Browse a source
 * TO DO - should become obsoleted by browse_source
 * Created: 2001/01/06, Perry Rapp
 * Implemented: 2001/01/27, Perry Rapp
 *==============================================*/
void
browse_source_node (NODE sour)
{
	browse_aux(sour);
}
/*=================================
 * browse_source -- Browse a source
 * Created: 2001/01/06, Perry Rapp
 * Implemented: 2001/01/27, Perry Rapp
 *===============================*/
void
browse_source (NOD0 sour)
{
	browse_source_node(nztop(sour));
}
/*=================================
 * browse_event -- Browse an event
 * Created: 2001/01/06, Perry Rapp
 * Implemented: 2001/01/27, Perry Rapp
 *===============================*/
void
browse_event (NOD0 even)
{
	browse_aux(nztop(even));
}
/*=================================
 * browse_other -- Browse an other
 * Created: 2001/01/06, Perry Rapp
 * Implemented: 2001/01/27, Perry Rapp
 *===============================*/
void
browse_other (NOD0 othr)
{
	browse_aux(nztop(othr));
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
	if (mode == 'x')
		c = aux_browse(node);
	else {
		ASSERT(0); /* no other modes supported */
	}
	unlock_cache(cel);
	return c;
}
/*====================================================
 * browse_aux -- Handle aux node browse.
 * Created: 2001/01/27, Perry Rapp
 *==================================================*/
static INT
browse_aux (NODE node)
{
	STRING key = rmvat(nxref(node));
	char ntype = key[0];
	INT i, c;
	INT nkeyp, auxmode, auxmodep;
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
		c = display_aux(node, auxmode);
		/* last keynum & mode, so can tell if changed */
		nkeyp = node_to_keynum(ntype, node);
		auxmodep = auxmode;
		if (!handle_menu_commands(c))
			switch (c)
		{
		case CMD_TEST88:
			message("Test88");
			break;
		case CMD_TEST999:
			message("Test999");
			break;
		case CMD_EDIT:
			switch(ntype) {
			case 'S': edit_source(node); break;
			case 'E': edit_event(node); break;
			case 'X': edit_other(node); break;
			}
			break;
		case CMD_NEXT:	/* Go to next indi in db */
			{
				i = xref_next(ntype, nkeyp);
				if (i)
					node = keynum_to_node(ntype, i);
				else message(norec);
				break;
			}
		case CMD_PREV:	/* Go to prev indi in db */
			{
				i = xref_prev(ntype, nkeyp);
				if (i)
					node = keynum_to_node(ntype, i);
				else message(norec);
				break;
			}
		case CMD_SCROLL_UP:       /* scroll details/pedigree up */
			show_scroll(-1);
			break;
		case CMD_SCROLL_DOWN:       /* scroll details/pedigree down */
			show_scroll(+1);
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
	return browse_indi_modes(pindi1, pindi2, pfam1, pfam2, pseq, 'i');
}
/*===========================================
 * display_fam -- Show family in current mode
 *=========================================*/
static INT
display_fam (NODE fam, INT fammode)
{
	CACHEEL icel;
	INT c=0;
	icel = fam_to_cacheel(fam);
	lock_cache(icel);
	if (fammode == 'f') {
		if (gedcom_mode)
			c = fam_ged_browse(fam);
		else
			c = fam_browse(fam);
	}
	else {
		ASSERT(0); /* no other modes */
	}
	unlock_cache(icel);
	return c;
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
	INT fammode='f';
	INT nkeyp, fammodep;
	NODE save = NULL, fam = *pfam1, node, husb, wife, chil, rest;
	NODE root, fref, spnodes[30];
	INDISEQ seq;
	STRING key, name, spstrings[2];
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
		c = display_fam(fam, fammode);
		/* last keynum & mode, so can tell if changed */
		nkeyp = fam_to_keynum(fam);
		fammodep = fammode;
		if (c != 'a' && c != 's') save = NULL;
		if (!handle_menu_commands_old(c))
			switch (c) 
		{
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
			node = edit_family(fam);
			if (node)
				fam = node;
			break;
		case 'f':	/* Browse to family's father */
			*pindi = choose_father(NULL, fam, nohusb,
			    idhbrs, NOASK1);
			if (*pindi) return BROWSE_INDI;
			break;
		case 'F':	/* Tandem Browse to family's fathers */
			*pindi = choose_father(NULL, fam, nohusb,
			    id1hbr, NOASK1);
			if (*pindi) {
			  *pdum = choose_father(NULL, fam, nohusb,
			    id2hbr, NOASK1);
			  if (*pdum) 
				return BROWSE_TAND;
			}
			break;
		case 'm':	/* Browse to family's mother */
			*pindi = choose_mother(NULL, fam, nowife,
			    idwbrs, NOASK1);
			if (*pindi) return BROWSE_INDI;
			break;
		case 'M':	/* Tandem Browse to family's mother */
			*pindi = choose_mother(NULL, fam, nowife,
			    id1wbr, NOASK1);
			if (*pindi) {
				*pdum = choose_mother(NULL, fam, nowife, 
					id2wbr, NOASK1);
				if (*pdum) 
					return BROWSE_TAND;
			}
			break;
		case 'c':	/* Browse to a child */
			*pindi = choose_child(NULL, fam, nocinf,
				idcbrs, NOASK1);
			if (*pindi) return BROWSE_INDI;
			break;
		case 'C':	/* browse to tandem children */
			*pindi = choose_child(NULL, fam, nocinf,
			    id1cbr, NOASK1);
			if (*pindi) {
				*pdum = choose_child(NULL, fam, nocinf,
					id2cbr, NOASK1);
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
			    idcrmv, DOASK1);
			if (*pindi) choose_and_remove_child(*pindi, NULL, TRUE);
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
			choose_and_remove_spouse(spnodes[i], fam, TRUE);
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
				seq=NULL;
				return BROWSE_INDI;
				break;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case 'z':	/* Zip browse to new person */
			*pindi = ask_for_indi(idpnxt, NOCONFIRM, NOASK1);
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
				i = xref_nextf(nkeyp);
				if (i)
					fam = keynum_to_fam(i);
				else message(nofam);
				break;
			}
		case '-':	/* Go to prev indi in db */
			{
				i = xref_prevf(nkeyp);
				if (i)
					fam = keynum_to_fam(i);
				else message(nofam);
				break;
			}
		case '$':	/* Browse to sources */
			node = choose_source(fam, nosour, idsour);
			if (node)
				browse_source_node(node);
			break;
		case '!': /* Switch to gedcom mode */
			gedcom_mode = !gedcom_mode;
			fammodep = 0; /* force redraw */
			break;
		case 'q':
		default:
			return BROWSE_QUIT;
		}
	}
}
/*======================================================
 * handle_menu_commands -- Handle menuing commands
 * Created: 2001/01/31, Perry Rapp
 *====================================================*/
static BOOLEAN
handle_menu_commands (INT c)
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
 * handle_menu_commands_old -- Handle menuing commands
 * Created: 2001/01/31, Perry Rapp
 *====================================================*/
static BOOLEAN
handle_menu_commands_old (INT c)
{
	switch(c) {
		case '<': adjust_menu_height(+1); return TRUE;
		case '>': adjust_menu_height(-1); return TRUE;
		case '?': cycle_menu(); return TRUE;
		case '*': toggle_menu(); return TRUE;
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
 * browse_sources -- browse list of all sources
 *================================================*/
void browse_sources (void)
{
	NOD0 nod0 = choose_any_source();
	if (nod0)
		browse_source(nod0);
}
/*==================================================
 * browse_events -- browse list of all events
 *================================================*/
void browse_events (void)
{
	INDISEQ seq = get_all_even();
	NOD0 nod0;
	if (!seq)
	{
		message(noeven);
		return;
	}
	nod0 = choose_from_indiseq(seq, DOASK1, ideven, ideven);
	remove_indiseq(seq, FALSE);
	if (nod0)
		browse_event(nod0);
}
/*==================================================
 * browse_others -- browse list of all sources
 *================================================*/
void browse_others (void)
{
	INDISEQ seq = get_all_othe();
	NOD0 nod0;
	if (!seq)
	{
		message(noothe);
		return;
	}
	nod0 = choose_from_indiseq(seq, DOASK1, idothe, idothe);
	remove_indiseq(seq, FALSE);
	if (nod0)
		browse_other(nod0);
}
