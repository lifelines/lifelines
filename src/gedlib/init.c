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
/* modified 17 Aug 2000 by Matt Emmerton (matt@gsicomp.on.ca)  */
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * init.c -- Initialize LifeLines data structures
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 05 Oct 94    3.0.2 - 09 Nov 94
 *   3.0.3 - 21 Sep 95
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "arch.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "version.h"
#include "lloptions.h"

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable;		/* table for tag strings */
TABLE placabbvs;	/* table for place abbrevs */
TABLE useropts;		/* table for user options */
BTREE BTR = NULL;	/* database */
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN selftest, writeable;
extern STRING btreepath,readpath;

/*********************************************
 * local function prototypes
 *********************************************/

static STRING getsaveenv(STRING key);

/*********************************************
 * local variables
 *********************************************/

static int rdr_count = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=================================
 * init_lifelines_global -- Initialize LifeLines
 *  before db opened
 * STRING * pmsg: heap-alloc'd error string if fails
 *===============================*/
BOOLEAN
init_lifelines_global (STRING * pmsg)
{
	STRING e;
	*pmsg = NULL;
	read_lloptions_from_config();
	if (lloptions.lleditor[0])
		e = lloptions.lleditor;
	else
		e = environ_determine_editor(PROGRAM_LIFELINES);
	editfile = environ_determine_tempfile();
	if (!editfile) {
		*pmsg = strsave("Error creating temp file");
		return FALSE;
	}
	editfile = strsave(editfile );
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	/* read dirs from env if lacking */
	if (!lloptions.llprograms[0])
		changeoptstr(&lloptions.llprograms, getsaveenv("LLPROGRAMS"));
	if (!lloptions.llreports[0])
		changeoptstr(&lloptions.llreports, getsaveenv("LLREPORTS"));
	if (!lloptions.llarchives[0])
		changeoptstr(&lloptions.llarchives, getsaveenv("LLARCHIVES"));
	if (!lloptions.lldatabases[0])
		changeoptstr(&lloptions.lldatabases, getsaveenv("LLDATABASES"));
	if (!lloptions.llnewdbdir[0])
		changeoptstr(&lloptions.llnewdbdir, getsaveenv("LLNEWDBDIR"));
	if (selftest) {
		/* need to always find test stuff locally */
		changeoptstr(&lloptions.llprograms, NULL);
		changeoptstr(&lloptions.llreports, NULL);
		changeoptstr(&lloptions.lldatabases, NULL);
		changeoptstr(&lloptions.llnewdbdir, NULL);
	}
	/* fallback for dirs is . */
	if (!lloptions.llprograms[0])
		changeoptstr(&lloptions.llprograms, strsave("."));
	if (!lloptions.llreports[0])
		changeoptstr(&lloptions.llreports, strsave("."));
	if (!lloptions.llarchives[0])
		changeoptstr(&lloptions.llarchives, strsave("."));
	if (!lloptions.lldatabases[0])
		changeoptstr(&lloptions.lldatabases, strsave("."));
	if (!lloptions.llnewdbdir[0])
		changeoptstr(&lloptions.llnewdbdir, strsave("."));
	return TRUE;
}
/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
void
init_lifelines_db (void)
{
	STRING emsg;

	tagtable = create_table();
	placabbvs = create_table();
	useropts = create_table();
	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", useropts, '=', &emsg);
	init_caches();
	init_browse_lists();
	init_mapping();
	read_lloptions_from_db();
	openxref();
}
/*===============================================
 * getsaveenv -- Return strsave'd env value
 *  returns saved("") if getenv was NULL
 * Created: 2001/02/04, Perry Rapp
 *=============================================*/
static STRING
getsaveenv (STRING key)
{
	STRING val = getenv(key);
	if (!val)
		val = "";
	return strsave(val);
}
/*===============================================
 * get_lifelines_version -- Return version string
 *  returns static buffer
 *=============================================*/
STRING
get_lifelines_version (INT maxlen)
{
	static char version[128];
	char *ptr=version;
	INT len=sizeof(version);
	if (len>maxlen)
		len=maxlen;
	llstrcatn(&ptr, LIFELINES_VERSION, &len);
	return version;
}
/*===================================
 * close_lifelines -- Close LifeLines
 *  Safe to call even if not opened
 *=================================*/
void
close_lifelines (void)
{
	closexref();
	unlink(editfile);
	if(BTR) {
		closebtree(BTR);
		BTR=NULL;
	}
	if (editfile) {
		stdfree(editfile);
		editfile=NULL;
	}
	if (editstr) {
		stdfree(editstr);
		editstr=NULL;
	}
}
/*==================================================
 * force_open_db -- open a writer-locked database
 *  returns FALSE & sets bterrno if error (eg, keyfile corrupt)
 *================================================*/
