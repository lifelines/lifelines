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
 *   3.0.3 - 03 Jul 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "liflines.h"
#include "screen.h"

/*********************************************
 * local types
 *********************************************/

/* block of pvalues - see comments in alloc_pvalue_memory() */
struct pv_block
{
	struct pv_block * next;
	struct ptag values[100]; /* arbitrary size may be adjusted */
};
typedef struct pv_block *PV_BLOCK;
#define BLOCK_VALUES (sizeof(((PV_BLOCK)0)->values)/sizeof(((PV_BLOCK)0)->values[0]))

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static CACHEEL access_cel_from_pvalue(PVALUE val);
static void clear_pv_indiseq(INDISEQ seq);
static PVALUE create_pvalue_from_keynum_impl(INT i, INT ptype);
static PVALUE create_pvalue_from_key_impl(STRING key, INT ptype);
static BOOLEAN eq_pstrings(PVALUE val1, PVALUE val2);
static void free_all_pvalues(void);
static BOOLEAN is_pvalue_or_freed(PVALUE pval);
static void symtab_cleaner(ENTRY ent);
static void table_pvcleaner(ENTRY ent);

/*********************************************
 * local variables
 *********************************************/

static char *ptypes[] = {
	"PNONE", "PANY", "PINT", "PLONG", "PFLOAT", "PBOOL", "PSTRING",
	"PGNODE", "PINDI", "PFAM", "PSOUR", "PEVEN", "POTHR", "PLIST",
	"PTABLE", "PSET"};

static PVALUE free_list = 0;
static INT live_pvalues = 0;
static PV_BLOCK block_list = 0;
static BOOLEAN reports_time = FALSE;
static BOOLEAN cleaning_time = FALSE;
static BOOLEAN alloclog_save = FALSE;
#ifdef DEBUG_PVALUES
static INT debugging_pvalues = TRUE;
#else
static INT debugging_pvalues = FALSE;
#endif

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*========================================
 * debug_check -- check every pvalue in free list
 * This is of course slow, but invaluable for tracking
 * down pvalue corruption bugs - so don't define
 * DEBUG_PVALUES unless you are working on a pvalue
 * corruption bug.
 * Created: 2001/03, Perry Rapp
 *======================================*/
static void
debug_check (void)
{
	PVALUE val;
	for (val=free_list; val; val=val->value)
	{
		ASSERT(val->type == 99 && val->value != val);
	}
}
/*========================================
 * alloc_pvalue_memory -- return new pvalue memory
 * We use a custom allocator, which lowers our overhead
 *  (no heap overhead per pvalue, only per block)
 *  and also allows us to clean them all up after the
 *  report.
 * NB: This is not traditional garbage collection - we're
 *  not doing any live/dead analysis; we depend entirely
 *  on carnal knowledge of the program.
 * We also mark freed ones, so we can scan the blocks
 *  to find leaked ones (which used to be nearly
 *  all of them)
 * Created: 2001/01/19, Perry Rapp
 *======================================*/
static PVALUE
alloc_pvalue_memory (void)
{
	PVALUE val;
	
	ASSERT(reports_time);
	/*
	We assume that all pvalues are scoped
	within report processing. If this ceases to
	be true, this has to be rethought.
	Eg, we could use a bitmask in the type field to mark
	truly heap-alloc'd pvalues, used outside of reports.
	*/
	if (!free_list) {
		/* no pvalues available, make a new block */
		PV_BLOCK new_block = stdalloc(sizeof(*new_block));
		INT i;
		new_block->next = block_list;
		block_list = new_block;
		/* add all pvalues in new block to free list */
		for (i=0; i<BLOCK_VALUES; i++) {
			PVALUE val1 = &new_block->values[i];
			val1->type = PFREED;
			val1->value = free_list;
			free_list = val1;
			if (debugging_pvalues)
				debug_check();
		}
	}
	/* pull pvalue off of free list */
	val = free_list;
	free_list = free_list->value;
	if (debugging_pvalues)
		debug_check();
	live_pvalues++;
	/* set type to uninitialized - caller ought to set type */
	ptype(val) = PUNINT;
	pvalue(val) = 0;
	return val;
}
/*========================================
 * free_pvalue_memory -- return pvalue to free-list
 * (see alloc_pvalue_memory comments)
 * Created: 2001/01/19, Perry Rapp
 *======================================*/
