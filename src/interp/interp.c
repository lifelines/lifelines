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
/*=============================================================
 * interp.c -- Interpret program statements
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 16 Aug 93
 *   3.0.0 - 26 Jul 94    3.0.2 - 25 Mar 95
 *===========================================================*/

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "interp.h"
#include "cache.h"
#include "indiseq.h"

extern STRING llprograms;

STRING Pfname = NULL;	/* file to read program from */
FILE *Pinfp  = NULL;	/* file to read program from */
FILE *Poutfp = NULL;	/* file to write program output to */
STRING Pinstr = NULL;	/* string to read program from */
STRING Poutstr = NULL;	/* string to write program output to */
INT Plineno = 1;
INT Perrors = 0;
LIST Plist;		/* list of program files still to read */

TABLE filetab, proctab, globtab, functab;
STRING ierror = (STRING) "Error: file \"%s\": line %d: ";
static STRING noprogram = (STRING) "No program was run.";
static STRING qrptname = (STRING) "What is the name of the program?";

/*=====================================
 * initinterp -- Initialize interpreter
 *===================================*/
initinterp ()
{
	initrassa();
	initset();
	Perrors = 0;
}
/*===================================
 * finishinterp -- Finish interpreter
 *=================================*/
finishinterp ()
{
	finishrassa();
}
/*==============================================
 * interp_program -- Interpret LifeLines program
 *============================================*/
