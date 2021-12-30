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
/*=============================================================
 * pvalue.c -- Handle program typed values
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.3 - 03 Jul 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interpi.h"
#include "liflines.h"
#include "feedback.h"
#include "zstr.h"
#include "vtable.h"
#include "array.h"
#include "object.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static FLOAT bool_to_float(BOOLEAN);
static INT bool_to_int(BOOLEAN);
static void clear_pv_indiseq(INDISEQ seq);
static PVALUE create_pvalue_from_keynum_impl(INT i, INT ptype);
static PVALUE create_pvalue_from_key_impl(CNSTRING key, INT ptype);
static PVALUE create_pvalue_from_record(RECORD rec, INT ptype);
/* static BOOLEAN eq_pstrings(PVALUE val1, PVALUE val2); */
static int float_to_int(float f);
static BOOLEAN is_record_pvaltype(INT valtype);
static OBJECT pvalue_copy(OBJECT obj, int deep);
static void pvalue_destructor(VTABLE *obj);
static void release_pvalue_contents(PVALUE val);
static void set_pvalue(PVALUE val, INT type, PVALUE_DATA pvd);

/*********************************************
 * local variables
 *********************************************/

/* These names are offset by the number of their type */
/* PFLOAT == 4, so "PFLOAT" must be at array offset 4 */
static char *ptypes[] = {
	"?", "PNULL", "PINT", "PLONG", "PFLOAT", "PBOOL", "PSTRING",
	"PGNODE", "PINDI", "PFAM", "PSOUR", "PEVEN", "POTHR", "PLIST",
	"PTABLE", "PSET", "PARRAY"
};
static struct tag_vtable vtable_for_pvalue = {
	VTABLE_MAGIC
	, "pvalue"
	, &pvalue_destructor
	, &nonrefcountable_isref
	, 0
	, 0
	, &pvalue_copy /* copy_fnc */
	, &generic_get_type_name
};


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*========================================
 * create_pvalue -- Create a program value
 *======================================*/
PVALUE
create_pvalue (INT type, PVALUE_DATA pvd)
{
	PVALUE val = create_new_pvalue();
	set_pvalue(val, type, pvd);
	return val;
}
/*==================================
 * set_pvalue -- Set a program value
 *  val:   [I/O] pvalue getting new info
 *  type:  [IN]  new type for pvalue
 *  value: [IN]  new value for pvalue
 *================================*/
static void
set_pvalue (PVALUE val, INT type, PVALUE_DATA pvd)
{
	/* for simple types, we can simply assign */
	/* but for indirect/pointer types, we must beware of self-assignment */
	if (type == PNULL) {
		clear_pvalue(val);
		return;
	} else if (type == PINT) {
		clear_pvalue(val);
		val->type = PINT;
		val->value.ixd = pvd.ixd;
	} else if (type == PFLOAT) {
		clear_pvalue(val);
		val->type = PFLOAT;
		val->value.fxd = pvd.fxd;
	} else if (type == PBOOL) {
		clear_pvalue(val);
		val->type = PBOOL;
		val->value.bxd = pvd.bxd;
	} else if (type == PSTRING) {
		STRING str = pvd.sxd;
		/* strings are always copied, no self-assignment issue */
		clear_pvalue(val);
		val->type = PSTRING;
		if (str)
			val->value.sxd = strsave(str);
		else
			val->value.sxd = 0;
	} else if (type == PGNODE) {
		NODE node = pvd.nxd;
		if (val->type == PGNODE && val->value.nxd == node)
			return; /* self-assignment */
		clear_pvalue(val);
		val->type = PGNODE;
		val->value.nxd = node;
		if (node) {
			++nrefcnt(node);
			dolock_node_in_cache(node, TRUE);
		}
	} else if (is_record_pvaltype(type)) { /* PINDI, PFAM, PSOUR, PEVEN, POTHR */
		RECORD rec = pvd.rxd;
		if (val->type == type && val->value.rxd == rec)
			return; /* self-assignment */
		clear_pvalue(val);
		val->type = type;
		val->value.rxd = rec;
		if (rec) {
			addref_record(rec);
		}
	} else if (type == PLIST) {
		LIST list = pvd.lxd;
		if (val->type == PLIST && val->value.lxd == list)
			return; /* self-assignment */
		clear_pvalue(val);
		val->type = PLIST;
		val->value.lxd = list;
		if (list) {
			addref_list(list);
		}
	} else if (type == PTABLE) {
		TABLE table = pvd.txd;
		if (val->type == PTABLE && val->value.txd == table)
			return; /* self-assignment */
		clear_pvalue(val);
		val->type = PTABLE;
		val->value.txd = table;
		if (table) {
			addref_table(table);
		}
	} else if (type == PSET) {
		INDISEQ seq = pvd.qxd;
		if (val->type == PSET && val->value.qxd == seq)
			return; /* self-assignment */
		clear_pvalue(val);
		val->type = PSET;
		val->value.qxd = seq;
		if (seq) {
			addref_indiseq(seq);
		}
	} else if (type == PARRAY) {
		ARRAY arr = pvd.axd;
		if (val->type == PARRAY && val->value.axd == arr)
			return; /* self-assignment */
		clear_pvalue(val);
		val->type = PARRAY;
		val->value.axd = arr;
		if (arr) {
			addref_array(arr);
		}
	}
}
/*========================================
 * dolock_node_in_cache -- Lock/unlock node in cache
 *  (if possible)
 * Created: 2003-02-04 (Perry Rapp)
 *======================================*/
