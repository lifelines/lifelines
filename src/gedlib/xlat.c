/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*===========================================================
 * xlat.c -- Implementation of translation module
 * xlat module converts between named codesets
 * It uses iconv & charmaps as steps in a chain, as needed
 *=========================================================*/

#include "llstdlib.h"
#include "translat.h"
#include "gedcom.h"
#include "zstr.h"
#include "icvt.h"
#include "gedcomi.h"
#include "xlat.h" 
#include "arch.h"

/*********************************************
 * local types
 *********************************************/

/* step of a translation, either iconv_src or trantble is NULL */
struct xlat_step_s {
	STRING iconv_src;
	STRING iconv_dest;
	TRANTABLE trantbl;
};


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static struct xlat_step_s * create_iconv_step(CNSTRING src, CNSTRING dest);
static XLAT create_xlat(CNSTRING src, CNSTRING dest, BOOLEAN adhoc);
static void free_tts(void);
static void free_xlat(XLAT xlat);
static void load_tt_from_dir(STRING dir);
static int select_tts(const struct dirent *entry);

/*********************************************
 * local variables
 *********************************************/

static LIST f_xlats=0;
static TABLE f_tts=0;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==========================================================
 * create_xlat -- Create a new translation
 * (also adds to cache)
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
static XLAT
create_xlat (CNSTRING src, CNSTRING dest, BOOLEAN adhoc)
{
	/* create & initialize new xlat */
	XLAT xlat = (XLAT)malloc(sizeof(*xlat));
	memset(xlat, 0, sizeof(*xlat));
	xlat->steps = create_list();
	set_list_type(xlat->steps, LISTDOFREE);
	xlat->src = strsave(src);
	xlat->dest = strsave(dest);
	xlat->adhoc = adhoc;
	/* add xlat to cache */
	if (!f_xlats) {
		f_xlats = create_list();
	}
	enqueue_list(f_xlats, xlat);
	return xlat;
}
/*==========================================================
 * free_xlat -- Free a translation
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
free_xlat (XLAT xlat)
{
	struct xlat_step_s * xstep=0;
	/* free each step */
	FORLIST(xlat->steps, el)
		xstep = (struct xlat_step_s *)el;
		strfree(&xstep->iconv_src);
		strfree(&xstep->iconv_dest);
		if (xstep->trantbl)
			remove_trantable(xstep->trantbl);
	ENDLIST
	make_list_empty(xlat->steps);
	remove_list(xlat->steps, 0);
}
/*==========================================================
 * create_iconv_step -- Create an iconv step of a translation chain
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static struct xlat_step_s * 
create_iconv_step (CNSTRING src, CNSTRING dest)
{
	struct xlat_step_s *xstep;
	xstep = (struct xlat_step_s * )malloc(sizeof(*xstep));
	xstep->iconv_dest = strsave(dest);
	xstep->iconv_src = strsave(src);
	xstep->trantbl = NULL;
	return xstep;
}
/*==========================================================
 * xl_get_xlat -- Find translation between specified codesets
 *  returns NULL if fails
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
XLAT
xl_get_xlat (CNSTRING src, CNSTRING dest)
{
	XLAT xlat=0;
	BOOLEAN adhoc = TRUE;
	
	if (!src || !src[0] || !dest || !dest[0])
		return 0;

	/* first check existing cache */
	if (f_xlats) {
		XLAT xlattemp;
		FORLIST(f_xlats, el)
			xlattemp = (XLAT)el;
			if (eqstr(xlattemp->src, src) && eqstr(xlattemp->dest, dest)) {
				return xlattemp;
			}
		ENDLIST
	}
	/* check if identity */
	if (eqstr(src, dest)) {
		/* new empty xlat will work for identity */
		return create_xlat(src, dest, adhoc);
	}
	if (iconv_can_trans(src, dest)) {
		struct xlat_step_s * xstep = create_iconv_step(src, dest);
		/* create new xlat & fill it out */
		xlat = create_xlat(src, dest, adhoc);
		/* put single iconv step into xlat */
		enqueue_list(xlat->steps, xstep);
		return xlat;
	}
	/*
	TODO: 2002-11-25
	check table of conversions in ttdir
	*/
	return xlat;
}
/*==========================================================
 * xl_do_xlat -- Perform a translation on a string
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
BOOLEAN
xl_do_xlat (XLAT xlat, ZSTR * pzstr)
{
	BOOLEAN cvtd=FALSE;
	struct xlat_step_s * xstep=0;
	if (!xlat) return cvtd;
	/* simply cycle through & perform each step */
	FORLIST(xlat->steps, el)
		xstep = (struct xlat_step_s *)el;
		if (xstep->iconv_src) {
			/* an iconv step */
			ZSTR ztemp=0;
			if (iconv_trans(xstep->iconv_src, xstep->iconv_dest, zs_str(*pzstr), &ztemp, "?")) {
				cvtd=TRUE;
				zs_free(pzstr);
				*pzstr = ztemp;
			} else {
				/* iconv failed, anything to do ? */
			}
		} else if (xstep->trantbl) {
			/* a custom translation table step */
			custom_translate(pzstr, xstep->trantbl);
		}
	ENDLIST
	return cvtd;
}
/*==========================================================
 * xl_load_all_tts -- Load internal list of available translation
 *  tables (based on *.tt files in TTPATH)
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_load_all_tts (CNSTRING ttpath)
{
	STRING dirs,p;
	free_tts();
	if (!ttpath ||  !ttpath[0])
		return;
	dirs = (STRING)stdalloc(strlen(ttpath)+2);
	/* find directories in dirs & delimit with zeros */
	chop_path(ttpath, dirs);
	/* now process each directory */
	for (p=dirs; *p; p+=strlen(p)+1) {
		load_tt_from_dir(p);
	}
	strfree(&dirs);

}
/*==========================================================
 * load_tt_from_dir -- Get list of all translation tables in 
 *  specified directory
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
load_tt_from_dir (STRING dir)
{
	struct dirent **programs;
	INT n = scandir(dir, &programs, select_tts, alphasort);
	INT i;
	for (i=0; i<n; ++i) {
/* TODO
		TABLE table = create_table();
		set_prop_dnum(table, 1, strsave("filename"), strsave(programs[i]->d_name));
		set_prop_dnum(table, 2, strsave("dir"), strsave(dir));
		stdfree(programs[i]);
		programs[i] = NULL;
		back_list(list, table);
		*/
	}
	if (n>0)
		stdfree(programs);
}
/*===========================================================
 * select_tts -- choose translation tables
 *==========================================================*/
