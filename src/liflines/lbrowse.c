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

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "menuitem.h"

#include "llinesi.h"

#define VIEWABLE 13

extern STRING qSlstnam, qSlstnon, qSlstwht, qSlstnad, qSlstpad, qSlstbot, qSlsttop;
extern STRING qSasknam;
extern STRING qSidplst, qSlstnew, qSmrkper;
extern INDISEQ current_seq;

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
	INT c, top, cur, mark, len, tmp, rc;
	STRING key, name, newname, lname="";
	NODE indi;
	INDISEQ seq, newseq;
	pfam1=pfam1; /* unused */
	pfam2=pfam2; /* unused */

	current_seq = NULL;
	if (!pseq || !(seq = *pseq) || (len = length_indiseq(seq)) <= 0)
		return  BROWSE_QUIT;
	top = cur = 0;
	mark =  -1;
	calc_indiseq_names(seq); /* ensure we have names */
	element_indiseq(seq, cur, &key, &name);
	indi = key_to_indi(key);
	current_seq = seq;

	while (TRUE) {
		switch (c = list_browse(seq, top, &cur, mark, &indi)) {
		case 'j':        /* Move down line */
		case CMD_KY_DN:
			if (cur >= len - 1) {
				message(_(qSlstbot));
				break;
			}
			cur++;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			if (cur >= top + VIEWABLE) top++;
			break;
		case 'd':        /* Move down one page */
		case CMD_KY_PGDN:
			if (top + VIEWABLE >= len-1) {
				message(_(qSlstbot));
				break;
			}
			cur += VIEWABLE;
			if (cur > len-1)
				cur = len-1;
			top += VIEWABLE;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			break;
		case 'D':        /* Move down several pages */
		case CMD_KY_SHPGDN:
			if (top + VIEWABLE >= len-1) {
				message(_(qSlstbot));
				break;
			}
			tmp = (len)/10;
			if (tmp < VIEWABLE*2) tmp = VIEWABLE*2;
			if (tmp > len - VIEWABLE - top)
				tmp = len - VIEWABLE - top;
			top += tmp;
			cur += tmp;
			if (cur > len-1)
				cur = len-1;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			break;
		case '$':        /* jump to end of list */
		case CMD_KY_END:
			top = len - VIEWABLE;
			if (top < 0)
				top = 0;
			cur = len-1;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			break;
		case 'k':        /* Move up line */
		case CMD_KY_UP:
			if (cur <= 0) {
				message(_(qSlsttop));
				break;
			}
			cur--;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			if (cur + 1 == top) top--;
			break;
		case 'u':        /* Move up one page */
		case CMD_KY_PGUP:
			if (top <= 0) {
				message(_(qSlsttop));
				break;
			}
			tmp = VIEWABLE;
			if (tmp > top) tmp = top;
			cur -= tmp;
			top -= tmp;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			break;
		case 'U':        /* Move up several pages */
		case CMD_KY_SHPGUP:
			if (top <= 0) {
				message(_(qSlsttop));
				break;
			}
			tmp = (len)/10;
			if (tmp < VIEWABLE*2) tmp = VIEWABLE*2;
			if (tmp > top) tmp = top;
			cur -= tmp;
			top -= tmp;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			break;
		case '^':        /* jump to top of list */
		case CMD_KY_HOME:
			top = cur = 0;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			break;
		case 'e':        /* Edit current person */
			indi = edit_indi(indi);
	    		if ((len = length_indiseq(seq)) <= 0) {
				remove_browse_list(lname, seq);
				current_seq = NULL;
				lname = NULL;
				return BROWSE_QUIT;
			}
			if (cur >= len) cur = len - 1;
			break;
		case 'i':        /* Browse current person */
		case CMD_KY_ENTER:
			*pindi1 = indi;
			if (current_seq)
				remove_indiseq(current_seq);
			current_seq = NULL;
			return BROWSE_INDI;
		case 'm':        /* Mark current person */
			mark = (cur == mark) ? -1: cur;
			break;
		case 'r':        /* Remove person from list */
			if (len <= 1) {
				if (current_seq)
					remove_indiseq(current_seq);
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
		case 't':        /* Enter tandem mode */
			if (mark == -1 || cur == mark) {
				message(_(qSmrkper));
				break;
			}
			*pindi2 = indi;
			element_indiseq(seq, mark, &key, &name);
			*pindi1 = key_to_indi(key);
			current_seq = NULL;
			return BROWSE_TAND;
		case 'b':        /* Browse new persons */
			newseq = (INDISEQ) ask_for_indiseq(_(qSidplst), 'I', &rc);
			if (!newseq) break;
			current_seq = seq = newseq;
			element_indiseq(seq, 0, &key, &name);
			indi = key_to_indi(key);
			if ((len = length_indiseq(seq)) == 1) {
				*pindi1 = indi;
				remove_indiseq(newseq);
				current_seq = NULL;
				return BROWSE_INDI;
			}
			top = cur = 0;
			mark = -1;
			break;
		case 'a':        /* Add persons to current list */
			newseq = (INDISEQ) ask_for_indiseq(_(qSlstpad), 'I', &rc);
			if (!newseq) {
				message(_(qSlstnad));
				break;
			}
			FORINDISEQ(newseq, e, i)
				append_indiseq_null(seq, strsave(skey(e)), snam(e), FALSE, TRUE);
			ENDINDISEQ
			namesort_indiseq(seq);
			cur = top = 0;
			mark = -1;
			len = length_indiseq(seq);
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			remove_indiseq(newseq);
			message(_(qSlstnew));
			break;
		case 'n':        /* Name this list */
			newname = ask_for_string(_(qSlstwht), _(qSasknam));
			if (!newname || *newname == 0)
				message(_(qSlstnon));
			else {
				newname = strsave(newname);
				add_browse_list(newname, copy_indiseq(seq));
				msg_info(_(qSlstnam), newname);
			}
			break;
		case 'x':        /* Swap current with marked */
			if (mark == -1) break;
			tmp = mark;
			mark = cur;
			cur = tmp;
			element_indiseq(seq, cur, &key, &name);
			indi = key_to_indi(key);
			if (cur < top) top = cur;
			if (cur > top + VIEWABLE - 1) top = cur;
			break;
		case 'q':        /* Return to main menu */
			current_seq = NULL;
			return BROWSE_QUIT;
		}
	}
}