void
dolock_node_in_cache (HINT_PARAM_UNUSED NODE node, HINT_PARAM_UNUSED BOOLEAN lock)
{
#if NOT_WORKING_ON_LARGE_DATA_SETS
/* This leads to cache overflow, so there is something
wrong here - Perry, 2003-03-07 */
	if (node) {
		RECORD rec = node->n_rec;
		if (rec) {
			CACHEEL cel = rec->cel;
			if (cel) {
				if (lock)
					lock_cache(cel);
				else
					unlock_cache(cel);
			}
		}
	}
#endif /* NOT_WORKING_ON_LARGE_DATA_SETS */
}
/*========================================
 * clear_pvalue -- Set pvalue to null
 *  releasing any contents
 * Created: 2007/12/19, Perry Rapp
 *======================================*/
void
clear_pvalue (PVALUE val)
{
	release_pvalue_contents(val);
	val->type = PNULL;
	val->value.pxd = 0;
}
/*========================================
 * release_pvalue_contents -- Empty contents of pvalue
 *  This doesn't bother to clear val->value
 *  because caller will do so
 * Created: 2001/01/20, Perry Rapp
 *======================================*/
static void
release_pvalue_contents (PVALUE val)
{
	check_pvalue_validity(val);
	switch (ptype(val)) {
	/*
	embedded values have no referenced memory to clear
	PINT, PBOOLEAN, PFLOAT
	*/
	/*
	PNULL is a null value
	*/
	case PGNODE:
		{
			NODE node = pvalue_to_node(val);
			if (node) {
				dolock_node_in_cache(node, FALSE);
				--nrefcnt(node);
				if (!nrefcnt(node) && is_temp_node(node)) {
					free_temp_node_tree(node);
				}
			}
		}
		return;
	case PSTRING:
		{
			STRING str = pvalue_to_string(val);
			if (str) {
				stdfree(str);
			}
		}
		return;
	case PLIST:
		{
			LIST list = pvalue_to_list(val);
			release_list(list);
		}
		return;
	case PTABLE:
		{
			TABLE table = pvalue_to_table(val);
			release_table(table);
		}
		return;
	case PSET:
		{
			INDISEQ seq = pvalue_to_seq(val);
			/* because of getindiset, seq might be NULL */
			if (seq) {
				--IRefcnt(seq);
				if (!IRefcnt(seq)) {
					clear_pv_indiseq(seq);
					remove_indiseq(seq);
				}
			}
		}
		return;
	/* record nodes handled below (PINDI, PFAM, PSOUR, PEVEN, POTHR) */
	}
	if (is_record_pvalue(val)) {
		RECORD rec = pvalue_to_record(val);
		release_record(rec);
	}
}
/*========================================
 * clear_pv_indiseq -- Clear PVALUES from indiseq
 * Created: 2001/03/24, Perry Rapp
 *======================================*/
static void
clear_pv_indiseq (INDISEQ seq)
{
	PVALUE val=NULL;
	/* NUL value indiseqs can get into reports via getindiset */
	ASSERT(IValtype(seq) == ISVAL_PTR || IValtype(seq) == ISVAL_NUL);
	FORINDISEQ(seq, el, ncount)
		val = (PVALUE)element_pval(el);
		if (val) {
			delete_pvalue(val);
			set_element_pval(el, NULL);
		}
	ENDINDISEQ
}
/*========================================
 * table_pvcleaner -- Clean pvalue entries
 *  from table
 * Created: 2001/03/24, Perry Rapp
 *======================================*/
#ifdef UNUSED
static void
table_pvcleaner (HINT_PARAM_UNUSED CNSTRING key, UNION uval)
{
	PVALUE val = uval.w;
	delete_pvalue(val);
	uval.w = NULL;
}
#endif
/*========================================
 * delete_vptr_pvalue -- Delete a program value
 *  (passed in as a VPTR)
 * Created: 2001/03/24, Perry Rapp
 *======================================*/
void
delete_vptr_pvalue (VPTR ptr)
{
	PVALUE val = (PVALUE)ptr;
	delete_pvalue(val);
}
/*========================================
 * remove_node_and_delete_pvalue -- Remove
 *  node inside pvalue, and delete pvalue
 *======================================*/