static void
free_pvalue_memory (PVALUE val)
{
	/* see alloc_pvalue_memory for discussion of this ASSERT */
	ASSERT(reports_time);
	if (ptype(val)==PFREED) {
		/*
		this can happen during cleaning - eg, if we find
		orphaned values in a table before we find the orphaned
		table
		*/
		ASSERT(cleaning_time);
		return;
	}
	/* put on free list */
	val->type = PFREED;
	val->value = free_list;
	free_list = val;
	if (debugging_pvalues)
		debug_check();
	live_pvalues--;
	ASSERT(live_pvalues>=0);
}
/*======================================
 * pvalues_begin -- Start of programs
 * Created: 2001/01/20, Perry Rapp
 *====================================*/
void
pvalues_begin (void)
{
	ASSERT(!reports_time);
	reports_time = TRUE;
#ifdef DEBUG_REPORT_MEMORY_DETAIL
	alloclog_save = alloclog;
	alloclog = TRUE;
#endif
}
/*======================================
 * pvalues_end -- End of programs
 * Created: 2001/01/20, Perry Rapp
 *====================================*/
void
pvalues_end (void)
{
	ASSERT(!cleaning_time);
	cleaning_time = TRUE;
	free_all_pvalues();
	free_all_pnodes();
	cleaning_time = FALSE;
	ASSERT(reports_time);
	reports_time = FALSE;
#ifdef DEBUG_REPORT_MEMORY
	{
		INT save = alloclog;
		alloclog = TRUE;
		report_alloc_live_count("pvalues_end");
		alloclog = save;
	}
#ifdef DEBUG_REPORT_MEMORY_DETAIL
	alloclog = alloclog_save;
#endif
#endif
}
/*======================================
 * free_all_pvalues -- Free every pvalue
 * Created: 2001/01/20, Perry Rapp
 *====================================*/
static void
free_all_pvalues (void)
{
	PV_BLOCK block;
	int leaked = live_pvalues;
	while ((block = block_list)) {
		PV_BLOCK next = block->next;
		/*
		As we free the blocks, all their pvalues go back to CRT heap
		so we must not touch them again - so keep zeroing out free_list
		for each block
		*/
		free_list = 0;
		if (live_pvalues) {
			INT i;
			for (i=0; i<BLOCK_VALUES; i++) {
				PVALUE val1=&block->values[i];
				if (val1->type != PFREED) {
					/* leaked */
					delete_pvalue(val1);
				}
			}
		}
		stdfree(block);
		block_list = next;
	}
	free_list = 0;
}
/*========================================
 * create_pvalue -- Create a program value
 *======================================*/
PVALUE
create_pvalue (INT type, VPTR value)
{
	PVALUE val;

	if (type == PNONE) return NULL;
	val = alloc_pvalue_memory();
	if (type == PSTRING) {
		if (value) {
			value = (VPTR) strsave((STRING) value);
		}
	}
	ptype(val) = type;
	pvalue(val) = value;
	/*
	we really ought strongly lock for PGNODE
	but it isn't convenient to find the cacheel
	2001/04/15, Perry Rapp
	*/
	if (is_record_pvalue(val)) {
		/* lock any cache elements, and unlock in clear_pvalue */
		CACHEEL cel = get_cel_from_pvalue(val);
		if (cel)
			semilock_cache(cel);
	}
	return val;
}
/*========================================
 * clear_pvalue -- Empty contents of pvalue
 * Created: 2001/01/20, Perry Rapp
 *======================================*/
