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
#include "interp.h"
#include "interpi.h"
#include "liflines.h"
#include "feedback.h"
#include "zstr.h"
#include "vtable.h"
#include "array.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static FLOAT bool_to_float(BOOLEAN);
static INT bool_to_int(BOOLEAN);
static void clear_pv_indiseq(INDISEQ seq);
static void clear_pvalue(PVALUE val);
static PVALUE create_pvalue_from_keynum_impl(INT i, INT ptype);
static PVALUE create_pvalue_from_key_impl(STRING key, INT ptype);
static PVALUE create_pvalue_from_record(RECORD rec, INT ptype);
static BOOLEAN eq_pstrings(PVALUE val1, PVALUE val2);
static int float_to_int(float f);
static void free_float_pvalue(PVALUE val);
static OBJECT pvalue_copy(OBJECT obj, int deep);
static void pvalue_destructor(VTABLE *obj);
static void table_pvcleaner(ENTRY ent);

/*********************************************
 * local variables
 *********************************************/

static char *ptypes[] = {
	"PNONE", "PANY", "PINT", "PLONG", "PFLOAT", "PBOOL", "PSTRING",
	"PGNODE", "PINDI", "PFAM", "PSOUR", "PEVEN", "POTHR", "PLIST",
	"PTABLE", "PSET"
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
	if (type == ptype(val) && value == pvalue(val)) {
		/* self-assignment */
		return;
	}
	clear_pvalue(val);

	/* sanity check */
	switch(type) {
	case PNONE: 
	case PANY:
		ASSERT(!value);
		break;
	}
	/* types that don't simply assign pointer */
	switch(type) {
	case PSTRING:
		{
		/* always copies string so caller doesn't have to */
		if (value)
			value = (VPTR) strsave((STRING) value);
		break;
		}
	case PFLOAT:
		{
			/* floats don't fit into VPTR, so we're using heap copies */
			float *ptr = (float *)stdalloc(sizeof(*ptr));
			*ptr = *(float *)value;
			value = ptr;
		}
		break;
	}

	ptype(val) = type;
	pvalue(val) = value;

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
			++lrefcnt(list);
		}
		break;
	case PTABLE:
		{
			TABLE table = pvalue_to_table(val);
			++table->refcnt;
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
	if (is_record_pvalue(val)) {
		/* instead of reference counting records, we lock them */
		dosemilock_record_in_cache(pvalue_to_rec(val), TRUE);
	}
}
/*========================================
 * dosemilock_record_in_cache -- semi-Lock/unlock record
 *  (if possible)
 * Created: 2003-02-06 (Perry Rapp)
 *======================================*/
