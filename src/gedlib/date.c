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
 * date.c -- Code to process dates
 * Copyright(c) 1992-94 by T. T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 17 Aug 93
 *   3.0.0 - 20 Jan 94    3.0.2 - 10 Nov 94
 *   3.0.3 - 17 Jul 95
 *============================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-04-12 J.F.Chandler */

#include <time.h>
#include "llstdlib.h"
#include "table.h"
#include "date.h"
#include "zstr.h"


/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSdatea_abtA,qSdatea_abtB,qSdatea_estA,qSdatea_estB,qSdatea_calA;
extern STRING qSdatea_calB,qSdatep_fromA,qSdatep_fromB,qSdatep_toA,qSdatep_toB;
extern STRING qSdatep_frtoA,qSdatep_frtoB,qSdater_befA,qSdater_befB,qSdater_aftA;
extern STRING qSdater_aftB,qSdater_betA,qSdater_betB;
extern STRING qSdatetrl_bcA,qSdatetrl_bcB,qSdatetrl_bcC,qSdatetrl_bcD;
extern STRING qSdatetrl_adA,qSdatetrl_adB,qSdatetrl_adC,qSdatetrl_adD;
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
 * local types used in local function prototypes
 *********************************************/

/* used in parsing dates -- 1st, 2nd, & 3rd numbers found */
struct tag_nums { struct tag_dnum num1; struct tag_dnum num2; struct tag_dnum num3; };

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void analyze_numbers(GDATEVAL, struct tag_gdate *, struct tag_nums *);
static void analyze_word(GDATEVAL gdv, struct tag_gdate * pdate
	, struct tag_nums * nums, INT ival, BOOLEAN * newdate);
static void assign_dnum(struct tag_dnum * dest, struct tag_dnum * src);
static void clear_dnum(struct tag_dnum * dnum);
static void clear_numbers(struct tag_nums * nums);
static ZSTR do_zformat_date(STRING str, INT dfmt, INT mfmt,
             INT yfmt, INT sfmt, INT efmt, INT cmplx);
static void format_cal(ZSTR zstr, INT cal);
static ZSTR format_complex(GDATEVAL gdv, INT cmplx, STRING ymd2, STRING ymd3);
static void format_day(struct tag_dnum da, INT dfmt, STRING output);
static STRING format_month(INT cal, struct tag_dnum mo, INT mfmt);
static void format_eratime(ZSTR zstr, INT eratime, INT efmt);
static STRING format_year(struct tag_dnum yr, INT yfmt);
static void format_ymd(ZSTR zstr, STRING syr, STRING smo, STRING sda, INT sfmt);
static void free_gdate(struct tag_gdate *);
static INT get_date_tok(struct tag_dnum*);
static void init_keywordtbl(void);
static void initialize_if_needed(void);
static BOOLEAN is_date_delim(char c);
static BOOLEAN is_valid_day(struct tag_gdate * pdate, struct tag_dnum day);
static BOOLEAN is_valid_month(struct tag_gdate * pdate, struct tag_dnum month);
static void load_lang(void);
static STRING lower_dup(STRING s);
static void mark_freeform(GDATEVAL gdv);
static void mark_invalid(GDATEVAL gdv);
static void set_date_string(STRING);
static STRING title_dup(STRING s);
static STRING upper_dup(STRING s);
static ZSTR zshorten_date(STRING date);

/*********************************************
 * local types & variables
 *********************************************/

/* token types, used in parsing */
enum { MONTH_TOK=1, CHAR_TOK, WORD_TOK, ICONS_TOK, CALENDAR_TOK, YEAR_TOK };
/* GEDCOM keywords */
enum { GD_ABT=1, GD_EST, GD_CAL, GD_BEF, GD_AFT, GD_BET, GD_AND, GD_FROM, GD_TO, GD_END1 };
enum { GD_BC=GD_END1, GD_AD, GD_END2 };

/* complex picture strings */
enum { ECMPLX_ABT, ECMPLX_EST, ECMPLX_CAL, ECMPLX_BEF, ECMPLX_AFT
	, ECMPLX_BET_AND, ECMPLX_FROM, ECMPLX_TO, ECMPLX_FROM_TO
	, ECMPLX_END };

/* custom picture strings for complex dates (0 means use default).
   These change when language changes.
   These should be stdalloc'd (unless 0). */
static STRING cmplx_custom[ECMPLX_END];
/* generated picture strings for complex dates
   eg, "BEFORE", "Before", "before", "BEF" 
   These change when language changes.
   These are stdalloc'd. */
static STRING cmplx_pics[ECMPLX_END][6];

/* custom picture string for ymd date format */
static STRING date_pic;

/* generated month names (for Gregorian/Julian months) */
static STRING calendar_pics[GDV_CALENDARS_IX];

typedef STRING MONTH_NAMES[6];

static STRING roman_lower[] = { "i","ii","iii","iv","v","vi","vii","viii"
	,"ix","x","xi","xii","xiii" };
static STRING roman_upper[] = { "I","II","III","IV","V","VI","VII","VIII"
	,"IX","X","XI","XII","XIII" };

static MONTH_NAMES months_gj[12];
static MONTH_NAMES months_fr[13];
static MONTH_NAMES months_heb[13];

struct gedcom_keywords_s {
	STRING keyword;
	INT value;
};

