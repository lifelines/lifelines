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

enum { BAD_YEAR=-99999 };
struct tag_dnum {
	INT val;    /* parsed value, eg, 2 for FEBRUARY */
	INT val2;   /* TODO: need comment explaining what this is */
	STRING str; /* raw string that was in date */
};

typedef struct tag_gdateval *GDATEVAL; /* definition of struct in datei.h */

ZSTR approx_time(INT seconds);
GDATEVAL create_gdateval(void);
STRING do_format_date(STRING, INT, INT, INT, INT, INT, INT);
void date_update_lang(void);
INT date_get_day(GDATEVAL gdv);
INT date_get_mod(GDATEVAL gdv);
INT date_get_month(GDATEVAL gdv);
INT date_get_year(GDATEVAL gdv);
STRING date_get_year_string(GDATEVAL gdv);
GDATEVAL extract_date(STRING);
void free_gdateval(GDATEVAL gdv);
BOOLEAN gdateval_isdual(GDATEVAL);
STRING get_todays_date(void);
BOOLEAN is_valid_dayfmt(INT dayfmt);
BOOLEAN is_valid_monthfmt(INT monthfmt);
BOOLEAN is_valid_yearfmt(INT yearfmt);
BOOLEAN set_cmplx_pic(INT ecmplx, STRING pic);
void set_date_pic(STRING pic);
STRING shorten_date(STRING);
void term_date(void);



#endif /* _DATE_H */
