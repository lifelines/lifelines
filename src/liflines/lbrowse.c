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
/*==============================================================
 * lbrowse.c -- Handle list browse mode
 * Copyright (c) 1993-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 09 Oct 94    3.0.2 - 30 Dec 94
 *   3.0.3 - 20 Jan 96
 *============================================================*/

#include "standard.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"

#define VIEWABLE 13

extern STRING lstnam, lstnon, lstwht, lstnad, lstpad, lstbot, lsttop;
extern STRING idplst, lstnew, mrkper;

extern INDISEQ current_seq;
LIST browse_lists;

/* in screen.c */
INT list_browse (INDISEQ seq, INT top, INT cur, INT mark);


/*=======================================
 * browse_list -- Handle list browse mode
 *=====================================*/
INT
browse_list (NODE *pindi1,
             NODE *pindi2,
             NODE *pfam1,
             NODE *pfam2,
             INDISEQ *pseq)
{
	INT top, cur, mark, len, tmp, rc;
	STRING key, name, newname, lname;
	NODE indi;
	INDISEQ seq, newseq;

	current_seq = NULL;
	if (!pseq || !(seq = *pseq) || (len = length_indiseq(seq)) <= 0)
		return  BROWSE_QUIT;
	top = cur = 0;
	mark =  -1;
	element_indiseq(seq, cur, &key, &name);
	indi = key_to_indi(key);
	current_seq = seq;

	while (TRUE) {
		switch (list_browse(seq, top, cur, mark)) {
		case 'j':	/* Move down line */
			if (cur >= len - 1) {
				message(lstbot);
				break;
			}
			cur++;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			if (cur >= top + VIEWABLE) top++;
			break;
		case 'k':	/* Move up line */
			if (cur <= 0) {
				message(lsttop);
				break;
			}
			cur--;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			if (cur + 1 == top) top--;
			break;
		case 'e':	/* Edit current person */
			indi = edit_indi(indi);
	    		if ((len = length_indiseq(seq)) <= 0) {
				remove_browse_list(lname, seq);
				current_seq = NULL;
				lname = NULL;
				return BROWSE_QUIT;
			}
			if (cur >= len) cur = len - 1;
			break;
		case 'i':	/* Browse current person */
			*pindi1 = indi;
			if (current_seq)
				remove_indiseq(current_seq, FALSE);
			current_seq = NULL;
			return BROWSE_INDI;
		case 'm':	/* Mark current person */
			mark = (cur == mark) ? -1: cur;
			break;
		case 'd':	/* Delete person from list */
			if (len <= 1) {
				if (current_seq)
					remove_indiseq(current_seq, FALSE);
				current_seq = NULL;
				return BROWSE_QUIT;
			}
			delete_indiseq(seq, NULL, NULL, cur);
			len--;
			if (mark == cur) mark = -1;
			if (mark > cur) mark--;
			if (cur == len)
				cur--;
				element_indiseq(seq, cur, &key, &name);
				indi = key_to_indi(key);
			if (cur < top) top = cur;
			break;
		case 't':	/* Enter tandem mode */
			if (mark == -1 || cur == mark) {
				message(mrkper);
				break;
			}
			*pindi2 = indi;
			element_indiseq(seq, mark, &key, &name);
			*pindi1 = key_to_indi(key);
			current_seq = NULL;
			return BROWSE_TAND;
		case 'b':	/* Browse new persons */
			newseq = (INDISEQ) ask_for_indiseq(idplst, &rc);
			if (!newseq) break;
			current_seq = seq = newseq;
			element_indiseq(seq, 0, &key, &name);
			indi = key_to_indi(key);
			if ((len = length_indiseq(seq)) == 1) {
				*pindi1 = indi;
				remove_indiseq(newseq, FALSE);
				current_seq = NULL;
				return BROWSE_INDI;
			}
			top = cur = 0;
			mark = -1;
			break;
		case 'a':	/* Add persons to current list */
			newseq = (INDISEQ) ask_for_indiseq(lstpad, &rc);
			if (!newseq) {
				message(lstnad);
				break;
			}
			FORINDISEQ(newseq, e, i)
				append_indiseq(seq, skey(e), snam(e), NULL,
				    FALSE, FALSE);
			ENDINDISEQ
			namesort_indiseq(seq);
			cur = top = 0;
			mark = -1;
			len = length_indiseq(seq);
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			remove_indiseq(newseq, FALSE);
			message(lstnew);
			break;
		case 'n':	/* Name this list */
			newname = ask_for_string(lstwht, "enter name: ");
			if (!newname || *newname == 0)
				message(lstnon);
			else {
				newname = strsave(newname);
				add_browse_list (newname, copy_indiseq(seq));
				mprintf(lstnam, newname);
			}
			break;
		case 'x':	/* Swap current with marked */
			if (mark == -1) break;
			tmp = mark;
			mark = cur;
			cur = tmp;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			if (cur < top) top = cur;
			if (cur > top + VIEWABLE - 1) top = cur;
			break;
		case 'q':	/* Return to main menu */
		default:
			current_seq = NULL;
			return BROWSE_QUIT;
		}
	}
}
