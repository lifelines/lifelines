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

/* translation table, entry in tt list */
struct tt_s {
	STRING path;
	TRANTABLE tt; /* when loaded */
	BOOLEAN loadfailure;
};

/* step of a translation, either iconv_src or trantble is NULL */
struct xlat_step_s {
	STRING iconv_src;
	STRING iconv_dest;
	struct tt_s * ptt;
};


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static INT check_tt_name(CNSTRING filename, ZSTR * pzsrc, ZSTR * pzdest);
static struct xlat_step_s * create_iconv_step(CNSTRING src, CNSTRING dest);
static struct xlat_step_s * create_tt_step(struct tt_s *ptt);
static XLAT create_xlat(CNSTRING src, CNSTRING dest, BOOLEAN adhoc);
static void free_tts(void);
static void free_xlat(XLAT xlat);
static struct tt_s * get_conversion_tt(CNSTRING src, CNSTRING dest);
static struct tt_s * get_subcoding_tt(CNSTRING codeset, CNSTRING subcoding);
static void load_tt_if_needed(struct tt_s *ptt);
static void load_ttlist_from_dir(STRING dir);
static int select_tts(const struct dirent *entry);

/*********************************************
 * local variables
 *********************************************/

static LIST f_xlats=0;
static TABLE f_tts=0;
static char f_ttext[] = ".tt";

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
		xstep->ptt = 0;
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
	memset(xstep, 0, sizeof(*xstep));
	xstep->iconv_dest = strsave(dest);
	xstep->iconv_src = strsave(src);
	return xstep;
}
/*==========================================================
 * create_tt_step -- Create a tt step of a translation chain
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static struct xlat_step_s *
create_tt_step (struct tt_s *ptt)
{
	struct xlat_step_s *xstep;
	xstep = (struct xlat_step_s * )malloc(sizeof(*xstep));
	memset(xstep, 0, sizeof(*xstep));
	xstep->ptt = ptt;
	return xstep;
}
/*==========================================================
 * xl_get_xlat -- Find translation between specified codesets
 *  returns NULL if fails
 * Created: 2002/11/25 (Perry Rapp)
 *========================================================*/
XLAT
xl_get_xlat (CNSTRING src, CNSTRING dest, BOOLEAN adhoc)
{
	XLAT xlat=0;
	ZSTR zsrc=0, zdest=0;
	LIST srcsubs=0, destsubs=0;
	struct tt_s * ptt=0;
	struct xlat_step_s * xstep=0;
	STRING subcoding=0;
	
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

	/* create new xlat & fill it out */
	xlat = create_xlat(src, dest, adhoc);

	/* parse out codesets & subcodings */
	xl_parse_codeset(src, &zsrc, &srcsubs);
	xl_parse_codeset(dest, &zdest, &destsubs);

	if (destsubs) {
		FORLIST(destsubs, el)
			subcoding = (STRING)el;
			ptt = get_subcoding_tt(zs_str(zsrc), subcoding);
			if (ptt) {
				load_tt_if_needed(ptt);
				if (!ptt->loadfailure) {
					xstep = create_tt_step(ptt);
					enqueue_list(xlat->steps, xstep);
				}
			}
		ENDLIST
	}

	if (iconv_can_trans(zs_str(zsrc), zs_str(zdest))) {
		xstep = create_iconv_step(src, dest);
		enqueue_list(xlat->steps, xstep);
	} else {
		ptt = get_conversion_tt(zs_str(zsrc), zs_str(zdest));
		if (ptt) {
			load_tt_if_needed(ptt);
			if (!ptt->loadfailure) {
				xstep = create_tt_step(ptt);
				enqueue_list(xlat->steps, xstep);
			}
		}
	}

	if (srcsubs) {
		FORLIST(srcsubs, el)
			subcoding = (STRING)el;
			ptt = get_subcoding_tt(zs_str(zdest), subcoding);
			if (ptt) {
				load_tt_if_needed(ptt);
				if (!ptt->loadfailure) {
					xstep = create_tt_step(ptt);
					enqueue_list(xlat->steps, xstep);
				}
			}
		ENDLIST
	}

	/*
	TODO: 2002-11-25
	check table of conversions in ttdir
	*/
	return xlat;
}
/*==========================================================
 * get_conversion_tt -- get tt entry for code conversion, if in tt list
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static struct tt_s *
get_conversion_tt (CNSTRING src, CNSTRING dest)
{
	struct tt_s * ptt;
	ZSTR zttname = zs_news(src);
	zs_appc(&zttname, '_');
	zs_apps(&zttname, dest);
	ptt = (struct tt_s *)valueof_ptr(f_tts, zs_str(zttname));
	zs_free(&zttname);
	return ptt;
}
/*==========================================================
 * get_subcoding_tt -- get tt entry for subcoding, if in tt list
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static struct tt_s *
get_subcoding_tt (CNSTRING codeset, CNSTRING subcoding)
{
	struct tt_s * ptt;
	ZSTR zttname = zs_news(codeset);
	zs_apps(&zttname, "__");
	zs_apps(&zttname, subcoding);
	ptt = (struct tt_s *)valueof_ptr(f_tts, zs_str(zttname));
	zs_free(&zttname);
	return ptt;
}
/*==========================================================
 * load_tt_if_needed -- load tt from disk (unless already loaded)
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
static void
load_tt_if_needed (struct tt_s *ptt)
{
	ZSTR zerr=0;
	if (ptt->tt || ptt->loadfailure)
		return;
	if (!init_map_from_file(ptt->path, ptt->path, &ptt->tt, &zerr)) {
		ptt->loadfailure = TRUE;
	}
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
	if (!xlat || !xlat->steps) return cvtd;
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
		} else if (xstep->ptt) {
			/* a custom translation table step */
			custom_translate(pzstr, xstep->ptt->tt);
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
	f_tts = create_table();
	dirs = (STRING)stdalloc(strlen(ttpath)+2);
	/* find directories in dirs & delimit with zeros */
	chop_path(ttpath, dirs);
	/* now process each directory */
	for (p=dirs; *p; p+=strlen(p)+1) {
		load_ttlist_from_dir(p);
	}
	strfree(&dirs);

}
/*==========================================================
 * load_ttlist_from_dir -- Get list of all translation tables in 
 *  specified directory
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
load_ttlist_from_dir (STRING dir)
{
	struct dirent **programs;
	INT n = scandir(dir, &programs, select_tts, alphasort);
	INT i;
	for (i=0; i<n; ++i) {
		CNSTRING ttfile = programs[i]->d_name;
		/* filename without extension */
		ZSTR zfile = zs_newsubs(ttfile, strlen(ttfile)-(sizeof(f_ttext)-1));
		ZSTR zsrc=0, zdest=0;
		INT ntype = check_tt_name(zs_str(zfile), &zsrc, &zdest);
		/*
		Valid names are like so:
			UTF-8_ISO-8859-1 (type 1; code conversion)
			UTF-8__HTML (type 2; subcoding)
		*/
		if (ntype==1 || ntype==2) {
			if (!valueof_ptr(f_tts, zs_str(zfile))) {
				struct tt_s *ptt = (struct tt_s *)malloc(sizeof(*ptt));
				memset(ptt, 0, sizeof(*ptt));
				ptt->path = concat_path_alloc(dir, ttfile);
				insert_table_ptr(f_tts, strsave(zs_str(zfile)), ptt);
			}
		}
		zs_free(&zfile);
		zs_free(&zsrc);
		zs_free(&zdest);
	}
	if (n>0)
		stdfree(programs);
}
/*===========================================================
 * check_tt_name -- Parse tt filename to see what codesets it does
 *  returns 1 if codeset-codeset conversion
 *  returns 2 if codeset subcoding conversion
 *  returns 0 if not valid name for tt
 *==========================================================*/
