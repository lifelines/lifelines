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

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable=NULL;		/* table for tag strings */
TABLE placabbvs=NULL;	/* table for place abbrevs */
STRING editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
STRING editfile=NULL; /* file used for editing, name obtained via mktemp */

/*********************************************
 * external/imported variables
 *********************************************/

extern BOOLEAN writeable;
extern STRING readpath,readpath_file;
extern STRING illegal_char;
extern INT opt_finnish, opt_mychar;

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN load_configs(STRING configfile, STRING * pmsg);
static BOOLEAN open_database_impl(LLDATABASE lldb, INT alteration);
static void post_codesets_hook(void);
static void pre_codesets_hook(void);
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

	if (!load_configs(configfile, pmsg)) {
		suppress_reload = FALSE;
		update_useropts(NULL);
		return FALSE;
	}

	pre_codesets_hook(); /* For MS-Windows user config of console codepages */

	/* now that codeset variables are set from config file, lets initialize codesets */
	/* although int_codeset can't be determined yet, we need GUI codeset for gettext */
	init_codesets();

	post_codesets_hook(); /* For Windows, link dynamically to gettext & iconv if available */


	/* until we have an internal codeset (which is until we open a database)
	we want output in display codeset */
	llgettext_init(PACKAGE, gui_codeset_out);

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
		setoptstr_fallback(dirvars[i], str);
	}
	/* also check environment variable for editor */
	{
		STRING str = getenv("LLEDITOR");
		if (!str)
			str = environ_determine_editor(PROGRAM_LIFELINES);
		setoptstr_fallback("LLEDITOR", str);
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

	/* Finnish always uses custom character sets */
	if (opt_finnish ) {
		opt_mychar = TRUE;
		mych_set_table(ISO_Latin1);
	}
	
	
	return TRUE;
}
/*=================================
 * init_lifelines_db -- Initialization after db opened
 *===============================*/
BOOLEAN
init_lifelines_db (void)
{
	STRING emsg;
	TABLE dbopts = create_table_str();

	tagtable = create_table_str(); /* values are same as keys */
	placabbvs = create_table_str();

	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", dbopts, '=', &emsg);
	set_db_options(dbopts);
	release_table(dbopts);
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
	lldb_close(&def_lldb); /* make sure database closed */
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
	xlat_shutdown();
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
open_database_impl (LLDATABASE lldb, INT alteration)
{
	int c;
	INT writ = !readonly + writeable;
	BTREE btree = 0;

	/* handle db adjustments (which only affect keyfile) first */
	if (alteration > 0 && !alterdb(alteration)) return FALSE;

	/* call btree module to do actual open of BTR */
	if (!(btree = bt_openbtree(readpath, FALSE, writ, immutable)))
		return FALSE;
	lldb_set_btree(lldb, btree);
	/* we have to set the global variable readonly correctly, because
	it is used widely */
	readonly = !bwrite(btree);
	immutable = bimmut(btree);
	if (readonly && writeable) {
		c = bkfile(btree).k_ostat;
		if (c < 0) {
			bterrno = BTERR_WRITER;
		} else {
			rdr_count = c-1;
			bterrno = BTERR_READERS;
		}
		return FALSE;
	}
	return TRUE;
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
/*==================================================
 * open_database -- open database
 *  forceopen:    [in] flag to override reader/writer protection
 *  dbpath:       [in] database path to open
 *================================================*/
BOOLEAN
open_database (INT alteration, STRING dbpath)
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

	rtn = open_database_impl(lldb, alteration);
	if (!rtn) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		lldb_close(&lldb);
		bterrno = myerr;
	}
	def_lldb = lldb;
	return rtn;
}
/*==================================================
 * create_database -- create (& open) brand new database
 *  dbpath:  [IN]  path of database about to create
 *================================================*/
