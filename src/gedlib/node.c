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
#include "liflines.h"
#include "screen.h"

INT lineno = 0;

static BOOLEAN buffer_to_line(STRING, INT*, STRING*, STRING*, STRING*, STRING*);
#ifdef UNUSED_CODE
static BOOLEAN all_digits (STRING);
#endif

STRING fileof = (STRING) "The file is as positioned at EOF.";
STRING reremp = (STRING) "Line %d: This line is empty; EOF?";
STRING rerlng = (STRING) "Line %d: This line is too long.";
STRING rernlv = (STRING) "Line %d: This line has no level number.";
STRING rerinc = (STRING) "Line %d: This line is incomplete.";
STRING rerbln = (STRING) "Line %d: This line has a bad link.";
STRING rernwt = (STRING) "Line %d: This line needs white space before tag.";
STRING rerilv = (STRING) "Line %d: This line has an illegal level.";
STRING rerwlv = (STRING) "The record begins at wrong level.";

/*==============================
 * fixup -- Save non-tag strings
 *============================*/
STRING
fixup (STRING str)
{
	if (!str || *str == 0) return NULL;
	return strsave(str);
}
/*=============================
 * fixtag -- Keep tags in table
 *===========================*/
STRING
fixtag (STRING tag)
{
	STRING str;
	if ((str = (STRING) valueof(tagtable, tag))) return str;
	str = strsave(tag);
	insert_table(tagtable, str, str);
	return str;
}
/*=====================================
 * alloc_node -- Special node allocator
 *===================================*/
