/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*==========================================================
 * charmaps.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   http://lifelines.sourceforge.net
 *========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "liflines.h"
#include "feedback.h"
#include "lloptions.h"
#include "zstr.h"

/*********************************************
 * global/exported variables
 *********************************************/

/*
 Keys for optional character set conversion tables in database
  MEDIN means Editor to Internal, etc
*/
const char *map_keys[] = {
	"MEDIN", "MINED", "MGDIN", "MINGD",
	"MDSIN", "MINDS", "MRPIN", "MINRP", "MSORT",
	"MCHAR", "MLCAS", "MUCAS", "MPREF"
};

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSbaddec,qSbadhex,qSnorplc,qSbadesc,qSnoorig,qSmaperr;

/*********************************************
 * local types
 *********************************************/

/* nodes that make up the tree that is a custom character translation table */
typedef struct tag_xnode *XNODE;
struct tag_xnode {
	XNODE parent;	/* parent node */
	XNODE sibling;	/* next sib node */
	XNODE child;	/* first child node */
	SHORT achar;	/* my character */
	SHORT count;	/* translation length */
	STRING replace;	/* translation string */
};

/* root of a custom character translation table */
struct tag_trantable {
	XNODE start[256];
	char name[20];
	INT total;
};



/*********************************************
 * local enums & defines
 *********************************************/


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static XNODE create_xnode(XNODE, INT, STRING);
static BOOLEAN init_map_from_str(STRING str, CNSTRING mapname, TRANTABLE * ptt, ZSTR zerr);
static void maperror(CNSTRING errmsg);
static void remove_xnodes(XNODE);
static void show_xnode(XNODE node);
static void show_xnodes(INT indent, XNODE node);
static XNODE step_xnode(XNODE, INT);
static INT translate_match(TRANTABLE tt, CNSTRING in, CNSTRING * out);

