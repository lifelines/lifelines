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


void
delete_obj (OBJECT obj)
{
	VTABLE vtable;

	if (!obj || !(*obj)) return;

	vtable = (*obj);
	ASSERT(vtable->vtable_magic == VTABLE_MAGIC);

	if ((*vtable->isref_fnc)(obj)) {
		/* reference counted */
		(*vtable->delref_fnc)(obj);
	} else {
		/* not reference counted */
		(*vtable->destroy_fnc)(obj);
	}
}

