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
 * node.c -- Standard GEDCOM NODE operations
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 04 Sep 93
 *   3.0.0 - 29 Aug 94    3.0.2 - 23 Dec 94
 *   3.0.3 - 16 Jan 96
 *===========================================================*/

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "feedback.h"
#include "warehouse.h"
#include "metadata.h"
#include "lloptions.h"

/*********************************************
 * global/exported variables
 *********************************************/

INT flineno = 0;
INT travlineno;
BOOLEAN add_metadata = FALSE;

/*********************************************
 * external/imported variables
 *********************************************/

extern STRING fileof, reremp, rerlng, rernlv, rerinc;
extern STRING rerbln, rernwt, rerilv, rerwlv, qSunsupuni;

/*********************************************
 * local types
 *********************************************/

/* node allocator's freelist */
typedef struct blck *NDALLOC;
struct blck { NDALLOC next; };

/*********************************************
 * local enums & defines
 *********************************************/

enum { NEW_RECORD, EXISTING_LACKING_WH_RECORD };

/*********************************************
 * local function prototypes, alphabetical
 *********************************************/

static RECORD alloc_new_record(void);
static RECORD alloc_record_from_key(STRING key);
static void alloc_record_wh(RECORD rec, INT isnew);
static NODE alloc_node(void);
static void assign_record(RECORD rec, char ntype, INT keynum);
static BOOLEAN buffer_to_line (STRING p, INT *plev, STRING *pxref
	, STRING *ptag, STRING *pval, STRING *pmsg);
static STRING fixup (STRING str);
static STRING fixtag (STRING tag);
static void load_record_wh(RECORD rec, char * whptr, INT whlen);
static INT node_strlen(INT levl, NODE node);
static BOOLEAN string_to_line(STRING *ps, INT *plev, STRING *pxref, 
	STRING *ptag, STRING *pval, STRING *pmsg);
static STRING swrite_node(INT levl, NODE node, STRING p);
static STRING swrite_nodes(INT levl, NODE node, STRING p);
static void write_node(INT levl, FILE *fp, TRANTABLE tt,
	NODE node, BOOLEAN indent);

/*********************************************
 * unused local function prototypes
 *********************************************/

#ifdef UNUSED_CODE
static BOOLEAN all_digits (STRING);
NODE children_nodes(NODE faml);
NODE father_nodes(NODE faml);
NODE mother_nodes(NODE faml);
NODE parents_nodes(NODE faml);
#endif /* UNUSED_CODE */

/*********************************************
 * local variables
 *********************************************/

/* node allocator's free list */
static NDALLOC first_blck = (NDALLOC) 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==============================
 * fixup -- Save non-tag strings
 *============================*/
static STRING
fixup (STRING str)
{
	if (!str || *str == 0) return NULL;
	return strsave(str);
}
/*=============================
 * fixtag -- Keep tags in table
 *===========================*/
static STRING
fixtag (STRING tag)
{
	STRING str;
	if ((str = valueof_str(tagtable, tag))) return str;
	str = strsave(tag);
	insert_table_str(tagtable, str, str);
	return str;
}
/*=====================================
 * alloc_node -- Special node allocator
 *===================================*/
static NODE
alloc_node (void)
{
	NODE node;
	NDALLOC blck;
	int i;
	if (first_blck == (NDALLOC) 0) {
		node = (NODE) stdalloc(100*sizeof(*node));
		first_blck = (NDALLOC) node;
		for (i = 1; i <= 99; i++) {
			blck = (NDALLOC) node;
			blck->next = (NDALLOC) (node + 1);
			node++;
		}
		((NDALLOC) node)->next = (NDALLOC) 0;
	}
	node = (NODE) first_blck;
	first_blck = first_blck->next;
	return node;
}
/*======================================
 * free_node -- Special node deallocator
 *====================================*/
void
free_node (NODE node)
{
	if (nxref(node)) stdfree(nxref(node));
	if (nval(node)) stdfree(nval(node));
	((NDALLOC) node)->next = first_blck;
	first_blck = (NDALLOC) node;
	
}
/*===========================
 * create_node -- Create NODE
 *
 * STRING xref  [in] xref
 * STRING tag   [in] tag
 * STRING val:  [in] value
 * NODE prnt:   [in] parent
 *=========================*/
