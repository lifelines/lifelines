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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*==========================================================
 * charmaps.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 25 Jul 1994    3.0.2 - 09 Nov 1994
 *========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"

/*********************************************
 * global/exported variables
 *********************************************/


char *map_keys[NUM_TT_MAPS] = {
	"MEDIN", "MINED", "MGDIN", "MINGD",
	"MDSIN", "MINDS", "MINRP", "MSORT",
	"MCHAR", "MLCAS", "MUCAS", "MPREF"
};

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSbaddec,qSbadhex,qSnorplc,qSbadesc,qSnoorig,qSmaperr;

/*********************************************
 * local enums & defines
 *********************************************/


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void check_for_user_charmaps(STRING basename, TRANMAPPING ttm, CNSTRING mapname);
static void init_charmaps_if_needed(void);
static BOOLEAN init_map_from_rec(INT, TRANTABLE*);
static BOOLEAN init_map_from_str(STRING, CNSTRING mapname, TRANTABLE*);
static void load_custom_db_mappings(void);
static void load_global_char_mapping(void);
static void load_user_charmap(CNSTRING ttpath, CNSTRING mapname, TRANMAPPING ttm);
static void maperror(CNSTRING mapname, INT entry, INT line, STRING errmsg);
static void set_zone_conversion(STRING optname, INT toint, INT fromint);

/*********************************************
 * local variables
 *********************************************/

