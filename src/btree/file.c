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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * file.c -- File access to btree database
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    3.0.0 - 24 Sep 94
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "btreei.h"


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN addfile_impl(BTREE btree, RKEY rkey, CNSTRING file
	, STRING mode, TRANSLFNC translfnc);
static RECORD_STATUS write_record_to_file_impl (BTREE btree, RKEY rkey, STRING file
	, TRANSLFNC translfnc, STRING mode);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=========================================
 * addfile -- Add record to btree from file
 *  btree:  [in] btree to add to
 *  rkey:   [in] key of new record
 *  file:   [in] file with new record
 *=======================================*/
BOOLEAN
addfile (BTREE btree, RKEY rkey, STRING file)
{
	TRANSLFNC translfnc = NULL; /* no translation for binary files */
	return addfile_impl(btree, rkey, file, LLREADBINARY, translfnc);
}
/*=========================================
 * addtextfile -- Add record to btree from text file
 *  btree:  [in] btree to add to
 *  rkey:   [in] key of new record
 *  file:   [in] file with new record
 *  tt:     [in] translation table for text
 * handles problem of MSDOS files \n <-> \r\n
 * internally records are kept with just \n, so db is portable
 *=======================================*/
BOOLEAN
addtextfile (BTREE btree, RKEY rkey, CNSTRING file, TRANSLFNC translfnc)
{
	return addfile_impl(btree, rkey, file, LLREADTEXT, translfnc);
}
/*=========================================
 * addfile_impl -- Add record to btree from file
 *  btree:  [in] btree to add to
 *  rkey:   [in] key of new record
 *  file:   [in] file with new record
 *=======================================*/
static BOOLEAN
addfile_impl (BTREE btree, RKEY rkey, CNSTRING file, STRING mode, TRANSLFNC translfnc)
{
	FILE *fp = NULL;
	STRING mem = 0, mem1 = 0;
	INT siz;
	struct stat buf;
	BOOLEAN result=FALSE;
	ASSERT(bwrite(btree) == 1);
	if ((fp = fopen(file, mode)) == NULL) goto end;
	if (fstat(fileno(fp), &buf) != 0) goto end;
	if (buf.st_size == 0) {
		/* why do we add a record here, esp. when filesize=0? */
		bt_addrecord(btree, rkey, mem, 0);
		result = TRUE;
		goto end;
	}
	if ((mem = stdalloc(buf.st_size+1)) == NULL) goto end;
	/* WARNING: with WIN32 reading in TEXT mode, fewer characters
	 * will be read than expected because of conversion of
	 * \r\n to \n
	 */
	siz = fread(mem, 1, buf.st_size, fp);
	mem1 = mem;
	if (ferror(fp)) goto end;
	mem[siz]=0;
	/* skip BOM (byte order marker) at top of unicode text file */
	if (eqstr(mode, LLREADTEXT))
		skip_BOM(&mem);
	if (translfnc) {
		STRING mem2 = (*translfnc)(mem, siz);
		stdfree(mem1);
		mem = mem2;
		mem1 = mem2;
		siz = strlen(mem);
	}
	bt_addrecord(btree, rkey, mem, siz);
	result = TRUE;
end:
	if (mem) stdfree(mem1);
	if (fp) fclose(fp);
	return result;
}
/*===================================================
 * write_record_to_file -- Get record from btree and write to file
 *  btree: [in] database btree
 *  rkey:  [in] record key
 *  file:  [in] file name
 * returns RECORD_SUCCESS, RECORD_NOT_FOUND, RECORD_ERROR
 *=================================================*/
RECORD_STATUS
write_record_to_file (BTREE btree, RKEY rkey, STRING file)
{
	TRANSLFNC translfnc = 0; /* no translation for binary files */
	return write_record_to_file_impl(btree, rkey, file, translfnc, LLWRITEBINARY);
}
/*===================================================
 * write_record_to_textfile -- Get record from btree and write to file
 *  btree: [in] database btree
 *  rkey:  [in] record key
 *  file:  [in] file name
 * returns RECORD_SUCCESS, RECORD_NOT_FOUND, RECORD_ERROR
 * handles textfiles for MSDOS \n <->\r\n
 * internally records are kept with just \n, so db is portable
 *=================================================*/
RECORD_STATUS
write_record_to_textfile (BTREE btree, RKEY rkey, STRING file, TRANSLFNC translfnc)
{
	return write_record_to_file_impl(btree, rkey, file, translfnc, LLWRITETEXT);
}
/*===================================================
 * write_record_to_file_impl -- Get record from btree and write to file
 *  btree:     [in] database btree
 *  rkey:      [in] record key
 *  file:      [in] file name
 *  translfnc: [in] optional text translation function (for caller-specified translation)
 *  mode:      [in] fopen mode (for MSDOS text file problem)
 * returns RECORD_SUCCESS, RECORD_NOT_FOUND, RECORD_ERROR
 * Originally named write_record_to_file
 *=================================================*/
static RECORD_STATUS
write_record_to_file_impl (BTREE btree, RKEY rkey, STRING file
	, TRANSLFNC translfnc, STRING mode)
{
	FILE *fp;
	INT len;
	INT siz;
	RAWRECORD record = bt_getrecord(btree, &rkey, &len);
	if (record == NULL)
		return RECORD_NOT_FOUND;
	if (translfnc) {
		STRING rec2 = (*translfnc)(record, len);
		stdfree(record);
		record = rec2;
		len = strlen(record);
	}
	if ((fp = fopen(file, mode)) == NULL) {
		stdfree(record);
		return RECORD_ERROR;
	}
	/* WARNING: with WIN32 writing in TEXT mode, more characters
	 * will be written than expected because of conversion of
	 * \n to \r\n
	 */
	siz = fwrite(record, 1, len, fp);
	if (ferror(fp)) {
		stdfree(record);
		fclose(fp);
		return RECORD_ERROR;
	}
	stdfree(record);
	if (fclose(fp) != 0)
		return RECORD_ERROR;
	return RECORD_SUCCESS;
}
