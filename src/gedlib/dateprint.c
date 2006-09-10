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
 * dateprint.c -- Code to print a date to a string
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

extern STRING qSdatetrl_bcA,qSdatetrl_bcB,qSdatetrl_bcC,qSdatetrl_bcD;
extern STRING qSdatetrl_adA,qSdatetrl_adB,qSdatetrl_adC,qSdatetrl_adD;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static ZSTR do_zformat_date(STRING str, INT dfmt, INT mfmt,
             INT yfmt, INT sfmt, INT efmt, INT cmplx);
static void format_cal(ZSTR zstr, INT cal);
static ZSTR format_complex(GDATEVAL gdv, INT cmplx, STRING ymd2, STRING ymd3);
static void format_day(struct tag_dnum da, INT dfmt, STRING output);
static STRING format_month(INT cal, struct tag_dnum mo, INT mfmt);
static void format_eratime(ZSTR zstr, INT eratime, INT efmt);
static STRING format_year(struct tag_dnum yr, INT yfmt);
static void format_ymd(ZSTR zstr, STRING syr, STRING smo, STRING sda, INT sfmt);
static ZSTR zshorten_date(STRING date);

/*********************************************
 * local types & variables
 *********************************************/

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

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
/*==========================================
 * do_zformat_date
 * See description above for do_format_date
 * (Except this returns alloc'd ZSTR
 *========================================*/
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
		if (gdv->valid == GDV_V_PHRASE) {
			/* GEDCOM date phrases (parenthesized) shown "as is" */
			return zs_news(gdv->text);
		}
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
		if (gdv->valid == GDV_V_PHRASE) {
			/* GEDCOM date phrases (parenthesized) shown "as is" */
			return zs_news(gdv->text);
		}
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
	ASSERT(cal>=0);
	ASSERT(cal<ARRSIZE(calendar_pics));
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
	ASSERT(ecmplx>=0);
	ASSERT(ecmplx<ECMPLX_END);
	ASSERT(cmplxnum>=0);
	ASSERT(cmplxnum<6);
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
 *                 14- as in GEDCOM
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
 *                 1 - num, lead 0 (blank for blank)
 *                 2 - num, as is
 *                21 - num, lead 0, and "00" for blank
 * output: [I/O] buffer in which to write
 *                must be at least 3 characters
 *=====================================*/
static void
format_day (struct tag_dnum da, INT dfmt, STRING output)
{
	STRING p;
	INT dayval = da.val; /* ignore complex days for now */
	if (dayval < 0 || dayval > 99 || !is_valid_dayfmt(dfmt)) {
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
	if (dayval == 0 && dfmt != 21) {
		if (dfmt == 2)
			output[0] = 0;
		return;
	}
	if (dfmt == 0)
		p++; /* leading space */
	else if (dfmt == 1 || dfmt == 21)
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
 *                 1 - num, lead 0 (blank for blank)
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
 *                21 - num, lead 0, and "00" for blank
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
	if (moval < 0 || moval > 13 || !is_valid_monthfmt(mfmt)) return NULL;
	if (mfmt <= 2 || mfmt == 21)  {
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
	ASSERT(casing>=0);
	ASSERT(casing<ARRSIZE(months_gj[0]));
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
		sprintf(scratch, "%ld", yrval);
		return scratch;
	}
	p = (yfmt==1 ? "000" : "   ");
	if (yrval < 10)
		strcpy(scratch, p);
	else if (yrval < 100)
		llstrncpy(scratch, p, 2+1, uu8);
	else
		llstrncpy(scratch, p, 1+1, uu8);
	sprintf(scratch+strlen(scratch), "%ld", yrval);
	return scratch;
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
/*====================================+
 * is_valid_dayfmt -- return FALSE if not a valid day format
 * See format_day for format descriptions
 *===================================*/
BOOLEAN
is_valid_dayfmt (INT dayfmt)
{
	if (dayfmt == 21) return TRUE;
	if (dayfmt >=0 && dayfmt <= 2) return TRUE;
	return FALSE;
}
/*====================================+
 * is_valid_monthfmt -- return FALSE if not a valid month format
 * See format_month for format descriptions
 *===================================*/
BOOLEAN
is_valid_monthfmt (INT monthfmt)
{
	if (monthfmt == 21) return TRUE;
	if (monthfmt >=0 && monthfmt <= 11) return TRUE;
	return FALSE;
}
/*====================================+
 * is_valid_yearfmt -- return FALSE if not a valid year format
 *===================================*/
BOOLEAN
is_valid_yearfmt (INT yearfmt)
{
	if (yearfmt >=0 && yearfmt <= 2) return TRUE;
	return FALSE;
}
