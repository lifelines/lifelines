/* 
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#include "interp.h"	/* for poutput */
#include "gengedc.h"

/*********************************************
 * local types
 *********************************************/

/*======================================================
 * CLOSURE -- holds data used to make output consistent 
 * as nodes are output, they are tested, and put on closure list
 * if required - at the end, the closure list is processed
 * (GENGENDCOM_WEAK does not accumulate closure list, but
  * modifies values being output instead)
 *====================================================*/
typedef struct closure_s
{ 
	int gengedcl;
	TABLE tab; /* table of nodes in seq */
	INDISEQ seq; /* (top-level) nodes to process */
	INDISEQ outseq; /* (top-level) nodes to output */
	/* filter criteria would be added here */
} CLOSURE;

/*********************************************
 * local enums
 *********************************************/

/*********************************************
 * local function prototypes
 *********************************************/

static BOOLEAN closure_has_key(CLOSURE * closure, STRING key);
static void closure_add_key(CLOSURE * closure, CNSTRING key, STRING tag);
static void closure_add_output_node (CLOSURE * closure, NODE node);
static void closure_init(CLOSURE * closure, int gengedcl);
static void closure_free(CLOSURE * closure);
static void closure_wipe_processlist(CLOSURE * closure);
static BOOLEAN closure_is_original(CLOSURE * closure);
static BOOLEAN closure_is_strong(CLOSURE * closure);
static BOOLEAN closure_is_dump(CLOSURE * closure);
static void process_node_value(CLOSURE * closure, STRING v);
static void output_any_node(CLOSURE * closure, NODE node, STRING toptag, INT lvl, BOOLEAN *eflg);
static void output_top_node(CLOSURE * closure, NODE node, BOOLEAN *eflg);
static void process_any_node(CLOSURE * closure, NODE node);


/*********************************************
 * local variables
 *********************************************/

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

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
 * We give seq a new alloc of the key (tab makes its own)
 * neither gets an alloc of the tag
 *====================================================*/
static void
closure_add_key (CLOSURE * closure, CNSTRING key, STRING tag)
{
	if (in_table(closure->tab, key)) return;

	/* filter for invalid pointers */
	if (!key_to_type(key, TRUE))
		return;

	/* we'll store the tag as the value 
	 during gengedcom, all append_indiseqs alloc their own keys & vals (tags) */
	append_indiseq_sval(closure->seq, strsave(key), NULL, strsave(tag),
	    TRUE, TRUE);
	/* generic tables handle their own allocation of keys & values */
	/* we don't use the value at all, only the fact that the
	key is present */
	insert_table_int(closure->tab, key, 1);
}
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
	closure->tab = create_table_int();
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
	remove_indiseq(closure->seq);
	closure->seq=NULL;
	remove_indiseq(closure->outseq);
	closure->outseq=NULL;
	destroy_table(closure->tab);
	closure->tab=NULL;
}
/*======================================================
 * closure_wipe_processlist -- empty the "to-process" list
 *====================================================*/
