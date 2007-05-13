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

CACHEEL even_to_cacheel(NODE);
CACHEEL fam_to_cacheel(RECORD frec);
CACHEEL fam_to_cacheel_old(NODE);
CACHEEL indi_to_cacheel(RECORD irec);
CACHEEL indi_to_cacheel_old(NODE);
CACHEEL record_to_cacheel(RECORD rec);
CACHEEL node_to_cacheel_old(NODE);
CACHEEL othr_to_cacheel(NODE);
CACHEEL sour_to_cacheel(NODE);

CACHEEL key_to_indi_cacheel(STRING);
CACHEEL key_to_fam_cacheel(STRING);
CACHEEL key_to_unknown_cacheel(STRING);
CACHEEL qkey_to_even_cacheel(STRING);
CACHEEL qkey_to_fam_cacheel(STRING);
CACHEEL qkey_to_indi_cacheel(STRING);
CACHEEL qkey_to_sour_cacheel(STRING);
CACHEEL qkey_to_othr_cacheel(STRING);

void lock_cache(CACHEEL);
void unlock_cache(CACHEEL);

void lock_record_in_cache(RECORD rec);
void unlock_record_from_cache(RECORD rec);

void lockrpt_cache(CACHEEL);
void unlockrpt_cache(CACHEEL);
INT cel_rptlocks(CACHEEL cel);
int free_all_rprtlocks(void);


CNSTRING cacheel_to_key(CACHEEL cel);
NODE cacheel_to_node(CACHEEL cel);

void set_all_nodetree_to_root_cel(NODE root);

#endif /* _CACHE_H */
