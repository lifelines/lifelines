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

#include "llstdlib.h"
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
/*====================================================
 * traverse_db_key_skeys -- traverse a span of records
 *  using STRING keys
 *==================================================*/
typedef struct
{
	BOOLEAN(*func)(STRING key, STRING data, INT len, void * param);
	void * param;
} TRAV_PARAM;
static BOOLEAN
trav_callback(RKEY rkey, STRING data, INT len, void * param)
{
	TRAV_PARAM *tparam = (TRAV_PARAM *)param;
	unsigned char skey[9];
	strcpy(skey, rkey2str(rkey));
	return tparam->func(skey, data, len, tparam->param);
}
void
traverse_db_rec_keys(STRING lo, STRING hi, 
	BOOLEAN(*func)(STRING key, STRING, INT len, void *param), void *param)
{
	TRAV_PARAM tparam;
	tparam.param = param;
	tparam.func = func;
	traverse_db_rec_rkeys(BTR, str2rkey(lo), str2rkey(hi), trav_callback, &tparam);
}
/*=================================================
 * del_in_dbase -- Write deleted record to database
 *===============================================*/
void
del_in_dbase (STRING key)
{
	if (!key || *key == 0) return;
	ASSERT(store_record(key, "DELE\n", 5));
}
