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
/*===========================================================
 * translat.c -- LifeLines character mapping functions
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.0 - 17 Jun 94    3.0.2 - 11 Nov 94
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

#ifdef max
#	undef max
#endif

/*********************************************
 * global/exported variables
 *********************************************/

STRING illegal_char = 0;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static XNODE create_xnode(XNODE, INT, STRING);
static ZSTR custom_translate(TRANTABLE tt, ZSTR zin);
static ZSTR global_translate(LIST gtlist, ZSTR zstr);
static ZSTR iconv_trans_ttm(TRANMAPPING ttm, ZSTR zin, CNSTRING illegal);
static void remove_xnodes(XNODE);
static void show_xnode(XNODE node);
static void show_xnodes(INT indent, XNODE node);
static XNODE step_xnode(XNODE, INT);
static INT translate_match(TRANTABLE tt, CNSTRING in, CNSTRING * out);


/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=============================================
 * create_trantable -- Create translation table
 *  lefts:  [IN]  patterns
 *  rights: [IN]  replacements
 *  n:      [IN]  num pairs
 *  name:   [IN]  user-chosen name
 *===========================================*/
TRANTABLE
create_trantable (STRING *lefts, STRING *rights, INT n, STRING name)
{
	TRANTABLE tt = (TRANTABLE) stdalloc(sizeof(*tt));
	STRING left, right;
	INT i, c;
	XNODE node;
	tt->name[0] = 0;
	tt->total = n;
	llstrncpy(tt->name, name, sizeof(tt->name), uu8);
	for (i = 0; i < 256; i++)
		tt->start[i] = NULL;
	/* if empty, n==0, this is valid */
	for (i = 0; i < n; i++) {
		left = lefts[i];
		right = rights[i];
		ASSERT(left && *left && right);
		c = (uchar) *left++;
		if (tt->start[c] == NULL)
			tt->start[c] = create_xnode(NULL, c, NULL);
		node = tt->start[c];
		while ((c = (uchar) *left++)) {
			node = step_xnode(node, c);
		}
		node->count = strlen(right);
		node->replace = right;
	}
	return tt;
}
/*=============================
 * create_xnode -- Create XNODE
 *  parent:  [in] parent of node to be created
 *  achar:   [in] start substring represented by this node
 *  string:  [in] replacement string for matches
 *===========================*/
static XNODE
create_xnode (XNODE parent, INT achar, STRING string)
{
	XNODE node = (XNODE) stdalloc(sizeof(*node));
	node->parent = parent;
	node->sibling = NULL;
	node->child = NULL;
	node->achar = achar;
	node->replace = string;
	node->count = string ? strlen(string) : 0;
	return node;
}
/*==========================================
 * step_xnode -- Step to node from character
 *========================================*/
static XNODE
step_xnode (XNODE node, INT achar)
{
	XNODE prev, node0 = node;
	if (node->child == NULL)
		return node->child = create_xnode(node0, achar, NULL);
	prev = NULL;
	node = node->child;
	while (node) {
		if (node->achar == achar) return node;
		prev = node;
		node = node->sibling;
	}
	return prev->sibling = create_xnode(node0, achar, NULL);
}
/*=============================================
 * remove_trantable -- Remove translation table
 *===========================================*/
void
remove_trantable (TRANTABLE tt)
{
	INT i;
	if (!tt) return;
	for (i = 0; i < 256; i++)
		remove_xnodes(tt->start[i]);
	stdfree(tt);
}
/*====================================
 * remove_xnodes -- Remove xnodes tree
 *==================================*/
static void
remove_xnodes (XNODE node)
{
	if (!node) return;
	remove_xnodes(node->child);
	remove_xnodes(node->sibling);
	if (node->replace) stdfree(node->replace);
	stdfree(node);
}
/*===================================================
 * translate_catn -- Translate & concatenate string
 *
 * tt:    translation table to use
 * pdest: address of destination (will be advanced)
 * src:   source string
 * len:   address of space left in destination (will be decremented)
 *=================================================*/
void
translate_catn (TRANMAPPING ttm, STRING * pdest, CNSTRING src, INT * len)
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
 * translate_match -- Find match for current point in string
 *  tt:    [in] tran table
 *  in:    [in] in string
 *  match: [out] match string
 * returns length of input matched
 * match string output points directly into trans table
 * memory, so it is longer-lived than a static buffer
 * Created: 2001/07/21 (Perry Rapp)
 *=================================================*/
