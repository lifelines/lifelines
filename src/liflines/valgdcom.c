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
#include "llstdlib.h"
/* llstdlib.h pulls in standard.h, config.h, sys_inc.h */
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcheck.h"
#include "liflines.h"
#include "llinesi.h"
#include "impfeed.h"
#include "lloptions.h"
#include "zstr.h"
#include "codesets.h"
#include "btree.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

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
static BOOLEAN named = FALSE; /* found a NAME in current INDI ? */
static INT members = 0; /* number members (HUSB,WIFE,CHIL) in current FAM */
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

static STRING qSundrec      = N_("Record %s is referred to but not defined.");
static STRING qSundrecl     = N_("Line %d: Reference to undefined record %s");
static STRING qSlinlev1     = N_("Line %d: Tag %s found in unexpected record: %s %s.");

ELMNT *index_data = NULL;


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static INT add_even_defn(IMPORT_FEEDBACK ifeed, STRING, INT);
static INT add_fam_defn(IMPORT_FEEDBACK ifeed, STRING, INT);
static INT add_indi_defn(IMPORT_FEEDBACK ifeed, STRING, INT, ELMNT*);
static INT add_othr_defn(IMPORT_FEEDBACK ifeed, STRING, INT);
static INT add_sour_defn(IMPORT_FEEDBACK ifeed, STRING, INT);
static INT add_to_structures(STRING, ELMNT);
static void append_path(ZSTR zstr, char delim, CNSTRING str);
static int check_akey (int firstchar, STRING keyp, INT *maxp);
static void check_even_links(IMPORT_FEEDBACK ifeed, ELMNT);
static void check_fam_links(IMPORT_FEEDBACK ifeed, ELMNT);
static void check_indi_links(IMPORT_FEEDBACK ifeed, ELMNT per);
static void check_level1_tag(IMPORT_FEEDBACK ifeed, CNSTRING tag, CNSTRING val, INT line, CNSTRING tag0, CNSTRING xref0);
static void check_othr_links(IMPORT_FEEDBACK ifeed, ELMNT);
static void check_references(IMPORT_FEEDBACK ifeed);
static void check_sour_links(IMPORT_FEEDBACK ifeed, ELMNT src);
static void clear_structures(void);
static ELMNT create_elmnt(CHAR eltype, CNSTRING xref);
static void free_elmnt(ELMNT el);
static void handle_fam_lev1(IMPORT_FEEDBACK ifeed, STRING tag, STRING val, INT line, CNSTRING tag0, CNSTRING xref0);
static void handle_indi_lev1(IMPORT_FEEDBACK ifeed, STRING tag, STRING val, INT line, CNSTRING tag0, CNSTRING xref0);
static void handle_head_lev1(IMPORT_FEEDBACK ifeed, STRING, STRING, INT);
static void handle_trlr_lev1(IMPORT_FEEDBACK ifeed, STRING, STRING, INT);
static void handle_value(STRING, INT);
static BOOLEAN openlog(void);
static void handle_warn(IMPORT_FEEDBACK ifeed, STRING, ...);
static void handle_err(IMPORT_FEEDBACK ifeed, STRING, ...);
static void set_import_log(STRING logpath);
static void report_missing_value(IMPORT_FEEDBACK ifeed, STRING tag, INT line, CNSTRING tag0, CNSTRING xref0);

/*===================================================
 * validate_gedcom -- Validate GEDCOM records in file
 *=================================================*/
