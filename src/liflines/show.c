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
 * show.c -- Curses version of display functions
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 24 Aug 93
 *   3.0.0 - 14 Sep 94    3.0.2 - 24 Dec 94
 *   3.0.3 - 03 May 95
 *===========================================================*/

#include "llstdlib.h"
#include "screen.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "cache.h"
#include "liflines.h"
#include "lloptions.h"

#include "llinesi.h"

/*********************************************
 * global/exported variables
 *********************************************/

struct rfmt_s disprfmt; /* reformatting used for display */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN opt_nocb;	/* TRUE to suppress display of cb. data */
extern INT LIST_LINES;		/* person info display lines above list */
extern INT MAINWIN_WIDTH;
extern INT listbadkeys;
extern char badkeylist[];
extern STRING dspa_chbr;

/*********************************************
 * local types
 *********************************************/

typedef char *LINESTRING;

/*********************************************
 * local enums & defines
 *********************************************/

#define MAXOTHERS 30

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_child_line(INT, NODE, INT width);
static void add_spouse_line(INT, NODE, NODE, INT width);
static BOOLEAN append_event(STRING * pstr, STRING evt, INT * plen, INT minlen);
static STRING disp_format_date(STRING date);;
static void family_events(STRING outstr, TRANTABLE tt, NODE indi, NODE fam, INT len);
static void indi_events(STRING outstr, TRANTABLE tt, NODE indi, INT len);
static void init_disp_reformat();
static void init_display_indi(NODE, INT width);
static void init_display_fam(NODE, INT width);
static STRING person_display(NODE, NODE, INT);
static STRING sh_fam_to_event(NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len, BOOLEAN shrt);
static STRING sh_indi_to_event(NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len, BOOLEAN shrt);
static void show_gedcom(WINDOW *w, NODE node, INT gdvw, INT row, INT hgt, BOOLEAN reuse);
static void wipe_window(WINDOW * w, INT row, INT hgt);

/*********************************************
 * local variables
 *********************************************/

static LINESTRING Spers, Sbirt, Sdeat, Sfath, Smoth, Smarr;
static LINESTRING Shusb, Shbirt, Shdeat, Swife, Swbirt, Swdeat;
static LINESTRING Sothers[MAXOTHERS];
static INT Solen = 0;
static INT Scroll1 = 0;
static INT Scroll2 = 0;
static INT number_child_enable = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===============================================
 * init_show_module -- Initialize display variables
 *=============================================*/
void
init_show_module ()
{
	INT i;
	INT width = ll_cols+1;

	Spers = (LINESTRING)stdalloc(width);
	Sbirt = (LINESTRING)stdalloc(width);
	Sdeat = (LINESTRING)stdalloc(width);
	Sfath = (LINESTRING)stdalloc(width);
	Smoth = (LINESTRING)stdalloc(width);
	Smarr = (LINESTRING)stdalloc(width);
	Shusb = (LINESTRING)stdalloc(width);
	Shbirt = (LINESTRING)stdalloc(width);
	Shdeat = (LINESTRING)stdalloc(width);
	Swife = (LINESTRING)stdalloc(width);
	Swbirt = (LINESTRING)stdalloc(width);
	Swdeat = (LINESTRING)stdalloc(width);
	for (i=0; i<MAXOTHERS; i++)
		Sothers[i] = (LINESTRING)stdalloc(width);
	init_disp_reformat();
}
/*===============================================
 * init_display_indi -- Initialize display person
 *  Fill in all the local buffers for normal person
 *  display mode (Spers, Sbirt, etc)
 *=============================================*/
