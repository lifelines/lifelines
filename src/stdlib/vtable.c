/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*==============================================================
 * vtable.c -- implement generic vtables for objects
 *============================================================*/

#include "standard.h"
#include "vtable.h"

/*********************************************
 * local types
 *********************************************/

typedef struct tag_generic_ref_object * GENERIC_REF_OBJECT;

/*********************************************
 * local variables
 *********************************************/

/* have compiler check types */
#ifdef not_used /* functions not used */
static struct tag_vtable testvtable = {
	VTABLE_MAGIC
	, "test"
	, 0 /* destroy_fnc */
	, &refcountable_isref
	, &refcountable_addref
	, &refcountable_release
	, 0 /* copy_fnc */
	, &generic_get_type_name
};
static struct tag_generic_ref_object testobj = { 
	&testvtable
	, 0
};
#endif

/*=================================================
 * generic_get_type_name -- simple get_type_name for any object
 *===============================================*/
const char *
generic_get_type_name (OBJECT obj)
{
	return (*obj)->vtable_class;
}
/*=================================================
 * nonrefcountable_isref -- simple isref for non-refcountable object
 *===============================================*/
int
nonrefcountable_isref (OBJECT obj)
{
	obj = obj; /* NOTUSED */
	return 0;
}
/*=================================================
 * refcountable_isref -- simple isref for generic refcountable object
 *===============================================*/
int
refcountable_isref (OBJECT obj)
{
	obj = obj; /* NOTUSED */
	return 1;
}
/*=================================================
 * refcountable_addref -- simple addref for generic refcountable object
 *===============================================*/
int
refcountable_addref (OBJECT obj)
{
	GENERIC_REF_OBJECT rob = (GENERIC_REF_OBJECT)obj;
	return ++rob->ref;
}
/*=================================================
 * refcountable_release -- simple release for generic refcountable object
 *===============================================*/
int
refcountable_release (OBJECT obj)
{
	GENERIC_REF_OBJECT rob = (GENERIC_REF_OBJECT)obj;
	int ref = --rob->ref;
	if (!ref)
		(*rob->vtable->destroy_fnc)(&rob->vtable);
	return ref;
}
