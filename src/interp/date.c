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
/*==============================================================
 * date.c -- Code to process dates
 * Copyright(c) 1992-94 by T. T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 17 Aug 93
 *   3.0.0 - 20 Jan 94    3.0.2 - 10 Nov 94
 *   3.0.3 - 17 Jul 95
 *============================================================*/

#include "standard.h"
#include "table.h"
#include <sys/types.h>
#include <time.h>

#define MONTH_TOK 1
#define CHAR_TOK  2
#define WORD_TOK  3
#define ICONS_TOK 4

static STRING format_day();
static STRING format_month();
static STRING format_year();
static set_date_string();
static INT get_date_tok();
static init_monthtbl();

#ifndef WIN32
STRING strcpy();
#endif

struct {
	char *sl, *su, *ll, *lu;
} monthstrs[12] = {
	"Jan", "JAN", "January", "JANUARY",
	"Feb", "FEB", "February", "FEBRUARY",
	"Mar", "MAR", "March", "MARCH",
	"Apr", "APR", "April", "APRIL",
	"May", "MAY", "May", "MAY",
	"Jun", "JUN", "June", "JUNE",
	"Jul", "JUL", "July", "JULY",
	"Aug", "AUG", "August", "AUGUST",
	"Sep", "SEP", "September", "SEPTEMBER",
	"Oct", "OCT", "October", "OCTOBER",
	"Nov", "NOV", "November", "NOVEMBER",
	"Dec", "DEC", "December", "DECEMBER",
};

static STRING sstr = NULL;
static TABLE monthtbl = NULL;

/*==========================================
 * format_date -- Do general date formatting
 *========================================*/
STRING format_date (str, dfmt, mfmt, yfmt, sfmt)
STRING str;	/* raw string containing a date */
INT dfmt;	/* day format:	0 - num, space 
				1 - num, lead 0
				2 - num, as is */
INT mfmt;	/* month format:0 - num, space
				1 - num, lead 0
				2 - num, as is
				3 - eg, MAR
				4 - eg, Mar
				5 - eg, MARCH
				6 - eg, March */
INT yfmt;	/* year format: none yet */
INT sfmt;	/* date format:	0 - da mo yr
				1 - mo da, yr
				2 - mo/da/yr
				3 - da/mo/yr
				4 - mo-da-yr
				5 - da-mo-yr
				6 - modayr
				7 - damoyr
        			8 - yr mo da
        			9 - yr/mo/da
        			10- yr-mo-da
        			11- yrmoda */
{
	INT da, mo, yr;
	STRING sda, smo, syr;
	static unsigned char scratch[20], daystr[4];
	STRING p = scratch;
	if (!str) return NULL;
	extract_date(str, &da, &mo, &yr);
	if (sda = format_day(da, dfmt)) sda = strcpy(daystr, sda);
	smo = format_month(mo, mfmt);
	syr = format_year(yr, yfmt);
	switch (sfmt) {
	case 0:		/* da mo yr */
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
			*p++ = ' ';
		}
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
			*p++ = ' ';
		}
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 1:		/* mo da, yr */
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
			*p++ = ' ';
		}
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
			*p++ = ',';
			*p++ = ' ';
		}
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 2:		/* mo/da/yr */
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
		}
		*p++ = '/';
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
		}
		*p++ = '/';
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 3:		/* da/mo/yr */
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
		}
		*p++ = '/';
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
		}
		*p++ = '/';
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 4:		/* mo-da-yr */
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
		}
		*p++ = '-';
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
		}
		*p++ = '-';
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 5:		/* da-mo-yr */
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
		}
		*p++ = '-';
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
		}
		*p++ = '-';
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 6:		/* modayr */
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
		}
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
		}
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
	case 7:		/* damoyr */
		if (sda) {
			strcpy(p, sda);
			p += strlen(p);
		}
		if (smo) {
			strcpy(p, smo);
			p += strlen(p);
		}
		if (syr) {
			strcpy(p, syr);
			p += strlen(p);
		}
		*p = 0;
		break;
        case 8:         /* yr mo da */
                if (syr) {
                        strcpy(p, syr);
                        p += strlen(p);
                }
                if (smo) {
                        *p++ = ' ';
                        strcpy(p, smo);
                        p += strlen(p);
                }
                if (sda) {
                        *p++ = ' ';
                        strcpy(p, sda);
                        p += strlen(p);
                }
                *p = 0;
                break;
        case 9:         /* yr/mo/da */
                if (syr) {
                        strcpy(p, syr);
                        p += strlen(p);
                }
                *p++ = '/';
                if (smo) {
                        strcpy(p, smo);
                        p += strlen(p);
                }
                *p++ = '/';
                if (sda) {
                        strcpy(p, sda);
                        p += strlen(p);
                }
                *p = 0;
                break;
        case 10:        /* yr-mo-da */
                if (syr) {
                        strcpy(p, syr);
                        p += strlen(p);
                }
                *p++ = '-';
                if (smo) {
                        strcpy(p, smo);
                        p += strlen(p);
                }
                *p++ = '-';
                if (sda) {
                        strcpy(p, sda);
                        p += strlen(p);
                }
                *p = 0;
                break;
        case 11:        /* yrmoda */
                if (syr) {
                        strcpy(p, syr);
                        p += strlen(p);
                }
                if (smo) {
                        strcpy(p, smo);
                        p += strlen(p);
                }
                if (sda) {
                        strcpy(p, sda);
                        p += strlen(p);
                }
                *p = 0;
                break;
        }
        return scratch;

}
/*=======================================
 * format_day -- Formats day part of date
 *=====================================*/
