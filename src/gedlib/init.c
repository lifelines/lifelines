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
#ifdef WIN32_ICONV_SHIM
#include "iconvshim.h"
#endif

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable=NULL;		/* table for tag strings */
TABLE placabbvs=NULL;	/* table for place abbrevs */
BTREE BTR=NULL;	/* database */
static char svconfigfile[MAXPATHLEN]="";
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN writeable;
extern STRING btreepath,readpath;
extern STRING qSdbrecstats;
extern STRING illegal_char;


/*********************************************
 * local function prototypes
 *********************************************/

static void add_dbs_to_list(LIST dblist, LIST dbdesclist, STRING dir);
static STRING getdbdesc(STRING path, STRING userpath);
static void init_win32_gettext_shim(void);
static void init_win32_iconv_shim(void);
static void update_db_options(void);

/*********************************************
 * local variables
 *********************************************/

static int rdr_count = 0;
static void (*f_dbnotify)(STRING db, BOOLEAN opening) = 0;
static BOOLEAN suppress_reload=FALSE;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================
 * init_lifelines_global -- Initialization options & misc. stuff
 *  This is called before first (or later) database opened
 *===============================*/
BOOLEAN
init_lifelines_global (STRING configfile, STRING * pmsg, void (*notify)(STRING db, BOOLEAN opening))
{
	STRING e;
	STRING dirvars[] = { "LLPROGRAMS", "LLREPORTS", "LLARCHIVES"
		, "LLDATABASES", "LLNEWDBDIR" };
	INT i;

	/* request notification when options change */
	register_notify(&update_useropts);
	suppress_reload = TRUE;

	f_dbnotify = notify;

	if (!configfile)
		configfile = getenv("LLCONFIGFILE");

	*pmsg = NULL;

	if (!configfile || !configfile[0])
		configfile = environ_determine_config_file();

	llstrncpy(svconfigfile, configfile, sizeof(svconfigfile), uu8);

	if (!load_global_options(svconfigfile, pmsg)) {
		suppress_reload = FALSE;
		update_useropts(NULL);
		return FALSE;
	}

	init_win32_gettext_shim();
	init_win32_iconv_shim();

#if ENABLE_NLS

#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	e = getoptstr("GuiCodeset", "");
	if (e && e[0]) {
		/* This doesn't work with gettext before 0.11.3pre2
		STRING suffix = getoptstr("GuiCodesetOutput", "");
		STRING f = strconcat(int_codeset, suffix);
		*/
		STRING f=e;
		bind_textdomain_codeset(PACKAGE, f);
	}
	/*
	otherwise let gettext try to figure it out
	gettext has a lot of nice code for this stuff, but it doesn't
	know about DOS console windows, and will default to the MS-Windows default
	codepage, which is not very close :(
	TODO: So revise this to use the codeset we determine at startup
	*/
#endif /* HAVE_BIND_TEXTDOMAIN_CODESET */

	/* allow run-time specification of locale directory */
	/* (LOCALEDIR is compile-time) */
	e = getoptstr("LocaleDir", "");
	if (e && *e) {
		bindtextdomain(PACKAGE, e);
		language_change(); /* TODO: is this necessary ? 2002-09-29, Perry */
	}

#endif /* ENABLE_NLS */


	/* check if any directories not specified, and try environment
	variables, and default to "." */
	for (i=0; i<ARRSIZE(dirvars); ++i) {
		STRING str = getenv(dirvars[i]);
		if (!str)
			str = ".";
		setoptstr_fallback(dirvars[i], strsave(str));
	}
	/* also check environment variable for editor */
	{
		STRING str = getenv("LLEDITOR");
		if (!str)
			str = environ_determine_editor(PROGRAM_LIFELINES);
		setoptstr_fallback("LLEDITOR", strsave(str));
	}
	/* editor falls back to platform-specific default */
	e = getoptstr("LLEDITOR", NULL);
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
	suppress_reload = FALSE;
	update_useropts(0);
	return TRUE;
}
/*=================================
 * init_win32_iconv_shim -- 
 *  Handle user-specified gettext dll path
 *===============================*/
static void
init_win32_iconv_shim (void)
{
#ifdef WIN32_ICONV_SHIM
	STRING e;
	/* (re)load iconv.dll if path specified */
	e = getoptstr("iconv.path", "");
	if (e && *e)
		iconvshim_set_property("dll_path", e);
#endif
}
/*=================================
 * init_win32_gettext_shim -- 
 *  Handle user-specified iconv dll path
 *===============================*/
