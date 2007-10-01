/* 
   Copyright (c) 1991-2005 Thomas T. Wetmore IV

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
 * datei.c -- Internal date code for structured shared betweeen date modules
 * Copyright(c) 1992-2005 by T. T. Wetmore IV; all rights reserved
 *============================================================*/

#include <time.h>
#include "llstdlib.h"
#include "table.h"
#include "date.h"
#include "datei.h"
#include "zstr.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSdatea_abtA,qSdatea_abtB,qSdatea_estA,qSdatea_estB,qSdatea_calA;
extern STRING qSdatea_calB,qSdatep_fromA,qSdatep_fromB,qSdatep_toA,qSdatep_toB;
extern STRING qSdatep_frtoA,qSdatep_frtoB,qSdater_befA,qSdater_befB,qSdater_aftA;
extern STRING qSdater_aftB,qSdater_betA,qSdater_betB;
extern STRING qScaljul,qScalheb,qScalfr,qScalrom;
extern STRING qSmon_gj1A,qSmon_gj1B,qSmon_gj2A,qSmon_gj2B,qSmon_gj3A,qSmon_gj3B;
extern STRING qSmon_gj4A,qSmon_gj4B,qSmon_gj5A,qSmon_gj5B,qSmon_gj6A,qSmon_gj6B;
extern STRING qSmon_gj7A,qSmon_gj7B,qSmon_gj8A,qSmon_gj8B,qSmon_gj9A,qSmon_gj9B;
extern STRING qSmon_gj10A,qSmon_gj10B,qSmon_gj11A,qSmon_gj11B,qSmon_gj12A,qSmon_gj12B;
extern STRING qSmon_heb1A,qSmon_heb1B,qSmon_heb2A,qSmon_heb2B,qSmon_heb3A,qSmon_heb3B;
extern STRING qSmon_heb4A,qSmon_heb4B,qSmon_heb5A,qSmon_heb5B,qSmon_heb6A,qSmon_heb6B;
extern STRING qSmon_heb7A,qSmon_heb7B,qSmon_heb8A,qSmon_heb8B,qSmon_heb9A,qSmon_heb9B;
extern STRING qSmon_heb10A,qSmon_heb10B,qSmon_heb11A,qSmon_heb11B;
extern STRING qSmon_heb12A,qSmon_heb12B,qSmon_heb13A,qSmon_heb13B;
extern STRING qSmon_fr1A,qSmon_fr1B,qSmon_fr2A,qSmon_fr2B,qSmon_fr3A,qSmon_fr3B;
extern STRING qSmon_fr4A,qSmon_fr4B,qSmon_fr5A,qSmon_fr5B,qSmon_fr6A,qSmon_fr6B;
extern STRING qSmon_fr7A,qSmon_fr7B,qSmon_fr8A,qSmon_fr8B,qSmon_fr9A,qSmon_fr9B;
extern STRING qSmon_fr10A,qSmon_fr10B,qSmon_fr11A,qSmon_fr11B;
extern STRING qSmon_fr12A,qSmon_fr12B,qSmon_fr13A,qSmon_fr13B;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void init_keywordtbl(void);
static void load_lang(void);
static STRING lower_dup(STRING s);
static STRING title_dup(STRING s);
static STRING upper_dup(STRING s);

/*********************************************
 * local variables
 *********************************************/

STRING cmplx_custom[ECMPLX_END];
STRING cmplx_pics[ECMPLX_END][6];

STRING date_pic;

/* generated month names (for Gregorian/Julian months) */
STRING calendar_pics[GDV_CALENDARS_IX];

STRING roman_lower[] = { "i","ii","iii","iv","v","vi","vii","viii"
	,"ix","x","xi","xii","xiii" };
STRING roman_upper[] = { "I","II","III","IV","V","VI","VII","VIII"
	,"IX","X","XI","XII","XIII" };

MONTH_NAMES months_gj[12];
MONTH_NAMES months_fr[13];
MONTH_NAMES months_heb[13];

