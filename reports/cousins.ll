/*
 * @progname       cousins.ll
 * @version        6.0
 * @author         Wetmore
 * @category       
 * @output         Text
 * @description    

  Finds the relationship between two persons in a
  LifeLines database.   If there is no common ancestor, the program
  will attempt to find a sequence of genetic relations that link the two
  persons.

  In the cases where the two persons are
  genetically related (have a common ancestor), the program will
  find and display the relationship.  If the two persons are not
  genetically related, the program will attempt to discover a
  sequence of genetic relationships that link the two persons.
  For example, the program will display the relationship between
  your two grandmothers as a sequence of two genetic relations with
  a grandchild as the link between.  Note that two persons may be
  related to each other in many ways; this program finds only the
  shortest one (or ones if there are different, equally short paths).

  This program requires version 3 of LifeLines.

author -- Tom Wetmore, ttw@beltway.att.com

version history
  1 - 08 Sep 1993 -- modified from the relate program
  2 - 08 Sep 1993 -- fixed niece/nephew bug
  3 - 09 Sep 1993 -- extensive modification
  4 - 13 Aug 1994 -- more modifications
  5 - 2 Mar 1995 -- check for direct descendants first.  Chris Bone
  6 - 2 Mar 1995 -- Neater cousin removes, find all short paths.  J.F. Chandler
*/

global(links)   /* table of links back one person */
global(rels)    /* table showing direction of the links */
global(klist)   /* list of found persons not linked back to yet */
global(numb)    /* number of persons considered so far */

/*======================================================================
 * notes on global data structures --
 *   o links -- implements the function link(key1) --> key2, where key1
 *     is the key of a person, and key2 is the key of the person that
 *     person key1 links back to
 *   o rels -- implements the function rels(key1) --> dir, where key1 is
 *     the key of a person, and dir is the relationship direction (up or
 *     down) between this person and person link(key1)
 *=====================================================================*/

/*=====================================================================
 * main -- Get the user to identify two persons; if all goes well, call
 *   relate to do the hard stuff.
 *===================================================================*/
proc main ()
{
	print("This program finds the relationship between two persons.\n\n")
	getindimsg(from, "Please identify the first person.")
	set(to, 0)
	if (from) {
		getindimsg(to, "Please identify the second person.")
	}
	if (and(from, to)) {
		print("Searching for relationships between:\n\t")
		print(name(from), " and ", name(to))
		print(".\n\nThis may take a while -- ")
		print("each dot is 25 persons considered.\n")
		set(fkey, save(key(from)))
		set(tkey, save(key(to)))
		call relate(tkey, fkey)
	} else {
		print("Please call again.\n")
	}
}

/*======================================================================
 * relate -- Attempt to find a relationship between two persons by
 *   constructing a path of parent and/or child links between them; if a
 *   path is found, call foundpath to display the results; else report
 *   that there is no relation between the persons.
 *====================================================================*/
proc relate (fkey, tkey)
{
	table(links)	/* table of links back one person */
	table(rels)
	list(klist)	/* keys of persons not linked back to yet */

	set(up, 1)
	set(down, neg(1))
	set(numb, 0)
	set(pathlength, 0)
	set(toolong, 0)
	set(found, 0)

/* Link the first person to him/herself with no direction, and make
   him/her the first entry in the list of unlinked back to persons.
   A "zero" person in the list marks the start of next-longer paths. */

	insert(links, fkey, fkey)
	insert(rels, fkey, 0)
	enqueue(klist, fkey)
	enqueue(klist, 0)

/* Iterate through the list of unlinked back to persons; remove them one by
   one; link their parents and children back to them; add their parents and
   children to the unlinked back to list; check each iteration to see if
   one of the new parents or children is the searched for person; if so
   quit the iteration and call foundpath; else continue iterating. */

	while ( gt(length(klist),1) ) {
		set(key, dequeue(klist))
		if(not(key)) {
			set(pathlength, add(1,pathlength))
			if(eq(pathlength,toolong)) { break() }
			enqueue(klist, 0)
			continue()
		}
		set(indi, indi(key))
		set(dir, lookup(rels, key))
		call include(key, father(indi), down)
		call include(key, mother(indi), down)
		families(indi, fam, spouse, num1) {
			children(fam, child, num2) {
				call include(key, child, up)
			}
		}
		if (key, lookup(links, tkey)) {
			if(found) {
				"\n\nAlternate relationship"
			} else {
				"Relationship from " name(indi(tkey))
				" to " name(indi(fkey))
			}
			":\n"
			set(found, 1)
			call foundpath(tkey)
			call fullpath(tkey)
			set(toolong, add(1,pathlength))
			insert(links, tkey, 0)
		}
	}

/* Check to see if there is no relation between the persons, and if there
   is none let the user know and quit. */

	if (not(found)) {
		print("\nThey are not blood-related to one another.")
		"They are not blood-related to one another."
	}
}

