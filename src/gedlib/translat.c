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

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "llstdlib.h"
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
static void show_xnode(XNODE node);
static void show_xnodes(INT indent, XNODE node);
static XNODE step_xnode(XNODE, INT);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=============================================
 * create_trantable -- Create translation table
 *  lefts:  [in] patterns
 *  rights: [in] replacements
 *  n:      [in] num pairs
 *===========================================*/
TRANTABLE
create_trantable (STRING *lefts, STRING *rights, INT n)
{
	TRANTABLE tt = (TRANTABLE) stdalloc(sizeof(*tt));
	STRING left, right;
	INT i, c;
	XNODE node;
	tt->name[0] = 0;
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
translate_catn (TRANTABLE tt, STRING * pdest, CNSTRING src, INT * len)
{
	INT added;
	if (*len > 1)
		translate_string(tt, src, *pdest, *len);
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
INT
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
 *  tt:    [in] tran table
 *  in:    [in] in string
 * returns dynamic buffer (bfptr type - see buf.h)
 * Created: 2001/07/19 (Perry Rapp)
 * Copied from translate_string, except this version
 * uses dynamic buffer, so it can expand if necessary
 *=================================================*/
void
translate_string_to_buf (TRANTABLE tt, CNSTRING in, bfptr bfs)
{
	CNSTRING p, q;
	bfReserve(bfs, (int)(strlen(in)*1.3));
	if (!in) {
		bfCpy(bfs, NULL);
		return;
	}
	if (!tt) {
		bfCpy(bfs, in);
		return;
	}
	p = q = in;
	while (*p) {
		STRING tmp;
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
 * translate_string -- Translate string via TRANTABLE
 *  tt:    [in] tran table
 *  in:    [in] in string
 *  out:   [out] string
 *  max:   [out] max len of out string
 * Output string is limited to max length via use of
 * add_char & add_string.
 *=================================================*/
void
translate_string (TRANTABLE tt, CNSTRING in, STRING out, INT max)
{
	bfptr bfs=0;
	if (!in || !in[0]) {
		out[0] = 0;
		return;
	}
	bfs = bfNew((int)(strlen(in)*1.3));
	translate_string_to_buf(tt, in, bfs);
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
translate_write(TRANTABLE tt, STRING in, INT *lenp
	, FILE *ofp, BOOLEAN last)
{
	char intmp[MAXLINELEN+2];
	char out[MAXLINELEN+2];
	char *tp;
	char *bp;
	int i,j;

	if(tt == NULL) {
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
		translate_string(tt, intmp, out, MAXLINELEN+2);
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
	TRANTABLE tts = tran_tables[MSORT];
	TRANTABLE ttc = tran_tables[MCHAR];
	TRANTABLE ttp = tran_tables[MPREF];
	STRING rep1, rep2;
	STRING ptr1=str1, ptr2=str2;
	INT len1, len2;
	if (!tts) return FALSE;
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
 * get_sort_desc -- Get string describing collate order
 * caller supplies buffer (& its size)
 * Created: 2001/07/21 (Perry Rapp)
 *=================================================*/
char *
get_sort_desc (STRING buffer, INT max)
{
	char * ptr = buffer;
	int mylen = max;
	buffer[0] = 0;
#ifdef HAVE_SETLOCALE
	llstrcatn(&ptr, setlocale(LC_COLLATE, NULL), &mylen);
#else
	llstrcatn(&ptr, "Locales not supported on this platform.", &mylen);
#endif
	return buffer;
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
	int mylen = max;
	int index=0, i;
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
INT
get_codeset (INT index)
{
	return ARRSIZE(codesets);
}
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
 * uilocale -- set locale to GUI locale
 *  (eg, for displaying a sorted list of people)
 * Created: 2001/08/02 (Perry Rapp)
 *========================================*/
void
uilocale (void)
{
#ifdef HAVE_SETLOCALE
	STRING str = getoptstr("UiLocale", "C");
	if (str)
		setlocale(LC_COLLATE, str);
#endif
}
/*==========================================
 * rptlocale -- set locale to report locale
 *  (eg, for _namesort)
 * Created: 2001/08/02 (Perry Rapp)
 *========================================*/
void
rptlocale (void)
{
#ifdef HAVE_SETLOCALE
	STRING str = getoptstr("RptLocale", "C");
	if (str)
		setlocale(LC_COLLATE, str);
#endif
}
