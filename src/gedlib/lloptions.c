/* 
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*==========================================================
 * lloptions.c -- Read options from config file (& db user options)
 *   added in 3.0.6 by Perry Rapp
 *========================================================*/

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "llstdlib.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "lloptions.h"


/*********************************************
 * external variables
 *********************************************/

extern STRING qSopt2long;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void copy_process(STRING dest, STRING src);
static void expand_variables(STRING valbuf, INT max);
static BOOLEAN load_config_file(STRING file, STRING * pmsg);
static void send_notifications(void);

/*********************************************
 * local variables
 *********************************************/

/* table holding option values, both keys & values in heap */
/* listed in descending priority */
static TABLE f_cmd=0; /* option values from command line of current execution */
static TABLE f_rpt=0; /* option values local to currently running report program */
	/* NB: cannot actually set any report option values currently 2002-10-07 */
static TABLE f_db=0; /* option values in current database (user/options) */
static TABLE f_global=0; /* option values from lines config file */
static TABLE f_predef=0; /* predefined variables during config file processing */
static TABLE f_fallback=0; /* lowest priority option values */
static LIST f_notifications=0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==========================================
 * copy_process -- copy config value line,
 *  converting any escape characters
 *  This handles \n, \t, and \\
 *  We do not trim out backslashes, unless they 
 *  are part of escape sequences. (This is mostly
 *  because backslashes are so prevalent in
 *  MS-Windows paths.)
 * The output (dest) is no longer than the input (src).
 * Created: 2001/11/09, Perry Rapp
 *========================================*/
static void
copy_process (STRING dest, STRING src)
{
	STRING q=dest,p=src;
	char c;
	while ((*q++ = c = *p++)) {
		if (c == '\\') {
			if (!(c = *p++)) {
				*q = 0;
				break;
			}
			if (c == 'n')
				q[-1] = '\n';
			else if (c == 't')
				q[-1] = '\t';
			else if (c == '\\')
				q[-1] = '\\';
			else
				--p;
		}
	}
}
/*==========================================
 * expand_variables -- do any variable substitutions
 *  (variables are option properties starting & ending with %
 * Created: 2002/10/21, Perry Rapp
 *========================================*/
static void
expand_variables (STRING valbuf, INT max)
{
	STRING start, end;
	STRING ptr; /* remainder of valbuf to check */
	ptr = valbuf;
	while ((start=strchr(ptr, '%')) && (end=strchr(start+1, '%'))) {
		STRING name = allocsubbytes(start, 0, end-start+1);
		STRING value = valueof_str(f_global, name);
		if (!value)
			value = valueof_str(f_predef, name);
		if (value) {
			INT newlen = strlen(valbuf)-(end-start+1)+strlen(value);
			if (newlen < max) {
				STRING copy = strdup(valbuf);
				if (start>valbuf)
					strncpy(valbuf, copy, start-valbuf);
				strcpy(start, value);
				strcpy(start+strlen(value), copy+(end-valbuf+1));
				stdfree(copy);
			}
		}
		stdfree(name);
		ptr = end+1;
	}
}
/*==========================================
 * dir_from_file -- return directory of file
 * heap-allocated
 *========================================*/
