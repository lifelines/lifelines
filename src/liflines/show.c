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
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "cache.h"
#include "liflines.h"
#include "lloptions.h"
#include "date.h"

#include "llinesi.h"
#include "screen.h"

/*********************************************
 * global/exported variables
 *********************************************/

struct rfmt_s disp_long_rfmt; /* reformatting used for display long forms */
struct rfmt_s disp_shrt_rfmt; /* reformatting used for display short forms */
INT Scroll1=0;

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN opt_nocb;	/* TRUE to suppress display of cb. data */
extern INT LIST_LINES;		/* person info display lines above list */
extern INT MAINWIN_WIDTH;
extern INT listbadkeys;
extern char badkeylist[];
extern STRING qSmisskeys;
extern STRING qSdspl_indi,qSdspl_fath,qSdspl_moth,qSdspl_spouse,qSdspl_child;
extern STRING qSdspa_resi,qSdspa_div;
extern STRING qSdspa_mar,qSdspa_bir,qSdspa_chr,qSdspa_dea,qSdspa_bur,qSdspa_chbr;
extern STRING qSdspl_mar,qSdspl_bir,qSdspl_chr,qSdspl_dea,qSdspl_bur;

/*********************************************
 * local types
 *********************************************/

typedef char *LINESTRING;

/*********************************************
 * local enums & defines
 *********************************************/

/* to handle large families, this needs to be made a regular
variable, and checked & resized at init_display_indi time */
#define MAXOTHERS 30

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_child_line(INT, NODE, INT width);
static void add_spouse_line(INT, NODE, NODE, INT width);
static BOOLEAN append_event(STRING * pstr, STRING evt, INT * plen, INT minlen);
static STRING disp_long_format_date(STRING date);
static STRING disp_shrt_format_date(STRING date);
static STRING disp_shrt_format_plac(STRING plac);
static void family_events(STRING outstr, TRANTABLE tt, NODE indi, NODE fam, INT len);
static void indi_events(STRING outstr, TRANTABLE tt, NODE indi, INT len);
static void init_disp_reformat(void);
static void init_display_indi(NODE, INT width);
static void init_display_fam(NODE, INT width);
static void pedigree_line(CANVASDATA canvas, INT x, INT y, STRING string, INT overflow);
static STRING person_display(NODE, NODE, INT);
static void put_out_line(UIWINDOW uiwin, INT x, INT y, STRING string, INT maxcol, INT flag);
static STRING sh_fam_to_event_shrt(NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len);
static STRING sh_indi_to_event_long(NODE node, TRANTABLE tt, STRING tag
	, STRING head, INT len);
static STRING sh_indi_to_event_shrt(NODE node, TRANTABLE tt, STRING tag
	, STRING head, INT len);

/*********************************************
 * local variables
 *********************************************/

static LINESTRING Spers, Sbirt, Sdeat, Sfath, Smoth, Smarr;
static LINESTRING Shusb, Shbirt, Shdeat, Swife, Swbirt, Swdeat;
static LINESTRING Sothers[MAXOTHERS];
static INT liwidth;
static INT Solen = 0;
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
init_show_module (void)
{
	INT i;
	liwidth = ll_cols+1;

	Spers = (LINESTRING)stdalloc(liwidth);
	Sbirt = (LINESTRING)stdalloc(liwidth);
	Sdeat = (LINESTRING)stdalloc(liwidth);
	Sfath = (LINESTRING)stdalloc(liwidth);
	Smoth = (LINESTRING)stdalloc(liwidth);
	Smarr = (LINESTRING)stdalloc(liwidth);
	Shusb = (LINESTRING)stdalloc(liwidth);
	Shbirt = (LINESTRING)stdalloc(liwidth);
	Shdeat = (LINESTRING)stdalloc(liwidth);
	Swife = (LINESTRING)stdalloc(liwidth);
	Swbirt = (LINESTRING)stdalloc(liwidth);
	Swdeat = (LINESTRING)stdalloc(liwidth);
	for (i=0; i<MAXOTHERS; i++)
		Sothers[i] = (LINESTRING)stdalloc(liwidth);
	init_disp_reformat();
}
/*===============================================
 * term_show_module -- Free memory used by show module
 *=============================================*/
