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
#if HAVE_ICONV
# include <iconv.h>
#endif
#include "translat.h"
#include "liflines.h"
#include "feedback.h"
#include "bfs.h"
#include "lloptions.h"

#ifdef max
#	undef max
#endif


/*********************************************
 * local types
 *********************************************/
static struct codeset_name
{
	STRING name;
	INT code;
} codesets[] = 
{
	{ "1 byte", 1 }
	, { "UTF-8", 8 }
};

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static XNODE create_xnode(XNODE, INT, STRING);
static void customlocale(STRING prefix);
static STRING get_current_locale(INT category);
static void iconv_trans(TRANMAPPING ttm, CNSTRING in, bfptr bfs);
static BOOLEAN llsetenv(STRING name, STRING value);
static STRING setmsgs(STRING localename);
static void show_xnode(XNODE node);
static void show_xnodes(INT indent, XNODE node);
static XNODE step_xnode(XNODE, INT);
static INT translate_match(TRANTABLE tt, CNSTRING in, CNSTRING * out);

/*********************************************
 * local variables
 *********************************************/

static STRING  deflocale_coll = NULL;
static STRING  deflocale_msgs = NULL;
static BOOLEAN customized_loc = FALSE;
static BOOLEAN customized_msgs = FALSE;

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
	llstrncpy(tt->name, name, sizeof(tt->name));
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
void
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
 * translate_string_to_buf -- Translate string via TRANTABLE
 *  ttm: [IN]  tranmapping to apply
 *  in:  [IN]  string to translate
 * returns dynamic buffer (bfptr type - see buf.h)
 * Created: 2001/07/19 (Perry Rapp)
 * Copied from translate_string, except this version
 * uses dynamic buffer, so it can expand if necessary
 *=================================================*/
void
translate_string_to_buf (TRANMAPPING ttm, CNSTRING in, bfptr bfs)
{
	CNSTRING p, q;
	TRANTABLE tt=0;
	bfReserve(bfs, (int)(strlen(in)*1.3+2));
	if (!in) {
		bfCpy(bfs, NULL);
		return;
	}
	if (!ttm) {
		bfCpy(bfs, in);
		return;
	}
#if HAVE_ICONV
	if (ttm->iconv_src && ttm->iconv_dest) {
		iconv_trans(ttm, in, bfs);
		/* TODO: need to decide how to integrate simultaneous iconv & custom transl. */
		return;
	}
#endif
	tt = get_trantable_from_tranmapping(ttm);
	if (!tt) {
		bfCpy(bfs, in);
		return;
	}
	p = q = in;
	while (*p) {
		CNSTRING tmp;
		TRANTABLE tt = get_trantable_from_tranmapping(ttm);
		INT len = translate_match(tt, p, &tmp);
		if (len) {
			p += len;
			bfCat(bfs, tmp);
		} else {
			bfCatChar(bfs, *p++);
		}
	}
	bfCatChar(bfs, 0);
	return;
}
/*===================================================
 * iconv_trans -- Translate string via iconv
 *  ttm:  [IN]   transmapping
 *  in:   [IN]   string to translate
 *  bfs:  [I/O]  output buffer
 * Only called if HAVE_ICONV
 *=================================================*/
#if HAVE_ICONV
static void
iconv_trans (TRANMAPPING ttm, CNSTRING in, bfptr bfs)
{
	/* TODO: Will have to modify this to use with iconv DLL */
	iconv_t ict = iconv_open(ttm->iconv_dest, ttm->iconv_src);
	CNSTRING inptr = in;
	STRING outptr=bfStr(bfs);
	size_t inleft=strlen(in), outleft=bfs->size-1, cvted=0;
cvting:
	cvted = iconv (ict, &inptr, &inleft, &outptr, &outleft);
	if (cvted == -1) {
		if (!outleft) {
			bfReserveExtra(bfs, (int)(inleft * 1.3+2));
			goto cvting;
		} else {
			/* invalid multibyte sequence, but we don't know how long, so advance
			one byte & retry */
			++inptr;
			goto cvting;
		}
	}
	*outptr=0; /* iconv doesn't zero terminate */
	iconv_close(ict);
}
#endif /* HAVE_ICONV */
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
	bfptr bfs=0;
	if (!in || !in[0]) {
		out[0] = 0;
		return;
	}
	bfs = bfNew((int)(strlen(in)*1.3));
	translate_string_to_buf(ttm, in, bfs);
	strncpy(out, bfStr(bfs), max-1);
	out[max-1]=0;
	bfDelete(bfs);
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
/*======================================
 * add_char -- Add char to output string
 *  buf:    [in]  output string
 *  plen:   [in,out]  address of current output length
 *  max:    [in] max output length
 *  achar:  [in] character to add
 * NB: Handles *plen >= max (won't write past max)
 *====================================*/