static STRING
dir_from_file (STRING file)
{
	STRING thisdir = strdup(file);
	STRING ptr;
	for (ptr=thisdir+strlen(thisdir)-1; ptr>thisdir; --ptr) {
		if (is_dir_sep(*ptr))
			break;
	}
	*ptr = 0;
	return thisdir;
}
/*==========================================
 * load_config_file -- read options in config file
 *  and load into table (f_global)
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static BOOLEAN
load_config_file (STRING file, STRING * pmsg)
{
	FILE * fp = 0;
	STRING ptr, val;
	STRING oldval=NULL;
	STRING thisdir = dir_from_file(file);
	BOOLEAN there, failed, noesc;
	char buffer[MAXLINELEN],valbuf[MAXLINELEN];
	fp = fopen(file, LLREADTEXT);
	if (!fp) {
	        free(thisdir);
		return TRUE; /* no config file, that is ok */
        }
	f_predef = create_table_new();

	table_insert_string(f_predef, "%thisdir%", thisdir);
	strfree(&thisdir);
	/* read thru config file til done (or error) */
	while (fgets(buffer, sizeof(buffer), fp)) {
		noesc = FALSE;
		if (buffer[0] == '#')
			continue; /* ignore lines starting with # */
		if (!feof(fp) && buffer[strlen(buffer)-1] != '\n') {
			/* bail out if line too long */
			break;
		}
		chomp(buffer); /* trim any trailing CR or LF */
		/* find =, which separates key from value */
		for (ptr = buffer; *ptr && *ptr!='='; ptr++)
			;
		if (*ptr != '=' || ptr==buffer)
			continue; /* ignore lines without = or key */
		*ptr=0; /* zero-terminate key */
		if (ptr[-1] == ':') {
			noesc = TRUE; /* := means don't do backslash escapes */
			ptr[-1] = 0;
		}
		/* overwrite any previous value */
		oldval = valueofbool_str(f_global, buffer, &there);
		if (there) {
			ASSERT(oldval); /* no nulls in f_global*/
			stdfree(oldval);
		}
		/* advance over separator to value */
		ptr++;
		/*
		process the value into valbuf
		this handles escapes (eg, "\n")
		the output (valbuf) is no longer than the input (ptr)
		*/
		if (noesc)
			llstrncpy(valbuf, ptr, sizeof(valbuf), uu8);
		else
			copy_process(valbuf, ptr);
		expand_variables(valbuf, sizeof(valbuf));
		val = valbuf;
		insert_table_str(f_global, strsave(buffer), strsave(val));
	}
	failed = !feof(fp);
	fclose(fp);
	if (failed) {
		/* error is in heap */
		*pmsg = strsave(_(qSopt2long));
		return FALSE;
	}
	free_optable(&f_predef);
	send_notifications();
	return TRUE;
}
/*=================================
 * load_global_options -- 
 *  Load internal table of global options from caller-specified config file
 * STRING * pmsg: heap-alloc'd error string if fails
 *===============================*/
BOOLEAN
load_global_options (STRING configfile, STRING * pmsg)
{
	*pmsg = NULL;
	if (!f_global) 
		f_global= create_table(FREEBOTH);
	if (!load_config_file(configfile, pmsg))
		return FALSE;
	return TRUE;
}
/*=================================
 * set_cmd_options -- Store cmdline options from caller
 * Created: 2002/10/07, Perry Rapp
 *===============================*/
void
set_cmd_options (TABLE opts)
{
	ASSERT(opts);
	free_optable(&f_cmd);
	f_cmd = create_table(FREEBOTH);
	copy_table(opts, f_cmd, FREEBOTH);
	send_notifications();
}
/*=================================
 * set_db_options -- Store db options from caller
 * Created: 2002/06/16, Perry Rapp
 *===============================*/
void
set_db_options (TABLE opts)
{
	ASSERT(opts);
	free_optable(&f_db);
	f_db = create_table(FREEBOTH);
	copy_table(opts, f_db, FREEBOTH);
	send_notifications();
}
/*=================================
 * get_db_options -- Copy db options to caller's table
 * Created: 2002/06/16, Perry Rapp
 *===============================*/
void
get_db_options (TABLE opts)
{
	if (!f_db)
		f_db = create_table(FREEBOTH);
	copy_table(f_db, opts, FREEBOTH);
}
/*=================================
 * set_global_options -- Store db options from caller
 * Created: 2002/06/18, Perry Rapp
 *===============================*/
void
set_global_options (TABLE opts)
{
	ASSERT(opts);
	free_optable(&f_global);
	f_global = create_table(FREEBOTH);
	copy_table(opts, f_global, FREEBOTH);
	send_notifications();
}
/*=================================
 * get_global_options -- Copy db options to caller's table
 * Created: 2002/06/18, Perry Rapp
 *===============================*/