static void
init_display_indi (NODE pers, INT width)
{
	INT nsp, nch, num, nm;
	STRING s,t;
	NODE fth;
	NODE mth;
	TRANTABLE ttd = tran_tables[MINDS];
	CACHEEL icel;

	ASSERT(width < ll_cols+1); /* size of Spers etc */


	ASSERT(pers);
	fth = indi_to_fath(pers);
	mth = indi_to_moth(pers);
	s = indi_to_name(pers, ttd, width-20);
	sprintf(Spers, "person: %s ", s);
	if((num = strlen(s)) < width-30) {
	    t = indi_to_title(pers, ttd, width-20 - num - 3);
	    if(t) sprintf(Spers+strlen(Spers), "[%s] ", t);
	}
	sprintf(Spers+strlen(Spers), "(%s)", key_of_record(pers));

	s = sh_indi_to_event(pers, ttd, "BIRT", "  born: ", (width-3), FALSE);
	if (!s) s = sh_indi_to_event(pers, ttd, "CHR", "  bapt: ", (width-3), FALSE);
	if (s) sprintf(Sbirt, s);
	else sprintf(Sbirt, "  born:");

	/* add a RESIdence if none was in the birth event */
	if(strchr(Sbirt, ',') == 0) {
		num = strlen(Sbirt);
		if(num < width-30) {
			s = sh_indi_to_event(pers, ttd, "RESI", ", of ", (width-3)-num-5, FALSE);
			if(s) {
				if(num < 8) strcat(Sbirt, s+1);
				else {
					/* overwrite the trailing "." on the birth info */
					strcpy(Sbirt + strlen(Sbirt)-1, s);
				}
			}
		}
	}

	s = sh_indi_to_event(pers, ttd, "DEAT", "  died: ", (width-3), FALSE);
	if (!s) s = sh_indi_to_event(pers, ttd, "BURI", "  buri: ", (width-3), FALSE);
	if (s) sprintf(Sdeat, s);
	else sprintf(Sdeat, "  died:");

	s = person_display(fth, NULL, width-13);
	if (s) sprintf(Sfath, "  father: %s", s);
	else sprintf(Sfath, "  father:");

	s = person_display(mth, NULL, width-13);
	if (s) sprintf(Smoth, "  mother: %s", s);
	else sprintf(Smoth, "  mother:");

	Solen = 0;
	nsp = nch = 0;
	icel = indi_to_cacheel(pers);
	lock_cache(icel);
	FORFAMSS(pers, fam, sp, num)
		if (sp) add_spouse_line(++nsp, sp, fam, width);
		FORCHILDREN(fam, chld, nm)
			if(chld) add_child_line(++nch, chld, width);
		ENDCHILDREN
	ENDFAMSS
	unlock_cache(icel);
}
/*==============================
 * show_person -- Display person
 *  win:    [in] which curses window (usually MAIN_WIN)
 *  pers:   [in] whom to display
 *  row:    [in] starting row to draw upon
 *  hgt:    [in] how many rows to use
 *  width:  [in] how many columns to use
 *  scroll: [in] how many rows to skip over at top
 *  reuse:  [in] flag to avoid recomputing display strings
 * Caller sets reuse flag if it knows that this is the same
 * person displayed last.
 *============================*/
void
show_person (WINDOW * win, NODE pers, INT row, INT hgt
	, INT width, INT *scroll, BOOLEAN reuse)
{
	INT i;
	INT localrow;
	INT overflow;
	badkeylist[0] = '\0';
	listbadkeys = 1;
	if (!hgt) return;
	if (!reuse)
		init_display_indi(pers, width);
	for (i = 0; i < hgt; i++) {
		wmove(win, row+i, 1);
		wclrtoeol(win);
#ifndef BSD
		mvwaddch(win, row+i, ll_cols-1, ACS_VLINE);
#endif
	}
	if (*scroll) {
		if (*scroll > Solen + 5 - hgt)
			*scroll = Solen + 5 - hgt;
		if (*scroll < 0)
			*scroll = 0;
	}
	localrow = row - *scroll;
	mvwaddstr(win, row+0, 1, Spers);
	if (hgt==1) return;
	mvwaddstr(win, row+1, 1, Sbirt);
	if (hgt==2) return;
	mvwaddstr(win, row+2, 1, Sdeat);
	if (hgt==3) return;
	mvwaddstr(win, row+3, 1, Sfath);
	if (hgt==4) return;
	mvwaddstr(win, row+4, 1, Smoth);
	if (hgt==5) return;
	for (i = *scroll; i < Solen && i < hgt-5+ *scroll; i++)
	{
		overflow = ((i+1 == hgt-5+ *scroll)&&(i+1 != Solen));
		if (*scroll && (i == *scroll))
			overflow = 1;
		put_out_line(win, localrow+5+i, 1, Sothers[i], width, overflow);
	}
	listbadkeys = 0;
	if(badkeylist[0]) {
		char buf[132];
		sprintf(buf, "WARNING: missing keys: %.40s", badkeylist);
		message(buf);
	}
}
/*====================================
 * show_person_main -- Display person
 *
 * [in] pers:  person
 * [in] row:   start row
 * [in] hgt:   avail rows
 * [in] reuse: flag to save recalculating display strings
 *==================================*/
