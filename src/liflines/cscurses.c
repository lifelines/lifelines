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


/*============================
 * int_to_disp -- convert internal codeset to GUI
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
void
int_to_disp (ZSTR * pzstr)
{
	XLAT xlat = transl_get_predefined_xlat(MINDS);
	if (xlat)
		transl_xlat(xlat, pzstr); /* ignore failure */
}
/*============================
 * disp_to_int -- convert GUI codeset to internal
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
void
disp_to_int (ZSTR * pzstr)
{
	XLAT xlat = transl_get_predefined_xlat(MDSIN);
	if (xlat)
		transl_xlat(xlat, pzstr); /* ignore failure */
}
/*============================
 * mvcwaddstr -- convert to GUI codeset & call mvwaddstr
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
mvccwaddstr (WINDOW *wp, int y, int x, char *cp)
{
	ZSTR zstr = zs_news(cp);
	int rtn;
	int_to_disp(&zstr);
	rtn = mvwaddstr(wp, y, x, zs_str(zstr));
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
vccwprintw (WINDOW *wp, char *fmt, va_list args)
{
	ZSTR zstr = zs_newvf(fmt, args);
	int rtn;
	int_to_disp(&zstr);
	rtn = waddstr(wp, zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
/*============================
 * vccprintf -- vprintf with codeset convert from internal to GUI
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
vccprintf (char *fmt, va_list args)
{
	int rtn;
	ZSTR zstr = zs_newvf(fmt, args);
	int_to_disp(&zstr);
	rtn = printf(zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
/*============================
 * wgetccnstr -- wgetnstr with codeset convert from internal to GUI
 * Created: 2002/12/03 (Perry Rapp)
 *==========================*/
int
wgetccnstr (WINDOW *wp, char *cp, int n)
{
	ZSTR zstr=0;
	int rtn = wgetnstr(wp, cp, n);
	zstr = zs_news(cp);
	disp_to_int(&zstr);
	llstrsets(cp, n, uu8, zs_str(zstr));
	zs_free(&zstr);
	return rtn;
}
