/* 
   Copyright (c) 2003-2005 Perry Rapp
   "The X11 license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * rptsort.c -- Implement report language built-in sort functions
 *  ("sort" and "rsort")
 *==============================================================*/


#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "interpi.h"
#include "array.h"

 /*========================================
 * sort_array_by_array -- sort first array of pvalues
 *  by comparing keys in second array of pvalues
 *======================================*/
struct array_by_pvarray_info {
	ARRAY arr_vals;
	ARRAY arr_keys;
};
#if UNUSED_CODE
static INT
obj_lookup_comparator (OBJECT *pobj1, OBJECT *pobj2, VPTR param)
{
	struct array_by_pvarray_info * info = (struct array_by_pvarray_info *)param;
	if (info->arr_keys) {
		int i1 = pobj1 - (OBJECT *)&AData(info->arr_vals)[0];
		int i2 = pobj2 - (OBJECT *)&AData(info->arr_vals)[0];
		PVALUE val1 = AData(info->arr_keys)[i1];
		PVALUE val2 = AData(info->arr_keys)[i2];
		return pvalues_collate(val1, val2);
	} else {
		PVALUE val1 = (PVALUE)(*pobj1);
		PVALUE val2 = (PVALUE)(*pobj2);
		return pvalues_collate(val1, val2);
	}
}
#endif
/*========================================
 * sortimpl -- sort first container [using second container as keys]
 * This implements llrpt_sort and llrpt_rsort.
 *======================================*/
