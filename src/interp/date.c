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

#include "sys_inc.h"
#include <time.h>
#include "llstdlib.h"
#include "table.h"
#include "interp.h"


/*********************************************
 * external/imported variables
 *********************************************/

extern STRING datea_abtA,datea_abtB,datea_estA,datea_estB,datea_calA;
extern STRING datea_calB,datep_fromA,datep_fromB,datep_toA,datep_toB;
extern STRING datep_frtoA,datep_frtoB,dater_befA,dater_befB,dater_aftA;
extern STRING dater_aftB,dater_betA,dater_betB;
extern STRING datetrl_bcA,datetrl_bcB,datetrl_bcC,datetrl_bcD;
extern STRING datetrl_adA,datetrl_adB,datetrl_adC,datetrl_adD;
extern STRING caljul,calheb,calfr,calrom;
extern STRING mon_gj1A,mon_gj1B,mon_gj2A,mon_gj2B,mon_gj3A,mon_gj3B;
extern STRING mon_gj4A,mon_gj4B,mon_gj5A,mon_gj5B,mon_gj6A,mon_gj6B;
extern STRING mon_gj7A,mon_gj7B,mon_gj8A,mon_gj8B,mon_gj9A,mon_gj9B;
extern STRING mon_gj10A,mon_gj10B,mon_gj11A,mon_gj11B,mon_gj12A,mon_gj12B;
extern STRING mon_heb1A,mon_heb1B,mon_heb2A,mon_heb2B,mon_heb3A,mon_heb3B;
extern STRING mon_heb4A,mon_heb4B,mon_heb5A,mon_heb5B,mon_heb6A,mon_heb6B;
extern STRING mon_heb7A,mon_heb7B,mon_heb8A,mon_heb8B,mon_heb9A,mon_heb9B;
extern STRING mon_heb10A,mon_heb10B,mon_heb11A,mon_heb11B;
extern STRING mon_heb12A,mon_heb12B,mon_heb13A,mon_heb13B;
extern STRING mon_fr1A,mon_fr1B,mon_fr2A,mon_fr2B,mon_fr3A,mon_fr3B;
extern STRING mon_fr4A,mon_fr4B,mon_fr5A,mon_fr5B,mon_fr6A,mon_fr6B;
extern STRING mon_fr7A,mon_fr7B,mon_fr8A,mon_fr8B,mon_fr9A,mon_fr9B;
extern STRING mon_fr10A,mon_fr10B,mon_fr11A,mon_fr11B;
extern STRING mon_fr12A,mon_fr12B,mon_fr13A,mon_fr13B;

/*********************************************
 * local types used in local function prototypes
 *********************************************/

/* used in parsing dates -- 1st, 2nd, & 3rd numbers found */
struct nums_s { INT num1; INT num2; INT num3; };

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void analyze_numbers(GDATEVAL, struct gdate_s *, struct nums_s *);
static void analyze_word(GDATEVAL gdv, struct gdate_s * pdate
	, struct nums_s * nums, INT ival, BOOLEAN * newdate);
static void format_cal(INT cal, CNSTRING src, STRING output, INT len);
static void format_complex(GDATEVAL gdv, STRING output, INT len, INT cmplx
	, STRING ymd2, STRING ymd3);
static void format_day(INT da, INT dfmt, STRING output);
static STRING format_month(INT cal, INT mo, INT mfmt);
static void format_eratime(struct gdate_s * pdate, CNSTRING ymd, INT efmt
	, STRING output, INT len);
static STRING format_year(INT, INT);
static void format_ymd(STRING, STRING, STRING, INT, STRING*, INT *len);
static INT get_date_tok(INT*, STRING*);
static void init_keywordtbl(void);
static BOOLEAN is_date_delim(char c);
static BOOLEAN is_valid_day(struct gdate_s * pdate, INT day);
static BOOLEAN is_valid_month(struct gdate_s * pdate, INT month);
static void load_lang(void);
static void mark_freeform(GDATEVAL gdv);
static void mark_invalid(GDATEVAL gdv);
static void set_date_string(STRING);
static void set_year(struct gdate_s * pdate, INT yr);

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
	,{ "BC", GD_BC }
	,{ "B.C.E.", GD_BC }
	,{ "BCE", GD_BC }
	,{ "A.D.", GD_AD }
	,{ "AD", GD_AD }
	,{ "C.E.", GD_AD }
	,{ "CE", GD_AD }
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


