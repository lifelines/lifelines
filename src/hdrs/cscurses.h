/* 
   Copyright (c) 2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * cscurses.h -- layer on top of curses,
 *  providing conversion to internal codeset
 *==============================================================*/

int mvccuwaddstr(UIWINDOW uiw, int y, int x, const char *cp);
int mvccwaddnstr(WINDOW *wp, int y, int x, const char *cp, int n);
int mvccwaddstr(WINDOW *wp, int y, int x, const char *cp);
int mvccwprintw(WINDOW *wp, int y, int x, ...);
int vccprintf(const char *fmt, va_list args);
int vccwprintw(WINDOW *wp, const char *fmt, va_list args);
int wgetccnstr(WINDOW *wp, char *cp, int n);
