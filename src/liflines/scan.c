/* 
   Copyright (c) 2000 Perry Rapp

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

#include "sys_inc.h"
#include <stdarg.h>
#include "llstdlib.h"
#include "table.h"
#include "indiseq.h"
#include "interp.h"
#include "screen.h"
#include "liflines.h"

#include "llinesi.h"

extern STRING scanrs, scannm, scantt;

typedef struct
{
	char string[12];
} NAME_PATTERN;

static INDISEQ seq;

/*=============================================
 * pattern_match -- Compare a name to a pattern
 *===========================================*/
static BOOLEAN
pattern_match (NAME_PATTERN *patt, STRING name)
{
	STRING p1,p2;
	/* match . to any letter, and trailing * to anything */
	p1=patt->string;
	p2=name;
	for (p1=patt->string,p2=name; *p1 || *p2; p1++,p2++) {
		if (*p1 == '*' && *(p1+1) == 0)
			return TRUE;
		if (*p1 != '.' && ll_toupper(*p1) != (*p2))
			return FALSE;
	}
	return TRUE;
}
/*===========================================
 * ns_callback -- callback for name traversal
 *=========================================*/
static BOOLEAN
ns_callback (STRING skey, STRING name, void *param)
{
	LIST list;
	INT len, ind;
	STRING piece;
	NAME_PATTERN * patt = (NAME_PATTERN *)param;
	list = create_list();
	name_to_list(name, list, &len, &ind);
	FORLIST(list, el)
		piece = (STRING)el;
		if (pattern_match(patt, piece)) {
			/* if we pass in name, append_indiseq won't check for dups */
			append_indiseq(seq, skey, NULL, 0, FALSE, FALSE);
			}
	ENDLIST
	free_name_list(list);
	
	return TRUE;
}
/*============================================
 * valid_pattern -- check pattern for validity
 *  for first cut, just refuse anything with spaces
 *===========================================*/
static BOOLEAN
valid_pattern (STRING str)
{
	INT i;
	for (i=0; str[i]; i++)
		if (str[i] == ' ')
			return FALSE;
	return TRUE;
}
/*=========================================================
 * name_scan -- traverse names looking for pattern matching
 *=======================================================*/
NODE
name_scan ()
{
	NAME_PATTERN patt;
	NODE indi = NULL;
	STRING str;
	
	while (1) {
		str = ask_for_string(scannm, scantt);
		if (!str || !str[0])
			return NULL;
		if (valid_pattern(str))
			break;
	}

	strcpy(patt.string, str);

	seq = create_indiseq();
	traverse_names(ns_callback, &patt);

	if (length_indiseq(seq)) {
		indi = format_and_choose_indi(seq, TRUE, scanrs, scanrs);
	}
	remove_indiseq(seq, FALSE);
	return indi;
}