static STRING sstr = NULL;
static TABLE keywordtbl = NULL;

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
	STRING smo, syr;
	static char daystr[3], ymd[60], ymd2[60], ymd3[60], complete[100];
	STRING p;
	INT len;
	GDATEVAL gdv = 0;
	if (!str) return NULL;
	if (sfmt==12) {
		/* This is what used to be the shrt flag */
		return shorten_date(str);
	}
	if (sfmt==14) {
		llstrncpy(complete, str, sizeof(complete));
		return complete;
	}
	if (!cmplx) {
		/* simple */
		gdv = extract_date(str);
		format_day(gdv->date1.day, dfmt, daystr);
		smo = format_month(gdv->date1.calendar, gdv->date1.month, mfmt);
		syr = format_year(gdv->date1.year, yfmt);
		p = ymd;
		len = sizeof(ymd);
		*p = 0;
		format_ymd(syr, smo, daystr, sfmt, &p, &len);
		if (gdv->date1.calendar) {
			format_eratime(&gdv->date1, ymd, efmt, ymd2, sizeof(ymd2));
			format_cal(gdv->date1.calendar, ymd2, ymd3, sizeof(ymd3));
		} else {
			format_eratime(&gdv->date1, ymd, efmt, ymd3, sizeof(ymd3));
		}
		free_gdateval(gdv);
		return ymd3;
	} else {
		/* complex (include modifier words) */
		gdv = extract_date(str);
		format_day(gdv->date1.day, dfmt, daystr);
		smo = format_month(gdv->date1.calendar, gdv->date1.month, mfmt);
		syr = (gdv->date1.yearstr ? gdv->date1.yearstr 
			: format_year(gdv->date1.year, yfmt));
		p = ymd;
		len = sizeof(ymd);
		*p = 0;
		format_ymd(syr, smo, daystr, sfmt, &p, &len);
		if (gdv->date1.calendar) {
			format_eratime(&gdv->date1, ymd, efmt, ymd2, sizeof(ymd2));
			format_cal(gdv->date1.calendar, ymd2, ymd3, sizeof(ymd3));
		} else {
			format_eratime(&gdv->date1, ymd, efmt, ymd3, sizeof(ymd3));
		}
		if (gdateval_isdual(gdv)) {
			/* build 2nd date string into ymd2 */
			format_day(gdv->date2.day, dfmt, daystr);
			smo = format_month(gdv->date2.calendar, gdv->date2.month, mfmt);
			syr = (gdv->date2.yearstr ? gdv->date2.yearstr 
				: format_year(gdv->date2.year, yfmt));
			p = ymd;
			len = sizeof(ymd);
			*p = 0;
			format_ymd(syr, smo, daystr, sfmt, &p, &len);
			if (gdv->date2.calendar) {
				format_eratime(&gdv->date2, ymd, efmt, complete, sizeof(complete));
				format_cal(gdv->date2.calendar, complete, ymd2, sizeof(ymd2));
			} else {
				format_eratime(&gdv->date2, ymd, efmt, ymd2, sizeof(ymd2));
			}
		} else {
			ymd2[0] = 0;
		}
		format_complex(gdv, complete, sizeof(complete), cmplx, ymd3, ymd2);
		free_gdateval(gdv);
		return complete;
	}
}
/*===================================================
 * format_eratime -- Add AD/BC info to date
 *  pdate:  [IN]  actual date information
 *  ymd:    [IN]  date string consisting of yr, mo, da portion
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
 *  output: [IN]  buffer in which to write
 *  len:    [IN]  size of buffer
 * Created: 2001/12/28 (Perry Rapp)
 *=================================================*/
