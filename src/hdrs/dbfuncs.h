/* 
   Copyright (c) 2004 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef dbfuncs_h_included
#define dbfuncs_h_included

/*
 vtable for backend database functions
*/

/* database environ; see below */
typedef struct tag_dbf_environ **DBF_ENVIRON;

/* open database; see below */
typedef struct tag_dbf_db **DBF_DB;

/* database environment */
		/* open & return database */
DBF_DB dbf_opendb(DBF_ENVIRON dbfenv);
		/* create & return new database */
DBF_DB dbf_createdb(DBF_ENVIRON dbfenv, CNSTRING name);
		/* does this database exist ? */
BOOLEAN dbf_dbexists(DBF_ENVIRON dbfenv, CNSTRING name);


/* database */
BOOLEAN dbf_addrecord(DBF_DB dbfdb, RKEY rkey, RAWRECORD rec, INT len);
void dbf_closedb(DBF_DB dbfdb);



#endif /* dbfuncs_h_included */