NODE
create_node (STRING xref, STRING tag, STRING val, NODE prnt)
{
	NODE node = alloc_node();
	nxref(node) = fixup(xref);
	ntag(node) = fixtag(tag);
	nval(node) = fixup(val);
	nparent(node) = prnt;
	nchild(node) = NULL;
	nsibling(node) = NULL;
	return node;
}
/*===================================
 * alloc_new_record -- record allocator
 *  perhaps should use special allocator like nodes
 * Created: 2001/01/25, Perry Rapp
 *=================================*/
static RECORD
alloc_new_record (void)
{
	RECORD rec;
	rec = (RECORD)stdalloc(sizeof(*rec));
	/* these must be filled in by caller */
	rec->nkey.key = "";
	rec->nkey.keynum = 0;
	rec->nkey.ntype = 0;
	rec->top = 0;
	rec->mdwh = 0;
	return rec;
}
/*===================================
 * alloc_record_from_key -- allocate record for key
 * Created: 2001/01/25, Perry Rapp
 *=================================*/
static RECORD
alloc_record_from_key (STRING key)
{
	RECORD rec = alloc_new_record();
	assign_record(rec, key[0], atoi(key+1));
	return rec;
}
/*===================================
 * assign_record -- put key info into record
 * Created: 2001/02/04, Perry Rapp
 *=================================*/
static void
assign_record (RECORD rec, char ntype, INT keynum)
{
	char xref[12];
	char key[9];
	sprintf(key, "%c%d", ntype, keynum);
	sprintf(xref, "@%s@", key);
	if (nztop(rec))
		nxref(nztop(rec)) = strsave(xref);
	rec->nkey.key = strsave(key);
	rec->nkey.keynum = keynum;
	rec->nkey.ntype = ntype;
}
/*===================================
 * init_new_record -- put key info into record
 *  of brand new record
 * Created: 2001/02/04, Perry Rapp
 *=================================*/
void
init_new_record (RECORD rec, char ntype, INT keynum)
{
	assign_record(rec, ntype, keynum);
	alloc_record_wh(rec, NEW_RECORD);
}
/*===================================
 * alloc_record_wh -- allocate warehouse for
 *  a record without one (new or existing)
 * Created: 2001/02/04, Perry Rapp
 *=================================*/
