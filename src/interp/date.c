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

#define MONTH_TOK 1
#define CHAR_TOK  2
#define WORD_TOK  3
#define ICONS_TOK 4

static void format_ymd(STRING, STRING, STRING, INT, INT, STRING*, INT *len);
static void format_mod(INT, STRING*);
static STRING format_day(INT, INT);
static STRING format_month(INT, INT);
static STRING format_year(INT, INT);
static void set_date_string(STRING);
static INT get_date_tok(INT*, STRING*);
static void init_monthtbl(void);

static struct {
	char *sl, *su, *ll, *lu;
} monthstrs[19] = {
	{ "Jan", "JAN", "January", "JANUARY" },
	{ "Feb", "FEB", "February", "FEBRUARY" },
	{ "Mar", "MAR", "March", "MARCH" },
	{ "Apr", "APR", "April", "APRIL" },
	{ "May", "MAY", "May", "MAY" },
	{ "Jun", "JUN", "June", "JUNE" },
	{ "Jul", "JUL", "July", "JULY" },
	{ "Aug", "AUG", "August", "AUGUST" },
	{ "Sep", "SEP", "September", "SEPTEMBER" },
	{ "Oct", "OCT", "October", "OCTOBER" },
	{ "Nov", "NOV", "November", "NOVEMBER" },
	{ "Dec", "DEC", "December", "DECEMBER" },

	/* date modifiers appended to the month table */

	{ "abt", "ABT", "about", "ABOUT" },     /*  1 */
	{ "bef", "BEF", "before", "BEFORE" },   /*  2 */
	{ "aft", "AFT", "after", "AFTER" },     /*  3 */
	{ "bet", "BET", "between", "BETWEEN" }, /*  4 - range */
	{ "and", "AND", "and", "AND" },         /*  5 */
	{ "from", "FROM", "from", "FROM" },     /*  6 - range */
	{ "to", "TO", "to", "TO" },             /*  7 */
};

static STRING sstr = NULL;
static TABLE monthtbl = NULL;

/*==========================================
 * do_format_date -- Do general date formatting
 * str - raw string containing a date
 *  dfmt; [IN]  day format code (see format_day function below)
 *  mfmt: [IN]  month format code (see format_month function below)
 *  yfmt: [IN]  year format code (see format_year function below)
 *  sfmt: [IN]  combining code
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
 * cmplx - 0 is year only, 1 is complex, including
 *         date modifiers, ranges, and/or double-dating
 * Returns static buffer
 *========================================*/
STRING
do_format_date (STRING str, INT dfmt, INT mfmt,
             INT yfmt, INT sfmt, INT cmplx)
{
	INT mod, da, mo, yr;
	STRING sda, smo, syr;
	static unsigned char scratch[50], daystr[4];
	STRING p = scratch;
	INT len = sizeof(scratch);
	if (!str) return NULL;
	if (sfmt==12) {
		/* This is what used to be the shrt flag */
		return shorten_date(str);
	}
	if (sfmt==14) {
		llstrncpy(scratch, str, sizeof(scratch));
		return scratch;
	}
	*p = 0;
	extract_date(str, &mod, &da, &mo, &yr, &syr);
	if ((sda = format_day(da, dfmt))) sda = strcpy(daystr, sda);
	smo = format_month(mo, mfmt);
	if (!cmplx) syr = format_year(yr, yfmt);
	else format_mod(mod%100, &p);
	format_ymd(syr, smo, sda, sfmt, mod, &p, &len);
	if (cmplx && (mod%100 == 4 || mod%100 == 6)) {
		*p++ = ' ';
		format_mod(mod%100 + 1, &p);
		extract_date(NULL, &mod, &da, &mo, &yr, &syr);
		if ((sda = format_day(da, dfmt))) sda = strcpy(daystr, sda);
		smo = format_month(mo, mfmt);
		format_ymd(syr, smo, sda, sfmt, mod, &p, &len);
	}
	return scratch;
}
/*===================================================
 * format_ymd -- Assembles date according to dateformat
 *  syr:    [IN]   year string
 *  smo:    [IN]   month string
 *  sda:    [IN]   day string
 *  sfmt:   [IN]   format code (see do_format_date)
 *  mod:    [IN]   modifier code (in bottom of monthstrs array)
 *  output: [I/O]  output string (is advanced)
 *  len:    [I/O]  length remaining in output string buffer (is decremented)
 *=================================================*/
