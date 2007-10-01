/* 
   Copyright (c) 2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * cscurses.c -- layer on top of curses,
 *  providing conversion to internal codeset
 *==============================================================*/

#include "llstdlib.h"
#include "liflines.h"
#include "llinesi.h"
#include "screen.h"
#include "cscurses.h"
#include "zstr.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void disp_to_int(ZSTR zstr);
static void int_to_disp(ZSTR zstr);
static size_t output_width(ZSTR zstr);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*============================
 * int_to_disp -- convert internal codeset to GUI
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
void
int_to_disp (ZSTR zstr)
{
	XLAT xlat = transl_get_predefined_xlat(MINDS);
	if (xlat)
		transl_xlat(xlat, zstr); /* ignore failure */
}
/*============================
 * disp_to_int -- convert GUI codeset to internal
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
void
disp_to_int (ZSTR zstr)
{
	XLAT xlat = transl_get_predefined_xlat(MDSIN);
	if (xlat)
		transl_xlat(xlat, zstr); /* ignore failure */
}
/*============================
 * output_width -- display width of string in characters
 *==========================*/
static size_t
output_width (ZSTR zstr)
{
	if (gui8) {
		/* TODO: Need to use wcswidth here */
		return str8chlen(zs_str(zstr));
	} else {
		return zs_len(zstr);

	}
}
/*============================
 * mvcuwaddstr -- convert to GUI codeset & output to screen
 * Created: 2002/12/13 (Perry Rapp)
 *==========================*/
int
mvccuwaddstr (UIWINDOW uiw, int y, int x, const char *cp)
{
	return mvccwaddnstr(uiw_win(uiw), y, x, cp, uiw_cols(uiw));
}
/*============================
 * mvcwaddstr -- convert to GUI codeset & call mvwaddstr
 * Created: 2002/12/03 (Perry Rapp)
 * TODO: Convert all calls of this to call mvccuwaddstr (or mvccwaddnstr) !
 *==========================*/
int
mvccwaddstr (WINDOW *wp, int y, int x, const char *cp)
{
	ZSTR zstr = zs_news(cp);
	int rtn;
	int_to_disp(zstr);
	rtn = mvwaddstr(wp, y, x, zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
/*============================
 * mvcwaddnstr -- convert to GUI codeset & call mvwaddstr with length limit
 * Created: 2002/12/13 (Perry Rapp)
  *==========================*/
int
mvccwaddnstr (WINDOW *wp, int y, int x, const char *cp, int n)
{
	ZSTR zstr = zs_news(cp);
	int rtn=0;
	
	int_to_disp(zstr);

	if (zs_len(zstr) < (unsigned int)n) {
		rtn = mvwaddstr(wp, y, x, zs_str(zstr));
	} else {
		if (output_width(zstr) > (size_t)n) {
			STRING str = zs_str(zstr);
			/* We need to do length truncation correctly for UTF-8 output */
			/* #1) We need to not break UTF-8 multibytes */

			INT width=0;
			STRING prev = find_prev_char(&str[n-1], &width, str, gui8);
			width += (prev - str);
			zs_chop(zstr, width);

			/* #2) We should account for zero-width characters, eg, use wcwidth */
			/* Unfortunately, lifelines doesn't yet use wcwidth or config test it */
			/* TODO:
			config test for wcswidth
			and substitute Markus Kuhn's
			http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
			*/
		}
		rtn = mvwaddnstr(wp, y, x, zs_str(zstr), n);
	}
	zs_free(&zstr);
	return rtn;
}
/*============================
 * mvccwprintw -- mvwprintw with codeset convert from internal to GUI
 * mvcwaddstr -- convert to GUI codeset & call mvwaddstr
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
mvccwprintw (WINDOW *wp, int y, int x, ...)
{
	va_list args;
	char * fmt;
	va_start(args, x);
	wmove(wp, y, x);
	fmt = va_arg(args, char *);
	return vccwprintw(wp, fmt, args);
}	
/*============================
 * vccwprintw -- vwprintw with codeset convert from internal to GUI
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
vccwprintw (WINDOW *wp, const char *fmt, va_list args)
{
	ZSTR zstr = zs_newvf(fmt, args);
	int rtn;
	int_to_disp(zstr);
	rtn = waddstr(wp, zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
/*============================
 * vccprintf -- vprintf with codeset convert from internal to GUI
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
vccprintf (const char *fmt, va_list args)
{
	int rtn;
	ZSTR zstr = zs_newvf(fmt, args);
	int_to_disp(zstr);
	rtn = printf(zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
/*============================
 * wgetccnstr -- wgetnstr with codeset convert from GUI to internal
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
wgetccnstr (WINDOW *wp, char *cp, int n)
{
	ZSTR zstr=0;
/* TODO: Need Win32-specific code here to handle Unicode input on NT family */
	int rtn = wgetnstr(wp, (char *)cp, n);
	zstr = zs_news(cp);
	disp_to_int(zstr);
	llstrsets(cp, n, uu8, zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
