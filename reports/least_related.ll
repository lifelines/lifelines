/*
 * @progname       least_related.ll
 * @version        8.1
 * @author         Jim Eggert
 * @category       
 * @output         Text
 * @description    

Calculates pairs of individuals in a database that are related to each other,
but are least related to each other.  The degree of relation is given by the
number of steps it takes to go from one person to another.  It takes one step
to go from a person to his/her parent, spouse, sibling, or child.  If several
pairs tie for least related status, all are output.  Each pair is actually
output twice, once in each direction.  If the database contains mutually
unrelated partitions, a warning is generated, but the results are still correct.

Each step in the relationship (or hop) is coded as follows, with the
capital letters denoting a person of unknown gender:
    father   f
    mother   m
    parent   P (unknown or non-matching gender)
    brother  b
    sister   s
    sibling  S (unknown gender)
    son      z (sorry)
    daughter d
    child    C (unknown gender)
    husband  h
    wife     w
    spouse   O (unknown or non-matching gender)

Note that some of these are individually (h w O), or can be in combination (e.g., zm),
non-blood relations.  Thus the least-related pairs may not be related by blood at all,
only by marriage.  In any large database, combinations of marriages and blood
relationships are to be expected in the least-related pairs.

In graph theoretical terms, this program calculates and describes the diameter of the
relationship graph with each relationship step counted as distance 1.

Version 1,  5 Jun 1994
Version 2,  5 Jun 1994 added generational width
Version 3, never released.  Versions 1-3 can produce incorrect output.
Version 4, 10 Nov 1995 changed algorithms
Version 5,  6 Nov 2017 New algorithm to handle larger databases
Version 6, 12 May 2020 Faster exclusion algorithm
Version 7, 14 May 2020 Better first guess makes much more effective exclusion
Version 8, 15 May 2020 Outputs list of IDs in path descriptions
Version 8.1,  17 May 2020 Bug fixes for first persons being loners and to handle long paths

*/