/*=========================================================================
 * include -- Links a newly discovered person (indi) back to another person
 *   (key), with a specified direction (rel); the new person is then put on
 *   the list of unlinked back to persons.
 *=======================================================================*/
proc include (key, indi, rel)
{
/* Only include the person if he/she has not been found before. */

	if (and(indi, not(lookup(links, key(indi))))) {

/* Keep user happy watching those dots! */

		set(numb, add(numb, 1))
		if (eq(0, mod(numb, 25))) {
			print(".")
		}

/* Update the data structures. */

		set(new, save(key(indi)))
		insert(links, new, key)
		insert(rels, new, rel)
		enqueue(klist, new)
	}
}

/*=================================================================
 * foundpath -- Show the relationship path between the two persons.
 *===============================================================*/
proc foundpath (key)
{
	print("\n") "\n"
	list(nexkeys)
	list(nexlens)

/* Init the nexus person from the first (to) person. */

	set(nexus, key)
	set(dir, lookup(rels, key))
	set(len, dir)
	set(again, 1)

/* Create the nexus list, the list of persons where relationships change
   direction; a nexus person is either the first person, the last person,
   or the common ancestor or descendent of two other nexus persons. */

	while (again) {

/* Get the next person from the path. */

		set(key, lookup(links, key))
		set(rel, lookup(rels, key))

/* If the new person's direction is 0 this is the last person in the path
   (the from person) so add the current nexus person and the last person
   to the nexus list and quit the loop */

		if (eq(0, rel)) {
			enqueue(nexkeys, nexus)
			enqueue(nexlens, len)
			enqueue(nexkeys, key)
			enqueue(nexlens, 0)
			set(again, 0)

/* if new person changes direction, add the current nexus person to the
   nexus list, and make the new person the new current nexus person */

		} elsif (ne(rel, dir)) {
			enqueue(nexkeys, nexus)
			enqueue(nexlens, len)
			set(nexus, key)
			set(dir, rel)
			set(len, rel)

/* if the new person continues in the same direction, record the step */

		} else {
			set(len, add(len, rel))
		}
	}

	set(one, dequeue(nexkeys))
	set(len, dequeue(nexlens))
	set(again, 1)

/* step down the nexus list, computing and displaying the relationships
   between either two nexus persons (where it is appropriate to show pure
   ancestry or descendency) or three nexus persons (where it is appropriate
   to show two nexus persons as cousins with their common ancestor) */

	while (and(again, length(nexkeys))) {
		set(llen, length(nexkeys))

/* If the initial direction is down, show first nexus person as a simple
   ancestor of the second nexus person; this condition can only be true
   in the first iteration */

		if (lt(len, 0)) {
			set(two, dequeue(nexkeys))
			set(new, dequeue(nexlens))
			call showancs(one, two, neg(len))
			set(one, two)
			set(len, new)

/* If the direction is up, there are two subcases: */

		} elsif (gt(len, 0)) {

/* If the nexus list has only one remaining entry then show the (last-1)th
   nexus person as a simple descendent of the last nexus person */

			if (eq(1, llen)) {
				set(two, dequeue(nexkeys))
				set(new, dequeue(nexlens))
				call showdesc(one, two, len)
				set(again, 0)

/* If the nexus list has more than one remaining entry then show the
   current nexus person and the next two as two "cousins" with an
   intervening common ancestor, and make the last of the three persons the
   current nexus person for the next loop iteration */

			} else {
				set(two, dequeue(nexkeys))
				set(tmp, dequeue(nexlens))
				set(three, dequeue(nexkeys))
				set(new, dequeue(nexlens))
				call showcous(one, two, three, len, neg(tmp))
				set(one, three)
				set(len, new)
			}

/* This is the special case where a person is related to him/herself. */

		} else {
			print("They're the same person.\n")
			"They're the same person.\n"
			set(again, 0)
		}
	}
}

/*=================================================
 * showancs -- Show a direct ancestry relationship.
 *==============================================*/
