/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*===========================================================
 * translat.c -- LifeLines character mapping functions
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   http://lifelines.sourceforge.net
 *=========================================================*/

#include <errno.h>
#include "llstdlib.h"
#include "btree.h"
#include "translat.h"
#include "xlat.h"
#include "codesets.h"
#include "gedcom.h"
#include "zstr.h"
#include "icvt.h"
#include "lloptions.h"
#include "gedcomi.h"
#include "arch.h" 


/*********************************************
 * global/exported variables
 *********************************************/

STRING illegal_char = 0;

/*********************************************
 * external/imported variables
 *********************************************/

extern BTREE BTR;

/*********************************************
 * local types
 *********************************************/

/* legacy (embedded) translation table */
struct legacytt_s {
	TRANTABLE tt;
	BOOLEAN first; /* comes at start of translation ? */
};
/* a predefined conversion, such as editor-to-internal */
struct conversion_s {
	INT trnum;
	CNSTRING key;
	CNSTRING name;
	INT zon_src;
	INT zon_dest;
	STRING * src_codeset;
	STRING * dest_codeset;
	XLAT xlat;
};
/* a predefined codeset, such as editor */
struct zone_s {
	INT znum;
	CNSTRING name;
};

/*********************************************
 * local enums & defines
 *********************************************/

enum { ZON_X, ZON_INT, ZON_GUI, ZON_EDI, ZON_RPT, ZON_GED, NUM_ZONES };

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void clear_legacy_tt(INT trnum);
static void clear_predefined_list(void);
static struct conversion_s * getconvert(INT trnum);
static BOOLEAN is_legacy_first(INT trnum);
static void local_init(void);


/*********************************************
 * local variables
 *********************************************/

struct zone_s zones[] = {
	{ ZON_X, "Invalid zone" }
	, { ZON_INT, "Internal codeset" }
	, { ZON_GUI, "Display codeset" }
	, { ZON_EDI, "Editor codeset" }
	, { ZON_RPT, "Report codeset" }
	, { ZON_GED, "GEDCOM codeset" }
};

/* These must be in enumeration order up to NUM_TT_MAPS */
static struct conversion_s conversions[] = {
	/* TRANSLATORS: Character set conversion from external editor to database internal */
	{ MEDIN, "MEDIN", N_("Editor to Internal"), ZON_EDI, ZON_INT, &editor_codeset_in, &int_codeset, 0 }
	/* TRANSLATORS: Character set conversion from database internal to external editor */
	, { MINED, "MINED", N_("Internal to Editor"), ZON_INT, ZON_EDI, &int_codeset, &editor_codeset_out, 0 }
	, { MGDIN, "MGDIN", N_("GEDCOM to Internal"), ZON_GED, ZON_INT, &gedcom_codeset_in, &int_codeset, 0 }
	, { MINGD, "MINGD", N_("Internal to GEDCOM"), ZON_INT, ZON_GED, &int_codeset, &gedcom_codeset_out, 0 }
	, { MDSIN, "MDSIN", N_("Display to Internal"), ZON_GUI, ZON_INT, &gui_codeset_in, &int_codeset, 0 }
	, { MINDS, "MINDS", N_("Internal to Display"), ZON_INT, ZON_GUI, &int_codeset, &gui_codeset_out, 0 }
	, { MRPIN, "MRPIN", N_("Report to Internal"), ZON_RPT, ZON_INT, &report_codeset_in, &int_codeset, 0 }
	, { MINRP, "MINRP", N_("Internal to Report"), ZON_INT, ZON_RPT, &int_codeset, &report_codeset_out, 0 }
	/* These are all special-purpose translation tables, and maybe shouldn't even be here ? */
	, { MSORT, "MSORT", "Custom Sort", ZON_X, ZON_X, 0, 0, 0 }
	, { MCHAR, "MCHAR", "Custom Charset", ZON_X, ZON_X, 0, 0, 0 }
	, { MLCAS, "MLCAS", "Custom Lowercase", ZON_X, ZON_X, 0, 0, 0 }
	, { MUCAS, "MUCAS", "Custom Uppercase", ZON_X, ZON_X, 0, 0, 0 }
	, { MPREF, "MPREF", "Custom Prefix", ZON_X, ZON_X, 0, 0, 0 }
};
static CNSTRING conversions_keys[] = {
	/* TRANSLATORS: key for "Editor to Internal" on translation table menu 
	Omit everything up to and including final | */
	N_("menu|trantable|e")
	/* TRANSLATORS: key for "Internal to Editor" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|m")
	/* TRANSLATORS: key for "GEDCOM to Internal" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|i")
	/* TRANSLATORS: key for "Internal to GEDCOM" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|x")
	/* TRANSLATORS: key for "Display to Internal" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|g")
	/* TRANSLATORS: key for "Internal to Display" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|d")
	/* TRANSLATORS: key for "Report to Internal" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|p")
	/* TRANSLATORS: key for "Internal to Report" on translation table menu
	Omit everything up to and including final | */
	, N_("menu|trantable|r")
};
/* currently loaded legacy (embedded) translation tables */
static struct legacytt_s legacytts[NUM_TT_MAPS]; /* initialized once by transl_init() */
static BOOLEAN inited=FALSE;


