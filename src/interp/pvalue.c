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
static void clear_pvalue(PVALUE val);

static PVALUE create_pvalue_from_keynum_impl(INT i, INT ptype);
static PVALUE create_pvalue_from_key_impl(CNSTRING key, INT ptype);
static PVALUE create_pvalue_from_record(RECORD rec, INT ptype);
/* static BOOLEAN eq_pstrings(PVALUE val1, PVALUE val2); */
static int float_to_int(float f);
static void free_float_pvalue(PVALUE val);
static BOOLEAN is_record_pvaltype(INT valtype);
static OBJECT pvalue_copy(OBJECT obj, int deep);
static void pvalue_destructor(VTABLE *obj);

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
create_pvalue (INT type, VPTR value)
{
	PVALUE val = create_new_pvalue();
	set_pvalue(val, type, value);
	return val;
}
/*==================================
 * set_pvalue -- Set a program value
 *  val:   [I/O] pvalue getting new info
 *  type:  [IN]  new type for pvalue
 *  value: [IN]  new value for pvalue
 *================================*/
void
set_pvalue (PVALUE val, INT type, VPTR value)
{
	if (type == ptype(val) && value == pvalvv(val)) {
		/* self-assignment */
		/* already have the value, or hold the pointer & reference */
		return;
	}

	/* record pvalues each have their own RECORD on the heap */

	clear_pvalue(val);

	/* sanity check */
	switch(type) {
	case PNULL:
		ASSERT(!value);
		break;
	}
	/* types that don't simply assign pointer */
	/* old refers to value passed in */
	/* new refers to newly allocated copy to set */
	switch(type) {
	case PSTRING:
		{
			/* always copies string so caller doesn't have to */
			if (value) {
				STRING strold = (STRING)value;
				STRING strnew = strsave(strold);
				value = strnew;
			}
		break;
		}
	case PFLOAT:
		{
			float valold, *ptrnew;
			ASSERT(value);
			/* floats don't fit into VPTR, so we're using heap copies */
			valold = *(float *)value;
			/* allocate new pointer & copy float into place */
			ptrnew = (float *)stdalloc(sizeof(*ptrnew));
			*ptrnew = valold;
			value = ptrnew;
		}
		break;
	}

	ptype(val) = type;
	pvalvv(val) = value;

	if (is_record_pvaltype(type)) {
		RECORD rec = pvalue_to_record(val);
		if (rec) {
			addref_record(rec);
		}
	}

	/* reference counted types and so forth */
	switch(type) {
	case PGNODE:
		{
			NODE node = pvalue_to_node(val);
			if (node) {
				++nrefcnt(node);
				dolock_node_in_cache(node, TRUE);
			}
		}
		break;
	case PLIST:
		{
			LIST list = pvalue_to_list(val);
			addref_list(list);
		}
		break;
	case PTABLE:
		{
			TABLE table = pvalue_to_table(val);
			addref_table(table);
		}
		break;
	case PSET:
		{
			INDISEQ seq = pvalue_to_seq(val);
			/* because of getindiset, seq might be NULL */
			if (seq) {
				++IRefcnt(seq);
			}
		}
		break;
	}
}
/*========================================
 * dolock_node_in_cache -- Lock/unlock node in cache
 *  (if possible)
 * Created: 2003-02-04 (Perry Rapp)
 *======================================*/
