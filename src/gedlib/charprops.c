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

/* 763 upper case letters, and 754 lower case letters, 2003-11-13 */
#define MAXCASES 1024

/*********************************************
 * local function prototypes
 *********************************************/

static ZSTR convert_utf8(TRANTABLE uppers, CNSTRING s);

/*********************************************
 * local variables
 *********************************************/

static int loaded_utf8 = 0; /* 1 if loaded, -1 if failed */
static int loaded_codepage = 0;
static TRANTABLE uppers = 0;
static TRANTABLE lowers = 0;
static STRING charset_name = 0; /* what charset is currently in charset_info */
static struct my_charset_info_tag * charset_info = 0;

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
	CNSTRING ttpath = getoptstr("TTPATH", ".");
	STRING upleft[MAXCASES], upright[MAXCASES], loleft[MAXCASES], loright[MAXCASES];
	INT upcount=0, locount=0;
	INT i;
	char filepath[MAXPATHLEN], line[MAXPATHLEN];

	if (loaded_utf8) return TRUE;
	
	loaded_utf8 = -1;
	strcpy(filepath, ttpath);
	strcat(filepath, LLSTRDIRSEPARATOR);
	strcat(filepath, "UnicodeData.txt");
	fp = fopen(filepath, "r");
	if (!fp)
		return FALSE;
	while (fgets(line, sizeof(line), fp)) {
		INT ch, chup, chlo;
		const char *ptr;
		if (1 != sscanf(line, "%x", &ch)) {
			continue;
		}
		ptr = line;
		chup = chlo = ch;
		for (i=0; i<13; ++i) {
			ptr = strchr(ptr, ';');
			if (!ptr)
				break;
			if (i==11) {
				if (1 != sscanf(ptr+1, "%x", &chup))
					chup = ch;
			} else if (i==12) {
				if (1 != sscanf(ptr+1, "%x", &chlo))
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
	lowers = create_trantable(loleft, loright, locount, "UTF-8 upper");

	for (i=0; i<upcount; ++i)
		stdfree(upleft[i]);
	for (i=0; i<locount; ++i)
		stdfree(loleft[i]);
	loaded_utf8 = 1;
	return TRUE;
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
	if (charset_info) {
		free(charset_info);
		charset_info = 0;
	}
	strfree(&charset_name);
	loaded_codepage = 0;
}
/*==========================================
 * charprops_load -- Load case tables for a single codepage
 *========================================*/
BOOLEAN
charprops_load (const char * codepage)
{
	/* check if already loaded */
	if (eqstr_ex(charset_name, codepage)) 
		return TRUE;

	loaded_codepage = -1;
	if (!loaded_utf8)
		charprops_load_utf8();
	if (loaded_utf8 != 1)
		return FALSE;
	/*
	#1) Check that we have UTF-8 charprops
	#2) See if we can convert to UTF-8
	#3) build the charset_info table
	#4) set loaded_codepage
	*/
	strupdate(&charset_name, codepage);
	return FALSE;
}
/*==========================================
 * charprops_toupperz -- Return uppercase version of string
 *========================================*/
ZSTR
charprops_toupperz (CNSTRING s, INT utf8)
{
	ZSTR zstr = 0;
	if (utf8) {
		if (loaded_utf8==1) {
			return convert_utf8(uppers, s);
		}
	} else {
		if (loaded_codepage==1) {
			/* TODO */
		}
	}
	return ll_toupperz(s, utf8);
}
/*==========================================
 * charprops_tolowerz -- Return lowercase version of string
 *========================================*/
ZSTR
charprops_tolowerz (CNSTRING s, INT utf8)
{
	ZSTR zstr = 0;
	if (utf8) {
		if (loaded_utf8==1) {
			return convert_utf8(lowers, s);
		}
	} else {
		if (loaded_codepage==1) {
			/* TODO */
		}
	}
	return ll_tolowerz(s, utf8);
}
/*==========================================
 * convert_utf8 -- translate string using specified trantable
 *========================================*/
static ZSTR
convert_utf8 (TRANTABLE uppers, CNSTRING s)
{
	ZSTR zstr = custom_translate(s, uppers);
	return zstr;
}