/*********************************************
 * local variables
 *********************************************/

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=============================================
 * create_trantable -- Create translation table
 *  lefts:  [IN]  patterns
 *  rights: [IN]  replacements (heap-alloc'd)
 *  n:      [IN]  num pairs
 *  name:   [IN]  user-chosen name
 *  This function consumes the right side strings
 *  That is, it takes over their memory
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
		ASSERT(left);
		ASSERT(right);
		ASSERT(*left);
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
 * init_map_from_rec -- Init single translation table
 *  indx:  [IN]  which translation table (see defn of map_keys)
 *  ptt:   [OUT] new translation table, if created
 * Returns FALSE if error
 * But if no tt found, *ptt=0 and returns TRUE.
 *=================================================*/
BOOLEAN
init_map_from_rec (CNSTRING key, INT trnum, TRANTABLE * ptt)
{
	STRING rawrec=0;
	INT len;
	BOOLEAN ok;
	ZSTR zerr=0;
	CNSTRING mapname = transl_get_map_name(trnum);

	*ptt = 0;
	if (!(rawrec = retrieve_raw_record(key, &len)))
		return TRUE;
	zerr = zs_new();
	ok = init_map_from_str(rawrec, mapname, ptt, zerr);
	stdfree(rawrec);
	if (!ok)
		maperror(zs_str(zerr));
	zs_free(&zerr);
	return ok;
}
/*====================================================
 * init_map_from_file -- Init single translation table
 *  file: [IN]  file from which to read translation table
 *  indx: [IN]  which translation table (see defn of map_keys)
 *  ptt:  [OUT] new translation table if created
 * Returns FALSE if error.
 * But if file is empty, *ptt=0 and returns TRUE.
 *==================================================*/
BOOLEAN
init_map_from_file (CNSTRING file, CNSTRING mapname, TRANTABLE * ptt, ZSTR zerr)
{
	FILE *fp;
	struct stat buf;
	STRING mem;
	INT siz;
	BOOLEAN ok;

	*ptt = 0;

	if ((fp = fopen(file, LLREADTEXT)) == NULL) return TRUE;
	ASSERT(fstat(fileno(fp), &buf) == 0);
	if (buf.st_size == 0) {
		fclose(fp);
		return TRUE;
	}
	mem = (STRING) stdalloc(buf.st_size+1);
	mem[buf.st_size] = 0;
	siz = fread(mem, 1, buf.st_size, fp);
	/* may not read full buffer on Windows due to CR/LF translation */
	ASSERT(siz == buf.st_size || feof(fp));
	fclose(fp);
	ok = init_map_from_str(mem, mapname, ptt, zerr);
	stdfree(mem);
	return ok;
}
/*==================================================
 * init_map_from_str -- Init single tranlation table
 *
 * Blank lines or lines beginning with "##" are ignored
 * Translation table entries have the following foramt:
 *
 * <original>{sep}<translation>
 * sep is separator character, by default tab
 *  str:   [IN] input string to translate
 *  indx:  [IN] which translation table (see defn of map_keys)
 *  ptt:   [OUT] new translation table if created
 *  pzerr: [OUT] error string (set if fails)
 * Returns NULL if ok, else error string
 *================================================*/
static BOOLEAN
init_map_from_str (STRING str, CNSTRING mapname, TRANTABLE * ptt, ZSTR zerr)
{
	INT i, n, maxn, entry=1, line=1, newc;
	INT sep = (uchar)'\t'; /* default separator */
	BOOLEAN done;
	BOOLEAN skip;
	BOOLEAN ok = TRUE; /* return value */
	unsigned char c;
	char scratch[50];
	STRING p, *lefts, *rights;
	TRANTABLE tt=NULL;
	STRING errmsg=0;
	char name[sizeof(tt->name)];
	name[0] = 0;

	ASSERT(str);
	*ptt = 0;

/* Count newlines to find lefts and rights sizes */
	p = str;
	n = 0;
	skip = TRUE;
	/* first pass through, count # of entries */
	while (*p) {
		skip=FALSE;
		/* skip blank lines and lines beginning with "##" */
		if (*p == '\r' || *p == '\n') skip=TRUE;
		if (*p =='#' && p[1] == '#') skip=TRUE;
		if (skip) {
			while(*p && (*p != '\n'))
				p++;
			if(*p == '\n')
				p++;
			continue;
		}
		while(*p) {
			if (*p++ == '\n') {
				n++;
				break;
			}
		}
	}
	if (!skip) ++n; /* include last line */
	if (!n) {
		/* empty translation table ignored */
		goto none;
	}
	lefts = (STRING *) stdalloc(n*sizeof(STRING));
	rights = (STRING *) stdalloc(n*sizeof(STRING));
	for (i = 0; i < n; i++) {
		lefts[i] = NULL;
		rights[i] = NULL;
	}

/* Lex the string for patterns and replacements */
	done = FALSE;
	maxn = n;	/* don't exceed the entries you have allocated */
	n = 0;
	while (!done && (n < maxn)) {
		skip=FALSE;
		if (!*str) break;
		/* skip blank lines and lines beginning with "##" */
		if (*str == '\r' || *str == '\n') skip=TRUE;
		if (*str =='#' && str[1] == '#') {
			skip=TRUE;
			if (!strncmp(str, "##!sep", 6)) {
				/* new separator character if legal */
				if (str[6]=='=')
					sep='=';
			}
			if (!strncmp(str, "##!name: ",9)) {
				STRING p1=str+9, p2=name;
				INT i=sizeof(name);
				while (*p1 && *p1 != '\n' && --i)
					*p2++ = *p1++;
				*p2=0;
			}
		}
		if (skip) {
			while(*str && (*str != '\n'))
				str++;
			if (*str == '\n')
				str++;
			continue;
		}
		p = scratch;
		while (TRUE) {
			c = *str++;
			if (c == '#')  {
				newc = get_decimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 3;
				} else {
					errmsg = _(qSbaddec);
					goto fail;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					errmsg = _(qSbadhex);
					goto fail;
				}
			} else if ((c == '\n') || (c == '\r'))   {
				errmsg = _(qSnorplc);
				goto fail;
			} else if (c == 0) {
				errmsg = _(qSnorplc);
				goto fail;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n'
				    || c == '\r') {
					errmsg = _(qSbadesc);
					goto fail;
				}
				*p++ = c;
			} else if (c == sep)
				break;
			else
				*p++ = c;
		}
		*p = 0;
		if (!scratch[0]) {
				errmsg = _(qSnoorig);
				goto fail;
		}
		lefts[n] = strsave(scratch);
		p = scratch;
		while (TRUE) {
			c = *str++;
			if (c == '#')  {
				newc = get_decimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 3;
				} else {
					errmsg = _(qSbaddec);
					goto fail;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					errmsg = _(qSbadhex);
					goto fail;
				}
			} else if (c == '\n') {
				++line;
				++entry;
				break;
			} else if (c == 0) {
				done = TRUE;
				break;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n'
				    || c == '\r') {
					errmsg = _(qSbadesc);
					goto fail;
				}
				if (c == 't') c='\t'; /* "\t" -> tab */
				*p++ = c;
			} else if (c == '\t' || c == sep) {
				/* treat as beginning of a comment */
				while(*str && (*str != '\n'))
					str++;
				if(*str == '\n') {
					str++;
					line++;
					entry++;
				}
				break;
			} else if (c == '\r') {
				/* ignore (MSDOS has this before \n) */
			} else {
				/* not special, just copy replacement char */
				*p++ = c;
			}
		}
		*p = 0;
		rights[n++] = strsave(scratch);
	}
	if (!name[0])
		llstrncpy(name, mapname, sizeof(name), uu8);
	*ptt = create_trantable(lefts, rights, n, name);
