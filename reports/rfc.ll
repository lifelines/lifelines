/*
 * @progname       rfc.ll
 * @version        1995-09-08
 * @author         Paul B. McBride (pbm%cybvax0@uunet.uu.net)
 * @category       
 * @output         Text
 * @description

                   Royalty For Commoners format report

Requirements:
        LifeLines 3.0.2 or later (I hope)
        sour.li - SOUR processing subroutine library

Background:

This report program generates a report in a format similar to that
used in the book "Royalty for Commoners", Stuart, 1992, which attempts
to list all of the "known" ancestors of John of Gaunt. In this book
the furtherest back generation has the highest number, and there is
an attempt to keep generation numbers relatively consistant in different
lines.

The format is similar to that used in "Ancestral Roots of Certain
American colonists who came to America before 1700", Weis, 1992, except
that here the earliest generation in a line is generation number 1.

I also use this report program to generate a report for a range of
people between an ancestor and a descendant when exchanging info
with other people.

Prompts:

        Identify the ancestor (Optional)

                If you want a complete report of all of the ancestors
                of a person, or if you don't want a complete
                report, but the earliest ancestor has the same
                surname as the descendant, then just press return

        Identify the descendant

                If you didn't enter the ancestor, then you must enter
                the descendant to get a report.

        All ancestors (1 = yes, 0 = no)

                If you haven't entered the ancestor, then you will
                be asked this question. If you answer 0 (no), then
                the program will use the earliest ancestor in the
                paternal line.

        Number of Generations

                If you haven't entered the descendant, then the program
                will look for a descendant this many generations below.

        First Generation Number (default is 1)

                If you want generations to count upward as in "Anceatral
                Roots..." then enter 1.

                If you want generations to count downward as in "Royalty
                for Commoners", an educated guess is necessary here,
                or you may end up with negative generation numbers.
                An ancestorset() will be generated. This will contain
                minimum generation numbers. The generation number
                in the ancestor set will be used to adjust the generation
                number upward if you enter a number which is too small,
                but this may not be sufficient. For my database, I needed
                to increase that number by 10.

        Generations count downward (1) or upward (0)

                You are only asked this question if the first generation
                number is greater than 1.

Tags processed by the report

        tag     prefix

        TITL
        NOTE
        BIRT    b.
        CHR     bp.
        DEAT    d.
        BUR     bur.
        LIVE    lv.
        RESI    r.

SOUR record processing

        Source references are accumulated for each line and the
        REFN's are reported at the end of the line.
        At the end of the report all of the REFN's are listed
        along with the source details. See my SOUR routine
        library (sour.li) for more info.

Future Development:

        - rather than specifying a single descendant, allow entry of
          a group of descendants.
        - allow optional reporting of more SOUR detail associated with tags.
        - sort aliases
        - sort reference keys

Edit History:

08-sep-95 Paul B. McBride (pbm%cybvax0@uunet.uu.net)
*/

include("sour.li")

global(atable)
global(xtable)
global(aset)
global(xlen)
global(nalist)
global(nilist)
global(aliascnt)
global(indicnt)

global(allsour_table)
global(allsour_list)

global(allanc)
global(part)
global(gnum)
global(tset)