void
get_global_options (TABLE opts)
{
	if (!f_global)
		f_global = create_table(FREEBOTH);
	copy_table(f_global, opts, FREEBOTH);
}
/*==========================================
 * free_optable -- free a table if it exists
 *========================================*/
void
free_optable (TABLE * ptab)
{
	if (*ptab) {
		destroy_table(*ptab);
		*ptab = 0;
	}
}
/*==========================================
 * term_lloptions -- deallocate structures
 * used by lloptions at program termination
 * Safe to be called more than once
 * Created: 2001/04/30, Matt Emmerton
 *========================================*/
void
term_lloptions (void)
{
	free_optable(&f_cmd);
	free_optable(&f_rpt);
	free_optable(&f_db);
	free_optable(&f_global);
	free_optable(&f_fallback);
	remove_listeners(&f_notifications);
}
/*===============================================
 * getoptstr -- get an option (from db or from global)
 * Example: 
 *  str = getoptstr("HDR_SUBM", "1 SUBM");
 * Created: 2001/11/22, Perry Rapp
 *=============================================*/
STRING
getoptstr (CNSTRING optname, STRING defval)
{
	STRING str = 0;
	if (!str && f_cmd)
		str = valueof_str(f_cmd, optname);
	if (!str && f_db)
		str = valueof_str(f_db, optname);
	if (!str && f_global)
		str = valueof_str(f_global, optname);
	if (!str && f_fallback)
		str = valueof_str(f_fallback, optname);
	if (!str)
		str = defval;
	return str;
}
/*===============================================
 * getoptstr_rpt -- get an option (checking report-local options first)
 * Example: 
 *  str = getoptstr("HDR_SUBM", "1 SUBM");
 * Created: 2002/06/16, Perry Rapp
 *=============================================*/
STRING
getoptstr_rpt (CNSTRING optname, STRING defval)
{
	STRING str = 0;
	if (!str && f_rpt)
		str = valueof_str(f_rpt, optname);
	if (!str)
		str = getoptstr(optname, defval);
	return str;
}
/*===============================================
 * getoptstr_dbonly -- get an option (but only look at db options)
 * Example: 
 *  str = getoptstr_dbonly("codeset", 0);
 * Created: 2002/06/16, Perry Rapp
 *=============================================*/
STRING
getoptstr_dbonly (CNSTRING optname, STRING defval)
{
	STRING str = 0;
	if (f_db)
		str = valueof_str(f_db, optname);
	if (!str)
		str = defval;
	return str;
}
/*===============================================
 * getoptint -- get a numerical option
 *  First tries user option table (looks up optname)
 *  Then tries config option table
 *  Finally defaults to defval
 * Example: 
	if (getoptint("FullReportCallStack", 0) > 0)
 * Created: 2001/11/22, Perry Rapp
 *=============================================*/
INT
getoptint (CNSTRING optname, INT defval)
{
	STRING str = getoptstr(optname, 0);
	return str ? atoi(str) : defval;
}
/*===============================================
 * setoptstr_fallback -- Set option fallback value
 *  newval must be in heap!
 *=============================================*/
void
setoptstr_fallback (STRING optname, STRING newval)
{
	if (!f_fallback)
		f_fallback = create_table(FREEBOTH);
	replace_table_str(f_fallback, strsave(optname), newval, FREEBOTH);
	send_notifications();
}
/*===============================================
 * register_notify -- Caller wants to be notified when options change
 *=============================================*/
void
register_notify (CALLBACK_FNC fncptr)
{
	add_listener(&f_notifications, fncptr, 0);
}
/*===============================================
 * unregister_notify -- Caller no longer wants to be notified when options change
 *=============================================*/
void
unregister_notify (CALLBACK_FNC fncptr)
{
	delete_listener(&f_notifications, fncptr, 0);
}
/*===============================================
 * send_notifications -- Send notifications to any registered listeners
 *=============================================*/
static void
send_notifications (void)
{
	notify_listeners(&f_notifications);
}