void
clear_pvalue (PVALUE val)
{
	if (cleaning_time) {
		ASSERT(is_pvalue_or_freed(val));
	} else {
		ASSERT(is_pvalue(val));
	}
	switch (ptype(val)) {
	/*
	embedded values have no referenced memory to clear
	PINT, PFLOAT, PLONG, PBOOLEAN 
	*/
	/*
	PANY is a null value
	PGNODEs point into cache memory
	*/
	/* nodes from cache handled below switch - PINDI, PFAM, PSOUR, PEVEN, POTHR */
	case PSTRING:
		{
			if (pvalue(val)) {
				stdfree((STRING) pvalue(val));
			}
		}
		return;
	case PLIST:
		{
			LIST list = (LIST) pvalue(val);
			lrefcnt(list)--;
			if (!lrefcnt(list)) {
				remove_list(list, delete_vptr_pvalue);
			}
		}
		return;
	case PTABLE:
		{
			TABLE table = (TABLE)pvalue(val);
			table->refcnt--;
			if (!table->refcnt) {
				traverse_table(table, table_pvcleaner);
				remove_table(table, FREEKEY);
			}
		}
		return;
	case PSET:
		{
			INDISEQ seq = (INDISEQ)pvalue(val);
			/* because of getindiset, seq might be NULL */
			if (seq) {
				IRefcnt(seq)--;
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
		CACHEEL cel = access_cel_from_pvalue(val);
		if (cel)
			unsemilock_cache(cel);
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
 * delete_pvalue -- Delete a program value
 * see create_pvalue - Perry Rapp, 2001/01/19
 *======================================*/
void
delete_pvalue (PVALUE val)
{
#ifdef DEBUG
	llwprintf("__delete_pvalue: val == ");
	show_pvalue(val);
	llwprintf("\n");
#endif	
	if (!val) return;
	clear_pvalue(val);
	free_pvalue_memory(val);
}
/*====================================
 * copy_pvalue -- Create a new pvalue & copy into it
 *  handles NULL
 *==================================*/
PVALUE
copy_pvalue (PVALUE val)
{
	VPTR newval;
	if (!val)
		return NULL;
	switch (ptype(val)) {
	/*
	embedded values have no referenced memory
	and are copied directly
	PINT, PFLOAT, PLONG, PBOOLEAN 
	*/
	case PSTRING:
		{
			/*
			copy_pvalue is called below, and it will
			strsave the value
			*/
		}
		break;
	case PANY:
		{
			/* unassigned values, should be NULL - Perry Rapp, 2001/04/21 */
			ASSERT(pvalue(val) == NULL);
		}
		break;
	case PGNODE:
		{
			/* pointers into cache elements */
		}
		break;
	/* nodes from caches handled below switch (q.v.), inside
	the is_record_pvalue code */
	case PLIST:
		{
			LIST list = (LIST) pvalue(val);
			lrefcnt(list)++;
		}
		break;
	case PTABLE:
		{
			TABLE table = (TABLE)pvalue(val);
			table->refcnt++;
		}
		break;
	case PSET:
		{
			INDISEQ seq = (INDISEQ)pvalue(val);
			/* because of getindiset, seq might be NULL */
			if (seq) {
				IRefcnt(seq)++;
			}
		}
		break;
	}
	if (is_record_pvalue(val)) {
		/*
		2001/03/21
		it is ok to just copy cache-managed elements,
		the reference count must be adjusted
		but create_pvalue() does this part
		(it knows about record pvalues, unlike lists etc)
		(this oddity will go away when I switch to key values)
		*/
	}

	newval = pvalue(val);

	return create_pvalue(ptype(val), newval);
}
/*==================================
 * get_cel_from_pvalue -- Extract record from pvalue
 *  and load into direct
 * Created: 2001/03/17, Perry Rapp
 *================================*/
CACHEEL
get_cel_from_pvalue (PVALUE val)
{
	CACHEEL cel = access_cel_from_pvalue(val);
	load_cacheel(cel);
	return cel;
}
/*==================================
 * access_cel_from_pvalue -- Extract record from pvalue
 *  doesn't load into direct
 * Created: 2001/03/17, Perry Rapp
 *================================*/
static CACHEEL
access_cel_from_pvalue (PVALUE val)
{
	CACHEEL cel = pvalue(val);
	ASSERT(is_record_pvalue(val));
	return cel;
}
/*=====================================================
 * create_pvalue_from_indi -- Return indi as pvalue
 *  handles NULL
 * Created: 2001/03/18, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_indi (NODE indi)
{
	CACHEEL cel = indi ? indi_to_cacheel(indi) : NULL;
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
	CACHEEL cel = fam ? fam_to_cacheel(fam) : NULL;
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
 * create_pvalue_from_node_impl -- Create pvalue from any node
 *  handles NULL
 * Created: 2001/03/20, Perry Rapp
 *===================================================*/
PVALUE
create_pvalue_from_node_impl (NODE node, INT ptype)
{
	CACHEEL cel = node ? node_to_cacheel(node) : NULL;
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
		return create_pvalue_from_node_impl(NULL, ptype);
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
 * create_pvalue_from_key_impl -- Create pvalue from any key
 * Created: 2001/03/20, Perry Rapp
 *================================*/
static PVALUE
create_pvalue_from_key_impl (STRING key, INT ptype)
{
	/* report mode, so may return NULL */
	NODE node = key_to_type(key, TRUE);
	PVALUE val = create_pvalue_from_node_impl(node, ptype);
	return val;
}
/*==================================
 * set_pvalue -- Set a program value
 *================================*/
void
set_pvalue (PVALUE val,
            INT type,
            VPTR value)
{
#ifdef DEBUG
	llwprintf("\nset_pvalue called: val=");
	show_pvalue(val);
	llwprintf(" new type=%d new value = %d\n", type, value);
#endif
	if (type == PSTRING && ptype(val) == PSTRING 
		&& value == pvalue(val)) {
		/* string self-assignment */
		return;
	}
	clear_pvalue(val);
	ptype(val) = type;
	if (type == PSTRING) {
		if (value)
			value = (VPTR) strsave((STRING) value);
	}
	pvalue(val) = value;
}
/*==================================================
 * numeric_pvalue -- See if program value is numeric
 *================================================*/
BOOLEAN
numeric_pvalue (PVALUE val)
{
	INT type = ptype(val);
	return type == PINT || type == PLONG || type == PFLOAT;
}
/*============================================================
 * num_conform_pvalues -- Make the types of two values conform
 *==========================================================*/
void
num_conform_pvalues (PVALUE val1,
                     PVALUE val2,
                     BOOLEAN *eflg)
{
	INT hitype;

	ASSERT(val1 && val2);
	if (ptype(val1) == PANY && pvalue(val1) == NULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PANY && pvalue(val2) == NULL)
		ptype(val2) = ptype(val1);
	if (numeric_pvalue(val1) && numeric_pvalue(val2)) {
		hitype = max(ptype(val1), ptype(val2));
		if (ptype(val1) != hitype) coerce_pvalue(hitype, val1, eflg);
		if (ptype(val2) != hitype) coerce_pvalue(hitype, val2, eflg);
		return;
	}
	*eflg = TRUE;
}
/*===========================================================
 * eq_conform_pvalues -- Make the types of two values conform
 *=========================================================*/
void
eq_conform_pvalues (PVALUE val1,
                    PVALUE val2,
                    BOOLEAN *eflg)
{
	INT hitype;

	ASSERT(val1 && val2);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PANY && pvalue(val1) == NULL)
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PANY && pvalue(val2) == NULL)
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (ptype(val1) == PINT && pvalue(val1) == 0 && !numeric_pvalue(val2))
		ptype(val1) = ptype(val2);
	if (ptype(val2) == PINT && pvalue(val2) == 0 && !numeric_pvalue(val1))
		ptype(val2) = ptype(val1);
	if (ptype(val1) == ptype(val2)) return;
	if (numeric_pvalue(val1) && numeric_pvalue(val2)) {
		hitype = max(ptype(val1), ptype(val2));
		if (ptype(val1) != hitype) coerce_pvalue(hitype, val1, eflg);
		if (ptype(val2) != hitype) coerce_pvalue(hitype, val2, eflg);
		return;
	}
	*eflg = TRUE;
}
/*=========================================================
 * coerce_pvalue -- Convert PVALUE from one type to another
 *=======================================================*/
void
coerce_pvalue (INT type,       /* type to convert to */
               PVALUE val,     /* old and new value */
               BOOLEAN *eflg)
{
#ifdef UNUSED_CODE
	PVALUE new;
	INT vint;
	/*LONG vlong;*/
	FLOAT vfloat;
	BOOLEAN vbool;
#endif
	UNION u;

#ifdef DEBUG
	llwprintf("coerce_pvalue: coerce ");
	show_pvalue(val);
	llwprintf(" to %s\n", ptypes[type]);
#endif
	if (*eflg) return;
	ASSERT(is_pvalue(val));
	if (type == ptype(val)) return;
	u.w = pvalue(val);
	if (type == PBOOL) {	 /* Handle PBOOL as special case */
		set_pvalue(val, PBOOL, (VPTR)(u.w != NULL));
		return;
	}
	if (type == PANY) {	/* Handle PANY as a special case */
		ptype(val) = PANY;
		return;
	}
/* NEW - 7/31/95 */
	/* PANY or PINT with NULL (0) value is convertible to any scalar */
	if ((ptype(val) == PANY || ptype(val) == PINT) && pvalue(val) == NULL) {
		if (type == PSET || type == PTABLE || type == PLIST) goto bad;
		ptype(val) = type;
		return;
	}
/* END */

	switch (ptype(val)) {

	case PINT:
		switch (type) {
		case PINT: return;
		case PLONG: /*u.l = int_to_long(u.w);*/ break;
		case PFLOAT: u.f = u.i; break;
		default: goto bad;
		}
		break;
#if 0
	case PLONG:
		switch (type) {
		case PINT: vint = long_to_int(u.w); break;
		case PLONG: return;
		case PFLOAT: vfloat = long_to_float(u.w); break;
		default: goto bad;
		}
		break;
#endif
	case PFLOAT:
		switch (type) {
		case PINT: u.i = (INT)u.f; break;
		case PLONG: /*u.l = float_to_long(u.w);*/ break;
		case PFLOAT: return;
		default: goto bad;
		}
		break;
	case PBOOL:
		switch (type) {
		case PINT: u.i = bool_to_int((BOOLEAN)u.w); break;
		case PLONG: /*u.l = bool_to_long((BOOLEAN)u.w);*/ break;
		case PFLOAT: u.f = bool_to_float((BOOLEAN)u.w); break;
		default: goto bad;
		}
		break;
	case PINDI:
		goto bad;
	case PANY:
		goto bad;
	case PGNODE:
		goto bad;
	default:
		goto bad;
	}
	ptype(val) = type;
	pvalue(val) = u.w;
	return;
bad:
	*eflg = TRUE;
	return;
}
/*===========================================================
 * is_pvalue -- Checks that PVALUE is a valid type
 *=========================================================*/
BOOLEAN
is_pvalue (PVALUE pval)
{
	if (!pval) return FALSE;
	return ptype(pval) >= PNONE  && ptype(pval) <= PSET;
}
/*===========================================================
 * is_pvalue_or_freed -- Checks that PVALUE is a valid type
 *  or freed
 *=========================================================*/
static BOOLEAN
is_pvalue_or_freed (PVALUE pval)
{
	if (!pval) return FALSE;
	if (ptype(pval) == PFREED) return TRUE;
	return ptype(pval) >= PNONE  && ptype(pval) <= PSET;
}
/*========================================
 * is_record_pvalue -- Does pvalue contain record ?
 *======================================*/
BOOLEAN
is_record_pvalue (PVALUE value)
{
	switch (ptype(value)) {
	case PGNODE: /* TO DO 2001/03/17 ?? */
		break;
	case PINDI: case PFAM: case PSOUR: case PEVEN: case POTHR:
		return TRUE;
	}
	return FALSE;
}
INT
bool_to_int (BOOLEAN b)
{
	return b ? 1 : 0;
}
FLOAT
bool_to_float (BOOLEAN b)
{
	return b ? 1. : 0.;
}
/*===============================
 * add_pvalues -- Add two PVALUEs
 *=============================*/
void
add_pvalues (PVALUE val1,
             PVALUE val2,
             BOOLEAN *eflg)
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   u1.i += u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f += u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*====================================
 * sub_pvalues -- Subtract two PVALUEs
 *==================================*/
void
sub_pvalues (PVALUE val1,
             PVALUE val2,
             BOOLEAN *eflg)
{
	UNION u1, u2;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   u1.i -= u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f -= u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*====================================
 * mul_pvalues -- Multiply two PVALUEs
 *==================================*/
void
mul_pvalues (PVALUE val1,
             PVALUE val2,
             BOOLEAN *eflg)
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	switch (ptype(val1)) {
	case PINT:   u1.i *= u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f *= u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*==================================
 * div_pvalues -- Divide two PVALUEs
 *================================*/
void
div_pvalues (PVALUE val1,
             PVALUE val2,
             BOOLEAN *eflg)
{
	UNION u1, u2;

	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	if (is_zero(val2)) {
		*eflg = TRUE;
		return;
	}
	switch (ptype(val1)) {
	case PINT:   u1.i /= u2.i; pvalue(val1) = u1.w; break;
	case PFLOAT: u1.f /= u2.f; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*===================================
 * mod_pvalues -- Modulus two PVALUEs
 *=================================*/
void
mod_pvalues (PVALUE val1,
             PVALUE val2,
             BOOLEAN *eflg)
{
	UNION u1, u2;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	u1.w = pvalue(val1);
	u2.w = pvalue(val2);
	if (is_zero(val2)) {
		*eflg = TRUE;
		return;
	}
	switch (ptype(val1)) {
	case PINT:   u1.i %= u2.i; pvalue(val1) = u1.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	delete_pvalue(val2);
}
/*===================================================================+
 * eqv_pvalues -- See if two PVALUEs are equal (no change to PVALUEs)
 *==================================================================*/
BOOLEAN
eqv_pvalues (PVALUE val1,
             PVALUE val2)
{
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
		default:
			rel = (pvalue(val1) == pvalue(val2));
			break;
		}
	}
	return rel;
}
/*===========================================
 * eq_pvalues -- See if two PVALUEs are equal
 *  and delete val2
 *=========================================*/
void
eq_pvalues (PVALUE val1,
            PVALUE val2,
            BOOLEAN *eflg)
{
	BOOLEAN rel;

	if (*eflg) return;
	eq_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	case PSTRING:
		rel = eq_pstrings(val1, val2);
		break;
	default:
		rel = (pvalue(val1) == pvalue(val2));
		break;
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*===============================================
 * eq_pstrings -- Compare two PSTRINGS
 *=============================================*/
static BOOLEAN
eq_pstrings (PVALUE val1, PVALUE val2)
{
	STRING str1 = pvalue(val1), str2 = pvalue(val2);
	if (!str1) str1 = "";
	if (!str2) str2 = "";
	return eqstr(str1, str2);
}
/*===============================================
 * ne_pvalues -- See if two PVALUEs are not equal
 *=============================================*/
void
ne_pvalues (PVALUE val1,
            PVALUE val2,
            BOOLEAN *eflg)
{
	BOOLEAN rel;

	if (*eflg) return;
	eq_conform_pvalues(val1, val2, eflg);

#ifdef DEBUG
	llwprintf("ne_pvalues: val1, val2, rel = ");
	show_pvalue(val1);
	llwprintf(", ");
	show_pvalue(val2);
	llwprintf(", ");
#endif

	if (*eflg) return;
	switch (ptype(val1)) {
	case PSTRING:
		rel = !eq_pstrings(val1, val2);
		break;
	default:
		rel = (pvalue(val1) != pvalue(val2));
		break;
	}

#ifdef DEBUG
	llwprintf("%d\n", rel);
#endif

	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*================================================
 * le_pvalues -- Check <= relation between PVALUEs
 *==============================================*/
void
le_pvalues (PVALUE val1,
            PVALUE val2,
            BOOLEAN *eflg)
{
	BOOLEAN rel;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) <= (INT) pvalue(val2));
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*================================================
 * ge_pvalues -- Check >= relation between PVALUEs
 *==============================================*/
void
ge_pvalues (PVALUE val1,
            PVALUE val2,
            BOOLEAN *eflg)
{
	BOOLEAN rel;
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;

#ifdef DEBUG
	llwprintf("ge_pvalues: val1, val2 = ");
	show_pvalue(val1);
	llwprintf(", ");
	show_pvalue(val2);
	llwprintf("\n");
#endif

	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) >= (INT) pvalue(val2));
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
}
/*===============================================
 * lt_pvalues -- Check < relation between PVALUEs
 *=============================================*/
void
lt_pvalues (PVALUE val1,
            PVALUE val2,
            BOOLEAN *eflg)
{
	BOOLEAN rel;
	if (prog_debug) {
		llwprintf("lt_pvalues: val1 = ");
		show_pvalue(val1);
		llwprintf(" val2 = ");
		show_pvalue(val2);
		llwprintf(" eflg = %d\n", *eflg);
	}
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (prog_debug) {
		llwprintf("lt_pvalues: after conforming: val1 = ");
		show_pvalue(val1);
		llwprintf(" val2 = ");
		show_pvalue(val2);
		llwprintf(" eflg = %d\n", *eflg);
	}
	if (*eflg) return;
	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) < (INT) pvalue(val2));
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
	if (prog_debug) {
		llwprintf("lt_pvalues: at end: val1 = ");
		show_pvalue(val1);
		llwprintf("\n");
	}
}
/*===============================================
 * gt_pvalues -- Check > relation between PVALUEs
 *=============================================*/
void
gt_pvalues (PVALUE val1,
            PVALUE val2,
            BOOLEAN *eflg)
{
	BOOLEAN rel;
if (prog_debug) {
	llwprintf("gt_pvalues: at start: val1 = ");
	show_pvalue(val1);
	llwprintf(" val2 = ");
	show_pvalue(val2);
	llwprintf(" eflg = %d\n", *eflg);
}
	if (*eflg) return;
	num_conform_pvalues(val1, val2, eflg);
	if (*eflg) return;
	switch (ptype(val1)) {
	/*case LONG:*/
	default: rel = ((INT) pvalue(val1) > (INT) pvalue(val2));
if (prog_debug) llwprintf("rel is %d\n", rel);
	}
	set_pvalue(val1, PBOOL, (VPTR)rel);
	delete_pvalue(val2);
if (prog_debug) {
	llwprintf("gt_pvalues: at end: val1 = ");
	show_pvalue(val1);
	llwprintf("\n");
}
}
/*==============================
 * exp_pvalues -- Exponentiation
 *============================*/
void
exp_pvalues (PVALUE val1,
             PVALUE val2,
             BOOLEAN *eflg)
{
	UNION u;
	INT xi, i, n;
	FLOAT xf;

	if (*eflg) return;
	coerce_pvalue(PINT, val2, eflg);
	if (*eflg) return;
	u.w = pvalue(val1);
	n = (INT) pvalue(val2);
	switch (ptype(val1)) {
	case PINT:
		xi = 1;
		for (i = 1; i <= n; i++)
			xi *= u.i;
		u.i = xi;
		break;
	case PFLOAT:
		xf = 1.;
		for (i = 1; i <= n; i++)
			xf *= u.f;
		u.f = xf;
		break;
	/*case PLONG:*/
	default: u.i = 0;
	}
	set_pvalue(val1, ptype(val1), (VPTR)u.w);
	delete_pvalue(val2);
}
/*==================================
 * incr_pvalue -- Increment a PVALUE
 *================================*/
void
incr_pvalue (PVALUE val,
             BOOLEAN *eflg)
{
	UNION u;
	if (*eflg) return;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT:   u.i += 1;  pvalue(val) = u.w; break;
	case PFLOAT: u.f += 1.; pvalue(val) = u.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; break;
	}
	return;
}
/*==================================
 * decr_pvalue -- Decrement a PVALUE
 *================================*/
void
decr_pvalue (PVALUE val,
             BOOLEAN *eflg)
{
	UNION u;
	if (*eflg) return;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT:   u.i -= 1;  pvalue(val) = u.w; break;
	case PFLOAT: u.f -= 1.; pvalue(val) = u.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; break;
	}
}
/*============================
 * neg_pvalue -- Negate PVALUE
 *==========================*/
void
neg_pvalue (PVALUE val,
            BOOLEAN *eflg)
{
	UNION u;
	if (*eflg) return;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT:   u.i = -u.i; pvalue(val) = u.w; break;
	case PFLOAT: u.f = -u.f; pvalue(val) = u.w; break;
	/*case PLONG:*/
	default: *eflg = TRUE; return;
	}
	return;
}
/*=================================
 * is_zero -- See if PVALUE is zero
 *===============================*/
