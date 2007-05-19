/* 
   Copyright (c) 2003 Matt Emmerton

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
 * pvalue.h -- PVALUE structures and prototypes
 * Copyright(c) 2003 by Matt Emmerton; all rights reserved
 *===========================================================*/
#ifndef _PVALUE_H
#define _PVALUE_H

#include "array.h"      /* for ARRAY */
#include "list.h"       /* for LIST */
#include "cache.h"      /* for CACHEEL */
#include "gedcom.h"     /* for NODE, RECORD */

#ifndef _INDISEQ_H
#include "indiseq.h"    /* for INDISEQ */
#endif

#include "table.h"      /* for TABLE */

typedef union {
	/* "basic" types, should be same as UNION */
        BOOLEAN b;
        INT     i;
        FLOAT   f;
	STRING	s;
        VPTR    p;	/* Try not to use this! */
	/* "complex" types */
	NODE	n;
	ARRAY	a;
	LIST	l;
	CACHEEL	c;
	RECORD	r;
	INDISEQ	q;
	TABLE	t;
} PVALUE_DATA;

typedef struct tag_pvalue *PVALUE;
struct tag_pvalue {
        struct tag_vtable * vtable;
        unsigned char type;     /* type of value */
        VPTR value;
        /* PVALUE_DATA value; */
};

/* PVALUE types */

#define PNULL      1  /* (freed pvalue) any value -- no type restriction - always NULL value */
#define PINT       2  /* integer */
/* PLONG==3 is obsolete */
#define PFLOAT     4  /* floating point */
#define PBOOL      5  /* boolean */
#define PSTRING    6  /* string */
#define PGNODE	   7  /* GEDCOM node */
#define PINDI      8  /* GEDCOM person record */
#define PFAM       9  /* GEDCOM family record */
#define PSOUR     10  /* GEDCOM source record */
#define PEVEN     11  /* GEDCOM event record */
#define POTHR     12  /* GEDCOM other record */
#define PLIST     13  /* list */
#define PTABLE    14  /* table */
#define PSET      15  /* set */
#define PARRAY    16  /* array */
#define PMAXLIVE  PARRAY /* maximum live type */
#define PFREED    99  /* returned to free list */

/* Handy PVALUE macros */
#define ptype(p)        ((p)->type)     /* type of expression */
#define pvalvv(p)       ((p)->value)    /* value of expression */

/* PVALUE Arithmetic Functions */
void add_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void sub_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void mul_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void div_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void mod_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void neg_pvalue(PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void decr_pvalue(PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void incr_pvalue(PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void exp_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void gt_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void ge_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void lt_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void le_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void ne_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);
void eq_pvalues(PVALUE, PVALUE, BOOLEAN*eflg, ZSTR * zerr);

/* PVALUE Functions */
void bad_type_error(CNSTRING op, ZSTR *zerr, PVALUE val1, PVALUE val2);
void coerce_pvalue(INT, PVALUE, BOOLEAN*);
PVALUE copy_pvalue(PVALUE);
PVALUE create_new_pvalue_list(void);
PVALUE create_new_pvalue_table(void);
PVALUE create_pvalue(INT, VPTR);
PVALUE create_pvalue_any(void);
PVALUE create_pvalue_from_bool(BOOLEAN bval);
PVALUE create_pvalue_from_cel(INT type, CACHEEL cel);
PVALUE create_pvalue_from_float(float fval);
PVALUE create_pvalue_from_even_keynum(INT i);
PVALUE create_pvalue_from_fam(NODE fam);
PVALUE create_pvalue_from_fam_key(STRING key);
PVALUE create_pvalue_from_fam_keynum(INT i);
PVALUE create_pvalue_from_indi(NODE indi);
PVALUE create_pvalue_from_indi_key(CNSTRING key);
PVALUE create_pvalue_from_indi_keynum(INT i);
PVALUE create_pvalue_from_int(INT ival);
PVALUE create_pvalue_from_list(LIST list);
PVALUE create_pvalue_from_node(NODE node);
PVALUE create_pvalue_from_othr_keynum(INT i);
PVALUE create_pvalue_from_seq(INDISEQ seq);
PVALUE create_pvalue_from_sour_keynum(INT i);
PVALUE create_pvalue_from_string(CNSTRING str);
PVALUE create_pvalue_from_zstr(ZSTR * pzstr);
PVALUE create_pvalue_from_table(TABLE tab);
ZSTR describe_pvalue(PVALUE);
void delete_vptr_pvalue(VPTR ptr);
void delete_pvalue(PVALUE);
void delete_pvalue_ptr(PVALUE * valp);
NODE remove_node_and_delete_pvalue(PVALUE *);
void eq_conform_pvalues(PVALUE, PVALUE, BOOLEAN*);
BOOLEAN eqv_pvalues(VPTR, VPTR);
BOOLEAN is_node_pvalue(PVALUE value);
BOOLEAN is_numeric_pvalue(PVALUE);
BOOLEAN is_pvalue(PVALUE);
BOOLEAN is_record_pvalue(PVALUE);
BOOLEAN is_numeric_zero(PVALUE);
void pvalues_begin(void);
void pvalues_end(void);

BOOLEAN pvalue_to_bool(PVALUE);
CACHEEL pvalue_to_cel(PVALUE val);
float pvalue_to_float(PVALUE val);
INT pvalue_to_int(PVALUE);
LIST pvalue_to_list(PVALUE val);
NODE pvalue_to_node(PVALUE val);
RECORD pvalue_to_record(PVALUE val);
INDISEQ pvalue_to_seq(PVALUE val);
STRING pvalue_to_string(PVALUE);
TABLE pvalue_to_table(PVALUE val);
struct tag_array *pvalue_to_array(PVALUE val);
void set_pvalue(PVALUE, INT, VPTR);
void set_pvalue_bool(PVALUE val, BOOLEAN bv);
void set_pvalue_float(PVALUE val, float fnum);
void set_pvalue_int(PVALUE val, INT iv);
void set_pvalue_to_pvalue(PVALUE val, const PVALUE src);
void set_pvalue_seq(PVALUE val, INDISEQ seq);
void set_pvalue_string(PVALUE val, CNSTRING str);
void show_pvalue(PVALUE);
INT which_pvalue_type(PVALUE);

PVALUE alloc_pvalue_memory(void);
void check_pvalue_validity(PVALUE val);
PVALUE create_new_pvalue(void);
void free_pvalue_memory(PVALUE val);
void set_pvalue_node(PVALUE val, NODE node);
INT pvalues_collate(PVALUE val1, PVALUE val2);
void init_pvalue_vtable(PVALUE val);

#endif /* _PVALUE_H */