static void
init_win32_gettext_shim (void)
{
#if ENABLE_NLS
#ifdef WIN32_INTL_SHIM
	STRING e;
	/* (re)load gettext dll if specified */
	e = getoptstr("gettext.path", "");
	if (e && *e)
	{
		if (intlshim_set_property("dll_path", e))
		{
			bindtextdomain(PACKAGE, LOCALEDIR);
			textdomain(PACKAGE);
		}
		/* tell gettext where to find iconv */
		e = getoptstr("iconv.path", "");
		if (e && *e)
			gt_set_property("iconv_path", e);
	}
	/*
	We could be more clever, and if our iconv_path is no good, ask gettext
	if it found iconv, but that would make this logic tortuous due to our having
	different shim macros (we'd have to save gettext's iconv path before setting it,
	in case ours is bad & its is good).
	*/
#endif
#endif
}
/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
BOOLEAN
init_lifelines_db (void)
{
	STRING emsg;
	TABLE dbopts = create_table();

	tagtable = create_table();
	placabbvs = create_table();

	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", dbopts, '=', &emsg);
	set_db_options(dbopts);
	init_caches();
	init_browse_lists();
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
	llstrncpyf(version, len, 0, LIFELINES_VERSION);
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
	/* TODO: reverse the rest of init_lifelines_db -- Perry, 2002.06.05
	remove_table(tagtable, FREE_KEY); values are same as keys
	remove_table(placabbvs, ??)
	*/
	closexref();
	if (BTR) {
		closebtree(BTR);
		BTR=NULL;
	}
	if (f_dbnotify)
		(*f_dbnotify)(readpath, FALSE);
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
	char fpath[MAXPATHLEN];

	/* tentatively copy paths into gedlib module versions */
	btreepath=strsave(dbrequested);
	llstrncpy(fpath, dbused, sizeof(fpath), 0);
	expand_special_fname_chars(fpath, sizeof(fpath));
	readpath=strsave(fpath);

	if (f_dbnotify)
		(*f_dbnotify)(readpath, TRUE);

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
	/* first test that newdb props are legal */
	STRING props = getoptstr("NewDbProps", 0);
	if (props && props[0]) {
		TABLE dbopts = create_table();
		STRING msg=0;
		if (!init_valtab_from_string(props, dbopts, '=', &msg)) {
			bterrno = BTERR_BADPROPS;
			remove_table(dbopts, FREEBOTH);
			return FALSE;
		}
		remove_table(dbopts, FREEBOTH);
	}

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
		llstrapp(b, n, u8, _("requested database does not exist."));
		break;
	case BTERR_DBBLOCKEDBYFILE:
		llstrapp(b, n, u8, _("db directory is file, not directory."));
		break;
	case BTERR_DBCREATEFAILED:
		llstrapp(b, n, u8, _("creation of new database failed."));
		break;
	case BTERR_DBACCESS:
		llstrapp(b, n, u8, _("error accessing database directory."));
		break;
	case BTERR_NOKEY:
		llstrapp(b, n, u8, _("no keyfile (directory does not appear to be a database)."));
		break;
	case BTERR_INDEX:
		llstrapp(b, n, u8,  _("could not open, read or write an index file."));
		break;
	case BTERR_KFILE:
		llstrapp(b, n, u8,  _("could not open, read or write the key file."));
		break;
	case BTERR_BLOCK:
		llstrapp(b, n, u8,  _("could not open, read or write a block file."));
		break;
	case BTERR_LNGDIR:
		llstrapp(b, n, u8,  _("name of database is too long."));
		break;
	case BTERR_WRITER:
		llstrapp(b, n, u8,  _("The database is already open for writing."));
		break;
	case BTERR_LOCKED:
		llstrapp(b, n, u8,  _("The database is locked (no readwrite access)."));
		break;
	case BTERR_ILLEGKF:
		llstrapp(b, n, u8,  _("keyfile is corrupt."));
		break;
	case BTERR_ALIGNKF:
		llstrapp(b, n, u8,  _("keyfile is wrong alignment."));
		break;
	case BTERR_VERKF:
		llstrapp(b, n, u8,  _("keyfile is wrong version."));
		break;
	case BTERR_EXISTS:
		llstrapp(b, n, u8,  _("Existing database found."));
		break;
	case BTERR_READERS:
		llstrappf(b, n, u8
			, _("The database is already opened for read access by %d users.\n  ")
			, rdr_count);
		break;
	case BTERR_BADPROPS:
		llstrapp(b, n, u8,  _("Invalid properties set for new database"));
		break;
	default:
		llstrapp(b, n, u8,  _("Undefined database error -- fix program."));
		break;
	}
}
/*===================================================
 * is_codeset_utf8 -- Is this the name of UTF-8 ?
 *=================================================*/
BOOLEAN
is_codeset_utf8 (STRING codename)
{
	if (!codename || !codename[0]) return FALSE;
	if (eqstr("UTF-8", codename)||eqstr("utf-8", codename)||eqstr("65001", codename))
		return TRUE;
	return FALSE;
}
/*===================================================
 * update_useropts -- Set any global variables
 * dependent on user options
 *=================================================*/
void
update_useropts (VPTR uparm)
{
	uparm = uparm; /* unused */
	if (suppress_reload)
		return;
	update_db_options(); /* deal with db-specific options */
	uilocale(); /* in case user changed locale */
	/* if we were clever, we'd remember the codesets, so we'd know if we need
	to reload mappings -- Perry, 2002.02.18 */
	load_char_mappings(); /* in case user changed codesets */
	strupdate(&illegal_char, getoptstr("IllegalChar", 0));
}
/*==================================================
 * update_db_options -- 
 *  check database-specific options for updates
 *================================================*/
