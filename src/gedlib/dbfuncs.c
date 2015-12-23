#include "llstdlib.h"
#include "gedcom.h"
#include "btree.h"

#include "dbfuncs.h"
#include "dbfuncsi.h"


DBF_DB
dbf_opendb (DBF_ENVIRON dbfenv)
{
	ASSERT(dbfenv);
	ASSERT((*dbfenv)->opendb_fnc);
	return ((*dbfenv)->opendb_fnc)(dbfenv);
}

/* create & return new database */
DBF_DB
dbf_createdb (DBF_ENVIRON dbfenv, CNSTRING name)
{
	ASSERT(dbfenv);
	if (!(*dbfenv)->createdb_fnc) return NULL;
	return ((*dbfenv)->createdb_fnc)(dbfenv, name);
}

/* does this database exist ? */
BOOLEAN
dbf_dbexists (DBF_ENVIRON dbfenv, CNSTRING name)
{
	ASSERT(dbfenv);
	ASSERT((*dbfenv)->dbexists_fnc);
	return ((*dbfenv)->dbexists_fnc)(dbfenv, name);
}


/* database */
BOOLEAN
dbf_addrecord (DBF_DB dbfdb, RKEY rkey, RAWRECORD rec, INT len)
{
	ASSERT(dbfdb);
	ASSERT((*dbfdb)->addrecord_fnc);

	return ((*dbfdb)->addrecord_fnc)(dbfdb, rkey, rec, len);
}


void
dbf_closedb (DBF_DB dbfdb)
{
	ASSERT(dbfdb);
	ASSERT((*dbfdb)->closedb_fnc);

	((*dbfdb)->closedb_fnc)(dbfdb);
}
