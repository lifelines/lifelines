/*
 * @progname       formatted_gedcom.ll
 * @version        1
 * @author         Eggert
 * @category       
 * @output         GEDCOM
 * @description    

This program outputs a LifeLines database in modified GEDCOM format.
Two additions to GEDCOM are made: an inter-record delimiter and a
level indenter.  These are set up as global parameters and initialized
at the beginning of the main() procedure.

formatted_gedcom - a LifeLines formatted GEDCOM listing program
         by Jim Eggert (eggertj@atc.ll.mit.edu)
         Version 1,  7 September 1993


The header() procedure writes a GEDCOM header.  You will definitely
want to edit this part of the program to reflect your name and
address.  Note that I have included a line specifying Macintosh
character encoding, appropriate for my database.  You may want to
delete or comment out this line.

*/

global(delimiter)
global(indenter)

proc header() {
    delimiter "0 HEAD\n"
    indenter "1 SOUR LIFELINES 2.3.3\n"
    indenter "1 DEST ANY\n"
    indenter "1 DATE " date(gettoday()) "\n"
    indenter "1 FILE " outfile() "\n"
    indenter "1 CHAR MACINTOSH\n"
    indenter "1 COMM Formatted GEDCOM output produced by formatted_gedcom\n"
    delimiter "0 @S1@ SUBM\n"
    indenter "1 NAME James Robert Eggert\n"
    indenter "1 ADDR 12 Bonnievale Drive\n"
    indenter indenter "2 CONT Bedford Massachusetts 01730\n"
    indenter indenter "2 CONT USA\n"
    indenter "1 PHON 617-275-2004\n"
}

proc main() {
    set(delimiter,
"--------------------------------------------------------------------------\n")
    set(indenter,"    ")

    call header()
    forindi(person,num) {
        call formatted_gedcom(inode(person),key(person))
    }
    forfam(family,num) {
        call formatted_gedcom(fnode(family),key(family))
    }

    delimiter "0 TRLR\n" delimiter
}

proc formatted_gedcom(node,key) {
    delimiter
    traverse(node,subnode,level) {
        if (level) {
            set(counter,0)
            while(lt(counter,level)) {
                indenter
                set(counter,add(counter,1))
            }
            d(level) " " tag(subnode) " " value(subnode) "\n"
        }
        else {
            "0 @" key "@ " tag(subnode) "\n"
        }
    }
}
