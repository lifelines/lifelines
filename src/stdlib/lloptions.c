/* 
   Copyright (c) 2000-2001 Perry Rapp

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

/*==========================================================
 * lloptions.c -- Read options from config file (& db user options)
 *   added in 3.0.6 by Perry Rapp
 *========================================================*/

#ifdef OS_LOCALE
#include <locale.h>
#endif
#include "llstdlib.h"
#include "screen.h"
#include "gedcom.h"
#include "liflines.h"
#include "lloptions.h"


/*********************************************
 * external variables
 *********************************************/

extern BOOLEAN selftest;
extern STRING errorfile;

/*********************************************
 * global/exported variables
 *********************************************/

struct lloptions_s lloptions;

/*********************************************
 * local types
 *********************************************/

struct int_option_s { STRING name; INT * value; INT defval; INT db; };
struct str_option_s { STRING name; STRING * value; STRING defval; INT db; };

/*********************************************
 * local enums & defines
 *********************************************/

enum { DBNO, DBYES };

/*********************************************
 * local function prototypes
 *********************************************/

static STRING numtostr(INT num);
static void load_config_file(STRING file);
static void read_db_options(void);
static void update_opt(ENTRY ent);
static void store_to_lloptions(void);
static STRING getsaveenv(STRING key);

/*********************************************
 * local variables
 *********************************************/

/* numeric user options */
static struct int_option_s int_options[] = {
	{ "ListDetailLines", &lloptions.list_detail_lines, 0, DBNO }
		/* add_metadata needs to be moved to dboptions somewhere
		then DBYES/DBNO can be removed entirely */
	,{ "AddMetadata", &lloptions.add_metadata, 0, DBYES }
	,{ "DenySystemCalls", &lloptions.deny_system_calls, 0, DBNO }
	,{ "PerErrorDelay", &lloptions.per_error_delay, 0, DBNO }
	,{ "FullReportCallStack", &lloptions.report_error_callstack, 0, DBNO }
	,{ "CustomizeLongDate", &lloptions.date_customize_long, 0, DBNO }
	,{ "CustomizeLongDfmt", &lloptions.date_long_dfmt, 0, DBNO }
	,{ "CustomizeLongMfmt", &lloptions.date_long_mfmt, 0, DBNO }
	,{ "CustomizeLongYfmt", &lloptions.date_long_yfmt, 0, DBNO }
	,{ "CustomizeLongSfmt", &lloptions.date_long_sfmt, 0, DBNO }
};
/* string user options */
static struct str_option_s str_options[] = {
	{ "UiLocale", &lloptions.uilocale, "C", DBNO }
	,{ "ReportLocale", &lloptions.rptlocale, "C", DBNO }
	,{ "LLEDITOR", &lloptions.lleditor, "", DBNO }
	,{ "LLPROGRAMS", &lloptions.llprograms, "", DBNO }
	,{ "LLREPORTS", &lloptions.llreports, "", DBNO }
	,{ "LLARCHIVES", &lloptions.llarchives, "", DBNO }
	,{ "LLDATABASES", &lloptions.lldatabases, "", DBNO }
	,{ "LLNEWDBDIR", &lloptions.llnewdbdir, "", DBNO }
	,{ "LLTTREF", &lloptions.llttref, "", DBNO }
	,{ "LLTTEXPORT", &lloptions.llttexport, "", DBNO }
	,{ "InputPath", &lloptions.inputpath, "", DBNO }
	,{ "ReportLog", &lloptions.reportlog, "", DBNO }
	,{ "ErrorLog", &lloptions.errorlog, "", DBNO }
};

static TABLE opttab=0;
static TABLE nodbopt=0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==========================================
 * init_lloptions -- Set options to default values
 * Created: 2001/03/17, Perry Rapp
 *========================================*/
