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
 * interp.c -- Interpret program statements
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 16 Aug 93
 *   3.0.0 - 26 Jul 94    3.0.2 - 25 Mar 95
 *   3.0.3 - 22 Sep 95
 *===========================================================*/

#include "sys_inc.h"
#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "cache.h"
#include "interp.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"
#include "arch.h"

extern STRING llprograms;
extern BOOLEAN progrunning;

STRING Pfname = NULL;	/* file to read program from */
STRING progname = NULL;	/* starting program name */
FILE *Pinfp  = NULL;	/* file to read program from */
FILE *Poutfp = NULL;	/* file to write program output to */
STRING Pinstr = NULL;	/* string to read program from */
STRING Poutstr = NULL;	/* string to write program output to */
INT Plineno = 1;
INT Perrors = 0;
LIST Plist;		/* list of program files still to read */
PNODE Pnode = NULL;	/* node being interpreted */

TABLE filetab, proctab, globtab, functab;
STRING ierror = (STRING) "Error: file \"%s\": line %d: ";
static STRING qrptname = (STRING) "What is the name of the program?";

static void remove_tables (void);
static void parse_file (STRING ifile, LIST plist);

/*====================================+
 * initinterp -- Initialize interpreter
 *===================================*/
void
initinterp (void)
{
	initrassa();
	initset();
	Perrors = 0;
}
/*==================================+
 * finishinterp -- Finish interpreter
 *=================================*/
void
finishinterp (void)
{
	finishrassa();
}
/*================================================================+
 * progmessage -- Display a status message about the report program
 *===============================================================*/
void
progmessage(char *msg)
{
    char buf[80];
    int len;
    char *dotdotdot;
    if(progname && *progname) {
	len = strlen(progname);
	if(len > 40) {
	  len -= 40;
	  dotdotdot = "...";
	}
	else {
	  len = 0;
	  dotdotdot = "";
	}
	sprintf(buf, "Program \"%s%s\" %s", dotdotdot, progname+len, msg);
    }
    else {
	sprintf(buf, "The program %s", msg);
    }
    message(buf);
}
/*=============================================+
 * interp_program -- Interpret LifeLines program
 *============================================*/
void
interp_program (STRING proc,    /* proc to call */
                INT nargs,      /* number of arguments */
                WORD *args,     /* arguments */
                INT nifiles,    /* number of program files - can be zero*/
                STRING *ifiles, /* program files */
                STRING ofile)   /* output file - can be NULL */
{
	FILE *fp;
	LIST plist = create_list();
	TABLE stab;
	PVALUE dummy;
	INT i;
	STRING ifile;
	PNODE first, parm;

   /* Get the initial list of program files */

	set_list_type(plist, LISTDOFREE);
	if (nifiles > 0) {
		for (i = 0; i < nifiles; i++) {
			enqueue_list(plist, strsave(ifiles[i]));
		}
	} else {
		ifile = NULL;
		fp = ask_for_program(LLREADTEXT, qrptname, &ifile,
			llprograms, ".ll");
		if (fp == NULL) {
			if (ifile != NULL)  {
				/* tried & failed to open file */
				llwprintf("Error: file \"%s\" not found.\n", ifile);
			}
			return;
		}
		fclose(fp);
		progname = strsave(ifile);

		enqueue_list(plist, strsave(ifile));
	}

   /* Parse each file in the list -- don't reparse any file */

	filetab = create_table();
	proctab = create_table();
	globtab = create_table();
	functab = create_table();
	initinterp();
	while (!empty_list(plist)) {
		ifile = (STRING) dequeue_list(plist);
		if (!in_table(filetab, ifile)) {
			insert_table(filetab, ifile, 0);
#ifdef DEBUG
			llwprintf("About to parse file %s.\n", ifile);
#endif
			parse_file(ifile, plist);
		} else
			stdfree(ifile);
	}
	if (Perrors) {
		progmessage("contains errors.\n");
		return;
	}

   /* Find top procedure */

	if (!(first = (PNODE) valueof(proctab, proc))) {
		progmessage("needs a starting procedure.");
		remove_tables();
		return;
	}

   /* Open output file if name is provided */

	if (ofile && !(Poutfp = fopen(ofile, LLWRITETEXT))) {
		mprintf_error("Error: file \"%s\" could not be created.\n", ofile);
		remove_tables();
		return;
	}
	if (Poutfp) setbuf(Poutfp, NULL);

   /* Link arguments to parameters in symbol table */

	parm = (PNODE) iargs(first);
	if (nargs != num_params(parm)) {
		mprintf_error("Proc %s must be called with %d (not %d) parameters.",
			proc, num_params(parm), nargs);
		remove_tables();
		return;
	}
	stab = create_table();
	for (i = 0; i < nargs; i++) {
		insert_table(stab, iident(parm), args[0]);
		parm = inext(parm);
	}

   /* Interpret top procedure */

	progrunning = TRUE;
	progmessage("is running...");
	switch (interpret((PNODE) ibody(first), stab, &dummy)) {
	case INTOKAY:
	case INTRETURN:
		progmessage("was run successfully.");
		break;
	default:
		progmessage("was not run because of errors.");
		break;
	}

   /* Clean up and return */

	progrunning = FALSE;
	remove_tables();
	remove_table(stab, DONTFREE);
	finishinterp();
	if (Poutfp) fclose(Poutfp);
	Pinfp = Poutfp = NULL;
}
/*===========================================+
 * remove_tables - Remove interpreter's tables
 *==========================================*/
