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

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "llstdlib.h"
#include "gedcom.h"
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
static BOOLEAN load_config_file(STRING file, STRING * pmsg);

/*********************************************
 * local variables
 *********************************************/

/* table holding option values, both keys & values in heap */
static TABLE opttab=0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==========================================
 * numtostr -- Convert INT to STRING
 *  returns static buffer
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
#ifdef UNUSED_CODE
static STRING
numtostr (INT num)
{
	static char buffer[33];
	snprintf(buffer, sizeof(buffer), "%d", num);
	return buffer;
}
#endif
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
 * load_config_file -- read options in config file
 *  and load into table (opttab)
 * Created: 2001/02/04, Perry Rapp
 *========================================*/
static BOOLEAN
load_config_file (STRING file, STRING * pmsg)
{
	FILE * fp = 0;
	STRING ptr, val;
	STRING oldval=NULL;
	BOOLEAN there, failed, noesc;
	char buffer[MAXLINELEN],valbuf[MAXLINELEN];
	fp = fopen(file, LLREADTEXT);
	if (!fp)
		return TRUE; /* no config file, that is ok */
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
		oldval = valueofbool_str(opttab, buffer, &there);
		if (there) {
			ASSERT(oldval); /* no nulls in opttab */
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
			llstrncpy(valbuf, ptr, sizeof(valbuf));
		else
			copy_process(valbuf, ptr);
		val = valbuf;
		insert_table_str(opttab, strsave(buffer), strsave(val));
	}
	failed = !feof(fp);
	fclose(fp);
	if (failed) {
		/* error is in heap */
		*pmsg = strsave(_(qSopt2long));
		return FALSE;
	}
	return TRUE;
}
/*=================================
 * init_lifelines_options -- Initialize LifeLines
 *  before db opened
 * STRING * pmsg: heap-alloc'd error string if fails
 *===============================*/
BOOLEAN
init_lifelines_options (STRING configfile, STRING * pmsg)
{
	*pmsg = NULL;
	term_lloptions(); /* clear if exists */
	opttab = create_table();
	if (!load_config_file(configfile, pmsg))
		return FALSE;
	return TRUE;
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
	if (opttab) {
		/* empty & delete actual option table */
		remove_table(opttab, FREEBOTH);
		opttab = 0;
	}
}
/*===============================================
 * getoptstr -- get an option
 *  First tries user option table (looks up optname)
 *  Then tries looking in lloption table by name
 *  Finally defaults to defval
 * Example: 
	str = getoptstr("HDR_SUBM", "1 SUBM");
 * Created: 2001/11/22, Perry Rapp
 *=============================================*/
STRING
getoptstr (STRING optname, STRING defval)
{
	STRING str = 0;
	if (useropts)
		str = valueof_str(useropts, optname);
	if (!str)
		str = valueof_str(opttab, optname);
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
getoptint (STRING optname, INT defval)
{
	STRING str = 0;
	if (useropts)
		str = valueof_str(useropts, optname);
	if (!str)
		str = valueof_str(opttab, optname);
	return str ? atoi(str) : defval;
}
/*===============================================
 * changeoptstr -- Change an option string
 *  newval must be in heap!
 * Created: 2001/02/04, Perry Rapp
 *=============================================*/
void
changeoptstr (STRING optname, STRING newval)
{
	replace_table_str(opttab, strsave(optname), newval, FREEBOTH);
}