NODE
remove_node_and_delete_pvalue (PVALUE * pval)
{
	NODE node=0;
	if (*pval) {
		PVALUE vl= *pval;
		node = vl->value.nxd;
		vl->value.nxd = 0;
		delete_pvalue(vl);
	}
	*pval = 0;
	return node;
}
/*========================================
 * delete_pvalue -- Delete a program value
 * see create_pvalue - Perry Rapp, 2001/01/19
 *======================================*/
void
delete_pvalue (PVALUE val)
{
	if (!val) return;
	clear_pvalue(val);
	free_pvalue_memory(val);
}
/*========================================
 * delete_pvalue_ptr -- Delete & clear a program value
 * Created: 2003-01-30 (Perry Rapp)
 *======================================*/
void
delete_pvalue_ptr (PVALUE * valp)
{
	if (valp) {
		delete_pvalue(*valp);
		*valp = 0;
	}
}
/*====================================
 * copy_pvalue -- Create a new pvalue & copy into it
 *  handles NULL
 * delegates all the real work to create_pvalue
 *==================================*/
PVALUE
copy_pvalue (PVALUE val)
{
	if (!val)
		return NULL;
	return create_pvalue(ptype(val), pvalvv(val));
}
/*=====================================================
 * create_pvalue_from_indi -- Return indi as pvalue
 *  handles NULL
 * Created: 2001/03/18, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_indi (NODE indi)
{
	if (indi)
		return create_pvalue_from_indi_key(indi_to_key(indi));
	else
		return create_pvalue_of_null_indi();
}
/*=====================================================
 * create_pvalue_of_null_indi -- Return pvalue of null indi
 * That is, a pvalue of INDI type pointing to nothing
 * Created: 2007/12/19, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_of_null_indi (void)
{
	PVALUE_DATA pvd;
	pvd.rxd = 0;
	return create_pvalue(PINDI, pvd);
}
/*=====================================================
 * create_pvalue_of_null_fam -- Return pvalue of null fam
 * That is, a pvalue of FAM type pointing to nothing
 * Created: 2007/12/19, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_of_null_fam (void)
{
	PVALUE_DATA pvd;
	pvd.rxd = 0;
	return create_pvalue(PFAM, pvd);
}
/*=====================================================
 * create_pvalue_from_indi_key
 *  handles NULL
 * Created: 2000/12/30, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_indi_key (CNSTRING key)
{
	return create_pvalue_from_key_impl(key, PINDI);
}
/*=====================================================
 * create_pvalue_from_fam_key
 *  handles NULL
 *===================================================*/
PVALUE
create_pvalue_from_fam_key (STRING key)
{
	return create_pvalue_from_key_impl(key, PFAM);
}
/*=====================================================
 * create_pvalue_from_cel
 * Created: 2002/02/17, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_cel (INT type, CACHEEL cel)
{
	PVALUE val=0;
	RECORD rec = cel ? get_record_for_cel(cel) : 0;
	val = create_pvalue_from_record(rec, type);
	release_record(rec); /* ownership transferred to pvalue */
	return val;
}
/*=====================================================
 * create_pvalue_from_indi_keynum -- Return indi as pvalue
 *  helper for __firstindi etc
 *  handles i==0
 * Created: 2000/12/30, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_indi_keynum (INT i)
{
	return create_pvalue_from_keynum_impl(i, PINDI);
}
/*=====================================================
 * create_pvalue_from_fam -- Return fam as pvalue
 *  handles NULL
 * Created: 2001/03/18, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_fam (NODE fam)
{
	if (fam)
		return create_pvalue_from_fam_key(fam_to_key(fam));
	else
		return create_pvalue_of_null_fam();
}
/*====================================================
 * create_pvalue_from_fam_keynum -- Return indi as pvalue
 *  helper for __firstfam etc
 *  handles i==0
 * Created: 2000/12/30, Perry Rapp
 *==================================================*/
PVALUE
create_pvalue_from_fam_keynum (INT i)
{
	return create_pvalue_from_keynum_impl(i, PFAM);
}
/*=====================================================
 * create_pvalue_from_sour_keynum -- Return new pvalue for source
 * Created: 2001/03/20, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_sour_keynum (INT i)
{
	return create_pvalue_from_keynum_impl(i, PSOUR);
}
/*=====================================================
 * create_pvalue_from_even_keynum -- Return new pvalue for event
 * Created: 2001/03/23, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_even_keynum (INT i)
{
	return create_pvalue_from_keynum_impl(i, PEVEN);
}
/*=====================================================
 * create_pvalue_from_othr_keynum -- Return new pvalue for other
 * Created: 2001/11/11, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_othr_keynum (INT i)
{
	return create_pvalue_from_keynum_impl(i, POTHR);
}
/*=====================================================
 * create_pvalue_from_record -- Create pvalue from any node
 *  handles NULL
 * If rec is not null, it is given to new pvalue to own
 *===================================================*/
