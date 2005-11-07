/*
 * @progname       partition.ll
 * @version        11
 * @author         Eggert
 * @category       
 * @output         GEDCOM
 * @description    

This program partitions individuals in a database into disjoint
partitions.  A partition is composed of people related by one or more
multiples of the following relations: parent, sibling, child, spouse.
There is no known relationship between people in different partitions.


partition - a LifeLines database partitioning program
    by Jim Eggert (eggertj@atc.ll.mit.edu)
    Version 1,  19 November 1992 (unreleased)
    Version 2,  20 November 1992 (completely revamped using queues)
    Version 3,  23 November 1992 (added GEDCOM TRLR line,
                                  changed to key-based queues)
    Version 4,   1 December 1992 (slight code updates)
    Version 5,   9 January  1993 (added birth and death dates to full)
    Version 6,  30 January  1993 (now writes multiple GEDCOM output files)
        This version requires LifeLines v2.3.3 or later.
    Version 7,  21 September1994 (can partition about selected person)
    Version 8,  31 March    1995 (allow non-traditional families)
    Version 9,  23 February 1999 (changed to depth-first algorithm)
    Version 10, 24 September 2001(use cumcount, not forindi loop variable)
    Version 11, 7 November 2005  (add header, switch to use gengedcomstrong
                                  change output gedcom files to end in .ged
				  changes by Stephen A. Dum)

This program partitions individuals in a database into disjoint
partitions.  A partition is composed of people related by one or more
multiples of the following relations: parent, sibling, child, spouse.
There is no known relationship between people in different partitions.
You may select a particular person about whom to construct the largest
partition, or you may do the whole database.  The partitions are
written to the report in overview form or full form with the
partitions delimited by a
------------------------------------------------------------
long line, or in GEDCOM form to separate partition files.  The
overview form merely lists the number of people in each partition by
the number of hops from the first person found in the partition.
(They are found in order of the forindi iterator.)  The full form
lists each person in each partition, giving the number of hops, key,
name, and birth and death dates (if known).  The GEDCOM form writes
the partitions in GEDCOM format.  You will be prompted for a root
filename for the GEDCOM files; individual GEDCOM filenames will be of
the form root_filename.p, where p is the partition number.

Each allowed relationship (parent, sibling, child, spouse) is called a
hop, and the degree of relationship is called the hop count.  While
the program is processing, it displays to the screen the number of the
partition it is working on followed by a colon, then the cumulative
number of individuals in that partition for each hop increment.

*/

global(include_new)
global(plist)
global(hlist)
global(mark)
global(pset)
global(pcount)

global(hopcount)
global(prev_hopcount)
global(prev_pcount)
global(setcount)
global(cumcount)

proc include(person,hops,setcount,report_type)
{
    if (person) {
    set(pkey,key(person))
    if (lookup(mark,pkey)) {
        set(include_new,0)
    }
    else {
        set(pkey2,save(pkey))
        enqueue(plist,pkey2)
        enqueue(hlist,hops)
        insert(mark,pkey2,setcount)
        addtoset(pset,person,hops)
        incr(pcount)
        if (not(mod(pcount,100))) {
            print(d(pcount),"/",d(length(plist))," ")
        }
        set(include_new,1)
        if (eq(report_type,1)) {
        d(setcount) col(6) d(hops)
        col(11) pkey col(18) name(person)
        col(48) stddate(birth(person))
        col(62) stddate(death(person)) "\n"
        }
    }
    }
}

proc main ()
{
    table(mark)
    list(plist)
    list(hlist)
    indiset(pset)

    dayformat(0)
    monthformat(4)
    dateformat(0)

    getindimsg(person_root,
    "Enter a person for just one partition, nothing for all partitions:")
    getintmsg(report_type,
    "Enter 0 for overview, 1 for full, 2 for GEDCOM report:")
    if (eq(report_type,2)) {
	if (person_root) {
	    set(prompt,"Enter filename for GEDCOM partition:")
	}
	else {
	    set(prompt,"Enter root filename for GEDCOM partitions:")
	}
	getstrmsg(gedcom_root,prompt)
	set(gedcom_root,save(concat(gedcom_root,"_")))
    }
    else { set(gedcom_root,0) }

    set(setcount,1)
    set(pcount,0)
    set(hopcount,1)
    set(prev_hopcount,neg(1))
    set(prev_pcount,0)
    set(cumcount,0)
    if (eq(report_type,1)) {
    "Ptn  Hops Key    Person"
    col(48) "Birthdate" col(62) "Deathdate\n"
    }
    if (person_root) {
    call do_one_partition(person_root,report_type,gedcom_root)
    }
    else {
    forindi(person,num) {
        call do_one_partition(person,report_type,gedcom_root)
    }
    if (le(report_type,1)) {
        "Entire database contains " d(cumcount) " individual"
        if (gt(cumcount,1)) { "s" }
        " in " d(sub(setcount,1)) " partition"
        if (gt(setcount,2)) { "s" }
        ".\n"
    }
    }
}

proc do_one_partition(person,report_type,gedcom_root) {
    list(hopcountlist)
    call include(person,hopcount,setcount,report_type)
    if (include_new) {
    if (eq(report_type,0)) {
        "Ptn  Hops Individuals\n"
    }
    print("\n",d(setcount),": ")
    while (pkey,pop(plist)) {
        set(person,indi(pkey))
        set(hopcount,pop(hlist))
/*    print(pkey,d(hopcount)) */
        setel(hopcountlist,hopcount,add(1,getel(hopcountlist,hopcount)))
        incr(hopcount)

/* Look for family links and follow them to the families,
   then look for links to other individuals in those families.
   Nonstandard linking tags may be added here.
*/

        fornodes(inode(person),node) {
        set(t,tag(node))
        if (or(not(strcmp(t,"FAMS")),
               not(strcmp(t,"FAMC")))) {
            set(family,fam(value(node)))
            fornodes(fnode(family),node2) {
            set(t,tag(node2))
            if (or(not(strcmp(t,"HUSB")),
                   not(strcmp(t,"WIFE")),
                   not(strcmp(t,"CHIL")))) {
                call include(indi(value(node2)),
                    hopcount,setcount,report_type)
            }
            }
        }
        }
    }
    if (eq(report_type,0)) {
        forlist(hopcountlist,counter,hops) {
        print(d(setcount),d(counter), " ")
        }
    }
    if (le(report_type,1)) {
        "Partition " d(setcount) " contains " d(pcount)
        " individual"
        if (gt(pcount,1)) { "s" }
        ".\n"
        "------------------------------------------------------------\n"
    }
    if (eq(report_type,2)) {
        newfile(concat(gedcom_root,d(setcount),".ged"),0)
	/* output a gedcom header */
        "0 HEAD\n"
        "1 SOUR Lifelines\n"
	"2 VERS " version() "\n"
        "1 DEST ANY\n"
        "1 DATE " date(gettoday()) "\n"
        "1 SUBM\n"
        "1 GEDC\n"
        "2 VERS 5.5\n"
        "2 FORM LINEAGE-LINKED\n"
        "1 CHAR ASCII\n"
        gengedcomstrong(pset)
        "0 TRLR\n"
    }
    set(cumcount,add(cumcount,pcount))
    indiset(pset)
    set(pcount,0)
    set(hopcount,0)
    set(prev_hopcount,neg(1))
    set(prev_pcount,pcount)
    incr(setcount)
    }
}
