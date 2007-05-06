/* 
   Copyright (c) 1991-2005 Thomas T. Wetmore IV

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
 * dbcontext.c -- LifeLines database code
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "arch.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "version.h"
#include "lloptions.h"
#include "codesets.h"
#include "menuitem.h"
#include "zstr.h"
#include "icvt.h"
#include "date.h"
#include "mychar.h"
#include "charprops.h"
#include "xlat.h"
#include "dbcontext.h"


/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN writeable;
extern STRING readpath,readpath_file;

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN alterdb(INT alteration, INT *lldberr);
static BOOLEAN open_database_impl(LLDATABASE lldb, INT alteration, INT *lldberr);

/*********************************************
 * local variables
 *********************************************/

static void (*f_dbnotify)(STRING db, BOOLEAN opening) = 0;
static int rdr_count = 0;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/
 
 /*==================================================
 * alterdb -- force open, lock, or unlock a database
 *  alteration: [in] 1=unlock, 2=lock, 3=force open
 *  returns FALSE & sets lldberr if error (eg, keyfile corrupt)
 *================================================*/
static BOOLEAN
alterdb (INT alteration, INT *lldberr)
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
		*lldberr = BTERR_KFILE_ALTERDB;
		goto force_open_db_exit;
	}
	if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
		  fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		*lldberr = BTERR_KFILE_ALTERDB;
		goto force_open_db_exit;
	}
	if (fread(&kfile2, sizeof(kfile2), 1, fp) == 1) {
		if (!validate_keyfile2(&kfile2, lldberr)) {
			/* validate set lldberr */
			goto force_open_db_exit;
		}
	}
	if (alteration == 1) {
		/* unlock db */
		if (kfile1.k_ostat != -2) {
			/* can't unlock a db unless it is locked */
			*lldberr = BTERR_UNLOCKED;
			goto force_open_db_exit;
		}
		kfile1.k_ostat = 0;
	} else if (alteration == 2) {
		/* lock db */
		if (kfile1.k_ostat == -2) {
			/* can't lock a db that is already locked */
			*lldberr = BTERR_LOCKED;
			goto force_open_db_exit;
		}
		if (kfile1.k_ostat != 0) {
			/* can't lock a db unless it is unused currently*/
			*lldberr = kfile1.k_ostat < 0 ? BTERR_WRITER : BTERR_READERS;
			goto force_open_db_exit;
		}
		kfile1.k_ostat = -2;
	} else if (alteration == 3) {
		/* force open db */
		if (kfile1.k_ostat == -2) {
			/* cannot force open a locked database */
			*lldberr = BTERR_LOCKED;
			goto force_open_db_exit;
		}
		kfile1.k_ostat = 0;
	} else {
		/* bad argument */
		FATAL();
	}
	rewind(fp);
	if (fwrite(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		*lldberr = BTERR_KFILE_ALTERDB;
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
 *  alteration:  [in] flag for forceopen (3), lock (2), & unlock (1)
 *  uses global readpath
 * Upon failure, sets lldberr and returns false
 *================================================*/
static BOOLEAN
open_database_impl (LLDATABASE lldb, INT alteration, INT *lldberr)
{
	int c;
	INT writ = !readonly + writeable;
	BTREE btree = 0;

	/* handle db adjustments (which only affect keyfile) first */
	if (alteration > 0 && !alterdb(alteration, lldberr)) return FALSE;

	/* call btree module to do actual open of BTR */
	if (!(btree = bt_openbtree(readpath, FALSE, writ, immutable, lldberr)))
		return FALSE;
	lldb_set_btree(lldb, btree);
	/* we have to set the global variable readonly correctly, because
	it is used widely */
	readonly = !bwrite(btree);
	immutable = bimmut(btree);
	if (readonly && writeable) {
		c = bkfile(btree).k_ostat;
		if (c < 0) {
			*lldberr = BTERR_WRITER;
		} else {
			rdr_count = c-1;
			*lldberr = BTERR_READERS;
		}
		return FALSE;
	}
	return TRUE;
}
/*==================================================
 * open_database -- open database
 *  forceopen:    [in] flag to override reader/writer protection
 *  dbpath:       [in] database path to open
 *================================================*/
BOOLEAN
open_database (INT alteration, STRING dbpath, INT *lldberr)
{
	LLDATABASE lldb = lldb_alloc();
	BOOLEAN rtn = FALSE;
	char fpath[MAXPATHLEN];

	/* tentatively copy paths into gedlib module versions */
	strupdate(&readpath_file, lastpathname(dbpath));
	llstrncpy(fpath, dbpath, sizeof(fpath), 0);
	expand_special_fname_chars(fpath, sizeof(fpath), uu8);
	strupdate(&readpath, fpath);

	if (f_dbnotify)
		(*f_dbnotify)(readpath, TRUE);

	rtn = open_database_impl(lldb, alteration, lldberr);
	if (!rtn) {
		/* open failed so clean up, preserve lldberr */
		int myerr = *lldberr;
		lldb_close(&lldb);
		*lldberr = myerr;
	}
	def_lldb = lldb;
	return rtn;
}
/*==================================================
 * create_database -- create (& open) brand new database
 *  dbpath:  [IN]  path of database about to create
 *================================================*/
BOOLEAN
create_database (STRING dbpath, INT *lldberr)
{
	LLDATABASE lldb = lldb_alloc();
	BTREE btree = 0;

	/* first test that newdb props are legal */
	STRING props = getlloptstr("NewDbProps", 0);
	if (props && props[0]) {
		TABLE dbopts = create_table_str();
		STRING msg=0;
		if (!init_valtab_from_string(props, dbopts, '=', &msg)) {
			*lldberr = BTERR_BADPROPS;
			destroy_table(dbopts);
			return FALSE;
		}
		destroy_table(dbopts);
	}

	/* tentatively copy paths into gedlib module versions */
	readpath_file=strsave(lastpathname(dbpath));
	readpath=strsave(dbpath);

	if (!(btree = bt_openbtree(dbpath, TRUE, 2, immutable, lldberr))) {
		/* open failed so clean up, preserve lldberr */
		int myerr = *lldberr;
		lldb_close(&lldb);
		*lldberr = myerr;
		return FALSE;
	}
	def_lldb = lldb;
	lldb_set_btree(lldb, btree);
	initxref();
	if (props)
		store_record("VUOPT", props, strlen(props));
	return TRUE;
}
/*===================================================
 * describe_dberror -- Describe database opening error
 * dberr:  [in] error whose description is sought
 * buffer: [out] buffer for description
 * buflen: [in]  size of buffer
 *=================================================*/
void
describe_dberror (INT dberr, STRING buffer, INT buflen)
{
	STRING b=buffer;
	INT n=buflen, u8=uu8;
	buffer[0]=0;
	if (dberr != BTERR_WRITER)
		llstrncpy(buffer, _("Database error: -- "), buflen, 0);

	switch (dberr) {
	case BTERR_NODB:
		llstrapps(b, n, u8, _("requested database does not exist."));
		break;
	case BTERR_DBBLOCKEDBYFILE:
		llstrapps(b, n, u8, _("db directory is file, not directory."));
		break;
	case BTERR_DBCREATEFAILED:
		llstrapps(b, n, u8, _("creation of new database failed."));
		break;
	case BTERR_DBACCESS:
		llstrapps(b, n, u8, _("error accessing database directory."));
		break;
	case BTERR_NOKEY:
		llstrapps(b, n, u8, _("no keyfile (directory does not appear to be a database)."));
		break;
	case BTERR_INDEX:
		llstrapps(b, n, u8,  _("could not open, read or write an index file."));
		break;
	case BTERR_KFILE:
		llstrapps(b, n, u8,  _("could not open, read or write the key file."));
		break;
	case BTERR_KFILE_ALTERDB:
		llstrapps(b, n, u8,  _("could not open, read or write the key file (to alter database)."));
		break;
	case BTERR_BLOCK:
		llstrapps(b, n, u8,  _("could not open, read or write a block file."));
		break;
	case BTERR_LNGDIR:
		llstrapps(b, n, u8,  _("name of database is too long."));
		break;
	case BTERR_WRITER:
		llstrapps(b, n, u8,  _("The database is already open for writing."));
		break;
	case BTERR_LOCKED:
		llstrapps(b, n, u8,  _("The database is locked (no readwrite access)."));
		break;
	case BTERR_UNLOCKED:
		llstrapps(b, n, u8,  _("The database is unlocked."));
		break;
	case BTERR_ILLEGKF:
		llstrapps(b, n, u8,  _("keyfile is corrupt."));
		break;
	case BTERR_ALIGNKF:
		llstrapps(b, n, u8,  _("keyfile is wrong alignment."));
		break;
	case BTERR_VERKF:
		llstrapps(b, n, u8,  _("keyfile is wrong version."));
		break;
	case BTERR_EXISTS:
		llstrapps(b, n, u8,  _("Existing database found."));
		break;
	case BTERR_READERS:
		llstrappf(b, n, u8
			, _pl("The database is already opened for read access by %d user."
				, "The database is already opened for read access by %d users."
				, rdr_count)
			, rdr_count);
		break;
	case BTERR_BADPROPS:
		llstrapps(b, n, u8,  _("Invalid properties set for new database"));
		break;
	default:
		llstrapps(b, n, u8,  _("Undefined database error -- fix program."));
		break;
	}
}
/*==================================================
 * dbnotify_close -- Send notification that default database closed
 *================================================*/
void
dbnotify_set(void (*notify)(STRING db, BOOLEAN opening))
{
	f_dbnotify = notify;
}
/*==================================================
 * dbnotify_close -- Send notification that default database closed
 *================================================*/
void
dbnotify_close (void)
{

	if (f_dbnotify)
		(*f_dbnotify)(readpath, FALSE);
}
