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
 * table.h -- Header file for hashed key table
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 20 Aug 93
 *   3.0.2 - 22 Dec 94
 *===========================================================*/

#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include "standard.h"


/*
tables are homogenous (a table of integers can only hold integers)
*/

typedef struct tag_table *TABLE;
typedef struct tag_table_iter * TABLE_ITER;

/* initialize table module, and any of its private modules */
void init_table_module(void);

/* creating and deleting table */
TABLE create_table_int(void);
TABLE create_table_str(void);
TABLE create_table_hptr(void);
TABLE create_table_vptr(void);
TABLE create_table_custom_vptr(void (*destroyel)(void *ptr));
TABLE create_table_obj(void);
void destroy_table(TABLE tab);
void addref_table(TABLE tab);
void release_table(TABLE tab);

/* copying table */
void copy_table(const TABLE src, TABLE dest);

/* working with entire table */
INT get_table_count(TABLE);

/* inserting elements */
void insert_table_int(TABLE, CNSTRING key, INT ival);
void insert_table_str(TABLE tab, CNSTRING key, CNSTRING value);
void insert_table_ptr(TABLE, CNSTRING key, VPTR ptr);
void insert_table_obj(TABLE, CNSTRING key, VPTR obj);

/* replacing elements */
void replace_table_str(TABLE tab, CNSTRING key, CNSTRING str);

/* deleting elements*/
void delete_table_element(TABLE tab, CNSTRING key);

/* working with elements of table */
BOOLEAN in_table(TABLE, CNSTRING);
INT valueof_int(TABLE tab, CNSTRING key);
VPTR valueof_obj(TABLE tab, CNSTRING key);
VPTR valueof_ptr(TABLE tab, CNSTRING key);
STRING valueof_str(TABLE tab, CNSTRING key);
INT valueofbool_int(TABLE tab, CNSTRING key, BOOLEAN *there);
VPTR valueofbool_obj(TABLE tab, CNSTRING key, BOOLEAN *there);
VPTR valueofbool_ptr(TABLE tab, CNSTRING key, BOOLEAN *there);
STRING valueofbool_str(TABLE tab, CNSTRING key, BOOLEAN *there);

/* shortcut for incrementing int value */
void increment_table_int(TABLE tab, CNSTRING key);

/* table iteration */
TABLE_ITER begin_table_iter(TABLE tab);
BOOLEAN next_table_ptr(TABLE_ITER tabit, CNSTRING *pkey, VPTR *pptr);
BOOLEAN next_table_int(TABLE_ITER tabit, CNSTRING *pkey, INT * pival);
void end_table_iter(TABLE_ITER * ptabit);


#endif /* TABLE_H_INCLUDED */