/* GEDCOM keywords (fixed, not language dependent) */
static struct gedcom_keywords_s gedkeys[] = {
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


static STRING sstr=NULL, sstr_start=NULL;
static TABLE keywordtbl = NULL;
static BOOLEAN lang_changed=FALSE;

/*==========================================
 * do_format_date -- Do general date formatting
 * str - raw string containing a date
 *  dfmt; [IN]  day format code (see format_day function below)
 *  mfmt: [IN]  month format code (see format_month function below)
 *  yfmt: [IN]  year format code (see format_year function below)
 *  sfmt: [IN]  combining code (see format_ymd function below)
 *  efmt: [IN]  era format (see format_eratime function below)
 * cmplx - 0 is year only, 1 is complex, including
 *         date modifiers, ranges, and/or double-dating
 * Returns static buffer
 *========================================*/
STRING
do_format_date (STRING str, INT dfmt, INT mfmt,
             INT yfmt, INT sfmt, INT efmt, INT cmplx)
{
	/* TODO: get rid of this function, and have callers call do_zformat_date */
	ZSTR zstr = do_zformat_date(str, dfmt, mfmt, yfmt, sfmt, efmt, cmplx);
	static char buffer[100];
	llstrsets(buffer, sizeof(buffer), uu8, zs_str(zstr));
	zs_free(&zstr);
	return buffer;
}
static ZSTR
do_zformat_date (STRING str, INT dfmt, INT mfmt,
             INT yfmt, INT sfmt, INT efmt, INT cmplx)
{
	STRING smo, syr;
	static char daystr[3];
	GDATEVAL gdv = 0;
	ZSTR zstr=zs_newn(40);
	
	if (!str) return zstr;

	initialize_if_needed();

	if (sfmt==12) {
		/* This is what used to be the shrt flag */
		zs_free(&zstr);
		return zshorten_date(str);
	}
	if (sfmt==14) {
		zs_sets(zstr, str);
		return zstr;
	}
	if (!cmplx) {
		/* simple */
		gdv = extract_date(str);
		format_day(gdv->date1.day, dfmt, daystr);
		smo = format_month(gdv->date1.calendar, gdv->date1.month, mfmt);
		syr = format_year(gdv->date1.year, yfmt);
		format_ymd(zstr, syr, smo, daystr, sfmt);
		format_eratime(zstr, gdv->date1.eratime, efmt);
		if (gdv->date1.calendar) {
			format_cal(zstr, gdv->date1.calendar);
		}
		free_gdateval(gdv);
		return zstr;
	} else {
		ZSTR zstr2 = zs_newn(40);
		ZSTR zstr3=0;
		/* complex (include modifier words) */
		gdv = extract_date(str);
		format_day(gdv->date1.day, dfmt, daystr);
		smo = format_month(gdv->date1.calendar, gdv->date1.month, mfmt);
		syr = (gdv->date1.year.str ? gdv->date1.year.str 
			: format_year(gdv->date1.year, yfmt));
		format_ymd(zstr, syr, smo, daystr, sfmt);
		format_eratime(zstr, gdv->date1.eratime, efmt);
		if (gdv->date1.calendar) {
			format_cal(zstr, gdv->date1.calendar);
		}
		if (gdateval_isdual(gdv)) {
			/* build 2nd date string into ymd2 */
			format_day(gdv->date2.day, dfmt, daystr);
			smo = format_month(gdv->date2.calendar, gdv->date2.month, mfmt);
			syr = (gdv->date2.year.str ? gdv->date2.year.str 
				: format_year(gdv->date2.year, yfmt));
			format_ymd(zstr2, syr, smo, daystr, sfmt);
			format_eratime(zstr2, gdv->date2.eratime, efmt);
			if (gdv->date2.calendar) {
				format_cal(zstr2, gdv->date2.calendar);
			}
		}
		zstr3 = format_complex(gdv, cmplx, zs_str(zstr), zs_str(zstr2));
		zs_free(&zstr);
		zs_free(&zstr2);
		free_gdateval(gdv);
		return zstr3;
	}
}
/*===================================================
 * format_eratime -- Add AD/BC info to date
 *  pdate:  [IN]  actual date information
 *  zstr:  [I/O] date string consisting of yr, mo, da portion
 *  efmt:   [IN]  eratime format code
 *                0 - no AD/BC marker
 *                1 - trailing B.C. if appropriate
 *                2 - trailing A.D. or B.C.
 *               11 - trailing BC if appropriate
 *               12 - trailng AD or BC
 *               21 - trailing B.C.E. if appropriate
 *               22 - trailing C.E. or B.C.E.
 *               31 - trailing BCE if appropriate
 *               32 - trailing CE or BCE
 * Created: 2001/12/28 (Perry Rapp)
 *=================================================*/
static void
format_eratime (ZSTR zstr, INT eratime, INT efmt)
{
	/* TODO: calendar-specific handling */
	if (eratime == GDV_BC) {
		if (efmt > 0) {
			STRING tag = 0;
			switch (efmt/10) {
				case 1: tag = _(qSdatetrl_bcB); break;
				case 2: tag = _(qSdatetrl_bcC); break;
				case 3: tag = _(qSdatetrl_bcD); break;
			}
			/* this way we handle if one is blank */
			if (!tag || !tag[0])
				tag = _(qSdatetrl_bcA);
			zs_apps(zstr, " ");
			zs_apps(zstr, tag);
			return;
		}
	} else {
		if (efmt > 1) {
			STRING tag = 0;
			switch (efmt/10) {
				case 1: tag = _(qSdatetrl_adB); break;
				case 2: tag = _(qSdatetrl_adC); break;
				case 3: tag = _(qSdatetrl_adD); break;
			}
			/* this way we handle if one is blank */
			if (!tag || !tag[0])
				tag = _(qSdatetrl_adA);
			zs_apps(zstr, " ");
			zs_apps(zstr, tag);
			return;
		}
	}
	/* no trailing tag at all */
}
/*===================================================
 * format_cal -- Add calender info to date
 *  cal:    [IN]  calendar number from date struct
 *  src:    [IN]  original date w/o calender info
 *  output: [I/O]  buffer for new version
 *  len:    [IN]  size of output buffer
 * Created: 2001/12/31 (Perry Rapp)
 *=================================================*/
static void
format_cal (ZSTR zstr, INT cal)
{
	ASSERT(cal>=0 && cal<ARRSIZE(calendar_pics));
	if (calendar_pics[cal]) {
		ZSTR zs2 = zprintpic1(calendar_pics[cal], zs_str(zstr));
		zs_move(zstr, &zs2);
	}
}
/*===================================================
 * get_cmplx_pic -- Get appropriate picture string
 *  examples:
 *   get_cmplx_pic(ECMPLX_BET_AND, 0) == "BET %1 AND %2"
 *   get_cmplx_pic(ECMPLX_BET_AND, 3) == "Between %1 And %2"
 *  (this checks for user-specified custom picture strings also)
 * Created: 2001/12/28 (Perry Rapp)
 *=================================================*/
static STRING
get_cmplx_pic (INT ecmplx, INT cmplxnum)
{
	/* load_lang already generated all the pictures */
	ASSERT(ecmplx>=0 && ecmplx<ECMPLX_END);
	ASSERT(cmplxnum>=0 && cmplxnum<6);
	if (cmplx_custom[ecmplx])
		return cmplx_custom[ecmplx];
	return cmplx_pics[ecmplx][cmplxnum];
}
/*===================================================
 * format_complex -- Format date string with modifiers
 *  gdv:    [IN]  actual date_val
 *  output: [I/O] whither to write string
 *  len:    [IN]  size of output
 *  cmplx:  [IN]  cmplx format code 
 *                 1=about (same as 8)
 *                 3=ABT
 *                 4=Abt
 *                 5=ABOUT
 *                 6=About
 *                 7=abt
 *                 8=about
 *  ymd2:   [IN]  formatted date1
 *  ymd3:   [IN]  formatted date2 (only used for dual dates)
 *                 (ie, full period or full range)
 * Created: 2001/12/28 (Perry Rapp)
 *=================================================*/
static ZSTR
format_complex (GDATEVAL gdv, INT cmplx	, STRING ymd2, STRING ymd3)
{
	STRING pic;
	ZSTR zstr=0;
	INT cmplxnum=cmplx-3; /* map cmplx to 0-5 */
	if (cmplxnum<0 || cmplxnum>5) cmplxnum=5;
	switch (gdv->type) {
	case GDV_PERIOD:
		switch (gdv->subtype) {
		case GDVP_FROM:
			pic = get_cmplx_pic(ECMPLX_FROM, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		case GDVP_TO:
			pic = get_cmplx_pic(ECMPLX_TO, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		case GDVP_FROM_TO:
			pic = get_cmplx_pic(ECMPLX_FROM_TO, cmplxnum);
			zstr = zprintpic2(pic, ymd2, ymd3);
			break;
		default:
			FATAL(); /* invalid period subtype */
			break;
		}
		break;
	case GDV_RANGE:
		switch (gdv->subtype) {
		case GDVR_BEF:
			pic = get_cmplx_pic(ECMPLX_BEF, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		case GDVR_AFT:
		case GDVR_BET: /* BET with no AND is treated as AFT */
			pic = get_cmplx_pic(ECMPLX_AFT, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		case GDVR_BET_AND:
			pic = get_cmplx_pic(ECMPLX_BET_AND, cmplxnum);
			zstr = zprintpic2(pic, ymd2, ymd3);
			break;
		default:
			FATAL(); /* invalid period subtype */
			break;
		}
		break;
	case GDV_APPROX:
		switch (gdv->subtype) {
		case GDVA_ABT:
			pic = get_cmplx_pic(ECMPLX_ABT, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		case GDVA_EST:
			pic = get_cmplx_pic(ECMPLX_EST, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		case GDVA_CAL:
			pic = get_cmplx_pic(ECMPLX_CAL, cmplxnum);
			zstr = zprintpic1(pic, ymd2);
			break;
		}
		break;
	case GDV_DATE:
	default:
		zstr = zs_news(ymd2);
		break;
	}
	return zstr;
}
/*===================================================
 * format_ymd -- Assembles date according to dateformat
 *  zstr:  [I/O]  resultant formatted zstring
 *  syr:    [IN]   year string
 *  smo:    [IN]   month string
 *  sda:    [IN]   day string
 *  sfmt:   [IN]   format code
 *                 0 - da mo yr
 *                 1 - mo da, yr
 *                 2 - mo/da/yr
 *                 3 - da/mo/yr
 *                 4 - mo-da-yr
 *                 5 - da-mo-yr
 *                 6 - modayr
 *                 7 - damoyr
 *                 8 - yr mo da
 *                 9 - yr/mo/da
 *                 10- yr-mo-da
 *                 11- yrmoda
 *                 12- yr   (year only, old short form)
 *                 13- dd/mo yr
 *                 14- as in GEDCOM (truncated to 50 chars)
 * This routine applies the custom date pic if present (date_pic)
 *=================================================*/
static void
format_ymd (ZSTR zstr, STRING syr, STRING smo, STRING sda, INT sfmt)
{
	zs_clear(zstr);

	if (date_pic) {
		ZSTR zs2 = zprintpic3(date_pic, syr, smo, sda);
		zs_move(zstr, &zs2);
		return;
	}
	switch (sfmt) {
	case 0:		/* da mo yr */
		if (sda) {
			zs_apps(zstr, sda);
			zs_appc(zstr, ' ');
		}
		if (smo) {
			zs_apps(zstr, smo);
			zs_appc(zstr, ' ');
		}
		if (syr) {
			zs_apps(zstr, syr);
		}
		break;
	case 1:		/* mo da, yr */
		if (smo) {
			zs_apps(zstr, smo);
			zs_appc(zstr, ' ');
		}
		if (sda) {
			zs_apps(zstr, sda);
			zs_apps(zstr, ", ");
		}
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 2:		/* mo/da/yr */
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, '/');
		if (sda)
			zs_apps(zstr, sda);
		zs_appc(zstr, '/');
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 3:		/* da/mo/yr */
		if (sda)
			zs_apps(zstr, sda);
		zs_appc(zstr, '/');
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, '/');
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 4:		/* mo-da-yr */
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, '-');
		if (sda)
			zs_apps(zstr, sda);
		zs_appc(zstr, '-');
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 5:		/* da-mo-yr */
		if (sda)
			zs_apps(zstr, sda);
		zs_appc(zstr, '-');
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, '-');
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 6:		/* modayr */
		if (smo)
			zs_apps(zstr, smo);
		if (sda)
			zs_apps(zstr, sda);
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 7:		/* damoyr */
		if (sda)
			zs_apps(zstr, sda);
		if (smo)
			zs_apps(zstr, smo);
		if (syr)
			zs_apps(zstr, syr);
		break;
	case 8:         /* yr mo da */
		if (syr)
			zs_apps(zstr, syr);
		if (smo) {
			zs_appc(zstr, ' ');
			zs_apps(zstr, smo);
		}
		if (sda) {
			zs_appc(zstr, ' ');
			zs_apps(zstr, sda);
		}
		break;
	case 9:         /* yr/mo/da */
		if (syr)
			zs_apps(zstr, syr);
		zs_appc(zstr, '/');
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, '/');
		if (sda)
			zs_apps(zstr, sda);
		break;
	case 10:        /* yr-mo-da */
		if (syr)
			zs_apps(zstr, syr);
		zs_appc(zstr, '-');
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, '-');
		if (sda)
			zs_apps(zstr, sda);
		break;
	case 11:        /* yrmoda */
		if (syr)
			zs_apps(zstr, syr);
		if (smo)
			zs_apps(zstr, smo);
		if (sda)
			zs_apps(zstr, sda);
		break;
	/* 12 (year only) was handled directly in do_format_date */
	case 13:      /* da/mo yr */
		if (sda)
			zs_apps(zstr, sda);
		zs_appc(zstr, '/');
		if (smo)
			zs_apps(zstr, smo);
		zs_appc(zstr, ' ');
		if (syr)
			zs_apps(zstr, syr);
		break;
	/* 14 (as GEDCOM) was handled directly in do_format_date */
        }
}

/*=======================================
 * format_day -- Formats day part of date
 *  day:   [IN]  numeric day (0 for unknown)
 *  dfmt:  [IN]    0 - num, space
 *                 1 - num, lead 0
 *                 2 - num, as is
 * output: [I/O] buffer in which to write
 *                must be at least 3 characters
 *=====================================*/
static void
format_day (struct tag_dnum da, INT dfmt, STRING output)
{
	STRING p;
	INT dayval = da.val; /* ignore complex days for now */
	if (dayval < 0 || dayval > 99 || dfmt < 0 || dfmt > 2) {
		output[0] = 0;
		return;
	}
	strcpy(output, "  ");
	if (dayval >= 10) {
		/* dfmt irrelevant with 2-digit days */
		output[0] = dayval/10 + '0';
		output[1] = dayval%10 + '0';
		return;
	}
	p = output;
	if (dayval == 0) {
		if (dfmt == 2)
			output[0] = 0;
		return;
	}
	if (dfmt == 0)
		p++; /* leading space */
	else if (dfmt == 1)
		*p++ = '0'; /* leading 0 */
	*p++ = dayval + '0';
	*p = 0;
}
/*==========================
 * gedcom_month -- return GEDCOM month keyword
 * Caller is responsible for cal & mo having legal values
 * Returns static string
 *========================*/
static STRING
gedcom_month (INT cal, INT mo)
{
	switch (cal) {
	case GDV_HEBREW:
		return gedkeys[mo+11].keyword;
	case GDV_FRENCH:
		return gedkeys[mo+24].keyword;
	default:
		return gedkeys[mo-1].keyword;
	}
}
/*===========================================
 * format_month -- Formats month part of date
 *  cal:   [IN]  calendar code (for named months)
 *  mo:    [IN]  numeric month (0 for unknown)
 *  mfmt:  [IN]    0 - num, space
 *                 1 - num, lead 0
 *                 2 - num, as is
 *                 3 - eg, MAR  (3-8 will be localized)
 *                 4 - eg, Mar
 *                 5 - eg, MARCH
 *                 6 - eg, March
 *                 7 - eg, mar
 *                 8 - eg, march
 *                 9 - eg, MAR (GEDCOM)
 *                10 - roman lowercase (eg, v for May)
 *                11 - roman uppercase (eg, V for May)
 *  TOD: Do we want space-extended roman ? Before or after ?
 *  returns static buffer or string constant or 0
 *=========================================*/
static STRING
format_month (INT cal, struct tag_dnum mo, INT mfmt)
{
	INT casing;
	MONTH_NAMES * parr=0;
	static char scratch[3];
	INT moval = mo.val; /* ignore complex months for now */
	if (moval < 0 || moval > 13 || mfmt < 0 || mfmt > 11) return NULL;
	if (mfmt <= 2)  {
		format_day(mo, mfmt, scratch);
		return scratch;
	}
	if (moval == 0) return (STRING) "   ";
	if (mfmt == 9)
		return gedcom_month(cal, moval);
	if (mfmt == 10)
		return roman_lower[moval-1];
	if (mfmt == 11)
		return roman_upper[moval-1];
	casing = mfmt-3;
	ASSERT(casing>=0 && casing<ARRSIZE(months_gj[0]));
	switch (cal) {
	case GDV_HEBREW: parr = months_heb; break;
	case GDV_FRENCH: parr = months_fr; break;
	default: 
		if (moval>12) return "   ";
		parr = months_gj; break;
	}
	if (parr[moval-1][casing])
		return parr[moval-1][casing];
	else
		return "?";
}
/*=========================================
 * format_year -- Formats year part of date
 *  yr:    [IN]  numeric year (0 for unknown)
 *  yfmt:  [IN]    0 - num, space
 *                 1 - num, lead 0
 *                 2 - num, as is
 *
 * (No point in supporting negative numbers here, because parser only
 *  picks up positive numbers.)
 *  returns static buffer or 0
 *=======================================*/
static STRING
format_year (struct tag_dnum yr, INT yfmt)
{
	static char scratch[7];
	STRING p;
	INT yrval = yr.val; /* ignore complex years for now */
	if (yrval > 9999 || yrval < 0) {
		switch(yfmt) {
		case 0:
			return "    ";
		case 1:
			return "0000";
		default:
			return NULL;
		}
	}
	if (yrval > 999 || yfmt == 2) {
		sprintf(scratch, "%d", yrval);
		return scratch;
	}
	p = (yfmt==1 ? "000" : "   ");
	if (yrval < 10)
		strcpy(scratch, p);
	else if (yrval < 100)
		llstrncpy(scratch, p, 2+1, uu8);
	else
		llstrncpy(scratch, p, 1+1, uu8);
	sprintf(scratch+strlen(scratch), "%d", yrval);
	return scratch;
}
/*=====================================================
 * mark_invalid -- Set a gdate_val to invalid
 *  gdv:  [I/O]  date_val we are building
 *===================================================*/
static void
mark_invalid (GDATEVAL gdv)
{
	gdv->valid = -1;
}
/*=====================================================
 * mark_freeform -- Set a gdate_val to freeform (unless invalid)
 *  gdv:  [I/O]  date_val we are building
 *===================================================*/
static void
mark_freeform (GDATEVAL gdv)
{
	if (gdv->valid > 0)
		gdv->valid = 0;
}
/*=====================================================
 * extract_date -- Extract date from free format string
 *  str:  [IN]  date to parse
 * returns new date_val
 *===================================================*/
GDATEVAL
extract_date (STRING str)
{
	/* we accumulate numbers to figure when we finish a
	date (with a full period or range, we may finish the
	first date partway thru) */
	INT tok;
	struct tag_dnum dnum = {BAD_YEAR, 0, 0};
	struct tag_nums nums = { {BAD_YEAR, 0, 0}, {BAD_YEAR, 0, 0}, {BAD_YEAR, 0, 0} };
	GDATEVAL gdv = create_gdateval();
	struct tag_gdate * pdate = &gdv->date1;
	BOOLEAN newdate;
	if (!str)
		return gdv;
	set_date_string(str);
	while ((tok = get_date_tok(&dnum))) {
		switch (tok) {
		case MONTH_TOK:
			if (!pdate->month.val) {
				assign_dnum(&pdate->month, &dnum);
				if (nums.num1.val != BAD_YEAR) {
					/* if single number before month, it is a day if legal */
					if (nums.num2.val == BAD_YEAR 
						&& is_valid_day(pdate, nums.num1)) {
						assign_dnum(&pdate->day, &nums.num1);
					} else {
						mark_freeform(gdv);
					}
				}
			}
			else
				mark_invalid(gdv);
			continue;
		case CALENDAR_TOK:
			if (!pdate->calendar)
				pdate->calendar = dnum.val;
			else
				mark_invalid(gdv);
			continue;
		case YEAR_TOK:
			if (pdate->year.val == BAD_YEAR) {
				assign_dnum(&pdate->year, &dnum);
			} else {
				mark_invalid(gdv);
			}
			continue;
		case CHAR_TOK:
			/* this was anything unrecognized, including unusable
			numeric strings */
			mark_freeform(gdv);
			continue;
		case WORD_TOK:
			analyze_word(gdv, pdate, &nums, dnum.val, &newdate);
			if (newdate) {
				analyze_numbers(gdv, pdate, &nums);
				clear_numbers(&nums);
				pdate = &gdv->date2;
			}
			continue;
		case ICONS_TOK:
			/* number */
			if (nums.num1.val == BAD_YEAR)
				assign_dnum(&nums.num1, &dnum);
			else if (nums.num2.val == BAD_YEAR)
				assign_dnum(&nums.num2, &dnum);
			else if (nums.num3.val == BAD_YEAR)
				assign_dnum(&nums.num3, &dnum);
			else
				mark_freeform(gdv);
			continue;
		default:
			FATAL();
		}
	}
	/* now analyze what numbers we got */
	analyze_numbers(gdv, pdate, &nums);
	clear_numbers(&nums);
	gdv->text = strsave(str);
	return gdv;
}
/*===============================================
 * analyze_word -- Interpret word found in date parsing
 *  gdv:     [I/O] current date_val we are building
 *  pdate:   [IN]  points to which date we're on
 *                 (&gdv->date1, unless finishing a range or period)
 *  nums     [I/O] accumulated potential numbers
 *  ival:    [IN]  word enum value (eg, GD_AFT)
 *  newdate: [OUT] flag we set if we are switching to 2nd date now
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
static void
analyze_word (GDATEVAL gdv, struct tag_gdate * pdate, struct tag_nums * nums
	, INT ival, BOOLEAN * newdate)
{
	/* GEDCOM word modifiers */
	*newdate = FALSE;
	if (gdv->type) {
		/* already have a modifier -- very few 2nd modifiers are allowed */
		switch (ival) {
		case GD_AND:
			if (gdv->type==GDV_RANGE && gdv->subtype==GDVR_BET) {
				gdv->subtype = GDVR_BET_AND;
				*newdate = TRUE;
			} else {
				mark_invalid(gdv);
			}
			break;
		case GD_TO:
			if (gdv->type==GDV_PERIOD && gdv->subtype==GDVP_FROM) {
				gdv->subtype = GDVP_FROM_TO;
				*newdate = TRUE;
			} else {
				mark_invalid(gdv);
			}
			break;
		case GD_AD:
			if (pdate->eratime)
				mark_invalid(gdv);
			else {
				mark_freeform(gdv); /* AD is not in GEDCOM */
				pdate->eratime = GDV_AD;
			}
			break;
		case GD_BC:
			if (pdate->eratime)
				mark_invalid(gdv);
			else
				pdate->eratime = GDV_BC;
			break;
		default:
			mark_invalid(gdv);
			break;
		}
	} else {
		/* first modifier */
		switch (ival) {
		case GD_ABT:
			gdv->type = GDV_APPROX;
			gdv->subtype = GDVA_ABT;
			break;
		case GD_EST:
			gdv->type = GDV_APPROX;
			gdv->subtype = GDVA_EST;
			break;
		case GD_CAL:
			gdv->type = GDV_APPROX;
			gdv->subtype = GDVA_CAL;
			break;
		case GD_BEF:
			gdv->type = GDV_RANGE;
			gdv->subtype = GDVR_BEF;
			break;
		case GD_AFT:
			gdv->type = GDV_RANGE;
			gdv->subtype = GDVR_AFT;
			break;
		case GD_BET:
			gdv->type = GDV_RANGE;
			gdv->subtype = GDVR_BET;
			break;
		/* AND is not a legal first modifier */
		case GD_FROM:
			gdv->type = GDV_PERIOD;
			gdv->subtype = GDVP_FROM;
			break;
		case GD_TO:
			if (pdate->day.val || pdate->month.val || pdate->year.val != BAD_YEAR
				|| nums->num1.val != BAD_YEAR) {
				/* if we have a date before TO, switch to 2nd date */
				/* (This is not legal GEDCOM syntax, however */
				*newdate = TRUE;
				gdv->type = GDV_PERIOD;
				gdv->subtype = GDVP_FROM_TO;
				analyze_numbers(gdv, pdate, nums);
				mark_freeform(gdv);
			} else {
				gdv->type = GDV_PERIOD;
				gdv->subtype = GDVP_TO;
			}
			break;
		case GD_AD:
			if (pdate->eratime)
				mark_invalid(gdv);
			else {
				mark_freeform(gdv); /* AD is not in GEDCOM */
				pdate->eratime = GDV_AD;
			}
			break;
		case GD_BC:
			if (pdate->eratime)
				mark_invalid(gdv);
			else
				pdate->eratime = GDV_BC;
			break;
		default:
			mark_invalid(gdv);
			break;
		}
	}
}
/*===============================================
 * analyze_numbers -- Parse numbers found in date
 *  gdv:  [I/O]  date_val we are building
 *  pdate: [IN]  pointer to current date (usually &gdv->date1)
 *  nums:  [IN]  unassigned numbers found in date line
 * This function does not clear the numbers -- caller must do so.
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
static void
analyze_numbers (GDATEVAL gdv, struct tag_gdate * pdate, struct tag_nums * nums)
{
	if (nums->num1.val == BAD_YEAR) {
		/* if we have no numbers, we're done */
		return;
	}
	/* we have at least 1 number */
	if (pdate->day.val && pdate->month.val && pdate->year.val != BAD_YEAR) {
		/* if we already have day & month & year, we're done */
		return;
	}
	/* we need something */
	if (nums->num2.val == BAD_YEAR) {
		/* if we only have 1 number */
		if (pdate->year.val == BAD_YEAR) {
			/* if we need year, it is year */
			assign_dnum(&pdate->year, &nums->num1);
			return;
		}
		if (pdate->month.val && is_valid_day(pdate, nums->num1)) {
			/* if we only need day, it is day (if legal) */
			assign_dnum(&pdate->day, &nums->num1);
			return;
		}
		/* otherwise give up (ignore it) */
		return;
	}
	/* we have at least 2 numbers */
	if (pdate->day.val && pdate->month.val) {
		/* if all we need is year, then it is year */
		assign_dnum(&pdate->year, &nums->num1);
		return;
	}
	/* we need at least day or month */
	/* and we have at least 2 numbers */
	mark_freeform(gdv);
	if (pdate->month.val && pdate->year.val != BAD_YEAR) {
		/* if all we need is day, see if it can be day */
		if (is_valid_day(pdate, nums->num1)) {
			assign_dnum(&pdate->day, &nums->num1);
		}
		return;
	}
	if (pdate->month.val) {
		/* if we get here, we need day & year */
		/* prefer first num for day, if legal */
		if (is_valid_day(pdate, nums->num1)) {
			assign_dnum(&pdate->day, &nums->num1);
			assign_dnum(&pdate->year, &nums->num2);
		} else {
			assign_dnum(&pdate->year, &nums->num1);
			if (is_valid_day(pdate, nums->num2))
				assign_dnum(&pdate->day, &nums->num2);
		}
		return;
	}
	/*
	if we get here, we need at least month and have 2+ numbers
	if we don't know month, then we don't know day either, as
	we only recognize day during parsing if we see it before month
	*/
	ASSERT(!pdate->day.val);
	/* so we need at least day & month, & have 2+ numbers */
	
	if (pdate->year.val != BAD_YEAR) {
		/* we need day & month, but not year, and have 2+ numbers */
		/* can we interpret them unambiguously ? */
		if (is_valid_month(pdate, nums->num1) 
			&& !is_valid_month(pdate, nums->num2)
			&& is_valid_day(pdate, nums->num2)) 
		{
			assign_dnum(&pdate->month, &nums->num1);
			assign_dnum(&pdate->day, &nums->num2);
			return;
		}
		if (is_valid_month(pdate, nums->num2) 
			&& !is_valid_month(pdate, nums->num1)
			&& is_valid_day(pdate, nums->num1)) 
		{
			assign_dnum(&pdate->month, &nums->num2);
			assign_dnum(&pdate->day, &nums->num1);
			return;
		}
		/* not unambiguous, so don't guess */
		return;
	}
	/* if we get here, we need day, month, & year, and have 2+ numbers */
	if (nums->num3.val == BAD_YEAR) {
		/* we need day, month, & year, and have 2 numbers */
		/* how about day, year ? */
		if (is_valid_day(pdate, nums->num1)) {
			assign_dnum(&pdate->day, &nums->num1);
			assign_dnum(&pdate->year, &nums->num2);
		}
		/* how about year, day ? */
		if (is_valid_day(pdate, nums->num2)) {
			assign_dnum(&pdate->day, &nums->num2);
			assign_dnum(&pdate->year, &nums->num1);
		}
		/* give up */
		return;
	}
	/* we need day, month, & year, and have 3 numbers */
	/* how about day, month, year ? */
	if (is_valid_day(pdate, nums->num1) && is_valid_month(pdate, nums->num2)) {
		assign_dnum(&pdate->day, &nums->num1);
		assign_dnum(&pdate->month, &nums->num2);
		assign_dnum(&pdate->year, &nums->num3);
	}
	/* how about month, day, year ? */
	if (is_valid_month(pdate, nums->num1) && is_valid_day(pdate, nums->num2)) {
		assign_dnum(&pdate->day, &nums->num2);
		assign_dnum(&pdate->month, &nums->num1);
		assign_dnum(&pdate->year, &nums->num3);
	}
	/* how about year, month, day ? */
	if (is_valid_day(pdate, nums->num3) && is_valid_month(pdate, nums->num2)) {
		assign_dnum(&pdate->day, &nums->num3);
		assign_dnum(&pdate->month, &nums->num2);
		assign_dnum(&pdate->year, &nums->num1);
	}
	/* give up */
}
/*===============================================
 * clear_dnum -- Empty a dnums_s structure
 *  nums:  [I/O]  date_val we are clearing
 * Created: 2002/02/03 (Perry Rapp)
 *=============================================*/
