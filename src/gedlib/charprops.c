/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*==========================================================
 * charprops.c -- Build case tables using external UnicodeData.txt file
 *========================================================*/

#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */
#include "charprops.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "lloptions.h"
#include "mychar.h"
#include "zstr.h"

/* 763 upper case letters, and 754 lower case letters, 2003-11-13 */
#define MAXCASES 1024

/*********************************************
 * external/imported variables
 *********************************************/
extern BOOLEAN opt_mychar;

/*********************************************
 * local function prototypes
 *********************************************/

static ZSTR convert_utf8(TRANTABLE tt, CNSTRING s);
static ZSTR charprops_toupperz(CNSTRING s);
static ZSTR charprops_tolowerz(CNSTRING s);

/*********************************************
 * local variables
 *********************************************/

static int loaded_utf8 = 0; /* 1 if loaded, -1 if failed */
static int loaded_codepage = 0;
static TRANTABLE uppers = 0;
static TRANTABLE lowers = 0;
static STRING charset_name = 0; /* what charset is currently in charset_info */
static struct my_charset_info_tag charset_info[256];

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==========================================
 * charprops_load_utf8 -- Load case tables for full UTF-8
 *========================================*/
BOOLEAN
charprops_load_utf8 (void)
{
	FILE * fp=0;
	CNSTRING ttpath = getlloptstr("TTPATH", ".");
	STRING upleft[MAXCASES], upright[MAXCASES], loleft[MAXCASES], loright[MAXCASES];
	INT upcount=0, locount=0;
	INT i;
	char filepath[MAXPATHLEN], line[MAXPATHLEN];

	if (loaded_utf8) return TRUE;
	
	loaded_utf8 = -1;
	concat_path(ttpath, "UnicodeDataExcerpt.txt", uu8, filepath, sizeof(filepath));
	fp = fopen(filepath, LLREADTEXT);
	if (!fp)
		return FALSE;
	while (fgets(line, sizeof(line), fp)) {
		INT ch, chup, chlo;
		unsigned int uich;
		const char *ptr;
		if (line[0] == '#')
			continue;
		if (1 != sscanf(line, "%x", &uich)) {
			continue;
		}
		ch = uich;
		ptr = line;
		chup = chlo = ch;
		for (i=0; i<13; ++i) {
			ptr = strchr(ptr, ';');
			if (!ptr)
				break;
			if (i==11) {
				if (1 == sscanf(ptr+1, "%x", &uich))
					chup = uich;
				else
					chup = ch;
			} else if (i==12) {
				if (1 == sscanf(ptr+1, "%x", &uich))
					chlo = uich;
				else
					chlo = ch;
			}
			++ptr;
		}
		if (chup != ch && upcount<MAXCASES) {
			char chleft[8], chright[8];
			unicode_to_utf8(ch, chleft);
			unicode_to_utf8(chup, chright);
			upleft[upcount] = strsave(chleft);
			upright[upcount] = strsave(chright);
			++upcount;
		}
		if (chlo != ch && upcount<MAXCASES) {
			char chleft[8], chright[8];
			unicode_to_utf8(ch, chleft);
			unicode_to_utf8(chlo, chright);
			loleft[locount] = strsave(chleft);
			loright[locount] = strsave(chright);
			++locount;
		}
		if (i != 13)
			continue;
	}
	fclose(fp);

	uppers = create_trantable(upleft, upright, upcount, "UTF-8 upper");
	lowers = create_trantable(loleft, loright, locount, "UTF-8 lower");

	for (i=0; i<upcount; ++i)
		stdfree(upleft[i]);
	for (i=0; i<locount; ++i)
		stdfree(loleft[i]);
	loaded_utf8 = 1;
	set_utf8_casing(charprops_toupperz, charprops_tolowerz);
	return TRUE;
}
/*==========================================
 * charprops_is_loaded -- Return 1 if UnicodeData.txt file was loaded
 *  (meaning we have our own uppercasing translation tables)
 *========================================*/
BOOLEAN
charprops_is_loaded (void)
{
	return (loaded_utf8 > 0);
}
/*==========================================
 * charprops_free_all -- Free all allocated resources
 *========================================*/
