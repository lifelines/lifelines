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

TABLE tagtable=NULL;		/* table for tag strings */
TABLE placabbvs=NULL;	/* table for place abbrevs */
TABLE useropts=NULL;		/* table for user options */
BTREE BTR=NULL;	/* database */
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */
INT int_codeset=0; /* internal codeset */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN writeable;
extern STRING btreepath,readpath;
extern STRING qSdbrecstats;


/*********************************************
 * local function prototypes
 *********************************************/

static void add_dbs_to_list(LIST dblist, STRING dir);
static void free_dblist_el(VPTR w);
static STRING getdbdesc(STRING path);
static INT select_lldbs(const struct dirent *entry);

/*********************************************
 * local variables
 *********************************************/

static int rdr_count = 0;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================
 * init_lifelines_global -- Initialization options & misc. stuff
 *  This is before database opened
 *===============================*/
BOOLEAN
init_lifelines_global (STRING * pmsg)
{
	STRING e;
	STRING dirvars[] = { "LLPROGRAMS", "LLREPORTS", "LLARCHIVES"
		, "LLDATABASES", "LLNEWDBDIR" };
	INT i;
	STRING configfile = getenv("LLCONFIGFILE");

	*pmsg = NULL;

	if (!configfile || !configfile[0])
		configfile = environ_determine_config_file();
	
	if (!init_lifelines_options(configfile, pmsg))
		return FALSE;
#if ENABLE_NLS
	e = getoptstr("GuiOutputCharset", "");
	if (e && *e)
		bind_textdomain_codeset(PACKAGE, e);
#endif

	/* check if any directories not specified, and try environment
	variables, and default to "." */
	for (i=0; i<ARRSIZE(dirvars); ++i) {
		if (!getoptstr(dirvars[i], NULL)) {
			STRING str = getenv(dirvars[i]);
			if (str)
				changeoptstr(dirvars[i], strsave(str));
			else
				changeoptstr(dirvars[i], strsave("."));
		}
	}
	/* also check environment variable for editor */
	if (!getoptstr("LLEDITOR", NULL)) {
		STRING str = getenv("LLEDITOR");
		if (str)
			changeoptstr("LLEDITOR", strsave(str));
	}
	/* editor falls back to platform-specific default */
	e = getoptstr("LLEDITOR", NULL);
	if (!e || !e[0])
		e = environ_determine_editor(PROGRAM_LIFELINES);
	/* configure tempfile & edit command */
	editfile = environ_determine_tempfile();
	if (!editfile) {
		*pmsg = strsave("Error creating temp file");
		return FALSE;
	}
	editfile = strsave(editfile );
	editstr = (STRING) stdalloc(strlen(e) + strlen(editfile) + 2);
	sprintf(editstr, "%s %s", e, editfile);
	set_usersort(custom_sort);
	return TRUE;
}

/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
BOOLEAN
init_lifelines_db (void)
{
	STRING emsg;

	tagtable = create_table();
	placabbvs = create_table();
	useropts = create_table();
	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", useropts, '=', &emsg);
	update_useropts();
	init_caches();
	init_browse_lists();
	init_mapping();
	if (!openxref(readonly))
		return FALSE;
	return TRUE;
}
/*===============================================
 * get_lifelines_version -- Return version string
 *  returns static buffer
 *=============================================*/
STRING
get_lifelines_version (INT maxlen)
{
	static char version[48];
	INT len=sizeof(version);
	if (len>maxlen)
		len=maxlen;
	snprintf(version, len, LIFELINES_VERSION);
	return version;
}
/*===================================
 * close_lifelines -- Close LifeLines
 *  Close entire lifelines engine - not just
 *  database (see close_lldb below).
 *=================================*/
void
close_lifelines (void)
{
	close_lldb(); /* make sure database closed */
	if (editfile) {
		unlink(editfile);
		stdfree(editfile);
		editfile=NULL;
	}
	if (editstr) {
		stdfree(editstr);
		editstr=NULL;
	}
	term_lloptions();
}
/*===================================
 * close_lldb -- Close current database
 *  Safe to call even if not opened
 *=================================*/
