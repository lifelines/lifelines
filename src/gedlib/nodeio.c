/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * nodeio.c -- I/O between nodes & files or strings
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "feedback.h"
#include "warehouse.h"
#include "metadata.h"
#include "lloptions.h"
#include "date.h"

/*********************************************
 * global/exported variables
 *********************************************/

INT flineno = 0;

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING qSfileof, qSreremp, qSrerlng, qSrernlv, qSrerinc;
extern STRING qSrerbln, qSrernwt, qSrerilv, qSrerwlv, qSunsupunix, qSunsupuniv;

/*********************************************
 * local function prototypes, alphabetical
 *********************************************/

static BOOLEAN buffer_to_line(STRING p, INT *plev, STRING *pxref
	, STRING *ptag, STRING *pval, STRING *pmsg);
static RECORD convert_first_fp_to_record(FILE *fp, BOOLEAN list, XLAT ttm
	, STRING *pmsg, BOOLEAN *peof);
static NODE do_first_fp_to_node(FILE *fp, BOOLEAN list, XLAT tt
	, STRING *pmsg, BOOLEAN *peof);
static RECORD do_first_fp_to_record(FILE *fp, BOOLEAN list, XLAT tt
	, STRING *pmsg, BOOLEAN *peof);
static void load_record_wh(RECORD rec, char * whptr, INT whlen);
static BOOLEAN string_to_line(STRING *ps, INT *plev, STRING *pxref, 
	STRING *ptag, STRING *pval, STRING *pmsg);
static STRING swrite_node(INT levl, NODE node, STRING p);
static STRING swrite_nodes(INT levl, NODE node, STRING p);
static void write_node(INT levl, FILE *fp, XLAT ttm,
	NODE node, BOOLEAN indent);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==========================================
 * file_to_line -- Get GEDCOM line from file
 *========================================*/
INT
file_to_line (FILE *fp,
              XLAT ttm,
              INT *plev,
              STRING *pxref,
              STRING *ptag,
              STRING *pval,
              STRING *pmsg)
{
	static char in[MAXLINELEN+2];
	static char out[MAXLINELEN+2];
	char *p = in;
	*pmsg = NULL;
	while (TRUE) {
		if (!(p = fgets(in, sizeof(in), fp))) return DONE;
		flineno++;
		if (ttm) {
			translate_string(ttm, in, out, MAXLINELEN+2);
			p = out;
		}
		if (!allwhite(p)) break;
	}
	return buffer_to_line(p, plev, pxref, ptag, pval, pmsg);
}
/*==============================================
 * string_to_line -- Get GEDCOM line from string
 *
 * STRING *ps:    [in,out] string ptr - advanced to next
 * INT *plev:     [out] level ptr
 * STRING *pxref: [out] cross-ref ptr
 * STRING *ptag:  [out] tag ptr
 * STRING *pval:  [out] value ptr
 * STRING *pmsg:  [out] error msg ptr
 *============================================*/
static BOOLEAN
string_to_line (STRING *ps, INT *plev, STRING *pxref, STRING *ptag
	, STRING *pval, STRING *pmsg)
{
	STRING s0, s;
	*pmsg = NULL;
	s0 = s = *ps;
	if (!s || *s == 0) return FALSE;
	while (*s && *s != '\n') s++;
	if (*s == 0)
		*ps = s;
	else {
		*s = 0;
		*ps = s + 1;
	}
	return buffer_to_line(s0, plev, pxref, ptag, pval, pmsg);
}
/*================================================================
 * buffer_to_line -- Get GEDCOM line from buffer with <= 1 newline
 *
 *  p:      [in]  buffer
 *  plev:   [out] level number
 *  pxref:  [out] xref
 *  ptag:   [out] tag
 *  pval:   [out] value
 *  pmsg:   [out] error msg (in static buffer)
 *==============================================================*/
