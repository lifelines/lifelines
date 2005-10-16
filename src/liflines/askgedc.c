/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * askgedc.c -- Construct list of GEDCOM files & prompt user
 *  Imitation of askprogram.c
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
static void add_gedcom_props(TABLE fileprops);
static void parse_gedcoms(TABLE * fileprops);
static int select_gedcoms(const struct dirent *entry);
static void set_gedcom_d0(TABLE * fileprops);

/* messages used */
extern STRING qSextchoos;

/*=========================
 * The supported meta-tags.
 *=======================*/

enum { P_SOUR=0, P_DATE=1 };

static CNSTRING f_tags[] = {
  "1 SOUR"
  , "1 DATE"
};


/*===========================================================
 * select_gedcoms -- choose files with the correct extention
 *==========================================================*/
static int
select_gedcoms (const struct dirent *entry)
{
	CNSTRING goodexts[] = { ".ged", ".gedcom" };
	INT i;
	/* examine end of entry->d_name */
	for (i=0; i<ARRSIZE(goodexts); ++i) {
		CNSTRING entext = entry->d_name + strlen(entry->d_name) - strlen(goodexts[i]);
		/* is it what we want ? use platform correct comparison, from path.c */
		if (path_match(goodexts[i], entext))
			return 1;
	}
	return 0;
}
/*==========================================================
 * add_gedcom_props -- set properties for gedcoms
 * Created: 2002/10/19, Perry Rapp
 *========================================================*/
static void
add_gedcom_props (TABLE fileprops)
{
	STRING tagsfound[ARRSIZE(f_tags)];
	FILE *fp;
	char str[MAXLINELEN];
	INT i;
	INT line=0;
	struct stat sbuf;

	/* first get full path & open file */
	STRING fname = valueof_str(fileprops, "filename");
	STRING dir = valueof_str(fileprops, "dir");
	STRING filepath = concat_path_alloc(dir, fname);

	if (!stat(filepath, &sbuf)) {
		if (sbuf.st_size > 9999999)
			sprintf(str, "%ldMb", (long)(sbuf.st_size/1000000));
		else if (sbuf.st_size > 9999)
			sprintf(str, "%ldKb", (long)(sbuf.st_size/1000));
		else
			sprintf(str, "%ldb", (long)sbuf.st_size);

		add_prop_dnum(fileprops, "bytes", str);
	}

	if (NULL == (fp = fopen(filepath, LLREADTEXT)))
		goto end_add_program_props;

	/* initialize array where we record metainfo we want */
	for (i=0; i<ARRSIZE(tagsfound); ++i)
		tagsfound[i] = 0;

	/* now read line by line looking for metainfo */
	while (NULL != fgets(str, sizeof(str), fp) && str[strlen(str)-1]=='\n' && line<20) {
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
		if (!strncmp(str, "0 @", 3))
			break;
		++line;
	}

	fclose(fp);

	/* add any metainfo we found to the property table */
	for (i=0; i<ARRSIZE(tagsfound); ++i) {
		if (tagsfound[i]) {
			add_prop_dnum(fileprops, f_tags[i], tagsfound[i]);
			strfree(&tagsfound[i]);
		}
	}

end_add_program_props:
	stdfree(filepath);
	return;
}
/*===================================================
 * parse_gedcoms -- parse gedcoms
 *  & record properties in property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
parse_gedcoms (TABLE * fileprops)
{
	INT i;
	for (i=0; fileprops[i]; ++i) {
		add_gedcom_props(fileprops[i]);
	}
}
/*===================================================
 * set_gedcom_d0 -- set display strings for gedcom files from property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
set_gedcom_d0 (TABLE * fileprops)
{
	INT i;
	for (i=0; fileprops[i]; ++i) {
		TABLE props = fileprops[i];
		char buf[MAXLINELEN];
		STRING filename = valueof_str(props, "filename");
		STRING sour = valueof_str(props, (STRING)f_tags[P_SOUR]);
		STRING date = valueof_str(props, (STRING)f_tags[P_DATE]);
		STRING bytes = valueof_str(props, "bytes");
		if (!sour) sour="?";
		if (!date) date="?";
		if (!bytes) bytes="?";
		snprintf(buf, sizeof(buf), "%s (%s - %s) [%s]"
			, filename, date, bytes, sour);
		set_prop_dnum(props, 0, strdup("prompt"), strdup(buf));
	}
}
/*===================================================
 * ask_for_gedcom -- Ask for gedcom file
 *  pfname: [OUT]  allocated on heap (name we tried to open)
 *=================================================*/
BOOLEAN
ask_for_gedcom (STRING mode,
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

	fileprops = get_proparray_of_files_in_path(path, select_gedcoms, &nfiles);
	parse_gedcoms(fileprops);
	set_gedcom_d0(fileprops);

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
		if (pfname)
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
