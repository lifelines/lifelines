/* 
   Copyright (c) 2003 Matt Emmerton

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
 * pvalue.h -- Storage for PVALUE
 * Copyright(c) 2003 by Matt Emmerton; all rights reserved
 *===========================================================*/
#ifndef _PVALUE_H
#define _PVALUE_H

#include "array.h"      /* for ARRAY */
#include "list.h"       /* for LIST */
#include "cache.h"      /* for CACHEEL */
#include "gedcom.h"     /* for NODE, RECORD */
#include "indiseq.h"    /* for INDISEQ */
#include "table.h"      /* for TABLE */

typedef union {
	/* "basic" types, should be same as UNION */
        BOOLEAN b;
        INT     i;
        FLOAT   f;
	STRING	s;
        VPTR    p;	/* Try not to use this! */
	/* "complex" types */
	NODE	n;
	ARRAY	a;
	LIST	l;
	CACHEEL	c;
	RECORD	r;
	INDISEQ	q;
	TABLE	t;
} PVALUE_DATA;

#endif /* _PVALUE_H */