static void
format_ymd (STRING syr, STRING smo, STRING sda, INT sfmt, INT mod
	, STRING *output, INT * len)
{
	STRING p = *output;

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
	if (mod >= 100)
		llstrcatn(&p, " BC", len);
	*output = p;
        return;
}

/*=====================================
 * format_mod -- Format date modifier
 *===================================*/
static void
format_mod (INT mod,
            STRING *pp)
{
	if (mod < 1 || mod > 7) return;
	strcpy(*pp, monthstrs[mod+12-1].ll);
	*pp += strlen(*pp);
	**pp = ' ';
	*pp += 1;
	return;
}
/*=======================================
 * format_day -- Formats day part of date
 *  day:  [IN]  numeric day (0 for unknown)
 *  dfmt: [IN]     0 - num, space
 *                 1 - num, lead 0
 *                 2 - num, as is
 *=====================================*/
static STRING
format_day (INT da, INT dfmt)
{
	static unsigned char scratch[3];
	STRING p;
	if (da < 0 || da > 99 || dfmt < 0 || dfmt > 2) return NULL;
	strcpy(scratch, "  ");
	if (da >= 10) {
		/* dfmt irrelevant with 2-digit days */
		scratch[0] = da/10 + '0';
		scratch[1] = da%10 + '0';
		return scratch;
	}
	p = scratch;
	if (da == 0) {
		if (dfmt == 2) return NULL;
		return scratch;
	}
	if (dfmt == 0)  p++; /* leading space */
	else if (dfmt == 1)  *p++ = '0'; /* leading 0 */
	*p++ = da + '0';
	*p = 0;
	return scratch;
}
/*===========================================
 * format_month -- Formats month part of date
 *  mo:    [IN]  numeric month (0 for unknown)
 *  mfmt:  [IN]    0 - num, space
 *                 1 - num, lead 0
 *                 2 - num, as is
 *                 3 - eg, MAR
 *                 4 - eg, Mar
 *                 5 - eg, MARCH
 *                 6 - eg, March
 *  returns static buffer
 *=========================================*/
static STRING
format_month (INT mo, INT mfmt)
{
	static char scratch[3];
	STRING p;
	if (mo < 0 || mo > 12 || mfmt < 0 || mfmt > 6) return NULL;
	if (mfmt <= 2)  {
		if ((p = format_day(mo, mfmt))) return strcpy(scratch, p);
		return NULL;
	}
	if (mo == 0) return (STRING) "   ";
	switch (mfmt) {
	case 3: return (STRING) monthstrs[mo-1].su;
	case 4: return (STRING) monthstrs[mo-1].sl;
	case 5: return (STRING) monthstrs[mo-1].lu;
	case 6: return (STRING) monthstrs[mo-1].ll;
	}
	return NULL;
}
/*=========================================
 * format_year -- Formats year part of date
 *  yr:   [IN]  numeric year (0 for unknown)
 *  yfmt: [IN]     0 - positive years only, no leading, no AD/BC
 *                 1 - bare year number, no leading, no AD/BC
 *
 *  TODO: Add support for BC formats
 *  returns static buffer
 *=======================================*/
static STRING
format_year (INT yr, INT yfmt)
{
	static char scratch[50];
	if (yr <= 0)  return NULL;
	switch (yfmt) {
	case 1:
		sprintf(scratch, "%d", yr);
		break;
	case 0:
	default:
		if (yr <= 0) return NULL;
		sprintf(scratch, "%d", yr);
		break;
	}
	return scratch;
}
/*=====================================================
 * extract_date -- Extract date from free format string
 *===================================================*/
