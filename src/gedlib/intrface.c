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
#include "gedcom.h"
#include "btree.h"

extern BTREE BTR;

/*=================================================
 * retrieve_raw_record -- Retrieve record string from database
 *  key:  [IN] key of desired record (eg, "    I543")
 *  plen: [OUT] length of record returned
 *===============================================*/
STRING
retrieve_raw_record (CNSTRING key, INT *plen)
{
	RKEY rkey = str2rkey(key);
	return bt_getrecord(BTR, &rkey, plen);
}
/*=========================================
 * store_record -- Store record in database
 *  key:  [IN] where to store record in db
 *  rec:  [IN] data to store
 *  len:  [IN] length of data to store
 *=======================================*/
BOOLEAN
store_record (CNSTRING key, STRING rec, INT len)
{
	return bt_addrecord (BTR, str2rkey(key), rec, len);
}
/*=========================================
 * retrieve_to_file -- Retrieve record to file
 *=======================================*/
RECORD_STATUS
retrieve_to_file (STRING key, STRING file)
{
	return write_record_to_file(BTR, str2rkey(key), file);
}
/*=========================================
 * retrieve_to_textfile -- Retrieve record to file
 *=======================================*/
RECORD_STATUS
retrieve_to_textfile (STRING key, STRING file, TRANSLFNC translfnc)
{
	return write_record_to_textfile(BTR, str2rkey(key), file, translfnc);
}
/*=====================================
 * store_file -- Store record from file
 *  key:  [IN] db key in which to store record
 *  file: [IN] file from which to get record
 *===================================*/
#ifdef UNUSED_CODE
BOOLEAN
store_file (STRING key, STRING file)
{
	return addfile(BTR, str2rkey(key), file);
}
#endif
/*=====================================
 * store_text_file_to_db -- Store record from text file
 *  key:  [IN] db key in which to store record
 *  file: [IN] file from which to get record
 * for MSDOS, translates \r\n in files to \n in record
 * Created: 2001/07/04 (Perry Rapp)
 *===================================*/
BOOLEAN
store_text_file_to_db (STRING key, CNSTRING file, TRANSLFNC transfnc)
{
	return addtextfile(BTR, str2rkey(key), file, transfnc);
}
/*===================================================
 * traverse_db_key_keys -- traverse a span of records
 *  using STRING keys
 *  either lo or hi can be NULL
 *=================================================*/
typedef struct
{
	BOOLEAN(*func)(CNSTRING key, STRING data, INT len, void * param);
	void * param;
} TRAV_PARAM;
/* see above */
static BOOLEAN
trav_callback (RKEY rkey, STRING data, INT len, void * param)
{
	TRAV_PARAM *tparam = (TRAV_PARAM *)param;
	char key[MAXKEYWIDTH+1];
	strcpy(key, rkey2str(rkey));
	return tparam->func(key, data, len, tparam->param);
}
/* see above */
void
traverse_db_rec_keys (CNSTRING lo, CNSTRING hi, 
	BOOLEAN(*func)(CNSTRING key, STRING data, INT len, void *param), void *param)
{
	RKEY lo1, hi1;
	TRAV_PARAM tparam;
	tparam.param = param;
	tparam.func = func;
	if (lo)
		lo1 = str2rkey(lo);
	else
		lo1.r_rkey[0] = 0;
	if (hi)
		hi1 = str2rkey(hi);
	else
		hi1.r_rkey[0] = 0;
	traverse_db_rec_rkeys(BTR, lo1, hi1, trav_callback, &tparam);
}
/*====================================================
 * traverse_db_key_recs -- traverse a span of records
 *  returns key & node
 *==================================================*/
typedef struct
{
	BOOLEAN(*func)(CNSTRING key, RECORD rec, void * param);
	void * param;
} TRAV_RECORD_PARAM;
/* see above */
static BOOLEAN
trav_rec_callback (CNSTRING key, STRING data, INT len, void * param)
{
	TRAV_RECORD_PARAM *tparam = (TRAV_RECORD_PARAM *)param;
	RECORD rec;
	BOOLEAN keepgoing;
	if (key[0]!='I' && key[0]!='F' && key[0]!='S' && key[0]!='E' && key[0]!='X')
		return TRUE;
	if (!strcmp(data, "DELE\n"))
		return TRUE;
	rec = string_to_record(data, key, len);
	keepgoing = tparam->func(key, rec, tparam->param);
	release_record(rec);
	return keepgoing;
}
/* see above */
void
traverse_db_key_recs (BOOLEAN(*func)(CNSTRING key, RECORD, void *param), void *param)
{
	TRAV_RECORD_PARAM tparam;
	CNSTRING lo,hi;
	tparam.param = param;
	tparam.func = func;
	lo = hi = NULL; /* all records */
	traverse_db_rec_keys(lo, hi, trav_rec_callback, &tparam);
}
/*=================================================
 * del_in_dbase -- Write deleted record to database
 *  Also update xreffile (module of free keys)
 *===============================================*/
void
del_in_dbase (CNSTRING key)
{
	if (!key || *key == 0) return;

/* Add key to list of unused keys */
	addxref(key); /* will assert if record already free (deleted) */

/* Free record in database - we just store deleted text */
	ASSERT(store_record(key, "DELE\n", 5));
}
/*=================================================
 * delete_record_missing_data_entry -- Delete a record which
 *  was orphaned in database (is in btree index
 *  but has no actual record)
 *===============================================*/
void
delete_record_missing_data_entry (CNSTRING key)
{
	/* simply deleting it will take care of it */
	del_in_dbase(key);
}
/*=================================================
 * mark_deleted_record_as_deleted -- Fix a record which
 * does not exist, but is not correctly on the free list
 * (that is, put it on the free list)
 * Ignores any invalid input
 *===============================================*/
BOOLEAN
mark_deleted_record_as_deleted (CNSTRING key)
{
	if (!key || !key[0]) return FALSE;
	return addxref_if_missing(key);
}
/*=================================================
 * mark_live_record_as_live -- Fix a record which exists
 * but is erroneously on the free list
 * Ignores any invalid input
 *===============================================*/
BOOLEAN
mark_live_record_as_live (CNSTRING key)
{
	if (!key || !key[0]) return FALSE;
	return delete_xref_if_present(key);
}
