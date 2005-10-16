/* 
   Copyright (c) 2001-2002 Petter Reinholdtsen & Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * askprogram.c -- Construct list of lifelines programs & prompt user
 *  Completely refactored 2002-10-19 to be usable for GUI
 *  and to be reusable for GEDCOM files (this code pushed into proptbls.c)
 *==============================================================*/

#include "standard.h"
#include "llstdlib.h"
#include "table.h"
#include "liflines.h"
#include "arch.h"
#include "proptbls.h"


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_program_props(TABLE fileprops);
static void parse_programs(TABLE * fileprops);
static int select_programs(const struct dirent *entry);
static void set_programs_d0(TABLE * fileprops);

/* messages used */
extern STRING qSextchoos;

/*=========================
 * The supported meta-tags.
 *=======================*/

enum { P_PROGNAME=0, P_VERSION=1, P_OUTPUT=4 };

static CNSTRING f_tags[] = {
  "progname"    /* The name of the script */
  , "version"     /* The version of the script */
  , "author"      /* The author of the script */
  , "category"    /* The category of the script */
  , "output"      /* The output format produced by this script */
  , "description" /* A description of purpose of the script */
};


/*===========================================================
 * select_programs -- choose files with the correct extention
 *==========================================================*/
static int
select_programs (const struct dirent *entry)
{
	CNSTRING goodext = ".ll";
	/* examine end of entry->d_name */
	CNSTRING entext = entry->d_name + strlen(entry->d_name) - strlen(goodext);

	/* is it what we want ? use platform correct comparison, from path.c */
	if (!path_match(goodext, entext))
		return 0;

	return 1;
}
/*==========================================================
 * add_program_props -- set properties for programs (parse program file for metainfo)
 * Created: 2002/10/19, Perry Rapp
 *========================================================*/
static void
add_program_props (TABLE fileprops)
{
	STRING tagsfound[ARRSIZE(f_tags)];
	FILE *fp;
	char str[MAXLINELEN];
	INT i;
	INT line=0;
	INT endcomment=0; /* flag when finished header comment */
	char * charset=0; /* charset of report, if found */

	/* first get full path & open file */
	STRING fname = valueof_str(fileprops, "filename");
	STRING dir = valueof_str(fileprops, "dir");
	STRING filepath = concat_path_alloc(dir, fname);
	char enc_cmd[] = "char_encoding(\"";

	if (NULL == (fp = fopen(filepath, LLREADTEXT)))
		goto end_add_program_props;

	/* initialize array where we record metainfo we want */
	for (i=0; i<ARRSIZE(tagsfound); ++i)
		tagsfound[i] = 0;

	/* what charset is the data in this report ? :( */
	/* TODO: need to watch for BOM or for report charset command */

	/* now read line by line looking for metainfo */
	while (NULL != fgets(str, sizeof(str), fp) && str[strlen(str)-1]=='\n' && line<20) {
		/* check for char encoding command, to help us interpret tag values */
		if (!strncmp(str, enc_cmd, sizeof(enc_cmd)-1)) {
			const char *start = str+sizeof(enc_cmd)-1;
			const char *end = start;
			while (*end && *end!='\"')
				++end;
			if (*end && end>start) {
				charset=stdalloc(strlen(str));
				llstrncpy(charset, start, end-start, 0);
			}
		}
		if (!endcomment) {
			/* pick up any tag values specified */
			STRING p;
			chomp(str); /* trim trailing CR or LF */
			for (i=0; i<ARRSIZE(f_tags); ++i) {
				CNSTRING tag = f_tags[i];
				if (tagsfound[i])
					continue; /* already have this tag */
				if (NULL != (p = strstr(str, tag))) {
					STRING s = p + strlen(tag);
					/* skip leading space */
					while (s[0] && isspace((uchar)s[0]))
						++s;
					striptrail(s);
					if (s[0])
						tagsfound[i] = strsave(s);
					break;
				}
			}
			if (strstr(str, "*/"))
				endcomment=1;
		}
		++line;
	}

	fclose(fp);

	/* add any metainfo we found to the property table */
	for (i=0; i<ARRSIZE(tagsfound); ++i) {
		if (tagsfound[i]) {
			/* TODO: translate tagsfound[i] with charset */
			add_prop_dnum(fileprops, f_tags[i], tagsfound[i]);
			strfree(&tagsfound[i]);
		}
	}

end_add_program_props:
	strfree(&charset);
	stdfree(filepath);
	return;
}
/*===================================================
 * parse_programs -- parse lifelines report programs 
 *  & record properties in property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
parse_programs (TABLE * fileprops)
{
	INT i;
	for (i=0; fileprops[i]; ++i) {
		add_program_props(fileprops[i]);
	}
}
/*===================================================
 * set_programs_d0 -- set display strings for programs from property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
set_programs_d0 (TABLE * fileprops)
{
	INT i;
	for (i=0; fileprops[i]; ++i) {
		TABLE props = fileprops[i];
		char buf[MAXLINELEN];
		STRING program = valueof_str(props, (STRING)f_tags[P_PROGNAME]);
		STRING version = valueof_str(props, (STRING)f_tags[P_VERSION]);
		STRING output = valueof_str(props, (STRING)f_tags[P_OUTPUT]);
		if (!program)
			program = valueof_str(props, "filename");
		if (!output)
			output = "?";
		if (!version)
			version = "V?.?";
		snprintf(buf, sizeof(buf), "%s (%s) [%s]"
			, program, version, output);
		set_prop_dnum(props, 0, strdup("prompt"), strdup(buf));
	}
}
/*===================================================
 * ask_for_program -- Ask for program
 *  pfname: [OUT]  allocated on heap (name we tried to open)
 *=================================================*/