void
remove_tables (void)
{
	remove_table(filetab, FREEKEY);
	remove_table(proctab, DONTFREE);
	remove_table(globtab, DONTFREE);
	remove_table(functab, DONTFREE);
}
/*======================================+
 * parse_file - Parse single program file
 *=====================================*/
void
parse_file (STRING ifile,
            LIST plist)
{
	Pfname = ifile;
	if (!ifile || *ifile == 0) return;
	Plist = plist;
	Pinfp = fopenpath(ifile, LLREADTEXT, llprograms, ".ll", (STRING *)NULL);
	if (!Pinfp) {
		llwprintf("Error: file \"%s\" not found.\n", ifile);
		Perrors++;
		return;
	}
	Plineno = 1;
	yyparse();
	fclose(Pinfp);
}
/*====================================+
 * interp_main -- Interpreter main proc
 *===================================*/
void
interp_main (void)
{
	interp_program("main", 0, NULL, 0, NULL, NULL);
}
/*======================================
 * interpret -- Interpret statement list
 *====================================*/
INTERPTYPE
interpret (PNODE node,          /* first node to interpret */
           TABLE stab,          /* current symbol table */
           PVALUE *pval)        /* possible return value */
{
	STRING str;
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	PVALUE val;

	*pval = NULL;
/*HERE*/
	while (node) {
		Pnode = node;
if (prog_debug) {
	llwprintf("i%di: ", iline(node));
	show_one_pnode(node);
	llwprintf("\n");
}
		switch (itype(node)) {
		case ISCONS:
			poutput(pvalue(ivalue(node)));
			break;
		case IIDENT:
			val = eval_and_coerce(PSTRING, node, stab, &eflg);
			if (eflg) {
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("identifier: %s should be a string\n",
				    iident(node));
				return INTERROR;
			}
			str = (STRING) pvalue(val);
			if (str) poutput(str);
			delete_pvalue(val);
			break;
		case IBCALL:
#ifdef DEBUG
			llwprintf("BCALL: %s\n", iname(node));
#endif
			val = evaluate_func(node, stab, &eflg);
			if (eflg) return INTERROR;
#if 0
			if (eflg) {
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in function: `%s'\n", iname(node));
				return INTERROR;
			}
#endif
			if (!val) break;
			if (ptype(val) == PSTRING && pvalue(val))
				poutput(pvalue(val));
			delete_pvalue(val);
			break;
		case IFCALL:
			val = evaluate_ufunc(node, stab, &eflg);
			if (eflg) return INTERROR;
#if 0
			if (eflg) {
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in function: `%s'\n", iname(node));
				return INTERROR;
			}
#endif
			if (!val) break;
			if (ptype(val) == PSTRING && pvalue(val))
				poutput(pvalue(val));
			delete_pvalue(val);
			break;
		case IPDEFN:
			FATAL();
		case ICHILDREN:
			switch (irc = interp_children(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in children loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case ISPOUSES:
			switch (irc = interp_spouses(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in spouses loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IFAMILIES:
			switch (irc = interp_families(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in families loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IFATHS:
			switch (irc = interp_fathers(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in fathers loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IMOTHS:
			switch (irc = interp_mothers(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in mothers loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IFAMCS:
			switch (irc = interp_parents(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in parents loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case ISET:
			switch (irc = interp_indisetloop(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in indiset loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IINDI:
			switch (irc = interp_forindi(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in forindi loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IFAM:
			switch (irc = interp_forfam(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in forfam loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case ISOUR:
			switch (irc = interp_forsour(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in forsour loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IEVEN:
			switch (irc = interp_foreven(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in foreven loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IOTHR:
			switch (irc = interp_forothr(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				printf("in forothr loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case ILIST:
			switch (irc = interp_forlist(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in forlist loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
                case INOTES:
                        switch (irc = interp_fornotes(node, stab, pval)) {
                        case INTOKAY:
                                break;
                        case INTERROR:
#if 0
                                llwprintf(ierror, ifname(node), iline(node));
                                llwprintf("in fornotes loop\n");
#endif
                                return INTERROR;
                        default:
                                return irc;
                        }
                        break;
		case INODES:
			switch (irc = interp_fornodes(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in fornodes loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case ITRAV:
			switch (irc = interp_traverse(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in traverse loop\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IIF:
			switch (irc = interp_if(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in if statement\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IWHILE:
			switch (irc = interp_while(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in while statement\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IPCALL:
			switch (irc = interp_call(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
#if 0
				llwprintf(ierror, ifname(node), iline(node));
				llwprintf("in procedure call\n");
#endif
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IBREAK:
			return INTBREAK;
		case ICONTINUE:
			return INTCONTINUE;
		case IRETURN:
			if (iargs(node))
				*pval = evaluate(iargs(node), stab, &eflg);
#ifdef DEBUG
				llwprintf("interp return ");
				show_pvalue(*pval);
				llwprintf("\n");
#endif
			return INTRETURN;
		default:
			llwprintf("itype(node) is %d\n", itype(node));
			llwprintf("HUH, HUH, HUH, HUNH!\n");
			return INTERROR;
		}
		node = inext(node);
	}
	return TRUE;
}
/*========================================+
 * interp_children -- Interpret child loop
 *  usage: chidren(INDI,INDI_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_children (PNODE node,
                 TABLE stab,
                 PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nchil;
	CACHEEL fcel, cel;
	INTERPTYPE irc;
	NODE fam = (NODE) eval_fam(iloopexp(node), stab, &eflg, &fcel);
	if (eflg) {
		prog_error(node, "1st arg to children must be a family");
		return INTERROR;
	}
	if (fam && nestr(ntag(fam), "FAM")) {
		prog_error(node, "1st arg to children has a major error");
		return INTERROR;
	}
	if (!fam) return INTOKAY;
	lock_cache(fcel);
	FORCHILDREN(fam, chil, nchil)
		insert_pvtable(stab, ichild(node), PINDI,
		    (cel = indi_to_cacheel(chil)));
		insert_pvtable(stab, inum(node), PINT, (WORD)nchil);
		lock_cache(cel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(cel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto a;
		case INTBREAK:
			unlock_cache(fcel);
			return INTOKAY;
		default:
			unlock_cache(fcel);
			return irc;
		}
a:	;
	ENDCHILDREN
	unlock_cache(fcel);
	return INTOKAY;
}
/*==============================================+
 * interp_spouses -- Interpret spouse loop
 *  usage: spouses(INDI,INDI_V,FAM_V,INT_V) {...}
 *=============================================*/
INTERPTYPE
interp_spouses (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nspouses;
	CACHEEL icel, scel, fcel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, "1st arg to spouses must be a person");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, "1st arg to spouses has a major error");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	FORSPOUSES(indi, spouse, fam, nspouses)
		insert_pvtable(stab, ispouse(node), PINDI,
		    (WORD) (scel = indi_to_cacheel(spouse)));
		insert_pvtable(stab, ifamily(node), PFAM,
		    (WORD) (fcel = fam_to_cacheel(fam)));
		lock_cache(scel);
		lock_cache(fcel);
		insert_pvtable(stab, inum(node), PINT, (WORD)nspouses);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(scel);
		unlock_cache(fcel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto b;
		case INTBREAK:
			unlock_cache(icel);
			return INTOKAY;
		default:
			unlock_cache(icel);
			return irc;
		}
b:	;
	ENDSPOUSES
	unlock_cache(icel);
	return INTOKAY;
}
/*===============================================+
 * interp_families -- Interpret family loop
 *  usage: families(INDI,FAM_V,INDI_V,INT_V) {...}
 *==============================================*/
INTERPTYPE
interp_families (PNODE node,
                 TABLE stab,
                 PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, "1st arg to families must be a person");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, "1st arg to families has a major error");
		return INTERROR;
	}
	if (!indi) return INTOKAY;
	lock_cache(icel);
	FORFAMSS(indi, fam, spouse, nfams)
		insert_pvtable(stab, ifamily(node), PFAM,
		    (fcel = fam_to_cacheel(fam)));
		insert_pvtable(stab, ispouse(node), PINDI,
		     (scel = indi_to_cacheel(spouse)));
		insert_pvtable(stab, inum(node), PINT, (WORD)nfams);
		lock_cache(fcel);
		if (scel) lock_cache(scel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		if (scel) unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto c;
		case INTBREAK:
			unlock_cache(icel);
			return INTOKAY;
		default:
			unlock_cache(icel);
			return irc;
		}
c:	;
	ENDFAMSS
	unlock_cache(icel);
	return INTOKAY;
}
/*========================================+
 * interp_fathers -- Interpret fathers loop
 *=======================================*/
INTERPTYPE
interp_fathers (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	INT ncount = 1;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, "1st arg to fathers must be a person");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, "1st arg to fathers has a major error");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	insert_pvtable(stab, inum(node), PINT, (WORD) 0);
	FORFAMCS(indi, fam, husb, wife, nfams)
		scel = indi_to_cacheel(husb);
		if (!scel) goto d;
		insert_pvtable(stab, ifamily(node), PFAM,
		    (WORD) (fcel = fam_to_cacheel(fam)));
		insert_pvtable(stab, iiparent(node), PINDI, (WORD) scel);
		insert_pvtable(stab, inum(node), PINT, (WORD) ncount++);
		lock_cache(fcel);
		lock_cache(scel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto d;
		case INTBREAK:
			unlock_cache(icel);
			return INTOKAY;
		default:
			unlock_cache(icel);
			return irc;
		}
d:	;
	ENDFAMCS
	unlock_cache(icel);
	return INTOKAY;
}
/*========================================+
 * interp_mothers -- Interpret mothers loop
 *=======================================*/
INTERPTYPE
interp_mothers (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	INT ncount = 1;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, "1st arg to mothers must be a person");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, "1st arg to mothers has a major error");
		return INTERROR;
	}
	if (eflg || (indi && nestr(ntag(indi), "INDI"))) return INTERROR;
	if (!indi) return TRUE;
	lock_cache(icel);
	insert_pvtable(stab, inum(node), PINT, (WORD) 0);
	FORFAMCS(indi, fam, husb, wife, nfams)
		scel = indi_to_cacheel(wife);
		if (!scel) goto e;
		insert_pvtable(stab, ifamily(node), PFAM,
		    (WORD) (fcel = fam_to_cacheel(fam)));
		insert_pvtable(stab, iiparent(node), PINDI, scel);
		insert_pvtable(stab, inum(node), PINT, (WORD) ncount++);
		lock_cache(fcel);
		lock_cache(scel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		unlock_cache(scel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto e;
		case INTBREAK:
			unlock_cache(icel);
			return INTOKAY;
		default:
			unlock_cache(icel);
			return irc;
		}
e:	;
	ENDFAMCS
	unlock_cache(icel);
	return INTOKAY;
}
/*========================================+
 * interp_parents -- Interpret parents loop
 *=======================================*/
INTERPTYPE
interp_parents (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INT nfams;
	CACHEEL icel, fcel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iloopexp(node), stab, &eflg, &icel);
	if (eflg) {
		prog_error(node, "1st arg to parents must be a person");
		return INTERROR;
	}
	if (indi && nestr(ntag(indi), "INDI")) {
		prog_error(node, "1st arg to parents has a major error");
		return INTERROR;
	}
	if (!indi) return TRUE;
	lock_cache(icel);
	FORFAMCS(indi, fam, husb, wife, nfams)
		insert_pvtable(stab, ifamily(node), PFAM,
		    (WORD) (fcel = fam_to_cacheel(fam)));
		insert_pvtable(stab, inum(node), PINT, (WORD) nfams);
		lock_cache(fcel);
		irc = interpret((PNODE) ibody(node), stab, pval);
		unlock_cache(fcel);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto f;
		case INTBREAK:
			unlock_cache(icel);
			return INTOKAY;
		default:
			unlock_cache(icel);
			return irc;
		}
f:	;
	ENDFAMCS
	unlock_cache(icel);
	return INTOKAY;
}
/*=======================================
 * interp_fornotes -- Interpret NOTE loop
 *=====================================*/
INTERPTYPE
interp_fornotes (PNODE node,
                 TABLE stab,
                 PVALUE *pval)
{
        BOOLEAN eflg = FALSE;
        INTERPTYPE irc;
        NODE root;
	PVALUE val = eval_and_coerce(PGNODE, iloopexp(node), stab, &eflg);
	if (eflg) {
		prog_error(node, "1st arg to fornotes must be a record line");
		return INTERROR;
	}
	root = (NODE) pvalue(val);
	delete_pvalue(val);
	if (!root) return INTOKAY;
/*HERE*/
        FORTAGVALUES(root, "NOTE", sub, vstring)
                insert_pvtable(stab, ielement(node), PSTRING, vstring);
                irc = interpret((PNODE) ibody(node), stab, pval);
                switch (irc) {
                case INTCONTINUE:
                case INTOKAY:
                        goto g;
                case INTBREAK:
                        return INTOKAY;
                default:
                        return irc;
                }
g:      ;
        ENDTAGVALUES
        return INTOKAY;
}
/*==========================================+
 * interp_fornodes -- Interpret fornodes loop
 *  usage: fornodes(NODE,NODE_V) {...}
 *=========================================*/
INTERPTYPE
interp_fornodes (PNODE node,
                 TABLE stab,
                 PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	NODE sub, root = (NODE) evaluate(iloopexp(node), stab, &eflg);
	PVALUE val = eval_and_coerce(PGNODE, iloopexp(node), stab, &eflg);
	if (eflg) {
		prog_error(node, "1st arg to fornodes must be a record line");
		return INTERROR;
	}
	root = (NODE) pvalue(val);
	delete_pvalue(val);
	if (!root) return INTOKAY;
/*HERE*/
	sub = nchild(root);
	while (sub) {
		insert_pvtable(stab, ielement(node), PGNODE, sub);
		irc = interpret((PNODE) ibody(node), stab, pval);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			sub = nsibling(sub);
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
	return INTOKAY;
}
/*========================================+
 * interp_forindi -- Interpret forindi loop
 *  usage: forindi(INDI_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_forindi (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	NODE indi;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, count = 0;
	INT icount = 0;
	insert_pvtable(stab, inum(node), PINT, 0);
	while (TRUE) {
		sprintf(key, "I%d", ++count);
		if (!(record = retrieve_record(key, &len))) {
		    if (icount < num_indis()) continue;
		    break;
		}
		if (!(indi = string_to_node(record))) continue;
		icount++;
		insert_pvtable(stab, ielement(node), PINDI,
		    indi_to_cacheel(indi));
		insert_pvtable(stab, inum(node), PINT, (WORD)count);
		irc = interpret((PNODE) ibody(node), stab, pval);
		free_nodes(indi);
		stdfree(record);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
	return INTOKAY;
}
/*========================================+
 * interp_forsour -- Interpret forsour loop
 *  usage: forsour(SOUR_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_forsour (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	NODE sour;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, count = 0;
	INT scount = 0;
	insert_pvtable(stab, inum(node), PINT, 0);
	while (TRUE) {
		sprintf(key, "S%d", ++count);
		if (!(record = retrieve_record(key, &len))) {
		    if(scount < num_sours()) continue;
		    break;
		}
		if (!(sour = string_to_node(record))) continue;
		scount++;
		insert_pvtable(stab, ielement(node), PSOUR,
		    sour_to_cacheel(sour));
		insert_pvtable(stab, inum(node), PINT, (WORD)count);
		irc = interpret((PNODE) ibody(node), stab, pval);
		free_nodes(sour);
		stdfree(record);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
	return INTOKAY;
}
/*========================================+
 * interp_foreven -- Interpret foreven loop
 *  usage: foreven(EVEN_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_foreven (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	NODE even;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, count = 0;
	INT ecount = 0;
	insert_pvtable(stab, inum(node), PINT, (WORD)count);
	while (TRUE) {
		sprintf(key, "E%d", ++count);
		if (!(record = retrieve_record(key, &len))) {
		    if(ecount < num_evens()) continue;
		    break;
		}
		if (!(even = string_to_node(record))) continue;
		ecount++;
		insert_pvtable(stab, ielement(node), PEVEN,
		    even_to_cacheel(even));
		insert_pvtable(stab, inum(node), PINT, (WORD)count);
		irc = interpret((PNODE) ibody(node), stab, pval);
		free_nodes(even);
		stdfree(record);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
	return INTOKAY;
}
/*========================================+
 * interp_forothr -- Interpret forothr loop
 *  usage: forothr(OTHR_V,INT_V) {...}
 *=======================================*/
INTERPTYPE
interp_forothr (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	NODE othr;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, count = 0;
	INT ocount = 0;
	insert_pvtable(stab, inum(node), PINT, (WORD)count);
	while (++count <= num_othrs()) {
		sprintf(key, "X%d", count);
		if (!(record = retrieve_record(key, &len)))
			continue;
		if (!(othr = string_to_node(record))) continue;
		ocount++;
		insert_pvtable(stab, ielement(node), POTHR,
		    (WORD)othr_to_cacheel(othr));
		insert_pvtable(stab, inum(node), PINT, (WORD)count);
		irc = interpret((PNODE) ibody(node), stab, pval);
		free_nodes(othr);
		stdfree(record);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
	return INTOKAY;
}
/*======================================+
 * interp_forfam -- Interpret forfam loop
 *  usage: forfam(FAM_V,INT_V) {...}
 *=====================================*/
INTERPTYPE
interp_forfam (PNODE node,
               TABLE stab,
               PVALUE *pval)
{
	NODE fam;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, count = 0;
	INT fcount = 0;
	insert_pvtable(stab, inum(node), PINT, (WORD)count);
	while (TRUE) {
		sprintf(key, "F%d", ++count);
		if (!(record = retrieve_record(key, &len))) {
		    if(fcount < num_fams()) continue;
		    break;
		}
		if (!(fam = string_to_node(record))) continue;
		fcount++;
		insert_pvtable(stab, ielement(node), PFAM,
		    (WORD) fam_to_cacheel(fam));
		insert_pvtable(stab, inum(node), PINT, (WORD)count);
		irc = interpret((PNODE) ibody(node), stab, pval);
		free_nodes(fam);
		stdfree(record);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
	return INTOKAY;
}
/*============================================+
 * interp_indisetloop -- Interpret indiset loop
 *===========================================*/
INTERPTYPE
interp_indisetloop (PNODE node,
                    TABLE stab,
                    PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	INDISEQ seq = NULL;
	PVALUE val = evaluate(iloopexp(node), stab, &eflg);
	if (eflg || !val || ptype(val) != PSET) {
		prog_error(node, "1st arg to forindiset must be set expr");
		return INTERROR;
	}
	seq = (INDISEQ) pvalue(val);
	delete_pvalue(val);
	insert_pvtable(stab, inum(node), PINT, (WORD) 0);
	FORINDISEQ(seq, el, ncount)
#ifdef DEBUG
		llwprintf("loopinterp - %s = ",ielement(node));
		llwprintf("\n");
#endif
		insert_pvtable(stab, ielement(node), PINDI,
		(WORD)key_to_indi_cacheel(skey(el)));
#ifdef DEBUG
		llwprintf("loopinterp - %s = ",ivalvar(node));
		llwprintf("\n");
#endif
		insert_table(stab, ivalvar(node),
			 (WORD) (sval(el) ? (WORD)sval(el)
				: (WORD)create_pvalue(PANY, (WORD)NULL)));
		insert_pvtable(stab, inum(node), PINT, (WORD) (ncount + 1));
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			goto h;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
h:	;
	ENDINDISEQ
	return INTOKAY;
}
/*=====================================+
 * interp_forlist -- Interpret list loop
 *====================================*/
INTERPTYPE
interp_forlist (PNODE node,
                TABLE stab,
                PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	INT ncount = 1;
	LIST list;
	PVALUE val = eval_and_coerce(PLIST, iloopexp(node), stab, &eflg);
	if (eflg || !val || ptype(val) != PLIST) {
		prog_error(node, "1st arg to forlist is not a list");
		return INTERROR;
	}
	list = (LIST) pvalue(val);
	delete_pvalue(val);
	if (!list) {
		prog_error(node, "1st arg to forlist is in error");
		return INTERROR;
	}
	insert_pvtable(stab, inum(node), PINT, (WORD) 0);
	FORLIST(list, el)
		insert_table(stab, ielement(node), copy_pvalue(el));
		insert_pvtable(stab, inum(node), PINT, (WORD) ncount++);
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			goto i;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
i:	;
	ENDLIST
	return INTOKAY;
}
/*===================================+
 * interp_if -- Interpret if structure
 *==================================*/
INTERPTYPE
interp_if (PNODE node,
           TABLE stab,
           PVALUE *pval)
{
	BOOLEAN eflg = FALSE;
	BOOLEAN cond = evaluate_cond(icond(node), stab, &eflg);
	if (eflg) return INTERROR;
	if (cond) return interpret((PNODE) ithen(node), stab, pval);
	if (ielse(node)) return interpret((PNODE) ielse(node), stab, pval);
	return INTOKAY;
}
/*=========================================+
 * interp_while -- Interpret while structure
 *========================================*/
INTERPTYPE
interp_while (PNODE node,
              TABLE stab,
              PVALUE *pval)
{
	BOOLEAN eflg = FALSE, cond;
	INTERPTYPE irc;
	while (TRUE) {
		cond = evaluate_cond(icond(node), stab, &eflg);
		if (eflg) return INTERROR;
		if (!cond) return INTOKAY;
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			continue;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
	}
}
/*=======================================+
 * interp_call -- Interpret call structure
 *======================================*/
INTERPTYPE
interp_call (PNODE node,
             TABLE stab,
             PVALUE *pval)
{
	TABLE newtab;
	INTERPTYPE irc;
	PNODE arg, parm, proc = (PNODE) valueof(proctab, iname(node));
	if (!proc) {
		llwprintf("``%s'': undefined procedure\n", iname(node));
		return INTERROR;
	}
	newtab = create_table();
	arg = (PNODE) iargs(node);
	parm = (PNODE) iargs(proc);
	while (arg && parm) {
		BOOLEAN eflg = FALSE;
		PVALUE value = evaluate(arg, stab, &eflg);
		if (eflg) return INTERROR;
		insert_table(newtab, iident(parm), (WORD) value);
		arg = inext(arg);
		parm = inext(parm);
	}
	if (arg || parm) {
		llwprintf("``%s'': mismatched args and params\n", iname(node));
		remove_table(newtab, DONTFREE);
		return INTERROR;
	}
	irc = interpret((PNODE) ibody(proc), newtab, pval);
#ifdef HOGMEMORY
	remove_table(newtab, DONTFREE);
#else
	remove_pvtable(newtab);
#endif
	switch (irc) {
	case INTRETURN:
	case INTOKAY:
		return INTOKAY;
	case INTBREAK:
	case INTCONTINUE:
	case INTERROR:
	default:
		return INTERROR;
	}
	return INTERROR;
}
/*==============================================+
 * interp_traverse -- Interpret traverse iterator
 *  usage: traverse(NODE,NODE_V,INT_V) {...}
 *=============================================*/
INTERPTYPE
interp_traverse (PNODE node,
                 TABLE stab,
                 PVALUE *pval)
{
	NODE snode, stack[100];
	BOOLEAN eflg = FALSE;
	INTERPTYPE irc;
	INT lev = -1;
	NODE root;
	PVALUE val = eval_and_coerce(PGNODE, iloopexp(node), stab, &eflg);
	if (eflg) {
		prog_error(node, "1st arg to traverse must be a record line");
		return INTERROR;
	}
	root = (NODE) pvalue(val);
	delete_pvalue(val);
	if (!root) return INTOKAY;
	stack[++lev] = snode = root;
	while (TRUE) {
		insert_pvtable(stab, ielement(node), PGNODE, (WORD)snode);
		insert_pvtable(stab, ilev(node), PINT, (WORD)lev);
		switch (irc = interpret((PNODE) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			break;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
		if (nchild(snode)) {
			snode = stack[++lev] = nchild(snode);
			continue;
		}
		if (nsibling(snode)) {
			snode = stack[lev] = nsibling(snode);
			continue;
		}
		while (--lev >= 0 && !nsibling(stack[lev]))
			;
		if (lev < 0) break;
		snode = stack[lev] = nsibling(stack[lev]);
	}
	return INTOKAY;
}
/*=============================================+
 * prog_error -- Report a run time program error
 *============================================*/
void
prog_error (PNODE node,
            STRING fmt, ...)
{
	va_list args;
	llwprintf("\nError in \"%s\" at line %d: ", ifname(node), iline(node));
	va_start(args, fmt);
	llvwprintf(fmt, args);
	va_end(args);
	llwprintf(".\n");
	sleep(5);
}
