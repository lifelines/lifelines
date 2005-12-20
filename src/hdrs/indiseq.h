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
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 03 Aug 93
 *   3.0.2 - 06 Dec 94    3.0.3 - 08 Aug 95
 *===========================================================*/

#ifndef _INDISEQ_H
#define _INDISEQ_H

#include "standard.h"		/* for INT, STRING, etc */
#include "gedcom.h"		/* for NODE */

typedef struct tag_sortel *SORTEL;


typedef INT (*ELCMPFNC)(SORTEL el1, SORTEL el2, VPTR param);

/*=================================================
 * Custom functions for caller-specified handling of values
 * (These cater to the interpreter, which stores PVALUEs)
 *===============================================*/

/*====================
 * indiseq val types
 *  NULL means it has no typed values yet
 *==================*/
#define ISVAL_INT 1
#define ISVAL_STR 2
#define ISVAL_PTR 3
#define ISVAL_NUL 4

/*
	value function table functions for INDISEQ
	change with set_indiseq_value_funcs
	Default ones (exposed as default_xxx below):
		copy produces null values except for int type values
		delete only deletes string type values
		create_gen creates null values except for int type values
*/
/*
	copy a value
	(union will use copy from the first operand for keys in both operands)
*/
typedef UNION (*SEQ_COPY_VALUE_FNC)(UNION uval, INT valtype);
/* delete a value (failing uniqueness check in append */
typedef void (*SEQ_DELETE_VALUE)(UNION uval, INT valtype);
/* create a value for ancestors or descendants (may change value type from NUL) */
typedef UNION (*SEQ_CREATE_GEN_VALUE)(INT gen, INT * valtype);
/* compare two values */
typedef INT (*SEQ_COMPARE_VALUES)(VPTR ptr1, VPTR ptr2, INT valtype);
/* function table for handling values in INDISEQ */
typedef struct tag_indiseq_value_fnctable
{
	SEQ_COPY_VALUE_FNC copy_fnc;
	SEQ_DELETE_VALUE delete_fnc;
	SEQ_CREATE_GEN_VALUE create_gen_fnc;
	SEQ_COMPARE_VALUES compare_val_fnc;
} * INDISEQ_VALUE_FNCTABLE;


/*=================================================
 * INDISEQ -- Data type for an entire indi sequence
 *===============================================*/
struct tag_indiseq {
	INT is_refcnt;     /* for interp */
	INT is_size;       /* current length of list  */
	INT is_max;        /* max length before increment */
	INT is_flags;      /* attribute flags */
	SORTEL *is_data;   /*  actual list of items */
	INT is_prntype;    /* for special cases (spouseseq & famseq) */
	INT is_valtype;    /* int, string, pointer */
	STRING is_locale;  /* used by namesort */
	INDISEQ_VALUE_FNCTABLE is_valfnctbl;
};
#ifndef INDISEQ_type_defined
typedef struct tag_indiseq *INDISEQ;
#define INDISEQ_type_defined
#endif

#define IRefcnt(s)   ((s)->is_refcnt)
#define ISize(s)     ((s)->is_size)
#define IMax(s)      ((s)->is_max)
#define IFlags(s)    ((s)->is_flags)
#define IData(s)     ((s)->is_data)
#define IPrntype(s)  ((s)->is_prntype)
#define IValtype(s)  ((s)->is_valtype)
#define ILocale(s)   ((s)->is_locale)
#define IValfnctbl(s) ((s)->is_valfnctbl)

#define KEYSORT       (1<<0)
#define NAMESORT      (1<<1)
#define UNIQUED       (1<<2)
#define VALUESORT     (1<<3)
#define CANONKEYSORT  (1<<4)
#define WITHNAMES     (1<<5)
#define ALLSORTS (KEYSORT+NAMESORT+VALUESORT+CANONKEYSORT)

#define length_indiseq(seq)  (ISize(seq))

/*=====================
 * INDISEQ -- Functions
 *===================*/

