/* 
   Copyright (c) 2005 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*======================================================================
 * generic.c -- routines to manipulate generics (simple values & objects)
 *====================================================================*/

#include "llstdlib.h"
#include "generic.h"
#include "object.h"

/*********************************************
 * local function prototypes
 *********************************************/

static void init_generic(GENERIC *gen);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================
 * init_generic -- construct a null generic
 *===============================================*/
static void
init_generic (GENERIC *gen)
{
	memset(gen, 0, sizeof(*gen));
	gen->selector = GENERIC_NULL;
}
/*=================================================
 * init_generic_null -- populate gen with null generic
 *===============================================*/
void
init_generic_null (GENERIC *gen)
{
	init_generic(gen);
}
/*=================================================
 * init_generic_int -- populate gen with integer generic
 *===============================================*/
void
init_generic_int (GENERIC *gen, INT ival)
{
	init_generic(gen);
	set_generic_int(gen, ival);
}
/*=================================================
 * init_generic_float -- populate gen with float generic
 *===============================================*/
void
init_generic_float (GENERIC *gen, FLOAT fval)
{
	init_generic(gen);
	set_generic_float(gen, fval);
}
/*=================================================
 * init_generic_string -- populate gen with string generic
 * This routine dups the string, and frees its copy at destruction
 *===============================================*/
void
init_generic_string (GENERIC *gen, CNSTRING sval)
{
	init_generic(gen);
	gen->selector = GENERIC_STRING;
	gen->data.sval = strdup(sval);
}
/*=================================================
 * init_generic_string_shared -- populate gen with shared string generic
 * This routine does not dup the string, and does not free it
 * Caller is reponsible for the string outliving this generic
 *===============================================*/
void
init_generic_string_shared (GENERIC *gen, STRING sval)
{
	init_generic(gen);
	gen->selector = GENERIC_STRING_SHARED;
	gen->data.sval = sval;
}
/*=================================================
 * init_generic_vptr -- populate gen with vptr (void pointer)
 * This routine copies vptr, which is opaque & will not be freed
 *===============================================*/
void
init_generic_vptr (GENERIC *gen, VPTR vptr)
{
	init_generic(gen);
	gen->selector = GENERIC_VPTR;
	gen->data.vptr = vptr;
}
/*=================================================
 * init_generic_object -- populate gen with object
 * This routine copies object, and frees its copy at destruction
 *===============================================*/
void
init_generic_object (GENERIC *gen, VPTR oval)
{
	OBJECT obj = (OBJECT)oval;
	init_generic(gen);
	gen->selector = GENERIC_OBJECT;
	gen->data.oval = obj;
	if (obj)
		addref_object(obj);
}
/*=================================================
 * set_generic_null -- populate gen with null generic
 *===============================================*/
void
set_generic_null (GENERIC *gen)
{
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
}
/*=================================================
 * set_generic_int -- populate gen with integer generic
 *===============================================*/
void
set_generic_int (GENERIC *gen, INT ival)
{
	/* no problem with self-assignment as no memory alloc involved */
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
	gen->selector = GENERIC_INT;
	gen->data.ival = ival;
}
/*=================================================
 * set_generic_float -- populate gen with float generic
 *===============================================*/
void
set_generic_float (GENERIC *gen, FLOAT fval)
{
	/* check for self-assignment */
	if (gen->selector == GENERIC_FLOAT) {
		*gen->data.fval = fval;
		return;
	}
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
	/* set new value */
	gen->selector = GENERIC_FLOAT;
	gen->data.fval = stdalloc(sizeof(fval));
	*gen->data.fval = fval;
}
/*=================================================
 * set_generic_string -- populate gen with string generic
 * This routine dups the string, and frees its copy at destruction
 *===============================================*/
void
set_generic_string (GENERIC *gen, CNSTRING sval)
{
	/* check for self-assignment */
	if (gen->selector == GENERIC_STRING
		&& eqstr(gen->data.sval, sval)) {
		return;
	}
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
	/* set new value */
	gen->selector = GENERIC_STRING;
	gen->data.sval = strdup(sval);
}
/*=================================================
 * set_generic_string_shared -- populate gen with shared string generic
 * This routine does not dup the string, and does not free it
 * Caller is reponsible for the string outliving this generic
 *===============================================*/
void
set_generic_string_shared (GENERIC *gen, STRING sval)
{
	/* check for self-assignment */
	if (gen->selector == GENERIC_STRING_SHARED
		&& eqstr(gen->data.sval, sval)) {
		return;
	}
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
	/* set new value */
	gen->selector = GENERIC_STRING_SHARED;
	gen->data.sval = sval;
}
/*=================================================
 * set_generic_vptr -- populate gen with opaque void pointer
 * This routine addrefs object, and delrefs it at destruction
 *===============================================*/
void
set_generic_vptr (GENERIC *gen, VPTR vptr)
{
	/* check for self-assignment */
	if (gen->selector == GENERIC_VPTR && gen->data.vptr == vptr) {
		return;
	}
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
	/* set new value */
	gen->selector = GENERIC_VPTR;
	gen->data.vptr = vptr;
}
/*=================================================
 * set_generic_object -- populate gen with object
 * This routine addrefs object, and delrefs it at destruction
 *===============================================*/