BOOLEAN
is_zero (PVALUE val)
{
	UNION u;
	u.w = pvalue(val);
	switch (ptype(val)) {
	case PINT: return u.i == 0;
	case PFLOAT: return u.f == 0.;
	default: return TRUE;
	}
}
/*======================================================
 * insert_symtab -- Update symbol table with new PVALUE
 * SYMTAB stab:  symbol table
 * STRING iden:  variable in symbol table
 * INT type:     type of new value to assign to identifier
 * VPTR value:   new value of identifier
 *====================================================*/
void
insert_symtab (SYMTAB stab, STRING iden, INT type, VPTR value)
{
	PVALUE val = (PVALUE) valueof_ptr(stab.tab, iden);
	if (val) delete_pvalue(val);
	insert_table_ptr(stab.tab, iden, create_pvalue(type, value));
}
/*======================================================
 * insert_symtab_pvalue -- Update symbol table with PVALUE
 * SYMTAB stab:  symbol table
 * STRING iden:  variable in symbol table
 * PVALUE val:   already created PVALUE
 *====================================================*/
void
insert_symtab_pvalue (SYMTAB stab, STRING iden, PVALUE val)
{
	PVALUE oldval = (PVALUE) valueof_ptr(stab.tab, iden);
	if (oldval) delete_pvalue(oldval);
	insert_table_ptr(stab.tab, iden, val);
}
/*======================================================
 * delete_symtab -- Delete a value from a symbol table
 * SYMTAB stab:  symbol table
 * STRING iden: variable in symbol table
 * Created: 2001/03/17, Perry Rapp
 *====================================================*/