static void
alloc_record_wh (RECORD rec, INT isnew)
{
	LLDATE creation;
	ASSERT(!rec->mdwh); /* caller must know what it is doing */
	if (!add_metadata)
		return;
	rec->mdwh = (WAREHOUSE)stdalloc(sizeof(*(rec->mdwh)));
	wh_allocate(rec->mdwh);
	get_current_lldate(&creation);
	wh_add_block_var(rec->mdwh, MD_CREATE_DATE, &creation, sizeof(creation));
	if (isnew == EXISTING_LACKING_WH_RECORD)
		wh_add_block_int(rec->mdwh, MD_CONVERTED_BOOL, 1);
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
/*===================================
 * create_record -- create record to wrap top node
 * Created: 2001/01/29, Perry Rapp
 *=================================*/
RECORD
create_record (NODE node)
{
	RECORD rec = 0;
	if (nxref(node))
		rec = alloc_record_from_key(node_to_key(node));
	else
		rec = alloc_new_record();
	rec->top = node;
	return rec;
}
/*===================================
 * free_rec -- record deallocator
 * Created: 2000/12/30, Perry Rapp
 *=================================*/
void
free_rec (RECORD rec)
{
	if (rec->top)
		free_nodes(rec->top);
	if (rec->mdwh) {
		stdfree(rec->mdwh);
	}
	if (rec->nkey.key[0])
		stdfree(rec->nkey.key);
	stdfree(rec);
}
/*=====================================
 * free_nodes -- Free all NODEs in tree
 *===================================*/
void
free_nodes (NODE node)
{
	NODE sib;
	while (node) {
		if (nchild(node)) free_nodes(nchild(node));
		sib = nsibling(node);
		free_node(node);
		node = sib;
	}
}
/*==========================================
 * file_to_line -- Get GEDCOM line from file
 *========================================*/
INT
file_to_line (FILE *fp,
              TRANTABLE tt,
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
		if (!(p = fgets(in, MAXLINELEN+2, fp))) return DONE;
		flineno++;
		if (tt) {
			translate_string(tt, in, out, MAXLINELEN+2);
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
string_to_line (STRING *ps,     /* string ptr - modified */
                INT *plev,      /* level ptr */
                STRING *pxref,  /* cross-ref ptr */
                STRING *ptag,   /* tag ptr */
                STRING *pval,   /* value ptr */
                STRING *pmsg)   /* error msg ptr */
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
		sprintf(scratch, reremp, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	striptrail(p);
	if (strlen(p) > MAXLINELEN) {
		sprintf(scratch, rerlng, flineno);
		*pmsg = scratch;
		return ERROR;
	}

/* Get level number */
	while (iswhite((uchar)*p)) p++;
	if (chartype((uchar)*p) != DIGIT) {
		sprintf(scratch, rernlv, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	lev = (uchar)*p++ - (uchar)'0';
	while (chartype((uchar)*p) == DIGIT)
		lev = lev*10 + (uchar)*p++ - (uchar)'0';
	*plev = lev;

/* Get cross reference, if there */
	while (iswhite((uchar)*p)) p++;
	if (*p == 0) {
		sprintf(scratch, rerinc, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	if (*p != '@') goto gettag;
	*pxref = p++;
	if (*p == '@') {
		sprintf(scratch, rerbln, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	while (*p != '@') p++;
	p++;
	if (*p == 0) {
		sprintf(scratch, rerinc, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	if (!iswhite((uchar)*p)) {
		sprintf(scratch, rernwt, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	*p++ = 0;

/* Get tag field */
gettag:
	while (iswhite((uchar)*p)) p++;
	if (*p == 0) {
		sprintf(scratch, rerinc, flineno);
		*pmsg = scratch;
		return ERROR;
	}
	*ptag = p++;
	while (!iswhite((uchar)*p) && *p != 0) p++;
	if (*p == 0) return OKAY;
	*p++ = 0;

/* Get the value field */
	while (iswhite((uchar)*p)) p++;
	*pval = p;
	return OKAY;
}
/*=================================================
 * file_to_record -- Convert GEDCOM file to in-memory record
 *
 * STRING fname:  [in] name of file that holds GEDCOM record
 * TRANTABLE tt:  [in] character translation table
 * STRING *pmsg:  [out] possible error message
 * BOOLEAN *pemp: [out] set true if file is empty
 *===============================================*/
RECORD
file_to_record (STRING fname, TRANTABLE tt, STRING *pmsg, BOOLEAN *pemp)
{
	NODE node = file_to_node(fname, tt, pmsg, pemp);
	RECORD rec = 0;
	if (node) {
		rec = create_record(node);
	}
	return rec;
}
/*=================================================
 * file_to_node -- Convert GEDCOM file to NODE tree
 *
 * STRING fname:  [in] name of file that holds GEDCOM record
 * TRANTABLE tt:  [in] character translation table
 * STRING *pmsg:  [out] possible error message
 * BOOLEAN *pemp: [out] set true if file is empty
 *===============================================*/
NODE
file_to_node (STRING fname, TRANTABLE tt, STRING *pmsg, BOOLEAN *pemp)
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
	if (!check_file_for_unicode(fp)) {
		*pmsg = _(qSunsupuni);
		return NULL;
	}
	node = first_fp_to_node(fp, TRUE, tt, pmsg, pemp);
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
 * first_fp_to_node -- Convert first GEDCOM record in file to tree
 *
 * FILE *fp:      [in] file that holds GEDCOM record/s
 * BOOLEAN list:  [in] can be list at level 0?
 * TRANTABLE tt:  [in] character translation table
 * STRING *pmsg:  [out] possible error message
 * BOOLEAN *peof: [out] set true if file is at end of file
 *==============================================================*/
NODE
first_fp_to_node (FILE *fp, BOOLEAN list, TRANTABLE tt,
	STRING *pmsg,  BOOLEAN *peof)
{
	INT rc;
	ateof = FALSE;
	flineno = 0;
	*pmsg = NULL;
	*peof = FALSE;
	rc = file_to_line(fp, tt, &lev, &xref, &tag, &val, pmsg);
	if (rc == DONE) {
		*peof = ateof = TRUE;
		*pmsg = fileof;
		return NULL;
	} else if (rc == ERROR)
		return NULL;
	lev0 = lev;
	lahead = TRUE;
	return next_fp_to_node(fp, list, tt, pmsg, peof);
}
/*==============================================================
 * next_fp_to_record -- Convert next GEDCOM record in file to tree
 *
 * FILE *fp:      [in] file that holds GEDCOM record/s
 * BOOLEAN list:  [in] can be list at level 0?
 * TRANTABLE tt:  [in] character translation table
 * STRING *pmsg:  [out] possible error message
 * BOOLEAN *peof: [out] set true if file is at end of file
 *============================================================*/
RECORD
next_fp_to_record (FILE *fp, BOOLEAN list, TRANTABLE tt,
	STRING *pmsg, BOOLEAN *peof)
{
	NODE node = next_fp_to_node(fp, list, tt, pmsg, peof);
	return create_record(node);
}
/*==============================================================
 * next_fp_to_node -- Convert next GEDCOM record in file to tree
 *
 * FILE *fp:      [in] file that holds GEDCOM record/s
 * BOOLEAN list:  [in] can be list at level 0?
 * TRANTABLE tt:  [in] character translation table
 * STRING *pmsg:  [out] possible error message
 * BOOLEAN *peof: [out] set true if file is at end of file
 *  callers should probably be converted to calling next_fp_to_record
 *============================================================*/
NODE
next_fp_to_node (FILE *fp, BOOLEAN list, TRANTABLE tt,
	STRING *pmsg, BOOLEAN *peof)
{
	INT curlev, bcode, rc;
	NODE root, node, curnode;
	char scratch[100];
	*pmsg = NULL;
	*peof = FALSE;
	if (ateof) {
		ateof = *peof = TRUE;
		*pmsg = fileof;
		lahead = FALSE;
		return NULL;
	}
	if (!lahead) {
		rc = file_to_line(fp, tt, &lev, &xref, &tag, &val, pmsg);
		if (rc == DONE) {
			ateof = *peof = TRUE;
			*pmsg = fileof;
			return NULL;
		} else if (rc == ERROR)
			return NULL;
		lahead = TRUE;
	}
	curlev = lev;
	if (curlev != lev0)  {
		*pmsg = rerwlv;
		return NULL;
	}
	root = curnode = create_node(xref, tag, val, NULL);
	bcode = OKAY;
	rc = file_to_line(fp, tt, &lev, &xref, &tag, &val, pmsg);
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
				sprintf(scratch, rerilv, flineno);
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
			sprintf(scratch, rerilv, flineno);
			*pmsg = scratch;
			bcode = ERROR;
			break;
		}
		rc = file_to_line(fp, tt, &lev, &xref, &tag, &val, pmsg);
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
string_to_record (STRING str, STRING key, INT len)
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
		rec->top = node;
		assign_record(rec, key[0], atoi(key+1));
		if (!rec->mdwh)
			alloc_record_wh(rec, EXISTING_LACKING_WH_RECORD);
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
write_node (INT levl, FILE *fp, TRANTABLE tt, NODE node,
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
	    if (tt) {
		translate_string(tt, nval(node), out, MAXLINELEN+1);
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
             TRANTABLE tt,   /* char map */
             NODE node,      /* root */
             BOOLEAN indent, /* indent? */
             BOOLEAN kids,   /* output kids? */
             BOOLEAN sibs)   /* output sibs? */
{
	if (!node) return;
	write_node(levl, fp, tt, node, indent);
	if (kids)
		write_nodes(levl+1, fp, tt, nchild(node), indent, TRUE, TRUE);
	if (sibs)
		write_nodes(levl, fp, tt, nsibling(node), indent, kids, TRUE);
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
/*==============================================================
 * tree_strlen -- Compute string length of tree -- don't count 0
 *============================================================*/
INT
tree_strlen (INT levl,       /* level */
             NODE node)      /* root */
{
	INT len = 0;
	while (node) {
		len += node_strlen(levl, node);
		if (nchild(node))
			len += tree_strlen(levl + 1, nchild(node));
		node = nsibling(node);
	}
	return len;
}
/*================================================================
 * node_strlen -- Compute NODE string length -- count \n but not 0
 *==============================================================*/
static INT
node_strlen (INT levl,       /* level */
             NODE node)      /* node */
{
	INT len;
	char scratch[10];
	sprintf(scratch, "%d", levl);
	len = strlen(scratch) + 1;
	if (nxref(node)) len += strlen(nxref(node)) + 1;
	len += strlen(ntag(node));
	if (nval(node)) len += strlen(nval(node)) + 1;
	return len + 1;
}
/*==========================================
 * unknown_node_to_dbase -- Store node of unknown type
 *  in database
 * Created: 2001/04/06, Perry Rapp
 *========================================*/
void
unknown_node_to_dbase (NODE node)
{
	/* skip tag validation */
	node_to_dbase(node, NULL);
}
/*==========================================
 * indi_to_dbase -- Store person in database
 *========================================*/
void
indi_to_dbase (NODE node)
{
	/*
	To start storing metadata, we need the RECORD here
	If we were using RECORD everywhere, we'd pass it in here
	We could look it up in the cache, but it might not be there
	(new records)
	(and this applies to fam_to_dbase, sour_to_dbase, etc)
	Perry, 2001/01/15
	*/
	node_to_dbase(node, "INDI");
}
/*=========================================
 * fam_to_dbase -- Store family in database
 *=======================================*/
void
fam_to_dbase (NODE node)
{
	node_to_dbase(node, "FAM");
}
/*=========================================
 * even_to_dbase -- Store event in database
 *=======================================*/
void
even_to_dbase (NODE node)
{
	node_to_dbase(node, "EVEN");
}
/*==========================================
 * sour_to_dbase -- Store source in database
 *========================================*/
void
sour_to_dbase (NODE node)
{
	node_to_dbase(node, "SOUR");
}
/*================================================
 * othr_to_dbase -- Store other record in database
 *==============================================*/
void
othr_to_dbase (NODE node)
{
	node_to_dbase(node, NULL);
}
/*===============================================
 * node_to_dbase -- Store GEDCOM tree in database
 *=============================================*/
void
node_to_dbase (NODE node,
               STRING tag)
{
	STRING str;
	ASSERT(node);
	if (tag) { ASSERT(eqstr(tag, ntag(node))); }
	str = node_to_string(node);
	ASSERT(store_record(rmvat(nxref(node)), str, strlen(str)));
	stdfree(str);
}
/*==================================================
 * indi_to_famc -- Return family-as-child for person
 *================================================*/
NODE
indi_to_famc (NODE node)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "FAMC"))) return NULL;
	return key_to_fam(rmvat(nval(node)));
}
/*========================================
 * fam_to_husb -- Return husband of family
 *======================================*/
NODE
fam_to_husb (NODE node)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "HUSB"))) return NULL;
	return key_to_indi(rmvat(nval(node)));
}
/*=====================================
 * fam_to_wife -- Return wife of family
 *===================================*/
NODE
fam_to_wife (NODE node)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "WIFE"))) return NULL;
	return key_to_indi(rmvat(nval(node)));
}
/*===============================================
 * fam_to_spouse -- Return other spouse of family
 *=============================================*/
NODE
fam_to_spouse (NODE fam,
               NODE indi)
{
    	INT num;
	if (!fam) return NULL;
	FORHUSBS(fam, husb, num)
		if(husb != indi) return(husb);
	ENDHUSBS
	FORWIFES(fam, wife, num)
	  if(wife != indi) return(wife);
	ENDWIFES
	return NULL;
}
/*==================================================
 * fam_to_first_chil -- Return first child of family
 *================================================*/
NODE
fam_to_first_chil (NODE node)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "CHIL"))) return NULL;
	return key_to_indi(rmvat(nval(node)));
}
/*=================================================
 * fam_to_last_chil -- Return last child of family
 *===============================================*/
