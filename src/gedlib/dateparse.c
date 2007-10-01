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
 * dateparse.c -- Code to parse a string into a date
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


/*********************************************
 * local types used in local function prototypes
 *********************************************/

/* token types, used in parsing */
enum { MONTH_TOK=1, CHAR_TOK, WORD_TOK, ICONS_TOK, CALENDAR_TOK, YEAR_TOK };


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
static void free_gdate(struct tag_gdate *);
static INT get_date_tok(struct tag_dnum*);
static BOOLEAN is_date_delim(char c);
static BOOLEAN is_valid_day(struct tag_gdate * pdate, struct tag_dnum day);
static BOOLEAN is_valid_month(struct tag_gdate * pdate, struct tag_dnum month);
static void mark_freeform(GDATEVAL gdv);
static void mark_invalid(GDATEVAL gdv);
static void set_date_string(STRING);

/*********************************************
 * local types & variables
 *********************************************/

static STRING sstr, sstr_start;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=====================================================
 * mark_invalid -- Set a gdate_val to invalid
 *  gdv:  [I/O]  date_val we are building
 *===================================================*/
static void
mark_invalid (GDATEVAL gdv)
{
	if (gdv->valid != GDV_V_PHRASE)
		gdv->valid = GDV_V_INVALID;
}
/*=====================================================
 * mark_freeform -- Set a gdate_val to freeform (unless invalid)
 *  gdv:  [I/O]  date_val we are building
 *===================================================*/
static void
mark_freeform (GDATEVAL gdv)
{
	if (gdv->valid == GDV_V_GOOD)
		gdv->valid = GDV_V_FREEFORM;
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
	if (!str || !str[0])
		return gdv;
	/* GEDCOM DATE PHRASE, eg, "(one 1srt January)" */
	if (str[0] == '(' && str[strlen(str)-1] == ')') {
		gdv->valid = GDV_V_PHRASE;
	}
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
			{
				BOOLEAN newdate=FALSE;
				analyze_word(gdv, pdate, &nums, dnum.val, &newdate);
				if (newdate) {
					analyze_numbers(gdv, pdate, &nums);
					clear_numbers(&nums);
					pdate = &gdv->date2;
				}
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
	gdv->valid = GDV_V_GOOD;
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
			/* Except Gregorian & Julian are not distinguished by month names */
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
	/* Assume these are all the valid delimiters in a date */
	if (c=='/' || c=='-' || c=='.' || c==',')
		return TRUE;
	return FALSE;
}