BOOLEAN
ask_for_program (STRING mode,
                 STRING ttl,
                 STRING *pfname,
                 STRING *pfullpath,
                 STRING path,
                 STRING ext,
                 BOOLEAN picklist)
{
	int choice;
	INT nfiles, i;
	TABLE * fileprops;
	STRING * choices;
	FILE * fp;

	if (pfname) *pfname = 0;
	if (pfullpath) *pfullpath = 0;

	if (!picklist) {
		goto AskForString;
	}


	fileprops = get_proparray_of_files_in_path(path, select_programs, &nfiles);
	parse_programs(fileprops);
	set_programs_d0(fileprops);
	if (!nfiles) goto AskForString;

	choices = (STRING *)stdalloc(sizeof(STRING)*(nfiles+1));
	/* choices are all shared pointers */
	choices[0] = _(qSextchoos);
	for (i=0; i<nfiles; ++i) {
		choices[i+1] = valueof_str(fileprops[i], valueof_str(fileprops[i], "d0"));
	}
	choice = choose_from_array_x(ttl, nfiles+1, choices, proparrdetails, fileprops);
	if (choice > 0) {
		TABLE props = fileprops[choice-1];
		STRING fname = valueof_str(props, "filename");
		STRING dir = valueof_str(props, "dir");
		STRING filepath = concat_path_alloc(dir, fname);
		*pfname = strsave(fname);
		*pfullpath = filepath;
	}
	free_proparray(&fileprops);
	stdfree(choices);
	if (choice == 0) {
		/* 0th choice is to go to AskForString prompt instead */
		goto AskForString;
	}
	return (choice > 0);

AskForString:
	fp = ask_for_input_file(mode, ttl, pfname, pfullpath, path, ext);
	if (fp)
		fclose(fp);
	return fp != 0;
}
/*================================================
 * proparrdetails -- print details of a file using proparray
 * Callback from choose_from_array_x
 *  arrdets:  [IN]  package of list & current status
 *  param:    [IN]  the param we passed when we called choose_from_array_x
 * Created: 2001/12/15 (Perry Rapp)
 *==============================================*/
void
proparrdetails (ARRAY_DETAILS arrdets, void * param)
{
	TABLE * fileprops = (TABLE *)param;
	TABLE props;
	INT count = arrdets->count;
	INT maxlen = arrdets->maxlen;
	INT index = arrdets->cur - 1; /* slot#0 used by choose string */
	INT row=0;
	INT scroll = arrdets->scroll;
	INT i;
	INT dnum;

	if (index<0) return;

	props = fileprops[index];
	
	dnum = ll_atoi(valueof_str(props, "dn"), 0);

	/* print tags & values */
	for (i=scroll; i<dnum && row<count; ++i, ++row) {
		STRING detail = arrdets->lines[row];
		char temp[20];
		STRING name=0, value=0;
		sprintf(temp, "d%d", i+1);
		name = valueof_str(props, temp);
		detail[0]=0;
		if (name) {
			value = valueof_str(props, name);
			llstrapps(detail, maxlen, uu8, name);
			llstrapps(detail, maxlen, uu8, ": ");
			if (value) {
				llstrapps(detail, maxlen, uu8, value);
			}
		}
	}
  /* TODO: read file header */
}