void
show_person_main (NODE pers, INT row, INT hgt, BOOLEAN reuse)
{
	show_person(main_win, pers, row, hgt, MAINWIN_WIDTH, &Scroll1, reuse);
}
/*=============================================
 * add_spouse_line -- Add spouse line to others
 *===========================================*/
static void
add_spouse_line (INT num, NODE indi, NODE fam, INT width)
{
	STRING line;
	if (Solen >= MAXOTHERS) return;
	line = person_display(indi, fam, width-14);
	sprintf(Sothers[Solen], "  spouse: %s", line);
	Sothers[Solen++][width-2] = 0;
}
/*===========================================
 * add_child_line -- Add child line to others
 *=========================================*/
static void
add_child_line (INT num, NODE indi, INT width)
{
	STRING line;
	if (Solen >= MAXOTHERS) return;
	line = person_display(indi, NULL, width-15);
	if (number_child_enable)
		sprintf(Sothers[Solen], "  %2dchild: %s", num, line);
	else
		sprintf(Sothers[Solen], "    child: %s", line);
	Sothers[Solen++][width-2] = 0;
}
/*==============================================
 * init_display_fam -- Initialize display family
 *============================================*/
static void
init_display_fam (NODE fam, INT width)
{
	NODE husb;
	NODE wife;
	STRING s, ik, fk;
	INT len, nch, nm;
	TRANTABLE ttd = tran_tables[MINDS];
	ASSERT(fam);
	husb = fam_to_husb(fam);
	wife = fam_to_wife(fam);
	fk = key_of_record(fam);
	if (husb) {
		ik = key_of_record(husb);
		len = 64 - (strlen(ik) + strlen(fk));
		s = indi_to_name(husb, ttd, len);
		sprintf(Shusb, "father: %s (%s) (%s)", s, ik, fk);
	} else
		sprintf(Shusb, "father: (%s)", fk);

	s = sh_indi_to_event(husb, ttd, "BIRT", "  born: ", width-3, FALSE);
	if (!s) s = sh_indi_to_event(husb, ttd, "CHR", "  bapt: ", width-3, FALSE);
	if (s) sprintf(Shbirt, s);
	else sprintf(Shbirt, "  born:");

	s = sh_indi_to_event(husb, ttd, "DEAT", "  died: ", width-3, FALSE);
	if (!s) s = sh_indi_to_event(husb, ttd, "BURI", "  buri: ", width-3, FALSE);
	if (s) sprintf(Shdeat, s);
	else sprintf(Shdeat, "  died:");

	if (wife) {
		ik = key_of_record(wife);
		len = (width-13) - strlen(ik);
		s = indi_to_name(wife, ttd, len);
		sprintf(Swife, "mother: %s (%s)", s, ik);
	} else
		sprintf(Swife, "mother:");

	s = sh_indi_to_event(wife, ttd, "BIRT", "  born: ", width-3, FALSE);
	if (!s) s = sh_indi_to_event(wife, ttd, "CHR", " bapt: ", width-3, FALSE);
	if (s) sprintf(Swbirt, s);
	else sprintf(Swbirt, "  born:");

	s = sh_indi_to_event(wife, ttd, "DEAT", "  died: ", width-3, FALSE);
	if (!s) s = sh_indi_to_event(wife, ttd, "BURI", " buri: ", width-3, FALSE);
	if (s) sprintf(Swdeat, s);
	else sprintf(Swdeat, "  died:");

	s = sh_indi_to_event(fam, ttd, "MARR", "married: ", width-3, FALSE);
	if (s) sprintf(Smarr, s);
	else sprintf(Smarr, "married:");

	Solen = 0;
	nch = 0;
	FORCHILDREN(fam, chld, nm)
		add_child_line(++nch, chld, width);
	ENDCHILDREN
}
/*===================================
 * show_long_family -- Display family
 * [in] fam:  whom to display
 * [in] row:   starting row to use
 * [in] hgt:   how many rows allowed
 * [in] width: how many columns allowed
 * [in] reuse: flag to save recalculating display strings
 *=================================*/