/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*===================================================
 * translate_catn -- Translate & concatenate string
 *
 * tt:    translation table to use
 * pdest: address of destination (will be advanced)
 * src:   source string
 * len:   address of space left in destination (will be decremented)
 *=================================================*/
void
translate_catn (XLAT ttm, STRING * pdest, CNSTRING src, INT * len)
{
	INT added;
	if (*len > 1)
		translate_string(ttm, src, *pdest, *len);
	else
		(*pdest)[0] = 0; /* to be safe */
	added = strlen(*pdest);
	*len -= added;
	*pdest += added;
}
/*===================================================
 * translate_string_to_zstring -- Translate string via TRANTABLE
 *  xlat: [IN]  translation to apply
 *  in:   [IN]  string to translate
 * Created: 2001/07/19 (Perry Rapp)
 * Copied from translate_string, except this version
 * uses dynamic buffer, so it can expand if necessary
 *=================================================*/
ZSTR
translate_string_to_zstring (XLAT xlat, CNSTRING in)
{
	ZSTR zstr = zs_news(in);
	transl_xlat(xlat, zstr);
	return zstr;
}
/*===================================================
 * translate_string -- Translate string via XLAT
 *  ttm:     [IN]  tranmapping
 *  in:      [IN]  in string
 *  out:     [OUT] string
 *  maxlen:  [OUT] max len of out string
 * Output string is limited to max length via use of
 * add_char & add_string.
 *=================================================*/
void
translate_string (XLAT ttm, CNSTRING in, STRING out, INT maxlen)
{
	ZSTR zstr=0;
	if (!in || !in[0]) {
		out[0] = 0;
		return;
	}
	zstr = translate_string_to_zstring(ttm, in);
	llstrsets(out, maxlen, uu8, zs_str(zstr));
	zs_free(&zstr);
}
/*==========================================================
 * translate_write -- Translate and output lines in a buffer
 *  tt:   [in] translation table (may be NULL)
 *  in:   [in] input string to write
 *  lenp: [in,out] #characters left in buffer (set to 0 if a full write)
 *  ofp:  [in] output file
 *  last: [in] flag to write final line if no trailing \n
 * Loops thru & prints out lines until end of string
 *  (or until last line if not terminated with \n)
 * *lenp will be set to zero unless there is a final line
 * not terminated by \n and caller didn't ask to write it anyway
 * NB: If no translation table, entire string is always written
 *========================================================*/
BOOLEAN
translate_write(XLAT ttm, STRING in, INT *lenp
	, FILE *ofp, BOOLEAN last)
{
	char intmp[MAXLINELEN+2];
	char out[MAXLINELEN+2];
	char *tp;
	char *bp;
	int i,j;

	if(ttm == NULL) {
	    ASSERT(fwrite(in, *lenp, 1, ofp) == 1);
	    *lenp = 0;
	    return TRUE;
	}

	bp = (char *)in;
	/* loop through lines one by one */
	for(i = 0; i < *lenp; ) {
		int outbytes;
		/* copy in to intmp, up to first \n or our buffer size-1 */
		tp = intmp;
		for(j = 0; (j <= MAXLINELEN) && (i < *lenp) && (*bp != '\n'); j++) {
			i++;
			*tp++ = *bp++;
		}
		*tp = '\0';
		if(i < *lenp) {
			/* partial, either a single line or a single buffer full */
			if(*bp == '\n') {
				/* single line, include the \n */
				/* it is important that we limited size earlier so we
				have room here to add one more character */
				*tp++ = *bp++;
				*tp = '\0';
				i++;
			}
		}
		else if(!last) {
			/* the last line is not complete, return it in buffer  */
			strcpy(in, intmp);
			*lenp = strlen(in);
			return(TRUE);
		}
		/* translate & write out current line */
		/* TODO (2002-11-28): modify to use dynamic string */
		translate_string(ttm, intmp, out, MAXLINELEN+2);
		if (out && strlen(out)) {
			outbytes = fwrite(out, 1, strlen(out), ofp);
			if (!outbytes || ferror(ofp)) {
				crashlog("outbytes=%d, errno=%d, outstr=%s"
					, outbytes, errno, out);
				FATAL();
			}
		}
	}
	*lenp = 0;
	return(TRUE);
}
/*==========================================================
 * get_xlat_to_int -- Get translation to internal codeset
 *  returns NULL if fails
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
XLAT
transl_get_xlat_to_int (CNSTRING codeset)
{
	BOOLEAN adhoc = TRUE;
	return xl_get_xlat(codeset, int_codeset, adhoc);
}
/*==========================================================
 * transl_get_xlat -- Get arbitrary translator
 *  returns NULL if fails
 * Created: 2002/11/30 (Perry Rapp)
 *========================================================*/