static PVALUE
create_pvalue_from_record (RECORD rec, INT ptype)
{
	PVALUE_DATA pvd;
	/* record pvalues simply point to their heap-alloc'd record */
	pvd.rxd = rec;
	return create_pvalue(ptype, pvd);
}
/*====================================================
 * create_pvalue_from_keynum_impl -- Create pvalue for any type
 * Created: 2001/03/20, Perry Rapp
 *==================================================*/
static PVALUE
create_pvalue_from_keynum_impl (INT i, INT ptype)
{
	static char key[10];
	char cptype = 'Q';
	if (!i)
		return create_pvalue_from_record(NULL, ptype);
	switch(ptype) {
	case PINDI: cptype = 'I'; break;
	case PFAM: cptype = 'F'; break;
	case PSOUR: cptype = 'S'; break;
	case PEVEN: cptype = 'E'; break;
	case POTHR: cptype = 'X'; break;
	default: ASSERT(0); break;
	}
	snprintf(key, sizeof(key), "%c" FMT_INT, cptype, i);
	return create_pvalue_from_key_impl(key, ptype);
}
/*==================================
 * create_pvalue_from_key_impl -- Create pvalue from any key
 * Created: 2001/03/20, Perry Rapp
 *================================*/
static PVALUE
create_pvalue_from_key_impl (CNSTRING key, INT ptype)
{
	/* report mode, so may return NULL */
	RECORD rec = qkey_to_record(key); /* addref'd record */
	PVALUE val = create_pvalue_from_record(rec, ptype);
	release_record(rec); /* release our reference, now only pvalue holds */
	return val;
}
/*==================================================
 * is_numeric_pvalue -- See if program value is numeric
 *================================================*/
BOOLEAN
is_numeric_pvalue (PVALUE val)
{
	INT type = ptype(val);
	return type == PINT || type == PFLOAT || type == PBOOL || type == PNULL;
}
/*===========================================================
 * eq_conform_pvalues -- Make the types of two values conform
 *=========================================================*/
void
eq_conform_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	INT hitype;

	ASSERT(val1);
	ASSERT(val2);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PNULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PNULL)
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PINT && pvalue_to_int(val1) == 0 && !is_numeric_pvalue(val2))
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PINT && pvalue_to_int(val2) == 0 && !is_numeric_pvalue(val1))
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (is_numeric_pvalue(val1) && is_numeric_pvalue(val2)) {
		hitype = max(ptype(val1), ptype(val2));
		if (ptype(val1) != hitype) coerce_pvalue(hitype, val1, eflg);
		if (ptype(val2) != hitype) coerce_pvalue(hitype, val2, eflg);
		return;
	}
	*eflg = TRUE;
}
/*===========================================================
 * get_pvalue_as_bool -- Return boolean equivalent of value
 *=========================================================*/
static BOOLEAN
get_pvalue_as_bool (PVALUE val)
{
	switch(val->type)
	{
	case PNULL: return FALSE;
	case PINT: return val->value.ixd != 0;
	case PFLOAT: return val->value.fxd != 0;
	case PBOOL: return val->value.bxd;
	case PSTRING: return val->value.sxd != 0;
	case PGNODE: return val->value.nxd != 0;
	case PINDI:
	case PFAM:
	case PSOUR:
	case PEVEN:
	case POTHR:
		return val->value.rxd != 0;
	case PLIST: return pvalue_to_list(val) != 0;
	case PTABLE: return pvalue_to_table(val) != 0;
	case PSET: return pvalue_to_seq(val) != 0;
	case PARRAY: return val->value.axd != 0;
	}
	return FALSE;
}
/*=========================================================
 * coerce_pvalue -- Convert PVALUE from one type to another
 *  type:  [in] type to convert to
 *  val:   [in,out] value to convert in place
 *  eflg:  [out] error flag (set to TRUE if error)
 *=======================================================*/