void
dolock_node_in_cache (NODE node, BOOLEAN lock)
{
	node = node;	/* NOTUSED */
	lock = lock;	/* NOTUSED */

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
 * clear_pvalue -- Empty contents of pvalue
 *  This doesn't bother to clear val->value
 *  because caller will do so
 * Created: 2001/01/20, Perry Rapp
 *======================================*/
static void
clear_pvalue (PVALUE val)
{
	check_pvalue_validity(val);
	switch (ptype(val)) {
	/*
	embedded values have no referenced memory to clear
	PINT, PBOOLEAN 
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
	case PFLOAT:
		free_float_pvalue(val);
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
table_pvcleaner (CNSTRING key, UNION uval)
{
	PVALUE val = uval.w;
	key=key; /* unused */
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
		node = pvalue_to_node(vl);
		pvalvv(vl) = 0; /* remove pointer to payload */
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
		return create_pvalue(PINDI, 0);
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
	val = create_pvalue(type, rec);
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
	CACHEEL cel = fam ? fam_to_cacheel_old(fam) : NULL;
	return create_pvalue_from_cel(PFAM, cel);
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
	/* record pvalues simply point to their heap-alloc'd record */
	return create_pvalue(ptype, rec);
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
	sprintf(key, "%c%d", cptype, i);
	return create_pvalue_from_key_impl(key, ptype);
}
/*==================================
 * free_float_pvalue -- Delete float pvalue
 * Inverse of make_float_pvalue
 * Created: 2002/01/09, Perry Rapp
 *================================*/
static void
free_float_pvalue (PVALUE val)
{
	float *ptr = (float *)pvalvv(val);
	stdfree(ptr);
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
	return type == PINT || type == PFLOAT || type == PNULL;
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
		/* Anything is convertible to PBOOL */
		BOOLEAN boo = (pvalvv(val) != NULL);
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
		/* for everything else, just compare value pointer */
		default:
			rel = (pvalvv(val1) == pvalvv(val2));
			break;
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
	llwprintf(zs_str(zstr));
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
 *  returns static buffer
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
	if (pvalvv(val) == NULL) {
		zs_apps(zstr, "NULL>");
		return zstr;
	}
	switch (type) {
	case PINT:
		zs_appf(zstr, "%d", pvalue_to_int(val));
		break;
	case PFLOAT:
		zs_appf(zstr, "%f", pvalue_to_float(val));
		break;
	case PSTRING:
		zs_appf(zstr, "\"%s\"", pvalue_to_string(val));
		break;
	case PINDI:
	case PFAM:
	case PSOUR:
	case PEVEN:
	case POTHR:
		{
			RECORD rec = pvalue_to_record(val);
			if (rec)
				zs_appf(zstr, nzkey(rec));
			else
				zs_appf(zstr, "NULL");
		}
		break;
	case PLIST:
		{
			LIST list = pvalue_to_list(val);
			INT n = length_list(list);
			zs_appf(zstr, _pl("%d item", "%d items", n), n);
		}
		break;
	case PTABLE:
		{
			TABLE table = pvalue_to_table(val);
			INT n = get_table_count(table);
			zs_appf(zstr, _pl("%d entry", "%d entries", n), n);
		}
		break;
	case PSET:
		{
			INDISEQ seq = pvalue_to_seq(val);
			INT n = length_indiseq(seq);
			zs_appf(zstr, _pl("%d record", "%d records", n), n);
		}
		break;
	case PARRAY:
		{
			ARRAY arr = pvalue_to_array(val);
			INT n = get_array_size(arr);
			zs_appf(zstr, _pl("%d element", "%d elements", n), n);
		}
		break;
	default:
		zs_appf(zstr, "%p", pvalvv(val));
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
	return create_pvalue(PNULL, NULL);
}
/*==================================
 * PINT: pvalue containing an int
 *================================*/
PVALUE
create_pvalue_from_int (INT ival)
{
	return create_pvalue(PINT, (VPTR) ival);
}
void
set_pvalue_int (PVALUE val, INT inum)
{
	set_pvalue(val, PINT, (VPTR)inum);
}
INT
pvalue_to_int (PVALUE val)
{
	return (INT)pvalvv(val);
}
/*==================================
 * PFLOAT: pvalue containing a float
 * ptag's value is not large enough, so we have to store
 * heap pointer.
 *================================*/
PVALUE
create_pvalue_from_float (float fval)
{
	return create_pvalue(PFLOAT, &fval);
}
void
set_pvalue_float (PVALUE val, float fnum)
{
	set_pvalue(val, PFLOAT, &fnum);
}
float
pvalue_to_float (PVALUE val)
{
	/* TODO: change when ptag goes to UNION */
	return *(float*)pvalvv(val);
}
/*==================================
 * PBOOL: pvalue containing a boolean
 *================================*/
PVALUE
create_pvalue_from_bool (BOOLEAN bval)
{
	return create_pvalue_from_int(bval);
}
void
set_pvalue_bool (PVALUE val, BOOLEAN bnum)
{
	set_pvalue(val, PBOOL, (VPTR)bnum);
}
BOOLEAN
pvalue_to_bool (PVALUE val)
{
	return (BOOLEAN)pvalvv(val);
}
/*==================================
 * PSTRING: pvalue containing a string
 *================================*/
PVALUE
create_pvalue_from_string (CNSTRING str)
{
	return create_pvalue(PSTRING, (VPTR)str);
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
	set_pvalue(val, PSTRING, (VPTR)str); /* makes new copy of string */
}
STRING
pvalue_to_string (PVALUE val)
{
	return (STRING)pvalvv(val);
}
/*==================================
 * PGNODE: pvalue containing a GEDCOM node
 *================================*/
PVALUE
create_pvalue_from_node (NODE node)
{
	return create_pvalue(PGNODE, node);
}
void
set_pvalue_node (PVALUE val, NODE node)
{
	set_pvalue(val, PGNODE, (VPTR)node);
}
NODE
pvalue_to_node (PVALUE val)
{
	return (NODE)pvalvv(val);
}
/*==================================
 * record pvalues (PINDI, PFAM, ...)
 *================================*/
RECORD
pvalue_to_record (PVALUE val)
{
	RECORD rec = pvalvv(val); /* may be NULL */
	ASSERT(is_record_pvalue(val));
	return rec;
}
CACHEEL
pvalue_to_cel (PVALUE val)
{
	RECORD rec = pvalue_to_record(val);
	NODE root = nztop(rec); /* force record into cache */
	CACHEEL cel = nzcel(rec);
	root = root;	/* NOTUSED */
	return cel;
}
/*==================================
 * LIST: pvalue containing a list
 *================================*/
PVALUE
create_pvalue_from_list (LIST list)
{
	return create_pvalue(PLIST, list);
}
LIST
pvalue_to_list (PVALUE val)
{
	return (LIST)pvalvv(val);
}
/*==================================
 * TABLE: pvalue containing a table
 *================================*/
PVALUE
create_pvalue_from_table (TABLE tab)
{
	return create_pvalue(PTABLE, tab);
}
TABLE
pvalue_to_table (PVALUE val)
{
	return (TABLE)pvalvv(val);
}
/*==================================
 * PSET: pvalue containing a set (INDISEQ)
 *================================*/
PVALUE
create_pvalue_from_seq (INDISEQ seq)
{
	return create_pvalue(PSET, seq);
}
void
set_pvalue_seq (PVALUE val, INDISEQ seq)
{
	set_pvalue(val, PSET, (VPTR)seq);
}
INDISEQ
pvalue_to_seq (PVALUE val)
{
	return (INDISEQ)pvalvv(val);
}
/*==================================
 * ARRAY: pvalue containing an array
 *================================*/
ARRAY
pvalue_to_array (PVALUE val)
{
	return (ARRAY)pvalvv(val);
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
