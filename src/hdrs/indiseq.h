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
/*=============================================================
 * indiseq.h -- Header file for the INDISEQ data type
 * Copyright(c) 1993-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 03 Aug 93
 *   3.0.2 - 06 Dec 94    3.0.3 - 08 Aug 95
 *===========================================================*/

#ifndef _INDISEQ_H
#define _INDISEQ_H

#include "standard.h"		/* for INT, STRING, etc */
#include "gedcom.h"		/* for NODE */

/*==================================================================
 * SORTEL -- Data type for indiseq elements; keys are always present
 *   and belong to the structure; names are always present for
 *   persons and belong; values do not belong to the structure
 *================================================================*/
typedef struct stag {
	STRING s_key;	/* person or family key */
	STRING s_nam;	/* name of person */
	WORD s_val;	/* any value */
	STRING s_prn;	/* menu print string */
	INT s_pri;	/* key as integer */
}
	*SORTEL;
#define skey(s) ((s)->s_key)
#define snam(s) ((s)->s_nam)
#define sval(s) ((s)->s_val)
#define sprn(s) ((s)->s_prn)
#define spri(s) ((s)->s_pri)

/*=================================================
 * INDISEQ -- Data type for an entire indi sequence
 *===============================================*/
typedef struct  {
	INT is_size;	/* current length of list  */
	INT is_max;	/* max length before increment */
	INT is_flags;	/* attribute flags */
	SORTEL *is_data;	/*  actual list of items */
	INT is_prntype; /* for special cases (spouseseq & famseq) */
} *INDISEQ;

#define ISize(s)  ((s)->is_size)
#define IMax(s)   ((s)->is_max)
#define IFlags(s) ((s)->is_flags)
#define IData(s)  ((s)->is_data)
#define IPrntype(s)  ((s)->is_prntype)

#define KEYSORT   (1<<0)	/* Values of attribute flags */
#define NAMESORT  (1<<1)
#define UNIQUED   (1<<2)
#define VALUESORT (1<<3)

#define length_indiseq(seq)  (ISize(seq))

/*=====================
 * INDISEQ -- Functions
 *===================*/

void add_browse_list(STRING, INDISEQ);
INDISEQ ancestor_indiseq(INDISEQ seq, WORD (*create_value_fnc)(INT gen));
void append_indiseq(INDISEQ, STRING, STRING, WORD, BOOLEAN, BOOLEAN);
INDISEQ child_indiseq(INDISEQ);
INDISEQ copy_indiseq(INDISEQ);
INDISEQ create_indiseq(void);
BOOLEAN delete_indiseq(INDISEQ, STRING, STRING, INT);
INDISEQ descendent_indiseq(INDISEQ seq, WORD (*create_value_fnc)(INT gen));
INDISEQ difference_indiseq(INDISEQ, INDISEQ);
BOOLEAN element_indiseq(INDISEQ, INT, STRING*, STRING*);
BOOLEAN elementval_indiseq(INDISEQ, INT, STRING*, INT *, STRING*);
INDISEQ fam_to_children(NODE);
INDISEQ fam_to_fathers(NODE);
INDISEQ fam_to_mothers(NODE);
INDISEQ find_named_seq(STRING);
INDISEQ get_all_even(void);
INDISEQ get_all_othe(void);
INDISEQ get_all_sour(void);
INT getpivot(INT, INT);
BOOLEAN in_indiseq(INDISEQ, STRING);
INDISEQ indi_to_children(NODE);
INDISEQ indi_to_families(NODE, BOOLEAN);
INDISEQ indi_to_fathers(NODE);
INDISEQ indi_to_mothers(NODE);
INDISEQ indi_to_spouses(NODE);
INDISEQ intersect_indiseq(INDISEQ, INDISEQ);
INDISEQ key_to_indiseq(STRING);
void keysort_indiseq(INDISEQ);
void llqsort(INT, INT);
INDISEQ name_to_indiseq(STRING);
void namesort_indiseq(INDISEQ);
void new_write_node(INT, NODE, BOOLEAN);
INDISEQ node_to_sources(NODE);
INDISEQ parent_indiseq(INDISEQ);
INT partition(INT, INT, SORTEL);
void partition_sort(SORTEL*, INT, INT (*func)(SORTEL, SORTEL));
void preprint_indiseq(INDISEQ);
void print_indiseq_element (INDISEQ seq, INT i, STRING buf, INT len);
INDISEQ refn_to_indiseq(STRING, INT letr, INT sort);
void remove_browse_list(STRING, INDISEQ);
void remove_indiseq(INDISEQ,BOOLEAN);
void rename_indiseq(INDISEQ, STRING);
INDISEQ sibling_indiseq(INDISEQ, BOOLEAN);
INDISEQ spouse_indiseq(INDISEQ);
INDISEQ str_to_indiseq(STRING);
void unique_indiseq(INDISEQ);
INDISEQ union_indiseq(INDISEQ, INDISEQ);
void update_browse_list(STRING, INDISEQ);
void valuesort_indiseq(INDISEQ, BOOLEAN*);
void write_family(STRING, TABLE);
void write_nonlink_indi(NODE);


/*==================
 * INDISEQ -- Macros
 *================*/

#define FORINDISEQ(s,e,i)\
	{	int i, _n;\
		SORTEL e, *_d;\
		_d = IData((INDISEQ)s);\
		for (i = 0, _n = ISize((INDISEQ)s); i < _n; i++) {\
			e = _d[i];
#define ENDINDISEQ }}

#endif /* _INDISEQ_H */