void
coerce_pvalue (INT type, PVALUE val, BOOLEAN *eflg)
{
	if (*eflg) return;
	ASSERT(is_pvalue(val));

	if (type == ptype(val)) return; /* no coercion needed */

	if (type == PBOOL) {
		BOOLEAN boo = get_pvalue_as_bool(val);
		set_pvalue_bool(val, boo);
		return;
	}
	/* Anything is convertible to PNULL */
	/* Perry, 2002.02.16: This looks suspicious to me, but I 
	don't know how it is used -- it might be used in some
	eq_conform_pvalues call(s) ? */
	if (type == PNULL) {
		ptype(val) = PNULL;
		return;
	}

	/* PNULL or PINT with NULL (0) value is convertible to any scalar (1995.07.31) */
	if (ptype(val) == PNULL || (ptype(val) == PINT && pvalue_to_int(val) == 0)) {
		if (type == PSET || type == PTABLE || type == PLIST) goto bad;
		/*
		  INTs convert to FLOATs numerically further down, no special 
		  conversion when INT value 0
		  (2003-06-08)
		*/
		if (type != PFLOAT) {
			ptype(val) = type;
			return;
		}
	}

	/* Any record is convertible to PGNODE (2002.02.16) */
	if (type == PGNODE) {
		if (is_record_pvalue(val) && record_to_node(val)) {
			return;
		} else {
			/* nothing else is convertible to PGNODE */
			goto bad;
		}
	}

	switch (ptype(val)) { /* switch on what we have */

	case PINT:
		if (type == PFLOAT) {
			/* PINT is convertible to PFLOAT */
			float flo = pvalue_to_int(val);
			set_pvalue_float(val, flo);
			return;
		} else {
			/* PINT isn't convertible to anything else */
			goto bad;
		}
		break;
	case PFLOAT:
		if (type == PINT) {
			/* PFLOAT is convertible to PINT */
			INT inum = float_to_int(pvalue_to_float(val));
			set_pvalue_int(val, inum);
			return;
		} else {
			/* PFLOAT isn't convertible to anything else */
			goto bad;
		}
		break;
	case PBOOL:
		if (type == PINT) {
			/* PBOOL is convertible to PINT */
			INT inum = bool_to_int(pvalue_to_bool(val));
			set_pvalue_int(val, inum);
			return;
		} else if (type == PFLOAT) {
			/* PBOOL is convertible to PFLOAT */
			float fnum = bool_to_float(pvalue_to_bool(val));
			set_pvalue_float(val, fnum);
			return;
		} else {
			/* PBOOL isn't convertible to anything else */
			goto bad;
		}
		break;
	/* Nothing else is convertible to anything else */
	/* record types (PINDI...), PNULL, PGNODE */
	}

	/* fall through to failure */

bad:
	*eflg = TRUE;
	return;
}
/*========================================
 * which_pvalue_type -- Return type number
 *======================================*/
INT
which_pvalue_type (PVALUE val)
{
	ASSERT(val);
	return ptype(val);
}
/*========================================
 * is_node_pvalue -- Does pvalue contain PGNODE ?
 *======================================*/
BOOLEAN
is_node_pvalue (PVALUE value)
{
	return ptype(value) == PGNODE;
}
/*========================================
 * is_record_pvalue -- Does pvalue contain record ?
 *======================================*/
BOOLEAN
is_record_pvalue (PVALUE value)
{
	return is_record_pvaltype(ptype(value));
}
/*========================================
 * is_record_pvaltype -- Does pvalue contain record ?
 *======================================*/
static BOOLEAN
is_record_pvaltype (INT valtype)
{
	switch (valtype) {
	case PINDI: case PFAM: case PSOUR: case PEVEN: case POTHR:
		return TRUE;
	}
	return FALSE;
}
/*========================================
 * Trivial conversions
 *======================================*/
static INT
bool_to_int (BOOLEAN b)
{
	return b ? 1 : 0;
}
static FLOAT
bool_to_float (BOOLEAN b)
{
	return b ? 1. : 0.;
}
static int
float_to_int (float f)
{
	return (int)f;
}
/*===================================================================+
 * eqv_pvalues -- See if two PVALUEs are equal (no change to PVALUEs)
 *==================================================================*/
BOOLEAN
eqv_pvalues (VPTR ptr1, VPTR ptr2)
{
	PVALUE val1=ptr1, val2=ptr2;
	STRING v1, v2;
	BOOLEAN rel = FALSE;
	if(val1 && val2 && (ptype(val1) == ptype(val2))) {
		switch (ptype(val1)) {
		/* types with value semantics do value comparison */
		case PSTRING:
			v1 = pvalue_to_string(val1);
			v2 = pvalue_to_string(val2);
			if(v1 && v2) rel = eqstr(v1, v2);
			else rel = (v1 == v2);
			break;
		case PFLOAT:
			rel = (pvalue_to_float(val1) == pvalue_to_float(val2));
			break;
		case PINT:
			rel = (pvalue_to_int(val1) == pvalue_to_int(val2));
			break;
		case PBOOL:
			rel = (pvalue_to_bool(val1) == pvalue_to_bool(val2));
			break;
		case PINDI: case PFAM: case PSOUR: case PEVEN: case POTHR:
		{
		    RECORD rec1,rec2;
		    rec1 = pvalue_to_record(val1);
		    rec2 = pvalue_to_record(val2);
		    if (rec1 && rec2) rel = eqstrn(nzkey(rec1),nzkey(rec2),MAXKEYWIDTH+1);
		    else rel = (rec1  == rec2);
		    break;
		 }
		/* types with pointer semantics do pointer comparison */
		case PLIST:
			return pvalue_to_list(val1) == pvalue_to_list(val2);
		case PTABLE:
			return pvalue_to_table(val1) == pvalue_to_table(val2);
		case PSET:
			return pvalue_to_seq(val1) == pvalue_to_seq(val2);
		case PARRAY:
			return pvalue_to_array(val1) == pvalue_to_array(val2);
		case PGNODE:
			return pvalue_to_node(val1) == pvalue_to_node(val2);
		}
	}
	return rel;
}
/*===========================================
 * bad_type_error -- Set error description
 *  for types that cannot be compared
 * Created: 2003-01-30 (Perry Rapp)
 *=========================================*/