void
delete_symtab (SYMTAB stab, STRING iden)
{
	PVALUE val = (PVALUE) valueof_ptr(stab.tab, iden);
	if (val) delete_pvalue(val);
	delete_table(stab.tab, iden);
}
/*======================================================
 * symtab_cleaner -- callback for clearing symbol table pvalues
 * clear out table values (PVALUEs), but don't touch keys
 * TABLE stab:  symbol table
 * Created: 2001/03/22, Perry Rapp
 *====================================================*/
static void
symtab_cleaner (ENTRY ent)
{
	PVALUE val = ent->uval.w;
	if (val) {
		ASSERT(is_pvalue(val));
		delete_pvalue(val);
		ent->uval.w = NULL;
	}
}
/*========================================
 * null_symtab -- Return null symbol table 
 * Created: 2001/03/24, Perry Rapp
 *======================================*/
SYMTAB
null_symtab (void)
{
	SYMTAB stab;
	stab.tab = NULL;
	return stab;
}
/*========================================
 * remove_symtab -- Remove symbol table 
 * TABLE stab:  symbol table
 * Created: 2001/03/22, Perry Rapp
 *======================================*/
void
remove_symtab (SYMTAB * stab)
{
	if (stab->tab)
	{
		traverse_table(stab->tab, symtab_cleaner);
		remove_table(stab->tab, DONTFREE);
		stab->tab = 0; /* same as *stab=null_symtab(); */
	}
}
/*======================================================
 * create_symtab -- Create a symbol table
 * Created: 2001/03/22, Perry Rapp
 *====================================================*/
