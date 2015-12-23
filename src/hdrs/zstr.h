/* 
   Copyright (c) 2002-2007 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*======================================================================
 * zstr.h -- dynamic buffer strings in C
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 *====================================================================*/

#ifndef zstr_h_included
#define zstr_h_included

struct tag_zstr;

ZSTR zs_new(void);
ZSTR zs_newn(unsigned int min);
ZSTR zs_news(const char * str);
ZSTR zs_newz(ZCSTR zsrc);
ZSTR zs_newf(const char * fmt, ...);
ZSTR zs_newvf(const char * fmt, va_list args);
ZSTR zs_newsubs(const char * str, unsigned int len);
void zs_free(ZSTR * pzstr);
void zs_move(ZSTR zstr, ZSTR * pzsrc);
char * zs_str(ZCSTR);
unsigned int zs_len(ZCSTR zstr);
unsigned int zs_allocsize(ZCSTR zstr);
char * zs_fix(ZSTR zstr);
void zs_chop(ZSTR zstr, unsigned int len);
char * zs_set_len(ZSTR zstr, unsigned int len);
char * zs_sets(ZSTR zstr, const char *);
char * zs_setz(ZSTR zstr, ZCSTR zsrc);
char * zs_apps(ZSTR zstr, const char *);
char * zs_appz(ZSTR zstr, ZCSTR zrc);
char * zs_appc(ZSTR zstr, char);
char * zs_setf(ZSTR zstr, const char * fmt, ...);
char * zs_appf(ZSTR zstr, const char * fmt, ...);
char * zs_setvf(ZSTR zstr, const char * fmt, va_list args);
char * zs_appvf(ZSTR zstr, const char * fmt, va_list args);
char * zs_clear(ZSTR zstr);
char * zs_reserve(ZSTR zstr, unsigned int min);
char * zs_reserve_extra(ZSTR zstr, unsigned int delta);

#endif /* zstr_h_included */
