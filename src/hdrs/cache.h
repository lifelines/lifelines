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
/*================================================================
 * cache.h -- Header file for NODE cache operations.
 * Copyright(c) 1991 by Thomas T. Wetmore IV; all rights reserved.
 *   Version 2.3.4 - 24 Jun 93 - controlled
 *   Version 2.3.5 - 19 Aug 93 - modified
 *================================================================
 */

#ifndef _CACHE_H
#define _CACHE_H

#include "gedcom.h"

/*===============================
 * CACHEEL -- Cache element type.
 *=============================*/
typedef struct c_elem *CACHEEL;
struct c_elem {
	NOD0 c_nod0;
	NODE c_node;	/* root node */
	CACHEEL c_prev;	/* previous el */
	CACHEEL c_next;	/* next el */
	STRING c_key;	/* record key */
	BOOLEAN c_lock;	/* locked? */
};
#define cnod0(e) ((e)->c_nod0)
#define cnode(e) ((e)->c_node)
#define cprev(e) ((e)->c_prev)
#define cnext(e) ((e)->c_next)
#define ckey(e)  ((e)->c_key)
#define cclock(e) ((e)->c_lock)
/*==============================
 * CACHE -- Internal cache type.
 *============================*/
typedef struct {
	TABLE c_data;		/* table of keys */
	CACHEEL c_firstdir;	/* first direct */
	CACHEEL c_lastdir;	/* last direct */
	CACHEEL c_firstind;	/* first indirect */
	CACHEEL c_lastind;	/* last indirect */
	INT c_maxdir;		/* max in direct */
	INT c_sizedir;		/* cur in direct */
	INT c_maxind;		/* max in indirect */
	INT c_sizeind;		/* cur in indirect */
} *CACHE;
#define cdata(c)     ((c)->c_data)
#define cfirstdir(c) ((c)->c_firstdir)
#define clastdir(c)  ((c)->c_lastdir)
#define cfirstind(c) ((c)->c_firstind)
#define clastind(c)  ((c)->c_lastind)
#define cmaxdir(c)   ((c)->c_maxdir)
#define csizeind(c)  ((c)->c_sizeind)
#define cmaxind(c)   ((c)->c_maxind)
#define csizedir(c)  ((c)->c_sizedir)

CACHEEL even_to_cacheel(NODE);
CACHEEL fam_to_cacheel(NODE);
CACHEEL indi_to_cacheel(NODE);
CACHEEL othr_to_cacheel(NODE);
CACHEEL sour_to_cacheel(NODE);

CACHEEL key_to_indi_cacheel(STRING);
CACHEEL key_to_fam_cacheel(STRING);
CACHEEL qkey_to_even_cacheel(STRING);
CACHEEL qkey_to_fam_cacheel(STRING);
CACHEEL qkey_to_indi_cacheel(STRING);
CACHEEL qkey_to_sour_cacheel(STRING);
CACHEEL qkey_to_othr_cacheel(STRING);

void lock_cache(CACHEEL);
void node_to_cache(CACHE, NODE);
void remove_from_cache(CACHE, STRING);
void unlock_cache(CACHEEL);

#endif /* _CACHE_H */