/* GEDCOM keywords (fixed, not language dependent) */
struct gedcom_keywords_s gedkeys[] = {
/* Gregorian/Julian months are values 1 to 12 */
	{ "JAN", 1 }
	,{ "FEB", 2 }
	,{ "MAR", 3 }
	,{ "APR", 4 }
	,{ "MAY", 5 }
	,{ "JUN", 6 }
	,{ "JUL", 7 }
	,{ "AUG", 8 }
	,{ "SEP", 9 }
	,{ "OCT", 10 }
	,{ "NOV", 11 }
	,{ "DEC", 12 }
/* Hebew months are values 301 to 313 */
	,{ "TSH", 301 }
	,{ "CSH", 302 }
	,{ "KSL", 303 }
	,{ "TVT", 304 }
	,{ "SHV", 305 }
	,{ "ADR", 306 }
	,{ "ADS", 307 }
	,{ "NSN", 308 }
	,{ "IYR", 309 }
	,{ "SVN", 310 }
	,{ "TMZ", 311 }
	,{ "AAV", 312 }
	,{ "ELL", 313 }
/* French Republic months are values 401 to 413 */
	,{ "VEND", 401 }
	,{ "BRUM", 402 }
	,{ "FRIM", 403 }
	,{ "NIVO", 404 }
	,{ "PLUV", 405 }
	,{ "VENT", 406 }
	,{ "GERM", 407 }
	,{ "FLOR", 408 }
	,{ "PRAI", 409 }
	,{ "MESS", 410 }
	,{ "THER", 411 }
	,{ "FRUC", 412 }
	,{ "COMP", 413 }
/* modifiers are values 1001 to 1000+GD_END2 */
	,{ "ABT", 1000+GD_ABT }
	,{ "EST", 1000+GD_EST }
	,{ "CAL", 1000+GD_CAL }
	,{ "BEF", 1000+GD_BEF }
	,{ "AFT", 1000+GD_AFT }
	,{ "BET", 1000+GD_BET }
	,{ "AND", 1000+GD_AND }
	,{ "FROM", 1000+GD_FROM }
	,{ "TO", 1000+GD_TO }
/* calendars are values 2001 to 2000+GDV_CALENDARS_IX */
	,{ "@#DGREGORIAN@", 2000+GDV_GREGORIAN }
	,{ "@#DJULIAN@", 2000+GDV_JULIAN }
	,{ "@#DHEBREW@", 2000+GDV_HEBREW }
	,{ "@#DFRENCH R@", 2000+GDV_FRENCH }
	,{ "@#DROMAN@", 2000+GDV_ROMAN }
/* BC */
	/* parentheses are handled by lexical tokenizer */
	,{ "B.C.", 1000+GD_BC }
/* Some liberal (non-GEDCOM) entries */
	,{ "BC", 1000+GD_BC }
	,{ "B.C.E.", 1000+GD_BC }
	,{ "BCE", 1000+GD_BC }
	,{ "A.D.", 1000+GD_AD }
	,{ "AD", 1000+GD_AD }
	,{ "C.E.", 1000+GD_AD }
	,{ "CE", 1000+GD_AD }
/* Some liberal (non-GEDCOM) but English-biased entries */
	,{ "JANUARY", 1 }
	,{ "FEBRUARY", 2 }
	,{ "MARCH", 3 }
	,{ "APRIL", 4 }
	,{ "MAY", 5 }
	,{ "JUNE", 6 }
	,{ "JULY", 7 }
	,{ "AUGUST", 8 }
	,{ "SEPTEMBER", 9 }
	,{ "OCTOBER", 10 }
	,{ "NOVEMBER", 11 }
	,{ "DECEMBER", 12 }
	,{ "ABOUT", 1000+GD_ABT }
	,{ "ESTIMATED", 1000+GD_EST }
	,{ "CALCULATED", 1000+GD_CAL }
	,{ "BEFORE", 1000+GD_BEF }
	,{ "AFTER", 1000+GD_AFT }
	,{ "BETWEEN", 1000+GD_BET }
};


TABLE keywordtbl = NULL;
BOOLEAN lang_changed=FALSE;


/*=============================
 * initialize_if_needed -- init module or reload language
 *===========================*/
void
initialize_if_needed (void)
{
	if (!keywordtbl) {
		init_keywordtbl();
		load_lang();
	} else if (lang_changed) {
		load_lang();
		lang_changed = FALSE;
	}
}
/*=========================================
 * init_keywordtbl -- Set up table of known
 *  keywords which we recognize in parsing dates
 *=======================================*/
