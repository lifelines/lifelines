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

#include "llinesi.h"

extern STRING nofath, nomoth, nospse, nocofp;
extern STRING twohsb, twowif, idsbrs, idplst, idcbrs;

/*=============================================
 * browse_tandem -- Two person browse operation
 *===========================================*/
INT
browse_tandem (NODE *pindi1,
               NODE *pindi2,
               NODE *pfam1,
               NODE *pfam2,
               INDISEQ *pseq)
{
	NODE node, indi1 = *pindi1, indi2 = *pindi2;
	STRING key, name;
	INDISEQ seq;
	INT rc;

	if (!indi1 || !indi2) return BROWSE_QUIT;
	while (TRUE) {
		switch (tandem_browse(indi1, indi2)) {
		case 'e': 	/* edit top person */
			indi1 = edit_indi(indi1);
			break;
		case 't': 	/* browse top person */
			*pindi1 = indi1;
			return BROWSE_INDI;
		case 'f': 	/* browse top person's father */
			if (!(node = indi_to_fath(indi1)))
				message(nofath);
			else
				indi1 = node;
			break;
		case 'm': 	/* browse top person's mother */
			if (!(node = indi_to_moth(indi1)))
				message(nomoth);
			else
				indi1 = node;
			break;
		case 's': 	/* browse top person's spouse/s */
			node = choose_spouse(indi1, nospse, idsbrs);
			if (node) indi1 = node;
			break;
		case 'c': 	/* browse top person's children */
			if ((node = choose_child(indi1, NULL, nocofp,
			    idcbrs, FALSE)))
				indi1 = node;
			break;
		case 'j': 	/* merge two persons */
			if ((node = merge_two_indis(indi2, indi1, TRUE))) {
				*pindi1 = node;
				return BROWSE_INDI;
			}
			break;
		case 'd': 	/* copy top person to bottom */
			indi2 = indi1;
			break;
		case 'x': 	/* swap two persons */
			node = indi1;
			indi1 = indi2;
			indi2 = node;
			break;
		case 'a': 	/* make two persons parents in family */
			node = add_family(indi1, indi2, NULL);
			if (!node)  break;
			*pfam1 = node;
			return BROWSE_FAM;
		case 'b': 	/* browse to new person list */
			seq = ask_for_indiseq(idplst, &rc);
			if (!seq) break;
			if (length_indiseq(seq) == 1) {
				element_indiseq(seq, 0, &key, &name);
				*pindi1 = key_to_indi(key);
				remove_indiseq(seq, FALSE);
				return BROWSE_INDI;
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
/*==================================================
 * browse_2fam -- Handle two family browse operation
 *================================================*/
INT
browse_2fam (NODE *pindi1,
             NODE *pindi2,
             NODE *pfam1,
             NODE *pfam2,
             INDISEQ *pseq)
{
	NODE node, fam1 = *pfam1, fam2 = *pfam2;
	INT c;
	ASSERT(fam1 && fam2);
	while (TRUE) {
		c = twofam_browse(fam1, fam2);
		switch (c) {
		case 'e':	/* edit top fam */
			fam1 = edit_family(fam1);
			break;
		case 't':	/* browse top fam */
			*pfam1 = fam1;
			return BROWSE_FAM;
		case 'b':	/* browse bottom fam */
			*pfam1 = fam2;
			return BROWSE_FAM;
		case 'f':	/* browse to husbs/faths */
			*pindi1 = fam_to_husb(fam1);
			*pindi2 = fam_to_husb(fam2);
			if (!*pindi1 || !*pindi2) {
				message(twohsb);
				break;
			}
			return BROWSE_TAND;
		case 'm':	/* browse to wives/moths */
			*pindi1 = fam_to_wife(fam1);
			*pindi2 = fam_to_wife(fam2);
			if (!*pindi1 || !*pindi2) {
				message(twowif);
				break;
			}
			return BROWSE_TAND;
		case 'j':	/* merge two fams */
			if ((node = merge_two_fams(fam2, fam1))) {
				*pfam1 = node;
				return BROWSE_FAM;
			}
			break;
		case 'x':	/* swap two fams */
			node = fam1;
			fam1 = fam2;
			fam2 = node;
			break;
		case 'q':	/* Return to main menu */
		default:
			return BROWSE_QUIT;
		}
	}
}