static void
update_db_options (void)
{
	TABLE opttab = create_table();
	STRING str=0;
	get_db_options(opttab);

	strfree(&int_codeset);
	if ((str = valueof_str(opttab, "codeset")) != 0) {
		if (!eqstr_ex(int_codeset, str)) {
			int_codeset = strsave(str);
			uu8 = is_codeset_utf8(int_codeset);
		}
	}
	
	remove_table(opttab, FREEBOTH);
}
/*==================================================
 * get_dblist -- find all dbs on path
 *  path:       [IN]  list of directories to be searched
 *  dblist:     [OUT] list of database paths found
 *  dbdesclist: [OUT] list of descriptions of databases found
 *================================================*/
INT
get_dblist (STRING path, LIST * dblist, LIST * dbdesclist)
{
	STRING dirs=0;
	INT ndirs=0;
	STRING p=0;
	ASSERT(!(*dblist) && !(*dbdesclist));
	*dblist = create_list();
	*dbdesclist = create_list();
	set_list_type(*dblist, LISTDOFREE);
	set_list_type(*dbdesclist, LISTDOFREE);
	if (!path || !path[0])
		return 0;
	dirs = (STRING)stdalloc(strlen(path)+2);
	/* find directories in dirs & delimit with zeros */
	ndirs = chop_path(path, dirs);
	/* now process each directory */
	for (p=dirs; *p; p+=strlen(p)+1) {
		add_dbs_to_list(*dblist, *dbdesclist, p);
	}
	strfree(&dirs);
	return llen(*dblist);
}
/*==================================================
 * add_dbs_to_list -- Add all dbs in specified dir to list
 *  dblist: [I/O] list of databases found
 *  dir:    [IN]  directory to be searched for more databases
 *================================================*/
static void
add_dbs_to_list (LIST dblist, LIST dbdesclist, STRING dir)
{
	int n=0;
	struct dirent **programs=0;
	char candidate[MAXPATHLEN];
	char userpath[MAXPATHLEN];
	STRING dbstr=0;
	char dirbuf[MAXPATHLEN];

	llstrncpy(dirbuf, dir, sizeof(dirbuf), 0);
	if (!expand_special_fname_chars(dirbuf, sizeof(dirbuf)))
		return;

	n = scandir(dirbuf, &programs, 0, 0);
	if (n < 0) return;
	while (n--) {
		concat_path(dirbuf, programs[n]->d_name, candidate, sizeof(candidate));
		concat_path(dir, programs[n]->d_name, userpath, sizeof(userpath));
		if ((dbstr = getdbdesc(candidate, userpath)) != NULL) {
			push_list(dblist, strsave(userpath));
			push_list(dbdesclist, dbstr);
		}
		stdfree(programs[n]);
	}
	stdfree(programs);
}
/*==================================================
 * getdbdesc -- Check a file or directory to see if it
 *  is a lifelines database
 *  returns stdalloc'd string or NULL (if not db)
 *================================================*/
static STRING
getdbdesc (STRING path, STRING userpath)
{
	BTREE btr;
	BOOLEAN cflag=FALSE, writ=FALSE, immut=TRUE;
	char desc[MAXPATHLEN];

	if (f_dbnotify)
		(*f_dbnotify)(path, TRUE);

	suppress_reload = TRUE; /* do not reload options */

	strcpy(desc, "");
	btr = openbtree(path, cflag, writ, immut);
	if (btr) {
		/* TODO: 'twould be nice to clean up & remove these globals */
		BTR = btr; /* various code assumes BTR is the btree */
		readonly = TRUE; /* openxrefs depends on this global */
		if (init_lifelines_db()) {
			desc[0]=0;
			llstrapp(desc, sizeof(desc), uu8, userpath);
			llstrapp(desc, sizeof(desc), uu8, " <");
			llstrappf(desc, sizeof(desc), uu8, _(qSdbrecstats)
				, num_indis(), num_fams(), num_sours(), num_evens(), num_othrs());
			llstrappf(desc, sizeof(desc), uu8, ">");
		}
	}
	close_lldb();
	readonly = FALSE;
	BTR = 0;
	suppress_reload = FALSE;
	return desc[0] ? strsave(desc) : 0;
}
/*====================================================
 * release_dblist -- free a dblist (caller is done with it)
 *==================================================*/
void
release_dblist (LIST dblist)
{
	if (dblist) {
		make_list_empty(dblist);
		remove_list(dblist, 0);
	}
}
/*====================================================
 * init_get_config_file -- return pointer to configfile
 *==================================================*/
CNSTRING
init_get_config_file (void)
{
	return svconfigfile;
}
/*====================================================
 * is_db_open -- 
 *==================================================*/
BOOLEAN
is_db_open (void)
{
	return BTR!=0;
}