void
add_char (STRING buf, INT *plen, INT max, INT achar)
{
	if (*plen >= max - 1)
		buf[max] = 0;
	else
		buf[(*plen)++] = achar;
}
/*==========================================
 * add_string -- Add string to output string
 *========================================*/
void
add_string (STRING buf, INT *plen, INT max, STRING str)
{
	INT len;
	ASSERT(str);
	len = strlen(str);
	if (*plen + len >= max - 1)
		buf[*plen] = 0;
	else {
		strncpy(buf + *plen, str, len);
		*plen += len;
	}
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
	TRANTABLE tts = get_trantable(MSORT);
	TRANTABLE ttc = get_trantable(MCHAR);
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
/*===================================================
 * get_codeset_desc -- Get string describing code set
 * (code set, not charset or codepage)
 * caller supplies buffer (& its size)
 * Created: 2001/08/02 (Perry Rapp)
 *=================================================*/
char *
get_codeset_desc (INT codeset, STRING buffer, INT max)
{
	char * ptr = buffer;
	INT mylen = max;
	INT index=0, i;
	for (i=0; i<ARRSIZE(codesets); ++i) {
		if (codeset == codesets[i].code) {
			index = i;
			break;
		}
	}
	buffer[0] = 0;
	llstrcatn(&ptr, codesets[index].name, &mylen);
	return buffer;
}
/*===================================================
 * get_codeset -- Get codeset code from index in list
 *  index is from list returned by get_codesets
 * Created: 2001/08/02 (Perry Rapp)
 *=================================================*/
#ifdef UNUSED_CODE
INT
get_codeset (INT index)
{
	return ARRSIZE(codesets);
}
#endif
/*===================================================
 * get_codeset_names -- Get list of codesets
 * returns list of static items (do not free items)
 * Created: 2001/08/02 (Perry Rapp)
 *=================================================*/
LIST
get_codesets (void)
{
	LIST list = create_list();
	INT i;
	for (i=0; i<ARRSIZE(codesets); ++i)
		push_list(list, codesets[i].name);
	return list;
}
/*==========================================
 * get_current_locale -- return current locale
 *  returns "C" in case setlocale(x, 0) returns 0
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
#ifdef HAVE_SETLOCALE
static STRING
get_current_locale (INT category)
{
	STRING str = 0;
	str = setlocale(category, NULL);
	return str ? str : "C";
}
#endif /* HAVE_SETLOCALE */
/*==========================================
 * save_original_locales -- grab current locales for later default
 *  We need these for an obscure problem. If user sets only
 *  locales for report, then when we switch back to GUI mode,
 *  we shouldn't stay in the customized report locale.
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
void
save_original_locales (void)
{
#ifdef HAVE_SETLOCALE
	deflocale_coll = strsave(get_current_locale(LC_COLLATE));
#endif /* HAVE_SETLOCALE */

#ifdef HAVE_SETLOCALE
#ifdef HAVE_LC_MESSAGES
	deflocale_msgs = strsave(get_current_locale(LC_MESSAGES));
#endif /* HAVE_LC_MESSAGES */
#endif /* HAVE_SETLOCALE */
	/* if we're not using LC_MESSAGES locale, we use it in the environment (see setmsgs) */
	if (!deflocale_msgs)
		deflocale_msgs = getenv("LC_MESSAGES");

}
/*==========================================
 * ll_langinfo -- wrapper for nl_langinfo
 *  in case not provided (eg, MS-Windows)
 *========================================*/