NODE
fam_to_last_chil (NODE node)
{
	NODE prev = NULL;
	if (!node) return NULL;
	/* find first CHIL in fam */
	if (!(node = find_tag(nchild(node), "CHIL"))) return NULL;
	/* cycle thru all remaining nodes, keeping most recent CHIL node */
	while (node) {
		if (eqstr(ntag(node),"CHIL"))
			prev = node;
		node = nsibling(node);
	}
	return key_to_indi(rmvat(nval(prev)));
}
/*========================================
 * indi_to_fath -- Return father of person
 *======================================*/
NODE
indi_to_fath (NODE node)
{
	return fam_to_husb(indi_to_famc(node));
}
/*========================================
 * indi_to_moth -- Return mother of person
 *======================================*/
NODE
indi_to_moth (NODE node)
{
	return fam_to_wife(indi_to_famc(node));
}
/*==================================================
 * indi_to_prev_sib -- Return previous sib of person
 *================================================*/
NODE
indi_to_prev_sib (NODE indi)
{
	NODE fam, prev, node;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	prev = NULL;
	node = CHIL(fam);
	/* loop thru all nodes following first child, keeping most recent CHIL */
	while (node) {
		if (eqstr(nxref(indi), nval(node))) {
			if (!prev) return NULL;
			return key_to_indi(rmvat(nval(prev)));
		}
		if (eqstr(ntag(node),"CHIL"))
			prev = node;
		node = nsibling(node);
	}
	return NULL;
}
/*==============================================
 * indi_to_next_sib -- Return next sib of person
 *============================================*/