static void
clear_dnum (struct tag_dnum * dnum)
{
	dnum->val = dnum->val2 = BAD_YEAR;
	if (dnum->str) {
		stdfree(dnum->str);
		dnum->str = 0;
	}
}
/*===============================================
 * clear_numbers -- Empty a nums_s structure
 *  nums:  [I/O]  date_val we are clearing
 * Created: 2002/02/03 (Perry Rapp)
 *=============================================*/
static void
clear_numbers (struct tag_nums * nums)
{
	clear_dnum(&nums->num1);
	clear_dnum(&nums->num2);
	clear_dnum(&nums->num3);
}
/*===============================================
 * assign_dnum -- Move dnum from one variable to another
 * Created: 2002/02/03 (Perry Rapp)
 *=============================================*/
static void
assign_dnum (struct tag_dnum * dest, struct tag_dnum * src)
{
	dest->val = src->val;
	dest->val2 = src->val2;
	dest->str = src->str;
	src->str = 0; /* transferring string to dest */
	src->val = BAD_YEAR;
	src->val2 = BAD_YEAR;
}
/*===============================================
 * create_gdateval -- Create new, empty GEDCOM date_val
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
GDATEVAL
create_gdateval (void)
{
	GDATEVAL gdv = (GDATEVAL)stdalloc(sizeof(*gdv));
	memset(gdv, 0, sizeof(*gdv));
	gdv->date1.year.val = BAD_YEAR;
	gdv->date2.year.val = BAD_YEAR;
	gdv->valid = 1;
	return gdv;

}
/*===============================================
 * free_gdate -- Delete existing GEDCOM date
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
void
free_gdate (struct tag_gdate * gdate)
{
	clear_dnum(&gdate->year);
	clear_dnum(&gdate->month);
	clear_dnum(&gdate->day);
}
/*===============================================
 * free_gdateval -- Delete existing GEDCOM date_val
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
void
free_gdateval (GDATEVAL gdv)
{
	if (!gdv) return;
	free_gdate(&gdv->date1);
	free_gdate(&gdv->date2);
	if (gdv->text)
		stdfree(gdv->text);
	stdfree(gdv);
}
/*===============================================
 * set_date_string -- Store date extraction string
 *  in static buffer for use during subsequent parsing
 *=============================================*/