void
show_long_family (NODE fam, INT row, INT hgt, INT width, BOOLEAN reuse)
{
	INT i;
	INT localrow;
	INT overflow;
	char buf[132];
	badkeylist[0] = '\0';
	listbadkeys = 1;
	if (!reuse)
		init_display_fam(fam, width);
	for (i = 0; i < hgt; i++) {
		wmove(main_win, row+i, 1);
		wclrtoeol(main_win);
#ifndef BSD
		mvwaddch(main_win, row+i, width-1, ACS_VLINE);
#endif
	}
	if (Scroll1) {
		if (Scroll1 > Solen + 7 - hgt)
			Scroll1 = Solen + 7 - hgt;
		if (Scroll1 < 0)
			Scroll1 = 0;
	}
	localrow = row - Scroll1;
	mvwaddstr(main_win, row+0, 1, Shusb);
	if (hgt==1) return;
	mvwaddstr(main_win, row+1, 1, Shbirt);
	if (hgt==2) return;
	mvwaddstr(main_win, row+2, 1, Shdeat);
	if (hgt==3) return;
	mvwaddstr(main_win, row+3, 1, Swife);
	if (hgt==4) return;
	mvwaddstr(main_win, row+4, 1, Swbirt);
	if (hgt==5) return;
	mvwaddstr(main_win, row+5, 1, Swdeat);
	if (hgt==6) return;
	mvwaddstr(main_win, row+6, 1, Smarr);
	for (i = Scroll1; i < Solen && i < hgt-7+Scroll1; i++)
	{
		overflow = ((i+1 == hgt-7+Scroll1)&&(i+1 != Solen));
		if (Scroll1 && (i == Scroll1))
			overflow = 1;
		put_out_line(main_win, localrow+7+i, 1, Sothers[i]+1, width, overflow);
	}
	listbadkeys = 0;
	if(badkeylist[0]) {
		sprintf(buf, "WARNING: missing keys: %.40s", badkeylist);
		message(buf);
	}
}
#ifdef UNUSED_CODE
/*====================================
 * show_short_family -- Display family
 * UNUSED CODE
 *==================================*/
void
show_short_family (NODE fam, INT row, INT hgt, INT width)
{
	INT i;
	char buf[132];
	badkeylist[0] = '\0';
	listbadkeys = 1;
	init_display_fam(fam, width);
	for (i = 0; i < hgt; i++) {
		wmove(main_win, row+i, 1);
		wclrtoeol(main_win);
#ifndef BSD
		mvwaddch(main_win, row+i, ll_cols-1, ACS_VLINE);
#endif
	}
	mvwaddstr(main_win, row+0, 1, Shusb);
	if (strlen(Shbirt) > 5)
		mvwaddstr(main_win, row+1, 1, Shbirt);
	else if (strlen(Shdeat) > 5)
		mvwaddstr(main_win, row+1, 1, Shdeat);
	else
		mvwaddstr(main_win, row+1, 1, Shbirt);

	mvwaddstr(main_win, row+2, 1, Swife);
	if (strlen(Swbirt) > 5)
		mvwaddstr(main_win, row+3, 1, Swbirt);
	else if (strlen(Swdeat) > 5)
		mvwaddstr(main_win, row+3, 1, Swdeat);
	else
		mvwaddstr(main_win, row+3, 1, Swbirt);

	mvwaddstr(main_win, row+4, 1, Smarr);
	for (i = 0; i < Solen && i < hgt-5; i++)
		mvwaddstr(main_win, row+5+i, 1, Sothers[i]+1);
	listbadkeys = 0;
	if(badkeylist[0]) {
		sprintf(buf, "WARNING: missing keys: %.40s", badkeylist);
		message(buf);
	}
}
#endif /* UNUSED_CODE */
/*================================================
 * show_ancestors -- Show pedigree/ancestors
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_ancestors (NODE indi, INT row, INT hgt, BOOLEAN reuse)
{
	wipe_window(main_win, row, hgt);
	pedigree_draw_ancestors(indi, row, hgt, reuse);
}
/*================================================
 * show_descendants -- Show pedigree/descendants
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_descendants (NODE indi, INT row, INT hgt, BOOLEAN reuse)
{
	wipe_window(main_win, row, hgt);
	pedigree_draw_descendants(indi, row, hgt, reuse);
}
/*================================================
 * wipe_window -- Clear window
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
static void
wipe_window (WINDOW * w, INT row, INT hgt)
{
	INT i;
	for (i = row; i <= row+hgt-1; i++) {
		wmove(w, i, 1);
		wclrtoeol(w);
#ifndef BSD
		mvwaddch(w, i, ll_cols-1, ACS_VLINE);
#endif
	}
}
/*================================================
 * show_gedcom -- Clear window & didsplay node in 
 *  gedcom format
 * Created: 2001/01/27, Perry Rapp
 *==============================================*/