/* custom translation tables embedded in the database */
static struct tranmapping_s trans_maps[NUM_TT_MAPS]; /* init'd by init_charmaps */
static INT charmaps_inited=0;
static CNSTRING map_names[] = {
	"Editor to Internal"
	,"Internal to Editor"
	,"GEDCOM to Internal"
	,"Internal to GEDCOM"
	,"Display to Internal"
	,"Internal to Display"
	,"Internal to Report"
	,"Custom Sort"
	,"Custom Charset"
	,"Custom Lowercase"
	,"Custom Uppercase"
	,"Custom Prefix"
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/


/*========================================
 * init_charmaps_if_needed -- one time initialization
 *======================================*/
static void
init_charmaps_if_needed (void)
{
	/* Check that all tables have all entries */
	ASSERT(NUM_TT_MAPS == ARRSIZE(trans_maps));
	ASSERT(NUM_TT_MAPS == ARRSIZE(map_names));
	ASSERT(NUM_TT_MAPS == ARRSIZE(map_keys));

	memset(&trans_maps, 0, sizeof(trans_maps));
}
/*========================================
 * clear_char_mappings -- empty & free all data
 *======================================*/
static void
clear_char_mappings (void)
{
	INT indx=-1;

	for (indx = 0; indx < NUM_TT_MAPS; indx++) {
		TRANMAPPING ttm = &trans_maps[indx];
		TRANTABLE *ptt = &ttm->dbtrantbl;
		TRANTABLE ttx = 0;
		remove_trantable(*ptt);
		*ptt = 0;
		strfree(&ttm->iconv_src);
		strfree(&ttm->iconv_dest);
		ttm->after = -1;
		/* TODO: revisit this */
		/* For the moment, we only transliterate going to display */
		ttm->translit = (indx == MINDS);
		if (ttm->global_trans) {
			FORLIST(ttm->global_trans, tbel)
				ttx = tbel;
				ASSERT(ttx);
				remove_trantable(ttx);
				ttx = 0;
			ENDLIST
		}
	}
}
/*========================================
 * load_global_char_mapping -- load char mapping info
 *  from global configuration
 * NB: This depends on internal codeset, which depends on
 * active database.
 *======================================*/
static void
load_global_char_mapping (void)
{
	INT indx=-1;

	/* no translations without internal codeset */
	if (!int_codeset || !int_codeset[0])
		return;


	/* set iconv conversions as applicable */
	set_zone_conversion("GuiCodeset", MDSIN, MINDS);
	set_zone_conversion("EditorCodeset", MEDIN, MINED);
	set_zone_conversion("GedcomCodeset", MGDIN, MINGD);
	set_zone_conversion("ReportCodeset", -1, MINRP);

	for (indx = 0; indx < NUM_TT_MAPS; indx++) {
		TRANMAPPING ttm = &trans_maps[indx];
		if (ttm->after >= 0) {
			char name[80];
			CNSTRING keyname = map_keys[indx];
			/* eg, TT.MDSIN.UTF-8 */
			llstrncpy(name, "TT.", sizeof(name));
			llstrapp(name, sizeof(name), keyname);
			llstrapp(name, sizeof(name), ".");
			llstrapp(name, sizeof(name), int_codeset);
			llstrapp(name, sizeof(name), ".");
			check_for_user_charmaps(name, ttm, map_names[indx]);
		}
	}
}
/*========================================
 * check_for_user_charmaps -- Load any translation tables
 *  specified by user in global config file
 *  name:  [IN]  eg, "TT.MDSIN.UTF-8."
 *  ttm:   [I/O] mapping data in which to add
 *======================================*/
static void
check_for_user_charmaps (STRING basename, TRANMAPPING ttm, CNSTRING mapname)
{
	char name[120];
	CNSTRING ttname=0;
	INT i=1;
	CNSTRING ttdir = getoptstr("TTDIR", ".");
	if (!ttdir || !ttdir[0])
		return;
	for (i=1; TRUE; ++i) {
		char ttpath[MAXPATHLEN];
		llstrncpy(name, basename, sizeof(name));
		llstrappf(name, sizeof(name), "%d", i);
		ttname = getoptstr(name, "");
		if (!ttname || !ttname[0])
			break;
		/* user wishes to load translation table named ttname */
		llstrncpy(ttpath, concat_path(ttdir, ttname), sizeof(ttpath));
		load_user_charmap(ttpath, mapname, ttm);
	}
}
/*========================================
 * load_user_charmap -- Add an external user-specified
 *  custom translation table to the list
 *======================================*/
static void
load_user_charmap (CNSTRING ttpath, CNSTRING mapname, TRANMAPPING ttm)
{
	TRANTABLE tt=0;
	if (init_map_from_file(ttpath, mapname, &tt) && tt) {
		if (!ttm->global_trans)
			ttm->global_trans = create_list();
		enqueue_list(ttm->global_trans, tt);
	} else {
		if (tt)
			remove_trantable(tt);
	}
}
/*========================================
 * set_zone_conversion -- Set conversions for one zone
 *  based on user codeset option (if found)
 * zones are GUI, Editor, GEDCOM, and report
 * but we rely on caller to know the zones
 *======================================*/
static void
set_zone_conversion (STRING optname, INT toint, INT fromint)
{
	STRING extcs = getoptstr(optname, "");
	if (toint >= 0)
		trans_maps[toint].after = TRUE;
	if (fromint >= 0)
		trans_maps[fromint].after = FALSE;
	if (!extcs || !extcs[0] || !int_codeset || !int_codeset[0])
		return;
	if (toint >= 0) {
		trans_maps[toint].iconv_src = strdup(extcs);
		trans_maps[toint].iconv_dest = strdup(int_codeset);
	}
	if (fromint >= 0) {
		trans_maps[fromint].iconv_src = strdup(int_codeset);
		trans_maps[fromint].iconv_dest = strdup(extcs);
	}
}
/*========================================
 * load_char_mappings -- Reload all mappings
 *  (custom translation tables & iconv mappings)
 *======================================*/
void
load_char_mappings (void)
{
	/* TODO: make a clear_all_mappings for external call at shutdown */

	init_charmaps_if_needed();
	clear_char_mappings();
	load_custom_db_mappings();
	load_global_char_mapping();
}
/*========================================
 * load_custom_db_mappings -- Reload mappings embedded in
 *  current database
 *======================================*/
static void
load_custom_db_mappings (void)
{
	INT indx=-1;
	for (indx = 0; indx < NUM_TT_MAPS; indx++) {
		TRANMAPPING ttm = &trans_maps[indx];
		TRANTABLE *tt = &ttm->dbtrantbl;
		remove_trantable(*tt);
		*tt = 0;
		if (is_db_open()) {
			if (!init_map_from_rec(indx, tt)) {
				msg_error(_("Error initializing %s map.\n"), map_names[indx]);
			}
		}
	}
}
/*========================================
 * get_dbtrantable -- Access into the custom translation tables
 *======================================*/
TRANTABLE
get_dbtrantable (INT ttnum)
{
	return get_tranmapping(ttnum)->dbtrantbl;
}
/*========================================
 * get_dbtrantable_from_tranmapping -- Access into custom
 *  translation table of a tranmapping
 *======================================*/
TRANTABLE
get_dbtrantable_from_tranmapping (TRANMAPPING ttm)
{
	return ttm ? ttm->dbtrantbl : 0;
}
/*========================================
 * get_tranmapping -- Access to a translation mapping
 *======================================*/
TRANMAPPING
get_tranmapping (INT ttnum)
{
	ASSERT(ttnum>=0 && ttnum<ARRSIZE(trans_maps));
	return &trans_maps[ttnum];
}
/*========================================
 * set_dbtrantable -- Assign a new custom translation table
 *======================================*/
void
set_dbtrantable (INT ttnum, TRANTABLE tt)
{
	ASSERT(ttnum>=0 && ttnum<ARRSIZE(trans_maps));
	if (trans_maps[ttnum].dbtrantbl)
		remove_trantable(trans_maps[ttnum].dbtrantbl);
	trans_maps[ttnum].dbtrantbl = tt;
}
/*===================================================
 * init_map_from_rec -- Init single translation table
 *  indx:  [IN]  which translation table (see defn of map_keys)
 *  ptt:   [OUT] new translation table, if created
 * Returns FALSE if error
 * But if no tt found, *ptt=0 and returns TRUE.
 *=================================================*/
BOOLEAN
init_map_from_rec (INT indx, TRANTABLE * ptt)
{
	STRING rawrec;
	INT len;
	BOOLEAN ok;

	*ptt = 0;
	if (!(rawrec = retrieve_raw_record(map_keys[indx], &len)))
		return TRUE;
	ok = init_map_from_str(rawrec, map_names[indx], ptt);
	stdfree(rawrec);
	return ok;
}
/*====================================================
 * init_map_from_file -- Init single translation table
 *  file: [IN]  file from which to read translation table
 *  indx: [IN]  which translation table (see defn of map_keys)
 *  ptt:  [OUT] new translation table if created
 * Returns FALSE if error.
 * But if file is empty, *ptt=0 and returns TRUE.
 *==================================================*/
BOOLEAN
init_map_from_file (CNSTRING file, CNSTRING mapname, TRANTABLE * ptt)
{
	FILE *fp;
	struct stat buf;
	STRING mem;
	INT siz;
	BOOLEAN ok;

	*ptt = 0;

	if ((fp = fopen(file, LLREADTEXT)) == NULL) return TRUE;
	ASSERT(fstat(fileno(fp), &buf) == 0);
	if (buf.st_size == 0) {
		fclose(fp);
		return TRUE;
	}
	mem = (STRING) stdalloc(buf.st_size+1);
	mem[buf.st_size] = 0;
	siz = fread(mem, 1, buf.st_size, fp);
	/* may not read full buffer on Windows due to CR/LF translation */
	ASSERT(siz == buf.st_size || feof(fp));
	fclose(fp);
	ok = init_map_from_str(mem, mapname, ptt);
	stdfree(mem);
	return ok;
}
/*==================================================
 * init_map_from_str -- Init single tranlation table
 *
 * Blank lines or lines beginning with "##" are ignored
 * Translation table entries have the following foramt:
 *
 * <original>{sep}<translation>
 * sep is separator character, by default tab
 *  str:  [IN] input string to translate
 *  indx: [IN] which translation table (see defn of map_keys)
 *  ptt:  [OUT] new translation table if created
 * May return NULL
 *================================================*/
static BOOLEAN
init_map_from_str (STRING str, CNSTRING mapname, TRANTABLE * ptt)
{
	INT i, n, maxn, entry=1, line=1, newc;
	INT sep = (uchar)'\t'; /* default separator */
	BOOLEAN done;
	BOOLEAN skip;
	BOOLEAN ok = TRUE; /* return value */
	unsigned char c;
	char scratch[50];
	STRING p, *lefts, *rights;
	TRANTABLE tt=NULL;
	char name[sizeof(tt->name)];
	name[0] = 0;

	ASSERT(str);
	*ptt = 0;

/* Count newlines to find lefts and rights sizes */
	p = str;
	n = 0;
	skip = TRUE;
	/* first pass through, count # of entries */
	while (*p) {
		skip=FALSE;
		/* skip blank lines and lines beginning with "##" */
		if (*p == '\r' || *p == '\n') skip=TRUE;
		if (*p =='#' && p[1] == '#') skip=TRUE;
		if (skip) {
			while(*p && (*p != '\n'))
				p++;
			if(*p == '\n')
				p++;
			continue;
		}
		while(*p) {
			if (*p++ == '\n') {
				n++;
				break;
			}
		}
	}
	if (!skip) ++n; /* include last line */
	if (!n) {
		/* empty translation table ignored */
		goto none;
	}
	lefts = (STRING *) stdalloc(n*sizeof(STRING));
	rights = (STRING *) stdalloc(n*sizeof(STRING));
	for (i = 0; i < n; i++) {
		lefts[i] = NULL;
		rights[i] = NULL;
	}

/* Lex the string for patterns and replacements */
	done = FALSE;
	maxn = n;	/* don't exceed the entries you have allocated */
	n = 0;
	while (!done && (n < maxn)) {
		skip=FALSE;
		if (!*str) break;
		/* skip blank lines and lines beginning with "##" */
		if (*str == '\r' || *str == '\n') skip=TRUE;
		if (*str =='#' && str[1] == '#') {
			skip=TRUE;
			if (!strncmp(str, "##!sep", 6)) {
				/* new separator character if legal */
				if (str[6]=='=')
					sep='=';
			}
			if (!strncmp(str, "##!name: ",9)) {
				STRING p1=str+9, p2=name;
				INT i=sizeof(name);
				while (*p1 && *p1 != '\n' && --i)
					*p2++ = *p1++;
				*p2=0;
			}
		}
		if (skip) {
			while(*str && (*str != '\n'))
				str++;
			if (*str == '\n')
				str++;
			continue;
		}
		p = scratch;
		while (TRUE) {
			c = *str++;
			if (c == '#')  {
				newc = get_decimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 3;
				} else {
					maperror(mapname, entry, line, _(qSbaddec));
					goto fail;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					maperror(mapname, entry, line, _(qSbadhex));
					goto fail;
				}
			} else if ((c == '\n') || (c == '\r'))   {
				maperror(mapname, entry, line, _(qSnorplc));
				goto fail;
			} else if (c == 0) {
				maperror(mapname, entry, line, _(qSnorplc));
				goto fail;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n'
				    || c == '\r') {
					maperror(mapname, entry, line, _(qSbadesc));
					goto fail;
				}
				*p++ = c;
			} else if (c == sep)
				break;
			else
				*p++ = c;
		}
		*p = 0;
		if (!scratch[0]) {
				maperror(mapname, entry, line, _(qSnoorig));
				goto fail;
		}
		lefts[n] = strsave(scratch);
		p = scratch;
		while (TRUE) {
			c = *str++;
			if (c == '#')  {
				newc = get_decimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 3;
				} else {
					maperror(mapname, entry, line, _(qSbaddec));
					goto fail;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					maperror(mapname, entry, line, _(qSbadhex));
					goto fail;
				}
			} else if (c == '\n') {
				++line;
				++entry;
				break;
			} else if (c == 0) {
				done = TRUE;
				break;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n'
				    || c == '\r') {
					maperror(mapname, entry, line, _(qSbadesc));
					goto fail;
				}
				if (c == 't') c='\t'; /* "\t" -> tab */
				*p++ = c;
			} else if (c == '\t' || c == sep) {
				/* treat as beginning of a comment */
				while(*str && (*str != '\n'))
					str++;
				if(*str == '\n') {
					str++;
					line++;
					entry++;
				}
				break;
			} else if (c == '\r') {
				/* ignore (MSDOS has this before \n) */
			} else {
				/* not special, just copy replacement char */
				*p++ = c;
			}
		}
		*p = 0;
		rights[n++] = strsave(scratch);
	}
	*ptt = create_trantable(lefts, rights, n, name);
