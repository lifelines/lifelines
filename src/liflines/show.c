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
#include "zstr.h"

#include "llinesi.h"
#include "screen.h"
#include "cscurses.h"

/*********************************************
 * global/exported variables
 *********************************************/

INT Scroll1=0;

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN opt_nocb;	/* TRUE to suppress display of cb. data */
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
struct tag_prefix {
	STRING tag;
	STRING prefix;
};

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
static void add_child_line(INT, RECORD, INT width);
static void add_spouse_line(INT, NODE, NODE, INT width);
static BOOLEAN append_event(STRING * pstr, STRING evt, INT * plen, INT minlen);
static void disp_person_birthdeath(ZSTR zstr, RECORD irec, struct tag_prefix * tags, RFMT rfmt);
static void disp_person_name(ZSTR zstr, STRING prefix, RECORD irec, INT width);
static void indi_events(STRING outstr, NODE indi, INT len);
static void init_display_indi(RECORD irec, INT width);
static void init_display_fam(RECORD frec, INT width);
static void pedigree_line(CANVASDATA canvas, INT x, INT y, STRING string, INT overflow);
static STRING person_display(NODE, NODE, INT);
static void put_out_line(UIWINDOW uiwin, INT x, INT y, STRING string, INT maxcol, INT flag);
static STRING sh_fam_to_event_shrt(NODE node, STRING tag, STRING head
	, INT len);
static STRING sh_indi_to_event_long(NODE node, STRING tag
	, STRING head, INT len);
static STRING sh_indi_to_event_shrt(NODE node, STRING tag
	, STRING head, INT len);

/*********************************************
 * local variables
 *********************************************/

static LINESTRING Sfath, Smoth, Smarr;
static ZSTR Spers=0, Sbirt=0, Sdeat=0;
static ZSTR Shusb=0, Swife=0;
static ZSTR Shbirt=0, Shdeat=0, Swbirt=0, Swdeat=0;
static LINESTRING Sothers[MAXOTHERS];
static INT liwidth;
static INT Solen = 0;
static INT Scroll2 = 0;
static INT number_child_enable = 0;
static struct tag_prefix f_birth_tags[] = {
	{ "BIRT", N_("born") } /* GEDCOM BIRT tag, label to precede date on display */
	,{ "CHR", N_("bapt") } /* GEDCOM CHR tag, label to precede date on display */
	,{ "BAPM", N_("bapt") } /* GEDCOM BAPM tag, label to precede date on display */
	,{ "BARM", N_("barm") } /* GEDCOM BARM tag, label to precede date on display */
	,{ "BASM", N_("basm") } /* GEDCOM BASM tag, label to precede date on display */
	,{ "BLES", N_("bles") }
	,{ "ADOP", N_("adop") }
	,{ "RESI", N_("resi") }
	,{ NULL, NULL }
};
static struct tag_prefix f_death_tags[] = {
	{ "DEAT", N_("died") }
	,{ "BURI", N_("buri") }
	,{ "CREM", N_("crem") }
	,{ NULL, NULL }
};

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

	Spers = zs_new();
	Sbirt = zs_new();
	Sdeat = zs_new();
	Sfath = (LINESTRING)stdalloc(liwidth);
	Smoth = (LINESTRING)stdalloc(liwidth);
	Smarr = (LINESTRING)stdalloc(liwidth);
	Shusb = zs_new();
	Shbirt = zs_new();
	Shdeat = zs_new();
	Swife = zs_new();
	Swbirt = zs_new();
	Swdeat = zs_new();
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

	zs_free(&Spers);
	zs_free(&Sbirt);
	zs_free(&Sdeat);
	stdfree(Sfath);
	stdfree(Smoth);
	stdfree(Smarr);
	zs_free(&Shusb);
	zs_free(&Shbirt);
	zs_free(&Shdeat);
	zs_free(&Swife);
	zs_free(&Swbirt);
	zs_free(&Swdeat);
	for (i=0; i<MAXOTHERS; i++)
		stdfree(Sothers[i]);
}
/*===============================================
 * disp_person_name -- Display person's name
 *  append REFN or key, & include title if room
 * Created: 2003-01-11 (Perry Rapp)
 *=============================================*/