BOOLEAN
create_database (STRING dbpath)
{
	LLDATABASE lldb = lldb_alloc();
	BTREE btree = 0;

	/* first test that newdb props are legal */
	STRING props = getoptstr("NewDbProps", 0);
	if (props && props[0]) {
		TABLE dbopts = create_table_str();
		STRING msg=0;
		if (!init_valtab_from_string(props, dbopts, '=', &msg)) {
			bterrno = BTERR_BADPROPS;
			destroy_table(dbopts);
			return FALSE;
		}
		destroy_table(dbopts);
	}

	/* tentatively copy paths into gedlib module versions */
	readpath_file=strsave(lastpathname(dbpath));
	readpath=strsave(dbpath);

	if (!(btree = bt_openbtree(dbpath, TRUE, 2, immutable))) {
		/* open failed so clean up, preserve bterrno */
		int myerr = bterrno;
		lldb_close(&lldb);
		bterrno = myerr;
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
is_codeset_utf8 (CNSTRING codename)
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
	if (def_lldb)
		update_db_options();
	/* in case user changed any codesets */
	init_codesets();
	/* in case user changed locale (need int_codeset already set) */
	uilocale();
	/* in case user changed codesets */
	/* TODO: Isn't this superfluous, as it was called in update_db_options above ? */
	transl_load_xlats();

	strupdate(&illegal_char, getoptstr("IllegalChar", 0));

	nodechk_enable(!!getoptint("nodecheck", 0));
}
/*==================================================
 * update_db_options -- 
 *  check database-specific options for updates
 *================================================*/
static void
update_db_options (void)
{
	TABLE opttab = create_table_str();
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
		if (uu8) {
			charprops_load_utf8();
		} else {
			charprops_load(int_codeset);
		}
	}

	destroy_table(opttab);
}
/*==================================================
 * pre_codesets_hook -- code to run just before initializing codesets
 * For MS-Windows user config of console codepages
 *================================================*/
static void
pre_codesets_hook (void)
{
#ifdef WIN32
	/* On MS-Windows, attempt to set any requested non-standard codepage */
	/* Do this now, before init_codesets below */
	INT i = getoptint("ConsoleCodepage", 0);
	if (i) {
		w_set_oemout_codepage(i);
		w_set_oemin_codepage(i);
	}
#endif
}
/*==================================================
 * post_codesets_hook -- code to run just after initializing codesets
 * For Windows, link dynamically to gettext & iconv if available
 *================================================*/
static void
post_codesets_hook (void)
{
	init_win32_gettext_shim();
	init_win32_iconv_shim(getoptstr("iconv.path",""));
}
/*==================================================
 * load_configs -- Load global config file(s)
 * returns FALSE if error, with message in pmsg
 *================================================*/
static BOOLEAN
load_configs (STRING configfile, STRING * pmsg)
{
	INT rtn=0;
	STRING str=0;

	/* TODO: Should read a system-wide config file */

	if (!configfile)
		configfile = getenv("LLCONFIGFILE");

	*pmsg = NULL;


	if (configfile && configfile[0]) {

		rtn = load_global_options(configfile, pmsg);
		if (rtn == -1) return FALSE;

	} else {

		/* No config file specified, so try local config_file */
		STRING cfg_file = environ_determine_config_file();

		rtn = load_global_options(cfg_file, pmsg);
		if (rtn == -1) return FALSE;
		if (rtn == 0) {

			/* No config file found, so try $HOME/config_file */
			char cfg_name[MAXPATHLEN];
			/* TODO: Shouldn't Win32 use getenv("USERPROFILE") ? */
			llstrncpy(cfg_name, getenv("HOME") , sizeof(cfg_name), 0);
			llstrappc(cfg_name, sizeof(cfg_name), '/');
			llstrapps(cfg_name, sizeof(cfg_name), 0, cfg_file);

			rtn = load_global_options(cfg_file, pmsg);
			if (rtn == -1) return FALSE;

		}
	}
	if (rtn == 0) return TRUE; /* no config file found */

	/* allow chaining to one more config file */

	str = getoptstr("LLCONFIGFILE", NULL);
	if (str && str[0]) {
		rtn = load_global_options(str, pmsg);
		if (rtn == -1) return FALSE;
	}
	return TRUE;
}