proc main ()
{
      table(allsour_table)
      list(allsour_list)

      indiset(iset)
      indiset(tset)
      indiset(uset)
      indiset(aset)
      table(atable)
      table(xtable)
      list(nalist)
      list(nilist)
      set(xlen, 0)
      set(aliascnt, 0)
      set(indicnt, 0)

      getindimsg(ancestor, "Identify the ancestor (Optional)")
      if(ancestor) {
        getindimsg(descendant,"Identify the descendant (Optional)")
      }
      else {
        getindimsg(descendant,"Identify the descendant (Required)")
      }
      set(allanc, 0)
      if(and(ne(descendant,0),eq(ancestor,0))) {
        getintmsg(allanc, "All Ancestors? (1 = yes, 0 = no)")
        set(ancestor, descendant)
        while(fath, father(ancestor)) {
          set(ancestor, fath)
        }
      }
      if(and(eq(descendant,0),ne(ancestor,0))) {
        getintmsg(gcount, "Number of Generations")
        set(descendant, ancestor)
        while(gcount, sub(gcount,1)) {
            set(cindi, 0)
            set(dindi, 0)
            families(descendant, fam, sps,  fnum) {
                if(gt(nchildren(fam),0)) {
                   children(fam, child, cnum) {
                     if(eq(cindi, 0)) { set(cindi, child) }
                     families(child, chfam, chsps,  chfnum) {
                       if(gt(nchildren(chfam),0)) {
                         set(dindi, child)
                         break()
                       }
                     }
                     if(ne(dindi, 0)) { break() }
                   }
                }
                if(ne(dindi, 0)) { break() }
            }
            if(dindi) { set(descendant, dindi) }
            elsif (cindi) {
                set(descendant, cindi)
                break()
            }
            else { break() }
        }
      }
      if(and(ne(ancestor, 0),ne(descendant,0))) {
        getintmsg(gnum, "First Generation Number (default is 1)")
        if(le(gnum,0)) { set(gnum,1) }
        set(down, 0)
        if(gt(gnum,1)) {
          getintmsg(down, "Generations count downward (1) or upward (0)")
        }
        set(firstgen, gnum)
        if(descendant) {
          /* output a line so that output file prompt will appear before
             the ancestor set is generated because it can take a long
             time.
           */
          if(allanc) {
            print("All Ancestors of ", name(descendant), nl())
            "All Ancestors of " name(descendant) nl()
          }
          else {
            print("Descendants of ", name(ancestor),
                " who are ancestors of ", name(descendant), nl())
            "Descendants of " call titledname(ancestor) nl()
            "  who are ancestors of " call titledname(descendant) nl()
          }
          /* find all the people of interest */
          print("Finding Ancestors... ")
          addtoset(iset, descendant, 0)
          set(tset, ancestorset(iset))
          deletefromset(iset, descendant, 1)
          print(d(lengthset(tset)), nl())

          if(allanc) {
            set(uset, tset)
          }
          else {
            print("Finding Descendants... ")
            addtoset(iset, ancestor, 0)
            set(uset, descendantset(iset))
            deletefromset(iset, ancestor, 1)
            print(d(lengthset(uset)), nl())
          }
          set(aset, intersect(tset, uset))
          addtoset(aset, ancestor, 0)
          addtoset(aset, descendant, 0)
          print("Generating Report for ",
                d(lengthset(aset)), " people")

          list(ilist)
          list(alist)
          list(plist)
          list(glist)

          set(part, 0)
          set(acount, 0)

         while(1) {
          if(allanc) {
            set(maxgen, 0)
            set(ancestor, 0)
            forindiset(tset, indi, ival, icnt) {
              if(or(eq(maxgen, 0),gt(ival,maxgen))) {
                set(maxgen, ival)
                set(ancestor, indi)
              }
            }
            if(eq(ancestor, 0)) { break() }

            if(and(ne(down,0), le(firstgen, maxgen))) {
              set(firstgen, add(maxgen, 1))
            }
            set(gnum, findgen(ancestor, down, firstgen, eq(acount,0)))
            print(nl(), name(ancestor), " ", d(add(part,1)),"-",d(gnum),". ",
                  d(lengthset(tset)), " remaining")
          }
          enqueue(alist, ancestor)
          enqueue(plist, 0)
          enqueue(glist, gnum)
          set(acount, add(acount, 1))
          while(aindi, dequeue(alist)) {
           print(".")
           nl()
           call sour_init()
           set(pnum, dequeue(plist))
           set(part, add(part, 1))
           set(gnum, dequeue(glist))
           "Line " d(part)
           if(pnum) {
             " from Line " d(pnum) " above."
           }
           /* if we are doing all of the ancestors, then start each line
              as far back as possible..
            */
           if(allanc) {
             set(changed, 0)
             while(1) {
               if(fath, father(aindi)) {
                 if(lookup(atable, key(fath))) { break() }
                 if(moth, mother(aindi)) {
                   if(eq(lookup(atable, key(moth)),0)) {
                     if(and(eq(father(fath),0),eq(mother(fath),0))) {
                       if(or(ne(father(moth),0),ne(mother(moth),0))) {
                         set(fath, moth)
                       }
                     }
                   }
                 }
                 set(tindi, aindi)
                 set(aindi, fath)
               }
               elsif(moth, mother(aindi)) {
                 if(lookup(atable, key(moth))) { break() }
                 set(tindi, aindi)
                 set(aindi, moth)
               }
               else { break() }
               print("+")
               if(eq(changed, 0)) {
                  set(changed, 1)
                  " [" name(tindi) " " d(pnum) "-" d(gnum) "]"
               }
               if(down) { set(gnum, add(gnum,1)) }
               else     { set(gnum, sub(gnum,1)) }
             }
           }
           nl() nl()
           enqueue(ilist, aindi)
           while(indi, dequeue(ilist)) {
            /* upper(roman(gnum)) */
            call addtoindex(indi, part, gnum)
            if(allanc) { deletefromset(tset, indi, 1) }
            d(gnum) ". " call titledname(indi) nl()
            set(tnum, lookup(atable, key(indi)))
            if(ne(tnum,0)) {
              "   [See Line " d(div(tnum,1000))
                  " Generation " d(mod(tnum,1000)) " above]" nl()
              continue()
            }
            insert(atable, save(key(indi)), add(mul(part,1000), gnum))
            call sour_addind(indi)
            call allnotes(indi, 8)
            call allplaces(indi, 5)
            /* set(bdate, "")
             * set(ddate, "")
             * if (eb, birth(indi)) { set(bdate,save(long(eb))) }
             * if (ed, death(indi)) { set(ddate,save(long(ed))) }
             * set(prefix, "    ")
             * if (strlen(bdate)) { prefix "b. " bdate nl() }
             * if (strlen(ddate)) { prefix "d. " ddate nl() }
             */
            set(desc, 0)
            set(nfam, nfamilies(indi))
            families(indi, fam, sps,  fnum) {
                if(sps) {
                   call sour_addind(sps)
                   call addtoindex(sps, part, gnum)
                   if(allanc) { deletefromset(tset, sps, 1) }
                   if(eq(nfam,1)) { "    m. " }
                   else           { "    m(" d(fnum) ") " }
                   call titledname(sps)
                   if (e, marriage(fam)) { " " long(e) }
                   nl()
                   set(bdate, "")
                   set(ddate, "")
                   if (eb, birth(sps)) { set(bdate,save(long(eb))) }
                   if (ed, death(sps)) { set(ddate,save(long(ed))) }
                   set(prefix, "       ")
                   if (strlen(bdate)) { prefix "b. " bdate nl() }
                   if (strlen(ddate)) { prefix "d. " ddate nl() }
                   set(findi, father(sps))
                   set(mindi, mother(sps))
                   if(or(findi, mindi)) {
                     "       "
                     if(male(sps)) { "son of " }
                     else { "daughter of " }
                     if(findi) {
                       call addtoindex(findi, part, gnum)
                       if(allanc) { deletefromset(tset, findi, 1) }
                       call titledname(findi)
                       call simplefam(findi, ne(mindi,0))
                       if(mindi) { " and " }
                     }
                     if(mindi) {
                       call addtoindex(mindi, part, gnum)
                       if(allanc) { deletefromset(tset, mindi, 1) }
                       call titledname(mindi)
                       call simplefam(mindi, 0)
                     }
                     nl()
                   }
                }
                if(gt(nchildren(fam),0)) {
                   if(eq(nfam,1)) { "    ch:   " }
                   else           { "    ch(" d(fnum) ") " }
                   set(needindent, 0)
                   children(fam, child, cnum) {
                        set(altdesc,0)
                        set(mcnum,mod(sub(cnum,1),4))
                        if(gt(cnum,1)) {
                           if(eq(mcnum,0)) { set(needindent,1) }
                        }
                        if(needindent) {
                            "," nl() "          "
                            set(needindent,0)
                        }
                        else {
                          if(gt(mcnum,0)) { ", "}
                        }
                        /* mark each child which is an ancestor with a "*",
                           but only use the first at the next generation.
                         */
                        set(seeabove, 0)
                        if(eq(child,descendant)) {
                              "*"
                              set(seeabove, lookup(atable, key(child)))
                              if(eq(seeabove, 0)) {
                                if(eq(desc,0)) {
                                  enqueue(ilist, child)
                                  set(desc,1)
                                }
                              }
                        }
                        else {
                           addtoset(iset, child, 0)
                           set(jset, intersect(aset, iset))
                           if(ne(lengthset(jset),0)) {
                              "*"
                              set(seeabove, lookup(atable, key(child)))
                              if(eq(seeabove,0)) {
                                if(eq(desc,0)) {
                                  enqueue(ilist, child)
                                  set(desc,1)
                                }
                                else {
                                  set(altdesc,1)
                                }
                              }
                             deletefromset(jset, child, 1)
                           }
                           deletefromset(iset, child, 1)
                           /*
                            forindiset(aset, ancestor, junkval, junknum) {
                            if(eq(child, ancestor)) {
                              "*"
                              if(eq(desc,0)) {
                                enqueue(ilist, child)
                                set(desc,1)
                              }
                              else {
                                set(altdesc,1)
                              }
                              break()
                            }
                            }
                           */
                        }
                        if(ne(strcmp(surname(child),
                                     surname(father(child))),0)) {
                               name(child)
                        }
                        else { givens(child) }
                        if(seeabove) {
                          call addtoindex(child, part, gnum)
                          " [See Line " d(div(seeabove,1000))
                          " Generation " d(mod(seeabove,1000)) " above]"
                          set(needindent, 1)
                        }
                        if(eq(altdesc,1)) {
                          if(down) { set(tnum, sub(gnum, 1)) }
                          else     { set(tnum, add(gnum, 1)) }
                          enqueue(alist, child)
                          enqueue(plist, part)
                          enqueue(glist, tnum)
                          set(acount, add(acount,1))
                          " [See Line " d(acount)
                          " Generation " d(tnum) " below]"
                          set(needindent, 1)
                        }
                   }
                   nl()
                }
            }
            if(down) { set(gnum, sub(gnum, 1)) }
            else     { set(gnum, add(gnum, 1)) }
          }
          if(sour_exists()) {
            nl() "References: "
            call sour_see(",", 70, 13)
            call sour_save(allsour_table, allsour_list)
            nl()
          }
         }
         if(eq(allanc,0)) { break() }
         }
        }
        /* list all references */
        call sour_restore(allsour_table, allsour_list)
        if(sour_exists()) {
            nl() "Key to References:" nl() nl()
            call sour_ref(10)
        }
        /* generate an index */
        call reportindex()
        call reportalias()
      }
}

