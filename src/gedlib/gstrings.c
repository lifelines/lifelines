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
/*=============================================================
 * gstrings.c -- Routines to creates child strings
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   3.0.0 - 02 May 94    3.0.2 - 24 Nov 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "lloptions.h"

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSdspa_mar,qSdspa_bir,qSdspa_chr,qSdspa_dea,qSdspa_bur;
extern STRING qSunksps;

/*********************************************
 * local variables
 *********************************************/

static INT nchil = 0, maxchil = 0;
static STRING *chstrings = NULL, *chkeys = NULL;

static BOOLEAN displaykeys=TRUE;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===================================================================
 * get_child_strings -- Return children strings; each string has name
 *   and event info, if avail  
 *  fam:   [in] family of interest
 *  rfmt:  [in] reformatting functions
 *  pnum:  [out] number of output strings
 *  pkeys: [out] array of output strings (children descriptions)
 *=================================================================*/
STRING *
get_child_strings (NODE fam, RFMT rfmt, INT *pnum, STRING **pkeys)
{
	NODE chil;
	INT i;

	for (i = 0; i < nchil; i++) {
		stdfree(chstrings[i]);
		stdfree(chkeys[i]);
	}
	nchil = *pnum = 0;
	if (!fam || !(chil = CHIL(fam))) return NULL;
	nchil = length_nodes(chil);
	if (nchil == 0) return NULL;
	if (nchil > (maxchil - 1)) {
		if (maxchil) {
			stdfree(chstrings); 
			stdfree(chkeys); 
		}
		chstrings = (STRING *) stdalloc((nchil+5)*sizeof(STRING));
		chkeys = (STRING *) stdalloc((nchil+5)*sizeof(STRING));
		maxchil = nchil + 5;
	}
	FORCHILDRENx(fam,child,i)
		chstrings[i-1] = indi_to_list_string(child, NULL, 66, rfmt, TRUE);
		chkeys[i-1] = strsave(rmvat(nxref(child)));
	ENDCHILDRENx
	*pnum = nchil;
	*pkeys = chkeys;
	return chstrings;
}
/*================================================
 * indi_to_list_string -- Return menu list string.
 *  returns heap-alloc'd string
 *  indi:   [IN]  source person
 *  fam:    [IN]  relevant family (used in spouse lists)
 *  len:    [IN]  max length desired
 *  rfmt:   [IN]  reformating functions (may be NULL)
 *  appkey: [IN]  allow appending key ?
 *==============================================*/
