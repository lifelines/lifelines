/* 
   generic.h
   Copyright (c) 2005 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*
 generic.h -- generic union + tag, to hold both simple types and objects
*/

#ifndef generic_h_included
#define generic_h_included


typedef enum { 
	GENERIC_NULL,
	GENERIC_INT,
	GENERIC_FLOAT,
	GENERIC_STRING,
	GENERIC_STRING_SHARED, /* not to free */
	GENERIC_VPTR, /* opaque pointer, not to free */
	GENERIC_OBJECT
} GENERIC_TYPE;

struct generic_tag {
	GENERIC_TYPE selector;
	union {
		INT     ival;
		FLOAT   *fval;
		STRING  sval;
		VPTR    oval; /* must be OBJECT */
		VPTR    vptr; /* opaque pointer */
	} data;
};
/* A GENERIC is a blob of about 12 bytes, not a pointer */
#ifndef GENERIC_TAG_GENERIC_DECLARED
typedef struct generic_tag GENERIC;
#define GENERIC_TAG_GENERIC_DECLARED
#endif /* #ifndef GENERIC_TAG_GENERIC_DECLARED */

/*
 NB:
  Most of these functions would be faster as macros,
  but during development these are functions to help catch bugs
*/

/* Functions that work on uninitialized GENERIC data */
void init_generic_null(GENERIC *gen);
void init_generic_int(GENERIC *gen, INT ival);
void init_generic_float(GENERIC *gen, FLOAT fval);
void init_generic_string(GENERIC *gen, CNSTRING sval);
void init_generic_string_shared(GENERIC *gen, STRING sval);
void init_generic_vptr(GENERIC *gen, VPTR vptr);
void init_generic_object(GENERIC *gen, VPTR oval);

/* Set new value into existing GENERIC data 
   (clears old value, unless self-assignment) 
*/
void set_generic_null(GENERIC *gen);
void set_generic_int(GENERIC *gen, INT ival);
void set_generic_float(GENERIC *gen, FLOAT fval);
void set_generic_string(GENERIC *gen, CNSTRING sval);
void set_generic_string_shared(GENERIC *gen, STRING sval);
void set_generic_vptr(GENERIC *gen, VPTR vptr);
void set_generic_object(GENERIC *gen, VPTR oval);

/* copy from one generic to another */
void copy_generic_value(GENERIC *gen, const GENERIC * src);

/* Test type of generic */
BOOLEAN is_generic_null(GENERIC *gen);
BOOLEAN is_generic_int(GENERIC *gen);
BOOLEAN is_generic_float(GENERIC *gen);
BOOLEAN is_generic_string(GENERIC *gen); /* includes shared strings */
BOOLEAN is_generic_vptr(GENERIC *gen);
BOOLEAN is_generic_object(GENERIC *gen);
 
/* Retrieve value */
INT get_generic_int(GENERIC *gen);
FLOAT get_generic_float(GENERIC *gen);
STRING get_generic_string(GENERIC *gen);
VPTR get_generic_vptr(GENERIC *gen);
VPTR get_generic_object(GENERIC *gen);

void clear_generic(GENERIC *gen);

#endif /* generic_h_included */
