/*
 * @progname    igi-filter.ll
 * @version     1 of 1993-02-15
 * @author      Jim Eggert (eggertj@atc.ll.mit.edu)
 * @category
 * @output      GedCom
 * @description
 *
 * Write GedCom of families/events containing given TAG/VALUE
 *
This program is meant to help you filter out useful data from a big IGI
download.  You specify what GEDCOM tag you want to look at, and what value
you want to accept.  Then it writes a GEDCOM file that contains only those
IGI entries that have what you want.  So for example, suppose you have
downloaded all the Hammer families from the IGI, but are really interested
only in those from Harthausen.  In this case, you specify PLAC as the GEDCOM
tag, and Harthausen as the value, and you get a GEDCOM file with only the
Harthausen Hammer families.

The program will look at every GEDCOM level to find the sought tag, in
both individual and family records.  For NAME and PLAC entries, all
name or components are searched for a match.  A match is defined as
string equality for all provided or available characters, ignoring
case.  Thus entering Harth as a desired value will match Harthausen,
Harthofen, and Hart as well.  Once a matching value is found, the
program will include in its output the whole matching "family" from
the IGI data.  (An IGI "family" is really just an event.)

This program will run on non-IGI data also.  For non-IGI data, it will
generally include somewhat more people in its output file that you
might expect.  No big deal.

igi-filter - a LifeLines program to filter IGI data
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1, 15 February 1993

*/

global(this_one)
global(the_tag)
global(the_value)
global(the_length)
global(name_tag)
global(plac_tag)

proc check_value(a_string) {
    set(a_length,strlen(a_string))
    if (gt(a_length,the_length)) {
        if (not(strcmp(upper(trim(a_string,the_length)),the_value))) {
            set(this_one,1)
        }
    } else {
        if (not(strcmp(trim(the_value,a_length),upper(a_string)))) {
            set(this_one,1)
        }
    }
}

proc check_values(root) {
    list(nlist)
    traverse(root,node,level) {
        if (and(not(this_one),not(strcmp(tag(node),the_tag)))) {
            if (name_tag) {
                extractnames(node,nlist,n,ns)
                forlist(nlist,n0,nnum) {
                    call check_value(n0)
                }
            }
            elsif (plac_tag) {
                extractplaces(node,nlist,n)
                forlist(nlist,n0,nnum) {
                    call check_value(n0)
                }
            }
            else {
                call check_value(value(node))
            }
        }
    }
}


proc main() {
    indiset(accept)
    getstrmsg(the_tag,"Enter tag for filtering:")
    set(the_tag,save(upper(the_tag)))
    if (not(strcmp(the_tag,"NAME"))) { set(name_tag,1) }
    else { set(name_tag,0) }
    if (not(strcmp(the_tag,"PLAC"))) { set(plac_tag,1) }
    else { set(plac_tag,0) }
    getstrmsg(the_value,"Enter value to be accepted:")
    set(the_value,save(upper(the_value)))
    set(the_length,strlen(the_value))

    set(accepted,0)
    forindi(person,pnum) {
        set(this_one,0)
        call check_values(inode(person))
        if (this_one) {
            addtoset(accept,person,0)
            set(accepted,add(accepted,1))
        }
    }
    print("Passed ") print(d(accepted))
    print(" of ") print(d(pnum)) print(" individuals.\n")
    set(accepted,0)
    forfam(family,fnum) {
        set(this_one,0)
        call check_values(fnode(family))
        if (this_one) {
            set(accepted,add(accepted,1))
            if (person,husband(family)) {
                addtoset(accept,person,1)
            }
            elsif (person,wife(family)) {
                addtoset(accept,person,1)
            }
            else {
                children(family,person,pnum) {
                    if (eq(pnum,1)) { addtoset(accept,person,2) }
                }
            }
        }
    }
    print("Passed ") print(d(accepted))
    print(" of ") print(d(fnum)) print(" families.\nWriting GEDCOM file...")
    set(accept,union(accept,spouseset(accept)))
    set(accept,union(accept,parentset(accept)))
    set(accept,union(accept,childset(accept)))
    gengedcom(accept)
    "0 TRLR\n"
    print("done")
}