NODE
indi_to_next_sib (NODE indi)
{
	NODE fam, node;
	BOOLEAN found;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	node = CHIL(fam);
	found = FALSE;  /* until we find indi */
	while (node) {
		if (!found) {
			if (eqstr(nxref(indi), nval(node)))
				found = TRUE;
		} else {
			if (eqstr(ntag(node),"CHIL"))
				return key_to_indi(rmvat(nval(node)));
		}
		node = nsibling(node);
	}
	return NULL;
}
/*======================================
 * indi_to_name -- Return name of person
 *====================================*/
STRING
indi_to_name (NODE node, TRANTABLE tt, INT len)
{
	if (node)
		node = find_tag(nchild(node), "NAME");
	if (!node)
		return _("NO NAME");
	return manip_name(nval(node), tt, TRUE, TRUE, len);
}
/*======================================
 * indi_to_title -- Return title of person
 *====================================*/
STRING
indi_to_title (NODE node,
               TRANTABLE tt,
               INT len)
{
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "TITL"))) return NULL;
	return manip_name(nval(node), tt, FALSE, TRUE, len);
}
/*======================================
 * node_to_tag -- Return a subtag of a node
 * (presumably top level, but not necessarily)
 *====================================*/
STRING node_to_tag (NODE node, STRING tag, TRANTABLE tt, INT len)
{
	static char scratch[MAXGEDNAMELEN+1];
	STRING refn;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag)))
		return NULL;
	refn = nval(node);
	if (len > (INT)sizeof(scratch)-1)
		len = sizeof(scratch)-1;
	translate_string(tt, refn, scratch, sizeof(scratch));
	return scratch;
}
/*==============================================
 * indi_to_event -- Convert event tree to string
 *  node: [in] event subtree to search
 *  tt:   [in] translation table to apply to event strings
 *  tag:  [in] desired tag (eg, "BIRT")
 *  head: [in] header to print in output (eg, "born: ")
 *  len:  [in] max length output desired
 * Searches node substree for desired tag
 *  returns formatted string (event_to_string) if found,
 *  else NULL
 *============================================*/
