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
/*==============================================================
 * interface.c -- Interface between LifeLines and BTREE database
 * Copyright(c) 1992-4 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 18 Aug 93
 *   3.0.0 - 20 Aug 94    3.0.2 - 09 Nov 94
 *============================================================*/

#include "standard.h"
#include "btree.h"

extern BTREE BTR;

/*=================================================
 * retrieve_record -- Retrieve record from database
 *===============================================*/
STRING
retrieve_record (STRING key,
                 INT *plen)
{
	return (STRING) getrecord(BTR, str2rkey(key), plen);
}
/*=========================================
 * store_record -- Store record in database
 *=======================================*/
BOOLEAN
store_record (STRING key,
              STRING rec,
              INT len)
{
	return addrecord (BTR, str2rkey(key), rec, len);
}
/*=========================================
 * retrieve_file -- Retrieve record to file
 *=======================================*/
BOOLEAN
retrieve_file (STRING key,
               STRING file)
{
	return getfile(BTR, str2rkey(key), file);
}
/*=====================================
 * store_file -- Store record from file
 *===================================*/
BOOLEAN
store_file (STRING key,
            STRING file)
{
	return addfile(BTR, str2rkey(key), file);
}