static BOOLEAN
force_open_db ()
{
	/*
	Forcefully alter reader/writer count to 0.
	But do check keyfile2 checks first (in case it is a
	database from a different alignment).
	*/
	char scratch[200];
	FILE *fp=NULL;
	BOOLEAN result=FALSE;
	KEYFILE1 kfile1;
	KEYFILE2 kfile2;
	struct stat sbuf;
	sprintf(scratch, "%s/key", readpath);
	if (stat(scratch, &sbuf) || !S_ISREG(sbuf.st_mode)) {
		bterrno = BTERR_KFILE;
		goto force_open_db_exit;
	}
	if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
		  fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		bterrno = BTERR_KFILE;
		goto force_open_db_exit;
	}
	if (fread(&kfile2, sizeof(kfile2), 1, fp) == 1) {
		if (!validate_keyfile2(&kfile2)) {
			/* validate set bterrno */
			goto force_open_db_exit;
		}
	}
	kfile1.k_ostat = 0;
	rewind(fp);
	if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		bterrno = BTERR_KFILE;
		goto force_open_db_exit;
	}
	/* ok everything went successfully */
	result=TRUE;

force_open_db_exit:
	if (fp) fclose(fp);
	return result;
}
/*==================================================
 * open_database_impl -- open database
 * forceopen: user requesting override of write-locked db
 *  uses globals btreepath & readpath
 *  btreepath: database to report
 *  readpath: actual database path (may be relative also)
 * Upon failure, sets bterrno and returns false
 *================================================*/
static BOOLEAN
open_database_impl (BOOLEAN forceopen)
{
	int c;

	if (forceopen) {
		if (!force_open_db()) {
			return FALSE;
		}

		/* okay, cleared reader/writer count
		now fall through to normal opening code */
	}
	/* call btree module to do actual open of BTR */
	if (!(BTR = openbtree(readpath, FALSE, !readonly)))
		return FALSE;
	readonly = !bwrite(BTR);
	if (readonly && writeable) {
		int myerr=0;
		c = bkfile(BTR).k_ostat;
		if (c < 0) {
			myerr = BTERR_WRITER;
		} else {
			rdr_count = c-1;
			myerr = BTERR_READERS;
		}
		/* close_lifelines could set bterrno itself */
		close_lifelines();
		bterrno = myerr;
		return FALSE;
	}
	return TRUE;
}
/*==================================================
 * open_database -- open database
 * dbrequested: database to report
 * dbused: actual database path (may be relative also)
 *================================================*/
BOOLEAN
open_database (BOOLEAN forceopen, STRING dbrequested, STRING dbused)
{
	BOOLEAN rtn;

	/* tentatively copy paths into gedlib module versions */
	btreepath=strsave(dbrequested);
	readpath=strsave(dbused);

	rtn = open_database_impl(forceopen);
	if (!rtn) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		close_lifelines();
		bterrno = myerr;
		strfree(&btreepath);
		strfree(&readpath);
	}
	return rtn;
}
/*===================================================
 * describe_dberror -- Describe database opening error
 * dberr: [in] error whose description is sought
 * buffer: [out] buffer for description
 * buflen: [in]  size of buffer
 *=================================================*/
void
describe_dberror (INT dberr, STRING buffer, INT buflen)
{
	char localbuff[128];
	STRING ptr = buffer;
	STRING msg;
	INT mylen = buflen;

	if (dberr != BTERR_WRITER)
		llstrcatn(&ptr, "Database error -- ", &mylen);

	switch (dberr) {
	case BTERR_NODB:
		msg = "requested database does not exist.";
		break;
	case BTERR_DBBLOCKEDBYFILE:
		msg = "db directory is file, not directory.";
		break;
	case BTERR_DBCREATEFAILED:
		msg = "creation of new database failed.";
		break;
	case BTERR_DBACCESS:
		msg = "error accessing database directory.";
		break;
	case BTERR_NOKEY:
		msg = "no keyfile (directory does not appear to be a database.";
		break;
	case BTERR_INDEX:
		msg = "could not open, read or write an index file.";
		break;
	case BTERR_KFILE:
		msg = "could not open, read or write the key file.";
		break;
	case BTERR_BLOCK:
		msg = "could not open, read or write a block file.";
		break;
	case BTERR_LNGDIR:
		msg = "name of database is too long.";
		break;
	case BTERR_WRITER:
		msg = "The database is already open for writing.";
		break;
	case BTERR_ILLEGKF:
		msg = "keyfile is corrupt.";
		break;
	case BTERR_ALIGNKF:
		msg = "keyfile is wrong alignment.";
		break;
	case BTERR_VERKF:
		msg = "keyfile is wrong version.";
		break;
	case BTERR_EXISTS:
		msg = "Existing database found.";
		break;
	case BTERR_READERS:
		sprintf(localbuff
			, "The database is already opened for read access by %d users.\n  "
			, rdr_count);
		msg = localbuff;
	default:
		msg = "Undefined database error -- This can't happen.";
		break;
	}
	llstrcatn(&ptr, msg, &mylen);
}