void
close_lldb (void)
{
	/* TODO: reverse the rest of init_lifelines_db -- Perry, 2002.06.05 */
	closexref();
	if (BTR) {
		closebtree(BTR);
		BTR=NULL;
	}
}
/*==================================================
 * alterdb -- force open, lock, or unlock a database
 *  alteration: [in] 1=unlock, 2=lock, 3=force open
 *  returns FALSE & sets bterrno if error (eg, keyfile corrupt)
 *================================================*/
static BOOLEAN
alterdb (INT alteration)
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
	if (alteration == 1) {
		/* unlock db */
		if (kfile1.k_ostat != -2) {
			/* can't unlock a db unless it is locked */
			bterrno = BTERR_UNLOCKED;
			goto force_open_db_exit;
		}
		kfile1.k_ostat = 0;
	} else if (alteration == 2) {
		/* lock db */
		if (kfile1.k_ostat == -2) {
			/* can't lock a db that is already locked */
			bterrno = BTERR_LOCKED;
			goto force_open_db_exit;
		}
		if (kfile1.k_ostat != 0) {
			/* can't lock a db unless it is unused currently*/
			bterrno = kfile1.k_ostat < 0 ? BTERR_WRITER : BTERR_READERS;
			goto force_open_db_exit;
		}
		kfile1.k_ostat = -2;
	} else if (alteration == 3) {
		/* force open db */
		if (kfile1.k_ostat == -2) {
			/* cannot force open a locked database */
			bterrno = BTERR_LOCKED;
			goto force_open_db_exit;
		}
		kfile1.k_ostat = 0;
	} else {
		/* bad argument */
		FATAL();
	}
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
 *  alteration:  [in] flag for forceopen (3), lock (2), & unlock (1)
 *  uses globals btreepath & readpath
 *  btreepath: database to report
 *  readpath: actual database path (may be relative also)
 * Upon failure, sets bterrno and returns false
 *================================================*/
static BOOLEAN
open_database_impl (INT alteration)
{
	int c;
	INT writ = !readonly + writeable;

	/* handle db adjustments (which only affect keyfile) first */
	if (alteration > 0 && !alterdb(alteration)) return FALSE;

	/* call btree module to do actual open of BTR */
	if (!(BTR = openbtree(readpath, FALSE, writ, immutable)))
		return FALSE;
	/* we have to set the global variable readonly correctly, because
	it is used widely */
	readonly = !bwrite(BTR);
	immutable = bimmut(BTR);
	if (readonly && writeable) {
		int myerr=0;
		c = bkfile(BTR).k_ostat;
		if (c < 0) {
			myerr = BTERR_WRITER;
		} else {
			rdr_count = c-1;
			myerr = BTERR_READERS;
		}
		/* close_lldb may have set bterrno itself */
		close_lldb();
		bterrno = myerr;
		return FALSE;
	}
	return TRUE;
}
/*==================================================
 * open_database -- open database
 *  forceopen:    [in] flag to override reader/writer protection
 *  dbrequested:  [in] database to report
 *  dbused:       [in] actual database path (may be relative also)
 *================================================*/
BOOLEAN
open_database (BOOLEAN alteration, STRING dbrequested, STRING dbused)
{
	BOOLEAN rtn;

	/* tentatively copy paths into gedlib module versions */
	btreepath=strsave(dbrequested);
	readpath=strsave(dbused);

	rtn = open_database_impl(alteration);
	if (!rtn) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		close_lldb();
		bterrno = myerr;
		strfree(&btreepath);
		strfree(&readpath);
	}
	return rtn;
}
/*==================================================
 * create_database -- create (& open) brand new database
 *  newpath:  [in] path of database about to create
 *================================================*/
