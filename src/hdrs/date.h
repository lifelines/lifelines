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
 * date.h - Header file for date.c
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#ifndef _DATE_H
#define _DATE_H


/* GEDCOM dates */
enum { BAD_YEAR=-99999 };
struct dnum_s { INT val; INT val2; STRING str; };
struct gdate_s {
	INT calendar;
	struct dnum_s year;
	struct dnum_s month;
	struct dnum_s day;
	INT mod;
	INT eratime; /* eg, AD, BC */
};
struct gdateval_s {
	struct gdate_s date1;
	struct gdate_s date2; /* used by period/from_to & range/bet_and */
	INT type;
	INT subtype;
	INT valid; /* -1=bad syntax, 0=freeform, 1=perfect GEDCOM date */
	STRING text; /* copy of original */
};
typedef struct gdateval_s *GDATEVAL;
enum { GDV_PERIOD=1, GDV_RANGE, GDV_DATE, GDV_APPROX  };
enum { GDVP_FROM=1, GDVP_TO, GDVP_FROM_TO }; /* period subtype */
enum { GDVR_BEF=1, GDVR_AFT, GDVR_BET, GDVR_BET_AND }; /* range subtype */
enum { GDVA_ABT=1, GDVA_EST, GDVA_CAL }; /* approx subtype */
enum { GDV_GREGORIAN=1, GDV_JULIAN, GDV_HEBREW, GDV_FRENCH, GDV_ROMAN, GDV_CALENDARS_IX };
enum { GDV_AD=1, GDV_BC };

GDATEVAL create_gdateval(void);
STRING do_format_date(STRING, INT, INT, INT, INT, INT, INT);
GDATEVAL extract_date(STRING);
void free_gdateval(GDATEVAL gdv);
BOOLEAN gdateval_isdual(GDATEVAL);
STRING get_todays_date(void);
BOOLEAN set_cmplx_pic(INT ecmplx, STRING pic);
void set_date_pic(STRING pic);


#endif /* _DATE_H */