static STRING format_day (da, dfmt)
INT da;		/* day - 0 for unknown */
INT dfmt;	/* format code */
{
	static unsigned char scratch[3];
	STRING p;
	if (da < 0 || da > 99 || dfmt < 0 || dfmt > 2) return NULL;
	strcpy(scratch, "  ");
	if (da >= 10) {
		scratch[0] = da/10 + '0';
		scratch[1] = da%10 + '0';
		return scratch;
	}
	p = scratch;
	if (da == 0) {
		if (dfmt == 2) return NULL;
		return scratch;
	}
	if (dfmt == 0)  p++;
	else if (dfmt == 1)  *p++ = '0';
	*p++ = da + '0';
	*p = 0;
	return scratch;
}
/*===========================================
 * format_month -- Formats month part of date
 *=========================================*/
static STRING format_month (mo, mfmt)
INT mo;		/* month - 0 for unknown */
INT mfmt;	/* format code */
{
	static char scratch[3];
	STRING p;
	if (mo < 0 || mo > 12 || mfmt < 0 || mfmt > 6) return NULL;
	if (mfmt <= 2)  {
		if (p = format_day(mo, mfmt)) return strcpy(scratch, p);
		return NULL;
	}
	if (mo == 0) return (STRING) "   ";
	switch (mfmt) {
	case 3: return (STRING) monthstrs[mo-1].su;
	case 4: return (STRING) monthstrs[mo-1].sl;
	case 5: return (STRING) monthstrs[mo-1].lu;
	case 6: return (STRING) monthstrs[mo-1].ll;
	}
}
/*=========================================
 * format_year -- Formats year part of date
 *=======================================*/
static STRING format_year (yr, yfmt)
INT yr;
INT yfmt;
{
	static unsigned char scratch[50];
	if (yr <= 0)  return NULL;
	sprintf(scratch, "%d", yr);
	return scratch;
}
/*=====================================================
 * extract_date -- Extract date from free format string
 *===================================================*/
extract_date (str, pda, pmo, pyr)
STRING str;
INT *pda, *pmo, *pyr;
{
	INT tok, ival;
	STRING sval;
	*pda = *pmo = *pyr = 0;
	set_date_string(str);
	while (tok = get_date_tok(&ival, &sval)) {
		switch (tok) {
		case MONTH_TOK:
			if (*pmo == 0) *pmo = ival;
			continue;
		case CHAR_TOK:
			continue;
		case WORD_TOK:
			continue;
		case ICONS_TOK:
			if (ival <= 31 && *pda == 0) *pda = ival;
			if (ival >= 100 && *pyr == 0) *pyr = ival;
			continue;
		default:
			FATAL();
		}
	}
}
/*===============================================
 * set_date_string -- Init date extraction string
 *=============================================*/
static set_date_string (str)
STRING str;
{
	sstr = str;
	if (!monthtbl) init_monthtbl();
}
/*==================================================
 * get_date_tok -- Return next date extraction token
 *================================================*/
static INT get_date_tok (pival, psval)
INT *pival;
STRING *psval;
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
		if ((i = valueof(monthtbl, upper(scratch))) > 0) {
			*pival = i;
			return MONTH_TOK;
		}
		*pival = 0;
		return WORD_TOK;
	}
	if (chartype(*sstr) == DIGIT) {
		i = 0;
		while (chartype(c = *p++ = *sstr++) == DIGIT)
			i = i*10 + c - '0';
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
static init_monthtbl ()
{
	INT i, j;
	monthtbl = create_table();
	for (i = 0; i < 12; i++) {
		j = i + 1;
		insert_table(monthtbl, monthstrs[i].su, j);
		insert_table(monthtbl, monthstrs[i].lu, j);
	}
}
/*=============================
 * get_date -- Get today's date
 *===========================*/
STRING get_date ()
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