proc main() {
  set(dmax, 0) /* least-related distance */
  list(froms)  /* from-to pairs of keys of least related individuals */
  list(tos)
  list(rels)   /* relationship strings of least related individuals */
  list(idrels) /* id strings for least related paths */
  set(nextip, 0)
  table(excluded)  /* people who are provably not least-related */
  table(links)     /* people whose least_related path length was explicitly calculated */
  set(exclude_count,0)

  forindi(person, ip) {
    set(database_size, ip)  /* Count people in database */
  }

  print("Please be patient.  The calculation eventually speeds up.  Status updates output in format I(D P -X), where\n",
        "  I = number of persons completed (out of ", d(database_size), " in the database),\n",
	"  D = maximum distance found so far,\n",
	"  P = number of pairs at current max distance, and\n",
	"  X = number of excluded persons (speeds up calculation)\n")

  set(furthest_personk, 0)
  set(first_nonloner, 1)

  forindi(person, ip) {
    set(personk, key(person))
    if (not(lookup(excluded, personk))) { /* person has not been excluded on the basis of proximity */
/* Find people related to this person, retaining those who are at least as distant as those found already */
      list(pklist)  /* queue of keys of people related to person to be processed*/
      list(dlist)   /* queue of distances from the person to be processed */
      list(rlist)   /* queue of relationship strings */
      list(idrlist) /* queue of id paths */
      list(prlist)  /* list of keys of all people related to this person processed so far */
      list(drlist)  /* corresponding list of distances from this person */
      table(seen)   /* lookup table for people known related to this person */
      enqueue(pklist, personk)
      enqueue(dlist, 0)
      enqueue(rlist, "")
      enqueue(idrlist, personk)
      set(pcounter, 0)
      set(proceed, 1)
      set(dnew, 0)
      while (pk, dequeue(pklist)) { /* process next related person */
        incr(pcounter)
        set(dnext, dequeue(dlist))
        set(r, dequeue(rlist))
	set(idr, dequeue(idrlist))
	enqueue(prlist, pk)
	enqueue(drlist, dnext)
	if (dlink, lookup(links, pk)) { /* The related person has a least_related length already, so only go further if needed */
	  if (gt(dlink, dmax)) { /* This should never happen */
	    print("\nError!  Output not reliable!   dl>dm! ", d(dlink), ">", d(dmax),  " ", d(length(links)), "\n")
	  }
	  if (lt(dlink, dnext)) { /* This should never happen */
	    print("\nError!  Output not reliable!   dl<dn! ", d(dlink), ">", d(dnext), " ", d(length(links)), "\n")
	  }
	  if (lt(add(dlink, dnext), dmax)) { /* Some persons related to this person can be excluded, and this person is done */
	    while (excludablepk, dequeue(prlist)) {
	      if (lt(add(dlink, dnext, dequeue(drlist)), dmax)) {
	        insert(excluded, excludablepk, 1)
	      }
	    }
	    while (excludablepk, dequeue(pklist)) { /* Note that this will empty pklist, which will cause the end of the enclosing while loop also */
	      if (lt(add(dlink, dnext, dequeue(dlist)), dmax)) {
	        insert(excluded, excludablepk, 1)
	      }
	      set(junk, dequeue(rlist))
	      set(junk, dequeue(idrlist))
	    }
	    set(proceed, 0)
	  }
	}
	if (proceed) {
          fornodes(inode(indi(pk)), node) { /* look for relations */
	    if (ni, index(" FAMS FAMC ", upper(tag(node)), 1)) { /* found a family, value is 2 or 7 */
	      fornodes(fnode(fam(value(node))), subnode) {                      /* look for other family members */
	        if (sni, index(" HUSB  WIFE  CHIL ", upper(tag(subnode)), 1)) { /* found a family member */
	          set(subp, indi(value(subnode)))
	          set(subpk, key(subp))
	          if (not(lookup(seen, subpk))) { /* haven't seen that person yet */
	            if(eq(ni,7)) { decr(sni) } /* Calculate relationship code */
		    if (male(subp)) { incr(sni, 2) } elsif (female(subp)) { incr(sni, 4) }
/* find code in magic string with offset " HUSB  WIFE  CHIL " */
		    set(rstep, substring("POfhPOPOPOmwSCbzsd", sni, sni))
		    set(rel, concat(r, rstep))
		    set(idrel, concat(idr, " ", subpk))
		    enqueue(rlist, rel)               /* keep track of relationship path */
		    enqueue(idrlist, idrel)
	            enqueue(pklist, subpk)              /* so enqueue them */
		    set(dnew, add(dnext,1))
		    enqueue(dlist, dnew)             /* in a breadth-first search */
		    insert(seen, subpk, 1)        /* and mark the person as seen */
		    if (gt(dnew, dmax)) {  /* new record furthest relation */
		      while (pop(froms)) {  /* so empty out previous furthest lists */
		        set(loser, pop(tos))
		        set(loser, pop(rels))
			set(loser, pop(idrels))
		      }
		      set(dmax, dnew)      /* and set new record distance */
		      set(furthest_personk, subpk)
		    }
		    if (eq(dnew, dmax)) { /* also always true if new record was just set */
		      enqueue(froms, personk)
		      enqueue(tos,   subpk)
		      enqueue(rels,  rel)
		      enqueue(idrels, idrel)
		    }
		  }
                }
	      }
	    }
	  }
	}
      }

      if (eq(ip,1)) {
        if (lt(pcounter, database_size)) { /* Warn if we didn't crawl through the whole database on the first person */
          print("\nWarning:  Database contains more than one partition.\nLeast-related pairs are found within partitions and are reported globally, not per partition.\n\n")
        }
      }

      if (eq(ip, first_nonloner)) {
        /* Find the furthest person from the furthest person from the first nonloner person (usually the first person), to improve exclusion */
	if (furthest_personk) {
          table(seen)  /* lookup table for people known related to this person */
          enqueue(pklist, furthest_personk)
          enqueue(dlist, 0)
          set(dnnew, 0)
          while (pk, dequeue(pklist)) { /* process next related person */
            set(dnext, dequeue(dlist))
            fornodes(inode(indi(pk)), node) { /* look for relations */
	      if (ni, index(" FAMS FAMC ", upper(tag(node)), 1)) { /* found a family, value is 2 or 7 */
	        fornodes(fnode(fam(value(node))), subnode) {                      /* look for other family members */
	          if (sni, index(" HUSB  WIFE  CHIL ", upper(tag(subnode)), 1)) { /* found a family member */
	            set(subpk, key(indi(value(subnode))))
	            if (not(lookup(seen, subpk))) { /* haven't seen that person yet */
	              enqueue(pklist, subpk)              /* so enqueue them */
		      set(dfnew, add(dnext,1))
		      enqueue(dlist, dfnew)             /* in a breadth-first search */
		      insert(seen, subpk, 1)        /* and mark the person as seen */
		      if (gt(dfnew, dmax)) {  /* new record furthest distance only */
		        /* Note that the furthest_person will be checked later anyway, and this pair can't include the initial person */
		        set(dmax, dfnew)       /* set new record distance */
		        while (pop(froms)) {  /* and empty out previous furthest lists */
		          set(loser, pop(tos))
		          set(loser, pop(rels))
			  set(loser, pop(idrels))
			}
		      }
		    }
		  }
                }
	      }
	    }
	  }
          insert(links, furthest_personk, dmax)
	} else {
	  incr(first_nonloner)
        }
      }

      if (and(proceed, dnew)) {
      /* No shortcut link found, so dnew contains the largest distance from this person */
        insert(links, personk, dnew)
        print(d(ip), "(", d(dmax), " ", d(length(froms)), " -", d(length(excluded)), ") ")
      }

    }
  }

  print("done\nWriting results...")
  if (ge(first_nonloner, database_size)) {
    "There are no relations in this database!\n"
  } else {
    "Longest relation distance " d(dmax) " steps found in " d(length(froms)) " pairs:\n\n"
    while (from,dequeue(froms)) {
      set(to,dequeue(tos))
      "from " from " " name(indi(from)) "\n"
      "  to " to   " " name(indi(to)) "\n"
      " rel " dequeue(rels) "\n"
      " ids " dequeue(idrels) "\n\n"
    }
  }
  print("done\n")
}