BOOLEAN
validate_gedcom (IMPORT_FEEDBACK ifeed, FILE *fp)
{
	INT lev, rc, curlev = 0;
	INT nhead, ntrlr, nindi, nfam, nsour, neven, nothr;
	ELMNT el;
	XLAT xlat = transl_get_predefined_xlat(MGDIN);
	STRING xref, tag, val, msg;
	STRING tag0=0;
	STRING xref0=0;

	nhead = ntrlr = nindi = nfam = nsour = neven = nothr = 0;
	num_errors = num_warns = 0;
	f_logopen = FALSE;
	f_flog = 0;
	set_import_log(getlloptstr("ImportLog", "errs.log"));
	defline = 0;
	curlev = 0;
	clear_structures();
	convtab = create_table_int();


	rc = file_to_line(fp, xlat, &lev, &xref, &tag, &val, &msg);
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
			rc = file_to_line(fp, xlat, &lev, &xref, &tag, &val,
			    &msg);
			xref = xref ? rmvat(xref) : NULL;
			continue;
		}
		if (lev > 1) {
			handle_value(val, flineno);
			curlev = lev;
			rc = file_to_line(fp, xlat, &lev, &xref, &tag, &val,
			    &msg);
			xref = xref ? rmvat(xref) : NULL;
			continue;
		}
		if (lev == 0) {
			if (rec_type == INDI_REC) {
				if (!named && getlloptint("RequireNames", 0)) {
					handle_err(ifeed, qSnoname, defline);
				}
			}
			if (rec_type == FAM_REC) {
				if (!members) {
					handle_warn(ifeed, _("Line %d: Family has no members (%s %s).")
						, defline, tag0, xref0);
				}
			}
			defline = flineno;
			strupdate(&tag0, tag); /* store current level 0 tag */
			strupdate(&xref0, xref); /* store current level 0 xref (eg I15) */
			if (eqstr("HEAD", tag))  {
				rec_type = (nhead==0 ? HEAD_REC : IGNR_REC);
			} else if (eqstr("TRLR", tag)) {
				rec_type = (ntrlr==0 ? TRLR_REC : IGNR_REC);
			} else {
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
					members = 0;
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
				if (ifeed && ifeed->validated_rec_fnc) {
					char ctype = (rec_type==OTHR_REC) ? 'X': tag[0];
					ifeed->validated_rec_fnc(ctype, tag, count);
				}
			}
		} else {
			/* specific handling for specific record types */
			if (rec_type == HEAD_REC)
				handle_head_lev1(ifeed, tag, val, flineno);
			else if (rec_type == TRLR_REC)
				handle_trlr_lev1(ifeed, tag, val, flineno);
			else if (rec_type == INDI_REC)
				handle_indi_lev1(ifeed, tag, val, flineno, tag0, xref0);
			else if (rec_type == FAM_REC)
				handle_fam_lev1(ifeed, tag, val, flineno, tag0, xref0);
			else
				handle_value(val, flineno);
			/* specific handling for specific tag types */
			check_level1_tag(ifeed, tag, val, flineno, tag0, xref0);
		}
		curlev = lev;
		rc = file_to_line(fp, xlat, &lev, &xref, &tag, &val, &msg);
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
	strfree(&tag0);
	return num_errors == 0;
}
/*=======================================
 * create_elmnt -- Return newly alloc'd ELMNT
 *=====================================*/
static ELMNT
create_elmnt (CHAR eltype, CNSTRING xref)
{
	ELMNT el = (ELMNT) stdalloc(sizeof(*el));
	memset(el, 0, sizeof(*el));
	Type(el) = eltype;
	if (xref)
		Key(el) = strsave(xref);
	return el;
}
/*=======================================
 * free_elmnt -- Free memory of element
 *=====================================*/