static INT
translate_match (TRANTABLE tt, CNSTRING in, CNSTRING * out)
{
	XNODE node, chnode;
	INT nxtch;
	CNSTRING q = in;
	node = tt->start[(uchar)*in];
	if (!node) {
		*out = "";
		return 0;
	}
	q = in+1;
/* Match as far as possible */
	while (*q && node->child) {
		nxtch = (uchar)*q;
		chnode = node->child;
		while (chnode && chnode->achar != nxtch)
			chnode = chnode->sibling;
		if (!chnode) break;
		node = chnode;
		q++;
	}
	while (TRUE) {
		if (node->replace) {
			/* replacing match */
			*out = node->replace;
			return q - in;
		}
		/* no replacement, only partial match,
		climb back & keep looking - we might have gone past
		a shorter but full (replacing) match */
		if (node->parent) {
			node = node->parent;
			--q;
			continue;
		}
		/*
		no replacement matches
		(climbed all the way back to start
		*/
		ASSERT(q == in+1);
		*out = "";
		return 0;
	}
	return 0;
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
translate_string_to_zstring (TRANMAPPING ttm, CNSTRING in)
{
	TRANTABLE ttdb = get_dbtrantable_from_tranmapping(ttm);
	ZSTR zout = zs_newn((unsigned int)(strlen(in)*1.3+2));
	zs_set(zout, in);
	if (!in || !ttm) {
		return zout;
	}
	if (!ttm->after) {
		if (ttdb) {
			/* custom translation before iconv */
			zout = custom_translate(ttdb, zout);
		}
		if (ttm->global_trans) {
			zout = global_translate(ttm->global_trans, zout);
		}
	}
	if (ttm->iconv_src && ttm->iconv_src[0] 
		&& ttm->iconv_dest && ttm->iconv_dest[0]) {
		zout = iconv_trans_ttm(ttm, zout, illegal_char);
	}
	if (ttm->after) {
		if (ttm->global_trans) {
			zout = global_translate(ttm->global_trans, zout);
		}
		if (ttdb) {
			/* custom translation after iconv */
			zout = custom_translate(ttdb, zout);
		}
	}
	return zout;
}
/*===================================================
 * custom_translate -- Translate string via custom translation table
 *  tt:  [IN]  custom translation table
 *  zin: [IN]  string to be translated & destroyed
 * returns translated string
 *=================================================*/
static ZSTR
custom_translate (TRANTABLE tt, ZSTR zin)
{
	ZSTR zout = zs_newn((unsigned int)(zs_len(zin)*1.3+2));
	STRING p = zs_str(zin);
	while (*p) {
		CNSTRING tmp;
		INT len = translate_match(tt, p, &tmp);
		if (len) {
			p += len;
			zs_cat(zout, tmp);
		} else {
			zs_catc(zout, *p++);
		}
	}
	zs_free(zin);
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
iconv_trans_ttm (TRANMAPPING ttm, ZSTR zin, CNSTRING illegal)
{
	CNSTRING dest=ttm->iconv_dest;
	CNSTRING src = ttm->iconv_src;
	ZSTR zout;
	BOOLEAN success;
	ASSERT(dest && src);
	zout = iconv_trans(src, dest, zin, illegal, &success);
	if (!success) {
		/* if invalid translation, clear it to avoid trying again */
		strfree(&ttm->iconv_src);
		strfree(&ttm->iconv_dest);
	}
	return zout;
}
/*===================================================
 * global_translate -- Apply list of user global transforms
 *=================================================*/
static ZSTR
global_translate (LIST gtlist, ZSTR zstr)
{
	TRANTABLE ttx=0;
	FORLIST(gtlist, tbel)
		ttx = tbel;
		ASSERT(ttx);
		zstr = custom_translate(ttx, zstr);
		ttx = 0;
	ENDLIST
	return zstr;
}
/*===================================================
 * translate_string -- Translate string via TRANMAPPING
 *  ttm:   [IN]  tranmapping
 *  in:    [IN]  in string
 *  out:   [OUT] string
 *  max:   [OUT] max len of out string
 * Output string is limited to max length via use of
 * add_char & add_string.
 *=================================================*/
void
translate_string (TRANMAPPING ttm, CNSTRING in, STRING out, INT max)
{
	ZSTR zstr=0;
	if (!in || !in[0]) {
		out[0] = 0;
		return;
	}
	zstr = translate_string_to_zstring(ttm, in);
	llstrncpy(out, zs_str(zstr), max, uu8);
	zs_free(zstr);
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
translate_write(TRANMAPPING ttm, STRING in, INT *lenp
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

#ifdef DEBUG
/*=======================================================
 * show_trantable -- DEBUG routine that shows a TRANTABLE
 *=====================================================*/
void
show_trantable (TRANTABLE tt)
{
	INT i;
	XNODE node;
	if (tt == NULL) {
		llwprintf("EMPTY TABLE\n");
		return;
	}
	for (i = 0; i < 256; i++) {
		node = tt->start[i];
		if (node) {
			show_xnodes(0, node);
		}
	}
}
#endif /* DEBUG */

/*===============================================
 * show_xnodes -- DEBUG routine that shows XNODEs
 *=============================================*/
static void
show_xnodes (INT indent, XNODE node)
{
	INT i;
	if (!node) return;
	for (i = 0; i < indent; i++)
		llwprintf("  ");
	show_xnode(node);
	show_xnodes(indent+1, node->child);
	show_xnodes(indent,   node->sibling);
}
/*================================================
 * show_xnode -- DEBUG routine that shows 1 XNODE
 *==============================================*/
static void
show_xnode (XNODE node)
{
	llwprintf("%d(%c)", node->achar, node->achar);
	if (node->replace) {
		if (node->count)
			llwprintf(" \"%s\"\n", node->replace);
		else
			llwprintf(" \"\"\n");
	} else
		llwprintf("\n");
}
/*===================================================
 * custom_sort -- Compare two strings with custom sort
 * returns FALSE if no custom sort table
 * otherwise sets *rtn correctly & returns TRUE
 * Created: 2001/07/21 (Perry Rapp)
 *=================================================*/
BOOLEAN
custom_sort (char *str1, char *str2, INT * rtn)
{
	TRANTABLE tts = get_dbtrantable(MSORT);
	TRANTABLE ttc = get_dbtrantable(MCHAR);
	CNSTRING rep1, rep2;
	STRING ptr1=str1, ptr2=str2;
	INT len1, len2;
	if (!tts) return FALSE;
/* This was an attempt at handling skip-over prefixes (eg, Mc) */
#if 0 /* must be done earlier */
	if (ptr1[0] && ptr2[0]) {
		/* check for prefix skips */
		len1 = translate_match(ttp, ptr1, &rep1);
		len2 = translate_match(ttp, ptr2, &rep2);
		if (len1 || len2) {
		}
		if (strchr(rep1,"s") && ptr1[len1]) {
			ptr1 += len1;
			len1 = translate_match(tts, ptr1, &rep1);
		}
		if (strchr(rep2,"s") && ptr2[len2]) {
			ptr2 += len2;
			len2 = translate_match(tts, ptr2, &rep1);
		}
	}
#endif
	/* main loop thru both strings looking for differences */
	while (1) {
		/* stop when exhaust either string */
		if (!ptr1[0] || !ptr2[0]) {
			/* only zero if both end simultaneously */
			*rtn = ptr1[0] - ptr2[0];
			return TRUE;
		}
		/* look up in sort table */
		len1 = translate_match(tts, ptr1, &rep1);
		len2 = translate_match(tts, ptr2, &rep2);
		if (len1 && len2) {
			/* compare sort table results */
			*rtn = atoi(rep1) - atoi(rep2);
			if (*rtn) return TRUE;
			ptr1 += len1;
			ptr2 += len2;
		} else {
			/* at least one not in sort table */
			/* try comparing single chars */
			*rtn = ptr1[0] - ptr2[0];
			if (*rtn) return TRUE;
			/* use charset to see how wide they are */
			if (ttc) {
				len1 = translate_match(ttc, ptr1, &rep1);
				len2 = translate_match(ttc, ptr2, &rep2);
				if (len1 || len2) {
					/* compare to width of wider char */
					*rtn = strncmp(ptr1, ptr2, len1>len2?len1:len2);
				}
			} else {
				/* TO DO - can we use locale here ? ie, locale + custom sort */
				len1 = len2 = 1;
				*rtn = ptr1[0] - ptr2[0];
			}
			if (*rtn) return TRUE;
			/* advance both by at least one */
			ptr1 += len1 ? len1 : 1;
			ptr2 += len2 ? len2 : 1;
		}
	}
}
