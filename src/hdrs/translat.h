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
/*===========================================================
 * translat.h -- Header file for translate feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.0 - 29 May 1994
 *=========================================================*/

#ifndef _TRANSLAT_H
#define _TRANSLAT_H

#include "standard.h"

/* Types */

/*
Need translation chain
 made up of translation step
 each step is either iconv step or custom step
 need routine to build chain (check for rule, & try to construct chain)
 put embedded table into chain (?) with flag for which end, or put into list ? how does this work now when edited ?
*/


typedef struct trantable_s *TRANTABLE;

/* a translation mapping, which may have a TRANTABLE, and may have iconv info */
typedef struct xlat_s *XLAT;

/* This will go into translat.c when new system is working */
struct xlat_s {
	/* All members either NULL or heap-alloc'd */
	TRANTABLE dbtrantbl; /* mappings embedded in active db */
	STRING iconv_src;
	STRING iconv_dest;
	LIST global_trans; /* list of global mappings */
	BOOLEAN after; /* do custom transtable after iconv ? */
	/* NEW 2002-11-25 */
	STRING src;
	STRING dest;
	LIST steps;
	BOOLEAN adhoc;
};


/* Variables */

extern CNSTRING map_names[];

/* Functions */

TRANTABLE create_trantable(STRING *lefts, STRING *rights, INT n, STRING name);
BOOLEAN init_map_from_rec(CNSTRING key, INT trnum, TRANTABLE*);
BOOLEAN custom_sort(char *str1, char *str2, INT * rtn);
XLAT get_tranmapping(INT ttnum);
TRANTABLE get_dbtrantable(INT ttnum);
TRANTABLE get_dbtrantable_from_tranmapping(XLAT ttm);
ZSTR get_trantable_desc(TRANTABLE tt);
void remove_trantable(TRANTABLE);
void set_dbtrantable(INT ttnum, TRANTABLE tt);
void translate_catn(XLAT ttm, STRING * pdest, CNSTRING src, INT * len);
void translate_string(XLAT, CNSTRING in, STRING out, INT max);
ZSTR translate_string_to_zstring(XLAT ttm, CNSTRING in);
BOOLEAN translate_write(XLAT, STRING, INT*, FILE*, BOOLEAN);


/*
New system under development 2002-11-25+
translat is the frontend, which knows about the various codesets (internal, GUI, ...)
 uses codesets & xlat
 xlat is the translation system
   uses charmaps
*/
BOOLEAN transl_are_all_conversions_ok(void);
XLAT transl_get_predefined_xlat(INT trnum);
XLAT transl_get_xlat(CNSTRING src, CNSTRING dest);
XLAT transl_get_xlat_to_int(CNSTRING codeset);
void transl_load_all_tts(void);
void transl_load_xlats(BOOLEAN indb);
void transl_parse_codeset(CNSTRING codeset, ZSTR * zcsname, LIST * subcodes);
BOOLEAN transl_xlat(XLAT xlat, ZSTR * pzstr);

#endif /* _TRANSLAT_H */