void
bad_type_error (CNSTRING op, ZSTR *zerr, PVALUE val1, PVALUE val2)
{
	if (zerr) {
		ZSTR zt1 = describe_pvalue(val1), zt2 = describe_pvalue(val2);
		ASSERT(!(*zerr));
		(*zerr) = zs_newf(_("%s: Incomparable types: %s and %s")
			, op, zs_str(zt1), zs_str(zt2));
		zs_free(&zt1);
		zs_free(&zt2);
	}
}
/*===============================================
 * eq_pstrings -- Compare two PSTRINGS
 *  Caller is responsible for ensuring these are PSTRINGS
 *=============================================*/
/* unused
static BOOLEAN
eq_pstrings (PVALUE val1, PVALUE val2)
{
	STRING str1 = pvalue_to_string(val1);
	STRING str2 = pvalue_to_string(val2);
	if (!str1) str1 = "";
	if (!str2) str2 = "";
	return eqstr(str1, str2);
}
unused */
/*===========================================
 * eq_pvalues -- See if two PVALUEs are equal
 * Result into val1, deletes val2
 *=========================================*/
void
eq_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	BOOLEAN rel;

	if (*eflg) return;
	eq_conform_pvalues(val1, val2, eflg);
	if (*eflg) {
		bad_type_error("eq", zerr, val1, val2);
		return;
	}
	rel = eqv_pvalues(val1, val2);

	/* Now store answer into val1, and delete val2 */
	set_pvalue_bool(val1, rel);
	delete_pvalue(val2);
}
/*===============================================
 * ne_pvalues -- See if two PVALUEs are not equal
 * Result into val1, deletes val2
 *=============================================*/
void
ne_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg, ZSTR * zerr)
{
	BOOLEAN rel;

	if (*eflg) return;
	eq_conform_pvalues(val1, val2, eflg);
	if (*eflg) {
		bad_type_error("ne", zerr, val1, val2);
		return;
	}
	rel = !eqv_pvalues(val1, val2);

	/* Now store answer into val1, and delete val2 */
	set_pvalue_bool(val1, rel);
	delete_pvalue(val2);
}
/*=================================================
 * show_pvalue -- DEBUG routine that shows a PVALUE
 *===============================================*/
void
show_pvalue (PVALUE val)
{
	ZSTR zstr = describe_pvalue(val);
	llwprintf("%s", zs_str(zstr));
	zs_free(&zstr);
}
/*=================================================
 * get_pvalue_type_name -- Return static string name of pvalue type
 *  eg, get_pvalue_type_name(PTABLE) => "PTABLE"
 *===============================================*/
CNSTRING
get_pvalue_type_name (INT ptype)
{
	if (ptype >= 0 && ptype <= ARRSIZE(ptypes)) {
		return ptypes[ptype];
	} else {
		return "INVALID ptype";
	}
}
/*======================================================
 * debug_pvalue_as_string -- DEBUG routine that shows a PVALUE
 *  returns zstring (dynamic string)
 *====================================================*/