static void
free_elmnt (ELMNT el)
{
	strfree(&el->key);
	strfree(&el->newstr);
	stdfree(el);
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
add_indi_defn (IMPORT_FEEDBACK ifeed, STRING xref, INT line, ELMNT *pel)
{
	ELMNT el;
	INT dex;

	*pel = NULL;
	if (!xref || *xref == 0) {
		handle_err(ifeed, _(qSmisixr), line);
		return -2;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		*pel = el = create_elmnt(INDI_REC, xref);
		xref = Key(el); /* dup'd copy of xref */
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
add_fam_defn (IMPORT_FEEDBACK ifeed, STRING xref, INT line)
{
	ELMNT el;
	INT dex;
	if (!xref || *xref == 0) {
		handle_err(ifeed, qSmisfxr, line);
		return -1;
	}
	if ((dex = xref_to_index(xref)) == -1) {
		el = create_elmnt(FAM_REC, xref);
		Line(el) = 0;
		New(el) = NULL;
		Male(el) = 0;
		Fmle(el) = 0;
		dex = add_to_structures(xref, el);
	} else {
		el = index_data[dex];
	}
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
add_sour_defn (IMPORT_FEEDBACK ifeed, STRING xref, INT line)
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
		el = create_elmnt(SOUR_REC, xref);
		xref = Key(el); /* dup'd copy of xref */
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
add_even_defn (IMPORT_FEEDBACK ifeed, STRING xref, INT line)
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
		el = create_elmnt(EVEN_REC, xref);
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
add_othr_defn (IMPORT_FEEDBACK ifeed, STRING xref, INT line)
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
		el = create_elmnt(OTHR_REC, xref);
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
 * handle_head_lev1 -- Handle level 1 lines in HEAD record
 * Created: 2002-12-15 (Perry Rapp)
 *=========================================================*/
static void
handle_head_lev1 (IMPORT_FEEDBACK ifeed, STRING tag, STRING val, INT line)
{
	ifeed=ifeed; /* unused */
	line=line; /* unused */
	if (eqstr(tag, "CHAR")) {
		strupdate(&gedcom_codeset_in, (val ? val : ""));
	}
}
/*===========================================================
 * handle_trlr_lev1 -- Handle level 1 lines in TRLR record
 * Created: 2002-12-15 (Perry Rapp)
 *=========================================================*/
static void
handle_trlr_lev1 (IMPORT_FEEDBACK ifeed, STRING tag, STRING val, INT line)
{
	ifeed=ifeed; /* unused */
	tag=tag; /* unused */
	val=val; /* unused */
	line=line; /* unused */
}
/*===========================================================
 * report_missing_value -- Report line with incorrectly empty value
 *=========================================================*/
static void
report_missing_value (IMPORT_FEEDBACK ifeed, STRING tag, INT line, CNSTRING tag0, CNSTRING xref0)
{
	handle_err(ifeed, _("Line %d: This %s line is missing a value field (%s %s).")
		, line, tag, tag0, xref0);
}
/*===========================================================
 * handle_indi_lev1 -- Handle level 1 lines in person records
 *=========================================================*/
static void
handle_indi_lev1 (IMPORT_FEEDBACK ifeed, STRING tag, STRING val, INT line, CNSTRING tag0, CNSTRING xref0)
{
	ELMNT indi, pers;
	ASSERT(person != -1);
	if (person == -2) {
		return;
	}
	indi = index_data[person];
	if (eqstr(tag, "FAMC")) {
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
			return;
		}
		(void) add_fam_defn(ifeed, rmvat(val), 0);
	} else if (eqstr(tag, "FAMS")) {
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
			return;
		}
		(void) add_fam_defn(ifeed, rmvat(val), 0);
	} else if (eqstr(tag, "FATH")) {
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
			return;
		}
		Male(indi) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_MALE;
	} else if (eqstr(tag, "MOTH")) {
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
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
			handle_err(ifeed, _("Line %d: Bad NAME syntax (%s %s)."),
				line, tag0, xref0);
			return;
		}
	} else
		handle_value(val, line);
}
/*==========================================================
 * handle_fam_lev1 -- Handle level 1 lines in family records
 *========================================================*/
static void
handle_fam_lev1 (IMPORT_FEEDBACK ifeed, STRING tag, STRING val, INT line, CNSTRING tag0, CNSTRING xref0)
{
	ELMNT fam, pers;
	fam = (family != -1) ? index_data[family] : NULL;
	if (eqstr(tag, "HUSB")) {
		++members;
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
			return;
		}
		if (fam) Male(fam) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_MALE;
	} else if (eqstr(tag, "WIFE")) {
		++members;
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
			return;
		}
		if (fam) Fmle(fam) += 1;
		if (add_indi_defn(ifeed, rmvat(val), 0, &pers) >= 0)
			Sex(pers) |= BE_FEMALE;
	} else if (eqstr(tag, "CHIL")) {
		++members;
		if (!pointer_value(val)) {
			report_missing_value(ifeed, tag, line, tag0, xref0);
			return;
		}
		(void) add_indi_defn(ifeed, rmvat(val), 0, &pers);
	} else
		handle_value(val, line);
}
/*==========================================================
 * check_level1_tag -- Warnings for specific tags at level 1
 *========================================================*/
static void
check_level1_tag (IMPORT_FEEDBACK ifeed, CNSTRING tag, CNSTRING val, INT line, CNSTRING tag0, CNSTRING xref0)
{
	/*
	lifelines expects lineage-linking records (FAMS, FAMC, HUSB, & WIFE)
	to be correct, so warn if any of them occur in unusual locations
	*/
	val = val;    /* unused */
	if (eqstr(tag, "FAMS")) {
		if (!eqstr(tag0, "INDI"))
			handle_warn(ifeed, qSlinlev1, line, tag, tag0, xref0);
		return;
	}
	if (eqstr(tag, "FAMC")) {
		if (!eqstr(tag0, "INDI"))
			handle_warn(ifeed, qSlinlev1, line, tag, tag0, xref0);
		return;
	}
	if (eqstr(tag, "HUSB")) {
		if (!eqstr(tag0, "FAM"))
			handle_warn(ifeed, qSlinlev1, line, tag, tag0, xref0);
		return;
	}
	if (eqstr(tag, "WIFE")) {
		if (!eqstr(tag0, "FAM"))
			handle_warn(ifeed, qSlinlev1, line, tag, tag0, xref0);
		return;
	}
}
/*=================================================
 * check_akey -- Check for a standard format key
 *               lifelines assumes key's are of form 
 *               a letter followed by a number with no leading zeros 
 *               e.g. I248
 *===============================================*/
