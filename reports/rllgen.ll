/*
 * @progname       rllgen.ll
 * @version        1.0
 * @author         Eggert
 * @category       
 * @output         RLL format
 * @description    

A LifeLines report program to aid in the generation of
Roots Location List (RLL) submissions.
Given a person, this generates a RLL-like submission for that person and
his/her ancestors.  The output will likely need considerable hand editing,
but that is how it is.  If you need to know what the RLL is, I have enclosed
a description at the end of this file.


Version 1, 18 November 1994, by Jim Eggert, eggertj@ll.mit.edu
                                Requires LifeLines 3.0.1 or higher

This program will work better if you follow strict placename
conventions.  You should probably run the places report program first
to see if your placenames are in good shape.

Here's what you will need to do by hand (you can consider this a list
of desired features for future versions of this report program):

BEFORE YOU RUN THE PROGRAM:

Change the routine write_rll_header() to use your submitter tag, name,
and address.  You may also want to change the personset calculation in
the beginning of the main routine.

AFTER YOU RUN THE PROGRAM:

Sort the location portion of the output file.
Eliminate empty or useless location lines.
Use RLL-standard abbreviations for placenames.
  - get FAMILY ABBREV as per instructions at end of this file.
Combine duplicate location lines where appropriate.
Check check check.
Send the final product to the RLL maintainer.
  - see the end of this file.

*/

global(placefirst)
global(placelast)
global(placelist)
global(submitter_tag)

/* write_rll_header sets the submitter tag and
   writes a little header for the RLL list maintainer
 */

proc write_rll_header() {
    set(submitter_tag,"jqpublic")

    "Roots Location List (RLL) submission of " date(gettoday())
    " by John Q. Public\n\n"
    submitter_tag
    col(12) "John Q. Public, jqpublic@my.node.address\n"
    col(12) "1234 North Maple, Homesville, OX 12345-6789, USA\n\n"
}


proc addplace(node) {
    set(placename,save(value(node)))
    set(pyear,atoi(year(parent(node))))
    if (not(pyear)) { set(pyear,neg(1)) }
    set(firstyear,lookup(placefirst,placename))
    if (and(gt(firstyear,0),gt(pyear,0))) {
        set(lastyear,lookup(placelast,placename))
        if (lt(pyear,firstyear)) { insert(placefirst,placename,pyear) }
        elsif (gt(pyear,lastyear)) { insert(placelast,placename,pyear) }
    }
    if (and(lt(firstyear,0),gt(pyear,0))) {
        insert(placefirst,placename,pyear)
        insert(placelast,placename,pyear)
    }
    if (eq(firstyear,0)) {
        insert(placefirst,placename,pyear)
        insert(placelast,placename,pyear)
        enqueue(placelist,placename)
    }
}

/* write_rll_entry writes one line in the rll submission */

proc write_rll_entry(placename) {
    list(tokenlist)
    set(firstyear,lookup(placefirst,placename))
    set(lastyear,lookup(placelast,placename))
    extracttokens(placename,tokenlist,ntokens,",")
    set(comma,0)
    while(token,pop(tokenlist)) {
        if(comma) { "," }
        token
        set(comma,1)
    }
    " "
    if (gt(firstyear,0)) {
        d(firstyear)
        if (gt(lastyear,firstyear)) { "-" d(lastyear) }
        " "
    }
    submitter_tag "\n"
}


proc main() {
    table(placefirst)
    table(placelast)
    indiset(personset)
    list(placelist)

    getindi(person)
    print("Forming set...")
    addtoset(personset,person,0)
    set(personset,ancestorset(personset))
    addtoset(personset,person,0)
    print("done\nComputing places...")
    set(nextpnum,0)

    forindiset (personset, person, pval, pnum) {
        if (ge(pnum,nextpnum)) {
            print(" ",d(pnum))
            set(nextpnum,add(nextpnum,100))
        }
        traverse (inode(person), node, level) {

            if (eq(strcmp(tag(node), "PLAC"), 0)) { call addplace(node) }
        }

        families (person, fam, sp, fnum) {
            if (or(not(husband(fam)), eq(person, husband(fam)))) {

                traverse (fnode(fam), node, level) {

                    if (eq(strcmp(tag(node), "PLAC"), 0)) {
                        call addplace(node)
                    }
                }
            }
        }
    }
    print(" done\nWriting places...")
    call write_rll_header()
    set(nextpnum,0)
    forlist(placelist,placename,pnum) {
        if (ge(pnum,nextpnum)) {
            print(" ",d(pnum))
            set(nextpnum,add(nextpnum,100))
        }
        call write_rll_entry(placename)
    }
}

/*
To: ROOTS-L Genealogy List
Subject: ROOTS LOCATION LIST, September, 1994

Next location list on 3rd Sunday in November.  (Deadlines for
submissions are generally the preceding Friday.)

         ## WHAT IS THE ROOTS LOCATION LIST?##

- The Roots Location List is compiled from locations e-mailed to me
by network people doing genealogical research in a particular
location and who are willing to exchange information. The idea is
that, if you had ancestors living in the same place in the same
period, it might be beneficial to compare notes -- maybe you and
the submitter are kinfolk or maybe you can help each other track
down unique sources dealing with the area.


-This list should not be confused with the ROOTS SURNAME LIST that
is maintained by Karen Isaacson in other files.  If you are
confused, send a message to: listserv@vm1.nodak.edu    In the body
of your message put: GET FAMILY INDEX.  This will show you all of
the files in this part of the genealogy files.

         ## HOW CAN I PARTICIPATE IN THE ROOTS LOCATION LIST? ##

- Send additions or corrections to me at AHCSBB@ukcc.uky.edu. Write
to me if you have general questions about the list.  I will
acknowledge (or attempt to acknowledge) all submissions.

Entries are formatted as follows:

Location/Date1-Date2/nametag
Date1 is the earliest date for which the submitter has information.
Date2 is the most recent date.

         ## HOW DO I CONTACT SOMEONE ON THE LIST? ##

- Write directly to the submitter if you would like to exchange
information.

-To contact the submitter of the information, use the nametag to
find the address of the submitter in the address list - FAMILY
LOCADDR.  The addresses of the submitters are in a separate file on
the listserver.  To obtain them, send a one line message: GET
FAMILY LOCADDR

A list of the abbreviations used is available directly from the
listserver:  send e-mail to LISTSERV@vm1.nodak.edu or
LISTSERV@NDSUVM1, with a one-line message that states:

GET FAMILY ABBREV

Include no other text, and leave the subject line blank.  The
listserver will return by e-mail the list of abbreviations.  Or
you can use anonymous FTP to vm1.nodak.edu

*/