static void
set_date_string (STRING str)
{
	sstr = str;
	sstr_start = str;
	initialize_if_needed();
}
/*==================================================
 * get_date_tok -- Return next date extraction token
 *  pdnum:   [OUT]  numeric value of token, if day/year number
 *                  or numeric value of month or calendar or keyword
 *  psval:   [OUT]  pointer to (static) copy of original text
 *                   (only used for slash years)
 *================================================*/
static INT
get_date_tok (struct tag_dnum *pdnum)
{
	static char scratch[90];
	STRING p = scratch;
	INT c;
	/* flag if token preceded by whitespace (or at start of buffer) */
	BOOLEAN white_before = FALSE;
	if (!sstr) return 0;
	if (strlen(sstr) > sizeof(scratch)-1) return 0;
	while (iswhite((uchar)*sstr++))
		;
	sstr--;
	white_before = (sstr==sstr_start || iswhite((uchar)sstr[-1]));
	if (sstr[0]=='@' && sstr[1]=='#' && sstr[2]=='D') {
		INT i;
		/* collect calendar escape to closing @ (or end of string) */
		do {
			*p++ = *sstr++;
		} while (sstr[0] && sstr[0]!='@');
		if (*sstr != '@') {
			return CHAR_TOK;
		} else {
			*p++ = *sstr++; /* consume the '@' */
			*p = 0;
		}
		/* look it up in our big table of GEDCOM keywords */
		i = valueof_int(keywordtbl, upperascii_s(scratch));
		if (i >= 2001 && i < 2000 + GDV_CALENDARS_IX) {
			pdnum->val = i - 2000;
			return CALENDAR_TOK;
		}
		/* unrecognized word */
		return CHAR_TOK;
	}
	if (isletter((uchar)*sstr)) {
		INT i;
		/* collect all letters (to end or whitespace or closeparen) */
		do {
			*p++ = *sstr++;
		} while (sstr[0] && sstr[0]!=')' && !iswhite((uchar)sstr[0]));
		*p = 0;
		/* look it up in our big table of GEDCOM keywords */
		i = valueof_int(keywordtbl, upperascii_s(scratch));
		if (!i) {
			/* unrecognized word */
			return CHAR_TOK;
		}
		if (i > 0 && i <= 999) {
			pdnum->val = i % 100;
			/* TODO: we need to use the fact that calendar is i/100 */
			/* That is, now we know what calendar this is in */
			return MONTH_TOK;
		}
		pdnum->val = 0;
		if (i >= 1001 && i < 1000 + GD_END2) {
			pdnum->val = i - 1000;
			return WORD_TOK;
		}
		FATAL(); /* something unexpected is in the keywordtbl ? Find out what! */
		return WORD_TOK;
	}
	if (chartype((uchar)*sstr) == DIGIT) {
		INT i=0; /* primary numeric value */
		INT j=BAD_YEAR; /* secondary numeric value (for compound number) */
		while (chartype(c = (uchar)(*p++ = *sstr++)) == DIGIT)
			i = i*10 + c - '0';
		if (i > 9999) {
			/* 5+ digit numbers are not recognized */
			return CHAR_TOK;
		}
		/* c is the char after the last digit,
		and sstr is the next char after that */
		/* check for compound number, if preceding whitespace */
		if ((c=='/' || c=='-') && white_before) {
			INT modnum=1;
			signed int delta;
			STRING saves = sstr, savep = p;
			char csave = c;
			j=0;
			while (chartype(c = (uchar)(*p++ = *sstr++)) == DIGIT) {
				modnum *= 10;
				j = j*10 + c - '0';
			}
			/* 2nd number must be larger than first (subject to mod)
			eg, 1953-54 is ok, but not 1953-52
			also must be followed by whitespace (or be at end) */
			delta = j - i % modnum;
			if (delta > 0 && (!c || iswhite((uchar)c))
				&& (csave == '-' || delta == 1)) {

				*--p = 0;
				pdnum->val = i;
				pdnum->val2 = i + delta;
				pdnum->str = strsave(scratch);
				if (is_valid_day(NULL, *pdnum))
					return ICONS_TOK;
				else
					return YEAR_TOK;
			}
			/* pop back to before slash/hyphen, so it can be handled as
			a number before a date delimiter */
			sstr = saves;
			p = savep;
		} else if ((c == 's' || c == 'S')
			&& (i % 10 == 0)
			&& (!sstr[0] || iswhite((uchar)sstr[0]))) {
			/* eg, 1850s -- this is English-specific */
			p[1] = 0;
			pdnum->val = i;
			pdnum->val2 = i+9;
			pdnum->str = strsave(scratch);
			if (is_valid_day(NULL, *pdnum))
				return ICONS_TOK;
			else
				return YEAR_TOK;
		}
		*--p = 0;
		sstr--;
		if (*sstr && !is_date_delim(*sstr)) {
			/* number only valid if followed by date delimiter */
			return CHAR_TOK;
		}
		pdnum->val = i;
		pdnum->val2 = i;
		pdnum->str = 0;
		return ICONS_TOK;
	}
	if (*sstr == 0)  {
		sstr = NULL;
		return 0;
	}
	++sstr;
	return CHAR_TOK;
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
	/* TODO: We need to load months of other calendars here */

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
 * load_one_cmplx_pic -- Generate case variations
 *  of one complex picture string.
 * Created: 2001/12/30 (Perry Rapp)
 *===========================*/
static void
load_one_cmplx_pic (INT ecmplx, STRING abbrev, STRING full)
{
	STRING loc_abbrev = strsave(abbrev);
	STRING loc_full = strsave(full);
	ASSERT(ecmplx>=0 && ecmplx <ECMPLX_END);
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
 * get_todays_date -- Get today's date
 *  returns static buffer
 *===========================*/
STRING
get_todays_date (void)
{
	struct tm *pt;
	time_t curtime;
	static char dat[20];
	STRING month;
	curtime = time(NULL);
	pt = localtime(&curtime);
	initialize_if_needed();
	/* TODO: Should this be one of the customizable formats ? */
	month = gedkeys[pt->tm_mon].keyword;
	sprintf(dat, "%d %s %d", pt->tm_mday, month, 1900 + pt->tm_year);
	return dat;
}
/*=============================
 * initialize_if_needed -- init module or reload language
 *===========================*/
static void
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
/*=============================
 * gdateval_isdual -- Does gdateval contain
 * two dates ?
 *===========================*/
BOOLEAN
gdateval_isdual (GDATEVAL gdv)
{
	if (gdv->type == GDV_PERIOD)
		return (gdv->subtype == GDVP_FROM_TO);
	else if (gdv->type == GDV_RANGE)
		return (gdv->subtype == GDVR_BET_AND);
	return FALSE;
}
/*=============================
 * is_valid_day -- Is this day legal for this date ?
 *  pdate:  [IN]  date in which day occurred (may be NULL)
 *  day:    [IN]  candidate day number
 * Created: 2001/12/28 (Perry Rapp)
 *===========================*/
static BOOLEAN
is_valid_day (struct tag_gdate * pdate, struct tag_dnum day)
{
	/* To consider: Fancy code with calendars */
	/* for now, use max (all cals all months), which is 31 */
	pdate=pdate; /* unused */
	return (day.val>=1 && day.val2<=31);
}
/*=============================
 * is_valid_month -- Is this month legal for this date ?
 *  pdate:  [IN]  date in which month occurred (may be NULL)
 *  month:    [IN]  candidate month number
 * Created: 2001/12/28 (Perry Rapp)
 *===========================*/
static BOOLEAN
is_valid_month (struct tag_gdate * pdate, struct tag_dnum month)
{
	INT cal = pdate ? pdate->calendar : 0;
	switch (cal) {
	case GDV_HEBREW:
	case GDV_FRENCH:
		return (month.val>=1 && month.val2<=13);
	default:
		return (month.val>=1 && month.val2<=12);
	}
}
/*=============================
 * is_date_delim -- Is this a valid character to end
 *  a number in a date ?
 * Created: 2001/12/28 (Perry Rapp)
 *===========================*/
static BOOLEAN
is_date_delim (char c)
{
	if (iswhite((uchar)c))
		return TRUE;
	/* TODO: Any other characters here ? Do we internationalize it ? */
	if (c=='/' || c=='-' || c=='.' || c==',')
		return TRUE;
	return FALSE;
}
/*=============================
 * set_cmplx_pic -- Set a custom complex date picture
 *  NULL or empty string will clear any existing custom pic
 * returns FALSE if invalid argument.
 * Created: 2001/12/30 (Perry Rapp)
 *===========================*/
BOOLEAN
set_cmplx_pic (INT ecmplx, STRING pic)
{
	if (ecmplx<0 || ecmplx>=ECMPLX_END)
		return FALSE;
	if (cmplx_custom[ecmplx]) {
		stdfree(cmplx_custom[ecmplx]);
		cmplx_custom[ecmplx] = 0;
	}
	if (pic && pic[0])
		cmplx_custom[ecmplx] = strsave(pic);
	return TRUE;
}
/*=============================
 * set_date_pic -- Set a custom ymd date picture
 *  NULL or empty string will clear any existing custom pic
 * Created: 2001/12/30 (Perry Rapp)
 *===========================*/
void
set_date_pic (STRING pic)
{
	if (date_pic) {
		stdfree(date_pic);
		date_pic = 0;
	}
	if (pic && pic[0]) {
		STRING p;
		date_pic = strsave(pic);
		/* convert %y %m %d format to %1 %2 %3 */
		for (p = date_pic; *p; ++p) {
			if (p[0]=='%') {
				if (p[1]=='y')
					p[1]='1';
				else if (p[1]=='m')
					p[1]='2';
				else if (p[1]=='d')
					p[1]='3';
			}
		}
	}
}
/*=============================
 * update_lang -- Adjust for new translation language
 *===========================*/
void
date_update_lang (void)
{
	lang_changed = TRUE;
}
/*================================================
 * shorten_date -- Return short form of date value
 * Returns static buffer.
 *==============================================*/
STRING
shorten_date (STRING date)
{
	static char buffer[3][MAXLINELEN+1];
	static int dex = 0;
	STRING p = date, q;
	INT c, len;
	/* Allow 3 or 4 digit years. The previous test for strlen(date) < 4
	 * prevented dates consisting of only 3 digit years from being
	 * returned. - pbm 12 oct 99 */
	if (!date || (INT) strlen(date) < 3) return NULL;
	if (++dex > 2) dex = 0;
	q = buffer[dex];
	while (TRUE) {
		while ((c = (uchar)*p++) && chartype(c) != DIGIT)
			;
		if (c == 0) return NULL;
		q = buffer[dex];
		*q++ = c;
		len = 1;
		while ((c = (uchar)*p++) && chartype(c) == DIGIT) {
			if (len < 6) {
				*q++ = c;
				len++;
			}
		}
		*q = 0;
		if (strlen(buffer[dex]) == 3 || strlen(buffer[dex]) == 4)
			return buffer[dex];
		if (c == 0) return NULL;
	}
}
static ZSTR
zshorten_date (STRING str)
{
	STRING sht = shorten_date(str);
	ZSTR zstr=zs_new();
	zs_sets(zstr, sht);
	return zstr;
}
/*====================================+
 * approx_time -- display duration specified in seconds
 *  eg, approx_time(70000) returns "19h26m"
 *===================================*/
ZSTR
approx_time (INT seconds)
{
	INT minutes, hours, days, years;
	minutes = seconds/60;
	if (!minutes) {
		/* TRANSLATORS: seconds time interval */
		return zs_newf(_("%02ds"), seconds);
	}
	seconds = seconds - minutes*60;
	hours = minutes/60;
	if (!hours) {
		/* TRANSLATORS: minutes & seconds time interval */
		return zs_newf(_("%dm%02ds"), minutes, seconds);
	}
	minutes = minutes - hours*60;
	days = hours/60;
	if (!days) {
		/* TRANSLATORS: hours & minutes time interval */
		return zs_newf(_("%dh%02dm"), hours, minutes);
	}
	hours = hours - days*24;
	years = days/365.2425;
	if (!years) {
		/* TRANSLATORS: days & hours time interval */
		return zs_newf(_("%dd%02dh"), days, hours);
	}
	days = days - years*365.2425;
	/* TRANSLATORS: years & days time interval */
	return zs_newf( _("%dy%03dd"), years, days);
}