XLAT
transl_get_xlat (CNSTRING src, CNSTRING dest)
{
	BOOLEAN adhoc = TRUE;
	return xl_get_xlat(src, dest, adhoc);
}
/*==========================================================
 * transl_load_all_tts -- Load internal list of available translation
 *  tables (based on *.tt files in TTPATH)
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
void
transl_load_all_tts (void)
{
	CNSTRING ttpath = getlloptstr("TTPATH", ".");
	if (!inited) local_init();
	xl_load_all_dyntts(ttpath);
}
/*==========================================================
 * transl_xlat -- Perform a translation on a string
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
void
transl_xlat (XLAT xlat, ZSTR zstr)
{
	INT index = xl_get_uparam(xlat)-1;
	struct legacytt_s * legtt = (index>=0 ? &legacytts[index] : NULL);
	if (legtt && legtt->tt && legtt->first) {
		custom_translatez(zstr, legtt->tt);
	}

	xl_do_xlat(xlat, zstr);

	if (legtt && legtt->tt && !legtt->first) {
		custom_translatez(zstr, legtt->tt);
	}
}
/*==========================================================
 * transl_init -- One-time initialization of this module
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
static void
local_init (void)
{
	INT i;

	ASSERT(NUM_TT_MAPS == ARRSIZE(conversions));
	ASSERT(NUM_ZONES == ARRSIZE(zones));

	for (i=0; i<NUM_TT_MAPS; ++i)
		legacytts[i].tt = 0;
	inited=TRUE;
}
/*==========================================================
 * transl_load_xlats -- Load translations for all regular codesets
 *  (internal, GUI, ...)
 * returns FALSE if needed conversions not available
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
void
transl_load_xlats (void)
{
	INT i;

	if (!inited) local_init();

	clear_predefined_list();

	for (i=0; i<NUM_TT_MAPS; ++i) {
		struct conversion_s * conv = getconvert(i);
		STRING src, dest;
		BOOLEAN adhoc = FALSE;
		ASSERT(conv->trnum == i);
		if (conv->src_codeset && int_codeset) {
			ASSERT(conv->dest_codeset);
			src = *conv->src_codeset;
			dest = *conv->dest_codeset;
			conv->xlat = xl_get_xlat(src, dest, adhoc);
		} else {
			/* even if codesets are unspecified, have to have a placeholder
			in which to store any legacy translation tables */
			conv->xlat = xl_get_null_xlat();
		}
		/* 2003-09-09, Perry
		I just added this today quickly today to get legacy translations 
		working; this is kind of confusing and ought to be cleaned
		up */
		xl_set_uparam(conv->xlat, 0);
		if (BTR) {
			TRANTABLE tt=0;
			if (init_map_from_rec(conv->key, i, &tt) && tt) {
				transl_set_legacy_tt(i, tt);
			}
			xl_set_uparam(conv->xlat, i+1);
		}
	}
}
/*==========================================================
 * is_legacy_first -- Should this legacy come before rest of translation ?
 * This is to make legacy tts run in internal codeset
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
static BOOLEAN
is_legacy_first (INT trnum)
{
	return (getconvert(trnum)->src_codeset == &int_codeset);
}
/*==========================================================
 * clear_predefined_list -- Free cached regular conversions
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
static void
clear_predefined_list (void)
{
	INT i;
	for (i=0; i<NUM_TT_MAPS; ++i) {
		getconvert(i)->xlat = 0; /* pointer into xlat.c cache, so we don't free it */
		clear_legacy_tt(i);
	}
}
/*==========================================================
 * transl_get_predefined_xlat -- Fetch a predefined translation
 *  eg, MEDIN (editor-to-internal)
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
XLAT
transl_get_predefined_xlat (INT trnum)
{
	return getconvert(trnum)->xlat;
}
/*==========================================================
 * transl_get_predefined_name -- Fetch name of a predefined translation
 *  eg, transl_get_predefined_name(MEDIN) == "editor-to-internal"
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
ZSTR
transl_get_predefined_name (INT trnum)
{
	return zs_news(_(getconvert(trnum)->name));
}
/*==========================================================
 * sgettext -- Version of gettext that strips out menu leaders
 * (menu leaders are everything up to last |)
 *========================================================*/