STRING
indi_to_event (NODE node, TRANTABLE tt, STRING tag, STRING head
	, INT len, RFMT rfmt)
{
	static char scratch[200];
	STRING p = scratch;
	INT mylen = sizeof(scratch)/sizeof(scratch[0]);
	STRING event;
	STRING omit;
	if (mylen > len+1) mylen = len+1; /* incl. trailing 0 */
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag))) return NULL;
	event = event_to_string(node, tt, rfmt);
	if (!event) return NULL;
	/* need at least room for head + 1 character + "..." or no point */
	if ((INT)strlen(head)+4>len) return NULL;
	p[0] = 0;
	llstrcatn(&p, head, &mylen);
	if (mylen<(INT)strlen(event)+1) {
		omit = getoptstr("ShortOmitString", NULL);
		if (omit) {
			mylen -= strlen(omit)+1; /* plus trailing 0 */
			llstrcatn(&p, event, &mylen);
			mylen += strlen(omit)+1;
			llstrcatn(&p, omit, &mylen);
		} else {
			llstrcatn(&p, event, &mylen);
		}
	} else {
		llstrcatn(&p, event, &mylen);
		if (mylen && p[-1]!='.')
		llstrcatn(&p, ".", &mylen);
	}
	return scratch;
}
/*===========================================
 * event_to_date_place  -- Find date & place
 *  node:  [IN]  node tree of event to describe
 *  date:  [OUT] value of first DATE line
 *  plac:  [OUT] value of first PLACE line
 *=========================================*/
static void
event_to_date_place (NODE node, STRING * date, STRING * plac)
{
	*date = *plac = NULL;
	if (!node) return;
	node = nchild(node);
	while (node) {
		if (eqstr("DATE", ntag(node)) && !*date) *date = nval(node);
		if (eqstr("PLAC", ntag(node)) && !*plac) *plac = nval(node);
		node = nsibling(node);
	}
}
/*===========================================
 * event_to_string -- Convert event to string
 * Finds DATE & PLACE nodes, and prints a string
 * representation of them.
 *  node:  [in] node tree of event to describe
 *  tt:    [in] translation table to use
 *  rfmt:  [in] reformatting info (may be NULL)
 *=========================================*/
