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

#ifdef WIN32
extern PVALUE __heapused(PNODE, TABLE, BOOLEAN *);
#endif
extern PVALUE ___alpha(PNODE, TABLE, BOOLEAN *);
extern PVALUE __add(PNODE, TABLE, BOOLEAN *);
extern PVALUE __addnode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __addtoset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __ancestorset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __and(PNODE, TABLE, BOOLEAN *);
extern PVALUE __bapt(PNODE, TABLE, BOOLEAN *);
extern PVALUE __birt(PNODE, TABLE, BOOLEAN *);
extern PVALUE __buri(PNODE, TABLE, BOOLEAN *);
extern PVALUE __capitalize(PNODE, TABLE, BOOLEAN *);
extern PVALUE __card(PNODE, TABLE, BOOLEAN *);
extern PVALUE __child(PNODE, TABLE, BOOLEAN *);
extern PVALUE __childset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __choosechild(PNODE, TABLE, BOOLEAN *);
extern PVALUE __choosefam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __chooseindi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __choosespouse(PNODE, TABLE, BOOLEAN *);
extern PVALUE __choosesubset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __col(PNODE, TABLE, BOOLEAN *);
extern PVALUE __complexdate(PNODE, TABLE, BOOLEAN *);
extern PVALUE __concat(PNODE, TABLE, BOOLEAN *);
extern PVALUE __copyfile(PNODE, TABLE, BOOLEAN *);
extern PVALUE __createnode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __d(PNODE, TABLE, BOOLEAN *);
extern PVALUE __database(PNODE, TABLE, BOOLEAN *);
extern PVALUE __date(PNODE, TABLE, BOOLEAN *);
extern PVALUE __dateformat(PNODE, TABLE, BOOLEAN *);
extern PVALUE __dayformat(PNODE, TABLE, BOOLEAN *);
extern PVALUE __deat(PNODE, TABLE, BOOLEAN *);
extern PVALUE __debug(PNODE, TABLE, BOOLEAN *);
extern PVALUE __decr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __deletefromset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __deletenode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __dequeue(PNODE, TABLE, BOOLEAN *);
extern PVALUE __descendentset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __difference(PNODE, TABLE, BOOLEAN *);
extern PVALUE __div(PNODE, TABLE, BOOLEAN *);
extern PVALUE __empty(PNODE, TABLE, BOOLEAN *);
extern PVALUE __enqueue(PNODE, TABLE, BOOLEAN *);
extern PVALUE __eq(PNODE, TABLE, BOOLEAN *);
extern PVALUE __eqstr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __exp(PNODE, TABLE, BOOLEAN *);
extern PVALUE __extractdate(PNODE, TABLE, BOOLEAN *);
extern PVALUE __extractdatestr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __extractnames(PNODE, TABLE, BOOLEAN *);
extern PVALUE __extractplaces(PNODE, TABLE, BOOLEAN *);
extern PVALUE __extracttokens(PNODE, TABLE, BOOLEAN *);
extern PVALUE __f(PNODE, TABLE, BOOLEAN *);
extern PVALUE __fam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __fath(PNODE, TABLE, BOOLEAN *);
extern PVALUE __female(PNODE, TABLE, BOOLEAN *);
extern PVALUE __firstchild(PNODE, TABLE, BOOLEAN *);
extern PVALUE __firstfam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __firstindi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __fnode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __free(PNODE, TABLE, BOOLEAN *);
extern PVALUE __free(PNODE, TABLE, BOOLEAN *);
extern PVALUE __fullname(PNODE, TABLE, BOOLEAN *);
extern PVALUE __ge(PNODE, TABLE, BOOLEAN *);
extern PVALUE __gengedcom(PNODE, TABLE, BOOLEAN *);
extern PVALUE __genindiset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getcol(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getel(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getfam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getindi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getindiset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getint(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getrecord(PNODE, TABLE, BOOLEAN *);
extern PVALUE __getstr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __gettoday(PNODE, TABLE, BOOLEAN *);
extern PVALUE __givens(PNODE, TABLE, BOOLEAN *);
extern PVALUE __gt(PNODE, TABLE, BOOLEAN *);
extern PVALUE __husband(PNODE, TABLE, BOOLEAN *);
extern PVALUE __incr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __index(PNODE, TABLE, BOOLEAN *);
extern PVALUE __indi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __indiset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __inlist(PNODE, TABLE, BOOLEAN *);
extern PVALUE __inlist(PNODE, TABLE, BOOLEAN *);
extern PVALUE __inode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __insert(PNODE, TABLE, BOOLEAN *);
extern PVALUE __inset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __intersect(PNODE, TABLE, BOOLEAN *);
extern PVALUE __key(PNODE, TABLE, BOOLEAN *);
extern PVALUE __keysort(PNODE, TABLE, BOOLEAN *);
extern PVALUE __lastchild(PNODE, TABLE, BOOLEAN *);
extern PVALUE __le(PNODE, TABLE, BOOLEAN *);
extern PVALUE __length(PNODE, TABLE, BOOLEAN *);
extern PVALUE __lengthset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __level(PNODE, TABLE, BOOLEAN *);
extern PVALUE __linemode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __list(PNODE, TABLE, BOOLEAN *);
extern PVALUE __lock(PNODE, TABLE, BOOLEAN *);
extern PVALUE __long(PNODE, TABLE, BOOLEAN *);
extern PVALUE __lookup(PNODE, TABLE, BOOLEAN *);
extern PVALUE __lower(PNODE, TABLE, BOOLEAN *);
extern PVALUE __lt(PNODE, TABLE, BOOLEAN *);
extern PVALUE __male(PNODE, TABLE, BOOLEAN *);
extern PVALUE __marr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __menuchoose(PNODE, TABLE, BOOLEAN *);
extern PVALUE __mod(PNODE, TABLE, BOOLEAN *);
extern PVALUE __monthformat(PNODE, TABLE, BOOLEAN *);
extern PVALUE __moth(PNODE, TABLE, BOOLEAN *);
extern PVALUE __mul(PNODE, TABLE, BOOLEAN *);
extern PVALUE __name(PNODE, TABLE, BOOLEAN *);
extern PVALUE __namesort(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nchildren(PNODE, TABLE, BOOLEAN *);
extern PVALUE __ne(PNODE, TABLE, BOOLEAN *);
extern PVALUE __neg(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nestr(PNODE, TABLE, BOOLEAN *);
extern PVALUE __newfile(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nextfam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nextindi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nextsib(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nfamilies(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nl(PNODE, TABLE, BOOLEAN *);
extern PVALUE __not(PNODE, TABLE, BOOLEAN *);
extern PVALUE __nspouses(PNODE, TABLE, BOOLEAN *);
extern PVALUE __or(PNODE, TABLE, BOOLEAN *);
extern PVALUE __ord(PNODE, TABLE, BOOLEAN *);
extern PVALUE __outfile(PNODE, TABLE, BOOLEAN *);
extern PVALUE __pagemode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __pageout(PNODE, TABLE, BOOLEAN *);
extern PVALUE __parent(PNODE, TABLE, BOOLEAN *);
extern PVALUE __parents(PNODE, TABLE, BOOLEAN *);
extern PVALUE __parentset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __place(PNODE, TABLE, BOOLEAN *);
extern PVALUE __pn(PNODE, TABLE, BOOLEAN *);
extern PVALUE __pop(PNODE, TABLE, BOOLEAN *);
extern PVALUE __pos(PNODE, TABLE, BOOLEAN *);
extern PVALUE __prevfam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __previndi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __prevsib(PNODE, TABLE, BOOLEAN *);
extern PVALUE __print(PNODE, TABLE, BOOLEAN *);
extern PVALUE __program(PNODE, TABLE, BOOLEAN *);
extern PVALUE __push(PNODE, TABLE, BOOLEAN *);
extern PVALUE __pvalue(PNODE, TABLE, BOOLEAN *);
extern PVALUE __qt(PNODE, TABLE, BOOLEAN *);
extern PVALUE __reference(PNODE, TABLE, BOOLEAN *);
extern PVALUE __requeue(PNODE, TABLE, BOOLEAN *);
extern PVALUE __rjustify(PNODE, TABLE, BOOLEAN *);
extern PVALUE __roman(PNODE, TABLE, BOOLEAN *);
extern PVALUE __rot(PNODE, TABLE, BOOLEAN *);
extern PVALUE __row(PNODE, TABLE, BOOLEAN *);
extern PVALUE __save(PNODE, TABLE, BOOLEAN *);
extern PVALUE __savenode(PNODE, TABLE, BOOLEAN *);
extern PVALUE __set(PNODE, TABLE, BOOLEAN *);
extern PVALUE __setel(PNODE, TABLE, BOOLEAN *);
extern PVALUE __sex(PNODE, TABLE, BOOLEAN *);
extern PVALUE __short(PNODE, TABLE, BOOLEAN *);
extern PVALUE __sibling(PNODE, TABLE, BOOLEAN *);
extern PVALUE __siblingset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __soundex(PNODE, TABLE, BOOLEAN *);
extern PVALUE __space(PNODE, TABLE, BOOLEAN *);
extern PVALUE __spouseset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __stddate(PNODE, TABLE, BOOLEAN *);
extern PVALUE __strcmp(PNODE, TABLE, BOOLEAN *);
extern PVALUE __strlen(PNODE, TABLE, BOOLEAN *);
extern PVALUE __strsoundex(PNODE, TABLE, BOOLEAN *);
extern PVALUE __strtoint(PNODE, TABLE, BOOLEAN *);
extern PVALUE __sub(PNODE, TABLE, BOOLEAN *);
extern PVALUE __substring(PNODE, TABLE, BOOLEAN *);
extern PVALUE __surname(PNODE, TABLE, BOOLEAN *);
extern PVALUE __system(PNODE, TABLE, BOOLEAN *);
extern PVALUE __table(PNODE, TABLE, BOOLEAN *);
extern PVALUE __tag(PNODE, TABLE, BOOLEAN *);
extern PVALUE __titl(PNODE, TABLE, BOOLEAN *);
extern PVALUE __trim(PNODE, TABLE, BOOLEAN *);
extern PVALUE __trimname(PNODE, TABLE, BOOLEAN *);
extern PVALUE __union(PNODE, TABLE, BOOLEAN *);
extern PVALUE __uniqueset(PNODE, TABLE, BOOLEAN *);
extern PVALUE __unlock(PNODE, TABLE, BOOLEAN *);
extern PVALUE __upper(PNODE, TABLE, BOOLEAN *);
extern PVALUE __value(PNODE, TABLE, BOOLEAN *);
extern PVALUE __valuesort(PNODE, TABLE, BOOLEAN *);
extern PVALUE __version(PNODE, TABLE, BOOLEAN *);
extern PVALUE __wife(PNODE, TABLE, BOOLEAN *);
extern PVALUE __writefam(PNODE, TABLE, BOOLEAN *);
extern PVALUE __writeindi(PNODE, TABLE, BOOLEAN *);
extern PVALUE __xref(PNODE, TABLE, BOOLEAN *);
extern PVALUE __year(PNODE, TABLE, BOOLEAN *);

#endif /* _INTERP_PRIV_H */