STRING
ll_langinfo (void)
{
	STRING str = nl_langinfo(CODESET);
	/* TODO: Should we apply norm_charmap.c ?
	http://www.cl.cam.ac.uk/~mgk25/ucs/norm_charmap.c
	*/
	/* TODO: In any case tho, Markus' nice replacement nl_langinfo gives the
	wrong default codepages for MS-Windows I think -- eg, should be 1252 for 
	generic default instead of 8859-1 */

	return str ? str : ""; /* I don't know if nl_langinfo ever returns NULL */
}
/*==========================================
 * termlocale -- free locale related variables
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
void
termlocale (void)
{
	/* free & zero out globals */
	strfree(&deflocale_coll);
	strfree(&deflocale_msgs);
}
/*==========================================
 * uilocale -- set locale to GUI locale
 *  (eg, for displaying a sorted list of people)
 * Created: 2001/08/02 (Perry Rapp)
 *========================================*/
void
uilocale (void)
{
	customlocale("UiLocale");
}
/*==========================================
 * setmsgs -- set locale for LC_MESSAGES
 * Returns non-null string if succeeds
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
static STRING
setmsgs (STRING localename)
{
#ifdef HAVE_SETLOCALE
#ifdef HAVE_LC_MESSAGES
	return setlocale(LC_MESSAGES, localename);
#endif /* HAVE_LC_MESSAGES */
#endif /* HAVE_SETLOCALE */

	return llsetenv("LC_MESSAGES", localename) ? "1" : 0;
}
/*==========================================
 * llsetenv -- assign a value to an environment variable
 * Returns TRUE if supported on this platform
 *========================================*/
static BOOLEAN
llsetenv (STRING name, STRING value)
{
	char buffer[128];
	STRING str = buffer;
	INT len = ARRSIZE(buffer);
	
	buffer[0] = 0;
	appendstr(&str, &len, name);
	appendstr(&str, &len, "=");
	appendstr(&str, &len, value);

#ifdef HAVE_SETENV
	setenv(name, value, 1);
	str = buffer;
#else
#ifdef HAVE_PUTENV
	putenv(buffer);
	str = buffer;
#else
#ifdef HAVE__PUTENV
	_putenv(buffer);
	str = buffer;
#else
	str = 0; /* failed */
#endif /* HAVE__PUTENV */
#endif /* HAVE_PUTENV */
#endif /* HAVE_SETENV */
	return str ? TRUE : FALSE;
}
/*==========================================
 * customlocale -- set locale to custom setting
 *  depending on user options
 *  prefix:  [IN]  option prefix (eg, "UiLocale")
 * Created: 2002/02/24 (Perry Rapp)
 *========================================*/
static void
customlocale (STRING prefix)
{
	char option[64];
	STRING str;
	INT prefixlen = strlen(prefix);
	
	if (prefixlen > 30) return;

	strcpy(option, prefix);

#ifdef HAVE_SETLOCALE
	/* did user set, eg, UiLocaleCollate option ? */
	strcpy(option+prefixlen, "Collate");
	str = getoptstr(option, 0);
	if (str) {
		customized_loc = TRUE;
		str = setlocale(LC_COLLATE, str);
	}
	if (!str) {
		/* did user set, eg, UiLocale option ? */
		option[prefixlen] = 0;
		str = getoptstr(option, 0);
		if (str) {
			customized_loc = TRUE;
			str = setlocale(LC_COLLATE, str);
		}
		/* nothing set, so try to revert to startup value */
		if (!str && customized_loc)
			setlocale(LC_COLLATE, deflocale_coll);
	}
#endif /* HAVE_SETLOCALE */

#if ENABLE_NLS
/*
 * TODO: 2002.03.03, Perry
 * We need to watch for changes here, and 
 * propagate them to menuitem and to date modules,
 * which have to reload arrays if the language changes
 */
	/* did user set, eg, UiLocaleMessages option ? */
	strcpy(option+prefixlen, "Messages");
	str = getoptstr(option, 0);
	if (str) {
		customized_msgs = TRUE;
		str = setmsgs(str);
	} else {
		/* did user set, eg, UiLocale option ? */
		option[prefixlen] = 0;
		str = getoptstr(option, 0);
		if (str) {
			customized_msgs = TRUE;
			str = setmsgs(str);
		}
		if (!str && customized_msgs)
			setmsgs(deflocale_msgs ? deflocale_msgs : "");
	}
	
	if (customized_msgs) {
		extern int _nl_msg_cat_cntr;
		++_nl_msg_cat_cntr;
	}
#endif /* ENABLE_NLS */
}
/*==========================================
 * rptlocale -- set locale to report locale
 *  (eg, for _namesort)
 * Created: 2001/08/02 (Perry Rapp)
 *========================================*/
void
rptlocale (void)
{
	customlocale("RptLocale");
}