static void
closure_wipe_processlist (CLOSURE * closure)
{
	remove_indiseq(closure->seq);
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
 * is_valid_key - verify that key points to a valid lifelines
 * gedcom xref - @[IFSEX][1-9][0-9]*@
 * note that number is not zero filled
 * with @'s length of key can be MAXKEYWIDTH+2
 *==========================================================*/
static int 
is_valid_key(CNSTRING key) {
    CNSTRING ptr = key;
    if (!ptr) return FALSE;
    if (*ptr++ != '@') return FALSE;
    if (*ptr != 'I' && *ptr != 'F' && *ptr != 'S' 
		&& *ptr != 'E' && *ptr != 'X') return FALSE;
    ptr++;
    if (*ptr < '1' || *ptr > '9') return FALSE;
    for (++ptr; isdigit(*ptr); ++ptr) ;
    if (ptr > key+MAXKEYWIDTH+1) return FALSE;
    if (*ptr != '@') return FALSE;
    return TRUE;
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
			char skeybuff[MAXKEYWIDTH+1];
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
output_any_node (CLOSURE * closure, NODE node, STRING toptag
	, INT lvl, BOOLEAN *eflg)
{
	char newval[MAXGEDNAMELEN]; /* for modified values */
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
			else if (v[0]=='@' && is_valid_key(v))
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
				char skeybuff[MAXKEYWIDTH+1];
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
		char scratch[MAXLINELEN+1];
		STRING pq = scratch;
		sprintf(pq, "%ld", lvl);
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
		poutput(scratch, eflg);
		if (*eflg)
			return;
		
		if (nchild(node)) {
			output_any_node(closure, nchild(node), toptag, lvl+1, eflg);
			if (*eflg)
				return;
			}
	}

	if (nsibling(node))
		output_any_node(closure, nsibling(node), toptag, lvl, eflg);
}
/*===================================================
 * output_top_node -- put a node onto the output list
 *=================================================*/
static void
output_top_node (CLOSURE * closure, NODE node, BOOLEAN *eflg)
{
	output_any_node(closure, node, ntag(node), 0, eflg);
}
/*========================================
 * process_any_node -- filter for pointers
 *=======================================*/
static void
process_any_node (CLOSURE * closure, NODE node)
{
	STRING v;
	CNSTRING w;
	v = nval(node);

	if (!closure_is_strong(closure))
		return;

	if (v && (w=strchr(v, '@')) && is_valid_key(w))
		process_node_value(closure, v);

	if (nchild(node))
		process_any_node(closure, nchild(node));

	if (nsibling(node))
		process_any_node(closure, nsibling(node));
}
/*===================================================================
 * gen_gedcom -- Generate GEDCOM file from sequence; only persons in
 *   sequence are in file; families that at least two persons in
 *   sequence refer to are also in file; other persons referred to by
 *   families are not included
 * this is a rewrite by Perry Rapp 2000/10
 *  of the orginal (by the same name) by Tom Wetmore
 *  this version handles sources (in strong mode - see gengedcl modes)
 * gengedcl tells which type of closure to make:
 *  GENGEDCOM_ORIGINAL: output broken references (to SOUR, NOTE, etc)
 *  GENGEDCOM_WEAK_DUMP: filter out references (to SOUR, NOTE, etc)
 *  GENGEDCOM_STRONG_DUMP: output referenced records (SOUR, NOTE etc)
 *=================================================================*/
void
gen_gedcom (INDISEQ seq, int gengedcl, BOOLEAN * eflg)
{
	INT num1=0;
	NODE indi=0, famc=0;
	NODE node=0;
	CLOSURE closure;
	INDISEQ tempseq=0;
	TABLE famstab=0; /* all families referenced - value is #references */
	if (!seq) return;

	closure_init(&closure, gengedcl);

	/* must load closure with all indis first
	 for succeeding logic to pick out what families to include */
	FORINDISEQ(seq, el, num)
		closure_add_key(&closure, element_skey(el), "INDI");
	ENDINDISEQ
	/* now go thru all indis and figure out which
	families to keep */

	famstab = create_table_int();
	FORINDISEQ(seq, el, num)
		indi = key_to_indi(element_skey(el));
		famc = indi_to_famc(indi);
		if (famc)
			increment_table_int(famstab, fam_to_key(famc));
		FORFAMS(indi, fam, num1)
			increment_table_int(famstab, fam_to_key(fam));
		ENDFAMS
	ENDINDISEQ

	/* add all families in table with #refs>1 to closure */
	if (TRUE) {
		TABLE_ITER tabit = begin_table_iter(famstab);
		CNSTRING key=0;
		INT count=0;
		while (next_table_int(tabit, &key, &count)) {
			if (count > 1) {
				closure_add_key(&closure, key, "FAM");
			}
		}
		end_table_iter(&tabit);
		destroy_table(famstab);
		famstab=0;
	}

	/* now we have to process every node, including new
	 ones that get added during processing */
	while (length_indiseq(closure.seq))
	{
		CNSTRING sval=0, skey=0;
		tempseq = create_indiseq_sval();
		/* move all from to-process list to
		temporary processing list, because the
		processing will add stuff to the to-process list */
		FORINDISEQ(closure.seq, el, num)
			sval = element_sval(el);
			skey = element_skey(el);
			/* during gengedcom, all append_indiseqs alloc their own keys & vals (tags) */
			append_indiseq_sval(tempseq, strsave(skey), NULL, strsave(sval),
				 TRUE, TRUE);
		ENDINDISEQ
		/* clear to-process list */
		closure_wipe_processlist(&closure);
		/* cycle thru temp processing list & process each */
		FORINDISEQ(tempseq, el, num)
			skey = element_skey(el);
			/* tag was stored in the value */
			node = key_to_type(skey, FALSE);
			process_any_node(&closure, node);
			closure_add_output_node(&closure, node);
		ENDINDISEQ
		remove_indiseq(tempseq);
		tempseq=NULL;
	}
	canonkeysort_indiseq(closure.outseq);
	FORINDISEQ(closure.outseq, el, num)
		node = key_to_type(element_skey(el), FALSE);
		output_top_node(&closure, node, eflg);
		if (*eflg) {
			closure_free(&closure);
			return;
		}
	ENDINDISEQ
	closure_free(&closure);
}