typedef struct tag_sortpair {
	PVALUE value;
	PVALUE key;
} *SORTPAIR;
#if UNUSED_CODE
/* comparison fnc to use with our partition_sort, commented out below */
static INT
sortpaircmp (SORTEL el1, SORTEL el2, VPTR param)
{
	SORTPAIR sp1 = (SORTPAIR)el1;
	SORTPAIR sp2 = (SORTPAIR)el2;
	return pvalues_collate(sp1->key, sp2->key);
}
#endif
static INT
sortpair_bin (const void * el1, const void * el2)
{
	SORTPAIR sp1, sp2;
	ASSERT(el1);
	ASSERT(el2);
	sp1 = *(SORTPAIR *)el1;
	sp2 = *(SORTPAIR *)el2;
	ASSERT(sp1->key);
	ASSERT(sp2->key);
	return pvalues_collate(sp1->key, sp2->key);
}
static PVALUE
sortimpl (PNODE node, SYMTAB stab, BOOLEAN *eflg, BOOLEAN fwd)
{
	PNODE arg = (PNODE) iargs(node);
	PVALUE val1 = eval_without_coerce(arg, stab, eflg), val2=0;
	LIST list_vals = 0, list_keys = 0;
	ARRAY arr_vals = 0, arr_keys = 0;
	INT nsort = 0; /* size of array & index */
	INT i=0;
	struct tag_sortpair * array = 0;
	SORTPAIR * index = 0;
	/* 1st is values collection */
	/* it must be a list or array */
	if (which_pvalue_type(val1) == PLIST) {
		list_vals = pvalue_to_list(val1);
		nsort = length_list(list_vals);
		array = (SORTPAIR)stdalloc(nsort * sizeof(array[0]));
		i=0;
		FORLIST(list_vals, el)
			array[i++].value = (PVALUE)el;
		ENDLIST
	} else if (which_pvalue_type(val1) == PARRAY) {
		arr_vals = pvalue_to_array(val1);
		nsort = get_array_size(arr_vals);
		array = (SORTPAIR)stdalloc(nsort * sizeof(array[0]));
		for (i=0; i<nsort; ++i) {
			array[i].value = (PVALUE)get_array_obj(arr_vals, i);
		}
	} else {
		prog_error(node, _("First argument to (r)sort must be list or array"));
		*eflg = TRUE;
		goto exit_sort;
	}
	/* (optional) 2nd argument is keys collection */
	/* we use the keys to collate */
	/* (if keys collection not provided, we collate on values) */
	arg = inext(arg);
	if (arg) {
		val2 = eval_without_coerce(arg, stab, eflg);
		if (which_pvalue_type(val2) == PLIST) {
			list_keys = pvalue_to_list(val2);
			if (nsort != length_list(list_keys)) {
				prog_error(node, _("Arguments to (r)sort must be of same size"));
				*eflg = TRUE;
				goto exit_sort;
			}
			i=0;
			FORLIST(list_keys, el)
				array[i++].key = (PVALUE)el;
			ENDLIST
		} else if (which_pvalue_type(val2) == PARRAY) {
			arr_keys = pvalue_to_array(val2);
			if (nsort != get_array_size(arr_keys)) {
				prog_error(node, _("Arguments to (r)sort must be of same size"));
				*eflg = TRUE;
				goto exit_sort;
			}
			for (i=0; i<nsort; ++i) {
				PVALUE val = (PVALUE)get_array_obj(arr_keys, i);
				array[i].key = val;
			}
		} else {
			prog_error(node, _("Second argument to (r)sort must be list or array"));
			*eflg = TRUE;
			return NULL;
		}
	}
	if (!arr_keys && !list_keys) {
		/* no keys collection (1st argument), */
		/* so sort directly on values collection (2nd argument) */
		for (i=0; i<nsort; ++i) {
			array[i].key = array[i].value;
		}
	}
	index = (SORTPAIR *)stdalloc(nsort * sizeof(index[0]));
	for (i=0; i<nsort; ++i) {
		index[i] = &array[i];
	}

	qsort(index, nsort, sizeof(index[0]), sortpair_bin);

/* I tried speeding up the lifelines version by removing recursion and
	doing median of three pivot, but it is still much slower than qsort
	on MS-Windows (Perry, 2003-03-01)
*/
	/* partition_sort((SORTEL *)index, nsort, sortpaircmp, 0);*/

	/* Now we reorder both the values (1st) and keys (2nd) collections */

	/* reorder the values collection (1st argument) */
	if (list_vals) {
		LIST_ITER listit=0;
		VPTR ptr=0;
		i=0;
		if (fwd)
			listit = begin_list_rev(list_vals);
		else
			listit = begin_list(list_vals);
		while (next_list_ptr(listit, &ptr)) {
			change_list_ptr(listit, index[i]->value);
			++i;
		}
		end_list_iter(&listit);
	} else {
		INT j;
		ASSERT(arr_vals);
		for (i=0; i<nsort; ++i) {
			OBJECT obj = (OBJECT)index[i]->value;
			j = (fwd ? i : nsort-i-1);
			set_array_obj(arr_vals, j, obj);
		}
	}
	if (list_keys) {
		LIST_ITER listit=0;
		VPTR ptr=0;
		i=0;
		if (fwd)
			listit = begin_list_rev(list_keys);
		else
			listit = begin_list(list_keys);
		while (next_list_ptr(listit, &ptr)) {
			change_list_ptr(listit, index[i]->key);
			++i;
		}
		end_list_iter(&listit);
	} else if (arr_keys) {
		INT j;
		for (i=0; i<nsort; ++i) {
			OBJECT obj = (OBJECT)index[i]->key;
			j = (fwd ? i : nsort-i-1);
			set_array_obj(arr_keys, j, obj);
		}
	}
	/* else, no keys collection (2nd argument), so no 2nd reorder */

exit_sort:
	delete_pvalue(val1);
	delete_pvalue(val2);
	if (array)
		stdfree(array);
	if (index)
		stdfree(index);
	return NULL;
}
/*========================================
 * llrpt_sort -- sort first container [using second container as keys]
 * usage: sort(LIST [, LIST]) -> VOID
 *======================================*/
PVALUE
llrpt_sort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	return sortimpl(node, stab, eflg, TRUE);
}
/*========================================
 * llrpt_rsort -- reverse sort first container [using second container as keys]
 * usage: rsort(LIST [, LIST]) -> VOID
 *======================================*/
PVALUE
llrpt_rsort (PNODE node, SYMTAB stab, BOOLEAN *eflg)
{
	return sortimpl(node, stab, eflg, FALSE);
}