/* report the index */

proc reportindex()
{
        print(nl(), "Index: ", d(lengthset(aset)), " people, ")
        print(d(xlen), " entries...")
        nl() "Index" nl() nl()
        namesort(aset)
        forindiset(aset, indi, ival, inum) {
          if(xref, lookup(xtable, key(indi))) {
            surname(indi) ", " givens(indi)
            col(30) key(indi)
            col(40) xref nl()
          }
        }
}

/* add to the index */

proc addtoindex(indi, part, gnum)
{
        if(xref, lookup(xtable, key(indi))) {
          set(xref, save(concat(xref, ",", save(d(part)), "-", save(d(gnum)))))
        }
        else {
          set(xref, save(d(part)))
          set(xref, save(concat(xref, "-", save(d(gnum)))))
          set(xlen, add(xlen, 1))
        }
        insert(xtable, save(key(indi)), xref)
}

/* report all of a person's titles */

proc titles(i)
{
        fornodes (inode(i), n) {
                if (eqstr(tag(n), "TITL")) {
                        value(n) " "
                }
        }
}

proc titledname(i)
{
        fornodes (inode(i), n) {
                if (eqstr(tag(n), "TITL")) {
                  if(or(eqstr(value(n), "Sir"),
                        eqstr(value(n),"Rev."))) {
                        value(n) " "
                  }
                }
        }
        name(i)
        fornodes (inode(i), n) {
                if (eqstr(tag(n), "TITL")) {
                  if(not(or(eqstr(value(n), "Sir"),
                        eqstr(value(n),"Rev.")))) {
                        " " value(n)
                  }
                }
        }
}

