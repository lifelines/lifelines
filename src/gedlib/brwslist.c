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
/*==============================================================
 * brwslist.c -- Browse list operations
 * Copyright (c) 1993-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 27 Nov 94
 *============================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"

LIST browse_lists;
INDISEQ current_seq = NULL;

typedef struct {
	STRING bl_name;
	INDISEQ bl_seq;
} *BLEL, BLEL_struct;

/*=====================================================
 *  init_browse_lists -- Initialize named browse lists.
 *===================================================*/
void
init_browse_lists (void)
{
	browse_lists = create_list();
}
/*===========================================
 *  add_browse_list -- Add named browse list.
 *=========================================*/
void
add_browse_list (STRING name,
                 INDISEQ seq)
{
	BLEL blel;
	BOOLEAN done = FALSE;
	if (!name) return;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (blel->bl_name && eqstr(name, blel->bl_name)) {
			remove_indiseq(blel->bl_seq, FALSE);
			blel->bl_seq = seq;
			done = TRUE;
			break;
		}
	ENDLIST
	if (done) return;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (!blel->bl_name) {
			blel->bl_name = name;
			blel->bl_seq = seq;
			return;
		}
	ENDLIST
 	blel = (BLEL) stdalloc(sizeof(BLEL_struct));
	blel->bl_name = name;
	blel->bl_seq = seq;
	enqueue_list(browse_lists, blel);
}
/*=================================================
 *  remove_browse_list -- Remove named browse list.
 *===============================================*/
void
remove_browse_list (STRING name,
                    INDISEQ seq)
{
	BLEL blel;
	remove_indiseq(seq, FALSE);
	if (!name) return;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (blel->bl_name && eqstr(name, blel->bl_name)) {
			blel->bl_name = NULL;
			blel->bl_seq = NULL;
		}
	ENDLIST
}
/*===========================================
 *  find_named_seq -- Find named browse list.
 *=========================================*/
INDISEQ
find_named_seq (STRING name)
{
	BLEL blel;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (eqstr(name, blel->bl_name))
			return copy_indiseq(blel->bl_seq);
	ENDLIST
	return NULL;
}
/*===================================================
 *  new_name_browse_list -- Rename named browse list.
 *=================================================*/
void
new_name_browse_list (STRING old,
                      STRING new)
{
	BLEL blel;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (eqstr(old, blel->bl_name)) {
			stdfree(blel->bl_name);
			blel->bl_name = new;
			return;
		}
	ENDLIST
}
/*===================================================
 *  update_browse_list -- Assign name to browse list.
 *=================================================*/
void
update_browse_list (STRING name,
                    INDISEQ seq)
{
	BLEL blel;
	if (!name) {	/* remove anonymous lists */
		remove_indiseq(seq, FALSE);
		return;
	}
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (eqstr(name, blel->bl_name))
			blel->bl_seq = seq;
	ENDLIST
}
/*==============================================================
 * remove_from_browse_lists -- Remove stale elements from lists.
 *============================================================*/
void
remove_from_browse_lists (STRING key)
{
	BLEL blel;
	INDISEQ seq;
	if (current_seq) {
		seq = current_seq;
		while (delete_indiseq(seq, key, NULL, 0))
			;
	}
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		seq = blel->bl_seq;
		while (delete_indiseq(seq, key, NULL, 0))
			;
	ENDLIST
}
/*================================================================
 * rename_from_browse_lists -- Re-figures name of possible element
 *   in browse lists.
 *==============================================================*/
void
rename_from_browse_lists (STRING key)
{
	INDISEQ seq;
	BLEL blel;
	if (current_seq) {
		seq = current_seq;
		rename_indiseq(seq, key);
	}
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		seq = blel->bl_seq;
		rename_indiseq(seq, key);
	ENDLIST
}