BOOLEAN
create_database (STRING dbrequested, STRING dbused)
{
	/* tentatively copy paths into gedlib module versions */
	btreepath=strsave(dbrequested);
	readpath=strsave(dbused);

	if (!(BTR = openbtree(dbused, TRUE, 2, immutable))) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		close_lldb();
		bterrno = myerr;
		strfree(&btreepath);
		strfree(&readpath);
		return FALSE;
	}
	initxref();
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
	case BTERR_LOCKED:
		msg = "The database is locked (no readwrite access).";
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
		break;
	default:
		msg = "Undefined database error -- This can't happen.";
		break;
	}
	llstrcatn(&ptr, msg, &mylen);
}
/*===================================================
 * update_useropts -- Set any global variables
 * dependent on user options
 *=================================================*/
void
update_useropts (void)
{
	STRING str;
	int_codeset=0;
	if ((str = valueof_str(useropts, "codeset"))!=NULL) {
		if (eqstr("1", str))
			int_codeset=1;
		else if (eqstr("UTF-8", str)||eqstr("utf-8", str)||eqstr("65001", str))
			int_codeset=8;
	}
	uilocale(); /* in case user changed locale */
}
/*==================================================
 * get_dblist -- find all dbs on path
 *  path:  [IN]  list of directories to be searched
 *  num:   [OUT] # dbs found (length of returned list)
 * TODO: put #elements into LIST objects, & we don't have
 * to pass this silly count around - Perry, 2002.06.05
 *================================================*/
LIST
get_dblist (STRING path, INT * num)
{
	char dirs[MAXPATHLEN+1];
	INT ndirs=0;
	STRING p=0;
	LIST dblist = create_list();
	if (!path || !path[0] || strlen(path) > sizeof(dirs)-2)
		return dblist;
	/* find directories in dirs & delimit with zeros */
	ndirs = chop_path(path, dirs);
	for (p=dirs; ndirs>0; --ndirs) {
		ASSERT(p);
		add_dbs_to_list(dblist, p);
		p += strlen(p)+1;
	}
	FORLIST(dblist, el)
		++(*num);
	ENDLIST
	return dblist;
}
/*==================================================
 * add_dbs_to_list -- Add all dbs in specified dir to list
 *  dblist: [I/O] list of databases found
 *  dir:    [IN]  directory to be searched for more databases
 *================================================*/
static void
add_dbs_to_list (LIST dblist, STRING dir)
{
	int n=0;
	struct dirent **programs=0;
	char candidate[MAXPATHLEN];
	STRING dbstr=0;

	n = scandir(dir, &programs, 0, 0);
	if (n < 0) return;
	while (n--) {
		strcpy(candidate, concat_path(dir, programs[n]->d_name));
		if ((dbstr = getdbdesc(candidate)) != NULL)
			push_list(dblist, dbstr);
		stdfree(programs[n]);
	}
	stdfree(programs);
}
/*==================================================
 * select_lldbs -- Check a file or directory to see if it
 *  is a lifelines database
 *  returns stdalloc'd string or NULL (if not db)
 *================================================*/
static STRING
getdbdesc (STRING path)
{
	BTREE btr;
	BOOLEAN cflag=FALSE, writ=FALSE, immut=TRUE;
	char desc[MAXPATHLEN];

	strcpy(desc, "");
	btr = openbtree(path, cflag, writ, immut);
	if (btr) {
		/* TODO: 'twould be nice to clean up & remove these globals */
		BTR = btr; /* various code assumes BTR is the btree */
		readonly = TRUE; /* openxrefs depends on this global */
		if (init_lifelines_db()) {
			strcpy(desc, path);
			if (strlen(path) < sizeof(desc) - 50) {
				char stats[45];
				snprintf(stats, sizeof(stats), _(qSdbrecstats), num_indis()
					, num_fams(), num_sours(), num_evens(), num_othrs());
				strcat(desc, stats);
			}
		}
	}
	close_lldb();
	BTR = 0;
	return desc[0] ? strdup(desc) : 0;
}
/*====================================================
 * free_string_el -- free alloc'd string element of list
 *==================================================*/
static void
free_dblist_el (VPTR w)
{
	stdfree((STRING)w);
}
/*====================================================
 * release_dblist -- free a dblist (caller is done with it)
 *==================================================*/
void
release_dblist (LIST dblist)
{
	if (dblist) {
		remove_list(dblist, free_dblist_el);
	}
}