static void
show_gedcom (WINDOW *w, NODE node, INT gdvw, INT row, INT hgt, BOOLEAN reuse)
{
	wipe_window(w, row, hgt);
	pedigree_draw_gedcom(node, gdvw, row, hgt, reuse);
}
/*================================================
 * show_gedcom_main -- Show node in gedcom format
 *  in window main_win
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_gedcom_main (NODE node, INT gdvw, INT row, INT hgt, BOOLEAN reuse)
{
	show_gedcom(main_win, node, gdvw, row, hgt, reuse);
}
/*================================================
 * switch_scrolls -- Interchange scroll1 & scroll2
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
switch_scrolls (void)
{
	INT save = Scroll1;
	Scroll1 = Scroll2;
	Scroll2 = save;
}
/*===============================================================
 * indi_to_ped_fix -- Construct person STRING for pedigree screen
 * returns static buffer
 *=============================================================*/
STRING
indi_to_ped_fix (NODE indi,
                 INT len)
{
	STRING bevt, devt, name, key;
	static unsigned char scratch[100];
	unsigned char tmp1[100];
	TRANTABLE ttd = tran_tables[MINDS];

	if (!indi) return (STRING) "------------";
	bevt = event_to_date(BIRT(indi), ttd, TRUE);
	if (!bevt) bevt = event_to_date(BAPT(indi), ttd, TRUE);
	if (!bevt) bevt = (STRING) "";
	devt = event_to_date(DEAT(indi), ttd, TRUE);
	if (!devt) devt = event_to_date(BURI(indi), ttd, TRUE);
	if (!devt) devt = (STRING) "";
	if (keyflag) {
	    	key = key_of_record(indi);
		sprintf(tmp1, " [%s-%s] (%s)", bevt, devt, key);
	}
	else
		sprintf(tmp1, " (%s-%s)", bevt, devt);
	name = indi_to_name(indi, ttd, len - strlen(tmp1));
	strcpy(scratch, name);
	strcat(scratch, tmp1);
	return scratch;
}
/*=============================================
 * append_event -- Add an event if present to output string
 *  pstr:  [in,out] end of printed event string 
 *  ttd:   [in] translation table to use for data
 *  indi:  [in] whom to display
 *  plen:  [in,out] max length of output
 * If event found, this prints it
 *  & advances *pstr & reduces *plen
 * Does nothing if event not found
 * returns FALSE if (*plen)<minlen after advancing
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static BOOLEAN
append_event (STRING * pstr, STRING evt, INT * plen, INT minlen)
{
	llstrcatn(pstr, ", ", plen);
	llstrcatn(pstr, evt, plen);
	return *plen >= minlen;
}
/*=============================================
 * family_events -- Print string of events
 *  outstr: [in,out] printed event string 
 *  ttd:    [in] translation table to use for data
 *  indi:   [in] whom to display
 *  fam:    [in] family record (used when displaying spouses)
 *  len:    [in] max length of output
 * If none are found, this will write a 0 to first char of outstr
 * If anything written, starts with ", "
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static void
family_events (STRING outstr, TRANTABLE ttd, NODE indi, NODE fam, INT len)
{
	STRING evt = NULL;
	STRING p = outstr;
	INT mylen = len;
	p[0] = 0;
	evt = sh_fam_to_event(fam, ttd, "MARR", "m. ", mylen, TRUE);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	if (!opt_nocb) {
		NODE chld;
		if ((chld = fam_to_first_chil(fam))) {
			evt = sh_indi_to_event(chld, ttd, "BIRT", dspa_chbr, mylen, TRUE);
			if (evt && !append_event(&p, evt, &mylen, 10))
				return;
			evt = sh_indi_to_event(chld, ttd, "CHR", dspa_chbr, mylen, TRUE);
			if (evt && !append_event(&p, evt, &mylen, 10))
				return;
		}
	}
	evt = sh_indi_to_event(indi, ttd, "BIRT", "b. ", mylen, TRUE);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event(indi, ttd, "CHR", "bap. ", mylen, TRUE);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event(indi, ttd, "DEAT", "d. ", mylen, TRUE);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event(indi, ttd, "BURI", "bur. ", mylen, TRUE);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
}
/*=============================================
 * indi_events -- Print string of events
 *  outstr: [in,out] printed event string 
 *  tt:     [in] translation table to use for data
 *  indi:   [in] whom to display
 *  len:    [in] max length of output
 * If none are found, this will write a 0 to first char of outstr
 * If anything written, starts with ", "
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static void
indi_events (STRING outstr, TRANTABLE ttd, NODE indi, INT len)
{
	STRING evt = NULL;
	INT width = (len-2)/2;
	STRING p = outstr;
	INT mylen = len;
	p[0] = 0;

	evt = sh_indi_to_event(indi, ttd, "BIRT", "b. ", width, TRUE);
	if (!evt)
		evt = sh_indi_to_event(indi, ttd, "CHR", "bap. ", width, TRUE);
	if (evt) {
		llstrcatn(&p, ", ", &mylen);
		llstrcatn(&p, evt, &mylen);
	}
	if (p == outstr)
		width = len;
	evt = sh_indi_to_event(indi, ttd, "DEAT", "d. ", width, TRUE);
	if (!evt) evt = sh_indi_to_event(indi, ttd, "BURI", "bur. ", width,  TRUE);
	if (evt) {
		llstrcatn(&p, ", ", &mylen);
		llstrcatn(&p, evt, &mylen);
	}
}
/*=============================================
 * person_display -- Create person display line
 *  indi:  [in] whom to display
 *  fam:   [in] family record (used when displaying spouses)
 *  len:   max length of output
 *===========================================*/