void
init_lloptions (void)
{
	INT i;
	ASSERT(!opttab && !nodbopt);
	opttab = create_table();
	nodbopt = create_table();

	/* load table with defaults */
	for (i=0; i<ARRSIZE(int_options); i++) {
		insert_table_str(opttab, int_options[i].name,
			strsave(numtostr(int_options[i].defval)));
		if (int_options[i].db == DBNO)
			insert_table_int(nodbopt, int_options[i].name, 1);
	}
	for (i=0; i<ARRSIZE(str_options); i++) {
		insert_table_str(opttab, str_options[i].name,
			strsave(str_options[i].defval));
		if (str_options[i].db == DBNO)
			insert_table_int(nodbopt, str_options[i].name, 1);
	}
}
/*==========================================
 * read_lloptions_from_config -- Read options
 *  from global config file
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
void
read_lloptions_from_config (void)
{
	init_lloptions();
	ASSERT(opttab && nodbopt);
	load_config_file(environ_determine_config_file());
	store_to_lloptions();
}
/*==========================================
 * read_lloptions_from_db -- Update options
 *  from database user options
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
void
read_lloptions_from_db (void)
{
	ASSERT(opttab && nodbopt);
	read_db_options();
	store_to_lloptions();
}
/*==========================================
 * term_lloptions -- clean up option memory
 * (for memory debugging)
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
void
term_lloptions (void)
{
	ASSERT(opttab && nodbopt);
	remove_table(opttab, FREEVALUE);
	remove_table(nodbopt, DONTFREE);
	opttab = nodbopt = 0;
}
/*==========================================
 * numtostr -- Convert INT to STRING
 *  returns static buffer
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static STRING
numtostr (INT num)
{
	static char buffer[33];
	snprintf(buffer, sizeof(buffer), "%d", num);
	return buffer;
}
/*==========================================
 * load_config_file -- read options in config file
 *  and load into table
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static void
load_config_file (STRING file)
{
	FILE * fp = fopen(file, LLREADTEXT);
	STRING ptr, val;
	STRING oldval=NULL;
	INT len;
	BOOLEAN there;
	char buffer[MAXLINELEN];
	if (!fp)
		return;
	while (fgets(buffer, sizeof(buffer), fp)) {
		if (buffer[0] == '#')
			continue; /* ignore lines starting with # */
		for (ptr = buffer; *ptr && *ptr!='='; ptr++)
			;
		if (*ptr != '=')
			continue; /* ignore lines without = */
		*ptr=0; /* zero-terminate key */
		oldval = valueofbool_str(opttab, buffer, &there);
		/* ignore keys not listed in opttab */
		if (!there) continue;
		ASSERT(oldval); /* no nulls in opttab */
		stdfree(oldval);
		ptr++;
		val = ptr;
		len = strlen(val);
		if (val[len-1]=='\n')
			val[len-1] = 0;
		insert_table_str(opttab, buffer, strsave(val));
	}
	fclose(fp);
}
/*==========================================
 * read_db_options -- read db user options
 *  and load into table
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static void
read_db_options (void)
{
	/*
	we'll cycle thru every key in opttab, and see if
	there is an update for it in useropts table
	*/
	traverse_table(opttab, update_opt);
}
/*==========================================
 * update_opt -- update one option from db user options
 *  used by read_db_options traversal
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static void
update_opt (ENTRY ent)
{
	STRING key, value;
	key = ent->ekey;
	/* ignore keys listed in dodbopt - can't be updated from db */
	if (valueof_int(nodbopt, key, 0))
		return;
	value = valueof_str(useropts, key);
	if (value) {
		/* switch to value from useropts */
		stdfree((STRING)ent->uval.w);
		ent->uval.w = strsave(value);
	}
}
/*===============================================
 * changeoptstr -- Change an option string
 *  precondition - option strings are never NULL
 * Created: 2001/02/04, Perry Rapp
 *=============================================*/
void
changeoptstr (STRING * str, STRING newval)
{
	INT i;
	STRING oldval=NULL;
	stdfree(*str);
	if (!newval)
		newval = strsave("");
	*str = newval;
	/* update value in table */
	for (i=0; i<ARRSIZE(str_options); i++) {
		if (str_options[i].value == str) {
			/* remove & free old value & replace with new one */
			BOOLEAN there;
			oldval = valueofbool_str(opttab, str_options[i].name, &there);
			ASSERT(there);
			stdfree(oldval);
			insert_table_str(opttab, str_options[i].name, strsave(newval));
		}
	}
}
/*==========================================
 * store_to_lloptions -- Update lloptions from
 *  opttab
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static void
store_to_lloptions (void)
{
	INT i;
	/* store int values in int options list */
	for (i=0; i<ARRSIZE(int_options); i++) {
		STRING str = valueof_str(opttab, int_options[i].name);
		*int_options[i].value = atoi(str);
	}
	/* store string values in string options list */
	for (i=0; i<ARRSIZE(str_options); i++) {
		STRING str = valueof_str(opttab, str_options[i].name);
		*str_options[i].value = strsave(str);
	}
}
/*==========================================
 * cleanup_lloptions -- deallocate structures
 * used by lloptions at program termination
 * Safe to be called more than once
 * Created: 2001/04/30, Matt Emmerton
 *========================================*/
void
cleanup_lloptions (void)
{
	INT i;
	/* free string values */
	for (i=0; i<ARRSIZE(str_options); i++) {
		STRING str = valueof_str(opttab, str_options[i].name);
		STRING * pstr = str_options[i].value;
		if (*pstr) {
			stdfree(*pstr);
			*pstr = NULL;
		}
	}
}
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

	set_usersort(custom_sort);
	llstrncpy(errorfile, lloptions.errorlog, sizeof(errorfile)/sizeof(errorfile[0]));
	return TRUE;
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