static void
disp_person_name (ZSTR zstr, STRING prefix, RECORD irec, INT width)
{
/* TODO: width handling is wrong, it should not be byte based */
	ZSTR zkey = zs_news(key_of_record(nztop(irec)));
	/* ": " between prefix and name, and " ()" for key */
	INT avail = width - strlen(prefix)-zs_len(zkey)-5;
	STRING name = indi_to_name(nztop(irec), avail);
	zs_clear(zstr);
	zs_setf(zstr, "%s: %s ", prefix, name);
	avail = width - zs_len(zstr)-zs_len(zkey)-2;
	if (avail > 10) {
		STRING t = indi_to_title(nztop(irec), avail-3);
		if (t) zs_appf(zstr, "[%s] ", t);
	}
/* TODO: add more names if room */
/* TODO: first implement new function namelist to get us all names */
	if(getlloptint("DisplayKeyTags", 0) > 0) {
		zs_appf(zstr, "(i%s)", zs_str(zkey));
	} else {
		zs_appf(zstr, "(%s)", zs_str(zkey));
	}
	zs_free(&zkey);
}
/*===============================================
 * disp_person_birthdeath -- Print birth string
 *  Try to find date & place info for birth (or approx)
 * Created: 2003-01-12 (Perry Rapp)
 *=============================================*/
static void
disp_person_birthdeath (ZSTR zstr, RECORD irec, struct tag_prefix * tags
	, RFMT rfmt)
{
	struct tag_prefix *tg, *tgdate=NULL, *tgplac=NULL;
	STRING date=NULL, plac=NULL, td=NULL, tp=NULL;
	STRING predate=NULL, preplac=NULL;
	ZSTR zdate=zs_new();
	for (tg = tags; tg->tag; ++tg) {
		record_to_date_place(irec, tg->tag, &td, &tp);
		if (!date) {
			date=td;
			tgdate=tg;
		}
		if (!plac) {
			plac=tp;
			tgplac=tg;
		}
		if (date && plac) break;
	}
	zs_sets(zstr, "  ");
	/* prefix display labels */
	if (date) {
		predate = _(tgdate->prefix);
		if (rfmt && rfmt->rfmt_date)
			date = (*rfmt->rfmt_date)(date);
		zs_appf(zdate, "%s: %s", predate, date);
	}
	if (plac) {
		ZSTR zplac=zs_new();
		preplac = _(tgplac->prefix);
		if (predate && eqstr(preplac, predate)) preplac=NULL;
		if (rfmt && rfmt->rfmt_plac)
			plac = (*rfmt->rfmt_plac)(plac);
		if (preplac)
			zs_setf(zplac, "%s: %s", preplac, plac);
		else
			zs_sets(zplac, plac);
		if (zs_len(zdate)) {
			/* have both date & place, so combine them */
			static char scratch1[MAXLINELEN+1];
			sprintpic2(scratch1, sizeof(scratch1), uu8, rfmt->combopic
				, zs_str(zdate), zs_str(zplac));
			zs_apps(zstr, scratch1);
		} else {
			/* have only place, so just append it */
			zs_appz(zstr, zplac);
		}
		zs_free(&zplac);
	} else {
		/* have only date, so just append it */
		zs_apps(zstr, zs_str(zdate));
	}
	if (zs_len(zstr)<3) {
		zs_apps(zstr, _(tags[0].prefix));
		zs_apps(zstr, ": ");
	}
	zs_free(&zdate);
}
/*===============================================
 * init_display_indi -- Initialize display person
 *  Fill in all the local buffers for normal person
 *  display mode (Spers, Sbirt, etc)
 *=============================================*/
