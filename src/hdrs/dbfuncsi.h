/* 
   Copyright (c) 2004 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef dbfuncsi_h_included
#define dbfuncsi_h_included

#ifndef dbfuncs_h_included
#include "dbfuncs.h"
#endif

/*
 vtable for backend database functions
*/


/* database environment; stuff not dependent on a particular database */
struct tag_dbf_environ {
		/* open & return database */
	DBF_DB (*opendb_fnc)(DBF_ENVIRON); 
		/* create & return new database */
	DBF_DB (*createdb_fnc)(DBF_ENVIRON, CNSTRING name);
		/* does this database exist ? */
	BOOLEAN (*dbexists_fnc)(DBF_ENVIRON, CNSTRING name);

};

/* database object */
struct tag_dbf_db {
	BOOLEAN (*addrecord_fnc)(DBF_DB, RKEY rkey, RAWRECORD rec, INT len);
	void (*closedb_fnc)(DBF_DB);
};



#endif /* dbfuncsi_h_included */
