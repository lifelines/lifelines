/*
   Copyright (c) 2000 Petter Reinholdtsen

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
/*======================================================================
 * interp-priv.h - Internal header file for report generator interpreter
 * Copyright(c) 2000 by Petter Reinholdtsen; all rights reserved
 *====================================================================*/

#ifndef _INTERP_PRIV_H
#define _INTERP_PRIV_H

#include "interp.h"

/* parse global context */
struct pactx_s {
	FILE *Pinfp;     /* file to read program from */
	STRING Pinstr;   /* string to read program from */
	TABLE filetab;   /* table of files called by current report (incl. itself) */
	STRING ifile;    /* user's requested program path (current report) */
	STRING fullpath; /* actual path of current program */
	INT lineno;      /* current line number (0-based) */
	INT charpos;     /* current offset on line (0-based) */
};

struct pathinfo_s {
	STRING fname;    /* filename as user specified */
	STRING fullpath; /* fully qualified path */
};


PVALUE alloc_pvalue_memory(void);
void check_pvalue_validity(PVALUE val);
PVALUE create_new_pvalue(void);
void free_pvalue_memory(PVALUE val);
void set_pvalue_node(PVALUE val, NODE node);



/* interpreter language command entry points */

extern PVALUE ___alpha(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __add(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __addnode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __addtoset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __ancestorset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __and(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __bapt(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __birt(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __buri(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __bytecode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __capitalize(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __card(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __child(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __childset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __choosechild(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __choosefam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __chooseindi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __choosespouse(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __choosesubset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __col(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __complexdate(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __complexformat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __complexpic(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __concat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __convertcode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __copyfile(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __createnode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __d(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __database(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __date(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __dateformat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __datepic(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __dayformat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __deat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __debug(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __decr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __deletefromset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __dequeue(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __descendentset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __detachnode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __difference(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __div(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __empty(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __enqueue(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __eq(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __eqstr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __exp(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __extractdate(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __extractdatestr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __extractnames(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __extractplaces(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __extracttokens(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __f(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __fam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __fath(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __female(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __firstchild(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __firstfam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __firstindi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __fnode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __free(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __free(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __fullname(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __ge(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __gengedcom(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __gengedcomweak(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __gengedcomstrong(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __genindiset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getcol(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getel(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getfam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getindi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getindiset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getint(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getproperty(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getrecord(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __getstr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __gettoday(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __givens(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __gt(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __heapused(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __husband(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __incr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __index(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __indi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __indiset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __inlist(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __inlist(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __inode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __insert(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __inset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __intersect(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __key(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __keysort(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lastchild(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lastfam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lastindi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __le(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __length(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lengthset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __level(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __linemode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __list(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lock(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __long(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lookup(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lower(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __lt(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __male(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __marr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __menuchoose(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __mod(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __monthformat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __moth(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __mul(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __name(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __namesort(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nchildren(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __ne(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __neg(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nestr(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __newfile(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nextfam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nextindi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nextsib(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nfamilies(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nl(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __not(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __nspouses(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __or(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __ord(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __eraformat(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __outfile(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __pagemode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __pageout(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __parent(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __parents(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __parentset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __place(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __pn(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __pop(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __pos(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __prevfam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __previndi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __prevsib(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __print(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __program(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __push(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __pvalue(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __qt(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __reference(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __requeue(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __rjustify(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __roman(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __rot(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __row(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __save(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __savenode(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __set(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __setel(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __setlocale(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __sex(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __short(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __sibling(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __siblingset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __soundex(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __space(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __spouseset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __stddate(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __strcmp(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __strlen(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __strsoundex(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __strtoint(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __sub(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __substring(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __surname(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __runsystem(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __table(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __tag(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __titl(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __titlcase(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __trim(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __trimname(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __union(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __uniqueset(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __unlock(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __upper(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __value(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __valuesort(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __version(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __wife(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __writefam(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __writeindi(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __xref(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __year(PNODE, SYMTAB, BOOLEAN *);
extern PVALUE __yearformat(PNODE, SYMTAB, BOOLEAN *);

#endif /* _INTERP_PRIV_H */