void
extract_date (STRING str,
              INT *pmod,
              INT *pda,
              INT *pmo,
              INT *pyr,
              STRING *pyrstr)
{
	INT tok, ival, era = 0;
	STRING sval;
	static unsigned char yrstr[10];
	*pyrstr = "";
	*pmod = *pda = *pmo = *pyr = 0;
	if (str) set_date_string(str);
	while ((tok = get_date_tok(&ival, &sval))) {
		switch (tok) {
		case MONTH_TOK:
			if (*pmo == 0) *pmo = ival;
			continue;
		case CHAR_TOK:
			continue;
		case WORD_TOK:
			if (*pyr == 0 && *pda == 0 && ival < 0) ival = -ival;
			if (ival > 0 && ival < 20 && *pmod == 0) *pmod = ival;
			if (ival == -99) era = 100;
			if ((*pmod == 4 && ival == 5) ||
			    (*pmod == 6 && ival == 7)) goto combine;
			continue;
		case ICONS_TOK:
/* years 1-99 are denoted by at least two leading zeroes */
			if (ival >= 100 ||
			    (ival > 0 && sval[0] == '0' && sval[1] == '0')) {
				if (eqstr(*pyrstr,"")) {
					strcpy(yrstr,sval);
					*pyrstr = yrstr;
					*pyr = ival;
				}
			}
			else if (ival <= 31 && *pda == 0) *pda = ival;
			continue;
		default:
			FATAL();
		}
	}
	combine:
	*pmod += era;
}
/*===============================================
 * set_date_string -- Init date extraction string
 *=============================================*/
static void
set_date_string (STRING str)
{
	sstr = str;
	if (!monthtbl) init_monthtbl();
}
/*==================================================
 * get_date_tok -- Return next date extraction token
 *================================================*/
static INT
get_date_tok (INT *pival,
              STRING *psval)
{
	static unsigned char scratch[30];
	STRING p = scratch;
	INT i, c;
	if (!sstr) return 0;
	while (iswhite(*sstr++))
		;
	sstr--;
	if (isletter(*sstr)) {
		while (isletter(*p++ = *sstr++))
			;
		*--p = 0;
		sstr--;
		*psval = scratch;
		if ((i = valueof_int(monthtbl, upper(scratch), 0)) > 0 && i <= 12) {
			*pival = i;
			return MONTH_TOK;
		}
		*pival = 0;
		if (i > 12) *pival = i - 12;
		if (i < 0) *pival = i;
		return WORD_TOK;
	}
	if (chartype(*sstr) == DIGIT) {
		i = 0;
		while (chartype(c = *p++ = *sstr++) == DIGIT)
			i = i*10 + c - '0';
		if (c == '/') {
			while (chartype(*p++ = *sstr++) == DIGIT) ;
		}
		*--p = 0;
		sstr--;
		*psval = scratch;
		*pival = i;
		return ICONS_TOK;
	}
	if (*sstr == 0)  {
		sstr = NULL;
		return 0;
	}
	*pival = *sstr++;
	*psval = (STRING) "";
	return CHAR_TOK;
}
/*=========================================
 * init_monthtbl -- Init month string table
 *=======================================*/
static void
init_monthtbl (void)
{
	INT i, j;
	monthtbl = create_table();
	for (i = 0; i < 19; i++) {
		j = i + 1;
		insert_table_int(monthtbl, monthstrs[i].su, j);
		insert_table_int(monthtbl, monthstrs[i].lu, j);
	}
	insert_table_int(monthtbl, "EST", -1);  /* ignored after date */
	insert_table_int(monthtbl, "BC", -99);
}
/*=============================
 * get_date -- Get today's date
 *===========================*/
STRING
get_date (void)
{
	struct tm *pt;
	time_t curtime;
	static unsigned char dat[20];
	curtime = time(NULL);
	pt = localtime(&curtime);
	sprintf(dat, "%d %s %d", pt->tm_mday, monthstrs[pt->tm_mon].su,
	    1900 + pt->tm_year);
	return dat;
}

