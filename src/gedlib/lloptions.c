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


#include "llstdlib.h"
#include "screen.h"
#include "gedcom.h"
#include "liflines.h"
#include "lloptions.h"


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

#define ARRSIZE(qq) (sizeof(qq)/sizeof(qq[0]))
enum { DBNO, DBYES };

/*********************************************
 * local function prototypes
 *********************************************/

static STRING numtostr(INT num);
static void load_config_file(STRING file);
static void read_db_options(void);
static INT update_opt(ENTRY ent);
static void store_to_lloptions(void);

/*********************************************
 * local variables
 *********************************************/

static struct int_option_s int_options[] = {
	"ListDetailLines", &lloptions.list_detail_lines, 0, DBYES
	,"AddMetadata", &lloptions.add_metadata, FALSE, DBYES
	,"ReadFromArchives", &lloptions.read_from_archives, FALSE, DBNO
};
static struct str_option_s str_options[] = {
	"EmailAddr", &lloptions.email_addr, "", DBYES
	,"LLEDITOR", &lloptions.lleditor, "", DBNO
	,"LLPROGRAMS", &lloptions.llprograms, "", DBNO
	,"LLREPORTS", &lloptions.llreports, "", DBNO
	,"LLARCHIVES", &lloptions.llarchives, "", DBNO
	,"LLDATABASES", &lloptions.lldatabases, "", DBNO
	,"LLNEWDBDIR", &lloptions.llnewdbdir, "", DBNO
};

static TABLE opttab=0;
static TABLE nodbopt=0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==========================================
 * read_lloptions_from_config -- Read options
 *  from global config file
 *========================================*/
void
read_lloptions_from_config (void)
{
	INT i;
	ASSERT(!opttab && !nodbopt);
	opttab = create_table();
	nodbopt = create_table();

	/* load table with defaults */
	for (i=0; i<ARRSIZE(int_options); i++) {
		insert_table(opttab, int_options[i].name,
			strsave(numtostr(int_options[i].defval)));
		if (int_options[i].db == DBNO)
			insert_table(nodbopt, int_options[i].name, 0);
	}
	for (i=0; i<ARRSIZE(str_options); i++) {
		insert_table(opttab, str_options[i].name,
			strsave(str_options[i].defval));
		if (str_options[i].db == DBNO)
			insert_table(nodbopt, str_options[i].name, 0);
	}

	load_config_file(environ_determine_config_file());

	store_to_lloptions();
}
/*==========================================
 * read_lloptions_from_db -- Update options
 *  from database user options
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
 *========================================*/
void
term_lloptions (void)
{
	remove_table(opttab, FREEVALUE);
	remove_table(nodbopt, DONTFREE);
}
/*==========================================
 * numtostr -- Convert INT to STRING
 *  returns static buffer
 *========================================*/
static STRING
numtostr (INT num)
{
	static char buffer[33];
	return itoa(num, buffer, 10);
}
/*==========================================
 * load_config_file -- read options in config file
 *  and load into table
 *========================================*/
static void
load_config_file (STRING file)
{
	FILE * fp = fopen(file, LLREADTEXT);
	STRING ptr, val;
	VPTR *oldval;
	INT len;
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
		oldval = access_value(opttab, buffer);
		if (!oldval)
			continue; /* ignore keys we don't have */
		free(*oldval);
		ptr++;
		val = ptr;
		len = strlen(val);
		if (val[len-1]=='\n')
			val[len-1] = 0;
		*oldval = strsave(val);
	}
	fclose(fp);
}
/*==========================================
 * read_db_options -- read db user options
 *  and load into table
 *========================================*/
static void
read_db_options (void)
{
	traverse_table(opttab, update_opt);
}
/*==========================================
 * update_opt -- update one option from db user options
 *========================================*/
static INT
update_opt (ENTRY ent)
{
	STRING key, value;
	key = ent->ekey;
	if (valueof(nodbopt, key))
		return 0; /* unused */
	value = (STRING) valueof(useropts, key);
	if (value) {
		free(ent->evalue);
		ent->evalue = strsave(value);
	}
	return 0; /* unused */
}
/*===============================================
 * changeoptstr -- Change an option string
 *  precondition - option strings are never NULL
 * Created: 2001/02/04, Perry Rapp
 *=============================================*/
void
changeoptstr (STRING * str, STRING newval)
{
	stdfree(*str);
	if (!newval)
		newval = strsave("");
	*str = newval;
}
/*==========================================
 * store_to_lloptions -- Update lloptions from
 *  opttab
 *========================================*/
static void
store_to_lloptions (void)
{
	INT i;
	/* store values back to options list */
	for (i=0; i<ARRSIZE(int_options); i++) {
		STRING str = valueof(opttab, int_options[i].name);
		*int_options[i].value = atoi(str);
	}
	for (i=0; i<ARRSIZE(str_options); i++) {
		STRING str = valueof(opttab, str_options[i].name);
		*str_options[i].value = strsave(str);
	}
}
