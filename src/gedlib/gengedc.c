/* 
   Copyright (c) 2000-2001 Perry Rapp

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
/*================================================================
 * gengedc.c -- Generate GEDCOM output from a sequence
 * Copyright(c) 2000-2001 by Perry Rapp; all rights reserved
 * This is a complete rewrite of the original gen_gedcom
 *   Created: 2000/10
 *==============================================================*/

#include "sys_inc.h"
#include <string.h>
#include "llstdlib.h"
#include "table.h"
#include "gedcom.h"
#include "indiseq.h"
#include "interp.h"
#include "gengedc.h"

/**
 closure holds data used to make output consistent 
 as nodes are output, they are tested, and put on closure list
 if required - at the end, the closure list is processed
 (GENGENDCOM_WEAK does not accumulate closure list, but
  modifies values being output instead)
*/
typedef struct closure_s
{ 
	int gengedcl;
	TABLE tab; /* table of nodes in seq */
	INDISEQ seq; /* (top-level) nodes to process */
	INDISEQ outseq; /* (top-level) nodes to output */
	/* filter criteria would be added here */
} CLOSURE;
/*======================================================
 * closure_has_key -- does closure have this key ?
 * the table stores all the keys we have
 * regardless of their status:
 *    in the seq list meaning to-process
 *    not in our lists if in-process (client uses temp list)
 *    in the output list
 *====================================================*/
static BOOLEAN
closure_has_key (CLOSURE * closure, STRING key)
{
	return in_table(closure->tab, key);
}
/*======================================================
 * closure_add_key -- add a (top-level) node to the closure
 * we store the key (eg, "I1") and also the tag (eg, "INDI")
 * both tab and seq get their own allocs of the key
 * neither gets an alloc of the tag
 *====================================================*/
static void
closure_add_key (CLOSURE * closure, STRING key, STRING tag)
{
	if (in_table(closure->tab, key)) return;

	/* filter for invalid pointers */
	if (!key_to_type(key, TRUE))
		return;

	/* we'll store the tag as the value 
	 during gengedcom, all append_indiseqs alloc their own keys & vals (tags) */
	append_indiseq_sval(closure->seq, strsave(key), NULL, strsave(tag),
	    TRUE, TRUE);
	/* during gengedcom, all tables alloc their own keys */
	insert_table(closure->tab, strsave(key), NULL);
}

#if 0

Unused code as of 2-Jan-2001

/*======================================================
 * closure_add_node -- add a (top-level) node to the closure
 * a top-level node will, of course, be identified by its xref
 *====================================================*/
static void
closure_add_node (CLOSURE * closure, NODE node)
{
	closure_add_key(closure, rmvat(nxref(node)), ntag(node));
}
#endif
/*======================================================
 * closure_add_output_node -- add a (top-level) node to the output list
 * the node should come from the processing list originally, and
 * therefore already be in the table
 *====================================================*/
static void
closure_add_output_node (CLOSURE * closure, NODE node)
{
	STRING key=rmvat(nxref(node));
	STRING tag=ntag(node);
	/* during gengedcom, all append_indiseqs alloc their own keys & vals (tags) */
	append_indiseq_sval(closure->outseq, strsave(key), NULL, strsave(tag),
	    TRUE, TRUE);
}
/*======================================================
 * closure_init -- allocate everything in closure
 *====================================================*/
static void
closure_init (CLOSURE * closure, int gengedcl)
{
	closure->tab = create_table();
	closure->seq = create_indiseq_sval();
	closure->outseq = create_indiseq_sval();
	closure->gengedcl = gengedcl;
}
/*======================================================
 * closure_free -- free everything in closure
 *====================================================*/
static void
closure_free (CLOSURE * closure)
{
	/* during gengedcom, all indiseqs alloc their own keys & vals */
	remove_indiseq(closure->seq, TRUE);
	remove_indiseq(closure->outseq, TRUE);
	remove_table(closure->tab, FREEKEY);
}
/*======================================================
 * closure_wipe_processlist -- empty the "to-process" list
 *====================================================*/
static void
closure_wipe_processlist (CLOSURE * closure)
{
	remove_indiseq(closure->seq, FALSE);
	closure->seq = create_indiseq_sval();
}
/*======================================================
 * closure_is_original -- is the closure in original mode ?
 * original mode is for backwards compatiblity - S,E, & X
 * pointers are output unmodified, but none of their records
 * are output - this yields an inconsistent output 
 *====================================================*/
