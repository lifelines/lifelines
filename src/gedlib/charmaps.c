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
/*==========================================================
 * charmaps.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 25 Jul 1994    3.0.2 - 09 Nov 1994
 *========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "liflines.h"
#include "feedback.h"

/*********************************************
 * global/exported variables
 *********************************************/

TRANTABLE tran_tables[] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
char *map_keys[] = {
	"MEDIN", "MINED", "MGDIN", "MINGD", "MDSIN", "MINDS", "MINRP"
	, "MSORT", "MCHAR", "MLCAS", "MUCAS", "MPREF"
};

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING baddec, badhex, norplc, badesc;

/*********************************************
 * local enums & defines
 *********************************************/


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static BOOLEAN init_map_from_rec(INT, TRANTABLE*);
static BOOLEAN init_map_from_str(STRING, INT, TRANTABLE*);
static void maperror(INT index, INT entry, INT line, STRING errmsg);

/*********************************************
 * local variables
 *********************************************/

static char *map_names[] = {
	"Editor to Internal",
	"Internal to Editor",
	"GEDCOM to Internal",
	"Internal to GEDCOM",
	"Display to Internal",
	"Internal to Display",
	"Internal to Report",
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*========================================
 * init_mapping -- Init translation tables
 *======================================*/
void
init_mapping (void)
{
	INT indx;
	for (indx = 0; indx < NUM_TT_MAPS; indx++) {
		if (!init_map_from_rec(indx, &tran_tables[indx])) {
			msg_error("Error initializing %s map.\n"
				, map_names[indx]);
		}
	}
}
/*===================================================
 * init_map_from_rec -- Init single translation table
 *  indx:  [IN]  which translation table (see defn of map_keys)
 *  ptt:   [OUT] new translation table, if created
 * Returns FALSE if error
 * But if no tt found, *ptt=0 and returns TRUE.
 *=================================================*/
BOOLEAN
init_map_from_rec (INT indx, TRANTABLE * ptt)
{
	STRING rawrec;
	INT len;
	BOOLEAN ok;

	*ptt = 0;
	if (!(rawrec = retrieve_raw_record(map_keys[indx], &len)))
		return TRUE;
	ok = init_map_from_str(rawrec, indx, ptt);
	stdfree(rawrec);
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
init_map_from_file (STRING file, INT indx, TRANTABLE * ptt)
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
	ok = init_map_from_str(mem, indx, ptt);
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
 *  str:  [IN] input string to translate
 *  indx: [IN] which translation table (see defn of map_keys)
 *  ptt:  [OUT] new translation table if created
 * May return NULL
 *================================================*/
static BOOLEAN
init_map_from_str (STRING str, INT indx, TRANTABLE * ptt)
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
					maperror(indx, entry, line, baddec);
					goto fail;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					maperror(indx, entry, line, badhex);
					goto fail;
				}
			} else if ((c == '\n') || (c == '\r'))   {
				maperror(indx, entry, line, norplc);
				goto fail;
			} else if (c == 0) {
				maperror(indx, entry, line, norplc);
				goto fail;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n'
				    || c == '\r') {
					maperror(indx, entry, line, badesc);
					goto fail;
				}
				*p++ = c;
			} else if (c == sep)
				break;
			else
				*p++ = c;
		}
		*p = 0;
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
					maperror(indx, entry, line, baddec);
					goto fail;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					maperror(indx, entry, line, badhex);
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
					maperror(indx, entry, line, badesc);
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
	*ptt = create_trantable(lefts, rights, n, name);
end:
	for (i = 0; i < n; i++)		/* don't free rights */
		stdfree(lefts[i]);
	stdfree(lefts);
	stdfree(rights);
none:
	return ok;

fail:
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
 * maperror -- Print error message from reading string
 *  indx:   [in] which translation table
 *  entry:  [in] index of entry, 1-based
 *  line:   [in] raw line number in file, 1-based
 *  errmsg:  [in] error message
 *==================================================*/
void
maperror (INT indx, INT entry, INT line, STRING errmsg)
{
	llwprintf("%s: line %d (entry %d): %s\n", map_names[indx], line, entry, errmsg);
}
