/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*======================================================================
 * array.c -- dynamic array 
 *====================================================================*/

#include "llstdlib.h"
#include "array.h"
#include "object.h"
#include "indiseq.h" /* we use the indiseq qsort */
#include "vtable.h"


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void array_destructor(VTABLE *obj);
static INT array_comparator(SORTEL el1, SORTEL el2, VPTR param);

/*********************************************
 * local variables
 *********************************************/

static struct tag_vtable vtable_for_array = {
	VTABLE_MAGIC
	, "array"
	, &array_destructor
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/


/*=========================================================
 * add_array_obj -- Append object to end of array
 *=======================================================*/
void
add_array_obj (ARRAY array, OBJECT obj)
{
	set_array_obj(array, ASize(array), obj);
}
/*=========================================================
 * create_array_objval -- Create array (which holds objects)
 *=======================================================*/
ARRAY
create_array_objval (INT size)
{
	ARRAY array = 0;
	if (!size) size=20;
	ASSERT(size >= 1);
	array = (ARRAY)stdalloc(sizeof(*array));
	memset(array, 0, sizeof(*array));
	array->vtable = &vtable_for_array;
	ASize(array) = 0;
	AMax(array) = size;
	AData(array) = (void **)stdalloc(size * sizeof(AData(array)[0]));
	return array;
}
/*=========================================================
 * destroy_array -- Delete array (releasing all elements)
 *=======================================================*/
void
destroy_array (ARRAY array)
{
	int i;
	for (i=0; i<ASize(array); ++i) {
		OBJECT obj = (OBJECT)AData(array)[i];
		delete_obj(obj);
	}
	stdfree(AData(array));
	AData(array) = 0;
	stdfree(array);
}
/*=========================================================
 * set_array_obj -- Set element (object) in array
 *  grow if necessary
 *=======================================================*/
void
set_array_obj (ARRAY array, INT i, OBJECT obj)
{
	ASSERT(i>=0);
	ASSERT(i< 0x1000000); /* 16,777,216 */
	if (i>=AMax(array)) {
		enlarge_array(array, i);
	}
	if (i>=ASize(array)) {
		int j;
		for (j=ASize(array); j<i; ++j)
			AData(array)[j] = 0;
		ASize(array)=i+1;
	}
	AData(array)[i] = obj;
}
/*=========================================================
 * get_array_obj -- Get element (object) from array
 *=======================================================*/
OBJECT
get_array_obj (ARRAY array, INT i)
{
	if (i<0 || i>ASize(array)) return 0;
	return (OBJECT)AData(array)[i];
}
/*==================================================
 * delete_array_obj - Delete element from array
 * This of course is somewhat expensive, as the array must be shifted down
 *================================================*/
BOOLEAN
delete_array_obj (ARRAY array, INT i)
{
	if (i<0 || i>ASize(array)) return FALSE;
	while (i<ASize(array)) {
		AData(array)[i] = AData(array)[i+1];
		++i;
	}
	--ASize(array);
	return TRUE;
}
/*=========================================================
 * get_array_size -- Return size of array (# of elements)
 *=======================================================*/
INT
get_array_size (ARRAY array)
{
	return ASize(array);
}
/*=========================================================
 * enlarge_array -- Make array large enough for [space] elements
 *=======================================================*/
void
enlarge_array (ARRAY array, INT space)
{
	int newsize = AMax(array);
	void ** ptr;
	int i;
	while (newsize <= space)
		newsize <<= 2;
	if (newsize<AMax(array)) return;
	ptr = (void **)stdalloc(newsize * sizeof(AData(array)[0]));
	for (i = 0; i < ASize(array); i++)
		ptr[i] = AData(array)[i];
	stdfree(AData(array));
	AData(array) = ptr;
	AMax(array) = newsize;
}
/*=========================================================
 * sort_array -- Sort elements of array using comparator
 *  function
 *=======================================================*/
struct tag_sort_array_info {
	VPTR param;
	OBJCMPFNC cmp;
};
static INT array_comparator (SORTEL el1, SORTEL el2, VPTR param)
{
	OBJECT obj1 = (OBJECT)el1;
	OBJECT obj2 = (OBJECT)el2;
	struct tag_sort_array_info * info = (struct tag_sort_array_info *)param;
	return (*info->cmp)(obj1, obj2, info->param);
}
void
sort_array (ARRAY array, OBJCMPFNC cmp, VPTR param)
{
	struct tag_sort_array_info info;
	info.cmp = cmp;
	info.param = param;
	partition_sort((SORTEL *)&AData(array)[0], ASize(array), array_comparator, &info);
}
/*=================================================
 * array_destructor -- destructor for array
 *  (destructor entry in vtable)
 *===============================================*/
static void
array_destructor (VTABLE *obj)
{
	ARRAY array = (ARRAY)obj;
	ASSERT((*obj) == &vtable_for_array);
	destroy_array(array);
}