typedef struct blck *ALLOC;
struct blck { ALLOC next; };
static ALLOC first_blck = (ALLOC) 0;
static NODE
alloc_node (void)
{
	NODE node;
	ALLOC blck;
	int i;
	if (first_blck == (ALLOC) 0) {
		node = (NODE) stdalloc(100*sizeof(*node));
		first_blck = (ALLOC) node;
		for (i = 1; i <= 99; i++) {
			blck = (ALLOC) node;
			blck->next = (ALLOC) (node + 1);
			node++;
		}
		((ALLOC) node)->next = (ALLOC) 0;
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
	((ALLOC) node)->next = first_blck;
	first_blck = (ALLOC) node;
}
/*===========================
 * create_node -- Create NODE
 *=========================*/
NODE
create_node (STRING xref,
             STRING tag,
             STRING val,
             NODE prnt)
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
 * free_nod0 -- Free a nod0 structure
 *=================================*/
void
free_nod0 (NOD0 nod0)
{
	free_nodes(nod0->top);
	stdfree(nod0);
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
		if (nxref(node)) stdfree(nxref(node));
		if (nval(node)) stdfree(nval(node));
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
		lineno++;
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
 *============================================*/
BOOLEAN
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
 *==============================================================*/
static BOOLEAN
buffer_to_line (STRING p,
                INT *plev,
                STRING *pxref,
                STRING *ptag,
                STRING *pval,
                STRING *pmsg)
{
	INT lev;
	extern INT lineno;
	static unsigned char scratch[MAXLINELEN+40];

	*pmsg = *pxref = *pval = 0;
	if (!p || *p == 0) {
		sprintf(scratch, reremp, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	striptrail(p);
	if (strlen(p) > MAXLINELEN) {
		sprintf(scratch, rerlng, lineno);
		*pmsg = scratch;
		return ERROR;
	}

/* Get level number */
	while (iswhite(*p)) p++;
	if (chartype(*p) != DIGIT) {
		sprintf(scratch, rernlv, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	lev = *p++ - '0';
	while (chartype(*p) == DIGIT)
		lev = lev*10 + *p++ - '0';
	*plev = lev;

/* Get cross reference, if there */
	while (iswhite(*p)) p++;
	if (*p == 0) {
		sprintf(scratch, rerinc, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	if (*p != '@') goto gettag;
	*pxref = p++;
	if (*p == '@') {
		sprintf(scratch, rerbln, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	while (*p != '@') p++;
	p++;
	if (*p == 0) {
		sprintf(scratch, rerinc, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	if (!iswhite(*p)) {
		sprintf(scratch, rernwt, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	*p++ = 0;

/* Get tag field */
gettag:
	while (iswhite(*p)) p++;
	if ((INT) *p == 0) {
		sprintf(scratch, rerinc, lineno);
		*pmsg = scratch;
		return ERROR;
	}
	*ptag = p++;
	while (!iswhite(*p) && *p != 0) p++;
	if (*p == 0) return OKAY;
	*p++ = 0;

/* Get the value field */
	while (iswhite(*p)) p++;
	*pval = p;
	return OKAY;
}
/*=================================================
 * file_to_node -- Convert GEDCOM file to NODE tree
 *===============================================*/
NODE
file_to_node (STRING fname,     /* name of file that holds GEDCOM record */
              TRANTABLE tt,     /* character translation table */
              STRING *pmsg,     /* possible error message */
              BOOLEAN *pemp)    /* set true if file is empty */
{
	FILE *fp;
	NODE node;
	static unsigned char scratch[100];
	*pmsg = NULL;
	*pemp = FALSE;
	if (!(fp = fopen(fname, LLREADTEXT))) {
		sprintf(scratch, "Could not open file %s",  fname);
		*pmsg = scratch;
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
 *==============================================================*/
NODE
first_fp_to_node (FILE *fp,     /* file that holds GEDCOM record/s */
                  BOOLEAN list, /* can be list at level 0? */
                  TRANTABLE tt, /* character translation table */
                  STRING *pmsg, /* possible error message */
                  BOOLEAN *peof)/* set true if file is at end of file */
{
	INT rc;
	ateof = FALSE;
	lineno = 0;
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
 * next_fp_to_node -- Convert next GEDCOM record in file to tree
 *============================================================*/
NODE
next_fp_to_node (FILE *fp,       /* file that holds GEDCOM record/s */
                 BOOLEAN list,   /* can be list at level 0? */
                 TRANTABLE tt,   /* character translation table */
                 STRING *pmsg,   /* possible error message */
                 BOOLEAN *peof)  /* set true if file is at end of file */
{
	INT curlev, bcode, rc;
	NODE root, node, curnode;
	static unsigned char scratch[100];
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
				sprintf(scratch, rerilv, lineno);
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
			sprintf(scratch, rerilv, lineno);
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
/*========================================
 * string_to_nod0 -- Read nod0 from string
 *======================================*/
NOD0
string_to_nod0 (STRING str, STRING key)
{
	NOD0 nod0 = (NOD0)stdalloc(sizeof(*nod0));
	nod0->keynum = atoi(key+1);
	nod0->ntype = key[0];
	if (*str == '0')
		nod0->top = string_to_node(str);
	else {
		ASSERT(*str == 'Q');
		ASSERT(0); /* not written yet - metadata processing */
	}
	return nod0;
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
	lineno = 0;
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
				    lineno);
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
			llwprintf("Error: line %d: illegal level", lineno);
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
 *======================================*/
void
write_node (INT levl,       /* level */
            FILE *fp,       /* file */
            TRANTABLE tt,   /* char map */
            NODE node,      /* node */
            BOOLEAN indent) /* indent? */
{
	unsigned char out[MAXLINELEN+1];
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
STRING
swrite_node (INT levl,       /* level */
             NODE node,      /* node */
             STRING p)       /* write string */
{
	unsigned char scratch[600];
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
STRING
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
INT
node_strlen (INT levl,       /* level */
             NODE node)      /* node */
{
	INT len;
	unsigned char scratch[10];
	sprintf(scratch, "%d", levl);
	len = strlen(scratch) + 1;
	if (nxref(node)) len += strlen(nxref(node)) + 1;
	len += strlen(ntag(node));
	if (nval(node)) len += strlen(nval(node)) + 1;
	return len + 1;
}
/*==========================================
 * indi_to_dbase -- Store person in database
 *========================================*/
void
indi_to_dbase (NODE node)
{
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
 * fam_to_last_chil -- Return first child of family
 *===============================================*/
NODE
fam_to_last_chil (NODE node)
{
	NODE prev = NULL;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), "CHIL"))) return NULL;
	while (node) {
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
	NODE fam, prev, chil;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	prev = NULL;
	chil = CHIL(fam);
	while (chil) {
		if (eqstr(nxref(indi), nval(chil))) {
			if (!prev) return NULL;
			return key_to_indi(rmvat(nval(prev)));
		}
		prev = chil;
		chil = nsibling(chil);
	}
	return NULL;
}
/*==============================================
 * indi_to_next_sib -- Return next sib of person
 *============================================*/
NODE
indi_to_next_sib (NODE indi)
{
	NODE fam, chil;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	chil = CHIL(fam);
	while (chil) {
		if (eqstr(nxref(indi), nval(chil))) {
			chil = nsibling(chil);
			if (!chil) return NULL;
			return key_to_indi(rmvat(nval(chil)));
		}
		chil = nsibling(chil);
	}
	return NULL;
}
/*======================================
 * indi_to_name -- Return name of person
 *====================================*/
STRING
indi_to_name (NODE node,
              TRANTABLE tt,
              INT len)
{
	if (!node) return (STRING) "NO NAME";
	if (!(node = find_tag(nchild(node), "NAME"))) return (STRING) "NO NAME";
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
	static unsigned char scratch[MAXNAMELEN+1];
	STRING refn;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag))) return NULL;
	refn = nval(node);
	if (len > sizeof(scratch)-1) len = sizeof(scratch)-1;
	translate_string(tt, refn, scratch, sizeof(scratch));
	return scratch;
}
/*==============================================
 * indi_to_event -- Convert event tree to string
 *============================================*/
STRING
indi_to_event (NODE node,
               TRANTABLE tt,
               STRING tag,
               STRING head,
               INT len,
               BOOLEAN shrt)
{
	static unsigned char scratch[200];
	STRING event;
	INT n;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag))) return NULL;
	event = event_to_string(node, tt, shrt);
	if (!event) return NULL;
	sprintf(scratch, "%s%s", head, event);
	n = strlen(scratch);
	if (scratch[n-1] != '.') {
		scratch[n] = '.';
		scratch[++n] = 0;
	}
	if (n > len)  scratch[len] = 0;
	return scratch;
}
/*===========================================
 * event_to_string -- Convert event to string
 *=========================================*/
STRING
event_to_string (NODE node,
                 TRANTABLE tt,
                 BOOLEAN shrt)
{
	static unsigned char scratch1[MAXLINELEN+1];
	static unsigned char scratch2[MAXLINELEN+1];
	STRING date, plac, p;
	date = plac = NULL;
	if (!node) return NULL;
	node = nchild(node);
	while (node) {
		if (eqstr("DATE", ntag(node)) && !date) date = nval(node);
		if (eqstr("PLAC", ntag(node)) && !plac) plac = nval(node);
		node = nsibling(node);
	}
	if (!date && !plac) return NULL;
	if (shrt) {
		date = shorten_date(date);
		plac = shorten_plac(plac);
		if (!date && !plac) return NULL;
	}
	p = scratch1;
	if (date && !plac) {
		strcpy(p, date);
		p += strlen(p);
	}
	if (date && plac) {
		strcpy(p, date);
		p += strlen(p);
		strcpy(p, ", ");
		p += 2;
		strcpy(p, plac);
		p += strlen(p);
	}
	if (!date && plac) {
		strcpy(p, plac);
		p += strlen(p);
	}
	*p = 0;
	translate_string(tt, scratch1, scratch2, MAXLINELEN);
	return scratch2;
}
/*=======================================
 * event_to_date -- Convert event to date
 *=====================================*/
STRING
event_to_date (NODE node,
               TRANTABLE tt,
               BOOLEAN shrt)
{
	static unsigned char scratch[MAXLINELEN+1];
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
 *==============================================*/
STRING
shorten_date (STRING date)
{
	static unsigned char buffer[3][MAXLINELEN+1];
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
		while ((c = *p++) && chartype(c) != DIGIT)
			;
		if (c == 0) return NULL;
		q = buffer[dex];
		*q++ = c;
		len = 1;
		while ((c = *p++) && chartype(c) == DIGIT) {
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
	if ((val = (STRING) valueof(placabbvs, plac))) return val;
	return plac;
}

#ifdef UNUSED_CODE
/*============================================
 * all_digits -- Check if string is all digits
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
#endif

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
 *=============================================================*/
INT tlineno;
BOOLEAN
traverse_nodes (NODE node,              /* root of tree to traverse */
                BOOLEAN (*func)(NODE))  /* function to call at each node */
{
	while (node) {
		tlineno++;
		if (!(*func)(node)) return FALSE;
		if (nchild(node)) {
			if (!traverse_nodes(nchild(node), func))
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
/*==================================================
 * num_fams_of_indi -- Returns number of families of person
 *================================================*/
INT
num_fams_of_indi (NODE indi)
{
	INT num;
	if (!indi) return 0;
	FORFAMSS(indi, fam, spouse, num) ENDFAMSS
	return num;
}
/*===================================================
 * find_node -- Find node with specific tag and value
 *=================================================*/
NODE
find_node (NODE prnt,      /* parent node */
           STRING tag,     /* tag, may be NULL */
           STRING val,     /* value, may be NULL */
           NODE *plast)    /* previous node, may be NULL */
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
/*=======================================================================
 * father_nodes -- Given list of FAMS or FAMC nodes, returns list of HUSB
 *   lines they contain
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