STRING
event_to_string (NODE node, TRANTABLE tt, RFMT rfmt)
{
	static char scratch1[MAXLINELEN+1];
	static char scratch2[MAXLINELEN+1];
	STRING date, plac;
	event_to_date_place(node, &date, &plac);
	if (!date && !plac) return NULL;
	/* Apply optional, caller-specified date & place reformatting */
	if (rfmt && date && rfmt->rfmt_date)
		date = (*rfmt->rfmt_date)(date);
	if (rfmt && plac && rfmt->rfmt_plac)
		plac = (*rfmt->rfmt_plac)(plac);
	if (date && date[0] && plac && plac[0]) {
		sprintpic2(scratch1, sizeof(scratch1), rfmt->combopic, date, plac);
	} else if (date && date[0]) {
		llstrncpy(scratch1, date, sizeof(scratch1));
	} else if (plac && plac[0]) {
		llstrncpy(scratch1, plac, sizeof(scratch1));
	} else {
		return NULL;
	}
	translate_string(tt, scratch1, scratch2, MAXLINELEN);
	return scratch2;
}
/*=======================================
 * event_to_date -- Convert event to date
 *  node: [in] event node
 *  tt:   [in] translation table to apply
 *  shrt: [in] flag - use short form if set
 *=====================================*/
STRING
event_to_date (NODE node, TRANTABLE tt, BOOLEAN shrt)
{
	static char scratch[MAXLINELEN+1];
	if (!node) return NULL;
	if (!(node = DATE(node))) return NULL;
	translate_string(tt, nval(node), scratch, MAXLINELEN);
	if (shrt) return shorten_date(scratch);
	return scratch;
}
/*========================================
 * event_to_plac -- Convert event to place
 *======================================*/
STRING
event_to_plac (NODE node,
               BOOLEAN shrt)
{
	if (!node) return NULL;
	node = PLAC(node);
	if (!node) return NULL;
	if (shrt) return shorten_plac(nval(node));
	return nval(node);
}
/*================================
 * show_node -- Show tree -- DEBUG
 *==============================*/
void
show_node (NODE node)
{
	if (!node) llwprintf("(NIL)");
	show_node_rec(0, node);
}
/*================================================
 * show_node_rec -- Recursive version of show_node
 *==============================================*/
void
show_node_rec (INT levl,
               NODE node)
{
	INT i;
	if (!node) return;
	for (i = 1;  i < levl;  i++)
		llwprintf("  ");
	llwprintf("%d", levl);
	if (nxref(node)) llwprintf(" %s", nxref(node));
	llwprintf(" %s", ntag(node));
	if (nval(node)) llwprintf(" %s", nval(node));
	llwprintf("\n");
	show_node_rec(levl + 1, nchild(node));
	show_node_rec(levl    , nsibling(node));
}
/*===========================================
 * length_nodes -- Return length of NODE list
 *=========================================*/
INT
length_nodes (NODE node)
{
	INT len = 0;
	while (node) {
		len++;
		node = nsibling(node);
	}
	return len;
}
/*================================================
 * shorten_date -- Return short form of date value
 * Returns static buffer.
 *==============================================*/
STRING
shorten_date (STRING date)
{
	static char buffer[3][MAXLINELEN+1];
	static int dex = 0;
	STRING p = date, q;
	INT c, len;
	/* Allow 3 or 4 digit years. The previous test for strlen(date) < 4
	 * prevented dates consisting of only 3 digit years from being
	 * returned. - pbm 12 oct 99 */
	if (!date || (INT) strlen(date) < 3) return NULL;
	if (++dex > 2) dex = 0;
	q = buffer[dex];
	while (TRUE) {
		while ((c = (uchar)*p++) && chartype(c) != DIGIT)
			;
		if (c == 0) return NULL;
		q = buffer[dex];
		*q++ = c;
		len = 1;
		while ((c = (uchar)*p++) && chartype(c) == DIGIT) {
			if (len < 6) {
				*q++ = c;
				len++;
			}
		}
		*q = 0;
		if (strlen(buffer[dex]) == 3 || strlen(buffer[dex]) == 4)
			return buffer[dex];
		if (c == 0) return NULL;
	}
}
/*=================================================
 * shorten_plac -- Return short form of place value
 * Returns modified input string, or value from placabbr table
 *===============================================*/
STRING
shorten_plac (STRING plac)
{
	STRING plac0 = plac, comma, val;
	if (!plac) return NULL;
	comma = (STRING) strrchr(plac, ',');
	if (comma) plac = comma + 1;
	while (*plac++ == ' ')
		;
	plac--;
	if (*plac == 0) return plac0;
	if ((val = valueof_str(placabbvs, plac))) return val;
	return plac;
}

#ifdef UNUSED_CODE
/*============================================
 * all_digits -- Check if string is all digits
 * UNUSED CODE
 *==========================================*/