end:
	for (i = 0; i < n; i++)		/* don't free rights */
		stdfree(lefts[i]);
	stdfree(lefts);
	stdfree(rights);
none:
	return ok;

fail:
	zs_setf(zerr, _(qSmaperr), mapname, line, entry, errmsg);

	for (i = 0; i < n; i++) /* rights not consumed by tt */
		stdfree(rights[i]);
	ok = FALSE;
	goto end;
}
/*==================================================
 * get_decimal -- Get decimal number from map string
 *================================================*/
INT
get_decimal (STRING str)
{
	INT value, c;
	if (chartype(c = (uchar)*str++) != DIGIT) return -1;
	value = c - '0';
	if (chartype(c = (uchar)*str++) != DIGIT) return -1;
	value = value*10 + c - '0';
	if (chartype(c = (uchar)*str++) != DIGIT) return -1;
	value = value*10 + c - '0';
	return (value >= 256) ? -1 : value;
}
/*==========================================================
 * get_hexidecimal -- Get hexidecimal number from map string
 *========================================================*/
INT
get_hexidecimal (STRING str)
{
	INT value, h;
	if ((h = hexvalue(*str++)) == -1) return -1;
	value = h;
	if ((h = hexvalue(*str++)) == -1) return -1;
	return value*16 + h;
}
/*================================================
 * hexvalue -- Find hexidecimal value of character
 *==============================================*/
INT
hexvalue (INT c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + c - 'a';
	if (c >= 'A' && c <= 'F') return 10 + c - 'A';
	return -1;
}
/*====================================================
 * maperror -- Print error message from reading map
 *==================================================*/
static void
maperror (CNSTRING errmsg)
{
	llwprintf((STRING)errmsg);
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
 * custom_translatez -- Translate string via custom translation table
 *  zstr: [I/O] string to be translated (in-place)
 *  tt:   [IN]  custom translation table
 *=================================================*/
void
custom_translatez (ZSTR zstr, TRANTABLE tt)
{
	ZSTR zout = custom_translate(zs_str(zstr), tt);
	zs_setz(zstr, zout);
	zs_free(&zout);
}
/*===================================================
 * custom_translate -- Translate string via custom translation table
 *  str:  [IN]  string to be translated
 *  tt:   [IN]  custom translation table
 * returns translated string
 *=================================================*/
ZSTR
custom_translate (CNSTRING str, TRANTABLE tt)
{
	ZSTR zout = zs_newn((unsigned int)(strlen(str)*1.3+2));
	CNSTRING p = str;
	while (*p) {
		CNSTRING tmp;
		INT len = translate_match(tt, p, &tmp);
		if (len) {
			p += len;
			zs_apps(zout, tmp);
		} else {
			zs_appc(zout, *p++);
		}
	}
	return zout;
}
/*===================================================
 * custom_sort -- Compare two strings with custom sort
 * returns FALSE if no custom sort table
 * otherwise sets *rtn correctly & returns TRUE
 * Created: 2001/07/21 (Perry Rapp)
 *=================================================*/
BOOLEAN
custom_sort (const char *str1, const char *str2, INT * rtn)
{
	TRANTABLE tts = transl_get_legacy_tt(MSORT);
	TRANTABLE ttc = transl_get_legacy_tt(MCHAR);
	CNSTRING rep1, rep2;
	CNSTRING ptr1=str1, ptr2=str2;
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
 * get_trantable_desc -- Get description of trantable
 * Created: 2002/11/26 (Perry Rapp)
 *=================================================*/
ZSTR
get_trantable_desc (TRANTABLE tt)
{
	ZSTR zstr=zs_new();
	char buffer[36];
	if (tt->name[0]) {
		zs_apps(zstr, tt->name);
	} else {
		zs_apps(zstr, "(Unnamed table)");
	}
	sprintf(buffer, " [%ld]", tt->total);
	zs_apps(zstr, buffer);
	return zstr;
}
/*===================================================
 * tt_get_name -- Return name of translation table
 *  or "unnamed"
 * Created: 2002/12/13 (Perry Rapp)
 *=================================================*/
CNSTRING
tt_get_name (TRANTABLE tt)
{
	CNSTRING name = tt->name;
	if (!name || !name[0])
		name = _("<unnamed>");
	return name;
}
