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
 * tandem.c -- LifeLines tandem browsing operations
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 05 May 94
 *   3.0.2 - 29 Dec 94
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
 * external/imported variables
 *********************************************/

extern STRING nofath, nomoth, nospse, nocofp;
extern STRING twohsb, twowif, idsbrs, idplst, idcbrs;

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN handle_tandem_scroll_cmds(INT c);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=============================================
 * browse_tandem -- Two person browse operation
 *===========================================*/
INT browse_tandem (NODE *pindi1, NODE *pindi2, NODE *pfam1, NODE *pfam2, INDISEQ *pseq)
{
	INT nkey1p, nkey2p, modep;
	NODE node, indi1 = *pindi1, indi2 = *pindi2;
	STRING key, name;
	INDISEQ seq;
	INT c, rc, reuse;
	static INT mode = 'n';

	if (!indi1 || !indi2) return BROWSE_QUIT;
	show_reset_scroll();
	nkey1p = 0; /* force redraw */
	modep = mode;

	while (TRUE) {
		if (indi_to_keynum(indi1) != nkey1p
			|| indi_to_keynum(indi2) != nkey2p
			|| mode != modep) {
			show_reset_scroll();
		}
		c = display_2indi(indi1, indi2, mode);
		/* last keynum & mode, so can tell if changed */
		nkey1p = indi_to_keynum(indi1);
		nkey2p = indi_to_keynum(indi2);
		modep = mode;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_indi_mode_cmds(c, &mode))
			continue;
		if (handle_tandem_scroll_cmds(c))
			continue;
		switch (c)
		{
		case CMD_EDIT: 	/* edit top person */
			indi1 = edit_indi(indi1);
			break;
		case CMD_TOP: 	/* browse top person */
			*pindi1 = indi1;
			return BROWSE_INDI;
		case CMD_FATHER: 	/* browse top person's father */
			if (!(node = indi_to_fath(indi1)))
				message(nofath);
			else
				indi1 = node;
			break;
		case CMD_MOTHER: 	/* browse top person's mother */
			if (!(node = indi_to_moth(indi1)))
				message(nomoth);
			else
				indi1 = node;
			break;
		case CMD_SPOUSE: 	/* browse top person's spouse/s */
			node = choose_spouse(indi1, nospse, idsbrs);
			if (node) indi1 = node;
			break;
		case CMD_CHILDREN: 	/* browse top person's children */
			if ((node = choose_child(indi1, NULL, nocofp,
			    idcbrs, NOASK1)))
				indi1 = node;
			break;
		case CMD_MERGE_BOTTOM_TO_TOP: 	/* merge two persons */
			if ((node = merge_two_indis(indi2, indi1, TRUE))) {
				*pindi1 = node;
				return BROWSE_INDI;
			}
			break;
		case CMD_COPY_TOP_TO_BOTTOM: 	/* copy top person to bottom */
			indi2 = indi1;
			break;
		case CMD_SWAPTOPBOTTOM: 	/* swap two persons */
			node = indi1;
			indi1 = indi2;
			indi2 = node;
			break;
		case CMD_ADDFAMILY: 	/* make two persons parents in family */
			node = add_family(indi1, indi2, NULL);
			if (!node)  break;
			*pfam1 = node;
			return BROWSE_FAM;
		case CMD_BROWSE: 	/* browse to new person list */
			seq = (INDISEQ) ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				*pindi1 = key_to_indi(key);
				remove_indiseq(seq);
				return BROWSE_INDI;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case CMD_QUIT:
		default:
			return BROWSE_QUIT;
		}
	}
}
/*==================================================
 * browse_2fam -- Handle two family browse operation
 *================================================*/
INT browse_2fam (NODE *pindi1, NODE *pindi2, NODE *pfam1, NODE *pfam2, INDISEQ *pseq)
{
	INT nkey1p, nkey2p, modep;
	NODE node, fam1 = *pfam1, fam2 = *pfam2;
	INT c, reuse;
	static INT mode = 'n';

	ASSERT(fam1 && fam2);
	show_reset_scroll();
	nkey1p = 0; /* force redraw */
	modep = mode;

	while (TRUE) {
		if (fam_to_keynum(fam1) != nkey1p
			|| fam_to_keynum(fam2) != nkey2p
			|| mode != modep) {
			show_reset_scroll();
		}
		c = display_2fam(fam1, fam2, mode);
		/* last keynum & mode, so can tell if changed */
		nkey1p = fam_to_keynum(fam1);
		nkey2p = fam_to_keynum(fam2);
		modep = mode;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_fam_mode_cmds(c, &mode))
			continue;
		if (handle_tandem_scroll_cmds(c))
			continue;
		switch (c)
		{
		case CMD_EDIT:	/* edit top fam */
			fam1 = edit_family(fam1);
			break;
		case CMD_TOP:	/* browse top fam */
			*pfam1 = fam1;
			return BROWSE_FAM;
		case CMD_BOTTOM:	/* browse bottom fam */
			*pfam1 = fam2;
			return BROWSE_FAM;
		case CMD_BOTH_FATHERS:	/* browse to husbs/faths */
			*pindi1 = fam_to_husb(fam1);
			*pindi2 = fam_to_husb(fam2);
			if (!*pindi1 || !*pindi2) {
				message(twohsb);
				break;
			}
			return BROWSE_TAND;
		case CMD_BOTH_MOTHERS:	/* browse to wives/moths */
			*pindi1 = fam_to_wife(fam1);
			*pindi2 = fam_to_wife(fam2);
			if (!*pindi1 || !*pindi2) {
				message(twowif);
				break;
			}
			return BROWSE_TAND;
		case CMD_MERGE_BOTTOM_TO_TOP:	/* merge two fams */
			if ((node = merge_two_fams(fam2, fam1))) {
				*pfam1 = node;
				return BROWSE_FAM;
			}
			break;
		case CMD_SWAPTOPBOTTOM:	/* swap two fams */
			node = fam1;
			fam1 = fam2;
			fam2 = node;
			break;
		case CMD_TOGGLE_CHILDNUMS:       /* toggle children numbers */
			show_childnumbers();
			break;
		case CMD_QUIT:	/* Return to main menu */
		default:
			return BROWSE_QUIT;
		}
	}
}
/*======================================================
 * handle_tandem_scroll_cmds -- Handle tandem scrolling
 * Created: 2001/02/04, Perry Rapp
 *====================================================*/
static BOOLEAN
handle_tandem_scroll_cmds (INT c)
{
	switch(c) {
	case CMD_SCROLL_TOP_UP: show_scroll(-1); return TRUE;
	case CMD_SCROLL_TOP_DOWN: show_scroll(+1); return TRUE;
	case CMD_SCROLL_BOTTOM_UP: show_scroll2(-1); return TRUE;
	case CMD_SCROLL_BOTTOM_DOWN: show_scroll2(+1); return TRUE;
	case CMD_SCROLL_BOTH_UP: show_scroll(-1); show_scroll2(-1); return TRUE;
	case CMD_SCROLL_BOTH_DOWN: show_scroll(+1); show_scroll2(+1); return TRUE;
	}
	return FALSE;
}