static BOOLEAN
closure_is_original (CLOSURE * closure)
{
	return closure->gengedcl==GENGEDCOM_ORIGINAL;
}
/*=========================================================
 * closure_is_strong -- is the closure in strong mode ?
 * strong mode means include all top-level nodes referenced
 * by pointers (of S,E, & X type)
 *=======================================================*/
static BOOLEAN
closure_is_strong (CLOSURE * closure)
{
	return closure->gengedcl==GENGEDCOM_STRONG_TRIM
		|| closure->gengedcl==GENGEDCOM_STRONG_DUMP;
}
/*=================================================
 * closure_is_dump -- is the closure in dump mode ?
 * dump mode means skip nodes with invalid pointers
 *===============================================*/
static BOOLEAN
closure_is_dump (CLOSURE * closure)
{
	return closure->gengedcl==GENGEDCOM_WEAK_DUMP
		|| closure->gengedcl==GENGEDCOM_STRONG_DUMP;
}
/*============================================================
 * process_node_value -- process the node's value for pointers
 * add any pointers in the value to the closure
 * this should only be called in strong mode
 *==========================================================*/
static void
process_node_value (CLOSURE * closure, STRING v)
{
	for ( ; *v; v++)
	{
		if (v[0]=='@' && v[1] && v[2])
		{
			int skeynum;
			char skeybuff[20]; /* what should this be ? max key size */
			skeynum = atoi(&v[2]);
			sprintf(skeybuff, "%c%d", v[1], skeynum);
			if (v[1]=='S')
				closure_add_key(closure, skeybuff, "SOUR");
			else if (v[1]=='E')
				closure_add_key(closure, skeybuff, "EVEN");
			else if (v[1]=='X')
				closure_add_key(closure, skeybuff, "XXXX");
		}
	}
}
/*==============================================
 * output_any_node -- send a node out
 * this is the only path to output for gengedcom
 * this filters for dumping & trimming options
 *============================================*/
static void
output_any_node (CLOSURE * closure, NODE node, STRING toptag, INT lvl)
{
	char newval[MAXNAMELEN]; /* for modified values */
	STRING v,pv;
	BOOLEAN dump, trimmed, insidepointer;
	v=nval(node);
	dump=0;
	trimmed=0;
	insidepointer=0;

	newval[0]=0;
	if (v)
	{
		pv=newval;
		while (*v)
		{
			int ispointer=0;
			if (insidepointer && v[0]=='@')
				insidepointer=0;
			else if (v[0]=='@' && v[1] && v[2])
			{
				if (closure_is_original(closure))
				{ /* original only suppressed family linking pointers */
					if (eqstr(toptag, "INDI"))
					{
						if (eqstr(ntag(node),"FAMC")||eqstr(ntag(node),"FAMS"))
							ispointer=1;
					}
					else
					{
						if (eqstr(ntag(node),"CHILD")||eqstr(ntag(node),"HUSB")||eqstr(ntag(node),"WIFE"))
							ispointer=1;
					}
				}
				else
					ispointer=1;
			}
			if (ispointer)
			{
				int skeynum;
				char skeybuff[32]; /* what should this be ? max key size */
				skeynum = atoi(&v[2]);
				sprintf(skeybuff, "%c%d", v[1], skeynum);
				if (!closure_has_key(closure, skeybuff))
				{
					if (closure_is_dump(closure))
					{
						dump=1;
						break;
					}
					trimmed=1;
					/* skip over pointer */
					for (v++; *v; v++)
					{
						if (*v == '@')
							break;
					}
					v++; /* skip over last @ */
				}
				else /* copy included pointers */
				{
					insidepointer=1;
					*pv++ = *v++;
				}

			}
			else /* copy normal data */
				*pv++ = *v++;
		}
		if (!dump)
			*pv = 0;
	}

	if (trimmed && !newval[0])
		dump=1; /* dump nodes whose values were trimmed to nothing */

	if (!dump)
	{
		char unsigned scratch[MAXLINELEN+1];
		STRING pq = scratch;
		sprintf(pq, "%d", lvl);
		pq += strlen(pq);
		if (nxref(node)) {
			sprintf(pq, " %s", nxref(node));
			pq += strlen(pq);
		}
		sprintf(pq, " %s", ntag(node));
		pq += strlen(pq);
		if (newval[0])
		{
			strcpy(pq, " ");
			pq++;
			strcpy(pq, newval);
			pq += strlen(pq);
		}
		sprintf(pq, "\n");
		poutput(scratch);
		
		if (nchild(node))
			output_any_node(closure, nchild(node), toptag, lvl+1);
	}

	if (nsibling(node))
		output_any_node(closure, nsibling(node), toptag, lvl);
}
/*===================================================
 * output_top_node -- put a node onto the output list
 *=================================================*/