void
term_show_module (void)
{
	INT i;
	stdfree(Spers);
	stdfree(Sbirt);
	stdfree(Sdeat);
	stdfree(Sfath);
	stdfree(Smoth);
	stdfree(Smarr);
	stdfree(Shusb);
	stdfree(Shbirt);
	stdfree(Shdeat);
	stdfree(Swife);
	stdfree(Swbirt);
	stdfree(Swdeat);
	for (i=0; i<MAXOTHERS; i++)
		stdfree(Sothers[i]);
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
	snprintf(Spers, liwidth, "%s: %s ", _(qSdspl_indi), s);
	if((num = strlen(s)) < width-30) {
	    t = indi_to_title(pers, ttd, width-20 - num - 3);
	    if(t) sprintf(Spers+strlen(Spers), "[%s] ", t);
	}
	sprintf(Spers+strlen(Spers), "(%s)", key_of_record(pers, ttd));

	s = sh_indi_to_event_long(pers, ttd, "BIRT", _(qSdspl_bir), (width-3));
	if (!s) s = sh_indi_to_event_long(pers, ttd, "CHR", _(qSdspl_chr), (width-3));
	if (s) sprintf(Sbirt, "  %s", s);
	else sprintf(Sbirt, "  %s", _(qSdspl_bir));

	/* add a RESIdence if none was in the birth event */
	if(strchr(Sbirt, ',') == 0) {
		num = strlen(Sbirt);
		if(num < width-30) {
			s = sh_indi_to_event_long(pers, ttd, "RESI", _(qSdspa_resi)
				, (width-3)-num-5);
			if(s) {
				if(num < 8) strcat(Sbirt, s+1);
				else {
					/* overwrite the trailing "." on the birth info */
					strcpy(Sbirt + strlen(Sbirt)-1, s);
				}
			}
		}
	}

	s = sh_indi_to_event_long(pers, ttd, "DEAT", _(qSdspl_dea), (width-3));
	if (!s) s = sh_indi_to_event_long(pers, ttd, "BURI", _(qSdspl_bur), (width-3));
	if (s) sprintf(Sdeat, "  %s", s);
	else sprintf(Sdeat, "  %s", _(qSdspl_dea));

	s = person_display(fth, NULL, width-13);
	if (s) snprintf(Sfath, liwidth, "  %s: %s", _(qSdspl_fath), s);
	else snprintf(Sfath, liwidth, "  %s:", _(qSdspl_fath));

	s = person_display(mth, NULL, width-13);
	if (s) snprintf(Smoth, liwidth, "  %s: %s", _(qSdspl_moth), s);
	else snprintf(Smoth, liwidth, "  %s:", _(qSdspl_moth));

	Solen = 0;
	nsp = nch = 0;
	icel = indi_to_cacheel_old(pers);
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
 * show_indi_vitals -- Display person using
 * the traditional LifeLines vitals format.
 *  uiwin:  [IN] which curses window (usually MAIN_WIN)
 *  pers:   [IN] whom to display
 *  row:    [IN] starting row to draw upon
 *  hgt:    [IN] how many rows to use
 *  width:  [IN] how many columns to use
 *  scroll: [IN] how many rows to skip over at top
 *  reuse:  [IN] flag to avoid recomputing display strings
 * Caller sets reuse flag if it knows that this is the same
 * person displayed last.
 *============================*/
void
show_indi_vitals (UIWINDOW uiwin, NODE pers, LLRECT rect
	, INT *scroll, BOOLEAN reuse)
{
	INT i;
	INT localrow;
	INT overflow;
	WINDOW * win = uiw_win(uiwin);
	INT row = rect->top;
	INT width = rect->right - rect->left + 1;
	INT hgt = rect->bottom - rect->top + 1;

	badkeylist[0] = '\0';
	listbadkeys = 1;
	if (hgt<=0) return;
	if (!reuse)
		init_display_indi(pers, width);
	wipe_window_rect(uiwin, rect);
	if (*scroll) {
		if (*scroll > Solen + 5 - hgt)
			*scroll = Solen + 5 - hgt;
		if (*scroll < 0)
			*scroll = 0;
	}
	/* we keep putting lines out til we run out or exhaust our alloted
	height */
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
		/* the other lines scroll internally, and we
		mark the top one displayed if not the actual top one, and the
		bottom one displayed if not the actual bottom */
		overflow = ((i+1 == hgt-5+ *scroll)&&(i+1 != Solen));
		if (*scroll && (i == *scroll))
			overflow = 1;
		put_out_line(uiwin, localrow+5+i, rect->left, Sothers[i], rect->right, overflow);
	}
	listbadkeys = 0;
	if(badkeylist[0]) {
		char buf[132];
		snprintf(buf, sizeof(buf), "%s: %.40s", _(qSmisskeys), badkeylist);
		message(buf);
	}
}
/*=============================================
 * add_spouse_line -- Add spouse line to others
 *===========================================*/