void
dosemilock_record_in_cache (RECORD rec, BOOLEAN lock)
{
	if (rec) {
		CACHEEL cel = rec->cel;
		if (cel) {
				if (lock)
					semilock_cache(cel);
				else
					unsemilock_cache(cel);
		}
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
	PINT, PBOOLEAN  (PLONG is unused)
	*/
	/*
	PANY is a null value
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
	/* nodes from cache handled below switch - PINDI, PFAM, PSOUR, PEVEN, POTHR */
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
			--lrefcnt(list);
			if (!lrefcnt(list)) {
				remove_list(list, delete_vptr_pvalue);
			}
		}
		return;
	case PTABLE:
		{
			TABLE table = pvalue_to_table(val);
			--table->refcnt;
			if (!table->refcnt) {
				/* TODO: this will go away when pvalues go away */
				traverse_table(table, table_pvcleaner);
				destroy_table(table);
			}
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
	}
	if (is_record_pvalue(val)) {
		/*
		unlock any cache elements
		don't worry about memory - it is owned by cache
		*/
		dosemilock_record_in_cache(pvalue_to_rec(val), FALSE);
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
		val = (PVALUE) sval(el).w;
		if (val) {
			delete_pvalue(val);
			sval(el).w = NULL;
		}
	ENDINDISEQ
}
/*========================================
 * table_pvcleaner -- Clean pvalue entries
 *  from table
 * Created: 2001/03/24, Perry Rapp
 *======================================*/
static void
table_pvcleaner (ENTRY ent)
{
	PVALUE val = ent->uval.w;
	delete_pvalue(val);
	ent->uval.w = NULL;
}
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
 * delete_pvalue_wrapper -- Delete the pvalue
 * shell, but not the value inside
 * Created: 2003-02-02 (Perry Rapp)
 *======================================*/
void
delete_pvalue_wrapper (PVALUE val)
{
	if (!val) return;
	pvalue(val) = 0; /* remove pointer to payload */
	delete_pvalue(val);
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
 *==================================*/
PVALUE
copy_pvalue (PVALUE val)
{
	if (!val)
		return NULL;
	return create_pvalue(ptype(val), pvalue(val));
}
/*=====================================================
 * create_pvalue_from_indi -- Return indi as pvalue
 *  handles NULL
 * Created: 2001/03/18, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_indi (NODE indi)
{
	CACHEEL cel = indi ? indi_to_cacheel_old(indi) : NULL;
	return create_pvalue(PINDI, cel);
}
/*=====================================================
 * create_pvalue_from_indi_key
 *  handles NULL
 * Created: 2000/12/30, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_indi_key (STRING key)
{
	return create_pvalue_from_key_impl(key, PINDI);
}
/*=====================================================
 * create_pvalue_from_cel
 * Created: 2002/02/17, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_cel (CACHEEL cel)
{
	return create_pvalue(PINDI, cel);
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
	return create_pvalue(PFAM, cel);
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
 * Created: 2001/03/20, Perry Rapp
 *===================================================*/
static PVALUE
create_pvalue_from_record (RECORD rec, INT ptype)
{
	CACHEEL cel = rec ? record_to_cacheel(rec) : NULL;
	return create_pvalue(ptype, cel);
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
	float *ptr = (float *)pvalue(val);
	stdfree(ptr);
}
/*==================================
 * create_pvalue_from_key_impl -- Create pvalue from any key
 * Created: 2001/03/20, Perry Rapp
 *================================*/
static PVALUE
create_pvalue_from_key_impl (STRING key, INT ptype)
{
	/* report mode, so may return NULL */
	RECORD rec = qkey_to_record(key);
	PVALUE val = create_pvalue_from_record(rec, ptype);
	return val;
}
/*==================================================
 * is_numeric_pvalue -- See if program value is numeric
 *================================================*/
BOOLEAN
is_numeric_pvalue (PVALUE val)
{
	INT type = ptype(val);
	return type == PINT || type == PFLOAT;
}
/*===========================================================
 * eq_conform_pvalues -- Make the types of two values conform
 *=========================================================*/
void
eq_conform_pvalues (PVALUE val1, PVALUE val2, BOOLEAN *eflg)
{
	INT hitype;

	ASSERT(val1 && val2);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PANY && pvalue(val1) == NULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PANY && pvalue(val2) == NULL)
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PINT && pvalue(val1) == 0 && !is_numeric_pvalue(val2))
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PINT && pvalue(val2) == 0 && !is_numeric_pvalue(val1))
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
		BOOLEAN boo = (pvalue(val) != NULL);
		set_pvalue_bool(val, boo);
		return;
	}
	/* Anything is convertible to PANY */
	/* Perry, 2002.02.16: This looks suspicious to me, but I 
	don't know how it is used -- it might be used in some
	eq_conform_pvalues call(s) ? */
	if (type == PANY) {
		ptype(val) = PANY;
		return;
	}

	/* PANY or PINT with NULL (0) value is convertible to any scalar (1995.07.31) */
	if ((ptype(val) == PANY || ptype(val) == PINT) && pvalue(val) == NULL) {
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
	/* record types (PINDI...), PANY, PGNODE */
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
 * is_record_pvalue -- Does pvalue contain record ?
 *======================================*/
BOOLEAN
is_record_pvalue (PVALUE value)
{
	switch (ptype(value)) {
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
		case PSTRING:
			v1 = pvalue(val1);
			v2 = pvalue(val2);
			if(v1 && v2) rel = eqstr(v1, v2);
			else rel = (v1 == v2);
			break;
		case PFLOAT:
			rel = (pvalue_to_float(val1) == pvalue_to_float(val2));
			break;
		/* for everything else, just compare value pointer */
		default:
			rel = (pvalue(val1) == pvalue(val2));
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
static BOOLEAN
eq_pstrings (PVALUE val1, PVALUE val2)
{
	STRING str1 = pvalue(val1), str2 = pvalue(val2);
	if (!str1) str1 = "";
	if (!str2) str2 = "";
	return eqstr(str1, str2);
}
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
	switch (ptype(val1)) {
	case PSTRING:
		rel = eq_pstrings(val1, val2);
		break;
	case PFLOAT:
		rel = (pvalue_to_float(val1) == pvalue_to_float(val2));
		break;
		/* for everything else, just compare value pointer */
	default:
		rel = (pvalue(val1) == pvalue(val2));
		break;
	}
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
	switch (ptype(val1)) {
	case PSTRING:
		rel = !eq_pstrings(val1, val2);
		break;
	case PFLOAT:
		rel = (pvalue_to_float(val1) != pvalue_to_float(val2));
		break;
	default:
		rel = (pvalue(val1) != pvalue(val2));
		break;
	}
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
	zs_apps(zstr, ptypes[type]);
	zs_appc(zstr, ',');
	if (pvalue(val) == NULL) {
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
		{
			NODE node;
			CACHEEL cel = (CACHEEL) pvalue(val);
			STRING nam;
			if (!cnode(cel))
				cel = key_to_indi_cacheel(ckey(cel));
       		node = cnode(cel);
			node = NAME(node);
			nam = node ? nval(node) : _("{NoName}");
			zs_appf(zstr, nam);
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
	default:
		zs_appf(zstr, "%p", pvalue(val));
		break;
	}
	zs_appc(zstr, '>');
	return zstr;
}
/*==================================
 * PANY: pvalue with no content
 *================================*/
PVALUE
create_pvalue_any (void)
{
	return create_pvalue(PANY, NULL);
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
	return (BOOLEAN)pvalue(val);
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
	return *(float*)pvalue(val);
}
float*
pvalue_to_pfloat (PVALUE val)
{
	/* convenience for math */
	/* TODO: change when ptag goes to UNION */
	return (float*)pvalue(val);
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
	return (NODE)pvalue(val);
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
	return (INT)pvalue(val);
}
INT*
pvalue_to_pint (PVALUE val)
{
	/* convenience for math */
	return (INT *)&pvalue(val);
}
/*==================================
 * ARRAY: pvalue containing an array
 *================================*/
ARRAY
pvalue_to_array (PVALUE val)
{
	return (ARRAY)pvalue(val);
}
/*==================================
 * LIST: pvalue containing a list
 *================================*/
LIST
pvalue_to_list (PVALUE val)
{
	return (LIST)pvalue(val);
}
/*==================================
 * record pvalues (PINDI, PFAM, ...)
 *================================*/
CACHEEL
pvalue_to_cel (PVALUE val)
{
	/* also load record into direct cache */
	CACHEEL cel = pvalue(val); /* may be NULL */
	ASSERT(is_record_pvalue(val));
	load_cacheel(cel); /* handles null cel ok */
	return cel;
}
RECORD
pvalue_to_rec (PVALUE val)
{
	CACHEEL cel = pvalue_to_cel(val);
	if (!cel) return NULL;
	return crecord(cel);
}
/*==================================
 * PSET: pvalue containing a set (INDISEQ)
 *================================*/
PVALUE
create_pvalue_from_set (INDISEQ seq)
{
	return create_pvalue(PSET, seq);
}
INDISEQ
pvalue_to_seq (PVALUE val)
{
	return (INDISEQ)pvalue(val);
}
/*==================================
 * PSTRING: pvalue containing a string
 *================================*/
PVALUE
create_pvalue_from_string (STRING str)
{
	return create_pvalue(PSTRING, str);
}
void
set_pvalue_string (PVALUE val, CNSTRING str)
{
	set_pvalue(val, PSTRING, (VPTR)str); /* makes new copy of string */
}
STRING
pvalue_to_string (PVALUE val)
{
	return (STRING)pvalue(val);
}
TABLE
pvalue_to_table (PVALUE val)
{
	return (TABLE)pvalue(val);
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