static void
output_top_node (CLOSURE * closure, NODE node)
{
	output_any_node(closure, node, ntag(node), 0);
}
/*========================================
 * process_any_node -- filter for pointers
 *=======================================*/
static void
process_any_node (CLOSURE * closure, NODE node)
{
	STRING v;
	v = nval(node);

	if (!closure_is_strong(closure))
		return;

	if (v && strchr(v, '@'))
		process_node_value(closure, v);

	if (nchild(node))
		process_any_node(closure, nchild(node));

	if (nsibling(node))
		process_any_node(closure, nsibling(node));
}
/*====================================================
 * table_incr_item -- increment value of item in table
 *  (or add with 1 value)
 *==================================================*/
static void
table_incr_item (TABLE tab, STRING key)
{
	INT * value;
	value = (INT *)access_value(tab, key);
	if (value)
		(*value)++;
	else
		/* during gengedcom, all tables alloc their own keys */
		insert_table(tab, strsave(key), (VPTR)1);
}
/*===================================================================
 * add_refd_fams -- add all families in table with #refs>1 to closure
 *  this is a callback from traverse_table_param
 *=================================================================*/
static int
add_refd_fams (ENTRY ent, VPTR param)
{
	CLOSURE * closure = (CLOSURE *)param;
	if ((INT)ent->evalue > 1)
		closure_add_key(closure, ent->ekey, "FAM");
	return 1;
}
/*===================================================================
 * gen_gedcom -- Generate GEDCOM file from sequence; only persons in
 *   sequence are in file; families that at least two persons in
 *   sequence refer to are also in file; other persons referred to by
 *   families are not included
 * this is a rewrite by Perry Rapp 2000/10
 *  of the orginal (by the same name) by Tom Wetmore
 *  this version handles sources (in strong mode - see gengedcl modes)
 *=================================================================*/
void
gen_gedcom (INDISEQ seq, int gengedcl)
{
	INT num1;
	NODE indi, famc;
	NODE node;
	CLOSURE closure;
	INDISEQ tempseq;
	TABLE fams; /* all families referenced - value is #references */
	if (!seq) return;

	closure_init(&closure, gengedcl);

	/* must load closure with all indis first
	 for succeeding logic to pick out what families to include */
	FORINDISEQ(seq, el, num)
		closure_add_key(&closure, skey(el), "INDI");
	ENDINDISEQ
	/* now go thru all indis and figure out which
	families to keep */

	fams = create_table();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(skey(el));
		famc = indi_to_famc(indi);
		if (famc)
			table_incr_item(fams, fam_to_key(famc));
		FORFAMSS(indi, fam, spouse, num1)
			table_incr_item(fams, fam_to_key(fam));
		ENDFAMSS
	ENDINDISEQ

	traverse_table_param(fams, &add_refd_fams, &closure);
	remove_table(fams, FREEKEY);

	/* now we have to process every node, including new
	 ones that get added during processing */
	while (length_indiseq(closure.seq))
	{
		tempseq = create_indiseq_sval();
		/* move all from to-process list to
		temporary processing list, because the
		processing will add stuff to the to-process list */
		FORINDISEQ(closure.seq, el, num)
			/* during gengedcom, all append_indiseqs alloc their own keys & vals (tags) */
			append_indiseq_sval(tempseq, strsave(skey(el)), NULL, strsave(sval(el)),
				 TRUE, TRUE);
		ENDINDISEQ
		/* clear to-process list */
		closure_wipe_processlist(&closure);
		/* cycle thru temp processing list & process each */
		FORINDISEQ(tempseq, el, num)
			/* tag was stored in the value */
			node = key_to_type(skey(el), FALSE);
			process_any_node(&closure, node);
			closure_add_output_node(&closure, node);
		ENDINDISEQ
		remove_indiseq(tempseq, TRUE);
	}
	canonkeysort_indiseq(closure.outseq);
	FORINDISEQ(closure.outseq, el, num)
		node = key_to_type(skey(el), FALSE);
		output_top_node(&closure, node);
	ENDINDISEQ
	closure_free(&closure);
}