static void
add_spouse_line (INT num, NODE indi, NODE fam, INT width)
{
	STRING line, ptr=Sothers[Solen];
	INT mylen=liwidth;
	num=num; /* unused */
	if (Solen >= MAXOTHERS) return;
	if (mylen>width) mylen=width;
	llstrcatn(&ptr, " ", &mylen);
	llstrcatn(&ptr, _(qSdspl_spouse), &mylen);
	llstrcatn(&ptr, ": ", &mylen);
	line = person_display(indi, fam, mylen-1);
	llstrcatn(&ptr, line, &mylen);
	++Solen;
}
/*===========================================
 * add_child_line -- Add child line to others
 *=========================================*/
static void
add_child_line (INT num, NODE indi, INT width)
{
	STRING line;
	STRING child = _(qSdspl_child);
	if (Solen >= MAXOTHERS) return;
	line = person_display(indi, NULL, width-15);
	if (number_child_enable)
		snprintf(Sothers[Solen], liwidth, "  %2d%s: %s", num, child, line);
	else
		snprintf(Sothers[Solen], liwidth, "    %s: %s", child, line);
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
	INT len, nch, nm, wtemp;
	TRANTABLE ttd = tran_tables[MINDS];
	STRING mother = _(qSdspl_moth);
	STRING father = _(qSdspl_fath);
	ASSERT(fam);
	husb = fam_to_husb(fam);
	wife = fam_to_wife(fam);
	fk = key_of_record(fam, ttd);
	if (husb) {
		ik = key_of_record(husb, ttd);
		len = liwidth - (10 + strlen(father) + strlen(ik) + strlen(fk));
		s = indi_to_name(husb, ttd, len);
		snprintf(Shusb, liwidth, "%s: %s (%s) (%s)", father, s, ik, fk);
	} else
		snprintf(Shusb, liwidth, "%s: (%s)", father, fk);

	s = sh_indi_to_event_long(husb, ttd, "BIRT", _(qSdspl_bir), width-3);
	if (!s) s = sh_indi_to_event_long(husb, ttd, "CHR", _(qSdspl_chr), width-3);
	if (s) sprintf(Shbirt, "  %s", s);
	else sprintf(Shbirt, "  %s", _(qSdspl_bir));

	s = sh_indi_to_event_long(husb, ttd, "DEAT", _(qSdspl_dea), width-3);
	if (!s) s = sh_indi_to_event_long(husb, ttd, "BURI", _(qSdspl_bur), width-3);
	if (s) snprintf(Shdeat, liwidth, "  %s", s);
	else snprintf(Shdeat, liwidth, "  %s", _(qSdspl_dea));

	if (wife) {
		ik = key_of_record(wife, ttd);
		len = width - (7 + strlen(mother) + strlen(ik));
		s = indi_to_name(wife, ttd, len);
		snprintf(Swife, liwidth, "%s: %s (%s)", mother, s, ik);
	} else
		snprintf(Swife, liwidth, "%s:", mother);

	s = sh_indi_to_event_long(wife, ttd, "BIRT", _(qSdspl_bir), width-3);
	if (!s) s = sh_indi_to_event_long(wife, ttd, "CHR", _(qSdspl_chr), width-3);
	if (s) snprintf(Swbirt, liwidth, "  %s", s);
	else snprintf(Swbirt, liwidth, "  %s", _(qSdspl_bir));

	s = sh_indi_to_event_long(wife, ttd, "DEAT", _(qSdspl_dea), width-3);
	if (!s) s = sh_indi_to_event_long(wife, ttd, "BURI", _(qSdspl_bur), width-3);
	if (s) snprintf(Swdeat, liwidth, "  %s", s);
	else snprintf(Swdeat, liwidth, "  %s", _(qSdspl_dea));

	s = sh_indi_to_event_long(fam, ttd, "MARR", _(qSdspl_mar), width-3);
	if (s) snprintf(Smarr, liwidth, s);
	else snprintf(Smarr, liwidth, _(qSdspl_mar));
	/* append divorce to marriage line, if room */
	/* (Might be nicer to make it a separate, following line */
	wtemp = width-5 - strlen(Smarr);
	if (wtemp > 10) {
		s = sh_indi_to_event_long(fam, ttd, "DIV", _(qSdspa_div), wtemp);
		if (s)
			snprintf(Smarr+strlen(Smarr), liwidth-strlen(Smarr), ", %s", s);
	}

	Solen = 0;
	nch = 0;
	FORCHILDREN(fam, chld, nm)
		add_child_line(++nch, chld, width);
	ENDCHILDREN
}
/*===================================
 * show_fam_vitals -- Display family
 * [in] fam:  whom to display
 * [in] row:   starting row to use
 * [in] hgt:   how many rows allowed
 * [in] width: how many columns allowed
 * [in] reuse: flag to save recalculating display strings
 *=================================*/
