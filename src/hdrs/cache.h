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
/*================================================================
 * cache.h -- Header file for NODE cache operations.
 * Copyright(c) 1991 by Thomas T. Wetmore IV; all rights reserved.
 *   Version 2.3.4 - 24 Jun 93 - controlled
 *   Version 2.3.5 - 19 Aug 93 - modified
 *================================================================
 */
/*===============================
 * CACHEEL -- Cache element type.
 *=============================*/
typedef struct c_elem *CACHEEL;
struct c_elem {
	NODE c_node;	/* root node */
	CACHEEL c_prev;	/* previous el */
	CACHEEL c_next;	/* next el */
	STRING c_key;	/* recork key */
	BOOLEAN c_lock;	/* locked? */
};
#define cnode(e) ((e)->c_node)
#define cprev(e) ((e)->c_prev)
#define cnext(e) ((e)->c_next)
#define ckey(e)  ((e)->c_key)
#define clock(e) ((e)->c_lock)
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

extern CACHEEL key_to_indi_cacheel();
extern CACHEEL key_to_fam_cacheel();
extern CACHEEL indi_to_cacheel();
extern CACHEEL fam_to_cacheel();
