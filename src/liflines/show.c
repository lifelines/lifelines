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
/*=============================================================
 * show.c -- Curses version of display functions
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 24 Aug 93
 *   3.0.0 - 14 Sep 94    3.0.2 - 24 Dec 94
 *   3.0.3 - 03 May 95
 *===========================================================*/

#include "standard.h"
#include "screen.h"
#include "table.h"
#include "gedcom.h"
#include "indiseq.h"
#include "translat.h"
#include "cache.h"

STRING person_display();
STRING key_of_record();

#define MAXOTHERS 30
typedef char LINESTRING[80];

static LINESTRING Spers, Sbirt, Sdeat, Sfath, Smoth, Smarr;
static LINESTRING Shusb, Shbirt, Shdeat, Swife, Swbirt, Swdeat;
static LINESTRING Sothers[MAXOTHERS];
static INT Solen = 0, Sotop = 0;

/*===============================================
 * init_display_indi -- Initialize display person
 *=============================================*/
init_display_indi (pers)
NODE pers;
{
	INT nsp, nch, num, nm;
	STRING s;
	NODE fth = indi_to_fath(pers), mth = indi_to_moth(pers);
	TRANTABLE ttd = tran_tables[MINDS];
	CACHEEL icel;

	ASSERT(pers);
	s = indi_to_name(pers, ttd, 60);
	sprintf(Spers, "person: %s (%s)", s, key_of_record(pers));

	s = indi_to_event(pers, ttd, "BIRT", "  born: ", 77, FALSE);
	if (!s) s = indi_to_event(pers, ttd, "CHR", "  bapt: ", 77, FALSE);
	if (s) sprintf(Sbirt, s);
	else sprintf(Sbirt, "  born:");

	s = indi_to_event(pers, ttd, "DEAT", "  died: ", 77, FALSE);
	if (!s) s = indi_to_event(pers, ttd, "BURI", "  buri: ", 77, FALSE);
	if (s) sprintf(Sdeat, s);
	else sprintf(Sdeat, "  died:");

	s = person_display(fth, NULL, 67);
	if (s) sprintf(Sfath, "  father: %s", s);
	else sprintf(Sfath, "  father:");

	s = person_display(mth, NULL, 67);
	if (s) sprintf(Smoth, "  mother: %s", s);
	else sprintf(Smoth, "  mother:");

	Solen = 0;
	nsp = nch = 0;
	icel = indi_to_cacheel(pers);
	lock_cache(icel);
	FORFAMSS(pers, fam, sp, num)
		if (sp) add_spouse_line(++nsp, sp, fam);
		FORCHILDREN(fam, chld, nm)
			add_child_line(++nch, chld);
		ENDCHILDREN
	ENDFAMSS
	unlock_cache(icel);
}
/*==============================
 * show_person -- Display person
 *============================*/
show_person (pers, row, hgt)
NODE pers;	/* person */
INT row;	/* start row */
INT hgt;	/* avail rows */
{
	INT i;
	init_display_indi(pers);
	for (i = 0; i < hgt; i++) {
		wmove(main_win, row+i, 1);
		wclrtoeol(main_win);
#ifndef BSD
		mvwaddch(main_win, row+i, 79, ACS_VLINE);
#endif
	}
	mvwaddstr(main_win, row+0, 1, Spers);
	mvwaddstr(main_win, row+1, 1, Sbirt);
	mvwaddstr(main_win, row+2, 1, Sdeat);
	mvwaddstr(main_win, row+3, 1, Sfath);
	mvwaddstr(main_win, row+4, 1, Smoth);
	Sotop = 0;
	for (i = 0; i < Solen && i < hgt-5; i++)
		mvwaddstr(main_win, row+5+i, 1, Sothers[i]);
}
/*=============================================
 * add_spouse_line -- Add spouse line to others
 *===========================================*/
add_spouse_line (num, indi, fam)
INT num;
NODE indi, fam;
{
	STRING line;
	if (Solen >= MAXOTHERS) return;
	line = person_display(indi, fam, 76);
	sprintf(Sothers[Solen], "  spouse: %s", line);
	Sothers[Solen++][78] = 0;
}
/*===========================================
 * add_child_line -- Add child line to others
 *=========================================*/