static BOOLEAN
buffer_to_line (STRING p, INT *plev, STRING *pxref
	, STRING *ptag, STRING *pval, STRING *pmsg)
{
	INT lev;
	static char scratch[MAXLINELEN+40];

	*pmsg = *pxref = *pval = 0;
	if (!p || *p == 0) {
		sprintf(scratch, _(qSreremp), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	striptrail(p);
	if (strlen(p) > MAXLINELEN) {
		sprintf(scratch, _(qSrerlng), flineno);
		*pmsg = scratch;
		return ERROR;
	}

/* Get level number */
	skipws(&p);
	if (chartype((uchar)*p) != DIGIT) {
		sprintf(scratch, _(qSrernlv), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	lev = (uchar)*p++ - (uchar)'0';
	while (chartype((uchar)*p) == DIGIT)
		lev = lev*10 + (uchar)*p++ - (uchar)'0';
	*plev = lev;

/* Get cross reference, if there */
	skipws(&p);
	if (*p == 0) {
		sprintf(scratch, _(qSrerinc), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	if (*p != '@') goto gettag;
	*pxref = p++;
	if (*p == '@') {
		sprintf(scratch, _(qSrerbln), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	while (*p != '@') p++;
	p++;
	if (*p == 0) {
		sprintf(scratch, _(qSrerinc), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	if (!iswhite((uchar)*p)) {
		sprintf(scratch, _(qSrernwt), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	*p++ = 0;

/* Get tag field */
gettag:
	skipws(&p);
	if (*p == 0) {
		sprintf(scratch, _(qSrerinc), flineno);
		*pmsg = scratch;
		return ERROR;
	}
	*ptag = p++;
	while (!iswhite((uchar)*p) && *p != 0) p++;
	if (*p == 0) return OKAY;
	*p++ = 0;

/* Get the value field */
	skipws(&p);
	*pval = p;
	return OKAY;
}
/*=================================================
 * file_to_record -- Convert GEDCOM file to in-memory record
 *
 * fname:[IN]  name of file that holds GEDCOM record
 * ttm:  [IN]  character translation table
 * pmsg: [OUT] possible error message
 * pemp: [OUT] set true if file is empty
 *===============================================*/
RECORD
file_to_record (STRING fname, XLAT ttm, STRING *pmsg, BOOLEAN *pemp)
{
	NODE node = file_to_node(fname, ttm, pmsg, pemp);
	RECORD rec = 0;
	if (node) {
		rec = create_record(node);
	}
	return rec;
}
/*=================================================
 * file_to_node -- Convert GEDCOM file to NODE tree
 *
 * fname: [IN]  name of file that holds GEDCOM record
 * ttm:   [IN]  character translation table
 * pmsg:  [OUT] possible error message
 * pemp:  [OUT] set true if file is empty
 * TODO: When can we delete this ?
 *===============================================*/
NODE
file_to_node (STRING fname, XLAT ttm, STRING *pmsg, BOOLEAN *pemp)
{
	FILE *fp;
	NODE node;
	static char scratch[100];
	*pmsg = NULL;
	*pemp = FALSE;
	if (!(fp = fopen(fname, LLREADTEXT))) {
		sprintf(scratch, "Could not open file %s",  fname);
		*pmsg = scratch;
		return NULL;
	}
	node = convert_first_fp_to_node(fp, TRUE, ttm, pmsg, pemp);
	fclose(fp);
	return node;
}
static INT lev;
static INT lev0;
static STRING xref;
static STRING tag;
static STRING val;
static BOOLEAN lahead = FALSE;
static BOOLEAN ateof = FALSE;

/*================================================================
 * convert_first_fp_to_node -- Convert first GEDCOM record in file to tree
 *
 * fp:   [IN]  file that holds GEDCOM record/s
 * list: [IN]  can be list at level 0?
 * ttm:  [IN]  character translation table
 * pmsg: [OUT] possible error message
 * peof: [OUT] set true if file is at end of file
 * TODO: revise import (restore_record) so can delete this
 *==============================================================*/
NODE
convert_first_fp_to_node (FILE *fp, BOOLEAN list, XLAT ttm,
	STRING *pmsg,  BOOLEAN *peof)
{
	STRING unitype = check_file_for_unicode(fp);
	if (unitype && !eqstr(unitype, "UTF-8")) {
		char msg[120];
		llstrncpyf(msg, sizeof(msg), uu8, _(qSunsupuniv), unitype);
		/* TODO: need to pass msg up to caller somehow */
		*pmsg = _(qSunsupunix);
		return NULL;
	}
	return do_first_fp_to_node(fp, list, ttm, pmsg, peof);
}
/*================================================================
 * convert_first_fp_to_record -- Convert first GEDCOM record in file to tree
 *
 * fp:   [IN]  file that holds GEDCOM record/s
 * list: [IN]  can be list at level 0?
 * ttm:  [IN]  character translation table
 * pmsg: [OUT] possible error message
 * peof: [OUT] set true if file is at end of file
 *==============================================================*/
static RECORD
convert_first_fp_to_record (FILE *fp, BOOLEAN list, XLAT ttm,
	STRING *pmsg,  BOOLEAN *peof)
{
	STRING unitype = check_file_for_unicode(fp);
	if (unitype && !eqstr(unitype, "UTF-8")) {
		char msg[120];
		llstrncpyf(msg, sizeof(msg), uu8, _(qSunsupuniv), unitype);
		/* TODO: need to pass msg up to caller somehow */
		*pmsg = _(qSunsupunix);
		return NULL;
	}
	return do_first_fp_to_record(fp, list, ttm, pmsg, peof);
}
/*================================================================
 * do_first_fp_to_node -- Convert first GEDCOM record in file to tree
 *
 *  fp:    [IN]  file that holds GEDCOM record/s
 *  list:  [IN]  can be list at level 0?
 *  tt:    [IN]  character translation table
 *  *pmsg: [OUT] possible error message
 *  *peof: [OUT] set true if file is at end of file
 * Called after unicode header processed
 * TODO: revise import (restore_record) so can delete this
 *==============================================================*/
static NODE
do_first_fp_to_node (FILE *fp, BOOLEAN list, XLAT ttm,
	STRING *pmsg,  BOOLEAN *peof)
{
	INT rc;
	ateof = FALSE;
	flineno = 0;
	*pmsg = NULL;
	*peof = FALSE;
	rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, pmsg);
	if (rc == DONE) {
		*peof = ateof = TRUE;
		*pmsg = _(qSfileof);
		return NULL;
	} else if (rc == ERROR)
		return NULL;
	lev0 = lev;
	lahead = TRUE;
	return next_fp_to_node(fp, list, ttm, pmsg, peof);
}
/*================================================================
 * do_first_fp_to_record -- Convert first GEDCOM record in file to tree
 *
 *  fp:    [IN]  file that holds GEDCOM record/s
 *  list:  [IN]  can be list at level 0?
 *  tt:    [IN]  character translation table
 *  *pmsg: [OUT] possible error message
 *  *peof: [OUT] set true if file is at end of file
 * Called after unicode header processed
 *==============================================================*/
static RECORD
do_first_fp_to_record (FILE *fp, BOOLEAN list, XLAT ttm,
	STRING *pmsg,  BOOLEAN *peof)
{
	INT rc;
	ateof = FALSE;
	flineno = 0;
	*pmsg = NULL;
	*peof = FALSE;
	rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, pmsg);
	if (rc == DONE) {
		*peof = ateof = TRUE;
		*pmsg = _(qSfileof);
		return NULL;
	} else if (rc == ERROR)
		return NULL;
	lev0 = lev;
	lahead = TRUE;
	return next_fp_to_record(fp, list, ttm, pmsg, peof);
}
/*==============================================================
 * next_fp_to_record -- Convert next GEDCOM record in file to tree
 *
 *  fp:   [IN]  file that holds GEDCOM record/s
 *  list: [IN]  can be list at level 0?
 *  ttm:  [IN]  character translation table
 *  pmsg: [OUT] possible error message
 *  peof: [OUT] set true if file is at end of file
 *============================================================*/
RECORD
next_fp_to_record (FILE *fp, BOOLEAN list, XLAT ttm,
	STRING *pmsg, BOOLEAN *peof)
{
	NODE node = next_fp_to_node(fp, list, ttm, pmsg, peof);
	return create_record(node);
}
/*==============================================================
 * next_fp_to_node -- Convert next GEDCOM record in file to tree
 *
 *  fp:   [IN]  file that holds GEDCOM record/s
 *  list: [IN]  can be list at level 0?
 *  ttm:  [IN]  character translation table
 *  pmsg: [OUT] possible error message
 *  peof: [OUT] set true if file is at end of file
 * callers should probably be converted to calling next_fp_to_record
 *============================================================*/
NODE
next_fp_to_node (FILE *fp, BOOLEAN list, XLAT ttm,
	STRING *pmsg, BOOLEAN *peof)
{
	INT curlev, bcode, rc;
	NODE root, node, curnode;
	char scratch[100];
	*pmsg = NULL;
	*peof = FALSE;
	if (ateof) {
		ateof = *peof = TRUE;
		lahead = FALSE;
		return NULL;
	}
	if (!lahead) {
		rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, pmsg);
		if (rc == DONE) {
			ateof = *peof = TRUE;
			return NULL;
		} else if (rc == ERROR)
			return NULL;
		lahead = TRUE;
	}
	curlev = lev;
	if (curlev != lev0)  {
		*pmsg = _(qSrerwlv);
		return NULL;
	}
	root = curnode = create_node(xref, tag, val, NULL);
	bcode = OKAY;
	rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, pmsg);
	while (rc == OKAY) {
		if (lev == curlev) {
			if (lev == lev0 && !list) {
				bcode = DONE;
				break;
			}
			node = create_node(xref, tag, val, nparent(curnode));
			nsibling(curnode) = node;
			curnode = node;
		} else if (lev == curlev + 1) {
			node = create_node(xref, tag, val, curnode);
			nchild(curnode) = node;
			curnode = node;
			curlev = lev;
		} else if (lev < curlev) {
			if (lev < lev0) {
				sprintf(scratch, _(qSrerilv), flineno);
				*pmsg = scratch;
				bcode = ERROR;
				break;
			}
			if (lev == lev0 && !list) {
				bcode = DONE;
				break;
			}
			while (lev < curlev) {
				curnode = nparent(curnode);
				curlev--;
			}
			node = create_node(xref, tag, val, nparent(curnode));
			nsibling(curnode) = node;
			curnode = node;
		} else {
			sprintf(scratch, _(qSrerilv), flineno);
			*pmsg = scratch;
			bcode = ERROR;
			break;
		}
		rc = file_to_line(fp, ttm, &lev, &xref, &tag, &val, pmsg);
	}
	if (bcode == DONE) return root;
	if (bcode == ERROR || rc == ERROR) {
		free_nodes(root);
		return NULL;
	}
	lahead = FALSE;
	ateof = *peof = TRUE;
	return root;
}
/*============================================
 * string_to_record -- Read record from data block
 *  This is the layout for metadata nodes:
 *   Q___      (four bytes, but only first char used)
 *   0016      (offset to 0 INDI... line)
 *   whhdr     (warehouse header)
 *   whdata    (warehouse data)
 *   0 INDI    (traditional node data)
 *  This is the layout for traditional nodes:
 *   0 INDI    (or 0 FAM or 0 SOUR etc)
 *==========================================*/
RECORD
string_to_record (STRING str, CNSTRING key, INT len)
{
	RECORD rec = 0;
	NODE node = 0;
	len=len; /* unused */

	/* create it now, & release it at bottom if we fail */
	rec = alloc_new_record();
	/* we must fill in the top & mdwh fields */

	if (*str == '0') /* traditional node, no metadata */
		node = string_to_node(str);
	else if (*str == 'Q') {
		/*
		first four bytes just used for the Q flag
		second four bytes contain node_offset from str
		NB: UNTESTED because it isn't yet being written
		Perry, 2001/01/15
		*/
		INT * ptr = (INT *)str;
		INT node_offset = ptr[1]; /* in characters */
		char * whptr = (char *)&ptr[2];
		char * node_ptr = str + node_offset;
		INT whlen = node_offset - 8;
		ASSERT(0); /* not yet being written */
		load_record_wh(rec, whptr, whlen);
		node = string_to_node(node_ptr);
	} else {
		if (!strcmp(str, "DELE\n")) {
			/* should have been filtered out in getrecord */
			ASSERT(0); /* deleted record */
		} else {
			ASSERT(0); /* corrupt record */
		}
	}
	if (node) {
		init_new_record_and_just_read_node(rec, node, key);
	} else { /* node==0, we failed, clean up */
		free_rec(rec);
		rec = 0;
	}
	return rec;
}
/*========================================
 * string_to_node -- Read tree from string
 *======================================*/
NODE
string_to_node (STRING str)
{
	INT lev;
	INT lev0;
	STRING xref;
	STRING tag;
	STRING val;

	INT curlev;
	NODE root, node, curnode;
	STRING msg;
	flineno = 0;
	if (!string_to_line(&str, &lev, &xref, &tag, &val, &msg))
		return NULL;
	lev0 = curlev = lev;
	root = curnode = create_node(xref, tag, val, NULL);
	while (string_to_line(&str, &lev, &xref, &tag, &val, &msg)) {
		if (lev == curlev) {
			node = create_node(xref, tag, val, nparent(curnode));
			nsibling(curnode) = node;
			curnode = node;
		} else if (lev == curlev + 1) {
			node = create_node(xref, tag, val, curnode);
			nchild(curnode) = node;
			curnode = node;
			curlev = lev;
		} else if (lev < curlev) {
			if (lev < lev0) {
				llwprintf("Error: line %d: illegal level",
				    flineno);
				return NULL;
			}
			while (lev < curlev) {
				curnode = nparent(curnode);
				curlev--;
			}
			node = create_node(xref, tag, val, nparent(curnode));
			nsibling(curnode) = node;
			curnode = node;
		} else {
			llwprintf("Error: line %d: illegal level", flineno);
			return NULL;
		}
	}
	if (!msg) return root;
	free_nodes(root);
	return NULL;
}
#if 0
/*============================================
 * node_to_file -- Convert tree to GEDCOM file
 *==========================================*/
BOOLEAN
node_to_file (INT levl,       /* top level */
              NODE node,      /* root node */
              STRING fname,   /* file */
              BOOLEAN indent, /* indent? */
              TRANTABLE tt)   /* char map */
{
	FILE *fp;
	if (!(fp = fopen(fname, LLWRITETEXT))) {
		llwprintf("Could not open file: `%s'\n", fname);
		return FALSE;
	}
	write_nodes(levl, fp, tt, node, indent, TRUE, TRUE);
	fclose(fp);
	return TRUE;
}
#endif
/*========================================
 * write_node -- Write NODE to GEDCOM file
 *
 * INT levl:       [in] level
 * FILE *fp:       [in] file
 * TRANTABLE tt    [in] char map
 * NODE node:      [in] node
 * BOOLEAN indent: [in]indent?
 *======================================*/
static void
write_node (INT levl, FILE *fp, XLAT ttm, NODE node,
	BOOLEAN indent)
{
	char out[MAXLINELEN+1];
	STRING p;
	if (indent) {
		INT i;
		for (i = 1;  i < levl;  i++)
			fprintf(fp, "  ");
	}
	fprintf(fp, "%d", levl);
	if (nxref(node)) fprintf(fp, " %s", nxref(node));
	fprintf(fp, " %s", ntag(node));
	if ((p = nval(node))) {
		if (ttm) {
			translate_string(ttm, nval(node), out, MAXLINELEN+1);
			p = out;
		}
		fprintf(fp, " %s", p);
	}
	fprintf(fp, "\n");
}
/*==========================================
 * write_nodes -- Write NODEs to GEDCOM file
 *========================================*/
void
write_nodes (INT levl,       /* level */
             FILE *fp,       /* file */
             XLAT ttm,   /* char map */
             NODE node,      /* root */
             BOOLEAN indent, /* indent? */
             BOOLEAN kids,   /* output kids? */
             BOOLEAN sibs)   /* output sibs? */
{
	if (!node) return;
	write_node(levl, fp, ttm, node, indent);
	if (kids)
		write_nodes(levl+1, fp, ttm, nchild(node), indent, TRUE, TRUE);
	if (sibs)
		write_nodes(levl, fp, ttm, nsibling(node), indent, kids, TRUE);
}
/*====================================
 * swrite_node -- Write NODE to string
 *==================================*/
static STRING
swrite_node (INT levl,       /* level */
             NODE node,      /* node */
             STRING p)       /* write string */
{
	char scratch[600];
	STRING q = scratch;
	sprintf(q, "%d ", levl);
	q += strlen(q);
	if (nxref(node)) {
		strcpy(q, nxref(node));
		q += strlen(q);
		*q++ = ' ';
	}
	strcpy(q, ntag(node));
	q += strlen(q);
	if (nval(node)) {
		*q++ = ' ';
		strcpy(q, nval(node));
		q += strlen(q);
	}
	*q++ = '\n';
	*q = 0;
	strcpy(p, scratch);
	return p + strlen(p);
}
/*=====================================
 * swrite_nodes -- Write tree to string
 *===================================*/
static STRING
swrite_nodes (INT levl,       /* level */
              NODE node,      /* root */
              STRING p)       /* write string */
{
	while (node) {
		p = swrite_node(levl, node, p);
		if (nchild(node))
			p = swrite_nodes(levl + 1, nchild(node), p);
		node = nsibling(node);
	}
	return p;
}
/*=========================================
 * node_to_string -- Convert tree to string
 *=======================================*/
STRING
node_to_string (NODE node)      /* root */
{
	INT len = tree_strlen(0, node) + 1;
	STRING str;
	if (len <= 0) return NULL;
	str = (STRING) stdalloc(len);
	(void) swrite_nodes(0, node, str);
	return str;
}
/*===================================
 * load_record_wh -- load existing warehouse
 *  for a record
 *  warehouse is opaque lump of metadata
 * Created: 2001/02/04, Perry Rapp
 *=================================*/
static void
load_record_wh (RECORD rec, char * whptr, INT whlen)
{
	rec->mdwh = (WAREHOUSE)stdalloc(sizeof(*(rec->mdwh)));
	wh_assign_from_blob(rec->mdwh, whptr, whlen);
}
/*=====================================
 * write_indi_to_file - write node tree into GEDCOM
 * (no user interaction)
 *===================================*/
void
write_indi_to_file (NODE indi, CNSTRING file)
{
	FILE *fp;
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	NODE name, refn, sex, body, famc, fams;
	
	ASSERT(fp = fopen(file, LLWRITETEXT));
	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
	write_nodes(0, fp, ttmo, indi, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, name, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, refn, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, sex,   TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, body , TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, famc,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, fams,  TRUE, TRUE, TRUE);
	fclose(fp);
	join_indi(indi, name, refn, sex, body, famc, fams);
}
/*=====================================
 * write_fam_to_file -- write node tree into GEDCOM
 * (no user interaction)
 *===================================*/
void
write_fam_to_file (NODE fam, CNSTRING file)
{
	FILE *fp;
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	NODE refn, husb, wife, chil, body;

	ASSERT(fp = fopen(file, LLWRITETEXT));
	split_fam(fam, &refn, &husb, &wife, &chil, &body);
	write_nodes(0, fp, ttmo, fam,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, refn, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, husb,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, wife,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, body,  TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, chil,  TRUE, TRUE, TRUE);
	join_fam(fam, refn, husb, wife, chil, body);
	fclose(fp);
}