void
set_generic_object (GENERIC *gen, VPTR oval)
{
	/* check for self-assignment */
	if (gen->selector == GENERIC_OBJECT && gen->data.oval == oval) {
		return;
	}
	/* clear old value, freeing any associated memory as appropriate */
	clear_generic(gen);
	/* set new value */
	gen->selector = GENERIC_OBJECT;
	if (oval)
		gen->data.oval = copy_or_addref_obj((OBJECT)oval, 0);
	else
		gen->data.oval = 0;
}
/*=================================================
 * copy_generic_value -- copy from src to gen
 * This routine addrefs object, and delrefs it at destruction
 *===============================================*/
void
copy_generic_value (GENERIC *gen, const GENERIC * src)
{
	/* check for self-assignment */
	if (src == gen)
		return;
	
	/* every case of select either calls clear_generic, 
	or a set call which calls clear_generic */
	
	/* set new value */
	switch(src->selector) {
	case GENERIC_NULL: clear_generic(gen); break;
	case GENERIC_INT: set_generic_int(gen, src->data.ival); break;
	case GENERIC_FLOAT: set_generic_float(gen, *src->data.fval); break;
	case GENERIC_STRING: set_generic_string(gen, src->data.sval); break;
	case GENERIC_STRING_SHARED: set_generic_string(gen, src->data.sval); break;
	case GENERIC_VPTR: set_generic_vptr(gen, src->data.vptr); break;
	case GENERIC_OBJECT: set_generic_object(gen, src->data.oval); break;
	default: ASSERT(0); break;
	}
}

/*=================================================
 * get_generic_int -- retrieve int from inside generic
 *===============================================*/
INT
get_generic_int (GENERIC *gen)
{
	ASSERT(gen->selector == GENERIC_INT);
	return gen->data.ival;
}
/*=================================================
 * get_generic_float -- retrieve float from inside generic
 *===============================================*/
FLOAT
get_generic_float (GENERIC *gen)
{
	ASSERT(gen->selector == GENERIC_FLOAT);
	return *gen->data.fval;
}
/*=================================================
 * get_generic_string -- retrieve string from inside generic
 *===============================================*/
STRING
get_generic_string (GENERIC *gen)
{
	ASSERT(gen->selector == GENERIC_STRING || gen->selector == GENERIC_STRING_SHARED);
	return gen->data.oval;
}
/*=================================================
 * get_generic_vptr -- retrieve opaque VPTR from inside generic
 *===============================================*/
VPTR
get_generic_vptr (GENERIC *gen)
{
	ASSERT(gen->selector == GENERIC_VPTR);
	return gen->data.vptr;
}
/*=================================================
 * get_generic_object -- retrieve object from inside generic
 *===============================================*/
VPTR
get_generic_object (GENERIC *gen)
{
	ASSERT(gen->selector == GENERIC_OBJECT);
	return gen->data.oval;
}
/*=================================================
 * clear_generic -- free any memory held
 *===============================================*/
void
clear_generic (GENERIC *gen)
{
	switch(gen->selector) {
	case GENERIC_NULL: break;
	case GENERIC_INT: break;
	case GENERIC_FLOAT:
		stdfree(gen->data.fval);
		gen->data.fval = 0;
	case GENERIC_STRING:
		stdfree(gen->data.sval);
		gen->data.sval = 0;
		break;
	case GENERIC_STRING_SHARED: break;
	case GENERIC_VPTR: break;
	case GENERIC_OBJECT:
		delete_obj(gen->data.oval);
		break;
	/* don't need to free any others */
	}
	gen->selector = GENERIC_NULL;
	gen->data.oval = 0;
}
/*=================================================
 * is_generic_null -- return TRUE if generic is null
 *===============================================*/
BOOLEAN
is_generic_null (GENERIC *gen)
{
	return gen->selector == GENERIC_NULL;
}
/*=================================================
 * is_generic_int -- return TRUE if generic is an integer
 *===============================================*/
BOOLEAN
is_generic_int (GENERIC *gen)
{
	return gen->selector == GENERIC_INT;
}
/*=================================================
 * is_generic_float -- return TRUE if generic is a float
 *===============================================*/
BOOLEAN
is_generic_float (GENERIC *gen)
{
	return gen->selector == GENERIC_FLOAT;
}
/*=================================================
 * is_generic_string -- return TRUE if generic is a string or shared string
 *===============================================*/
BOOLEAN
is_generic_string (GENERIC *gen)
{
	return gen->selector == GENERIC_STRING || GENERIC_STRING_SHARED;
}
/*=================================================
 * is_generic_vptr -- return TRUE if generic is a vptr (void pointer)
 *===============================================*/
BOOLEAN
is_generic_vptr (GENERIC *gen)
{
	return gen->selector == GENERIC_VPTR;
}
/*=================================================
 * is_generic_object -- return TRUE if generic is an object
 *===============================================*/
BOOLEAN
is_generic_object (GENERIC *gen)
{
	return gen->selector == GENERIC_OBJECT;
}