proc showancs (one, two, len)
{
	set(indi, indi(one))
	if (male(indi))	     { set(pword, "father ") }
	elsif (female(indi)) { set(pword, "mother ") }
	else		     { set(pword, "parent ") }
	if (eq(1, len))	     { set(aword, "the ") }
	else		     { set(aword, "a ") }

	print(name(indi), " is ", aword)
	name(indi) " is " aword
	if (eq(2, len))	   { print("grand") "grand" }
	elsif (eq(3, len)) { print("great grand") "great grand" }
	elsif (lt(3, len)) {
		print("great(", d(sub(len, 2)), ") grand")
		"great(" d(sub(len, 2)) ") grand"
	}
	print(pword, "of\n  ", name(indi(two)), ".\n")
	pword "of\n  " name(indi(two)) ".\n"
}

/*====================================================
 * showdesc -- Show a direct descendency relationship.
 *==================================================*/
proc showdesc (one, two, len)
{
	set(indi, indi(one))
	if (male(indi))	     { set(pword, "son ") }
	elsif (female(indi)) { set(pword, "daughter ") }
	else		     { set(pword, "child ") }

	print(name(indi), " is a ")
	name(indi) " is a "
	if (eq(2, len))	   { print("grand") "grand" }
	elsif (eq(3, len)) { print("great grand") "great grand" }
	elsif (lt(3, len)) {
		print("great(", d(sub(len, 2)), ") grand")
		"great(" d(sub(len, 2)) ") grand"
	}
	print(pword, "of\n  ", name(indi(two)), ".\n")
	pword "of\n  " name(indi(two)) ".\n"
}

/*=========================================================================
 * showcous -- Show a cousin relationship; for the purposes of this
 *   program, siblings, uncles, aunts, nieces and nephews are considered to
 *   be special cases of cousins.
 *=======================================================================*/
proc showcous (one, two, three, up, down)
{
	set(indi, indi(one))
	if (male(indi)) {
		set(sword, " brother ")
		set(nword, " nephew ")
		set(uword, " uncle ")
	} elsif (female(indi)) {
		set(sword, " sister ")
		set(nword, " niece ")
		set(uword, " aunt ")
	} else {
		set(sword, " sibling ")
		set(nword, " niece or nephew ")
		set(uword, " uncle or aunt ")
	}
	print(name(indi(one)), " is a")
	name(indi(one)) " is a"
	if (and(eq(up,1), eq(down, 1))) {	/* sibling cases */
		print(sword, "of")
		sword "of"
	} elsif (eq(up, 1)) {			/* uncle/aunt cases */
		if (eq(down, 2)) {
			print("n", uword, "of")
			"n" uword "of"
		} elsif (eq(down, 3)) {
			print(" great", uword, "of")
			" great" uword "of"
		} else {
			print(" great(", d(sub(down, 2)), ")", uword, "of")
			" great(" d(sub(down, 2)) ")" uword "of"
		}
	} elsif (eq(down, 1)) {			/* niece/nephew cases */
		if (eq(up, 2)) {
			print(nword, "of")
			nword "of"
		} elsif (eq(up, 3)) {
			print(" great", nword, "of")
			" great" nword "of"
		} else {
			print(" great(", d(sub(up, 2)), ")", nword, "of")
			" great(" d(sub(up, 2)) ")" nword "of"
		}
	} else {				/* cousin cases */
		if (gt(up, down)) {
			set(gen, down)
			set(rem, sub(up, down))
		} else {
			set(gen, up)
			set(rem, sub(down, up))
		}
		print(" ", ord(sub(gen,1)), " cousin ")
		" " ord(sub(gen,1)) " cousin "
		if (ne(rem, 0)) {
			if (eq(rem,1)) {print("once") "once"}
			elsif (eq(rem,2)) {print("twice") "twice"}
			elsif (eq(rem,3)) {print("thrice") "thrice"}
			else {
				print(card(rem), " times")
				card(rem) " times" }
			print(" removed ") " removed "
		}
		print("of") "of"
	}
	print("\n  ", name(indi(three)))
	"\n  " name(indi(three))
	print(", through their ancestor,\n  ", name(indi(two)), ".\n")
	", through their ancestor,\n  " name(indi(two)) ".\n"
}

/*=======================================================================
 * fullpath -- Show full path between the two persons.
 *======================================================================*/
proc fullpath (key)
{
	"\nThe full relationship path between them is:\n\n"
	set(again, 1)
	while (again) {
		name(indi(key))
		set(new, lookup(links, key))
		set(dir, lookup(rels, key))
		if (gt(dir, 0)) {
			" is the child of"
		}
		if (lt(dir, 0)) {
			" is the parent of"
		}
		"\n"
		if (eq(0, strcmp(key, new))) {
			set(again, 0)
		} else {
			set(key, new)
		}
	}
}
