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
	/*
	TODO
	free uppers & lowers
	*/
	loaded_utf8 = 0;
}
/*==========================================
 * charprops_load -- Load case tables for a single codepage
 *========================================*/
BOOLEAN
charprops_load (const char * codepage)
{
	loaded_codepage = -1;
	charprops_load_utf8();
	if (loaded_utf8 != 1) return FALSE;
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
	return zstr;
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