static int
select_tts (const struct dirent *entry)
{
	CNSTRING goodext = ".tt";
	/* examine end of entry->d_name */
	CNSTRING entext = entry->d_name + strlen(entry->d_name) - strlen(goodext);

	/* is it what we want ? use platform correct comparison, from path.c */
	if (!path_match(goodext, entext))
		return 0;

	return 1;
}
/*==========================================================
 * free_adhoc_xlats -- Free all the adhoc translation chains
 *  (ie, the ones used during report processing)
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_free_adhoc_xlats (void)
{
    XLAT xlattemp;
	LIST newlist;
	if (!f_xlats)
		return;
	/* we don't have a way to delete items from a list,
	so just copy the ones we want to a new list */
	newlist = create_list();
	set_list_type(newlist, LISTDOFREE);
	FORLIST(f_xlats, el)
		xlattemp = (XLAT)el;
		if (xlattemp->adhoc) {
			free_xlat(xlattemp);
		} else {
			back_list(newlist, xlattemp);
		}
	ENDLIST
	make_list_empty(f_xlats);
	remove_list(f_xlats, 0);
	f_xlats = newlist;
}
/*==========================================================
 * free_xlats -- Free all the translation chains
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
void
xl_free_xlats (void)
{
    XLAT xlattemp;
	if (!f_xlats)
		return;
	FORLIST(f_xlats, el)
		xlattemp = (XLAT)el;
		free_xlat(xlattemp);
	ENDLIST
	make_list_empty(f_xlats);
	remove_list(f_xlats, 0);
	f_xlats = 0;
	free_tts();
}
/*==========================================================
 * free_tts -- Free table of translation tables
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
free_tts (void)
{
	if (f_tts) {
		remove_table(f_tts, FREEBOTH);
		f_tts = 0;
	}
}