/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*==============================================================
 * vtable.h -- vtables for objects
 * This header is used by object implementations.
 * Clients merely using objects use object.h.
 *============================================================*/

#ifndef vtable_h_included
#define vtable_h_included

struct tag_vtable;
typedef struct tag_vtable *VTABLE;

struct tag_vtable {
		/* for sanity checking */
	int vtable_magic; 
		/* for debugging convenience */
	const char * vtable_class;
		/* destroy object */
	void (*destroy_fnc)(OBJECT obj);
		/* returns 0 if not reference counted */
	int (*isref_fnc)(OBJECT obj);
		/* increment refcount */
		/* returns new refcount, or -1 if not refcounted */
	int (*addref_fnc)(OBJECT obj);
		/* decrement refcount & delete if 0 */
		/* returns new refcount, or -1 if not refcounted */
	int (*release_fnc)(OBJECT obj);
		/* returns a copy of object */
	OBJECT (*copy_fnc)(OBJECT obj, int deep);
		/* returns name of object type (doesn't need to be freed) */
	const char * (*get_type_name_fnc)(OBJECT obj);
};

enum { VTABLE_MAGIC = 0x77999977 };

/* to use the generic refcounting implementation below,
 the object must begin like this */
struct tag_generic_ref_object {
	VTABLE vtable;
	int ref;
};


/* vtable functions implementations */

/* for any object (content to use vtable_class) */
const char * generic_get_type_name(OBJECT obj);

/* for non-refcountable object */
int nonrefcountable_isref(OBJECT obj);

/* for refcountable object (with refcount right after vtable) */
int refcountable_isref(OBJECT obj);
int refcountable_addref(OBJECT obj);
int refcountable_release(OBJECT obj);

#endif /* vtable_h_included */

