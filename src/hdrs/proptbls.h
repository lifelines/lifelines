/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * proptbls.h -- functions for making lists or arrays of property tables
 *  for files (ie, a property table for each file)
 *==============================================================*/

#ifndef proptbls_h_included
#define proptbls_h_included

/* proptbls.c */
typedef int (*SELECT_FNC)(const struct dirent *);
INT add_dir_files_to_proplist(CNSTRING dir, SELECT_FNC selectfnc, LIST list);
INT add_path_files_to_proplist(CNSTRING path, SELECT_FNC selectfnc, LIST list);
void add_prop_dnum(TABLE props, CNSTRING name, CNSTRING value);
TABLE * convert_proplist_to_proparray(LIST list);
void free_proparray(TABLE ** props);
TABLE * get_proparray_of_files_in_path(CNSTRING path, SELECT_FNC selectfnc, INT * nfiles);
void set_prop_dnum(TABLE props, INT n, CNSTRING name, CNSTRING value);

#endif /* proptbls_h_included */
