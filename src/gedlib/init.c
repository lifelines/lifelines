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
#include "codesets.h"
#include "menuitem.h"
#include "zstr.h"
#include "icvt.h"
#include "date.h"

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable=NULL;		/* table for tag strings */
TABLE placabbvs=NULL;	/* table for place abbrevs */
BTREE BTR=NULL;	/* database */
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN writeable;
extern STRING readpath,readpath_file;
extern STRING qSdbrecstats;
extern STRING illegal_char;


/*********************************************
 * local function prototypes
 *********************************************/

static void add_dbs_to_list(LIST dblist, LIST dbdesclist, STRING dir);
static STRING getdbdesc(STRING path, STRING userpath);
static void init_win32_gettext_shim(void);
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
		, "LLDATABASES", };
	INT i;

	/* request notification when options change */
	register_notify(&update_useropts);
	suppress_reload = TRUE;

	f_dbnotify = notify;

	if (!configfile)
		configfile = getenv("LLCONFIGFILE");

	*pmsg = NULL;


	if (!configfile || !configfile[0]) {
		STRING cfg_file;
		char cfg_name[MAXPATHLEN];

		cfg_file = environ_determine_config_file();
		/* first try $HOME/config_file */
		llstrncpy(cfg_name,getenv("HOME") , sizeof(cfg_name), 0);
		llstrappc(cfg_name, sizeof(cfg_name), '/');
		llstrapps(cfg_name, sizeof(cfg_name), 0, cfg_file);
		if (!load_global_options(cfg_name, pmsg)) {
			suppress_reload = FALSE;
			update_useropts(NULL);
			return FALSE;
		}
		configfile = cfg_file;
		/* fall through to open config file */
	}
	if (!load_global_options(configfile, pmsg)) {
		suppress_reload = FALSE;
		update_useropts(NULL);
		return FALSE;
	}

#ifdef WIN32
	/* On MS-Windows, attempt to set any requested non-standard codepage */
	/* Do this now, before init_codesets below */
	i = getoptint("ConsoleCodepage", 0);
	if (i) {
		w_set_oemout_codepage(i);
		w_set_oemin_codepage(i);
	}
#endif

	/* now that codeset variables are set from config file, lets initialize codesets */
	/* although int_codeset can't be determined yet, we need GUI codeset for gettext */
	init_codesets();

	/* for Windows, link dynamically to gettext & iconv if available */
	init_win32_gettext_shim();
	init_win32_iconv_shim(getoptstr("iconv.path",""));

#if ENABLE_NLS

	/* until we have an internal codeset (which is until we open a database)
	we want output in display codeset */
	set_gettext_codeset(PACKAGE, gui_codeset_out);

	/* allow run-time specification of locale directory */
	/* (LOCALEDIR is compile-time) */
	e = getoptstr("LocaleDir", "");
	if (e && *e) {
		bindtextdomain(PACKAGE, e);
		locales_notify_language_change(); /* TODO: is this necessary ? 2002-09-29, Perry */
	}

#endif /* ENABLE_NLS */

	/* read available translation tables */
	transl_load_all_tts();
	/* set up translations (for first time, will do it again after 
	loading config table, and then again after loading database */
	transl_load_xlats();

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
 * set_gettext_codeset -- Tell gettext what codeset we want
 * Created: 2002/11/28 (Perry Rapp)
 *===============================*/
#if ENABLE_NLS
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
void
set_gettext_codeset (CNSTRING domain, CNSTRING codeset)
{
	static STRING prevcodeset = 0;
	if (eqstr_ex(prevcodeset, codeset))
		return;
	if (codeset && codeset[0]) {
		ZSTR zcsname=zs_new();
		/* extract just the codeset name, without any subcodings */
		/* eg, just "UTF-8" out of "UTF-8//TrGreekAscii//TrCyrillicAscii" */
		transl_parse_codeset(codeset, zcsname, 0);
		if (zs_str(zcsname)) {
			strupdate(&prevcodeset, zs_str(zcsname));
			/* gettext automatically appends //TRANSLIT */
		} else {
			/* what do we do if they gave us an empty one ? */
			strupdate(&prevcodeset, "ASCII");
		}
		zs_free(&zcsname);
	} else {
		/* 
		We need to set some codeset, in case it was set to 
		UTF-8 in last db 
		*/
		strupdate(&prevcodeset, gui_codeset_out);
	}
	bind_textdomain_codeset(domain, prevcodeset);
	if (eqstr(domain, PACKAGE))
		locales_notify_uicodeset_changes();
}
#endif /* HAVE_BIND_TEXTDOMAIN_CODESET */
#endif /* ENABLE_NLS */
/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
BOOLEAN
init_lifelines_db (void)
{
	STRING emsg;
	TABLE dbopts = create_table(FREEBOTH);

	tagtable = create_table(FREEKEY); /* values are same as keys */
	placabbvs = create_table(FREEBOTH);

	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", dbopts, '=', &emsg);
	set_db_options(dbopts);
	free_optable(&dbopts);
	init_caches();
	init_browse_lists();
	if (!openxref(readonly))
		return FALSE;


	transl_load_xlats();

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
	term_date();
	term_codesets();
	strfree(&int_codeset);
}
/*===================================
 * close_lldb -- Close current database
 *  Safe to call even if not opened
 * fullclose is FALSE when building dblist
 *=================================*/