void
charprops_free_all (void)
{
	if (uppers) {
		remove_trantable(uppers);
		uppers = 0;
	}
	if (lowers) {
		remove_trantable(lowers);
		lowers = 0;
	}
	loaded_utf8 = 0;
	strfree(&charset_name);
	loaded_codepage = 0;
}
/*==========================================
 * charprops_load -- Load case tables for a single codepage
 *========================================*/
BOOLEAN
charprops_load (const char * codepage)
{
	XLAT tt8=0, ttback = 0;
	INT ch;
	char src[2];

	/* check if already loaded */
	if (eqstr_ex(charset_name, codepage)) 
		return TRUE;

	/* assume that we don't have a table, in case we bail */
	loaded_codepage = -1;
	opt_mychar = 0;

	/* check that we have UTF-8 */
	if (!loaded_utf8)
		charprops_load_utf8();
	if (loaded_utf8 != 1)
		return FALSE;

	/* can we get from desired codepage to UTF-8 ? */
	tt8 = transl_get_xlat(codepage, "UTF-8");
	if (!tt8)
		return FALSE;
	/* and back ? */
	ttback = transl_get_xlat("UTF-8", codepage);
	if (!ttback)
		return FALSE;

	src[1] = 0;
	for (ch=0; ch<256; ++ch) {
		ZSTR zsrc, zup, zdn;

		/* set default as noncased */
		charset_info[ch].toup = ch;
		charset_info[ch].tolow = ch;
		charset_info[ch].iscntrl = 0;
		charset_info[ch].isup = 0;
		charset_info[ch].islow = 0;
		src[0] = ch;
		/*
		Convert to UTF-8, do casing, check if it changed
		if so, convert back, check that result is single char
		different from ch, if so, save it
		*/
		zsrc = translate_string_to_zstring(tt8, src);
		/* zstr is UTF-8 version of ch */
		zup = custom_translate(zs_str(zsrc), uppers);
		/* zup is uppercased zstr */
		if (!eqstr(zs_str(zsrc), zs_str(zup))) {
			transl_xlat(ttback, zup);
			/* zup is now uppercased in original codepage */
			if (zs_len(zup) == 1 && zs_str(zup)[0] != ch) {
				charset_info[ch].islow = 1;
				charset_info[ch].toup = zs_str(zup)[0];
			}
		}
		zs_free(&zup);
		zdn = custom_translate(zs_str(zsrc), lowers);
		/* zdn is lowercased zstr */
		if (!eqstr(zs_str(zsrc), zs_str(zdn))) {
			transl_xlat(ttback, zdn);
			/* zdn is now lowercased in original codepage */
			if (zs_len(zdn) == 1 && zs_str(zdn)[0] != ch) {
				charset_info[ch].isup = 1;
				charset_info[ch].tolow = zs_str(zdn)[0];
			}
		}
		zs_free(&zdn);
		zs_free(&zsrc);
	}

	/* activate new table of character properties */
	mych_set_table(charset_info);
	opt_mychar = 1;

	strupdate(&charset_name, codepage);
	return TRUE;
}
/*==========================================
 * charprops_toupperz -- Return uppercase version of string
 * Only used when internal codeset is UTF-8
 * This is called as a plugin from ll_toupperz
 *========================================*/
static ZSTR
charprops_toupperz (CNSTRING s)
{
	ASSERT(uu8);
	ASSERT(loaded_utf8==1);
	return convert_utf8(uppers, s);
}
/*==========================================
 * charprops_tolowerz -- Return lowercase version of string
 * Only used when internal codeset is UTF-8
 * This is called as a plugin from ll_toupperz
 *========================================*/
static ZSTR
charprops_tolowerz (CNSTRING s)
{
	ASSERT(uu8);
	ASSERT(loaded_utf8==1);
	return convert_utf8(lowers, s);
}
/*==========================================
 * convert_utf8 -- translate string using specified trantable
 *========================================*/
static ZSTR
convert_utf8 (TRANTABLE tt, CNSTRING s)
{
	ZSTR zstr = custom_translate(s, tt);
	return zstr;
}
