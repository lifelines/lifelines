/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * proptbls.c -- functions for making lists or arrays of property tables
 *  for files (ie, a property table for each file)
 *==============================================================*/


#include "llstdlib.h"
#include "arch.h" /* dirent used in scandir */
#include "table.h"
#include "proptbls.h"

/*==========================================================
 * add_prop_dnum -- Add named property table as new last display order
 * Caller must have already dup'd name & value
 * Created: 2002/10/19, Perry Rapp
 *========================================================*/
void
add_prop_dnum (TABLE props, STRING name, STRING value)
{
	STRING str = valueof_str(props, "dn");
	INT n = ll_atoi(str, 0)+1;
	char temp[20];
	sprintf(temp, "d%d", n);
	insert_table_str(props, strsave(temp), name);
	insert_table_str(props, strsave(name), value);
	sprintf(temp, "%d", n);
	replace_table_str(props, strsave("dn"), strsave(temp), FREEBOTH);
}
/*==========================================================
 * set_prop_dnum -- Set named property in table, at specified display number
 * Caller must have already dup'd name & value
 * Created: 2002/10/19, Perry Rap9
 *========================================================*/
void
set_prop_dnum (TABLE props, INT n, STRING name, STRING value)
{
	STRING str = valueof_str(props, "dn");
	INT max = ll_atoi(str, 0);
	char temp[20];
	sprintf(temp, "d%d", n);
	replace_table_str(props, strsave(temp), name, FREEBOTH);
	replace_table_str(props, strsave(name), value, FREEBOTH);
	if (n>max) {
		sprintf(temp, "%d", n);
		replace_table_str(props, strsave("dn"), strsave(temp), FREEBOTH);
	}
}
/*===================================================
 * add_dir_files_to_proplist -- Add all files in dir to list of property tables
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
INT
add_dir_files_to_proplist (CNSTRING dir, SELECT_FNC selectfnc, LIST list)
{
	struct dirent **programs;
	INT n = scandir(dir, &programs, selectfnc, alphasort);
	INT i;
	for (i=0; i<n; ++i) {
		TABLE table = create_table();
		set_prop_dnum(table, 1, strsave("filename"), strsave(programs[i]->d_name));
		set_prop_dnum(table, 2, strsave("dir"), strsave(dir));
		stdfree(programs[i]);
		programs[i] = NULL;
		back_list(list, table);
	}
	if (n>0)
		stdfree(programs);
	return i;
}
/*===================================================
 * add_path_files_to_proplist -- Add all files on path to list of property tables
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
INT
add_path_files_to_proplist (CNSTRING path, SELECT_FNC selectfnc, LIST list)
{
	STRING dirs, p;
	INT ct=0;
	if (!path || !path[0]) return 0;
	dirs = (STRING)stdalloc(strlen(path)+2);
	chop_path(path, dirs);
	for (p=dirs; *p; p+=strlen(p)+1) {
		add_dir_files_to_proplist(p, selectfnc, list);
	}
	return ct;
}
/*===================================================
 * convert_proplist_to_proparray -- 
 *  Convert a list of property tables to an array of same
 *  Consumes the list (actually removes it at end)
 *  Output array is one larger than list, and last entry is NULL
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
TABLE *
convert_proplist_to_proparray (LIST list)
{
	TABLE * props;
	INT i;
	props = (TABLE *)malloc((length_list(list)+1)*sizeof(props[0]));
	i = 0;
	FORLIST(list, el)
		props[i++] = (TABLE)el;
	ENDLIST
	props[i] = NULL; /* null marker at end of array */
	remove_list(list, NULL);
	return props;
}
/*===================================================
 * get_proparray_of_files_in_path -- get array of property tables of files in path
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
TABLE *
get_proparray_of_files_in_path (CNSTRING path, SELECT_FNC selectfnc, INT * nfiles)
{
	/* get array of file property tables */
	LIST list = create_list();
	add_path_files_to_proplist(path, selectfnc, list);
	*nfiles = length_list(list);
	return convert_proplist_to_proparray(list);
}
/*===================================================
 * free_proparray -- free array of property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
void
free_proparray (TABLE ** props)
{
	INT i;
	for (i=0; (*props)[i]; ++i) {
		remove_table((*props)[i], FREEBOTH);
	}
	stdfree((*props));
	*props = NULL;
}