/* report all places */

proc allplaces(person, colnum)
{
      traverse(inode(person), node, lev) {
        set(prefix, "")
        if (eqstr(tag(node),"RESI")) { set(prefix, "r. ") }
        elsif (eqstr(tag(node),"LIVE")) { set(prefix, "lv. ") }
        elsif (eqstr(tag(node),"BIRT")) { set(prefix, "b. ") }
        elsif (eqstr(tag(node),"CHR")) { set(prefix, "bp. ") }
        elsif (eqstr(tag(node),"DEAT")) { set(prefix, "d. ") }
        elsif (eqstr(tag(node),"BURI")) { set(prefix, "bur. ") }
        if(gt(strlen(prefix), 0)) {
           set(edate,save(long(node)))
           if (strlen(edate)) {
               if(gt(colnum, 0)) { col(colnum) }
               prefix edate nl()
           }
        }
     }
}

/* report all notes */

proc allnotes(person, colnum)
{
        fornodes(inode(person), node) {
                if (eq(0,strcmp("NOTE", tag(node)))) {
                        if(gt(colnum, 0)) { col(colnum) }
                        value(node) nl()
                        fornodes(node, subnode) {
                                if (eq(0,strcmp("CONT", tag(subnode)))) {
                                        if(gt(colnum, 0)) { col(colnum) }
                                        value(subnode) nl()
                                }
                        }
                }
        }
}

/* report aliases */