void add_browse_list(STRING, INDISEQ);
INDISEQ ancestor_indiseq(INDISEQ seq);
void append_indiseq_null(INDISEQ, STRING key, CNSTRING name, BOOLEAN sure, BOOLEAN alloc);
void append_indiseq_ival(INDISEQ, STRING key, STRING name, INT val, BOOLEAN sure, BOOLEAN alloc);
void append_indiseq_pval(INDISEQ, STRING key, STRING name, VPTR val, BOOLEAN sure);
void append_indiseq_sval(INDISEQ, STRING key, CNSTRING name, STRING sval, BOOLEAN sure, BOOLEAN alloc);
void calc_indiseq_names(INDISEQ seq);
void canonkeysort_indiseq(INDISEQ);
INDISEQ child_indiseq(INDISEQ);
INDISEQ copy_indiseq(INDISEQ seq);
INDISEQ create_indiseq_ival(void);
INDISEQ create_indiseq_null(void);
INDISEQ create_indiseq_pval(void);
INDISEQ create_indiseq_sval(void);
UNION default_copy_value(UNION uval, INT valtype);
void default_delete_value(UNION uval, INT valtype);
UNION default_create_gen_value(INT gen, INT * valtype);
INT default_compare_values(VPTR ptr1, VPTR ptr2, INT valtype);
BOOLEAN delete_indiseq(INDISEQ, STRING, STRING, INT);
INDISEQ descendent_indiseq(INDISEQ seq);
INDISEQ difference_indiseq(INDISEQ, INDISEQ);
INT element_ikey(SORTEL el);
BOOLEAN element_indiseq(INDISEQ, INT, STRING*, STRING*);
BOOLEAN element_indiseq_ival(INDISEQ seq, INT index, STRING*, INT *, STRING*);
CNSTRING element_key_indiseq(INDISEQ seq, INT index);
CNSTRING element_name(SORTEL el);
VPTR element_pval(SORTEL el);
CNSTRING element_skey(SORTEL el);
CNSTRING element_sval(SORTEL el);
INDISEQ fam_to_children(NODE);
INDISEQ fam_to_fathers(NODE);
INDISEQ fam_to_mothers(NODE);
INDISEQ find_named_seq(STRING);
INDISEQ get_all_even(void);
INDISEQ get_all_othe(void);
INDISEQ get_all_sour(void);
INT get_indiseq_ival(INDISEQ seq, INT i);
INT getpivot(INT, INT);
BOOLEAN in_indiseq(INDISEQ, STRING);
INDISEQ indi_to_children(NODE);
INDISEQ indi_to_families(NODE, BOOLEAN);
INDISEQ indi_to_fathers(NODE);
INDISEQ indi_to_mothers(NODE);
INDISEQ indi_to_spouses(NODE);
BOOLEAN indiseq_is_valtype_ival(INDISEQ);
BOOLEAN indiseq_is_valtype_null(INDISEQ);
INDISEQ intersect_indiseq(INDISEQ, INDISEQ);
INDISEQ key_to_indiseq(STRING name, char ctype);
void keysort_indiseq(INDISEQ);
INDISEQ name_to_indiseq(STRING);
void namesort_indiseq(INDISEQ);
void new_write_node(INT, NODE, BOOLEAN);
INDISEQ node_to_notes(NODE);
INDISEQ node_to_pointers(NODE node);
INDISEQ node_to_sources(NODE);
INDISEQ parent_indiseq(INDISEQ);
void partition_sort(SORTEL*, INT, ELCMPFNC func, VPTR param);
void preprint_indiseq(INDISEQ, INT len, RFMT rfmt);
void print_indiseq_element (INDISEQ seq, INT i, STRING buf, INT len, RFMT rfmt);
INDISEQ refn_to_indiseq(STRING, INT letr, INT sort);
void remove_browse_list(STRING, INDISEQ);
void remove_indiseq(INDISEQ);
void rename_indiseq(INDISEQ, STRING);
void set_element_pval(SORTEL el, VPTR ptr);
void set_indiseq_value_funcs(INDISEQ seq, INDISEQ_VALUE_FNCTABLE valfnctbl);
INDISEQ sibling_indiseq(INDISEQ, BOOLEAN);
INDISEQ spouse_indiseq(INDISEQ);
INDISEQ str_to_indiseq(STRING name, char ctype);
void unique_indiseq(INDISEQ);
INDISEQ union_indiseq(INDISEQ one, INDISEQ two);
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