static BOOLEAN
all_digits (STRING s)
{
	INT c;
	while ((c = *s++)) {
		if (c < '0' || c > '9') return FALSE;
	}
	return TRUE;
}
#endif /* UNUSED_CODE */
/*=======================
 * copy_node -- Copy node
 *=====================*/
NODE
copy_node (NODE node)
{
	return create_node(nxref(node), ntag(node), nval(node), NULL);
}
/*========================
 * copy_nodes -- Copy tree
 *======================*/
NODE
copy_nodes (NODE node,
            BOOLEAN kids,
            BOOLEAN sibs)
{
	NODE new, kin;
	if (!node) return NULL;
	new = copy_node(node);
	if (kids && nchild(node)) {
		kin = copy_nodes(nchild(node), TRUE, TRUE);
		ASSERT(kin);
		nchild(new) = kin;
		while (kin) {
			nparent(kin) = new;
			kin = nsibling(kin);
		}
	}
	if (sibs && nsibling(node)) {
		kin = copy_nodes(nsibling(node), kids, TRUE);
		ASSERT(kin);
		nsibling(new) = kin;
	}
	return new;
}
/*===============================================================
 * traverse_nodes -- Traverse nodes in tree while doing something
 * NODE node:    root of tree to traverse
 * func:         function to call at each node (returns FALSE to stop traversal)
 * param:        opaque pointer for client use, passed thru to callback
 *=============================================================*/
BOOLEAN
traverse_nodes (NODE node, BOOLEAN (*func)(NODE, VPTR), VPTR param)
{
	while (node) {
		travlineno++;
		if (!(*func)(node, param)) return FALSE;
		if (nchild(node)) {
			if (!traverse_nodes(nchild(node), func, param))
				return FALSE;
		}
		node = nsibling(node);
	}
	return TRUE;
}
/*==================================================
 * num_spouses_of_indi -- Returns number of spouses of person
 *================================================*/
INT
num_spouses_of_indi (NODE indi)
{
	INT nsp;
	if (!indi) return 0;
	FORSPOUSES(indi, spouse, fam, nsp) ENDSPOUSES
	return nsp;
}
/*===================================================
 * find_node -- Find node with specific tag and value
 *
 * NODE prnt:   [in] parent node
 * STRING tag:  [in] tag, may be NULL
 * STRING val:  [in] value, may be NULL
 * NODE *plast: [out] previous node, may be NULL
 *=================================================*/
NODE
find_node (NODE prnt, STRING tag, STRING val, NODE *plast)
{
	NODE last, node;

	if (plast) *plast = NULL;
	if (!prnt || (!tag && !val)) return NULL;
	for (last = NULL, node = nchild(prnt); node;
	     last = node, node = nsibling(node)) {
		if (tag && nestr(tag, ntag(node))) continue;
		if (val && nestr(val, nval(node))) continue;
		if (plast) *plast = last;
		return node;
	}
	return NULL;
}

#ifdef UNUSED_CODE
/*=======================================================================
 * father_nodes -- Given list of FAMS or FAMC nodes, returns list of HUSB
 *   lines they contain
 * UNUSED CODE
 *=====================================================================*/
NODE
father_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, husb, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
/*=======================================================================
 * mother_nodes -- Given list of FAMS or FAMC nodes, returns list of WIFE
 *   lines they contain
 *=====================================================================*/
NODE
mother_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, wife, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
/*=========================================================================
 * children_nodes -- Given list of FAMS or FAMC nodes, returns list of CHIL
 *   lines they contain
 *=======================================================================*/
NODE
children_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, chil, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
/*========================================================================
 * parents_nodes -- Given list of FAMS or FAMC nodes, returns list of HUSB
 *   and WIFE lines they contain
 *======================================================================*/
NODE
parents_nodes (NODE faml)      /* list of FAMC and/or FAMS nodes */
{
	NODE fam, refn, husb, wife, chil, rest;
	NODE old = NULL, new = NULL;
	while (faml) {
		ASSERT(eqstr("FAMC", ntag(faml)) || eqstr("FAMS", ntag(faml)));
		ASSERT(fam = key_to_fam(rmvat(nval(faml))));
		split_fam(fam, &refn, &husb, &wife, &chil, &rest);
		new = union_nodes(old, husb, FALSE, TRUE);
		free_nodes(old);
		old = new;
		new = union_nodes(old, wife, FALSE, TRUE);
		free_nodes(old);
		old = new;
		join_fam(fam, refn, husb, wife, chil, rest);
		faml = nsibling(faml);
	}
	return new;
}
#endif /* UNUSED_CODE */