void
create_symtab (SYMTAB * stab)
{
	remove_symtab(stab);
	stab->tab = create_table();
}
/*======================================================
 * in_symtab -- Does symbol table have this entry ?
 * Created: 2001/03/23, Perry Rapp
 *====================================================*/
BOOLEAN
in_symtab (SYMTAB stab, STRING key)
{
	return in_table(stab.tab, key);
}
/*=================================================
 * show_pvalue -- DEBUG routine that shows a PVALUE
 *===============================================*/
void
show_pvalue (PVALUE val)
{
	NODE node;
	CACHEEL cel;
	INT type;
	UNION u;

	if (!is_pvalue(val)) {
		if(val) llwprintf("*NOT PVALUE (%d,?)*", ptype(val));
		else llwprintf("*NOT PVALUE (NULL)*");
		return;
	}
	type = ptype(val);
	llwprintf("<%s,", ptypes[type]);
	if (pvalue(val) == NULL) {
		llwprintf("0>");
		return;
	}
	u.w = pvalue(val);
	switch (type) {
	case PINT:
		llwprintf("%d>", u.i);
		return;
	case PFLOAT:
		llwprintf("%f>", u.f);
		break;
	case PSTRING:
		llwprintf("%s>", (STRING) pvalue(val));
		return;
	case PINDI:
		cel = get_cel_from_pvalue(val);
		if (!cnode(cel))
			cel = key_to_indi_cacheel(ckey(cel));
		node = cnode(cel);
		llwprintf("%s>", nval(NAME(node)));
		return;
	default:
		llwprintf("%d>", pvalue(val)); return;
	}
}
/*======================================================
 * pvalue_to_string -- DEBUG routine that shows a PVALUE
 *  returns static buffer
 *====================================================*/