static INT
check_tt_name (CNSTRING filename, ZSTR * pzsrc, ZSTR * pzdest)
{
	CNSTRING ptr;
	CNSTRING underbar=0;
	for (ptr=filename; *ptr; ++ptr) {
		if (*ptr=='_') {
			if (!underbar) {
				underbar = ptr;
			} else {
				if (underbar+1 != ptr)
					return 0;
			}
		} else if (*ptr==' ') {
			return 0;
		}
	}
	if (!underbar || underbar==filename ||
		!underbar[1] || (underbar[1]=='_' && underbar[2])) {
		return 0;
	}
	*pzsrc = zs_newsubs(filename, underbar-filename);
	if (underbar[1]=='_') {
		zs_sets(pzdest, underbar+2);
		return 2;
	} else {
		zs_sets(pzdest, underbar+1);
		return 1;
	}
}
/*===========================================================
 * select_tts -- choose translation tables
 *==========================================================*/
static int
select_tts (const struct dirent *entry)
{
	INT tlen = strlen(entry->d_name);
	CNSTRING entext;
	if (tlen < (INT)sizeof(f_ttext)) return 0;

	/* examine end of entry->d_name */
	entext = entry->d_name + tlen - (sizeof(f_ttext)-1);

	/* is it what we want ? use platform correct comparison, from path.c */
	if (!path_match(f_ttext, entext))
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
}
/*==========================================================
 * free_tts -- Free table of translation tables
 * Created: 2002/11/27 (Perry Rapp)
 *========================================================*/
static void
free_tts (void)
{
	if (f_tts) {
		struct table_iter_s tabit;
		if (begin_table(f_tts, &tabit)) {
			VPTR ptr;
			STRING key;
			while (next_table_ptr(&tabit, &key, &ptr)) {
				struct tt_s * ptt = (struct tt_s *)ptr;
				strfree(&ptt->path);
				remove_trantable(ptt->tt);
				ptt->tt = 0;
			}
		}
		remove_table(f_tts, FREEBOTH);
		f_tts = 0;
		xl_free_xlats();
	}
}
/*==========================================================
 * xl_parse_codeset -- Parse out subcode suffixes of a codeset
 *  eg, "CP437//TrGreekAscii//TrCyrillicAscii"
 *  will recognize CP437 as the codeset name, and list
 *  "TrGreekAscii" and "TrCyrillicAscii"  as subcodes
 * Created: 2002/12/01 (Perry Rapp)
 *========================================================*/
void
xl_parse_codeset (CNSTRING codeset, ZSTR * zcsname, LIST * subcodes)
{
	CNSTRING p=codeset, prev=codeset;
	BOOLEAN base=FALSE;
	for ( ; ; ++p) {
		if ( !p[0] || (p[0]=='/' && p[1]=='/')) {
			if (!base) {
				if (zcsname) {
					zs_free(zcsname);
					*zcsname = zs_newsubs(codeset, p-prev);
				}
				base=TRUE;
			} else {
				ZSTR ztemp=0;
				if (subcodes) {
					if (!*subcodes) {
						*subcodes = create_list();
						set_list_type(*subcodes, LISTDOFREE);
					}
					ztemp = zs_newsubs(prev, p-prev);
					enqueue_list(*subcodes, strsave(zs_str(ztemp)));
					zs_free(&ztemp);
				}
			}
			if (!p[0])
				return;
			prev = p+2;
			p = p+1; /* so we jump over both slashes */
		}
	}
}