static void
init_keywordtbl (void)
{
	INT i, j;
	keywordtbl = create_table_int();
	/* Load GEDCOM keywords & values into keyword table */
	for (i=0; i<ARRSIZE(gedkeys); ++i) {
		j = gedkeys[i].value;
		insert_table_int(keywordtbl, gedkeys[i].keyword, j);
	}
}
/*=============================
 * load_one_cmplx_pic -- Generate case variations
 *  of one complex picture string.
 * Created: 2001/12/30 (Perry Rapp)
 *===========================*/
static void
load_one_cmplx_pic (INT ecmplx, STRING abbrev, STRING full)
{
	STRING loc_abbrev = strsave(abbrev);
	STRING loc_full = strsave(full);
	ASSERT(ecmplx>=0);
	ASSERT(ecmplx <ECMPLX_END);
	/* 0=ABT (cmplx=3) */
	cmplx_pics[ecmplx][0] = upper_dup(loc_abbrev);
	/* 1=Abt (cmplx=4) */
	cmplx_pics[ecmplx][1] = title_dup(loc_abbrev);
	/* 2=ABOUT (cmplx=5) */
	cmplx_pics[ecmplx][2] = upper_dup(loc_full);
	/* 3=About (cmplx=6) */
	cmplx_pics[ecmplx][3] = title_dup(loc_full);
	/* 4=abt (cmplx=7) */
	cmplx_pics[ecmplx][4] = lower_dup(loc_abbrev);
	/* 5=about (cmplx=8) */
	cmplx_pics[ecmplx][5] = lower_dup(loc_full);
	stdfree(loc_abbrev);
	stdfree(loc_full);

}
/*=============================
 * load_one_month -- Generate case variations
 *  of one month name
 *  @monum:  [IN]  month num (0-based)
 *  @monarr: [I/O] month array
 *  @abbrev: [IN]  eg, "jan"
 *  @full:   [IN]  eg, "january"
 *===========================*/
static void
load_one_month (INT monum, MONTH_NAMES * monarr, STRING abbrev, STRING full)
{
	/* 0-5 codes as in load_cmplx_pic(...) above */
	STRING locx_abbrev = strsave(abbrev);
	STRING loc_full = strsave(full);
	STRING loc_abbrev = locx_abbrev;
	/* special handling for **may, to differentiate from may */
	if (loc_abbrev[0]=='*' && loc_abbrev[1]=='*')
		loc_abbrev += 2;
	monarr[monum][0] = upper_dup(loc_abbrev);
	monarr[monum][1] = title_dup(loc_abbrev);
	monarr[monum][2] = upper_dup(loc_full);
	monarr[monum][3] = title_dup(loc_full);
	monarr[monum][4] = lower_dup(loc_abbrev);
	monarr[monum][5] = lower_dup(loc_full);
	stdfree(locx_abbrev);
	stdfree(loc_full);
}
/*=============================
 * clear_lang -- Free all generated picture strings
 * This is used both changing languages, and at cleanup time
 *===========================*/
static void
clear_lang (void)
{
	INT i,j;
	/* clear complex pics */
	for (i=0; i<ECMPLX_END; ++i) {
		for (j=0; j<6; ++j) {
			strfree(&cmplx_pics[i][j]);
		}
	}
	/* clear calendar pics */
	for (i=0; i<ARRSIZE(calendar_pics); ++i) {
		strfree(&calendar_pics[i]);
	}
	/* clear Gregorian/Julian month names */
	for (i=0; i<ARRSIZE(months_gj); ++i) {
		for (j=0; j<ARRSIZE(months_gj[0]); ++j) {
			strfree(&months_gj[i][j]);
		}
	}
	/* clear Hebrew month names */
	for (i=0; i<ARRSIZE(months_heb); ++i) {
		for (j=0; j<ARRSIZE(months_heb[0]); ++j) {
			strfree(&months_heb[i][j]);
		}
	}
	/* clear French Republic month names */
	for (i=0; i<ARRSIZE(months_fr); ++i) {
		for (j=0; j<ARRSIZE(months_fr[0]); ++j) {
			strfree(&months_fr[i][j]);
		}
	}
}
/*=============================
 * load_lang -- Load generated picture strings
 *  based on current language
 * This must be called if current language changes.
 *===========================*/