add_child_line (num, indi)
INT num;
NODE indi;
{
	STRING line;
	if (Solen >= MAXOTHERS) return;
	line = person_display(indi, NULL, 65);
	sprintf(Sothers[Solen], "    child: %s", line);
	Sothers[Solen++][78] = 0;
}
/*==============================================
 * init_display_fam -- Initialize display family
 *============================================*/
init_display_fam (fam)
NODE fam;	/* family */
{
	NODE husb = fam_to_husb(fam);
	NODE wife = fam_to_wife(fam);
	STRING s, ik, fk;
	INT len, nch, nm;
	TRANTABLE ttd = tran_tables[MINDS];
	ASSERT(fam);
	fk = key_of_record(fam);
	if (husb) {
		ik = key_of_record(husb);
		len = 64 - (strlen(ik) + strlen(fk));
		s = indi_to_name(husb, ttd, len);
		sprintf(Shusb, "father: %s (%s) (%s)", s, ik, fk);
	} else
		sprintf(Shusb, "father: (%s)", fk);

	s = indi_to_event(husb, ttd, "BIRT", "  born: ", 77, FALSE);
	if (!s) s = indi_to_event(husb, ttd, "CHR", "  bapt: ", 77, FALSE);
	if (s) sprintf(Shbirt, s);
	else sprintf(Shbirt, "  born:");

	s = indi_to_event(husb, ttd, "DEAT", "  died: ", 77, FALSE);
	if (!s) s = indi_to_event(husb, ttd, "BURI", "  buri: ", 77, FALSE);
	if (s) sprintf(Shdeat, s);
	else sprintf(Shdeat, "  died:");

	if (wife) {
		ik = key_of_record(wife);
		len = 67 - strlen(ik);
		s = indi_to_name(wife, ttd, len);
		sprintf(Swife, "mother: %s (%s)", s, ik);
	} else
		sprintf(Swife, "mother:");

	s = indi_to_event(wife, ttd, "BIRT", "  born: ", 77, FALSE);
	if (!s) s = indi_to_event(wife, ttd, "CHR", " bapt: ", 77, FALSE);
	if (s) sprintf(Swbirt, s);
	else sprintf(Swbirt, "  born:");

	s = indi_to_event(wife, ttd, "DEAT", "  died: ", 77, FALSE);
	if (!s) s = indi_to_event(wife, ttd, "BURI", " buri: ", 77, FALSE);
	if (s) sprintf(Swdeat, s);
	else sprintf(Swdeat, "  died:");

	s = indi_to_event(fam, ttd, "MARR", "married: ", 78, FALSE);
	if (s) sprintf(Smarr, s);
	else sprintf(Smarr, "married:");

	Solen = 0;
	nch = 0;
	FORCHILDREN(fam, chld, nm)
		add_child_line(++nch, chld);
	ENDCHILDREN
}
/*===================================
 * show_long_family -- Display family
 *=================================*/
show_long_family (fam, row, hgt)
NODE fam;
INT row, hgt;
{
	INT i;
	init_display_fam(fam);
	for (i = 0; i < hgt; i++) {
		wmove(main_win, row+i, 1);
		wclrtoeol(main_win);
#ifndef BSD
		mvwaddch(main_win, row+i, COLS-1, ACS_VLINE);
#endif
	}
	mvwaddstr(main_win, row+0, 1, Shusb);
	mvwaddstr(main_win, row+1, 1, Shbirt);
	mvwaddstr(main_win, row+2, 1, Shdeat);
	mvwaddstr(main_win, row+3, 1, Swife);
	mvwaddstr(main_win, row+4, 1, Swbirt);
	mvwaddstr(main_win, row+5, 1, Swdeat);
	mvwaddstr(main_win, row+6, 1, Smarr);
	Sotop = 0;
	for (i = 0; i < Solen && i < hgt-7; i++)
		mvwaddstr(main_win, row+7+i, 1, Sothers[i]+1);
}
/*====================================
 * show_short_family -- Display family
 *==================================*/