proc reportalias()
{
        print(nl(), "Aliases...")
        nl() "Alias" col(30) "Key" col(40) "Name" nl() nl()

        /* assume that the set is already sorted. see reportindex() */

        forindiset(aset, indi, ival, inum) {
          set(count, 0)
          fornodes(inode(indi), subnode){
            if(eqstr(tag(subnode), "NAME")){
              incr(count)
              if(ge(count, 2)){
                list(np)
                extractnames(subnode, np, nc, sc)
                /* process the surname first */
                if(sc) {
                   set(sn, getel(np, sc))
                   if(eq(strlen(sn), 0)) { "____," }
                   else { sn "," }
                }
                else   { "____," }
                /* process the rest of the name */
                forlist(np, v, i) {
                  if(ne(i, sc)) { " " v }
                }
                col(30) key(indi)
                col(40)
                surname(indi) ", " givens(indi)
                nl()
              }
            }
          }
        }
}

/* output the parents of a person if it is a simple family where the
   father and mother have only one family and this is their only
   child, and their parents are not known.
 */

proc simplefam(indi, indent)
{
        set(findi, father(indi))
        set(mindi, mother(indi))
        set(simple, or(ne(findi,0), ne(mindi,0)))
        if(simple) {
          if(findi) {
            if(or(father(findi), mother(findi))) { set(simple,0) }
            elsif(ne(nfamilies(findi),1)) { set(simple,0) }
            else {
              families(findi, fam, sps, fnum) {
                if(ne(nchildren(fam),1)) { set(simple, 0) }
              }
            }
          }
        }
        if(simple) {
          if(mindi) {
            if(or(father(mindi), mother(mindi))) { set(simple,0) }
            elsif(ne(nfamilies(mindi),1)) { set(simple,0) }
            else {
              families(mindi, fam, sps, fnum) {
                if(ne(nchildren(fam),1)) { set(simple, 0) }
              }
            }
          }
        }
        if(simple) {
          nl() "            ["
          if(male(indi)) { "son of " }
          else { "daughter of " }
          if(findi) {
            call addtoindex(findi, part, gnum)
            if(allanc) { deletefromset(tset, findi, 1) }
            call titledname(findi)
            if(mindi) { nl() "                 and " }
          }
          if(mindi) {
            call addtoindex(mindi, part, gnum)
            if(allanc) { deletefromset(tset, mindi, 1) }
            call titledname(mindi)
          }
          "]"
          if(indent) { nl() "       " }
        }
}

/* find the generation number for an individual */

func findgen(aindi, down, maxgen, first)
{
        list(tilist)
        indiset(tiset)
        indiset(tjset)

        enqueue(tilist, aindi)
        set(gnum, 0)
        set(tnum, 0)
        if(eq(first,0)) {
           while(indi, dequeue(tilist)) {
            set(tnum, lookup(atable, key(indi)))
            if(ne(tnum,0)) {
                  call dumpindi("person", indi, tnum, gnum)
                  set(tnum, mod(tnum,1000))
                  break()
            }
            set(desc, 0)
            families(indi, fam, sps,  fnum) {
                if(sps) {
                  set(tnum, lookup(atable, key(sps)))
                  if(ne(tnum,0)) {
                    call dumpindi("spouse", sps, tnum, gnum)
                    set(tnum, mod(tnum,1000))
                    break()
                  }
                }
                if(gt(nchildren(fam),0)) {
                   children(fam, child, cnum) {
                        set(tnum, lookup(atable, key(child)))
                        if(ne(tnum,0)) {
                          set(gnum, add(gnum, 1))
                          call dumpindi("child", child, tnum, gnum)
                          set(tnum, mod(tnum,1000))
                          break()
                        }
                        if(eq(desc,0)) {
                          addtoset(tiset, child, 0)
                          set(tjset, intersect(aset, tiset))
                          deletefromset(tiset, child, 1)
                          if(ne(lengthset(tjset),0)) {
                            deletefromset(tjset, child, 1)
                            set(desc, 1)
                            enqueue(tilist, child)
                          }
                        }
                   }
                }
                if(tnum) { break() }
            }
            if (tnum) { break() }
            set(gnum, add(gnum, 1))
          }
        }
        set(ngen, 0)
        if(tnum) {
          if(down) {
            set(ngen, add(tnum, gnum))
          }
          else {
            set(ngen, sub(tnum, gnum))
          }
        }
        if(down) {
          set(ogen, maxgen)
        }
        else {
          set(ogen, 1)
        }
        if(eq(ngen, 0)) { set(ngen, ogen) }
        return(ngen)
}

/* dump a previously referenced individual to show basis of generation
   number of new line
 */

proc dumpindi(type, indi, tnum, gnum)
{
        nl()
        "...The generation numbers of the next line are based on " type nl()
        "   " name(indi)
        " " d(div(tnum,1000)) "-" d(mod(tnum,1000))
        " " d(gnum) " generations below" nl()
}
