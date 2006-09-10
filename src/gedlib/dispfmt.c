/* 
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*==============================================================
 * dispfmt.c -- Code to reformat dates & places for user preferences
 *  (for use in reports and/or GUI)
 *============================================================*/

#include "llstdlib.h"
#include "gedcom.h"
#include "lloptions.h"
#include "date.h"

/*********************************************
 * global/exported variables
 *********************************************/

struct tag_rfmt disp_long_rfmt; /* reformatting used for display long forms */
struct tag_rfmt disp_shrt_rfmt; /* reformatting used for display short forms */


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static STRING disp_long_format_date(STRING date);
static STRING disp_shrt_format_date(STRING date);
static STRING disp_shrt_format_plac(STRING plac);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===============================================
 * init_disp_reformat -- Initialize reformatting for display
 * Set up format descriptions for both long & short display forms
 * Created: 2001/07/12 (Perry Rapp)
 *=============================================*/
void
init_disp_reformat (void)
{
	/* Set up long formats */
	memset(&disp_long_rfmt, 0, sizeof(disp_long_rfmt));
	disp_long_rfmt.rfmt_date = &disp_long_format_date;
	disp_long_rfmt.rfmt_plac = 0; /* use place as is */
	disp_long_rfmt.combopic = "%1, %2";
	/* Set up short formats */
	memset(&disp_shrt_rfmt, 0, sizeof(disp_shrt_rfmt));
	disp_shrt_rfmt.rfmt_date = &disp_shrt_format_date;
	disp_shrt_rfmt.rfmt_plac = &disp_shrt_format_plac;
	disp_shrt_rfmt.combopic = "%1, %2";
}
/*===========================================================
 * disp_long_format_date -- Convert date according to options
 *=========================================================*/
static STRING
disp_long_format_date (STRING date)
{
	INT dfmt=0,mfmt=0,yfmt=0,sfmt=0,efmt=0, cmplx;
	INT n;
	STRING fmts, pic;

	if (!date) return NULL;

	n = 0;
	fmts = getlloptstr("LongDisplayDate", NULL);
	if (fmts) {
		/* try to use user-specified format */
		n = sscanf(fmts, "%ld,%ld,%ld,%ld,%ld,%ld" 
			, &dfmt, &mfmt, &yfmt, &sfmt, &efmt, &cmplx);
	}
	if (n != 6) {
		dfmt=mfmt=yfmt=sfmt=efmt=cmplx=0;
		sfmt=14; /* GEDCOM as is */
	}

	pic = getlloptstr("LongDisplayDatePic", NULL);
	if (pic && pic[0])
		set_date_pic(pic);
	
	return do_format_date(date, dfmt, mfmt, yfmt, sfmt, efmt, cmplx);
}
/*===============================================================
 * disp_shrt_format_date -- short form of date for display
 *  This is used for dates in option strings, and in single-line
 *  descriptions of people (ie, in event summaries).
 * Created: 2001/10/29 (Perry Rapp)
 *=============================================================*/
static STRING
disp_shrt_format_date (STRING date)
{
	INT dfmt=0,mfmt=0,yfmt=0,sfmt=0,efmt=0, cmplx;
	INT n;
	STRING fmts, pic;

	if (!date) return NULL;

	n = 0;
	fmts = getlloptstr("ShortDisplayDate", NULL);
	if (fmts) {
		/* try to use user-specified format */
		n = sscanf(fmts, "%ld,%ld,%ld,%ld,%ld,%ld"
			, &dfmt, &mfmt, &yfmt, &sfmt, &efmt, &cmplx);
	}
	if (n != 6) {
		dfmt=mfmt=yfmt=sfmt=cmplx=0;
		sfmt=12; /* old style short form -- year only */
	}

	pic = getlloptstr("ShortDisplayDatePic", NULL);
	if (pic && pic[0])
		set_date_pic(pic);

	return do_format_date(date, dfmt, mfmt, yfmt, sfmt, efmt, cmplx);
}
/*================================================================
 * disp_shrt_format_plac -- short form of place for display
 *  This is used for places in single-line descriptions of people
 *  (ie, in event summaries).
 * Created: 2001/10/29 (Perry Rapp)
 *==============================================================*/
static STRING
disp_shrt_format_plac (STRING plac)
{
	if (!plac) return NULL;
	return shorten_plac(plac);
}