static void
init_display_indi (RECORD irec, INT width)
{
	NODE pers=nztop(irec);
	NODE this_fam = 0;
	INT nsp, nch, num, nm;
	STRING s;
	NODE fth;
	NODE mth;
	CACHEEL icel;

	ASSERT(width < ll_cols+1); /* size of Spers etc */


	ASSERT(pers);

	disp_person_name(Spers, _(qSdspl_indi), irec, width);

	disp_person_birthdeath(Sbirt, irec, f_birth_tags, &disp_long_rfmt);

	disp_person_birthdeath(Sdeat, irec, f_death_tags, &disp_long_rfmt);

	fth = indi_to_fath(pers);
	s = person_display(fth, NULL, width-13);
	if (s) llstrncpyf(Sfath, liwidth, uu8, "  %s: %s", _(qSdspl_fath), s);
	else llstrncpyf(Sfath, liwidth, uu8, "  %s:", _(qSdspl_fath));

	mth = indi_to_moth(pers);
	s = person_display(mth, NULL, width-13);
	if (s) llstrncpyf(Smoth, liwidth, uu8, "  %s: %s", _(qSdspl_moth), s);
	else llstrncpyf(Smoth, liwidth, uu8, "  %s:", _(qSdspl_moth));

	Solen = 0;
	nsp = nch = 0;
	icel = indi_to_cacheel_old(pers);
	lock_cache(icel);
	FORFAMSS(pers, fam, sp, num)
		if (sp) add_spouse_line(++nsp, sp, fam, width);
	        if (this_fam != fam) {
		        this_fam = fam; /* only do each family once */
			FORCHILDREN(fam, chld, nm)
				if(chld) add_child_line(++nch, chld, width);
			ENDCHILDREN
		}
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
show_indi_vitals (UIWINDOW uiwin, RECORD irec, LLRECT rect
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
		init_display_indi(irec, width);
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

	mvccwaddnstr(win, row+0, 1, zs_str(Spers), width-1);
	if (hgt==1) return;
	mvccwaddnstr(win, row+1, 1, zs_str(Sbirt), width-1);
	if (hgt==2) return;
	mvccwaddnstr(win, row+2, 1, zs_str(Sdeat), width-1);
	if (hgt==3) return;
	mvccwaddnstr(win, row+3, 1, Sfath, width-1);
	if (hgt==4) return;
	mvccwaddnstr(win, row+4, 1, Smoth, width-1);
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
		llstrncpyf(buf, sizeof(buf), uu8, "%s: %.40s", _(qSmisskeys), badkeylist);
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
add_child_line (INT num, RECORD irec, INT width)
{
	STRING line;
	STRING child = _(qSdspl_child);
	if (Solen >= MAXOTHERS) return;
	line = person_display(nztop(irec), NULL, width-15);
	if (number_child_enable)
		llstrncpyf(Sothers[Solen], liwidth, uu8, "  %2d%s: %s", num, child, line);
	else
		llstrncpyf(Sothers[Solen], liwidth, uu8, "    %s: %s", child, line);
	Sothers[Solen++][width-2] = 0;
}
/*==============================================
 * init_display_fam -- Initialize display family
 *============================================*/
static void
init_display_fam (RECORD frec, INT width)
{
	NODE fam=nztop(frec);
	NODE husb=0, wife=0;
	STRING s=0;
	ZSTR famkey = zs_news(key_of_record(fam));
	INT nch, nm, wtemp;
	STRING father = _(qSdspl_fath);
	STRING mother = _(qSdspl_moth);
	RECORD ihusb=0, iwife=0;
	INT husbstatus = 0;
	INT wifestatus = 0;
	NODE fnode;

	/* Get the first two spouses in the family and use them rather than
	 * displaying first husband and first mother
	 * This causes a more reasonable presentation of non-traditional
	 * familes.  Also it will display first hustband and first wife
	 * for traditional families (as there's only one) and the db routines
	 * insert HUSB records before WIFE records.
	 */
	if (fam) {
		fnode = nchild(fam);
		husbstatus = next_spouse(&fnode,&ihusb);
		husb = nztop(ihusb);
		if (fnode) {
			fnode = nsibling(fnode);
			wifestatus = next_spouse(&fnode,&iwife);
			wife = nztop(iwife);
		}
	}
	/* if the only spouse is female, list in second slot
	 * hiding the non-traditional behavior
	 */
	if (!wife && husb && SEX(husb) == SEX_FEMALE) {
		wife = husb;
		husb = 0;
		iwife = ihusb;
		ihusb = 0;
		wifestatus = husbstatus;
		husbstatus  = 0;
	}

	if (husbstatus == 1) {
		INT avail = width - zs_len(famkey) - 3;
		disp_person_name(Shusb, SEX(husb)==SEX_MALE?father:mother, ihusb, avail);
	} else {
		zs_setf(Shusb, "%s:", father);
		if (husbstatus == -1)
			zs_apps(Shusb, "??");
	}
	if(getlloptint("DisplayKeyTags", 0) > 0) {
		zs_appf(Shusb, " (f%s)", zs_str(famkey));
	} else {
		zs_appf(Shusb, " (%s)", zs_str(famkey));
	}
	zs_free(&famkey);

	disp_person_birthdeath(Shbirt, ihusb, f_birth_tags, &disp_long_rfmt);
	disp_person_birthdeath(Shdeat, ihusb, f_death_tags, &disp_long_rfmt);

	if (wifestatus == 1) {
		INT avail = width;
		disp_person_name(Swife, SEX(wife)==SEX_MALE?father:mother, iwife, avail);
	} else {
		zs_setf(Swife, "%s:", mother);
		if (wifestatus == -1)
			zs_apps(Swife, "??");
	}

	disp_person_birthdeath(Swbirt, iwife, f_birth_tags, &disp_long_rfmt);
	disp_person_birthdeath(Swdeat, iwife, f_death_tags, &disp_long_rfmt);

	s = sh_indi_to_event_long(fam, "MARR", _(qSdspl_mar), width-3);
	if (s) llstrncpyf(Smarr, liwidth, uu8, s);
	else llstrncpyf(Smarr, liwidth, uu8, _(qSdspl_mar));
	/* append divorce to marriage line, if room */
	/* (Might be nicer to make it a separate, following line */
	wtemp = width-5 - strlen(Smarr);
	if (wtemp > 10) {
		s = sh_indi_to_event_long(fam, "DIV", _(qSdspa_div), wtemp);
		if (s)
			llstrncpyf(Smarr+strlen(Smarr), liwidth-strlen(Smarr), uu8, ", %s", s);
	}

	Solen = 0;
	nch = 0;
	FORCHILDREN(fam, chld, nm)
		add_child_line(++nch, chld, width);
	ENDCHILDREN
	release_record(ihusb);
	release_record(iwife);
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
show_fam_vitals (UIWINDOW uiwin, RECORD frec, INT row, INT hgt
	, INT width, INT *scroll, BOOLEAN reuse)
{
	INT i, len;
	INT localrow;
	INT overflow;
	char buf[132];
	INT maxcol = width-1;
	WINDOW * win = uiw_win(uiwin);
	struct tag_llrect rect;

	rect.bottom = row+hgt-1;
	rect.top = row;
	rect.left = 1;
	rect.right = width-2;

	badkeylist[0] = '\0';
	listbadkeys = 1;
	if (!reuse)
		init_display_fam(frec, width);
	wipe_window_rect(uiwin, &rect);
	if (*scroll) {
		if (*scroll > Solen + 7 - hgt)
			*scroll = Solen + 7 - hgt;
		if (*scroll < 0)
			*scroll = 0;
	}
	localrow = row - *scroll;
	mvccwaddnstr(win, row+0, 1, zs_str(Shusb), width-2);
	if (hgt==1) return;
	mvccwaddnstr(win, row+1, 1, zs_str(Shbirt), width-2);
	if (hgt==2) return;
	mvccwaddnstr(win, row+2, 1, zs_str(Shdeat), width-2);
	if (hgt==3) return;
	mvccwaddnstr(win, row+3, 1, zs_str(Swife), width-2);
	if (hgt==4) return;
	mvccwaddnstr(win, row+4, 1, zs_str(Swbirt), width-2);
	if (hgt==5) return;
	mvccwaddnstr(win, row+5, 1, zs_str(Swdeat), width-2);
	if (hgt==6) return;
	mvccwaddnstr(win, row+6, 1, Smarr, width-2);
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
show_ancestors (UIWINDOW uiwin, RECORD irec, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	struct tag_canvasdata canvas;
		/* parameters for drawing tree */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(main_win, rect);
	pedigree_draw_ancestors(irec, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * show_descendants -- Show pedigree/descendants
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_descendants (UIWINDOW uiwin, RECORD rec, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	struct tag_canvasdata canvas;
		/* parameters for drawing tree */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(main_win, rect);
	pedigree_draw_descendants(rec, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * show_gedcom -- Display record in raw gedcom format
 * Created: 2001/01/27, Perry Rapp
 *==============================================*/
void
show_gedcom (UIWINDOW uiwin, RECORD rec, INT gdvw, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	struct tag_canvasdata canvas;
		/* parameters for drawing */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(uiwin, rect);
	pedigree_draw_gedcom(rec, gdvw, &canvas, reuse);
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

/*	return person_display(indi, 0, len); */

	if (!indi) return (STRING) "------------";
	bevt = event_to_date(BIRT(indi), TRUE);
	if (!bevt) bevt = event_to_date(BAPT(indi), TRUE);
	if (!bevt) bevt = (STRING) "";
	devt = event_to_date(DEAT(indi), TRUE);
	if (!devt) devt = event_to_date(BURI(indi), TRUE);
	if (!devt) devt = (STRING) "";
	if (keyflag) {
		key = key_of_record(indi);
		if(getlloptint("DisplayKeyTags", 0) > 0) {
			sprintf(tmp1, " [%s-%s] (i%s)", bevt, devt, key);
		} else {
			sprintf(tmp1, " [%s-%s] (%s)", bevt, devt, key);
		}
	}
	else
		sprintf(tmp1, " (%s-%s)", bevt, devt);
	name = indi_to_name(indi, len - strlen(tmp1));
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
 *  outstr: [I/O] printed event string 
 *  indi:   [IN]  whom to display
 *  fam:    [IN]  family record (used when displaying spouses)
 *  len:    [IN]  max length of output
 * If none are found, this will write a 0 to first char of outstr
 * If anything written, starts with ", "
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static void
family_events (STRING outstr, NODE indi, NODE fam, INT len)
{
	STRING evt = NULL;
	STRING p = outstr;
	INT mylen = len;
	p[0] = 0;
	evt = sh_fam_to_event_shrt(fam, "MARR", _(qSdspa_mar), mylen);
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
			evt = sh_indi_to_event_shrt(chld, "BIRT", _(qSdspa_chbr), mylen-2);
			if (evt && !append_event(&p, evt, &mylen, 10))
				return;
			if (!evt) {
				evt = sh_indi_to_event_shrt(chld, "CHR", _(qSdspa_chbr), mylen-2);
				if (evt && !append_event(&p, evt, &mylen, 10))
					return;
			}
		}
	}
	evt = sh_indi_to_event_shrt(indi, "BIRT", _(qSdspa_bir), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, "CHR", _(qSdspa_chr), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, "DEAT", _(qSdspa_dea), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, "BURI", _(qSdspa_bur), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
}
/*=============================================
 * indi_events -- Print string of events
 *  outstr: [I/O] printed event string 
 *  indi:   [IN]  whom to display
 *  len:    [IN]  max length of output
 * If none are found, this will write a 0 to first char of outstr
 * If anything written, starts with ", "
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static void
indi_events (STRING outstr, NODE indi, INT len)
{
	STRING evt = NULL;
	INT width = (len-2)/2;
	STRING p = outstr;
	INT mylen = len;
	p[0] = 0;

	evt = sh_indi_to_event_shrt(indi, "BIRT", _(qSdspa_bir), width);
	if (!evt)
		evt = sh_indi_to_event_shrt(indi, "CHR", _(qSdspa_chr), width);
	if (evt) {
		llstrcatn(&p, ", ", &mylen);
		llstrcatn(&p, evt, &mylen);
	}
	if (p == outstr)
		width = len;
	evt = sh_indi_to_event_shrt(indi, "DEAT", _(qSdspa_dea), width);
	if (!evt) evt = sh_indi_to_event_shrt(indi, "BURI", _(qSdspa_bur), width);
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
	/* parentheses & leading space & possible "i" */
	INT keyspace = max_keywidth() + 4; 
	INT evlen, namelen, temp;
	/* don't overflow scratch1, into which we catenate name & events */
	if (len > ARRSIZE(scratch1)-1)
		len = ARRSIZE(scratch1)-1;

	/* keywidth for key, 2 for comma space, and split between name & events */
	evlen = (len-2-keyspace)/2;
	namelen = evlen;

	if (!indi) return NULL;

	/* test to see if name is short */
	p = indi_to_name(indi, 100);
	if ((temp = strlen(p)) < evlen) {
		/* name is short, give extra to events */
		evlen += (namelen - temp);
		namelen -= (namelen - temp);
	}

	if (evlen > ARRSIZE(scratch2)-1) /* don't overflow name buffer */
		evlen = ARRSIZE(scratch2)-1;
	if (fam) {
		family_events(scratch2, indi, fam, evlen);
	} else {
		indi_events(scratch2, indi, evlen);
	}

	/* give name any unused space events left */
	if ((INT)strlen(scratch2)<evlen)
		namelen += evlen-(INT)strlen(scratch2);
	p = scratch1;
	strcpy(p, indi_to_name(indi, namelen));
	p += strlen(p);
	if (scratch2[0]) {
		strcpy(p, scratch2);
		p += strlen(p);
	}
	if(getlloptint("DisplayKeyTags", 0) > 0) {
		snprintf(p, scratch1+len-p, " (i%s)", key_of_record(indi));

	} else {
		snprintf(p, scratch1+len-p, " (%s)", key_of_record(indi));
	}
	return scratch1;
}
/*========================================================
 * show_aux -- Show source, event or other record
 *======================================================*/
void
show_aux (UIWINDOW uiwin, RECORD rec, INT mode, LLRECT rect
	, INT * scroll, BOOLEAN reuse)
{
	if (mode == 'g')
		show_gedcom(uiwin, rec, GDVW_NORMAL, rect, scroll, reuse);
	else if (mode == 't')
		show_gedcom(uiwin, rec, GDVW_TEXT, rect, scroll, reuse);
	else
		show_gedcom(uiwin, rec, GDVW_EXPANDED, rect, scroll, reuse);
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
 * _disp version means string is already in display encoding
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
	/* TODO: Should convert to output codeset now, before limiting text */

	/* copy into local buffer (here we enforce maxcol) */
	llstrncpy(buffer, string, buflen, uu8);
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
	mvccwaddstr(win, y, x, buffer);
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
	ZSTR zstr = zs_new();
	ZSTR zstr_ind = get_cache_stats_indi();
	ZSTR zstr_fam = get_cache_stats_fam();
	zs_appf(zstr, _("Cached: I:%s; F:%s")
		, zs_str(zstr_ind), zs_str(zstr_fam));
	msg_info(zs_str(zstr));
	zs_free(&zstr);
	zs_free(&zstr_ind);
	zs_free(&zstr_fam);
}
/*================================================
 * sh_indi_to_event -- Pass-thru to indi_to_event
 *  using long display reformatting
 *==============================================*/
static STRING
sh_indi_to_event_long (NODE node, STRING tag, STRING head, INT len)
{
	return indi_to_event(node, tag, head, len, &disp_long_rfmt);
}
/*================================================
 * sh_indi_to_event_shrt -- Pass-thru to indi_to_event, short display
 *  using short display reformatting
 *==============================================*/
static STRING
sh_indi_to_event_shrt (NODE node, STRING tag, STRING head, INT len)
{
	return indi_to_event(node, tag, head, len, &disp_shrt_rfmt);
}
/*==================================================
 * sh_fam_to_event_shrt -- Pass-thru to fam_to_event
 *  using display reformatting
 *================================================*/
static STRING
sh_fam_to_event_shrt (NODE node, STRING tag, STRING head
	, INT len)
{
	return fam_to_event(node, tag, head, len, &disp_shrt_rfmt);
}
