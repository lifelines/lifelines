#include "sys_inc.h"
#include "llstdlib.h"
#include "arch.h"
#include "btree.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "version.h"
#include "lloptions.h"
#include "codesets.h"
#include "menuitem.h"
#include "zstr.h"
#include "icvt.h"
#include "date.h"
#include "mychar.h"
#include "charprops.h"
#include "xlat.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSdbrecstats;


/*********************************************
 * local function prototypes
 *********************************************/

static void add_dbs_to_list(LIST dblist, LIST dbdesclist, STRING dir);
static CNSTRING getdbdesc(STRING path, STRING userpath);


/*==================================================
 * get_dblist -- find all dbs on path
 *  path:       [IN]  list of directories to be searched
 *  dblist:     [OUT] list of database paths found
 *  dbdesclist: [OUT] list of descriptions of databases found
 *================================================*/
INT
get_dblist (STRING path, LIST * dblist, LIST * dbdesclist)
{
	STRING dirs=0;
	INT ndirs=0;
	STRING p=0;
	ASSERT(!(*dblist));
	ASSERT(!(*dbdesclist));
	*dblist = create_list2(LISTDOFREE);
	*dbdesclist = create_list2(LISTDOFREE);
	if (!path || !path[0])
		return 0;
	dirs = (STRING)stdalloc(strlen(path)+2);
	/* find directories in dirs & delimit with zeros */
	ndirs = chop_path(path, dirs);
	/* now process each directory */
	for (p=dirs; *p; p+=strlen(p)+1) {
		add_dbs_to_list(*dblist, *dbdesclist, p);
	}
	strfree(&dirs);
	return length_list(*dblist);
}
/*==================================================
 * add_dbs_to_list -- Add all dbs in specified dir to list
 *  dblist: [I/O] list of databases found
 *  dir:    [IN]  directory to be searched for more databases
 *================================================*/
static void
add_dbs_to_list (LIST dblist, LIST dbdesclist, STRING dir)
{
	int n=0;
	struct dirent **programs=0;
	char candidate[MAXPATHLEN];
	char userpath[MAXPATHLEN];
	CNSTRING dbstr=0;
	char dirbuf[MAXPATHLEN];

	llstrncpy(dirbuf, dir, sizeof(dirbuf), 0);
	if (!expand_special_fname_chars(dirbuf, sizeof(dirbuf), uu8))
		return;

	n = scandir(dirbuf, &programs, 0, 0);
	if (n < 0) return;
	while (n--) {
		concat_path(dirbuf, programs[n]->d_name, uu8, candidate, sizeof(candidate));
		concat_path(dir, programs[n]->d_name, uu8, userpath, sizeof(userpath));
		if ((dbstr = getdbdesc(candidate, userpath)) != NULL) {
			back_list(dblist, strsave(userpath));
			back_list(dbdesclist, (STRING)dbstr);
		}
		stdfree(programs[n]);
	}
	stdfree(programs);
}
/*==================================================
 * getdbdesc -- Check a file or directory to see if it
 *  is a lifelines database
 *  returns stdalloc'd string or NULL (if not db)
 *================================================*/
static CNSTRING
getdbdesc (STRING path, STRING userpath)
{
	INT nindis=0, nfams=0, nsours=0, nevens=0, nothrs=0;
	char desc[MAXPATHLEN] = "";
	char * errptr = 0;
	if (xrefs_get_counts_from_unopened_db(path, &nindis, &nfams
		, &nsours, &nevens, &nothrs, &errptr)) {

		llstrapps(desc, sizeof(desc), uu8, userpath);
		llstrapps(desc, sizeof(desc), uu8, " <");
		llstrappf(desc, sizeof(desc), uu8, _(qSdbrecstats)
			, nindis, nfams, nsours, nevens, nothrs);
		llstrappf(desc, sizeof(desc), uu8, ">");
	} else if (errptr) {
		llstrapps(desc, sizeof(desc), uu8, userpath);
		llstrappf(desc, sizeof(desc), uu8, " - bad db: %s", errptr);
	}
	return desc[0] ? strsave(desc) : 0;
}
/*====================================================
 * release_dblist -- free a dblist (caller is done with it)
 *==================================================*/
void
release_dblist (LIST dblist)
{
	if (dblist) {
		destroy_list(dblist);
	}
}
