/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-08-22 J.F.Chandler */
/*=============================================================
 * valgdcom.c -- Validate GEDCOM file
 * Copyright(c) 1993-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93     2.3.5 - 02 Sep 93
 *   3.0.0 - 06 Jul 94     3.0.2 - 18 Feb 95
 *   3.0.3 - 29 Jun 96
 *===========================================================*/

#include "sys_inc.h"
#include <stdarg.h>
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcheck.h"
#include "liflines.h"
#include "llinesi.h"
#include "impfeed.h"
#include "lloptions.h"

extern STRING qSmisixr, qSmisfxr;
extern STRING qSmulper, qSmulfam;
extern STRING qSmatper, qSmatfam;
extern STRING qSundper, qSundfam, qSundsrc;
extern STRING qSundevn, qSbadlev, qSnoname;

/* external data set by check_stdkeys() , used by addmissingkeys() */

INT gd_itot = 0;        /* total number of individuals */
INT gd_ftot = 0;        /* total number of families */
INT gd_stot = 0;        /* total number of sources */
INT gd_etot = 0;        /* total number of events */
INT gd_xtot = 0;        /* total number of others */
INT gd_imax = 0;        /* maximum individual key number */
INT gd_fmax = 0;        /* maximum family key number */
INT gd_smax = 0;        /* maximum source key number */
INT gd_emax = 0;        /* maximum event key number */
INT gd_xmax = 0;        /* maximum other key number */

static TABLE convtab = NULL;
static INT rec_type;
static BOOLEAN named = FALSE;
static INT person = -1;
static INT family = -1;
static INT event  = -1;
static INT source = -1;
static INT other  = -1;
static INT struct_len = 0;
static INT struct_max = 0;
static INT num_errors;
static INT num_warns;
static INT defline;
static BOOLEAN f_logopen = FALSE;
static FILE *f_flog = 0;
static char f_logpath[MAXPATHLEN] = "import.log";

static STRING qSmisval      = N_("Line %d: This %s line is missing a value field.");
static STRING qSundrec      = N_("Record %s is referred to but not defined.");

ELMNT *index_data = NULL;


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static INT add_even_defn(struct import_feedback * ifeed, STRING, INT);
static INT add_fam_defn(struct import_feedback * ifeed, STRING, INT);
static INT add_indi_defn(struct import_feedback * ifeed, STRING, INT, ELMNT*);
static INT add_othr_defn(struct import_feedback * ifeed, STRING, INT);
static INT add_sour_defn(struct import_feedback * ifeed, STRING, INT);
static INT add_to_structures(STRING, ELMNT);
static int check_akey (int firstchar, STRING keyp, INT *maxp);
static void check_even_links(struct import_feedback * ifeed, ELMNT);
static void check_fam_links(struct import_feedback * ifeed, ELMNT);
static void check_indi_links(struct import_feedback * ifeed, ELMNT per);
static void check_othr_links(struct import_feedback * ifeed, ELMNT);
static void check_references(struct import_feedback * ifeed);
static void check_sour_links(struct import_feedback * ifeed, ELMNT src);
static void clear_structures(void);
static void handle_fam_lev1(struct import_feedback * ifeed, STRING tag, STRING val, INT line);
static void handle_indi_lev1(struct import_feedback * ifeed, STRING, STRING, INT);
static void handle_value(STRING, INT);
static BOOLEAN openlog(void);
static void handle_warn(struct import_feedback * ifeed, STRING, ...);
static void handle_err(struct import_feedback * ifeed, STRING, ...);
static void set_import_log(STRING logpath);

/*===================================================
 * validate_gedcom -- Validate GEDCOM records in file
 *=================================================*/
