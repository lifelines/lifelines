/* 
   Copyright (c) 2002 Perry Rapp
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

struct zstr_s;

ZSTR zs_new(void);
ZSTR zs_newn(unsigned int min);
ZSTR zs_news(const char * str);
ZSTR zs_newz(ZSTR zsrc);
ZSTR zs_newvf(const char * fmt, va_list args);
ZSTR zs_newsubs(const char * str, unsigned int len);
void zs_free(ZSTR * pzstr);
char * zs_str(ZSTR);
unsigned int zs_len(ZSTR zstr);
unsigned int zs_allocsize(ZSTR zstr);
char * zs_fix(ZSTR zstr);
char * zs_set_len(ZSTR * pzstr, unsigned int len);
char * zs_sets(ZSTR * pzstr, const char *);
char * zs_setz(ZSTR * pzstr, ZSTR zsrc);
char * zs_apps(ZSTR * pzstr, const char *);
char * zs_appz(ZSTR * pzstr, ZSTR zrc);
char * zs_appc(ZSTR * pzstr, char);
char * zs_setf(ZSTR * pzstr, const char * fmt, ...);
char * zs_appf(ZSTR * pzstr, const char * fmt, ...);
char * zs_setvf(ZSTR * pzstr, const char * fmt, va_list args);
char * zs_appvf(ZSTR * pzstr, const char * fmt, va_list args);
char * zs_clear(ZSTR * pzstr);
char * zs_reserve(ZSTR * pzstr, unsigned int min);
char * zs_reserve_extra(ZSTR * pzstr, unsigned int delta);

#endif /* zstr_h_included */