static int
check_akey (int firstchar,
            STRING keyp,
            INT *maxp)
{
    INT val;
    if(keyp && (*keyp == firstchar)) {
	keyp++;
	if(*keyp && (*keyp != '0') && isdigit(*keyp)) {
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
		STRING skey = Key(el);
		/* check that key is not too long */
		if (strlen(skey)>RKEYLEN) {
			retval = FALSE;
			break;
		}
		switch (Type(el)) {
		case INDI_REC:
		    gd_itot++;
		    retval = check_akey('I', skey, &gd_imax);
		    break;
		case FAM_REC:
		    gd_ftot++;
		    retval = check_akey('F', skey, &gd_fmax);
		    break;
		case EVEN_REC:
		    gd_etot++;
		    retval = check_akey('E', skey, &gd_emax);
		    break;
		case SOUR_REC:
		    gd_stot++;
		    retval = check_akey('S', skey, &gd_smax);
		    break;
		case OTHR_REC:
		    gd_xtot++;
		    retval = check_akey('X', skey, &gd_xmax);
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
				ASSERT(j>0);
				ASSERT(j <= tmax);
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
check_references (IMPORT_FEEDBACK ifeed)
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
		default: handle_err(ifeed, qSundrecl, Line(el), Key(el));
		}
	}
}
/*=======================================
 * check_indi_links -- Check person links
 *=====================================*/
static void
check_indi_links (IMPORT_FEEDBACK ifeed, ELMNT per)
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
check_fam_links (IMPORT_FEEDBACK ifeed, ELMNT fam)
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
check_even_links (IMPORT_FEEDBACK ifeed, ELMNT evn)
{
	if (Line(evn) == 0) handle_err(ifeed, qSundevn, Key(evn));
}
/*=======================================
 * check_sour_links -- Check source links
 *=====================================*/
static void
check_sour_links (IMPORT_FEEDBACK ifeed, ELMNT src)
{
	if (Line(src) == 0) handle_err(ifeed, qSundsrc, Key(src));
}
/*======================================
 * check_othr_links -- Check other links
 *====================================*/
static void
check_othr_links (IMPORT_FEEDBACK ifeed, ELMNT otr)
{
	if (Line(otr) == 0) handle_err(ifeed, qSundrec, Key(otr));
}
/*=======================================
 * handle_value -- Handle arbitrary value
 *=====================================*/
static void
handle_value (STRING val, INT line)
{
	ELMNT el;
	STRING xref;

	if (rec_type == IGNR_REC) return;
	if (!pointer_value(val)) return;
	xref = rmvat(val);
	if (xref_to_index(xref) != -1) return;
	el = create_elmnt(UNKN_REC, xref);
	New(el) = NULL;
	Line(el) = line;
	add_to_structures(xref, el);
}
/*==================================
 * handle_err -- Handle GEDCOM error
 *================================*/
static void
handle_err (IMPORT_FEEDBACK ifeed, STRING fmt, ...)
{
	ZSTR zstr=zs_new();

	if (openlog()) {
		va_list args;
		fprintf(f_flog, "%s: ", _("error"));
		va_start(args, fmt);
		vfprintf(f_flog, fmt, args);
		va_end(args);
		fprintf(f_flog, "\n");
	}

	++num_errors;
	zs_setf(zstr, _pl("%6d Error", "%6d Errors", num_errors), num_errors);
	if (f_logopen)
		zs_appf(zstr, _(" (see log file <%s>)"), f_logpath);
	else
		zs_apps(zstr, _(" (no log file)"));

	if (ifeed && ifeed->validation_error_fnc)
		(*ifeed->validation_error_fnc)(zs_str(zstr));
	zs_free(&zstr);
}
/*=====================================
 * handle_warn -- Handle GEDCOM warning
 *===================================*/
static void
handle_warn (IMPORT_FEEDBACK ifeed, STRING fmt, ...)
{
	ZSTR zstr=zs_new();

	if (openlog()) {
		va_list args;
		fprintf(f_flog, "%s: ", _("warning"));
		va_start(args, fmt);
		vfprintf(f_flog, fmt, args);
		va_end(args);
		fprintf(f_flog, "\n");
	}
	
	++num_warns;
	zs_setf(zstr, _pl("%6d Warning", "%6d Warnings", num_warns), num_warns);
	if (f_logopen)
		zs_appf(zstr, _(" (see log file <%s>)"), f_logpath);
	else
		zs_appf(zstr, _(" (no log file)"));
	if (ifeed && ifeed->validation_warning_fnc)
		(*ifeed->validation_warning_fnc)(zs_str(zstr));
	zs_free(&zstr);
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
add_to_structures (STRING xref, ELMNT el)
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

	if (convtab) {
		/* elements are destroyed below, because
		index_data points to them */
		destroy_table(convtab);
		convtab = NULL;
	}
	for (i = 0; i < struct_len; i++) {
		ELMNT el = index_data[i];
		index_data[i] = 0;
		free_elmnt(el);
	}
	struct_len = 0;
}
/*=====================================
 * set_import_log -- Specify where import errors logged
 *===================================*/
static void
set_import_log (STRING logpath)
{
	if (!logpath)
		logpath = "";
	llstrncpy(f_logpath, logpath, sizeof(f_logpath), uu8);
}
/*============================================================
 * scan_header -- Scan header of GEDCOM record
 *  collecting metadata as found (metadatatab is a FREEBOTH tab)
 * Created: 2003-02-03 (Perry Rapp)
 *==========================================================*/
BOOLEAN
scan_header (FILE * fp, TABLE metadatatab, ZSTR * zerr)
{
	STRING parents[2] = { 0, 0 };
	INT linno, head=0, lev=-1, curlev,i;
	INT lastoff=0;
	ZSTR zpath = zs_new();
	*zerr = 0;
	for (linno=1; 1; ++linno) {
		XLAT xlat=0;
		/* no codeset translation yet, b/c we've not found the file's
		encoding declaration yet */
		INT rc;
		STRING xref, tag, val, msg;
		lastoff = ftell(fp);
		curlev = lev;
		if (linno==500) {
			*zerr = zs_newf(_("Processed %d lines without finding end of HEAD"), linno);
			break;
		}
		rc = file_to_line(fp, xlat, &lev, &xref, &tag, &val, &msg);
		if (rc==DONE) {
			*zerr = zs_newf(_("End of file at line %d"), linno);
			break;
		}
		if (rc==ERROR) {
			*zerr = zs_newf(_("Error at line %d: %s"), linno, msg);
			break;
		}
		if (lev < 0 || lev > curlev+1) {
			*zerr = zs_newf(_("Bad level at line %d"), linno);
			break;
		}
		if (lev==0) {
			if (eqstr(tag, "HEAD")) {
				if (head) {
					*zerr = zs_newf(_("Duplicate HEAD line at line %d"), linno);
					break;
				} else {
					head=1;
				}
			} else if (!head) {
				*zerr = zs_newf(_("Missing HEAD line at line %d"), linno);
				break;
			} else {
				fseek(fp, lastoff, SEEK_SET);
				flineno--;
				/* finished head */
				break;
			}
		}
		if (lev < 3 && tag && tag[0] && val && val[0]) { /* we don't care about anything beyond level 2 */
			zs_clear(zpath);
			for (i=1; i<lev && i<ARRSIZE(parents); ++i)
				append_path(zpath, '.', parents[i-1]);
			append_path(zpath, '.', tag);
			insert_table_str(metadatatab, zs_str(zpath), val);
		}
		if (lev>0 && lev-1<ARRSIZE(parents))
			strupdate(&parents[lev-1], tag);
		/* clear any obsolete parents */
		for (i=lev; i<ARRSIZE(parents); ++i) {
			strupdate(&parents[i], 0);
		}
	}

	for (i=0; i<ARRSIZE(parents); ++i) {
		strupdate(&parents[i], 0);
	}
	zs_free(&zpath);
	return !(*zerr);
}
/*============================================================
 * append_path -- Add path to end of string, prefixing with delimiter
 *  if string non-empty
 * Created: 2003-02-03 (Perry Rapp)
 *==========================================================*/
static void
append_path (ZSTR zstr, char delim, CNSTRING str)
{
	if (zs_len(zstr))
		zs_appc(zstr, delim);
	zs_apps(zstr, str);
}
/*===================================================
 * validate_get_warning_count -- How many warnings were found ?
 *=================================================*/
INT
validate_get_warning_count (void)
{
	return num_warns;
}
/*===================================================
 * validate_end_import -- Free any memory left after import finished
 *=================================================*/
void
validate_end_import (void)
{
	clear_structures();
}