static void
format_eratime (struct gdate_s * pdate, CNSTRING ymd, INT efmt, STRING output
	, INT len)
{
	/* TODO: calendar-specific handling */
	if (pdate->eratime == GDV_BC) {
		if (efmt > 0) {
			STRING p = output;
			STRING tag = 0;
			p[0] = 0;
			llstrcatn(&p, ymd, &len);
			switch (efmt/10) {
				case 1: tag = datetrl_bcB; break;
				case 2: tag = datetrl_bcC; break;
				case 3: tag = datetrl_bcD; break;
			}
			/* this way we handle if, eg, datetrl_bc4 is blank */
			if (!tag || !tag[0])
				tag = datetrl_bcA;
			llstrcatn(&p, " ", &len);
			llstrcatn(&p, tag, &len);
			return;
		}
	} else {
		if (efmt > 1) {
			STRING p = output;
			STRING tag = 0;
			p[0] = 0;
			llstrcatn(&p, ymd, &len);
			switch (efmt/10) {
				case 1: tag = datetrl_adB; break;
				case 2: tag = datetrl_adC; break;
				case 3: tag = datetrl_adD; break;
			}
			/* this way we handle if, eg, datetrl_ad4 is blank */
			if (!tag || !tag[0])
				tag = datetrl_adA;
			llstrcatn(&p, " ", &len);
			llstrcatn(&p, tag, &len);
			return;
		}
	}
	/* no trailing tag at all */
	llstrncpy(output, ymd, len);

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
format_cal (INT cal, CNSTRING src, STRING output, INT len)
{
	ASSERT(cal>=0 && cal<ARRSIZE(calendar_pics));
	if (calendar_pics[cal]) {
		sprintpic1(output, len, calendar_pics[cal], src);
	} else {
		llstrncpy(output, src, len);
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
static void
format_complex (GDATEVAL gdv, STRING output, INT len, INT cmplx
	, STRING ymd2, STRING ymd3)
{
	STRING pic;
	INT cmplxnum=cmplx-3; /* map cmplx to 0-5 */
	if (cmplxnum<0 || cmplxnum>5) cmplxnum=5;
	switch (gdv->type) {
	case GDV_PERIOD:
		switch (gdv->subtype) {
		case GDVP_FROM:
			pic = get_cmplx_pic(ECMPLX_FROM, cmplxnum);
			sprintpic1(output, len, pic, ymd2);
			break;
		case GDVP_TO:
			pic = get_cmplx_pic(ECMPLX_TO, cmplxnum);
			sprintpic1(output, len, pic, ymd2);
			break;
		case GDVP_FROM_TO:
			pic = get_cmplx_pic(ECMPLX_FROM_TO, cmplxnum);
			sprintpic2(output, len, pic, ymd2, ymd3);
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
			sprintpic1(output, len, pic, ymd2);
			break;
		case GDVR_AFT:
		case GDVR_BET: /* BET with no AND is treated as AFT */
			pic = get_cmplx_pic(ECMPLX_AFT, cmplxnum);
			sprintpic1(output, len, pic, ymd2);
			break;
		case GDVR_BET_AND:
			pic = get_cmplx_pic(ECMPLX_BET_AND, cmplxnum);
			sprintpic2(output, len, pic, ymd2, ymd3);
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
			sprintpic1(output, len, pic, ymd2);
			break;
		case GDVA_EST:
			pic = get_cmplx_pic(ECMPLX_EST, cmplxnum);
			sprintpic1(output, len, pic, ymd2);
			break;
		case GDVA_CAL:
			pic = get_cmplx_pic(ECMPLX_CAL, cmplxnum);
			sprintpic1(output, len, pic, ymd2);
			break;
		}
		break;
	case GDV_DATE:
	default:
		snprintf(output, len, ymd2);
		break;
	}
}
/*===================================================
 * format_ymd -- Assembles date according to dateformat
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
 *  mod:    [IN]   modifier code (in bottom of monthstrs array)
 *  output: [I/O]  output string (is advanced)
 *  len:    [I/O]  length remaining in output string buffer (is decremented)
 * This routine applies the custom date pic if present (date_pic)
 *=================================================*/
static void
format_ymd (STRING syr, STRING smo, STRING sda, INT sfmt
	, STRING *output, INT * len)
{
	STRING p = *output;

	if (date_pic) {
		sprintpic3(*output, *len, date_pic, syr, smo, sda);
		*len += strlen(*output);
		return;
	}
	switch (sfmt) {
	case 0:		/* da mo yr */
		if (sda) {
			llstrcatn(&p, sda, len);
			llstrcatn(&p, " ", len);
		}
		if (smo) {
			llstrcatn(&p, smo, len);
			llstrcatn(&p, " ", len);
		}
		if (syr) {
			llstrcatn(&p, syr, len);
		}
		break;
	case 1:		/* mo da, yr */
		if (smo) {
			llstrcatn(&p, smo, len);
			llstrcatn(&p, " ", len);
		}
		if (sda) {
			llstrcatn(&p, sda, len);
			llstrcatn(&p, ", ", len);
		}
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 2:		/* mo/da/yr */
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, "/", len);
		if (sda)
			llstrcatn(&p, sda, len);
		llstrcatn(&p, "/", len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 3:		/* da/mo/yr */
		if (sda)
			llstrcatn(&p, sda, len);
		llstrcatn(&p, "/", len);
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, "/", len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 4:		/* mo-da-yr */
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, "-", len);
		if (sda)
			llstrcatn(&p, sda, len);
		llstrcatn(&p, "-", len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 5:		/* da-mo-yr */
		if (sda)
			llstrcatn(&p, sda, len);
		llstrcatn(&p, "-", len);
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, "-", len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 6:		/* modayr */
		if (smo)
			llstrcatn(&p, smo, len);
		if (sda)
			llstrcatn(&p, sda, len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 7:		/* damoyr */
		if (sda)
			llstrcatn(&p, sda, len);
		if (smo)
			llstrcatn(&p, smo, len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	case 8:         /* yr mo da */
		if (syr)
			llstrcatn(&p, syr, len);
		if (smo) {
			llstrcatn(&p, " ", len);
			llstrcatn(&p, smo, len);
		}
		if (sda) {
			llstrcatn(&p, " ", len);
			llstrcatn(&p, sda, len);
		}
		break;
	case 9:         /* yr/mo/da */
		if (syr)
			llstrcatn(&p, syr, len);
		llstrcatn(&p, "/", len);
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, "/", len);
		if (sda)
			llstrcatn(&p, sda, len);
		break;
	case 10:        /* yr-mo-da */
		if (syr)
			llstrcatn(&p, syr, len);
		llstrcatn(&p, "-", len);
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, "-", len);
		if (sda)
			llstrcatn(&p, sda, len);
		break;
	case 11:        /* yrmoda */
		if (syr)
			llstrcatn(&p, syr, len);
		if (smo)
			llstrcatn(&p, smo, len);
		if (sda)
			llstrcatn(&p, sda, len);
		break;
	/* 12 (year only) was handled directly in do_format_date */
	case 13:      /* mo/da yr */
		if (sda)
			llstrcatn(&p, sda, len);
		llstrcatn(&p, "/", len);
		if (smo)
			llstrcatn(&p, smo, len);
		llstrcatn(&p, " ", len);
		if (syr)
			llstrcatn(&p, syr, len);
		break;
	/* 14 (as GEDCOM) was handled directly in do_format_date */
        }
	*output = p;
	return;
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
format_day (INT da, INT dfmt, STRING output)
{
	STRING p;
	if (da < 0 || da > 99 || dfmt < 0 || dfmt > 2) {
		output[0] = 0;
		return;
	}
	strcpy(output, "  ");
	if (da >= 10) {
		/* dfmt irrelevant with 2-digit days */
		output[0] = da/10 + '0';
		output[1] = da%10 + '0';
		return;
	}
	p = output;
	if (da == 0) {
		if (dfmt == 2)
			output[0] = 0;
		return;
	}
	if (dfmt == 0)
		p++; /* leading space */
	else if (dfmt == 1)
		*p++ = '0'; /* leading 0 */
	*p++ = da + '0';
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
format_month (INT cal, INT mo, INT mfmt)
{
	INT casing;
	MONTH_NAMES * parr=0;
	static char scratch[3];
	if (mo < 0 || mo > 13 || mfmt < 0 || mfmt > 11) return NULL;
	if (mfmt <= 2)  {
		format_day(mo, mfmt, scratch);
		return scratch;
	}
	if (mfmt == 9)
		return gedcom_month(cal, mo);
	if (mfmt == 10)
		return roman_lower[mo-1];
	if (mfmt == 11)
		return roman_upper[mo-1];
	if (mo == 0) return (STRING) "   ";
	casing = mfmt-3;
	ASSERT(casing>=0 && casing<ARRSIZE(months_gj[0]));
	switch (cal) {
	case GDV_HEBREW: parr = months_heb; break;
	case GDV_FRENCH: parr = months_fr; break;
	default: 
		if (mo>12) return "   ";
		parr = months_gj; break;
	}
	if (parr[mo-1][casing])
		return parr[mo-1][casing];
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
format_year (INT yr, INT yfmt)
{
	static char scratch[7];
	STRING p;
	if (yr > 9999 || yr < 0) {
		switch(yfmt) {
		case 0:
			return "    ";
		case 1:
			return "0000";
		default:
			return NULL;
		}
	}
	if (yr > 999 || yfmt == 2) {
		sprintf(scratch, "%d", yr);
		return scratch;
	}
	p = (yfmt==1 ? "000" : "   ");
	if (yr < 10)
		strcpy(scratch, p);
	else if (yr < 100)
		llstrncpy(scratch, p, 2+1);
	else
		llstrncpy(scratch, p, 1+1);
	sprintf(scratch+strlen(scratch), "%d", yr);
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
	INT tok, ival;
	struct nums_s nums = { BAD_YEAR, BAD_YEAR, BAD_YEAR };
	STRING sval;
	GDATEVAL gdv = create_gdateval();
	struct gdate_s * pdate = &gdv->date1;
	BOOLEAN newdate;
	if (!str)
		return gdv;
	set_date_string(str);
	while ((tok = get_date_tok(&ival, &sval))) {
		switch (tok) {
		case MONTH_TOK:
			if (!pdate->month) {
				pdate->month = ival;
				if (nums.num1 != BAD_YEAR) {
					/* if number before month, it is a day if legal */
					if (nums.num2 == BAD_YEAR && is_valid_day(pdate, nums.num1)) {
						pdate->day = nums.num1;
						nums.num1 = BAD_YEAR;
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
				pdate->calendar = ival;
			else
				mark_invalid(gdv);
			continue;
		case YEAR_TOK:
			if (pdate->year == BAD_YEAR) {
				pdate->year = ival;
				if (sval) /* alphanum year */
					pdate->yearstr = strdup(sval);
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
			analyze_word(gdv, pdate, &nums, ival, &newdate);
			if (newdate) {
				analyze_numbers(gdv, pdate, &nums);
				nums.num1 = nums.num2 = nums.num3 = BAD_YEAR;
				pdate = &gdv->date2;
			}
			continue;
		case ICONS_TOK:
			/* number */
			if (nums.num1 == BAD_YEAR)
				nums.num1 = ival;
			else if (nums.num2 == BAD_YEAR)
				nums.num2 = ival;
			else if (nums.num3 == BAD_YEAR)
				nums.num3 = ival;
			else
				mark_freeform(gdv);
			continue;
		default:
			FATAL();
		}
	}
	/* now analyze what numbers we got */
	analyze_numbers(gdv, pdate, &nums);
	gdv->text = strdup(str);
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
analyze_word (GDATEVAL gdv, struct gdate_s * pdate, struct nums_s * nums
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
			if (pdate->day || pdate->month || pdate->year != BAD_YEAR
				|| nums->num1 != BAD_YEAR) {
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
analyze_numbers (GDATEVAL gdv, struct gdate_s * pdate, struct nums_s * nums)
{
	if (nums->num1 == BAD_YEAR) {
		/* if we have no numbers, we're done */
		return;
	}
	/* we have at least 1 number */
	if (pdate->day && pdate->month && pdate->year != BAD_YEAR) {
		/* if we already have day & month & year, we're done */
		return;
	}
	/* we need something */
	if (nums->num2 == BAD_YEAR) {
		/* if we only have 1 number */
		if (pdate->year == BAD_YEAR) {
			/* if we need year, it is year */
			set_year(pdate, nums->num1);
			return;
		}
		if (pdate->month && is_valid_day(pdate, nums->num1)) {
			/* if we only need day, it is day (if legal) */
			pdate->day = nums->num1;
			return;
		}
		/* otherwise give up (ignore it) */
		return;
	}
	/* we have at least 2 numbers */
	if (pdate->day && pdate->month) {
		/* if all we need is year, then it is year */
		set_year(pdate, nums->num1);
		return;
	}
	/* we need at least day or month */
	/* and we have at least 2 numbers */
	mark_freeform(gdv);
	if (pdate->month && pdate->year != BAD_YEAR) {
		/* if all we need is day, see if it can be day */
		if (is_valid_day(pdate, nums->num1)) {
			pdate->day = nums->num1;
		}
		return;
	}
	if (pdate->month) {
		/* if we get here, we need day & year */
		/* prefer first num for day, if legal */
		if (is_valid_day(pdate, nums->num1)) {
			pdate->day = nums->num1;
			set_year(pdate, nums->num2);
		} else {
			set_year(pdate, nums->num1);
			if (is_valid_day(pdate, nums->num2))
				pdate->day = nums->num2;
		}
		return;
	}
	/*
	if we get here, we need at least month and have 2+ numbers
	if we don't know month, then we don't know day either, as
	we only recognize day during parsing if we see it before month
	*/
	ASSERT(!pdate->day);
	/* so we need at least day & month, & have 2+ numbers */
	
	if (pdate->year != BAD_YEAR) {
		/* we need day & month, but not year, and have 2+ numbers */
		/* can we interpret them unambiguously ? */
		if (is_valid_month(pdate, nums->num1) 
			&& !is_valid_month(pdate, nums->num2)
			&& is_valid_day(pdate, nums->num2)) 
		{
			pdate->month = nums->num1;
			pdate->day = nums->num2;
			return;
		}
		if (is_valid_month(pdate, nums->num2) 
			&& !is_valid_month(pdate, nums->num1)
			&& is_valid_day(pdate, nums->num1)) 
		{
			pdate->month = nums->num2;
			pdate->day = nums->num1;
			return;
		}
		/* not unambiguous, so don't guess */
		return;
	}
	/* if we get here, we need day, month, & year, and have 2+ numbers */
	if (nums->num3 == BAD_YEAR) {
		/* we need day, month, & year, and have 2 numbers */
		/* how about day, year ? */
		if (is_valid_day(pdate, nums->num1)) {
			pdate->day = nums->num1;
			set_year(pdate, nums->num2);
		}
		/* how about year, day ? */
		if (is_valid_day(pdate, nums->num2)) {
			pdate->day = nums->num2;
			set_year(pdate, nums->num1);
		}
		/* give up */
		return;
	}
	/* we need day, month, & year, and have 3 numbers */
	/* how about day, month, year ? */
	if (is_valid_day(pdate, nums->num1) && is_valid_month(pdate, nums->num2)) {
		pdate->day = nums->num1;
		pdate->month = nums->num2;
		set_year(pdate, nums->num3);
	}
	/* how about month, day, year ? */
	if (is_valid_month(pdate, nums->num1) && is_valid_day(pdate, nums->num2)) {
		pdate->day = nums->num2;
		pdate->month = nums->num1;
		set_year(pdate, nums->num3);
	}
	/* how about year, month, day ? */
	if (is_valid_day(pdate, nums->num3) && is_valid_month(pdate, nums->num2)) {
		pdate->day = nums->num3;
		pdate->month = nums->num2;
		set_year(pdate, nums->num1);
	}
	/* give up */
}
/*===============================================
 * set_year -- Helper to assign year number
 *  (fills in the year string also)
 *  pdate:  [I/O]  date we are building
 *  yr:     [IN]   new year number
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
static void
set_year (struct gdate_s * pdate, INT yr)
{
	pdate->year = yr;
	/* we leave yearstr as 0 because we have a normal numeric year */
	pdate->yearstr = 0;
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
	gdv->date1.year = BAD_YEAR;
	gdv->date2.year = BAD_YEAR;
	gdv->valid = 1;
	return gdv;

}
/*===============================================
 * free_gdateval -- Delete existing GEDCOM date_val
 * Created: 2001/12/28 (Perry Rapp)
 *=============================================*/
void
free_gdateval (GDATEVAL gdv)
{
	if (!gdv) return;
	if (gdv->date1.yearstr)
		stdfree(gdv->date1.yearstr);
	if (gdv->date2.yearstr)
		stdfree(gdv->date2.yearstr);
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
	if (!keywordtbl) {
		init_keywordtbl();
		load_lang();
	} else if (0) /* language changed */
		load_lang();
}
/*==================================================
 * get_date_tok -- Return next date extraction token
 *  pival:   [OUT]  word enum value (eg, GD_AFT)
 *  psval:   [OUT]  pointer to (static) copy of original text
 *                   (only used for slash years)
 *================================================*/
static INT
get_date_tok (INT *pival, STRING *psval)
{
	static char scratch[30];
	STRING p = scratch;
	INT c;
	if (!sstr) return 0;
	*psval = NULL;
	while (iswhite((uchar)*sstr++))
		;
	sstr--;
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
		i = valueof_int(keywordtbl, upper(scratch), 0);
		if (i >= 2001 && i < 2000 + GDV_CALENDARS_IX) {
			*pival = i - 2000;
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
		i = valueof_int(keywordtbl, upper(scratch), 0);
		if (!i) {
			/* unrecognized word */
			return CHAR_TOK;
		}
		if ((i = valueof_int(keywordtbl, upper(scratch), 0)) > 0 && i <= 999) {
			*pival = i % 100;
			/* TODO: we need to use the fact that calendar is i/100 */
			return MONTH_TOK;
		}
		*pival = 0;
		if (i >= 1001 && i < 1000 + GD_END2) {
			*pival = i - 1000;
			return WORD_TOK;
		}
		FATAL(); /* something unexpected is in the keywordtbl ? Find out what! */
		return WORD_TOK;
	}
	if (chartype(*sstr) == DIGIT) {
		INT j=BAD_YEAR, i=0; /* i is year value, j is slash year value */
		*pival = *sstr;
		while (chartype(c = (uchar)(*p++ = *sstr++)) == DIGIT)
			i = i*10 + c - '0';
		if (i > 9999) {
			/* 5+ digit numbers are not recognized */
			return CHAR_TOK;
		}
		if (c == '/') {
			INT modnum=1;
			STRING saves = sstr, savep = p;
			j=0;
			while (chartype(c = (uchar)(*p++ = *sstr++)) == DIGIT) {
				modnum *= 10;
				j = j*10 + c - '0';
			}
			/* slash years only valid if differ by one year
			and there is not another slash after slash year
			(so we don't parse 8/9/1995 as a slash year */
			if (*sstr=='/' || (j != (i+1) % modnum)) {
				sstr = saves;
				p = savep;
				j = BAD_YEAR;
			}
		}
		*--p = 0;
		sstr--;
		if (*sstr && !is_date_delim(*sstr)) {
			/* number only valid if followed by date delimiter */
			return CHAR_TOK;
		}
		*pival = i;
		if (j != BAD_YEAR) {
			*psval = scratch;
			return YEAR_TOK;
		}
		return ICONS_TOK;
	}
	if (*sstr == 0)  {
		sstr = NULL;
		return 0;
	}
	*pival = *sstr++;
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
	keywordtbl = create_table();
	/* Load GEDCOM keywords & values into keyword table */
	for (i=0; i<ARRSIZE(gedkeys); ++i) {
		j = gedkeys[i].value;
		insert_table_int(keywordtbl, gedkeys[i].keyword, j);
	}
	/* TODO: We need to load months of other calendars here */

}
/*=============================
 * load_one_cmplx_pic -- Generate case variations
 *  of one complex picture string.
 * Created: 2001/12/30 (Perry Rapp)
 *===========================*/
static void
load_one_cmplx_pic (INT ecmplx, STRING abbrev, STRING full)
{
	ASSERT(ecmplx>=0 && ecmplx <ECMPLX_END);
	/* 0=ABT (cmplx=3) */
	cmplx_pics[ecmplx][0] = strdup(upper(abbrev));
	/* 1=Abt (cmplx=4) */
	cmplx_pics[ecmplx][1] = strdup(titlecase(abbrev));
	/* 2=ABOUT (cmplx=5) */
	cmplx_pics[ecmplx][2] = strdup(upper(full));
	/* 3=About (cmplx=6) */
	cmplx_pics[ecmplx][3] = strdup(titlecase(full));
	/* 4=abt (cmplx=7) */
	cmplx_pics[ecmplx][4] = strdup(lower(abbrev));
	/* 5=about (cmplx=8) */
	cmplx_pics[ecmplx][5] = strdup(lower(full));

}
/*=============================
 * load_one_month -- Generate case variations
 *  of one month name
 *  monum:  [IN]  month num (0-based)
 *  monarr: [I/O] month array
 *  abbrev: [IN]  eg, "jan"
 *  full:   [IN]  eg, "january"
 * Created: 2001/12/31 (Perry Rapp)
 *===========================*/
static void
load_one_month (INT monum, MONTH_NAMES * monarr, STRING abbrev, STRING full)
{
	/* 0-5 codes as in load_cmplx_pic(...) above */
	monarr[monum][0] = strdup(upper(abbrev));
	monarr[monum][1] = strdup(titlecase(abbrev));
	monarr[monum][2] = strdup(upper(full));
	monarr[monum][3] = strdup(titlecase(full));
	monarr[monum][4] = strdup(lower(abbrev));
	monarr[monum][5] = strdup(lower(full));
}
/*=============================
 * load_lang -- Load generated picture strings
 *  based on current language
 * This must be called if current language changes.
 * Created: 2001/12/30 (Perry Rapp)
 *===========================*/
static void
load_lang (void)
{
	INT i,j;
	
	/* TODO: if we have language-specific cmplx_custom, deal with
	that here */

	/* clear complex pics */
	for (i=0; i<ECMPLX_END; ++i) {
		for (j=0; j<6; ++j) {
			if (cmplx_pics[i][j]) {
				stdfree(cmplx_pics[i][j]);
				cmplx_pics[i][j] = 0;
			}
		}
	}
	/* load complex pics */
	load_one_cmplx_pic(ECMPLX_ABT, datea_abtA, datea_abtB);
	load_one_cmplx_pic(ECMPLX_EST, datea_estA, datea_estB);
	load_one_cmplx_pic(ECMPLX_CAL, datea_calA, datea_calB);
	load_one_cmplx_pic(ECMPLX_FROM, datep_fromA, datep_fromB);
	load_one_cmplx_pic(ECMPLX_TO, datep_toA, datep_toB);
	load_one_cmplx_pic(ECMPLX_FROM_TO, datep_frtoA, datep_frtoB);
	load_one_cmplx_pic(ECMPLX_BEF, dater_befA, dater_befB);
	load_one_cmplx_pic(ECMPLX_AFT, dater_aftA, dater_aftB);
	load_one_cmplx_pic(ECMPLX_BET_AND, dater_betA, dater_betB);
	/* test that all were loaded */	
	for (i=0; i<ECMPLX_END; ++i) {
		for (j=0; j<6; ++j) {
			ASSERT(cmplx_pics[i][j]);
		}
	}

	/* clear calendar pics */
	for (i=0; i<ARRSIZE(calendar_pics); ++i) {
		if (calendar_pics[i]) {
			stdfree(calendar_pics[i]);
			calendar_pics[i] = 0;
		}
	}
	calendar_pics[GDV_JULIAN] = strdup(caljul);
	calendar_pics[GDV_HEBREW] = strdup(calheb);
	calendar_pics[GDV_FRENCH] = strdup(calfr);
	calendar_pics[GDV_ROMAN] = strdup(calrom);
	/* not all slots in calendar_pics are used */

	/* clear Gregorian/Julian month names */
	for (i=0; i<ARRSIZE(months_gj); ++i) {
		for (j=0; j<ARRSIZE(months_gj[0]); ++j) {
			if (months_gj[i][j]) {
				stdfree(months_gj[i][j]);
				months_gj[i][j] = 0;
			}
		}
	}
	/* load Gregorian/Julian month names */
	load_one_month(0, months_gj, mon_gj1A, mon_gj1B);
	load_one_month(1, months_gj, mon_gj2A, mon_gj2B);
	load_one_month(2, months_gj, mon_gj3A, mon_gj3B);
	load_one_month(3, months_gj, mon_gj4A, mon_gj4B);
	load_one_month(4, months_gj, mon_gj5A, mon_gj5B);
	load_one_month(5, months_gj, mon_gj6A, mon_gj6B);
	load_one_month(6, months_gj, mon_gj7A, mon_gj7B);
	load_one_month(7, months_gj, mon_gj8A, mon_gj8B);
	load_one_month(8, months_gj, mon_gj9A, mon_gj9B);
	load_one_month(9, months_gj, mon_gj10A, mon_gj10B);
	load_one_month(10, months_gj, mon_gj11A, mon_gj11B);
	load_one_month(11, months_gj, mon_gj12A, mon_gj12B);
	/* test that all were loaded */	
	for (i=0; i<ARRSIZE(months_gj); ++i) {
		for (j=0; j<ARRSIZE(months_gj[0]); ++j) {
			ASSERT(months_gj[i][j]);
		}
	}

	/* clear Hebrew month names */
	for (i=0; i<ARRSIZE(months_heb); ++i) {
		for (j=0; j<ARRSIZE(months_heb[0]); ++j) {
			if (months_heb[i][j]) {
				stdfree(months_heb[i][j]);
				months_heb[i][j] = 0;
			}
		}
	}
	/* load Hebrew month names */
	load_one_month(0, months_heb, mon_heb1A, mon_heb1B);
	load_one_month(1, months_heb, mon_heb2A, mon_heb2B);
	load_one_month(2, months_heb, mon_heb3A, mon_heb3B);
	load_one_month(3, months_heb, mon_heb4A, mon_heb4B);
	load_one_month(4, months_heb, mon_heb5A, mon_heb5B);
	load_one_month(5, months_heb, mon_heb6A, mon_heb6B);
	load_one_month(6, months_heb, mon_heb7A, mon_heb7B);
	load_one_month(7, months_heb, mon_heb8A, mon_heb8B);
	load_one_month(8, months_heb, mon_heb9A, mon_heb9B);
	load_one_month(9, months_heb, mon_heb10A, mon_heb10B);
	load_one_month(10, months_heb, mon_heb11A, mon_heb11B);
	load_one_month(11, months_heb, mon_heb12A, mon_heb12B);
	load_one_month(12, months_heb, mon_heb13A, mon_heb13B);
	/* test that all were loaded */	
	for (i=0; i<ARRSIZE(months_heb); ++i) {
		for (j=0; j<ARRSIZE(months_heb[0]); ++j) {
			ASSERT(months_heb[i][j]);
		}
	}

	/* clear French Republic month names */
	for (i=0; i<ARRSIZE(months_fr); ++i) {
		for (j=0; j<ARRSIZE(months_fr[0]); ++j) {
			if (months_fr[i][j]) {
				stdfree(months_fr[i][j]);
				months_fr[i][j] = 0;
			}
		}
	}
	/* load French Republic month names */
	load_one_month(0, months_fr, mon_fr1A, mon_fr1B);
	load_one_month(1, months_fr, mon_fr2A, mon_fr2B);
	load_one_month(2, months_fr, mon_fr3A, mon_fr3B);
	load_one_month(3, months_fr, mon_fr4A, mon_fr4B);
	load_one_month(4, months_fr, mon_fr5A, mon_fr5B);
	load_one_month(5, months_fr, mon_fr6A, mon_fr6B);
	load_one_month(6, months_fr, mon_fr7A, mon_fr7B);
	load_one_month(7, months_fr, mon_fr8A, mon_fr8B);
	load_one_month(8, months_fr, mon_fr9A, mon_fr9B);
	load_one_month(9, months_fr, mon_fr10A, mon_fr10B);
	load_one_month(10, months_fr, mon_fr11A, mon_fr11B);
	load_one_month(11, months_fr, mon_fr12A, mon_fr12B);
	load_one_month(12, months_fr, mon_fr13A, mon_fr13B);
	/* test that all were loaded */	
	for (i=0; i<ARRSIZE(months_fr); ++i) {
		for (j=0; j<ARRSIZE(months_fr[0]); ++j) {
			ASSERT(months_fr[i][j]);
		}
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
	if (!keywordtbl) {
		init_keywordtbl();
		load_lang();
	}
	/* TODO: Should this be one of the customizable formats ? */
	month = gedkeys[pt->tm_mon].keyword;
	sprintf(dat, "%d %s %d", pt->tm_mday, month, 1900 + pt->tm_year);
	return dat;
}
/*=============================
 * gdateval_isdual -- Does gdateval contain
 * two dates ?
 * Created: 2001/12/28 (Perry Rapp)
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
 * Created: 2001/12/28 (Perry Rapp)
 *===========================*/
static BOOLEAN
is_valid_day (struct gdate_s * pdate, INT day)
{
	/* To consider: Fancy code with calendars */
	/* for now, use max (all cals all months), which is 31 */
	pdate=pdate; /* unused */
	return (day>=1 && day<=31);
}
/*=============================
 * is_valid_month -- Is this month legal for this date ?
 * Created: 2001/12/28 (Perry Rapp)
 *===========================*/
static BOOLEAN
is_valid_month (struct gdate_s * pdate, INT month)
{
	INT cal = pdate ? pdate->calendar : 0;
	switch (cal) {
	case GDV_HEBREW:
	case GDV_FRENCH:
		return (month>=1 && month<=13);
	default:
		return (month>=1 && month<=12);
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
	if (c=='/' || c=='-' || c=='.')
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
		cmplx_custom[ecmplx] = strdup(pic);
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
		date_pic = strdup(pic);
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