static const char *
sgettext (const char *msgid)
{
	char *msgval = _(msgid);
	if (msgval == msgid)
		msgval = strrchr (msgid, '|') + 1;
	return msgval;
}
/*==========================================================
 * transl_get_predefined_menukey -- Menu key for predefined translation
 * (localized)
 *========================================================*/
ZSTR
transl_get_predefined_menukey (INT trnum)
{
	ASSERT(trnum>=0);
	ASSERT(trnum<ARRSIZE(conversions_keys));
	return zs_news(sgettext(conversions_keys[trnum]));
}
/*==========================================================
 * transl_get_description -- Fetch description of a translation
 *  eg, "3 steps with iconv(UTF-8, CP1252)"
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
ZSTR
transl_get_description (XLAT xlat)
{
	ZSTR zstr = xlat_get_description(xlat);
	INT index = xl_get_uparam(xlat)-1;
	struct legacytt_s * legtt = (index>=0 ? &legacytts[index] : NULL);
	if (legtt && legtt->tt) {
		ZSTR zdesc = get_trantable_desc(legtt->tt);
		/* TRANSLATORS: db internal translation table note for tt menu */
		zs_appf(zstr, _(" (dbint tt: %s)"), zs_str(zdesc));
		zs_free(&zdesc);
	}

	return zstr;
}
/*==========================================================
 * transl_parse_codeset -- Parse out subcode suffixes of a codeset
 *  eg, "CP437//TrGreekAscii//TrCyrillicAscii"
 *  will recognize CP437 as the codeset name, and list
 *  "TrGreekAscii" and "TrCyrillicAscii"  as subcodes
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
void
transl_parse_codeset (CNSTRING codeset, ZSTR zcsname, LIST * subcodes)
{
	xl_parse_codeset(codeset, zcsname, subcodes);
}
/*==========================================================
 * transl_are_all_conversions_ok -- 
 *  return FALSE if there any conversions we couldn't figure out
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
BOOLEAN
transl_are_all_conversions_ok (void)
{
	INT i;
	for (i=0; i<NUM_TT_MAPS; ++i) {
		if (conversions[i].src_codeset && !conversions[i].xlat)
			return FALSE;
	}
	return TRUE;
}
/*==========================================================
 * getconvert -- return conversion for trnum
 * Simply a wrapper for ASSERT to check validity of trnum
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
static struct conversion_s *
getconvert (INT trnum)
{
	ASSERT(trnum>=0);
	ASSERT(trnum<NUM_TT_MAPS);
	return &conversions[trnum];
}
/*==========================================================
 * transl_has_legacy_tt -- Is there a legacy (in-database)
 *  translation table for this entry ?
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
TRANTABLE
transl_get_legacy_tt (INT trnum)
{
	getconvert(trnum); /* check validity of trnum */
	return legacytts[trnum].tt;
}
/*==========================================================
 * transl_has_legacy_tt -- Is there a legacy (in-database)
 *  translation table for this entry ?
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
void
transl_set_legacy_tt (INT trnum, TRANTABLE tt)
{
	struct legacytt_s * leg; 
	clear_legacy_tt(trnum); /* ensures trnum validity */
	getconvert(trnum); /* check validity of trnum */
	leg = &legacytts[trnum];
	leg->tt = tt;
	leg->first = is_legacy_first(trnum);
}
/*==========================================================
 * clear_legacy_tt -- Remove this legacy tt if loaded
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
static void
clear_legacy_tt (INT trnum)
{
	struct legacytt_s * leg; 
	getconvert(trnum); /* check validity of trnum */
	leg = &legacytts[trnum];
	if (leg->tt) {
		remove_trantable(leg->tt);
		leg->tt = 0;
	}
}
/*==========================================================
 * transl_free_predefined_xlats -- Free all our predefined
 *  translations; this is called when a database is closed
 * Created: 2002/12/13 (Perry Rapp)
 *========================================================*/
void
transl_free_predefined_xlats (void)
{
	clear_predefined_list();
}
/*==========================================================
 * transl_is_xlat_valid -- Does it do the job ?
 * Created: 2002/12/15 (Perry Rapp)
 *========================================================*/
BOOLEAN
transl_is_xlat_valid (XLAT xlat)
{
	return xl_is_xlat_valid(xlat);
}
/*==========================================================
 * transl_get_map_name -- get name of translation
 * eg, "Editor to Internal"
 * (localized)
 *========================================================*/
CNSTRING
transl_get_map_name (INT trnum)
{
	ASSERT(trnum>=0);
	ASSERT(trnum<NUM_TT_MAPS);
	return _(getconvert(trnum)->name);
}
/*==========================================================
 * transl_release_xlat -- Client finished with this
 * Created: 2002/12/15 (Perry Rapp)
 *========================================================*/
void
transl_release_xlat (XLAT xlat)
{
	xl_release_xlat(xlat);
}