STRING
indi_to_list_string (NODE indi, NODE fam, INT len, RFMT rfmt, BOOLEAN appkey)
{
	char scratch[MAXLINELEN];
	STRING name, evt = NULL, p = scratch;
	int hasparents;
	int hasfamily;
	if (len>(INT)sizeof(scratch))
		len = sizeof(scratch);
	if (indi) {
		ASSERT(name = indi_to_name(indi, len));
	} else
		name = _(qSunksps);
	sprintf(p, "%s", name);
	/* TODO: Shouldn't we len -= strlen(p) first ? Perry, 2007-09-29 */
	p += strlen(p);
	if (fam)  evt = fam_to_event(fam, "MARR", _(qSdspa_mar), len, rfmt);
	if (!evt) evt = indi_to_event(indi, "BIRT", _(qSdspa_bir), len, rfmt);
	if (!evt) evt = indi_to_event(indi, "CHR", _(qSdspa_chr), len, rfmt);
	if (!evt) evt = indi_to_event(indi, "DEAT", _(qSdspa_dea), len, rfmt);
	if (!evt) evt = indi_to_event(indi, "BURI", _(qSdspa_bur), len, rfmt);
	if (evt) {
		sprintf(p, ", %s", evt);
		p += strlen(p);
	}
	if (appkey && indi && displaykeys) {
		if (getlloptint("DisplayKeyTags", 0) > 0) {
			sprintf(p, " (i%s)", key_of_record(indi));
		} else {
			sprintf(p, " (%s)", key_of_record(indi));
		}
		p += strlen(p);
	}
	if (appkey && fam && displaykeys) {
		if (getlloptint("DisplayKeyTags", 0) > 0) {
			sprintf(p, " (f%s)", key_of_record(fam));
		} else {
			sprintf(p, " (%s)", key_of_record(fam));
		}
		p += strlen(p);
	}
	if(indi) {
	    if(FAMC(indi)) hasparents = 1;
	    else hasparents = 0;
	    if(FAMS(indi)) hasfamily = 1;
	    else hasfamily = 0;
	    if(hasfamily || hasparents) {
		*p++ = ' ';
		*p++ = '[';
		if(hasparents) *p++ = 'P';
		if(hasfamily) *p++ = 'S';
		*p++ = ']';
		*p = '\0';
	    }
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * sour_to_list_string -- Return menu list string.
 * Created: 2000/11/29, Perry Rapp
 *==============================================*/
STRING
sour_to_list_string (NODE sour, INT len, STRING delim)
{
	char scratch[1024];
	STRING name, p=scratch;
	INT mylen=len;
	if (mylen>(INT)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	llstrcatn(&p, "(S", &mylen);
	llstrcatn(&p, rmvat(nxref(sour))+1, &mylen);
	llstrcatn(&p, ") ", &mylen);
	name = node_to_tag(sour, "REFN", len);
	if (name)
		llstrcatn(&p, name, &mylen);
	name = node_to_tag(sour, "TITL", len);
	if (name && mylen > 20)
	{
		llstrcatn(&p, delim, &mylen);
		llstrcatn(&p, name, &mylen);
	}
	name = node_to_tag(sour, "AUTH", len);
	if (name && mylen > 20)
	{
		llstrcatn(&p, delim, &mylen);
		llstrcatn(&p, name, &mylen);
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * even_to_list_string -- Return menu list string.
 * Created: 2001/12/16, Perry Rapp
 *==============================================*/
STRING
even_to_list_string (NODE even, INT len, STRING delim)
{
	char scratch[1024];
	STRING name, p=scratch;
	INT mylen=len;
	delim=delim; /* unused */
	if (mylen>(INT)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	llstrcatn(&p, "(E", &mylen);
	llstrcatn(&p, rmvat(nxref(even))+1, &mylen);
	llstrcatn(&p, ") ", &mylen);
	name = node_to_tag(even, "NAME", len);
	if (name)
		llstrcatn(&p, name, &mylen);
        name = node_to_tag(even, "REFN", len);
        if (name) {
		llstrcatn(&p, " (", &mylen);
                llstrcatn(&p, name, &mylen);
		llstrcatn(&p, ")", &mylen);
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * fam_to_list_string -- Return menu list string.
 * Created: 2001/02/17, Perry Rapp
 *==============================================*/
STRING
fam_to_list_string (NODE fam, INT len, STRING delim)
{
	char scratch[1024];
	STRING name, p=scratch;
	INT mylen=len;
	char counts[32];
	INT husbands=0, wives=0, children=0;
	INT templen=0;
	NODE refn, husb, wife, chil, rest, node;
	if (mylen>(INT)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	llstrcatn(&p, "(F", &mylen);
	llstrcatn(&p, rmvat(nxref(fam))+1, &mylen);
	llstrcatn(&p, ")", &mylen);
	name = node_to_tag(fam, "REFN", len);
	if (name) {
		llstrcatn(&p, " ", &mylen);
		llstrcatn(&p, name, &mylen);
	}
	split_fam(fam, &refn, &husb, &wife, &chil, &rest);
	for (node=husb; node; node=nsibling(node))
		husbands++;
	for (node=wife; node; node=nsibling(node))
		wives++;
	for (node=chil; node; node=nsibling(node))
		children++;
	sprintf(counts, "%ldh,%ldw,%ldch", husbands, wives, children);
	llstrcatn(&p, " ", &mylen);
	llstrcatn(&p, counts, &mylen);
	if (husbands) {
		node = qkey_to_indi(rmvat(nval(husb)));
		if (node) {
			llstrcatn(&p, delim, &mylen);
			if (wives)
				templen = (mylen-4)/2;
			else
				templen = mylen;
			llstrcatn(&p, indi_to_name(node, templen), &mylen);
			if (wives)
				llstrcatn(&p, " m. ", &mylen);
		}
	}
	if (wives) {
		node = qkey_to_indi(rmvat(nval(wife)));
		if (node) {
			if (!templen)
				templen = mylen;
			/* othewise we set templen above */
			llstrcatn(&p, indi_to_name(node, templen), &mylen);
		}
	}
	join_fam(fam, refn, husb, wife, chil, rest);
	/* TO DO - print a husband and a wife out */
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * other_to_list_string -- Return menu list string.
 * Created: 2000/11/29, Perry Rapp
 *==============================================*/
STRING
other_to_list_string(NODE node, INT len, STRING delim)
{
	char scratch[1024];
	STRING name, p=scratch;
	INT mylen=len;
	NODE child;
	delim=delim; /* unused */
	if (mylen>(INT)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	llstrcatn(&p, "(X", &mylen);
	llstrcatn(&p, rmvat(nxref(node))+1, &mylen);
	llstrcatn(&p, ") (", &mylen);
	llstrcatn(&p, ntag(node), &mylen);
	llstrcatn(&p, ") ", &mylen);
	name = node_to_tag(node, "REFN", mylen);
	if (name)
		llstrcatn(&p, name, &mylen);
	if (nval(node)) {
		llstrcatn(&p, nval(node), &mylen);
	}
	/* append any CONC/CONT nodes that fit */
	child = nchild(node);
	while (mylen>5 && child) {
		if (!strcmp(ntag(child), "CONC")
			|| !strcmp(ntag(child), "CONT")) {
			/* skip empty CONC/CONT nodes */
			if (nval(child)) {
				llstrcatn(&p, " ", &mylen);
				llstrcatn(&p, nval(child), &mylen);
			}
		} else {
			break;
		}
		if (nchild(child))
			break;
		else if (nsibling(child))
			child = nsibling(child);
		else
			break;
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*===========================================
 * generic_to_list_string -- Format a print line from
 *  a top-level node of any type
 * Caller may specify either node or key (& leave other NULL)
 *  returns heap-alloc'd string
 * Caller must specify either node or key (or both)
 * Used in lists and in extended gedcom view
 * Created: 2001/02/12, Perry Rapp
 *  node:   [IN]  node tree of indi or fam ... to be described
 *  key:    [IN]  key of record specified by node
 *  len:    [IN]  max description desired
 *  delim:  [IN]  separator to use between events
 *  rfmt:   [IN]  reformatting information
 *  appkey: [IN]  allow appending key ?
 *=========================================*/
STRING
generic_to_list_string (NODE node, STRING key, INT len, STRING delim, RFMT rfmt, BOOLEAN appkey)
{
	STRING str;
	str=NULL; /* set to appropriate format */
	if (!node && key)
		node = qkey_to_type(key);
	if (!key && node)
		key = rmvat(nxref(node));
	if (node) {
		switch (key[0])
		{
		case 'I':
			str = indi_to_list_string(node, NULL, len, rfmt, appkey);
			break;
		case 'S':
			str = sour_to_list_string(node, len, delim);
			break;
		case 'F':
			str = fam_to_list_string(node, len, delim);
			break;
		case 'E':
			str = even_to_list_string(node, len, delim);
			break;
		case 'X':
			str = other_to_list_string(node, len, delim);
			break;
		}
	}
	if (!str) {
		if (key)
			str = strsave(key);
		else
			str = strsave("??");
	}
	return str;
}
/*=======================================================
 * set_displaykeys -- Enable/disable keys in list strings
 *  That is, whether or not to show key numbers in items
 * Created: 2001/01/01, Perry Rapp
 *=====================================================*/
void
set_displaykeys (BOOLEAN keyflag)
{
	displaykeys = keyflag;
}