STRING
pvalue_to_string (PVALUE val)
{
	NODE node;
	CACHEEL cel;
	INT type;
	UNION u;
	static char scratch[40];
	char *p;

	if (!is_pvalue(val)) return (STRING) "*NOT PVALUE*";
	type = ptype(val);
	sprintf(scratch, "<%s,", ptypes[type]);
	p = scratch + strlen(scratch);
	if (pvalue(val) == NULL) {
		sprintf(p, "NULL>");
		return (STRING) scratch;
	}
	u.w = pvalue(val);
	switch (type) {
	case PINT:
		sprintf(p, "%d>", u.i);
		break;
	case PFLOAT:
		sprintf(p, "%f>", u.f);
		break;
	case PSTRING:
		sprintf(p, "\"%s\">", (STRING) pvalue(val));
		break;
	case PINDI:
		cel = (CACHEEL) pvalue(val);
		if (!cnode(cel))
			cel = key_to_indi_cacheel(ckey(cel));
        	node = cnode(cel);
		sprintf(p, "%s>", nval(NAME(node)));
		break;
	default:
		sprintf(p, "%p>", pvalue(val));
		break;
	}
	return (STRING) scratch;
}
/*======================================================
 * symtab_valueofbool -- Convert pvalue to boolean if present
 * SYMTAB stab:     [in] symbol table
 * STRING key:      [in] key desired
 * BOOLEAN *there:  [out] whether or not key was found
 *  returns PVALUE assigned to key in symbol table, if found
 * Created: 2001/03/22, Perry Rapp
 *====================================================*/
PVALUE
symtab_valueofbool (SYMTAB stab, STRING key, BOOLEAN *there)
{
	return (PVALUE)valueofbool_ptr(stab.tab, key, there);
}