BOOLEAN
validate_gedcom (struct import_feedback * ifeed, FILE *fp)
{
	INT lev, rc, curlev = 0;
	INT nindi, nfam, nsour, neven, nothr;
	ELMNT el;
	TRANMAPPING ttm = get_tranmapping(MGDIN);
	STRING xref, tag, val, msg;

	nindi = nfam = nsour = neven = nothr = 0;
	num_errors = num_warns = 0;
	f_logopen = FALSE;
	f_flog = 0;
	set_import_log(getoptstr("ImportLog", "errs.log"));
	flineno = 0;
	defline = 0;
	curlev = 0;
	clear_structures();
	convtab = create_table();


	rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, &msg);
	xref = xref ? rmvat(xref) : NULL;
	rec_type = OTHR_REC;
	while (rc != DONE)  {
		if (lev > curlev + 1 || rc == ERROR) {
			if (rc == ERROR)
				handle_err(ifeed, msg);
			else
				handle_err(ifeed, qSbadlev, flineno);
			handle_value(val, flineno);
			curlev = lev;
			rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val,
			    &msg);
			xref = xref ? rmvat(xref) : NULL;
			continue;
		}
		if (lev > 1) {
			handle_value(val, flineno);
			curlev = lev;
			rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val,
			    &msg);
			xref = xref ? rmvat(xref) : NULL;
			continue;
		}
		if (lev == 0) {
			if (rec_type == INDI_REC && !named) {
				if (getoptint("RequireNames", 0)) {
					handle_err(ifeed, qSnoname, defline);
				}
			}
			defline = flineno;
			if (eqstr("HEAD", tag) || eqstr("TRLR", tag))
				rec_type = IGNR_REC;
			else {
				INT count=0;
				if (eqstr("INDI", tag)) {
					count = ++nindi;
					rec_type = INDI_REC;
					named = FALSE;
					person = add_indi_defn(ifeed, xref, flineno, &el);
					/* pretend a NAME if the record is skipped */
					if (person == -2) named = TRUE;
				} else if (eqstr("FAM", tag)) {
					count = ++nfam;
					rec_type = FAM_REC;
					family = add_fam_defn(ifeed, xref, flineno);
				} else if (eqstr("EVEN", tag)) {
					count = ++neven;
					rec_type = EVEN_REC;
					event = add_even_defn(ifeed, xref, flineno);
				} else if (eqstr("SOUR", tag)) {
					count = ++nsour;
					rec_type = SOUR_REC;
					source = add_sour_defn(ifeed, xref, flineno);
				} else {
					count = ++nothr;
					rec_type = OTHR_REC;
					other = add_othr_defn(ifeed, xref, flineno);
				}
				if (ifeed && ifeed->added_rec_fnc)
					ifeed->validated_rec_fnc(tag[0], tag, count);
			}
		} else {
			if (rec_type == INDI_REC)
				handle_indi_lev1(ifeed, tag, val, flineno);
			else if (rec_type == FAM_REC)
				handle_fam_lev1(ifeed, tag, val, flineno);
			else
				handle_value(val, flineno);
		}
		curlev = lev;
		rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, &msg);
		xref = xref ? rmvat(xref) : NULL;
	}
	if (rec_type == INDI_REC && !named)
		handle_err(ifeed, qSnoname, defline);
	check_references(ifeed);
	if (f_logopen) {
		fclose(f_flog);
		f_logopen = FALSE;
		f_flog = 0;
	}
	return num_errors == 0;
}
/*=======================================
 * add_indi_defn -- Add person definition
 *  return index of person structure or
 *         -1 if unexplained error or
 *         -2 if explained error
 *  xref:    ref value
 *  line:    line num
 *  pel:
 *=====================================*/