interp_program (proc, nargs, args, nifiles, ifiles, ofile)
STRING proc;	/* proc to call */
INT nargs;	/* number of arguments */
WORD *args;	/* arguments */
INT nifiles;	/* number of program files - can be zero */
STRING *ifiles;	/* program files */
STRING ofile;	/* output file - can be NULL */
{
	LIST plist = create_list();
	TABLE stab;
	WORD dummy;
	INT i;
	STRING ifile;
	INTERP first, parm;

   /* Get the initial list of program files */

	set_list_type(plist, LISTDOFREE);
	if (nifiles > 0) {
		for (i = 0; i < nifiles; i++) {
			enqueue_list(plist, strsave(ifiles[i]));
		}
	} else {
		ifile = ask_for_string(qrptname, "enter string: ");
		if (!ifile)  {
			message(noprogram);
			return;
		}
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
/*wprintf("About to parse file %s.\n", ifile);/*DEBUG*/
			parse_file(ifile, plist);
		} else
			stdfree(ifile);
	}
	if (Perrors) {
		message("The program contains errors.\n");
		return;
	}

   /* Find top procedure */

	if (!(first = (INTERP) valueof(proctab, proc))) {
		message("Report program needs a starting procedure.");
		remove_tables();
		return;
	}

   /* Link arguments to parameters in symbol table */

	parm = (INTERP) iparams(first);
	if (nargs != num_params(parm)) {
		mprintf("Proc %s must be called with %d (not %d) parameters.",
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

	switch (interpret((INTERP) ibody(first), stab, &dummy)) {
	case INTOKAY:
	case INTRETURN:
		message("The program was run successfully.");
		break;
	default:
		message("The program was not run because of errors.");
		break;
	}

   /* Clean up and return */

	remove_tables();
	remove_table(stab, DONTFREE);
	finishinterp();
	if (Poutfp) fclose(Poutfp);
	Pinfp = Poutfp = NULL;
}
/*============================================
 * remove_tables - Remove interpreter's tables
 *==========================================*/
remove_tables ()
{
	remove_table(filetab, FREEKEY);
	remove_table(proctab, DONTFREE);
	remove_table(globtab, DONTFREE);
	remove_table(functab, DONTFREE);
}
/*=======================================
 * parse_file - Parse single program file
 *=====================================*/
parse_file (ifile, plist)
STRING ifile;
LIST plist;
{
	Pfname = ifile;
	if (!ifile || *ifile == 0) return;
	Plist = plist;
	Pinfp = fopenpath(ifile, "r", llprograms);
	if (!Pinfp) {
		wprintf("Error: file \"%s\" not found.\n", ifile);
		Perrors++;
		return;
	}
	Plineno = 1;
	yyparse();
	fclose(Pinfp);
}
/*=====================================
 * interp_main -- Interpreter main proc
 *===================================*/
interp_main ()
{
	interp_program("main", 0, NULL, 0, NULL, NULL);/*DEBUG*/
}
/*======================================
 * interpret -- Interpret statement list
 *====================================*/
INTERPTYPE interpret (node, stab, pval)
INTERP node;	/* first node to interpret */
TABLE stab;	/* current symbol table */
WORD *pval;	/* possible return value */
{
	STRING str;
	BOOLEAN eflg;
	INTERPTYPE irc;

	*pval = NULL;
	while (node) {
		switch (itype(node)) {
		case ILITERAL:
			poutput(iliteral(node));
			break;
		case IIDENT:
			str = (STRING) evaluate_iden(node, stab, &eflg);
			if (eflg) {
				wprintf(ierror, Pfname, iline(node));
				wprintf("identifier: %s\n", iident(node));
				return FALSE;
			}
			poutput(str);
			break;
		case IBCALL:
			str = (STRING) evaluate_func(node, stab, &eflg);
			if (eflg) {
				wprintf(ierror, Pfname, iline(node));
				wprintf("in function: `%s'\n", iname(node));
				return FALSE;
			}
			if (str) poutput(str);
			break;
		case IFCALL:
			str = (STRING) evaluate_ufunc(node, stab, &eflg);
			if (eflg) {
				wprintf(ierror, Pfname, iline(node));
				wprintf("in function: `%s'\n", iname(node));
				return FALSE;
			}
			if (str) poutput(str);
			break;
		case IPDEFN:
			FATAL();
		case ICHILDREN:
			switch (irc = interp_children(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
				wprintf(ierror, Pfname, iline(node));
				wprintf("in children loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in spouses loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in families loop\n");
				return INTERROR;
			default:
				return irc;
			}
			break;
		case IINDICES:
			switch (irc = interp_indisetloop(node, stab, pval)) {
			case INTOKAY:
				break;
			case INTERROR:
				wprintf(ierror, Pfname, iline(node));
				wprintf("in indiset loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in forindi loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in forfam loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in forlist loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in fornotes loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in fornodes loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in traverse loop\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in if statement\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in while statement\n");
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
				wprintf(ierror, Pfname, iline(node));
				wprintf("in procedure call\n");
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
			return INTRETURN;
		default:
			wprintf("itype(node) is %d\n", itype(node));
			wprintf("HUH, HUH, HUH, HUNH!\n");
			return FALSE;
		}
		node = inext(node);
	}
	return TRUE;
}
/*========================================
 * interp_children -- Interpret child loop
 *======================================*/
INTERPTYPE interp_children (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INT nchil;
	CACHEEL fcel, cel;
	INTERPTYPE irc;
	NODE fam = (NODE) eval_fam(ifamily(node), stab, &eflg, &fcel);
	if (eflg || (fam && nestr(ntag(fam), "FAM"))) return FALSE;
	if (!fam) return TRUE;
	lock_cache(fcel);
	FORCHILDREN(fam, chil, nchil)
		insert_table(stab, ichild(node),
		    (WORD) (cel = indi_to_cacheel(chil)));
		insert_table(stab, inum(node), (WORD) nchil);
		lock_cache(cel);
		irc = interpret((INTERP) ibody(node), stab, pval);
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
/*========================================
 * interp_spouses -- Interpret spouse loop
 *======================================*/
INTERPTYPE interp_spouses (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INT nspouses;
	CACHEEL icel, scel, fcel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iprinc(node), stab, &eflg, &icel);
	if (eflg || (indi && nestr(ntag(indi), "INDI"))) return FALSE;
	if (!indi) return TRUE;
	lock_cache(icel);
	FORSPOUSES(indi, spouse, fam, nspouses)
		insert_table(stab, ispouse(node),
		    (WORD) (scel = indi_to_cacheel(spouse)));
		insert_table(stab, ifamvar(node),
		    (WORD) (fcel = fam_to_cacheel(fam)));
		lock_cache(scel);
		lock_cache(fcel);
		insert_table(stab, inum(node), (WORD) nspouses);
		irc = interpret((INTERP) ibody(node), stab, pval);
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
/*=========================================
 * interp_families -- Interpret family loop
 *=======================================*/
INTERPTYPE interp_families (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INT nfamilies;
	CACHEEL icel, fcel, scel;
	INTERPTYPE irc;
	NODE indi = (NODE) eval_indi(iprinc(node), stab, &eflg, &icel);
	if (eflg || (indi && nestr(ntag(indi), "INDI"))) return FALSE;
	if (!indi) return TRUE;
	lock_cache(icel);
	FORFAMSS(indi, fam, spouse, nfamilies)
		insert_table(stab, ifamvar(node),
		    (WORD) (fcel = fam_to_cacheel(fam)));
		insert_table(stab, ispouse(node),
		    (WORD) (scel = indi_to_cacheel(spouse)));
		insert_table(stab, inum(node), (WORD) nfamilies);
		lock_cache(fcel);
		if (scel) lock_cache(scel);
		irc = interpret((INTERP) ibody(node), stab, pval);
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
/*=======================================
 * interp_fornotes -- Interpret NOTE loop
 *=====================================*/
INTERPTYPE interp_fornotes (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INTERPTYPE irc;
	NODE root = (NODE) evaluate(inode(node), stab, &eflg);
	if (eflg || !root) return FALSE;
	FORTAGVALUES(root, "NOTE", sub, vstring)
		insert_table(stab, istrng(node), (WORD) vstring);
		irc = interpret((INTERP) ibody(node), stab, pval);
		switch (irc) {
		case INTCONTINUE:
		case INTOKAY:
			goto d;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
d:	;
	ENDTAGVALUES
	return INTOKAY;
}
/*===========================================
 * interp_fornodes -- Interpret fornodes loop
 *=========================================*/
INTERPTYPE interp_fornodes (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INTERPTYPE irc;
	NODE sub, root = (NODE) evaluate(inode(node), stab, &eflg);
	if (eflg || !root) return FALSE;
	sub = nchild(root);
	while (sub) {
		insert_table(stab, isubnode(node), (WORD) sub);
		irc = interpret((INTERP) ibody(node), stab, pval);
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
/*=========================================
 * interp_forindi -- Interpret forindi loop
 *=======================================*/
INTERPTYPE interp_forindi (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	NODE indi;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, i = 0;
	while (TRUE) {
		sprintf(key, "I%d", ++i);
		if (!(record = retrieve_record(key, &len))) break;
		if (!(indi = string_to_node(record))) continue;
		insert_table(stab, iindivar(node),
		    (WORD) indi_to_cacheel(indi));
		insert_table(stab, inum(node), (WORD) i);
		irc = interpret((INTERP) ibody(node), stab, pval);
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
/*=======================================
 * interp_forfam -- Interpret forfam loop
 *=====================================*/
INTERPTYPE interp_forfam (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	NODE fam;
	static char key[10];
	STRING record;
	INTERPTYPE irc;
	INT len, i = 0;
	while (TRUE) {
		sprintf(key, "F%d", ++i);
		if (!(record = retrieve_record(key, &len))) break;
		if (!(fam = string_to_node(record))) continue;
		insert_table(stab, iindivar(node),
		    (WORD) fam_to_cacheel(fam));
		insert_table(stab, inum(node), (WORD) i);
		irc = interpret((INTERP) ibody(node), stab, pval);
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
/*=============================================
 * interp_indisetloop -- Interpret indiset loop
 *===========================================*/
INTERPTYPE interp_indisetloop (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INTERPTYPE irc;
	INDISEQ seq = (INDISEQ) evaluate(iindex(node), stab, &eflg);
	if (eflg || !seq) return FALSE;
	FORINDISEQ(seq, el, ncount)
		insert_table(stab, iindivar(node),
		    (WORD) key_to_indi_cacheel(skey(el)));
		insert_table(stab, ivalvar(node), (WORD) sval(el));
		insert_table(stab, inum(node), (WORD) (ncount + 1));
		switch (irc = interpret((INTERP) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			goto e;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
e:	;
	ENDINDISEQ
	return INTOKAY;
}
/*======================================
 * interp_forlist -- Interpret list loop
 *====================================*/
INTERPTYPE interp_forlist (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	INTERPTYPE irc;
	INT ncount = 1;
	LIST lst = (LIST) evaluate(ilist(node), stab, &eflg);
	if (eflg || !lst) return FALSE;
	FORLIST(lst, el)
		insert_table(stab, ielement(node), (WORD) el);
		insert_table(stab, inum(node), (WORD) ncount++);
		switch (irc = interpret((INTERP) ibody(node), stab, pval)) {
		case INTCONTINUE:
		case INTOKAY:
			goto f;
		case INTBREAK:
			return INTOKAY;
		default:
			return irc;
		}
f:	;
	ENDLIST
	return INTOKAY;
}
/*====================================
 * interp_if -- Interpret if structure
 *==================================*/
INTERPTYPE interp_if (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	WORD value = evaluate_cond(icond(node), stab, &eflg);
	if (eflg) return INTERROR;
	if (value) return interpret((INTERP) ithen(node), stab, pval);
	if (ielse(node)) return interpret((INTERP) ielse(node), stab, pval);
	return INTOKAY;
}
/*==========================================
 * interp_while -- Interpret while structure
 *========================================*/
INTERPTYPE interp_while (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	BOOLEAN eflg;
	WORD value;
	INTERPTYPE irc;
	while (TRUE) {
		value = evaluate_cond(icond(node), stab, &eflg);
		if (eflg) return INTERROR;
		if (value == (WORD) FALSE) return INTOKAY;
		switch (irc = interpret((INTERP) ibody(node), stab, pval)) {
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
/*========================================
 * interp_call -- Interpret call structure
 *======================================*/
INTERPTYPE interp_call (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	TABLE newtab;
	INTERPTYPE irc;
	INTERP arg, parm, proc = (INTERP) valueof(proctab, iname(node));
	if (!proc) {
		wprintf("``%s'': undefined procedure\n", iname(node));
		return INTERROR;
	}
	newtab = create_table();
	arg = (INTERP) iargs(node);
	parm = (INTERP) iparams(proc);
	while (arg && parm) {
		BOOLEAN eflg;
		WORD value = evaluate(arg, stab, &eflg);
		if (eflg) return INTERROR;
		insert_table(newtab, iident(parm), (WORD) value);
		arg = inext(arg);
		parm = inext(parm);
	}
	if (arg || parm) {
		wprintf("``%s'': mismatched args and params\n", iname(node));
		remove_table(newtab, DONTFREE);
		return INTERROR;
	}
	irc = interpret((INTERP) ibody(proc), newtab, pval);
	remove_table(newtab, DONTFREE);
	switch (irc) {
	case INTRETURN:
	case INTOKAY:
		return INTOKAY;
	case INTBREAK:
	case INTCONTINUE:
	case INTERROR:
		return INTERROR;
	}
}
/*===============================================
 * interp_traverse -- Interpret traverse iterator
 *=============================================*/
INTERPTYPE interp_traverse (node, stab, pval)
INTERP node; TABLE stab; WORD *pval;
{
	NODE snode, stack[100];
	BOOLEAN eflg;
	INTERPTYPE irc;
	INT lev = -1;
	NODE root = (NODE) evaluate (inode(node), stab, &eflg);
	if (eflg || !root) return FALSE;
	stack[++lev] = snode = root;
	while (TRUE) {
		insert_table(stab, isubnode(node), (WORD) snode);
		insert_table(stab, ilev(node), (WORD) lev);
		switch (irc = interpret((INTERP) ibody(node), stab, pval)) {
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