static STRING
person_display (NODE indi, NODE fam, INT len)
{
	static unsigned char scratch1[120];
	static unsigned char scratch2[100];
	STRING p;
	TRANTABLE ttd = tran_tables[MINDS];
	/* 10 for key, 2 for comma space, and split between name & events */
	INT evlen = (len-12)/2;
	INT namelen;

	if (!indi) return NULL;

	if (evlen > sizeof(scratch2)/sizeof(scratch2[0])-1)
		evlen = sizeof(scratch2)/sizeof(scratch2[0])-1;
	if (fam) {
		family_events(scratch2, ttd, indi, fam, evlen);
	} else {
		indi_events(scratch2, ttd, indi, evlen);
	}
	namelen = len - strlen(scratch2);
	p = scratch1;
	strcpy(p, indi_to_name(indi, ttd, namelen));
	p += strlen(p);
	if (scratch2[0]) {
		strcpy(p, scratch2);
		p += strlen(p);
	}
	sprintf(p, " (%s)", key_of_record(indi));
	return scratch1;
}

static STRING empstr = (STRING) "                                                 ";
/*==========================================
 * show_list - Show name list in list screen
 *========================================*/
#define VIEWABLE 13
void
show_list (INDISEQ seq,
           INT top,
           INT cur,
           INT mark)
{
	WINDOW *win = main_win;
	INT i, j, row, len = length_indiseq(seq);
	STRING key, name;
	NODE indi;
	char scratch[200], *p;
	TRANTABLE ttd = tran_tables[MINDS];

	for (i = LIST_LINES+2; i < LIST_LINES+2+VIEWABLE; i++)
		mvwaddstr(win, i, 1, empstr);
	row = LIST_LINES+2;
	for (i = top, j = 0; j < VIEWABLE && i < len; i++, j++) {
		element_indiseq(seq, i, &key, &name);
		indi = key_to_indi(key);
		if (i == mark) mvwaddch(win, row, 2, 'x');
		if (i == cur) {
			mvwaddch(win, row, 3, '>');
			show_person_main(indi, 1, LIST_LINES, FALSE);
		}
		name = manip_name(name, ttd, TRUE, TRUE, 40);
		strcpy(scratch, name);
		p = scratch + strlen(scratch);
		*p++ = ' ';
		sprintf(p, "(%s)", key_of_record(indi));
		/*sprintf(p, "(%s)", &key[1]);*/
		mvwaddstr(win, row, 4, scratch);
		row++;
	}
}
/*========================================================
 * show_aux_display -- Show source, event or other record
 *======================================================*/