void
close_lldb ()
{
	if (tagtable)
		destroy_table(tagtable);
	tagtable = 0;
	/* TODO: reverse the rest of init_lifelines_db -- Perry, 2002.06.05 */
	if (placabbvs) {
		destroy_table(placabbvs);
		placabbvs = NULL;
	}
	free_caches();
	closexref();
	if (BTR) {
		closebtree(BTR);
		BTR=NULL;
	}
	if (f_dbnotify)
		(*f_dbnotify)(readpath, FALSE);
	transl_free_predefined_xlats(); /* clear any active legacy translation tables */
	strfree(&readpath_file);
	strfree(&readpath);
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
		bterrno = BTERR_KFILE_ALTERDB;
		goto force_open_db_exit;
	}
	if (!(fp = fopen(scratch, LLREADBINARYUPDATE)) ||
		  fread(&kfile1, sizeof(kfile1), 1, fp) != 1) {
		bterrno = BTERR_KFILE_ALTERDB;
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
		bterrno = BTERR_KFILE_ALTERDB;
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
 *  dbpath:       [in] database path to open
 *================================================*/
BOOLEAN
open_database (INT alteration, STRING dbpath)
{
	BOOLEAN rtn;
	char fpath[MAXPATHLEN];

	/* tentatively copy paths into gedlib module versions */
	strupdate(&readpath_file, lastpathname(dbpath));
	llstrncpy(fpath, dbpath, sizeof(fpath), 0);
	expand_special_fname_chars(fpath, sizeof(fpath), uu8);
	strupdate(&readpath, fpath);

	if (f_dbnotify)
		(*f_dbnotify)(readpath, TRUE);

	rtn = open_database_impl(alteration);
	if (!rtn) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		close_lldb();
		bterrno = myerr;
	}
	return rtn;
}
/*==================================================
 * create_database -- create (& open) brand new database
 *  dbpath:  [IN]  path of database about to create
 *================================================*/
BOOLEAN
create_database (STRING dbpath)
{
	/* first test that newdb props are legal */
	STRING props = getoptstr("NewDbProps", 0);
	if (props && props[0]) {
		TABLE dbopts = create_table(FREEBOTH);
		STRING msg=0;
		if (!init_valtab_from_string(props, dbopts, '=', &msg)) {
			bterrno = BTERR_BADPROPS;
			destroy_table(dbopts);
			return FALSE;
		}
		remove_table(dbopts, FREEBOTH);
	}

	/* tentatively copy paths into gedlib module versions */
	readpath_file=strsave(lastpathname(dbpath));
	readpath=strsave(dbpath);

	if (!(BTR = openbtree(dbpath, TRUE, 2, immutable))) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		close_lldb();
		bterrno = myerr;
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
			, _("The database is already opened for read access by %d users.\n  ")
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
	/* deal with db-specific options */
	/* includes setting int_codeset */
	if (BTR)
		update_db_options();
	/* in case user changed any codesets */
	init_codesets();
	/* in case user changed locale (need int_codeset already set) */
	uilocale();
	/* in case user changed codesets */
	transl_load_xlats();

	strupdate(&illegal_char, getoptstr("IllegalChar", 0));
}
/*==================================================
 * update_db_options -- 
 *  check database-specific options for updates
 *================================================*/
static void
update_db_options (void)
{
	TABLE opttab = create_table(FREEBOTH);
	STRING str=0;
	get_db_options(opttab);

	str = valueof_str(opttab, "codeset");
	if (!str) str="";
	if (!int_codeset)
		strupdate(&int_codeset, "");
	if (!eqstr_ex(int_codeset, str)) {
		strupdate(&int_codeset, str);
		uu8 = is_codeset_utf8(int_codeset);
		/* always translate to internal codeset */
#ifdef ENABLE_NLS
		set_gettext_codeset(PACKAGE, int_codeset);
#endif /* ENABLE_NLS */
		/* need to reload all predefined codeset conversions */
		transl_load_xlats();
	}

	destroy_table(opttab);
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
	if (!expand_special_fname_chars(dirbuf, sizeof(dirbuf), uu8))
		return;

	n = scandir(dirbuf, &programs, 0, 0);
	if (n < 0) return;
	while (n--) {
		concat_path(dirbuf, programs[n]->d_name, uu8, candidate, sizeof(candidate));
		concat_path(dir, programs[n]->d_name, uu8, userpath, sizeof(userpath));
		if ((dbstr = getdbdesc(candidate, userpath)) != NULL) {
			back_list(dblist, strsave(userpath));
			back_list(dbdesclist, dbstr);
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
			llstrapps(desc, sizeof(desc), uu8, userpath);
			llstrapps(desc, sizeof(desc), uu8, " <");
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
 * is_db_open -- 
 *==================================================*/
BOOLEAN
is_db_open (void)
{
	return BTR!=0;
}