static void
load_lang (void)
{
	INT i,j;
	
	/* TODO: if we have language-specific cmplx_custom, deal with
	that here */

	/* free all pictures */
	clear_lang();

	/* load complex pics */
	load_one_cmplx_pic(ECMPLX_ABT, _(qSdatea_abtA), _(qSdatea_abtB));
	load_one_cmplx_pic(ECMPLX_EST, _(qSdatea_estA), _(qSdatea_estB));
	load_one_cmplx_pic(ECMPLX_CAL, _(qSdatea_calA), _(qSdatea_calB));
	load_one_cmplx_pic(ECMPLX_FROM, _(qSdatep_fromA), _(qSdatep_fromB));
	load_one_cmplx_pic(ECMPLX_TO, _(qSdatep_toA), _(qSdatep_toB));
	load_one_cmplx_pic(ECMPLX_FROM_TO, _(qSdatep_frtoA), _(qSdatep_frtoB));
	load_one_cmplx_pic(ECMPLX_BEF, _(qSdater_befA), _(qSdater_befB));
	load_one_cmplx_pic(ECMPLX_AFT, _(qSdater_aftA), _(qSdater_aftB));
	load_one_cmplx_pic(ECMPLX_BET_AND, _(qSdater_betA), _(qSdater_betB));
	/* test that all were loaded */	
	for (i=0; i<ECMPLX_END; ++i) {
		for (j=0; j<6; ++j) {
			ASSERT(cmplx_pics[i][j]);
		}
	}


	calendar_pics[GDV_JULIAN] = strsave(_(qScaljul));
	calendar_pics[GDV_HEBREW] = strsave(_(qScalheb));
	calendar_pics[GDV_FRENCH] = strsave(_(qScalfr));
	calendar_pics[GDV_ROMAN] = strsave(_(qScalrom));
	/* not all slots in calendar_pics are used */

	/* load Gregorian/Julian month names */
	load_one_month(0, months_gj, _(qSmon_gj1A), _(qSmon_gj1B));
	load_one_month(1, months_gj, _(qSmon_gj2A), _(qSmon_gj2B));
	load_one_month(2, months_gj, _(qSmon_gj3A), _(qSmon_gj3B));
	load_one_month(3, months_gj, _(qSmon_gj4A), _(qSmon_gj4B));
	load_one_month(4, months_gj, _(qSmon_gj5A), _(qSmon_gj5B));
	load_one_month(5, months_gj, _(qSmon_gj6A), _(qSmon_gj6B));
	load_one_month(6, months_gj, _(qSmon_gj7A), _(qSmon_gj7B));
	load_one_month(7, months_gj, _(qSmon_gj8A), _(qSmon_gj8B));
	load_one_month(8, months_gj, _(qSmon_gj9A), _(qSmon_gj9B));
	load_one_month(9, months_gj, _(qSmon_gj10A), _(qSmon_gj10B));
	load_one_month(10, months_gj, _(qSmon_gj11A), _(qSmon_gj11B));
	load_one_month(11, months_gj, _(qSmon_gj12A), _(qSmon_gj12B));
	/* test that all were loaded */	
	for (i=0; i<ARRSIZE(months_gj); ++i) {
		for (j=0; j<ARRSIZE(months_gj[0]); ++j) {
			ASSERT(months_gj[i][j]);
		}
	}

	/* load Hebrew month names */
	load_one_month(0, months_heb, _(qSmon_heb1A), _(qSmon_heb1B));
	load_one_month(1, months_heb, _(qSmon_heb2A), _(qSmon_heb2B));
	load_one_month(2, months_heb, _(qSmon_heb3A), _(qSmon_heb3B));
	load_one_month(3, months_heb, _(qSmon_heb4A), _(qSmon_heb4B));
	load_one_month(4, months_heb, _(qSmon_heb5A), _(qSmon_heb5B));
	load_one_month(5, months_heb, _(qSmon_heb6A), _(qSmon_heb6B));
	load_one_month(6, months_heb, _(qSmon_heb7A), _(qSmon_heb7B));
	load_one_month(7, months_heb, _(qSmon_heb8A), _(qSmon_heb8B));
	load_one_month(8, months_heb, _(qSmon_heb9A), _(qSmon_heb9B));
	load_one_month(9, months_heb, _(qSmon_heb10A), _(qSmon_heb10B));
	load_one_month(10, months_heb, _(qSmon_heb11A), _(qSmon_heb11B));
	load_one_month(11, months_heb, _(qSmon_heb12A), _(qSmon_heb12B));
	load_one_month(12, months_heb, _(qSmon_heb13A), _(qSmon_heb13B));
	/* test that all were loaded */	
	for (i=0; i<ARRSIZE(months_heb); ++i) {
		for (j=0; j<ARRSIZE(months_heb[0]); ++j) {
			ASSERT(months_heb[i][j]);
		}
	}

	/* load French Republic month names */
	load_one_month(0, months_fr, _(qSmon_fr1A), _(qSmon_fr1B));
	load_one_month(1, months_fr, _(qSmon_fr2A), _(qSmon_fr2B));
	load_one_month(2, months_fr, _(qSmon_fr3A), _(qSmon_fr3B));
	load_one_month(3, months_fr, _(qSmon_fr4A), _(qSmon_fr4B));
	load_one_month(4, months_fr, _(qSmon_fr5A), _(qSmon_fr5B));
	load_one_month(5, months_fr, _(qSmon_fr6A), _(qSmon_fr6B));
	load_one_month(6, months_fr, _(qSmon_fr7A), _(qSmon_fr7B));
	load_one_month(7, months_fr, _(qSmon_fr8A), _(qSmon_fr8B));
	load_one_month(8, months_fr, _(qSmon_fr9A), _(qSmon_fr9B));
	load_one_month(9, months_fr, _(qSmon_fr10A), _(qSmon_fr10B));
	load_one_month(10, months_fr, _(qSmon_fr11A), _(qSmon_fr11B));
	load_one_month(11, months_fr, _(qSmon_fr12A), _(qSmon_fr12B));
	load_one_month(12, months_fr, _(qSmon_fr13A), _(qSmon_fr13B));
	/* test that all were loaded */	
	for (i=0; i<ARRSIZE(months_fr); ++i) {
		for (j=0; j<ARRSIZE(months_fr[0]); ++j) {
			ASSERT(months_fr[i][j]);
		}
	}
}
/*=============================
 * term_date -- Cleanup for finishing program
 * Created: 2003-02-02 (Perry Rapp)
 *===========================*/
