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

#include "llstdlib.h"
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

/* TODO: need to delete map_names in favor of new system */
CNSTRING map_names[] = {
	"Editor to Internal"
	,"Internal to Editor"
	,"GEDCOM to Internal"
	,"Internal to GEDCOM"
	,"Display to Internal"
	,"Internal to Display"
	,"Report to Internal"
	,"Internal to Report"
	,"Custom Sort"
	,"Custom Charset"
	,"Custom Lowercase"
	,"Custom Uppercase"
	,"Custom Prefix"
};

/*********************************************
 * local types
 *********************************************/

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
	TRANTABLE tt_legacy;
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
static void global_translate(ZSTR * pzstr, LIST gtlist);
static ZSTR iconv_trans_ttm(XLAT ttm, ZSTR zin, CNSTRING illegal);
static void load_conv_array(void);
static void free_xlat_ptrs(void);


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

struct conversion_s conversions[] = {
	{ MEDIN, "MEDIN", "Editor to Internal", ZON_EDI, ZON_INT, &editor_codeset_in, &int_codeset, 0, 0 }
	, { MINED, "MINED", "Internal to Editor", ZON_INT, ZON_EDI, &int_codeset, &editor_codeset_out, 0, 0 }
	, { MGDIN, "MGDIN", "GEDCOM to Internal", ZON_GED, ZON_INT, &gedcom_codeset_in, &int_codeset, 0, 0 }
	, { MINGD, "MINGD", "Internal to GEDCOM", ZON_INT, ZON_GED, &int_codeset, &gedcom_codeset_out, 0, 0 }
	, { MDSIN, "MDSIN", "Display to Internal", ZON_GUI, ZON_INT, &gui_codeset_in, &int_codeset, 0, 0 }
	, { MINDS, "MINDS", "Internal to Display", ZON_INT, ZON_GUI, &int_codeset, &gui_codeset_out, 0, 0 }
	, { MRPIN, "MRPIN", "Report to Internal ", ZON_RPT, ZON_INT, &report_codeset_in, &int_codeset, 0, 0 }
	, { MINRP, "MINRP", "Internal to Report", ZON_INT, ZON_RPT, &int_codeset, &report_codeset_out, 0, 0 }
	/* These are all special-purpose translation tables, and maybe shouldn't even be here ? */
	, { MSORT, "MSORT", "Custom Sort", ZON_X, ZON_X, 0, 0, 0, 0 }
	, { MCHAR, "MCHAR", "Custom Charset", ZON_X, ZON_X, 0, 0, 0, 0 }
	, { MLCAS, "MLCAS", "Custom Lowercase", ZON_X, ZON_X, 0, 0, 0, 0 }
	, { MUCAS, "MUCAS", "Custom Uppercase", ZON_X, ZON_X, 0, 0, 0, 0 }
	, { MPREF, "MPREF", "Custom Prefix", ZON_X, ZON_X, 0, 0, 0, 0 }
};



static INT conv_array[NUM_TT_MAPS];


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
 *  ttm: [IN]  tranmapping to apply
 *  in:  [IN]  string to translate
 * Created: 2001/07/19 (Perry Rapp)
 * Copied from translate_string, except this version
 * uses dynamic buffer, so it can expand if necessary
 *=================================================*/
ZSTR
translate_string_to_zstring (XLAT ttm, CNSTRING in)
{
	/*
	TODO: This must go away with the new system
	2002-11-28
	Just call xlat_do_xlat(), no ?
	*/
	TRANTABLE ttdb = get_dbtrantable_from_tranmapping(ttm);
	ZSTR zout = zs_newn((unsigned int)(strlen(in)*1.3+2));
	zs_sets(&zout, in);
	if (!in || !ttm) {
		return zout;
	}
	if (!ttm->after) {
		if (ttdb) {
			/* custom translation before iconv */
			custom_translate(&zout, ttdb);
		}
		if (ttm->global_trans) {
			global_translate(&zout, ttm->global_trans);
		}
	}
	if (ttm->iconv_src && ttm->iconv_src[0] 
		&& ttm->iconv_dest && ttm->iconv_dest[0]) {
		zout = iconv_trans_ttm(ttm, zout, illegal_char);
	}
	if (ttm->after) {
		if (ttm->global_trans) {
			global_translate(&zout, ttm->global_trans);
		}
		if (ttdb) {
			/* custom translation after iconv */
			custom_translate(&zout, ttdb);
		}
	}
	return zout;
}
/*===================================================
 * iconv_trans_ttm -- Translate string via iconv  & transmapping
 *  ttm:  [IN]   transmapping
 *  in:   [IN]   string to translate (& delete)
 *  zin:  [I/O]  input buffer (may be returned if iconv fails)
 * Only called if HAVE_ICONV
 *=================================================*/