void
show_fam_vitals (UIWINDOW uiwin, NODE fam, INT row, INT hgt
	, INT width, INT *scroll, BOOLEAN reuse)
{
	INT i, len;
	INT localrow;
	INT overflow;
	char buf[132];
	INT maxcol = width-1;
	WINDOW * win = uiw_win(uiwin);
	struct llrect_s rect;

	rect.bottom = row+hgt-1;
	rect.top = row;
	rect.left = 1;
	rect.right = width-2;

	badkeylist[0] = '\0';
	listbadkeys = 1;
	if (!reuse)
		init_display_fam(fam, width);
	wipe_window_rect(uiwin, &rect);
	if (*scroll) {
		if (*scroll > Solen + 7 - hgt)
			*scroll = Solen + 7 - hgt;
		if (*scroll < 0)
			*scroll = 0;
	}
	localrow = row - *scroll;
	mvwaddstr(win, row+0, 1, Shusb);
	if (hgt==1) return;
	mvwaddstr(win, row+1, 1, Shbirt);
	if (hgt==2) return;
	mvwaddstr(win, row+2, 1, Shdeat);
	if (hgt==3) return;
	mvwaddstr(win, row+3, 1, Swife);
	if (hgt==4) return;
	mvwaddstr(win, row+4, 1, Swbirt);
	if (hgt==5) return;
	mvwaddstr(win, row+5, 1, Swdeat);
	if (hgt==6) return;
	mvwaddstr(win, row+6, 1, Smarr);
	for (i = *scroll; i < Solen && i < hgt-7+*scroll; i++)
	{
		overflow = ((i+1 == hgt-7+*scroll)&&(i+1 != Solen));
		if (*scroll && (i == *scroll))
			overflow = 1;
		len = strlen(Sothers[i]+1);
		put_out_line(uiwin, localrow+7+i, 1, Sothers[i]+1, maxcol, overflow);
	}
	listbadkeys = 0;
	if(badkeylist[0]) {
		sprintf(buf, "WARNING: missing keys: %.40s", badkeylist);
		message(buf);
	}
}
/*================================================
 * show_ancestors -- Show pedigree/ancestors
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_ancestors (UIWINDOW uiwin, NODE indi, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	struct canvasdata_s canvas;
		/* parameters for drawing tree */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(main_win, rect);
	pedigree_draw_ancestors(indi, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * show_descendants -- Show pedigree/descendants
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_descendants (UIWINDOW uiwin, NODE indi, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	struct canvasdata_s canvas;
		/* parameters for drawing tree */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(main_win, rect);
	pedigree_draw_descendants(indi, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * show_gedcom -- Display record in raw gedcom format
 * Created: 2001/01/27, Perry Rapp
 *==============================================*/
void
show_gedcom (UIWINDOW uiwin, NODE node, INT gdvw, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	struct canvasdata_s canvas;
		/* parameters for drawing */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(uiwin, rect);
	pedigree_draw_gedcom(node, gdvw, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * switch_scrolls -- Interchange scroll1 & scroll2
 * This is how the tandem modes do their drawing of
 * the lower part -- they swap in the second set of
 * scroll briefly for displaying the lower part, then
 * swap back to normal as soon as finishing lower part.
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
 *  indi: [in] person to display
 *  len:  [in] max width
 * Does internal-to-display translation
 *=============================================================*/
STRING
indi_to_ped_fix (NODE indi, INT len)
{
	STRING bevt, devt, name, key;
	static char scratch[100];
	char tmp1[100];
	TRANTABLE ttd = tran_tables[MINDS];

/*	return person_display(indi, 0, len); */

	if (!indi) return (STRING) "------------";
	bevt = event_to_date(BIRT(indi), ttd, TRUE);
	if (!bevt) bevt = event_to_date(BAPT(indi), ttd, TRUE);
	if (!bevt) bevt = (STRING) "";
	devt = event_to_date(DEAT(indi), ttd, TRUE);
	if (!devt) devt = event_to_date(BURI(indi), ttd, TRUE);
	if (!devt) devt = (STRING) "";
	if (keyflag) {
		key = key_of_record(indi, ttd);
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
 *  pstr:   [I/O] end of printed event string 
 *  evt:    [IN]  event string (must be valid)
 *  plen:   [I/O] max length of output
 *  minlen: [IN]  threshold for caller to stop
 * If event found, this prints it, advances *pstr, and reduces *plen
 * returns FALSE if (*plen)<minlen after advancing
 *  (signal to caller to stop)
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
	evt = sh_fam_to_event_shrt(fam, ttd, "MARR", _(qSdspa_mar), mylen);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
/*
	mylen is up-to-date how many chars left we have
	(we keep passing mylen-2 because events are prefixed with ", ")
	if we ever have too few left (<10), append_event will return FALSE
*/
	if (!opt_nocb) {
		NODE chld;
		/* Look for birth or christening of first child */
		if ((chld = fam_to_first_chil(fam))) {
			evt = sh_indi_to_event_shrt(chld, ttd, "BIRT", _(qSdspa_chbr), mylen-2);
			if (evt && !append_event(&p, evt, &mylen, 10))
				return;
			if (!evt) {
				evt = sh_indi_to_event_shrt(chld, ttd, "CHR", _(qSdspa_chbr), mylen-2);
				if (evt && !append_event(&p, evt, &mylen, 10))
					return;
			}
		}
	}
	evt = sh_indi_to_event_shrt(indi, ttd, "BIRT", _(qSdspa_bir), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, ttd, "CHR", _(qSdspa_chr), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, ttd, "DEAT", _(qSdspa_dea), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, ttd, "BURI", _(qSdspa_bur), mylen-2);
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

	evt = sh_indi_to_event_shrt(indi, ttd, "BIRT", _(qSdspa_bir), width);
	if (!evt)
		evt = sh_indi_to_event_shrt(indi, ttd, "CHR", _(qSdspa_chr), width);
	if (evt) {
		llstrcatn(&p, ", ", &mylen);
		llstrcatn(&p, evt, &mylen);
	}
	if (p == outstr)
		width = len;
	evt = sh_indi_to_event_shrt(indi, ttd, "DEAT", _(qSdspa_dea), width);
	if (!evt) evt = sh_indi_to_event_shrt(indi, ttd, "BURI", _(qSdspa_bur), width);
	if (evt) {
		llstrcatn(&p, ", ", &mylen);
		llstrcatn(&p, evt, &mylen);
	}
}
/*==========================================================
 * max_keywidth -- Figure the width of the widest extant key
 *========================================================*/
static INT
max_keywidth (void)
{
	INT maxkey = xref_max_any();
	if (maxkey>9999) {
		if (maxkey>999999)
			return 7;
		if (maxkey>99999)
			return 6;
		return 5;
	}
	if (maxkey>999)
		return 4;
	if (maxkey>99)
		return 3;
	return 2;
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
	static char scratch1[120];
	static char scratch2[100];
	STRING p;
	TRANTABLE ttd = tran_tables[MINDS];
	INT keyspace = max_keywidth() + 3; /* parentheses & leading space */
	INT evlen, namelen, temp;
	/* don't overflow scratch1, into which we catenate name & events */
	if (len > ARRSIZE(scratch1)-1)
		len = ARRSIZE(scratch1)-1;

	/* keywidth for key, 2 for comma space, and split between name & events */
	evlen = (len-2-keyspace)/2;
	namelen = evlen;

	if (!indi) return NULL;

	/* test to see if name is short */
	p = indi_to_name(indi, ttd, 100);
	if ((temp = strlen(p)) < evlen) {
		/* name is short, give extra to events */
		evlen += (namelen - temp);
		namelen -= (namelen - temp);
	}

	if (evlen > ARRSIZE(scratch2)-1) /* don't overflow name buffer */
		evlen = ARRSIZE(scratch2)-1;
	if (fam) {
		family_events(scratch2, ttd, indi, fam, evlen);
	} else {
		indi_events(scratch2, ttd, indi, evlen);
	}

	/* give name any unused space events left */
	if ((INT)strlen(scratch2)<evlen)
		namelen += evlen-(INT)strlen(scratch2);
	p = scratch1;
	strcpy(p, indi_to_name(indi, ttd, namelen));
	p += strlen(p);
	if (scratch2[0]) {
		strcpy(p, scratch2);
		p += strlen(p);
	}
	sprintf(p, " (%s)", key_of_record(indi, ttd));
	return scratch1;
}
/*========================================================
 * show_aux -- Show source, event or other record
 *======================================================*/
void
show_aux (UIWINDOW uiwin, NODE node, INT mode, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	if (mode == 'g')
		show_gedcom(uiwin, node, GDVW_NORMAL, rect, scroll, reuse);
	else if (mode == 't')
		show_gedcom(uiwin, node, GDVW_TEXT, rect, scroll, reuse);
	else
		show_gedcom(uiwin, node, GDVW_EXPANDED, rect, scroll, reuse);
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
}
/*=====================================
 * pedigree_line - callback from pedigree code
 *  to put out each line of pedigree
 *====================================*/
static void
pedigree_line (CANVASDATA canvas, INT y, INT x, STRING string, INT overflow)
{
	if (!string || !string[0]) return;
	/* vertical clip to rect */
	if (y < canvas->rect->top || y > canvas->rect->bottom) return;
	/* horizontal clip to rect */
	if (x < canvas->rect->left) {
		INT delta = canvas->rect->left - x;
		if ((INT)strlen(string) <= delta) return;
		string += delta;
		x = canvas->rect->left;
	}
	put_out_line((UIWINDOW)canvas->param, y, x, string, canvas->rect->right, overflow);
}
/*=====================================
 * put_out_line - move string to screen
 * but also append ++ at end if flagged
 * start string at x,y, and do not go beyond maxcol
 *====================================*/
static void
put_out_line (UIWINDOW uiwin, INT y, INT x, STRING string, INT maxcol, INT flag)
{
	WINDOW * win = uiw_win(uiwin);
	static LINESTRING buffer=0; /* local buffer resized when needed */
	static INT buflen=0;
	INT maxlen = maxcol - x + 1;
	/* ensure enough room in buffer */
	if (!buflen || buflen < maxlen+1) {
		if (buffer) stdfree(buffer);
		buflen = maxlen+1;
		buffer = (LINESTRING)stdalloc(buflen);
	}
	/* copy into local buffer (here we enforce maxcol) */
	llstrncpy(buffer, string, buflen);
	if (flag) {
		/* put ++ against right, padding if needed */
		INT i = strlen(buffer);
		INT pos = maxcol-x-3;
		if (i>pos)
			i = pos;
		for (; i<pos; i++)
			buffer[i] = ' ';
		buffer[i++] = ' ';
		buffer[i++] = '+';
		buffer[i++] = '+';
		buffer[i++] = '\0';
	}
	mvwaddstr(win, y, x, buffer);
}
/*==================================================================
 * show_childnumbers - toggle display of numbers for children
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
	msg_info(stats);
}
/*===============================================
 * init_disp_reformat -- Initialize reformatting for display
 * Set up format descriptions for both long & short display forms
 * Created: 2001/07/12 (Perry Rapp)
 *=============================================*/
static void
init_disp_reformat (void)
{
	/* Set up long formats */
	memset(&disp_long_rfmt, 0, sizeof(disp_long_rfmt));
	disp_long_rfmt.rfmt_date = &disp_long_format_date;
	disp_long_rfmt.rfmt_plac = 0; /* use place as is */
	disp_long_rfmt.combopic = "%1, %2";
	/* Set up short formats */
	memset(&disp_shrt_rfmt, 0, sizeof(disp_shrt_rfmt));
	disp_shrt_rfmt.rfmt_date = &disp_shrt_format_date;
	disp_shrt_rfmt.rfmt_plac = &disp_shrt_format_plac;
	disp_shrt_rfmt.combopic = "%1, %2";
}
/*===========================================================
 * disp_long_format_date -- Convert date according to options
 *=========================================================*/
static STRING
disp_long_format_date (STRING date)
{
	INT dfmt=0,mfmt=0,yfmt=0,sfmt=0,efmt=0, cmplx;
	INT n;
	STRING fmts, pic;

	if (!date) return NULL;

	n = 0;
	fmts = getoptstr("LongDisplayDate", NULL);
	if (fmts) {
		/* try to use user-specified format */
		n = sscanf(fmts, "%d,%d,%d,%d,%d,%d" 
			, &dfmt, &mfmt, &yfmt, &sfmt, &efmt, &cmplx);
	}
	if (n != 6) {
		dfmt=mfmt=yfmt=sfmt=efmt=cmplx=0;
		sfmt=14; /* GEDCOM as is */
	}

	pic = getoptstr("LongDisplayDatePic", NULL);
	if (pic && pic[0])
		set_date_pic(pic);
	
	return do_format_date(date, dfmt, mfmt, yfmt, sfmt, efmt, cmplx);
}
/*===============================================================
 * disp_shrt_format_date -- short form of date for display
 *  This is used for dates in option strings, and in single-line
 *  descriptions of people (ie, in event summaries).
 * Created: 2001/10/29 (Perry Rapp)
 *=============================================================*/
static STRING
disp_shrt_format_date (STRING date)
{
	INT dfmt=0,mfmt=0,yfmt=0,sfmt=0,efmt=0, cmplx;
	INT n;
	STRING fmts, pic;

	if (!date) return NULL;

	n = 0;
	fmts = getoptstr("ShortDisplayDate", NULL);
	if (fmts) {
		/* try to use user-specified format */
		n = sscanf(fmts, "%d,%d,%d,%d,%d,%d"
			, &dfmt, &mfmt, &yfmt, &sfmt, &efmt, &cmplx);
	}
	if (n != 6) {
		dfmt=mfmt=yfmt=sfmt=cmplx=0;
		sfmt=12; /* old style short form -- year only */
	}

	pic = getoptstr("ShortDisplayDatePic", NULL);
	if (pic && pic[0])
		set_date_pic(pic);

	return do_format_date(date, dfmt, mfmt, yfmt, sfmt, efmt, cmplx);
}
/*================================================================
 * disp_shrt_format_plac -- short form of place for display
 *  This is used for places in single-line descriptions of people
 *  (ie, in event summaries).
 * Created: 2001/10/29 (Perry Rapp)
 *==============================================================*/
static STRING
disp_shrt_format_plac (STRING plac)
{
	if (!plac) return NULL;
	return shorten_plac(plac);
}
/*================================================
 * sh_indi_to_event -- Pass-thru to indi_to_event
 *  using long display reformatting
 *==============================================*/
static STRING
sh_indi_to_event_long (NODE node, TRANTABLE tt, STRING tag
	, STRING head, INT len)
{
	return indi_to_event(node, tt, tag, head, len, &disp_long_rfmt);
}
/*================================================
 * sh_indi_to_event_shrt -- Pass-thru to indi_to_event, short display
 *  using short display reformatting
 *==============================================*/
static STRING
sh_indi_to_event_shrt (NODE node, TRANTABLE tt, STRING tag
	, STRING head, INT len)
{
	return indi_to_event(node, tt, tag, head, len, &disp_shrt_rfmt);
}
/*==================================================
 * sh_fam_to_event_shrt -- Pass-thru to fam_to_event
 *  using display reformatting
 *================================================*/
static STRING
sh_fam_to_event_shrt (NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len)
{
	return fam_to_event(node, tt, tag, head, len, &disp_shrt_rfmt);
}
