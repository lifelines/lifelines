/* 
   Copyright (c) 2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*===========================================================
 * xlat.h -- Header file for translation module
 * xlat module converts between named codesets
 * It uses iconv & charmaps as steps in a chain, as needed
 *=========================================================*/

#ifndef xlat_h_included
#define xlat_h_included

/* xlat.c */
BOOLEAN xl_do_xlat(XLAT xlat, ZSTR zstr);
void xl_free_adhoc_xlats(void);
void xl_free_xlats(void);
ZSTR xlat_get_description(XLAT xlat);
CNSTRING xl_get_dest_codeset(XLAT xlat);
TRANTABLE xl_get_legacy_tt(XLAT xlat);
XLAT xl_get_null_xlat(void);
INT xl_get_uparam(XLAT);
XLAT xl_get_xlat(CNSTRING src, CNSTRING dest, BOOLEAN adhoc);
BOOLEAN xl_is_xlat_valid(XLAT xlat);
void xl_load_all_dyntts(CNSTRING ttpath);
void xl_parse_codeset(CNSTRING codeset, ZSTR zcsname, LIST * subcodes);
void xl_release_xlat(XLAT xlat);
void xl_set_uparam(XLAT, INT uparam);
void xlat_shutdown(void);

#endif /* xlat_h_included */
