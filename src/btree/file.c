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
 * file.c -- File access to btree database
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    3.0.0 - 24 Sep 94
 *===========================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "standard.h"
#include "btree.h"

/*=========================================
 * addfile -- Add record to btree from file
 *=======================================*/
BOOLEAN addfile (btree, rkey, file)
BTREE btree;	/* btree to add to */
RKEY rkey;	/* key of new record */
STRING file;	/* file with new record */
{
	FILE *fp;
	STRING mem;
	INT len;
	struct stat buf;
	ASSERT(bwrite(btree));
	if ((fp = fopen(file, "r")) == NULL) return FALSE;
	if (fstat(fileno(fp), &buf) != 0) {
		fclose(fp);
		return FALSE;
	}
	if ((len = buf.st_size) == 0) {
		addrecord(btree, rkey, mem, 0);
		fclose(fp);
		return TRUE;
	}
	if ((mem = (STRING) stdalloc(buf.st_size)) == NULL) {
		fclose(fp);
		return FALSE;
	}
	if (fread(mem, buf.st_size, 1, fp) != 1) {
		fclose(fp);
		return FALSE;
	}
	addrecord(btree, rkey, mem, buf.st_size);
	stdfree(mem);
	fclose(fp);
	return TRUE;
}
/*===================================================
 * getfile -- Get record from btree and write to file
 *=================================================*/
BOOLEAN getfile (btree, rkey, file)
BTREE btree;	/* btree to get from */
RKEY rkey;	/* key of record */
STRING file;	/* file to write to */
{
	FILE *fp;
	INT len;
	RECORD record = getrecord(btree, rkey, &len);
	if (record == NULL) return FALSE;
	if ((fp = fopen(file, "w")) == NULL) {
		stdfree(record);
		return FALSE;
	}
	if (fwrite(record, len, 1, fp) != 1) {
		stdfree(record);
		fclose(fp);
		return FALSE;
	}
	stdfree(record);
	fclose(fp);
	return TRUE;
}