static ZSTR
iconv_trans_ttm (XLAT ttm, ZSTR zin, CNSTRING illegal)
{
	/* 
	TODO: 2002-11-28
	Move to xlat for use with new system
	*/
	CNSTRING dest=ttm->iconv_dest;
	CNSTRING src = ttm->iconv_src;
	ZSTR zout=0;
	ASSERT(dest && src);
	if (!iconv_trans(src, dest, zs_str(zin), &zout, illegal)) {
		/* if invalid translation, clear it to avoid trying again */
		strfree(&ttm->iconv_src);
		strfree(&ttm->iconv_dest);
	}
	if (!zout)
		zs_setz(&zout, zin);
	return zout;
}
/*===================================================
 * global_translate -- Apply list of user global transforms
 *=================================================*/
static void
global_translate (ZSTR * pzstr, LIST gtlist)
{
	TRANTABLE ttx=0;
	FORLIST(gtlist, tbel)
		ttx = tbel;
		ASSERT(ttx);
		custom_translate(pzstr, ttx);
		ttx = 0;
	ENDLIST
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
		ASSERT(fwrite(out, strlen(out), 1, ofp) == 1);
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
	CNSTRING ttpath = getoptstr("TTPATH", ".");
	xl_load_all_tts(ttpath);
}
/*==========================================================
 * xl_do_xlat -- Perform a translation on a string
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
BOOLEAN
transl_xlat (XLAT xlat, ZSTR * pzstr)
{
	return xl_do_xlat(xlat, pzstr);
}
/*==========================================================
 * transl_load_xlats -- Load translations for all regular codesets
 *  (internal, GUI, ...)
 * returns FALSE if needed conversions not available
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
void
transl_load_xlats (BOOLEAN indb)
{
	INT i;

	ASSERT(NUM_TT_MAPS == ARRSIZE(map_names));
	ASSERT(NUM_TT_MAPS == ARRSIZE(conversions));
	ASSERT(NUM_ZONES == ARRSIZE(zones));
	ASSERT(NUM_TT_MAPS == ARRSIZE(conv_array));

	load_conv_array();

	free_xlat_ptrs();

	if (!int_codeset)
		return;

	for (i=0; i<NUM_TT_MAPS; ++i) {
		STRING src, dest;
		BOOLEAN adhoc = FALSE;
		ASSERT(conversions[i].trnum == i);
		if (!conversions[i].src_codeset)
			continue;
		ASSERT(conversions[i].dest_codeset);
		src = *conversions[i].src_codeset;
		dest = *conversions[i].dest_codeset;
		conversions[i].xlat = xl_get_xlat(src, dest, adhoc);
		if (indb)
			init_map_from_rec(conversions[i].key, i, &conversions->tt_legacy);
	}
}
/*==========================================================
 * free_xlat_ptrs -- Free cached regular conversions
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
static void
free_xlat_ptrs (void)
{
	INT i;
	for (i=0; i<NUM_TT_MAPS; ++i) {
		/*
		xlat actually lives in xlat cache over in xlat.c.
		we just zero our pointer to it.
		*/
		conversions[i].xlat = 0;
	}
}
/*==========================================================
 * load_conv_array -- Load up conv_array
 *  it is used to map a trnum to a slot in the conversions array
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
static void
load_conv_array (void)
{
	INT i;
	for (i=0; i<NUM_TT_MAPS; ++i) {
		conv_array[i] = -1;
	}
	for (i=0; i<NUM_TT_MAPS; ++i) {
		INT trnum = conversions[i].trnum;
		ASSERT(trnum>=0 && trnum<NUM_TT_MAPS);
		ASSERT(conv_array[trnum]==-1);
		conv_array[trnum]=i;
	}
	for (i=0; i<NUM_TT_MAPS; ++i) {
		ASSERT(conv_array[i] >= 0);
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
	struct conversion_s * conv;
	ASSERT(trnum>=0 && trnum<NUM_TT_MAPS);
	conv = &conversions[conv_array[trnum]];
	return conv->xlat;
}
/*==========================================================
 * transl_parse_codeset -- Parse out subcode suffixes of a codeset
 *  eg, "CP437//TrGreekAscii//TrCyrillicAscii"
 *  will recognize CP437 as the codeset name, and list
 *  "TrGreekAscii" and "TrCyrillicAscii"  as subcodes
 * Created: 2002/11/28 (Perry Rapp)
 *========================================================*/
void
transl_parse_codeset (CNSTRING codeset, ZSTR * zcsname, LIST * subcodes)
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