void
term_date (void)
{
	clear_lang();
	if (keywordtbl) {
		destroy_table(keywordtbl);
		keywordtbl = 0;
	}
}
/*=============================
 * upper_dup -- Get uppercase & strdup it
 *===========================*/
static STRING
upper_dup (STRING s)
{
	ZSTR zstr = ll_toupperz(s, uu8);
	STRING str = strdup(zs_str(zstr));
	zs_free(&zstr);
	return str;
}
/*=============================
 * lower_dup -- Get lowercase & strdup it
 *===========================*/
static STRING
lower_dup (STRING s)
{
	ZSTR zstr = ll_tolowerz(s, uu8);
	STRING str = strdup(zs_str(zstr));
	zs_free(&zstr);
	return str;
}
/*=============================
 * title_dup -- Get titlecase & strdup it
 *===========================*/
static STRING
title_dup (STRING s)
{
	ZSTR zstr = ll_totitlecasez(s, uu8);
	STRING str = strdup(zs_str(zstr));
	zs_free(&zstr);
	return str;
}
/*=============================
 * date_get_day -- Day number of first date
 *===========================*/
INT
date_get_day (GDATEVAL gdv)
{
	ASSERT(gdv);
	return gdv->date1.day.val;
}
/*=============================
 * date_get_month -- Month number of first date
 *===========================*/
INT
date_get_month (GDATEVAL gdv)
{
	ASSERT(gdv);
	return gdv->date1.month.val;
}
/*=============================
 * date_get_year -- Year number of first date
 *===========================*/
INT
date_get_year (GDATEVAL gdv)
{
	ASSERT(gdv);
	return gdv->date1.year.val;
}
/*=============================
 * date_get_year_string -- Raw year string of first date
 *===========================*/
STRING
date_get_year_string (GDATEVAL gdv)
{
	ASSERT(gdv);
	return gdv->date1.year.str;
}
/*=============================
 * date_get_mod -- Mod value of first date
 * Perry, 2005-09-25, I don't think this is ever populated
 *===========================*/
INT
date_get_mod (GDATEVAL gdv)
{
	ASSERT(gdv);
	return gdv->date1.mod;
}
