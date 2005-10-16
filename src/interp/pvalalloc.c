/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * pvalalloc.c -- Allocation of pvalues (intrepreter values)
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
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
#include "lloptions.h"

/* define following to disable block allocator for pvalues
   this is useful to find sources of leaks with valgrind
 */
/* #define NOBLOCKALLOCATOR 1 */

/*********************************************
 * local types
 *********************************************/

/* block of pvalues - see comments in alloc_pvalue_memory() */
struct tag_pv_block
{
	struct tag_pv_block * next;
	struct tag_pvalue values[100]; /* arbitrary size may be adjusted */
};
typedef struct tag_pv_block *PV_BLOCK;
#define BLOCK_VALUES (sizeof(((PV_BLOCK)0)->values)/sizeof(((PV_BLOCK)0)->values[0]))

/*********************************************
 * local function prototypes
 *********************************************/

static void free_all_pvalues(void);
static BOOLEAN is_pvalue_or_freed(PVALUE pval);

/*********************************************
 * local variables
 *********************************************/

#ifndef NOBLOCKALLOCATOR
static PVALUE free_list = 0;
static INT live_pvalues = 0;
static PV_BLOCK block_list = 0;
#endif
static BOOLEAN reports_time = FALSE;
static BOOLEAN cleaning_time = FALSE;
#ifdef DEBUG_REPORT_MEMORY_DETAIL
static BOOLEAN alloclog_save = FALSE;
#endif
#ifndef NOBLOCKALLOCATOR
#ifdef DEBUG_PVALUES
static INT debugging_pvalues = TRUE;
#else
static INT debugging_pvalues = FALSE;
#endif
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
#ifndef NOBLOCKALLOCATOR
static void
debug_check (void)
{
	PVALUE val;
	for (val=free_list; val; val=val->value)
	{
		ASSERT(val->type == PFREED && val->value != val);
	}
}
#endif
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
PVALUE
alloc_pvalue_memory (void)
{
	PVALUE val;
	
	ASSERT(reports_time);
#ifdef NOBLOCKALLOCATOR
	val = stdalloc(sizeof(*val));
#else 
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
		for (i=0; i<(INT)BLOCK_VALUES; i++) {
			PVALUE val1 = &new_block->values[i];
			init_pvalue_vtable(val1);
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
#endif
	/* set type to uninitialized - caller ought to set type */
	ptype(val) = PNULL;
	pvalvv(val) = 0;
	return val;
}
/*========================================
 * free_pvalue_memory -- return pvalue to free-list
 * (see alloc_pvalue_memory comments)
 * Created: 2001/01/19, Perry Rapp
 *======================================*/
void
free_pvalue_memory (PVALUE val)
{
	/* see alloc_pvalue_memory for discussion of this ASSERT */
	ASSERT(reports_time);
#ifdef NOBLOCKALLOCATOR
	stdfree(val);
#else
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
#endif
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
	free_all_pnodes(); /* some nodes hold pvalues */
	free_all_pvalues();
	cleaning_time = FALSE;
	ASSERT(reports_time);
	reports_time = FALSE;
	clear_rptinfos();
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
#ifdef NOBLOCKALLOCATOR
#else
	PV_BLOCK block;
	INT found_leaks=0;
	INT orig_leaks = live_pvalues;
	TABLE leaktab = create_table_int(); /* count of leaks by type */

	/* Notes
	live_pvalues is the count of leaked pvalues
	We have to go through all blocks and free all pvalues
	in first pass, because, due to containers, pvalues can
	cross-link between blocks (ie, a list on one block could
	contain pointers to pvalues on other blocks)
	*/
	/* First check out all leaked pvalues for consistency
	 * in numbers - this has to be done first, since when we
	 * delete a set,list, etc. other pvalues get deleted as well */
	for (block = block_list; block; block = block->next) {
		INT i;
		for (i=0; i<(INT)BLOCK_VALUES; i++) {
			PVALUE val1=&block->values[i];
			CNSTRING typestr = get_pvalue_type_name(val1->type);
			if (val1->type != PFREED) {
				increment_table_int(leaktab, typestr);
				++found_leaks;
			}
		}
	}
	for (block = block_list; block; block = block->next) {
		if (live_pvalues) {
			INT i;
			for (i=0; i<(INT)BLOCK_VALUES; i++) {
				PVALUE val1=&block->values[i];
				if (val1->type != PFREED) {
					/* leaked */
					ZSTR zstr = describe_pvalue(val1);
					/* zstr is just for debugging, we don't record it */
					delete_pvalue(val1);
					zs_free(&zstr);
				}
			}
		}
	}
	ASSERT(orig_leaks == found_leaks);
	ASSERT(live_pvalues == 0);
	/* finally, free the blocks */
	while ((block = block_list)) {
		PV_BLOCK next = block->next;
		stdfree(block);
		block_list = next;
	}
	free_list = 0;
	if (found_leaks) {
		STRING report_leak_path = getlloptstr("ReportLeakLog", NULL);
		FILE * fp=0;
		if (report_leak_path)
			fp = fopen(report_leak_path, LLAPPENDTEXT);
		if (fp) {
			LLDATE date;
			TABLE_ITER tabit = begin_table_iter(leaktab);
			CNSTRING key=0;
			INT ival=0;
			fprintf(fp, _("PVALUE memory leak report:"));
			get_current_lldate(&date);
			fprintf(fp, " %s", date.datestr);
			fprintf(fp, "\n");
			while (next_table_int(tabit, &key, &ival)) {
				fprintf(fp, "  %s: ", key);
				fprintf(fp, _pl("%d item leaked", "%d items leaked", ival), ival);
				fprintf(fp, "\n");
			}
			end_table_iter(&tabit);
			fclose(fp);
			fp = 0;
		}
	}
	destroy_table(leaktab);
#endif
}
/*======================================
 * check_pvalue_validity -- ASSERT that pvalue is valid
 *====================================*/
void
check_pvalue_validity (PVALUE val)
{
	if (cleaning_time) {
		ASSERT(is_pvalue_or_freed(val));
	} else {
		ASSERT(is_pvalue(val));
	}
}
/*===========================================================
 * is_pvalue -- Checks that PVALUE is a valid type
 *=========================================================*/
BOOLEAN
is_pvalue (PVALUE pval)
{
	if (!pval) return FALSE;
	return (ptype(pval) <= PMAXLIVE);
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
	return (ptype(pval) <= PMAXLIVE);
}
/*========================================
 * create_new_pvalue -- Create new program value
 *======================================*/
PVALUE
create_new_pvalue (void)
{
	return alloc_pvalue_memory();
}
