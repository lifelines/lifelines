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
	if (loaded_utf8) return TRUE;
	/*
	Load UnicodeData.txt file into uppers and lowers
	Use TTPATH to find it
	NB: actual load must be done in charmaps.c, where tag_trantable is
	set loaded_utf8 to +1 or -1
	*/
	loaded_utf8 = -1;
	return FALSE;
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
