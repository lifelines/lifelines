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

typedef struct xnode *XNODE;
struct xnode {
	XNODE parent;	/* parent node */
	XNODE sibling;	/* next sib node */
	XNODE child;	/* first child node */
	SHORT achar;	/* my character */
	SHORT count;	/* translation length */
	STRING replace;	/* translation string */
};

typedef struct {
	XNODE start[256];
} *TRANTABLE;

/* Variables */

extern TRANTABLE tran_tables[];

/* Functions */

void add_char(STRING, INT*, INT, INT);
void add_string(STRING, INT*, INT, STRING);
TRANTABLE create_trantable(STRING*, STRING*, INT);
TRANTABLE init_map_from_str(STRING, INT, BOOLEAN*);
void remove_trantable(TRANTABLE);
void remove_xnodes(XNODE);
void show_xnode(XNODE);
void show_xnodes(INT, XNODE);
void translate_catn(TRANTABLE tt, STRING * pdest, CNSTRING src, INT * len);
void translate_string(TRANTABLE, CNSTRING in, STRING out, INT max);
BOOLEAN translate_write(TRANTABLE, STRING, INT*, FILE*, BOOLEAN);


#endif /* _TRANSLAT_H */
