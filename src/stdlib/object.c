/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*======================================================================
 * object.c -- routines to manipulate objects via vtable
 *====================================================================*/

#include "llstdlib.h"
#include "vtable.h"
#include "object.h"


/*=================================================
 * delete_obj -- release or delete object as appropriate
 * obj may be null
 *===============================================*/
void
delete_obj (OBJECT obj)
{
	VTABLE vtable;

	if (!obj) return; /* Like C++ delete, may be called on NULL */

	vtable = (*obj);
	ASSERT(vtable->vtable_magic == VTABLE_MAGIC);

	if ((*vtable->isref_fnc)(obj)) {
		/* refcounted */
		(*vtable->release_fnc)(obj);
	} else {
		/* non-refcounted */
		(*vtable->destroy_fnc)(obj);
	}
}
/*=================================================
 * copy_or_addref_obj -- addref or copy object (if possible)
 * obj may be null
 * returns null if object neither refcountable nor copyable
 *===============================================*/
OBJECT
copy_or_addref_obj (OBJECT obj, int deep)
{
	VTABLE vtable;

	if (!obj) return NULL;

	vtable = (*obj);
	ASSERT(vtable->vtable_magic == VTABLE_MAGIC);

	if ((*vtable->isref_fnc)(obj)) {
		/* refcounted */
		(*vtable->addref_fnc)(obj);
		return obj;
	} else if (vtable->copy_fnc) {
		/* not refcounted, but copyable */
		return (*vtable->copy_fnc)(obj, deep);
	} else {
		/* not refcounted or copyable */
		return NULL;
	}
}
/*=================================================
 * addref_object -- addref object
 * Must be valid object with addref method
 *===============================================*/
int
addref_object (OBJECT obj)
{
	VTABLE vtable;

	ASSERT(obj);

	vtable = (*obj);
	ASSERT(vtable->vtable_magic == VTABLE_MAGIC);

	ASSERT(vtable->isref_fnc);
	ASSERT(vtable->addref_fnc);
	return (*vtable->addref_fnc)(obj);
}
/*=================================================
 * release_object -- release a reference on an object
 * Must be valid object with release method
 *===============================================*/
int
release_object (OBJECT obj)
{
	VTABLE vtable;

	ASSERT(obj);

	vtable = (*obj);

	ASSERT(vtable->vtable_magic == VTABLE_MAGIC);
	ASSERT(vtable->isref_fnc);
	ASSERT(vtable->release_fnc);

	return (*vtable->release_fnc)(obj);
}