show_short_family (fam, row, hgt)
NODE fam;
INT row;
INT hgt;
{
	INT i;
	init_display_fam(fam);
	for (i = 0; i < hgt; i++) {
		wmove(main_win, row+i, 1);
		wclrtoeol(main_win);
#ifndef BSD
		mvwaddch(main_win, row+i, COLS-1, ACS_VLINE);
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
	Sotop = 0;
	for (i = 0; i < Solen && i < hgt-5; i++)
		mvwaddstr(main_win, row+5+i, 1, Sothers[i]+1);
}
/*================================================
 * show_pedigree -- Show person in pedigree format
 *==============================================*/
show_pedigree (indi)
NODE indi;
{
	NODE f = indi_to_fath(indi);	/* 2nd generation */
	NODE m = indi_to_moth(indi);
	NODE ff = indi_to_fath(f);	/* 3rd generation */
	NODE fm = indi_to_moth(f);
	NODE mf = indi_to_fath(m);
	NODE mm = indi_to_moth(m);
	NODE fff = indi_to_fath(ff);	/* 4th generation */
	NODE ffm = indi_to_moth(ff);
	NODE fmf = indi_to_fath(fm);
	NODE fmm = indi_to_moth(fm);
	NODE mff = indi_to_fath(mf);
	NODE mfm = indi_to_moth(mf);
	NODE mmf = indi_to_fath(mm);
	NODE mmm = indi_to_moth(mm);
	STRING indi_to_ped_fix();
	WINDOW *w = main_win;
	INT i;

	for (i = 1; i <= 15; i++) {
		wmove(w, i, 1);
		wclrtoeol(w);
#ifndef BSD
		mvwaddch(w, i, COLS-1, ACS_VLINE);
#endif
	}
	mvwprintw(w, 1, 2, "                  %s", indi_to_ped_fix(fff, 52));
	mvwprintw(w, 2, 2, "            %s", indi_to_ped_fix(ff, 60));
	mvwprintw(w, 3, 2, "                  %s", indi_to_ped_fix(ffm, 52));
	mvwprintw(w, 4, 2, "      %s", indi_to_ped_fix(f, 68));
	mvwprintw(w, 5, 2, "                  %s", indi_to_ped_fix(fmf, 52));
	mvwprintw(w, 6, 2, "            %s", indi_to_ped_fix(fm, 60));
	mvwprintw(w, 7, 2, "                  %s", indi_to_ped_fix(fmm, 52));
 	mvwprintw(w, 8, 2, "%s",indi_to_ped_fix(indi, 68));
	mvwprintw(w, 9, 2, "                  %s", indi_to_ped_fix(mff, 52));
	mvwprintw(w, 10, 2, "            %s", indi_to_ped_fix(mf, 60));
	mvwprintw(w, 11, 2, "                  %s", indi_to_ped_fix(mfm, 52));
	mvwprintw(w, 12, 2, "      %s", indi_to_ped_fix(m, 68));
	mvwprintw(w, 13, 2, "                  %s", indi_to_ped_fix(mmf, 52));
	mvwprintw(w, 14, 2, "            %s", indi_to_ped_fix(mm, 60));
	mvwprintw(w, 15, 2, "                  %s", indi_to_ped_fix(mmm, 52));
}
/*===============================================================
 * indi_to_ped_fix -- Construct person STRING for pedigree screen
 *=============================================================*/
STRING indi_to_ped_fix (indi, len)
NODE indi;
INT len;
{
	STRING bevt, devt, name, key;
	static unsigned char scratch[100];
	TRANTABLE ttd = tran_tables[MINDS];

	if (!indi) return (STRING) "------------";
	bevt = event_to_date(BIRT(indi), ttd, TRUE);
	if (!bevt) bevt = event_to_date(BAPT(indi), ttd, TRUE);
	if (!bevt) bevt = (STRING) "";
	devt = event_to_date(DEAT(indi), ttd, TRUE);
	if (!devt) devt = event_to_date(BURI(indi), ttd, TRUE);
	if (!devt) devt = (STRING) "";
	name = indi_to_name(indi, ttd, len);
	if (keyflag) key = key_of_record(indi);
	if (keyflag)
		sprintf(scratch, "%s [%s-%s] (%s)", name, bevt, devt, key);
	else
		sprintf(scratch, "%s (%s-%s)", name, bevt, devt);
	return scratch;
}
/*=============================================
 * person_display -- Create person display line
 *===========================================*/
STRING person_display (indi, fam, len)
NODE indi;
NODE fam;
INT len;
{
	static unsigned char scratch[100];
	STRING p = scratch, evt = NULL;
	NODE chld;
	TRANTABLE ttd = tran_tables[MINDS];
	if (!indi) return NULL;
	strcpy(p, indi_to_name(indi, ttd, 40));
	p += strlen(p);
	if (fam) {
		evt = fam_to_event(fam, ttd, "MARR", "m. ", 35, TRUE);
		if (!evt) {
			if (chld = fam_to_first_chil(fam)) {
				evt = indi_to_event(chld, ttd, "BIRT", "cb. ",
				    35, TRUE);
				if (!evt) evt = indi_to_event(chld, ttd, "CHR",
					    "cb. ", 35, TRUE);
			}
		}
		if (!evt) evt = indi_to_event(indi, ttd, "BIRT", "b. ", 35,
		    TRUE);
		if (!evt) evt = indi_to_event(indi, ttd, "CHR", "bap. ", 35,
		    TRUE);
		if (!evt) evt = indi_to_event(indi, ttd, "DEAT", "d. ", 35,
		    TRUE);
		if (!evt) evt = indi_to_event(indi, ttd, "BURI", "bur. ", 35,
		    TRUE);
	} else {
		evt = indi_to_event(indi, ttd, "BIRT", "b. ", 20, TRUE);
		if (!evt) evt = indi_to_event(indi, ttd, "CHR", "bap. ", 20,
		    TRUE);
		if (evt) {
			sprintf(p, ", %s", evt);
			p += strlen(p);
		}
		evt = indi_to_event(indi, ttd, "DEAT", "d. ", 20, TRUE);
		if (!evt) evt = indi_to_event(indi, ttd, "BURI", "bur. ", 20,
		    TRUE);
	}
	if (evt) {
		sprintf(p, ", %s", evt);
		p += strlen(p);
	}
	sprintf(p, " (%s)", key_of_record(indi));
	scratch[66] = 0;
	return scratch;
}

static STRING empstr = (STRING) "                                                 ";
/*==========================================
 * show_list - Show name list in list screen
 *========================================*/
#define VIEWABLE 13
show_list (seq, top, cur, mark)
INDISEQ seq;
INT top, cur, mark;
{
	WINDOW *win = main_win;
	INT i, j, row, len = length_indiseq(seq);
	STRING key, name;
	NODE indi;
	char scratch[200], *p;
	TRANTABLE ttd = tran_tables[MINDS];

	for (i = 8; i <= 20; i++)
		mvwaddstr(win, i, 1, empstr);
	row = 8;
	for (i = top, j = 0; j < VIEWABLE && i < len; i++, j++) {
		element_indiseq(seq, i, &key, &name);
		indi = key_to_indi(key);
		if (i == mark) mvwaddch(win, row, 2, 'x');
		if (i == cur) {
			mvwaddch(win, row, 3, '>');
			show_person(indi, 1, 6);
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
show_aux_display (node, row, hgt)
NODE node;
INT row, hgt;
{
	STRING key;
	INT i;

	ASSERT(node);
	for (i = 0; i < hgt; i++) {
		wmove(main_win, row+i, 1);
		wclrtoeol(main_win);
#ifndef BSD
		mvwaddch(main_win, row+i, 79, ACS_VLINE);
#endif
	}
	key = rmvat(nxref(node));
#if 0
	switch (*key) {
	case 'S':
		show_sour_display(node, row, hgt);
		break;
	case 'E':
	case 'X':
	}
#endif
}
/*=========================================
 * show_sour_display -- Show source display
 *=======================================*/
show_sour_display (node, row, hgt)
NODE node;
INT row, hgt;
{
	STRING key;
}