void
show_aux_display (NODE node, INT mode, INT hgt, BOOLEAN reuse)
{
	if (mode == 'g')
		show_gedcom_main(node, GDVW_NORMAL, 1, hgt, reuse);
	else if (mode == 't')
		show_gedcom_main(node, GDVW_TEXT, 1, hgt, reuse);
	else
		show_gedcom_main(node, GDVW_EXPANDED, 1, hgt, reuse);
}
/*===============================================
 * show_scroll - vertically scroll person display
 *=============================================*/
void
show_scroll (INT delta)
{
	Scroll1 += delta;
	if (Scroll1 < 0)
		Scroll1 = 0;
	pedigree_scroll(delta);
}
/*===================================
 * show_scroll2 - scroll lower window
 *  (in tandem mode)
 *=================================*/
void
show_scroll2 (INT delta)
{
	Scroll2 += delta;
	if (Scroll2 < 0)
		Scroll2 = 0;
}
/*=================================
 * show_reset_scroll - clear scroll
 *===============================*/
void
show_reset_scroll (void)
{
	Scroll1 = 0;
	Scroll2 = 0;
	pedigree_reset_scroll();
}
/*=====================================
 * put_out_line - move string to screen
 * but also append + at end if requested
 *====================================*/
void
put_out_line (WINDOW * win, INT x, INT y, STRING string, INT width, INT flag)
{
	if (!flag)
	{
		mvwaddstr(win, x, y, string);
	}
	else
	{
		INT i, pos;
		LINESTRING linebuffer = (LINESTRING)stdalloc(width);
		char * ptr = &linebuffer[0];
		int mylen=width;
		ptr[0] = 0;
		llstrcatn(&ptr, string, &mylen);
		i = strlen(linebuffer);
		pos = width-4;
		if (i>pos)
			i = pos;
		for (; i<pos; i++)
			linebuffer[i] = ' ';
		linebuffer[i++] = '+';
		linebuffer[i++] = '+';
		linebuffer[i++] = '\0';
		mvwaddstr(win, x, y, linebuffer);
		stdfree(linebuffer);
	}
}
/*==================================================================
 * show_childnumbers() - toggle display of numbers for children
 *================================================================*/
void
show_childnumbers (void)
{
	number_child_enable = !number_child_enable;
}
/*================================
 * cache_stats -- Show cache stats
 *==============================*/
void
display_cache_stats (void)
{
	STRING stats = get_cache_stats();
	mprintf_info(stats);
}
/*===============================================
 * init_disp_reformat -- Initialize reformatting for display
 * Created: 2001/07/12 (Perry Rapp)
 *=============================================*/
static void
init_disp_reformat ()
{
	disprfmt.rfmt_date = &disp_format_date;
}
/*================================================
 * display_date -- Convert date according to options
 *==============================================*/
static STRING
disp_format_date (STRING date)
{
	static unsigned char buffer[MAXLINELEN+1];
	if (!date) return NULL;
	if (!lloptions.date_customize_long) return date;
	return format_date(date, lloptions.date_long_dfmt
		, lloptions.date_long_mfmt, lloptions.date_long_yfmt
		, lloptions.date_long_sfmt, TRUE);
}
/*================================================
 * sh_indi_to_event -- Pass-thru to indi_to_event
 *  using display reformatting
 *==============================================*/
static STRING
sh_indi_to_event (NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len, BOOLEAN shrt)
{
	return indi_to_event(node, tt, tag, head, len, shrt, &disprfmt);
}
/*================================================
 * sh_fam_to_event -- Pass-thru to fam_to_event
 *  using display reformatting
 *==============================================*/
static STRING
sh_fam_to_event (NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len, BOOLEAN shrt)
{
	return fam_to_event(node, tt, tag, head, len, shrt, &disprfmt);
}
