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
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_LANGINFO_CODESET
# include <langinfo.h>
#else
# include "langinfz.h"
#endif
#include "translat.h"
#include "liflines.h"
#include "feedback.h"
#include "zstr.h"
#include "icvt.h"
#include "lloptions.h"
#include "gedcomi.h"

#ifdef max
#	undef max
#endif

/*********************************************
 * global/exported variables
 *********************************************/

STRING illegal_char = 0;

/*********************************************
 * local types
 *********************************************/

/* step of a translation, either iconv_src or trantble is NULL */
struct xlat_step_s {
	CNSTRING iconv_src;
	CNSTRING iconv_dest;
	TRANTABLE trantbl;
};

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void global_translate(ZSTR * pzstr, LIST gtlist);
static XLAT get_xlat(CNSTRING src, CNSTRING dest);
static ZSTR iconv_trans_ttm(XLAT ttm, ZSTR zin, CNSTRING illegal);

/*********************************************
 * local variables
 *********************************************/

static LIST f_xlats=0;

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
 *  ttm:   [IN]  tranmapping
 *  in:    [IN]  in string
 *  out:   [OUT] string
 *  max:   [OUT] max len of out string
 * Output string is limited to max length via use of
 * add_char & add_string.
 *=================================================*/
void
translate_string (XLAT ttm, CNSTRING in, STRING out, INT max)
{
	ZSTR zstr=0;
	if (!in || !in[0]) {
		out[0] = 0;
		return;
	}
	zstr = translate_string_to_zstring(ttm, in);
	llstrncpy(out, zs_str(zstr), max, uu8);
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
		translate_string(ttm, intmp, out, MAXLINELEN+2);
		ASSERT(fwrite(out, strlen(out), 1, ofp) == 1);
	}
	*lenp = 0;
	return(TRUE);
}
/*==========================================================
 * get_xlat_to_int -- Get translation to internal codeset
 *  returns NULL if fails
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
XLAT
get_xlat_to_int (CNSTRING codeset)
{
	return get_xlat(codeset, int_codeset);
}
/*==========================================================
 * create_xlat -- Create a new translation
 * (also adds to cache)
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
static XLAT
create_xlat (CNSTRING src, CNSTRING dest)
{
	/* create & initialize new xlat */
	XLAT xlat = (XLAT)malloc(sizeof(*xlat));
	memset(xlat, 0, sizeof(*xlat));
	xlat->steps = create_list();
	xlat->src = strsave(src);
	xlat->dest = strsave(dest);
	/* add xlat to cache */
	if (!f_xlats) {
		f_xlats = create_list();
	}
	enqueue_list(f_xlats, xlat);
	return xlat;
}
/*==========================================================
 * get_xlat -- Find translation between specified codesets
 *  returns NULL if fails
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
static XLAT
get_xlat (CNSTRING src, CNSTRING dest)
{
	XLAT xlat=0;
	
	if (!src || !src[0] || !dest || !dest[0])
		return 0;

	/* first check existing cache */
	if (f_xlats) {
		XLAT xlattemp;
		FORLIST(f_xlats, el)
			xlattemp = (XLAT)el;
			if (eqstr(xlattemp->src, src) && eqstr(xlattemp->dest, dest)) {
				return xlattemp;
			}
		ENDLIST
	}
	/* check if identity */
	if (eqstr(src, dest)) {
		/* new empty xlat will work for identity */
		return create_xlat(src, dest);
	}
	/*
	TODO: 2002-11-25
	check table of conversions in ttdir
	*/
	if (iconv_can_trans(src, dest)) {
		struct xlat_step_s * xstep = 0;
		/* create new xlat & fill it out */
		xlat = create_xlat(src, dest);
		/* create a single iconv step & fill it out*/
		xstep = (struct xlat_step_s * )malloc(sizeof(*xstep));
		xstep->iconv_dest = strsave(dest);
		xstep->iconv_src = strsave(src);
		xstep->trantbl = NULL;
		/* put single iconv step into xlat */
		enqueue_list(xlat->steps, xstep);
		return xlat;
	}
	return xlat;
}
/*==========================================================
 * do_xlat -- Perform a translation on a string
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
BOOLEAN
do_xlat (XLAT xlat, ZSTR * pzstr)
{
	BOOLEAN cvtd=FALSE;
	struct xlat_step_s * xstep=0;
	if (!xlat) return cvtd;
	/* simply cycle through & perform each step */
	FORLIST(xlat->steps, el)
		xstep = (struct xlat_step_s *)el;
		if (xstep->iconv_src) {
			/* an iconv step */
			ZSTR ztemp=0;
			if (iconv_trans(xstep->iconv_src, xstep->iconv_dest, zs_str(*pzstr), &ztemp, "?")) {
				cvtd=TRUE;
				zs_free(pzstr);
				*pzstr = ztemp;
			} else { /* iconv failed, so clear it to avoid trying again later */
				xstep->iconv_src = 0;
			}
		} else if (xstep->trantbl) {
			/* a custom translation table step */
			custom_translate(pzstr, xstep->trantbl);
		}
	ENDLIST
	return cvtd;
}
