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
 *   2.3.4 - 24 Jun 93    2.3.5 - 20 Aug 93
 *   3.0.2 - 22 Dec 94
 *===========================================================*/

#ifndef _TABLE_H
#define _TABLE_H

#include "standard.h"

#define MAXHASH 512

#define DONTFREE  0
#define FREEKEY   1
#define FREEVALUE 2
#define FREEBOTH  3

/*
tables hold key,value pairs, but all values in a specific
table must be of the same UNION type
INT, or VPTR, or STRING
(and ASSERTs in the insert_table_xxx functions enforce this)
*/
typedef struct etag *ENTRY;
struct etag {
	STRING ekey;
	UNION uval;
	ENTRY enext;
};

/*
	table_s created 2001/01/20 by Perry Rapp
	for reference counting when used in pvalues
	(they were being leaked)
*/
struct table_s {
	ENTRY *entries;
	INT count; /* #entries */
	INT refcnt; /* used by pvalues which share table pointers */
	INT valtype; /* TB_VALTYPE enum in table.c */
};
typedef struct table_s *TABLE;

TABLE create_table(void);
void insert_table_ptr(TABLE, STRING key, VPTR);
void insert_table_int(TABLE, STRING key, INT);
void insert_table_str(TABLE, STRING key, STRING);
void delete_table(TABLE, STRING);
void remove_table(TABLE, INT whattofree);
void traverse_table(TABLE, void (*proc)(ENTRY));
void traverse_table_param(TABLE tab, INT (*tproc)(ENTRY, VPTR), VPTR param);
BOOLEAN in_table(TABLE, STRING);
VPTR valueof_ptr(TABLE, STRING);
INT valueof_int(TABLE, STRING, INT defval);
STRING valueof_str(TABLE, STRING);
VPTR valueofbool_ptr(TABLE, STRING, BOOLEAN*);
INT valueofbool_int(TABLE, STRING, BOOLEAN*);
STRING valueofbool_str(TABLE, STRING, BOOLEAN*);
VPTR * access_value_ptr(TABLE tab, STRING key);
INT * access_value_int(TABLE tab, STRING key);
/*
access_value_str can't be done because of type limitation in implementation
(UNION doesn't have STRING inside it
*/

#endif /* _TABLE_H */