static INT
add_indi_defn (struct import_feedback * ifeed, STRING xref, INT line, ELMNT *pel)
{
	ELMNT el;
	INT dex;

	*pel = NULL;
	if (!xref || *xref == 0) {
		handle_err(ifeed, _(qSmisixr), line);
		return -2;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		*pel = el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = INDI_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		Sex(el) = 0;
		Male(el) = 0;
		Fmle(el) = 0;
		dex = add_to_structures(xref, el);
	} else
		*pel = el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != INDI_REC) {
		handle_err(ifeed, qSmatper, line, xref);
		return -2;
	}
	if (Type(el) == INDI_REC) {
		if (Line(el) && line) {
			handle_err(ifeed, qSmulper, Line(el), line, xref);
			return -2;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = INDI_REC;
	Line(el) = line;
	Sex(el) = 0;
	Male(el) = 0;
	Fmle(el) = 0;
	return dex;
}
/*======================================
 * add_fam_defn -- Add family definition
 *  xref:    ref value
 *  line:    line num
 *====================================*/
static INT
add_fam_defn (struct import_feedback * ifeed, STRING xref, INT line)
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(ifeed, qSmisfxr, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = FAM_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		Male(el) = 0;
		Fmle(el) = 0;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != FAM_REC) {
		handle_err(ifeed, qSmatfam, line, xref);
		return -1;
	}
	if (Type(el) == FAM_REC) {
		if (Line(el) && line) {
			handle_err(ifeed, qSmulfam, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = FAM_REC;
	Line(el) = line;
	Male(el) = 0;
	Fmle(el) = 0;
	return dex;
}
/*=======================================
 * add_sour_defn -- Add source definition
 *  xref:    ref value
 *  line:    line num
 *=====================================*/
static INT
add_sour_defn (struct import_feedback * ifeed, STRING xref, INT line)
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(ifeed
			, _("Line %d: The source defined here has no key.")
			, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = SOUR_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != SOUR_REC) {
		handle_err(ifeed
			, _("Line %d: Source %s has an incorrect key.")
			, line, xref);
		return -1;
	}
	if (Type(el) == SOUR_REC) {
		if (Line(el) && line) {
			handle_err(ifeed
				, _("Lines %d and %d: Source %s is multiply defined.")
				, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = SOUR_REC;
	Line(el) = line;
	return dex;
}
/*======================================
 * add_even_defn -- Add event definition
 *  xref:    ref value
 *  line:    line num
 *====================================*/
static INT
add_even_defn (struct import_feedback * ifeed, STRING xref, INT line)
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(ifeed
			, _("Line %d: The event defined here has no key.")
			, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = EVEN_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != EVEN_REC) {
		handle_err(ifeed
			, _("Line %d: Event %s has an incorrect key.")
			, line, xref);
		return -1;
	}
	if (Type(el) == EVEN_REC) {
		if (Line(el) && line) {
			handle_err(ifeed
				, _("Lines %d and %d: Event %s is multiply defined.")
				, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = EVEN_REC;
	Line(el) = line;
	return dex;
}
/*==================================================
 * add_othr_defn -- Add other record type definition
 *  xref:    ref value
 *  line:    line num
 *================================================*/
static INT
add_othr_defn (struct import_feedback * ifeed, STRING xref, INT line)
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(ifeed
			, _("Line %d: The record defined here has no key.")
			, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = (ELMNT) stdalloc(sizeof(*el));
		Type(el) = OTHR_REC;
		Key(el) = xref = strsave(xref);
		Line(el) = 0;
		New(el) = NULL;
		dex = add_to_structures(xref, el);
	} else
		el = index_data[dex];
	if (KNOWNTYPE(el) && Type(el) != OTHR_REC) {
		handle_err(ifeed
			, _("Line %d: Record %s has an incorrect key.")
			, line, xref);
		return -1;
	}
	if (Type(el) == OTHR_REC) {
		if (Line(el) && line) {
			handle_err(ifeed
				, _("Lines %d and %d: Record %s is multiply defined.")
				, Line(el), line, xref);
			return -1;
		}
		if (line) Line(el) = line;
		return dex;
	}
	Type(el) = OTHR_REC;
	Line(el) = line;
	return dex;
}
/*===========================================================
 * handle_indi_lev1 -- Handle level 1 lines in person records
 *=========================================================*/
static void
handle_indi_lev1 (struct import_feedback * ifeed, STRING tag, STRING val, INT line)
{
	ELMNT indi, pers;
	ASSERT(person != -1);
	if (person == -2) {
		return;
	}
	indi = index_data[person];
	if (eqstr(tag, "FAMC")) {
		if (!pointer_value(val)) {
			handle_err(ifeed, qSmisval, line, "FAMC");
			return;
		}
		(void) add_fam_defn(ifeed, rmvat(val), 0);
	} else if (eqstr(tag, "FAMS")) {
		if (!pointer_value(val)) {
			handle_err(ifeed, qSmisval, line, "FAMS");
			return;
		}
		(void) add_fam_defn(ifeed, rmvat(val), 0);
	} else if (eqstr(tag, "FATH")) {
		if (!pointer_value(val)) {
			handle_warn(ifeed, qSmisval, line, "FATH");
			return;
		}
		Male(indi) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_MALE;
	} else if (eqstr(tag, "MOTH")) {
		if (!pointer_value(val)) {
			handle_warn(ifeed, qSmisval, line, "MOTH");
			return;
		}
		Fmle(indi) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_FEMALE;
	} else if (eqstr(tag, "SEX")) {
		if (val && (*val == 'M'))
			Sex(indi) |= IS_MALE;
		else if (val && (*val == 'F'))
			Sex(indi) |= IS_FEMALE;
	} else if (eqstr(tag, "NAME")) {
		named = TRUE;
		if (!val || *val == 0 || !valid_name(val)) {
			handle_err(ifeed, _("Line %d: Bad NAME syntax."), line);
			return;
		}
	} else
		handle_value(val, line);
}
/*==========================================================
 * handle_fam_lev1 -- Handle level 1 lines in family records
 *========================================================*/
static void
handle_fam_lev1 (struct import_feedback * ifeed, STRING tag, STRING val, INT line)
{
	ELMNT fam, pers;
#ifdef DEBUG
	llwprintf("handle_fam_lev1: %s, %s, %d\n",tag,val,line);
	llwprintf("handle_fam_lev1: family == %d\n", family);
#endif
	fam = (family != -1) ? index_data[family] : NULL;
	if (eqstr(tag, "HUSB")) {
		if (!pointer_value(val)) {
			handle_err(ifeed, qSmisval, line, "HUSB");
			return;
		}
		if (fam) Male(fam) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_MALE;
	} else if (eqstr(tag, "WIFE")) {
		if (!pointer_value(val)) {
			handle_err(ifeed, qSmisval, line, "WIFE");
			return;
		}
		if (fam) Fmle(fam) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_FEMALE;
	} else if (eqstr(tag, "CHIL")) {
		if (!pointer_value(val)) {
			handle_err(ifeed, qSmisval, line, "CHIL");
			return;
		}
		(void) add_indi_defn(ifeed, rmvat(val), 0, &pers);
	} else
		handle_value(val, line);
}
/*=================================================
 * check_akey -- Check for a standard format key
 *===============================================*/
static int
check_akey (int firstchar,
            STRING keyp,
            INT *maxp)
{
    INT val;
    if(keyp && (*keyp == firstchar)) {
	keyp++;
	if(*keyp && isdigit((uchar)*keyp)) {
	    val = atoi(keyp);
	    if(val > *maxp) *maxp = val;
	    while(*keyp && isdigit((uchar)*keyp)) keyp++;
	    if(*keyp == '\0') return TRUE;
	}
    }
    return FALSE;
}
/*=================================================
 * check_stdkeys -- Check for standard format keys
 *===============================================*/
int
check_stdkeys (void)
{
	INT i;
	int retval = TRUE;
	gd_imax = 0; gd_itot = 0;
	gd_fmax = 0; gd_ftot = 0;
	gd_emax = 0; gd_etot = 0;
	gd_smax = 0; gd_stot = 0;
	gd_xmax = 0; gd_xtot = 0;
	for (i = 0; retval && (i < struct_len); i++) {
		ELMNT el = index_data[i];
		switch (Type(el)) {
		case INDI_REC:
		    gd_itot++;
		    retval = check_akey('I', Key(el), &gd_imax);
		    break;
		case FAM_REC:
		    gd_ftot++;
		    retval = check_akey('F', Key(el), &gd_fmax);
		    break;
		case EVEN_REC:
		    gd_etot++;
		    retval = check_akey('E', Key(el), &gd_emax);
		    break;
		case SOUR_REC:
		    gd_stot++;
		    retval = check_akey('S', Key(el), &gd_smax);
		    break;
		case OTHR_REC:
		    gd_xtot++;
		    retval = check_akey('X', Key(el), &gd_xmax);
		    break;
		default: retval = FALSE; break;
		}
	}
	return retval;
}
/*================================================
 * addmissingkeys -- add keys which are not in use
 *==============================================*/
void
addmissingkeys (INT t)          /* type of record: INDI_REC ... */
{
  	INT tmax, ttot;
	INT i,j;
	INT keystoadd;
	char *kp;

	switch(t)
	{
	case INDI_REC: ttot = gd_itot; tmax = gd_imax; break;
	case FAM_REC:  ttot = gd_ftot; tmax = gd_fmax; break;
	case EVEN_REC: ttot = gd_etot; tmax = gd_emax; break;
	case SOUR_REC: ttot = gd_stot; tmax = gd_smax; break;
	case OTHR_REC: ttot = gd_xtot; tmax = gd_xmax; break;
	default: return;
	}

	if((keystoadd = (tmax - ttot)) > 0) {
		ASSERT(kp = (char *)stdalloc(tmax+1));
		for(i = 0; i < tmax; i++) kp[i] = 0;
		for (i = 0; i < struct_len; i++) {
			ELMNT el = index_data[i];
			if(Type(el) == t) {
				j = atoi(Key(el)+1);
				ASSERT((j>0) && (j <= tmax));
				kp[j] = 1;
			}
		}
		/*
		TO DO - ought to run this loop down instead of up
		because it is much more efficient for xreffile.c
		but I'm not sure if keystoadd is all of them
		Also ought to tell xreffile.c to preallocate,
		except I don't know how many of which.
		Perry, 2001/01/05
		*/
		for(i = 1; (keystoadd > 0) && (i <= tmax); i++) {
			if(kp[i] == 0) {
				switch(t)
				{
				case INDI_REC: addixref(i); break;
				case FAM_REC:  addfxref(i); break;
				case EVEN_REC: addexref(i); break;
				case SOUR_REC: addsxref(i); break;
				case OTHR_REC: addxxref(i); break;
				}
				keystoadd--;
			}
		}
		stdfree(kp);
	}
}
/*=================================================
 * check_references -- Check for undefined problems
 *===============================================*/
static void
check_references (struct import_feedback * ifeed)
{
	INT i;
	for (i = 0; i < struct_len; i++) {
		ELMNT el = index_data[i];
		switch (Type(el)) {
		case INDI_REC: check_indi_links(ifeed, el); break;
		case FAM_REC:  check_fam_links(ifeed, el);  break;
		case EVEN_REC: check_even_links(ifeed, el); break;
		case SOUR_REC: check_sour_links(ifeed, el); break;
		case OTHR_REC: check_othr_links(ifeed, el); break;
		default: handle_err(ifeed, qSundrec, Key(el));
		}
	}
}
/*=======================================
 * check_indi_links -- Check person links
 *=====================================*/
static void
check_indi_links (struct import_feedback * ifeed, ELMNT per)
{
	BOOLEAN jm, jf, bm, bf;
	if (Male(per) > 1)
		handle_warn(ifeed
		, _("Line %d: Person %s has multiple father links.")
		, Line(per), Key(per));
	if (Fmle(per) > 1)
		handle_warn(ifeed
		, _("Line %d: Person %s has multiple mother links.")
		, Line(per), Key(per));
	if (Line(per) == 0) {
		handle_err(ifeed, qSundper, Key(per));
		return;
	}
	jm = Sex(per) & IS_MALE;
	jf = Sex(per) & IS_FEMALE;
	bm = Sex(per) & BE_MALE;
	bf = Sex(per) & BE_FEMALE;
	if (jm && jf)
		handle_err(ifeed
		, _("Line %d: Person %s is both male and female.")
		, Line(per), Key(per));
	if (jm && bf)
		handle_err(ifeed
		, _("Line %d: Person %s is male but must be female.")
		, Line(per), Key(per));
	if (jf && bm)
		handle_err(ifeed
		, _("Line %d: Person %s is female but must be male.")
			, Line(per), Key(per));
	if (bm && bf)
		handle_err(ifeed
		, _("Line %d: Person %s is implied to be both male and female.")
		, Line(per), Key(per));
}
/*======================================
 * check_fam_links -- Check family links
 *====================================*/
static void
check_fam_links (struct import_feedback * ifeed, ELMNT fam)
{
	if (Line(fam) == 0) handle_err(ifeed, qSundfam, Key(fam));
	if (Male(fam) > 1)
		handle_warn(ifeed
		, _("Line %d: Family %s has multiple husband links.")
		, Line(fam), Key(fam));
	if (Fmle(fam) > 1)
		handle_warn(ifeed
		, _("Line %d: Family %s has multiple wife links.")
			, Line(fam), Key(fam));

}
/*======================================
 * check_even_links -- Check event links
 *====================================*/
static void
check_even_links (struct import_feedback * ifeed, ELMNT evn)
{
	if (Line(evn) == 0) handle_err(ifeed, qSundevn, Key(evn));
}
/*=======================================
 * check_sour_links -- Check source links
 *=====================================*/
static void
check_sour_links (struct import_feedback * ifeed, ELMNT src)
{
	if (Line(src) == 0) handle_err(ifeed, qSundsrc, Key(src));
}
/*======================================
 * check_othr_links -- Check other links
 *====================================*/
static void
check_othr_links (struct import_feedback * ifeed, ELMNT otr)
{
	if (Line(otr) == 0) handle_err(ifeed, qSundrec, Key(otr));
}
/*=======================================
 * handle_value -- Handle arbitrary value
 *=====================================*/
static void
handle_value (STRING val,
              INT line)
{
	ELMNT el;
	STRING xref;

	if (rec_type == IGNR_REC) return;
	if (!pointer_value(val)) return;
	xref = rmvat(val);
	if (xref_to_index(xref) != -1) return;
	el = (ELMNT) stdalloc(sizeof(*el));
	Type(el) = UNKN_REC;
	Key(el) = xref = strsave(xref);
	New(el) = NULL;
	Line(el) = line;
	add_to_structures(xref, el);
}
/*==================================
 * handle_err -- Handle GEDCOM error
 *================================*/
static void
handle_err (struct import_feedback * ifeed, STRING fmt, ...)
{
	char msg[100+MAXPATHLEN];

	if (openlog()) {
		va_list args;
		fprintf(f_flog, "%s: ", _("error"));
		va_start(args, fmt);
		vfprintf(f_flog, fmt, args);
		va_end(args);
		fprintf(f_flog, "\n");
	}

	++num_errors;
	llstrncpyf(msg, sizeof(msg)
		, ngettext("%6d Error", "%6d Errors", num_errors)
		, num_errors);
	if (f_logopen)
		llstrappf(msg, sizeof(msg), _(" (see log file <%s>)"), f_logpath);
	else
		llstrapp(msg, sizeof(msg), _(" (no log file)"));

	if (ifeed && ifeed->validation_error_fnc)
		(*ifeed->validation_error_fnc)(msg);
}
/*=====================================
 * openlog -- open import error log if not already open
 *===================================*/
static BOOLEAN
openlog (void)
{
	if (f_logopen)
		return TRUE;

	ASSERT(!f_flog);
	if (!f_logpath[0])
		return FALSE;

	f_flog = fopen(f_logpath, LLWRITETEXT);
	f_logopen = (f_flog != 0);
	return f_logopen;
}
/*=====================================
 * handle_warn -- Handle GEDCOM warning
 *===================================*/
static void
handle_warn (struct import_feedback * ifeed, STRING fmt, ...)
{
	char msg[100+MAXPATHLEN];
	if (openlog()) {
		va_list args;
		fprintf(f_flog, "%s: ", _("warning"));
		va_start(args, fmt);
		vfprintf(f_flog, fmt, args);
		va_end(args);
		fprintf(f_flog, "\n");
	}
	
	++num_warns;
	llstrncpyf(msg, sizeof(msg)
		, ngettext("%6d Warning", "%6d Warnings", num_warns)
		, num_warns);
	if (f_logopen)
		llstrappf(msg, sizeof(msg), _(" (see log file <%s>)"), f_logpath);
	else
		llstrapp(msg, sizeof(msg), _(" (no log file)"));
	if (ifeed && ifeed->validation_warning_fnc)
		(*ifeed->validation_warning_fnc)(msg);
}
/*=========================================
 * xref_to_index - Convert pointer to index
 *=======================================*/
INT
xref_to_index (STRING xref)
{
	BOOLEAN there;
	INT dex = valueofbool_int(convtab, xref, &there);
	return there ? dex : -1;
}
/*=========================================================
 * add_to_structures -- Add new elements to data structures
 *=======================================================*/
static INT
add_to_structures(STRING xref,
                  ELMNT el)
{
	INT i, n;

	if ((n = struct_len) >= struct_max)  {
		ELMNT *newi = (ELMNT *) stdalloc(sizeof(ELMNT)*
		    (n + 10000));
		for (i = 0; i < n; i++)
			newi[i] = index_data[i];
		if (n) stdfree(index_data);
		index_data = newi;
		struct_max += 10000;
	}
	i = struct_len;
	index_data[i] = el;
	insert_table_int(convtab, xref, struct_len++);
	return struct_len - 1;
}
/*========================================================
 * clear_structures -- Clear GEDCOM import data structures
 *======================================================*/
static void
clear_structures (void)
{
	INT i;

	for (i = 0; i < struct_len; i++)
		stdfree(index_data[i]);
	struct_len = 0;
	if (convtab) {
		remove_table(convtab, DONTFREE);
		convtab = NULL;
	}
}
/*=====================================
 * set_import_log -- Specify where import errors logged
 *===================================*/
static void
set_import_log (STRING logpath)
{
	if (!logpath)
		logpath = "";
	llstrncpy(f_logpath, logpath, sizeof(f_logpath));
}