ZSTR
describe_pvalue (PVALUE val)
{
	INT type;
	ZSTR zstr = zs_new();

	if (!val) {
		zs_sets(zstr, _("NOT PVALUE: NULL!"));
		return zstr;
	}
	if (!is_pvalue(val)) {
		zs_setf(zstr, _("NOT PVALUE: invalid type=%d)!"), ptype(val));
		return zstr;
	}
	type = ptype(val);
	zs_appc(zstr, '<');
	zs_apps(zstr, get_pvalue_type_name(type));
	zs_appc(zstr, ',');

	switch (type) {
	case PNULL:
		zs_appf(zstr, "<NULL>");
		break;
	case PINT:
		zs_appf(zstr, FMT_INT, pvalue_to_int(val));
		break;
	case PFLOAT:
		zs_appf(zstr, "%f", pvalue_to_float(val));
		break;
	case PBOOL:
		zs_apps(zstr, pvalue_to_bool(val) ? _("True") : _("False"));
		break;
	case PSTRING:
		zs_appf(zstr, "\"%s\"", pvalue_to_string(val));
		break;
	case PGNODE:
		{
			NODE node = pvalue_to_node(val);
			if (!node)
				zs_apps(zstr, "NULL");
			else {
				STRING tag = ntag(node);
				if (!tag)
					zs_apps(zstr, "null tag");
				else
					zs_appf(zstr, "tag='%s'", tag);
			}
		}
		break;
	case PINDI:
	case PFAM:
	case PSOUR:
	case PEVEN:
	case POTHR:
		{
			RECORD rec = pvalue_to_record(val);
			if (rec)
				zs_appf(zstr, "%s", nzkey(rec));
			else
				zs_appf(zstr, "NULL");
		}
		break;
	case PLIST:
		{
			LIST list = pvalue_to_list(val);
			INT n = length_list(list);
			zs_appf(zstr, _pl(FMT_INT " item", FMT_INT " items", n), n);
		}
		break;
	case PTABLE:
		{
			TABLE table = pvalue_to_table(val);
			INT n = get_table_count(table);
			zs_appf(zstr, _pl(FMT_INT " entry", FMT_INT " entries", n), n);
		}
		break;
	case PSET:
		{
			INDISEQ seq = pvalue_to_seq(val);
			INT n = length_indiseq(seq);
			zs_appf(zstr, _pl(FMT_INT " record", FMT_INT " records", n), n);
		}
		break;
	case PARRAY:
		{
			ARRAY arr = pvalue_to_array(val);
			INT n = get_array_size(arr);
			zs_appf(zstr, _pl(FMT_INT " element", FMT_INT " elements", n), n);
		}
		break;
	default:
		zs_appf(zstr, "%p", (void*)&pvalvv(val));
		break;
	}
	zs_appc(zstr, '>');
	return zstr;
}
/*==================================
 * PNULL: pvalue with no content
 *================================*/
PVALUE
create_pvalue_any (void)
{
	PVALUE_DATA pvd;
	pvd.pxd = 0;
	return create_pvalue(PNULL, pvd);
}
/*==================================
 * PINT: pvalue containing an int
 *================================*/
PVALUE
create_pvalue_from_int (INT ival)
{
	PVALUE_DATA pvd;
	pvd.ixd = ival;
	return create_pvalue(PINT, pvd);
}
void
set_pvalue_int (PVALUE val, INT inum)
{
	PVALUE_DATA pvd;
	pvd.ixd = inum;
	set_pvalue(val, PINT, pvd);
}
INT
pvalue_to_int (PVALUE val)
{
	return val->value.ixd;
}
/*==================================
 * PFLOAT: pvalue containing a float
 * ptag's value is not large enough, so we have to store
 * heap pointer.
 *================================*/
PVALUE
create_pvalue_from_float (float fval)
{
	PVALUE_DATA pvd;
	pvd.fxd = fval;
	return create_pvalue(PFLOAT, pvd);
}
void
set_pvalue_float (PVALUE val, float fnum)
{
	PVALUE_DATA pvd;
	pvd.fxd = fnum;
	set_pvalue(val, PFLOAT, pvd);
}
float
pvalue_to_float (PVALUE val)
{
	return pvalvv(val).fxd;
}
/*==================================
 * PBOOL: pvalue containing a boolean
 *================================*/
PVALUE
create_pvalue_from_bool (BOOLEAN bval)
{
	PVALUE_DATA pvd;
	pvd.bxd = bval;
	return create_pvalue(PBOOL, pvd);
}
void
set_pvalue_bool (PVALUE val, BOOLEAN bnum)
{
	PVALUE_DATA pvd;
	pvd.bxd = bnum;
	set_pvalue(val, PBOOL, pvd);
}
BOOLEAN
pvalue_to_bool (PVALUE val)
{
	return pvalvv(val).bxd;
}
/*==================================
 * PSTRING: pvalue containing a string
 *================================*/
PVALUE
create_pvalue_from_string (CNSTRING str)
{
	PVALUE_DATA pvd;
	pvd.sxd = (STRING)str;
	return create_pvalue(PSTRING, pvd);
}
PVALUE
create_pvalue_from_zstr (ZSTR * pzstr)
{
	PVALUE val = create_pvalue_from_string(zs_str(*pzstr));
	zs_free(pzstr);
	return val;
}
void
set_pvalue_string (PVALUE val, CNSTRING str)
{
	PVALUE_DATA pvd;
	pvd.sxd = (STRING)str;
	set_pvalue(val, PSTRING, pvd);
}
STRING
pvalue_to_string (PVALUE val)
{
	return pvalvv(val).sxd;
}
/*==================================
 * PGNODE: pvalue containing a GEDCOM node
 *================================*/
PVALUE
create_pvalue_from_node (NODE node)
{
	PVALUE_DATA pvd;
	pvd.nxd = node;
	return create_pvalue(PGNODE, pvd);
}
void
set_pvalue_node (PVALUE val, NODE node)
{
	PVALUE_DATA pvd;
	pvd.nxd = node;
	set_pvalue(val, PGNODE, pvd);
}
NODE
pvalue_to_node (PVALUE val)
{
	return pvalvv(val).nxd;
}
/*==================================
 * record pvalues (PINDI, PFAM, ...)
 *================================*/