end:
	for (i = 0; i < n; i++)		/* don't free rights */
		stdfree(lefts[i]);
	stdfree(lefts);
	stdfree(rights);
none:
	return ok;

fail:
	for (i = 0; i < n; i++) /* rights not consumed by tt */
		stdfree(rights[i]);
	ok = FALSE;
	goto end;
}
/*==================================================
 * get_decimal -- Get decimal number from map string
 *================================================*/
INT
get_decimal (STRING str)
{
	INT value, c;
	if (chartype(c = (uchar)*str++) != DIGIT) return -1;
	value = c - '0';
	if (chartype(c = (uchar)*str++) != DIGIT) return -1;
	value = value*10 + c - '0';
	if (chartype(c = (uchar)*str++) != DIGIT) return -1;
	value = value*10 + c - '0';
	return (value >= 256) ? -1 : value;
}
/*==========================================================
 * get_hexidecimal -- Get hexidecimal number from map string
 *========================================================*/
INT
get_hexidecimal (STRING str)
{
	INT value, h;
	if ((h = hexvalue(*str++)) == -1) return -1;
	value = h;
	if ((h = hexvalue(*str++)) == -1) return -1;
	return value*16 + h;
}
/*================================================
 * hexvalue -- Find hexidecimal value of character
 *==============================================*/
INT
hexvalue (INT c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + c - 'a';
	if (c >= 'A' && c <= 'F') return 10 + c - 'A';
	return -1;
}
/*====================================================
 * maperror -- Print error message from reading string
 *  indx:   [in] which translation table
 *  entry:  [in] index of entry, 1-based
 *  line:   [in] raw line number in file, 1-based
 *  errmsg:  [in] error message
 *==================================================*/
static void
maperror (CNSTRING mapname, INT entry, INT line, STRING errmsg)
{
	llwprintf(_(qSmaperr), mapname, line, entry, errmsg);
}
CNSTRING
get_map_name (INT ttnum)
{
	ASSERT(ttnum>=0 && ttnum<NUM_TT_MAPS);
	return map_names[ttnum];
}