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
/*==========================================================
 * charmaps.c -- LifeLines character mapping feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 25 Jul 1994    3.0.2 - 09 Nov 1994
 *========================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "translat.h"

TRANTABLE tran_tables[] = {
	NULL, NULL, NULL, NULL, NULL, NULL,
};
char *map_names[] = {
	"Editor to Internal",
	"Internal to Editor",
	"GEDCOM to Internal",
	"Internal to GEDCOM",
	"Internal to Display",
	"Internal to Report",
};
char *map_keys[] = {
	"MEDIN", "MINED", "MGDIN", "MINGD", "MINDS", "MINRP",
};
#define NOMAPS 6

TRANTABLE init_map_from_rec();
TRANTABLE init_map_from_str();
TRANTABLE init_map_from_file();

static STRING baddec = (STRING) "Bad decimal number format.";
static STRING badhex = (STRING) "Bad hexidecimal number format.";
static STRING norplc = (STRING) "No replacement string on line.";
static STRING badesc = (STRING) "Bad escape format.";
static STRING notabs = (STRING) "Tabs not allowed in replacement string.";

/*========================================
 * init_mapping -- Init translation tables
 *======================================*/
init_mapping ()
{
	INT indx;
	BOOLEAN err;
	for (indx = 0; indx < NOMAPS; indx++) {
		tran_tables[indx] = init_map_from_rec(indx, &err);
		if (err) {
			wprintf("Error initializing %s map.\n",
			   map_names[indx]);
		}
	}
}
/*===================================================
 * init_map_from_rec -- Init single translation table
 *=================================================*/
TRANTABLE init_map_from_rec (indx, perr)
INT indx;
BOOLEAN *perr;
{
	STRING key, rec;
	INT len;
	TRANTABLE tt;

	*perr = FALSE;
	if (!(rec = retrieve_record(map_keys[indx], &len)))
		return NULL;
	tt = init_map_from_str(rec, indx, perr);
	stdfree(rec);
	return tt;
}
/*====================================================
 * init_map_from_file -- Init single translation table
 *==================================================*/
TRANTABLE init_map_from_file (file, indx, perr)
STRING file;
INT indx;
BOOLEAN *perr;
{
	FILE *fp;
	struct stat buf;
	STRING mem;

	*perr = FALSE;
	if ((fp = fopen(file, "r")) == NULL) return NULL;
	ASSERT(fstat(fileno(fp), &buf) == 0);
	if (buf.st_size == 0) {
		fclose(fp);
		return NULL;
	}
	mem = (STRING) stdalloc(buf.st_size+1);
	mem[buf.st_size] = 0;
	ASSERT(fread(mem, buf.st_size, 1, fp) == 1);
	fclose(fp);
	return init_map_from_str(mem, indx, perr);
}
/*==================================================
 * init_map_from_str -- Init single tranlation table
 *================================================*/
TRANTABLE init_map_from_str (str, indx, perr)
STRING str;
INT indx;
BOOLEAN *perr;
{
	INT i, n, line = 1, newc;
	BOOLEAN done;
	unsigned char c, scratch[50];
	STRING p, q, *lefts, *rights;
	TRANTABLE tt;

	ASSERT(str);

/* Count newlines to find lefts and rights sizes */
	*perr = TRUE;
	p = str;
	n = 1;
	while (c = *p++) {
		if (c == '\n')
			n++;
	}
	lefts = (STRING *) stdalloc(n*sizeof(STRING));
	rights = (STRING *) stdalloc(n*sizeof(STRING));
	for (i = 0; i < n; i++) {
		lefts[i] = NULL;
		rights[i] = NULL;
	}

/* Lex the string for patterns and replacements */
	done = FALSE;
	n = 0;
	while (!done) {
		if (!*str) break;
		p = scratch;
		while (TRUE) {
			c = *str++;
			if (c == '#')  {
				newc = get_decimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 3;
				} else {
					maperror(indx, line, baddec);
					return NULL;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					maperror(indx, line, badhex);
					return NULL;
				}
			} else if (c == '\n') {
				maperror(indx, line, norplc);
				return NULL;
			} else if (c == 0) {
				maperror(indx, line, norplc);
				return NULL;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n') {
					maperror(indx, line, badesc);
					return NULL;
				}
				*p++ = c;
			} else if (c == '\t')
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
					maperror(indx, line, baddec);
					return NULL;
				}
			} else if (c == '$') {
				newc = get_hexidecimal(str);
				if (newc >= 0) {
					*p++ = newc;
					str += 2;
				} else {
					maperror(indx, line, badhex);
					return NULL;
				}
			} else if (c == '\n') {
				line++;
				break;
			} else if (c == 0) {
				done = TRUE;
				break;
			} else if (c == '\\') {
				c = *str++;
				if (c == '\t' || c == 0 || c == '\n') {
					maperror(indx, line, badesc);
					return NULL;
				}
				*p++ = c;
			} else if (c == '\t') {
				maperror(indx, line, notabs);
				return NULL;
			} else
				*p++ = c;
		}
		*p = 0;
		rights[n++] = strsave(scratch);
	}
	tt = create_trantable(lefts, rights, n);
	for (i = 0; i < n; i++)		/* don't free rights */
		stdfree(lefts[i]);
	stdfree(lefts);
	stdfree(rights);
	*perr = FALSE;
	return tt;
}
/*==================================================
 * get_decimal -- Get decimal number from map string
 *================================================*/
INT get_decimal (str)
STRING str;
{
	INT value, c;
	if (chartype(c = *str++) != DIGIT) return -1;
	value = c - '0';
	if (chartype(c = *str++) != DIGIT) return -1;
	value = value*10 + c - '0';
	if (chartype(c = *str++) != DIGIT) return -1;
	value = value*10 + c - '0';
	return (value >= 256) ? -1 : value;
}
/*==========================================================
 * get_hexidecimal -- Get hexidecimal number from map string
 *========================================================*/
INT get_hexidecimal (str)
STRING str;
{
	INT value, c, h;
	if ((h = hexvalue(c = *str++)) == -1) return -1;
	value = h;
	if ((h = hexvalue(c = *str++)) == -1) return -1;
	return value*16 + h;
}
/*================================================
 * hexvalue -- Find hexidecimal value of character
 *==============================================*/
INT hexvalue (c)
INT c;
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + c - 'a';
	if (c >= 'A' && c <= 'F') return 10 + c - 'A';
	return -1;
}
/*====================================================
 * maperror -- Print error message from reading string
 *==================================================*/
maperror(indx, line, errmsg)
INT indx, line;
STRING errmsg;
{
	wprintf("%s: line %d: %s\n", map_names[indx], line, errmsg);
}