RECORD
pvalue_to_record (PVALUE val)
{
	ASSERT(is_record_pvalue(val));
	return pvalvv(val).rxd; /* may be NULL */
}
CACHEEL
pvalue_to_cel (PVALUE val)
{
	RECORD rec = pvalue_to_record(val);
	HINT_VAR_UNUSED NODE root = nztop(rec); /* force record into cache */
	CACHEEL cel = nzcel(rec);
	return cel;
}
/*==================================
 * LIST: pvalue containing a list
 *================================*/
PVALUE
create_pvalue_from_list (LIST list)
{
	PVALUE_DATA pvd;
	pvd.lxd = list;
	return create_pvalue(PLIST, pvd);
}
LIST
pvalue_to_list (PVALUE val)
{
	return pvalvv(val).lxd;
}
/*==================================
 * TABLE: pvalue containing a table
 *================================*/
PVALUE
create_pvalue_from_table (TABLE tab)
{
	PVALUE_DATA pvd;
	pvd.txd = tab;
	return create_pvalue(PTABLE, pvd);
}
TABLE
pvalue_to_table (PVALUE val)
{
	return pvalvv(val).txd;
}
/*==================================
 * PSET: pvalue containing a set (INDISEQ)
 *================================*/
PVALUE
create_pvalue_from_seq (INDISEQ seq)
{
	PVALUE_DATA pvd;
	pvd.qxd = seq;
	return create_pvalue(PSET, pvd);
}
void
set_pvalue_seq (PVALUE val, INDISEQ seq)
{
	PVALUE_DATA pvd;
	pvd.qxd = seq;
	set_pvalue(val, PSET, pvd);
}
INDISEQ
pvalue_to_seq (PVALUE val)
{
	return pvalvv(val).qxd;
}
/*==================================
 * ARRAY: pvalue containing an array
 *================================*/
ARRAY
pvalue_to_array (PVALUE val)
{
	return pvalvv(val).axd;
}
/*========================================
 * init_pvalue_vtable -- set vtable (for allocator in pvalalloc.c)
 *======================================*/
void
init_pvalue_vtable (PVALUE val)
{
	val->vtable = &vtable_for_pvalue;
}
/*=================================================
 * pvalue_destructor -- destructor for vtable
 *===============================================*/
static void
pvalue_destructor (VTABLE *obj)
{
	PVALUE val = (PVALUE)obj;
	ASSERT((*obj)->vtable_class == vtable_for_pvalue.vtable_class);
	delete_pvalue(val);
}
/*=================================================
 * value_copy -- copy for vtable
 *===============================================*/
static OBJECT
pvalue_copy (OBJECT obj, int deep)
{
	PVALUE val = (PVALUE)obj;
	ASSERT((*obj)->vtable_class == vtable_for_pvalue.vtable_class);
	if (deep) {
		/* cannot implement deep copy until all objects implement copy */
		ASSERT(0);
	} else {
		PVALUE newval = copy_pvalue(val);
		return (OBJECT)newval;
	}
	return 0;
}
/*=============================================
 * pvalues_collate -- Compare two pvalues for collation
 *============================================*/
INT
pvalues_collate (PVALUE val1, PVALUE val2)
{
	/* if dissimilar types, we'll use the numerical order of the types */
	if (ptype(val1) != ptype(val2))
		return ptype(val1) - ptype(val2);

	/* ok, they are the same types, how do we compare them ? */
	switch(ptype(val1)) {
	case PSTRING:
		return cmpstrloc(pvalue_to_string(val1), pvalue_to_string(val2));
	case PINT:
		return pvalue_to_int(val1) - pvalue_to_int(val2);
	case PFLOAT:
		return pvalue_to_float(val1) - pvalue_to_float(val2);
	}
	return 0; /* TODO: what about other types ? */
}
/*=============================================
 * create_new_pvalue_table -- Create new table inside new pvalue
 *============================================*/
PVALUE
create_new_pvalue_table (void)
{
	TABLE tab = create_table_custom_vptr(delete_vptr_pvalue);
	PVALUE val = create_pvalue_from_table(tab);
	release_table(tab);
	return val;
}
/*=============================================
 * create_new_pvalue_list -- Create new list inside new pvalue
 *============================================*/
PVALUE
create_new_pvalue_list (void)
{
	LIST list = create_list3(delete_vptr_pvalue);
	PVALUE val = create_pvalue_from_list(list);
	release_list(list); /* release our ref to list */
	return val;
}
/*=============================================
 * set_pvalue_to_pvalue -- Set val to be same value as src
 *============================================*/
void
set_pvalue_to_pvalue (PVALUE val, const PVALUE src)
{
	set_pvalue(val, ptype(src), pvalvv(src));
}
