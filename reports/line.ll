/*
 * @progname       line.ll
 * @version        1
 * @author         J.F. Chandler
 * @category       
 * @output         Text
 * @description
 *
displays the descendancy line(s) from one person to another.
This program assumes no individual has more than one set of parents.

Algorithm partly borrowed from TTW's cousins program.

Version 1 - 1998 Apr 22 - J.F. Chandler

  This program requires version 3 of LifeLines.
*/

global(link1)   /* table of links back one person */
global(link2)   /* table of alternate links */
global(elist)   /* list of chain ends */
global(dots)    /* person counter */

proc main () {
getindimsg(from,"Which ancestor?")
set(to,0)
if(from) {
        getindimsg(to,"Which descendant?")
}
if(not(and(from,to))) {
        print("Not found\n")
        return()
}
set(fkey,save(key(from)))
set(tkey,save(key(to)))
"Descendancy line from " name(indi(fkey)) "\nto " name(indi(tkey)) ":\n"
print("Searching for the line(s) from:\n",name(from)," to ",name(to))
print(".\n\nThis may take a while -- ")
print("each dot is 25 persons considered.\n")

table(link1)
table(link2)
list(elist)

set(dots,0)
set(found,0)
set(gen,0)
set(maxgen,0)

/* Link the ancestor to self (unique marker), and add as the first
entry in the list of chain ends.  A "zero" person in the list marks
the end of a generation. */

insert(link1,fkey,fkey)
enqueue(elist,fkey)
enqueue(elist,0)

/* Iterate through the list of chain ends, removing them one by one;
link their children back to them; add the children to the chain end
list; check each iteration to see if the target person has been found
through both parents; if so quit the iteration; also quit three
generations after finding through either parent.  */

while(gt(length(elist),1)) {
        set(key,dequeue(elist))
        if(not(key)) {
                set(gen,add(1,gen))
                if(eq(gen,maxgen)) { break() }
                enqueue(elist,0)
                continue()
        }
        set(indi,indi(key))
        families(indi,fam,sp,n1) {
                children(fam,child,n2) {
                        call include(key,child)
                }
        }
        if(not(found)) {
                if(lookup(link1,tkey)) {
                        set(found,1)
                        set(maxgen,add(3,gen))
                }
        } elsif(lookup(link2,tkey)) { break() }
}

/* Quit if the "from" is not an ancestor of the "to" person. */

if(not(found)) {
        print("\nThere is no such line.")
        "There is no such line.\n"
        return()
}

set(gen,1)
"\nWorking back:\n\n1. " call do_person(indi(tkey))
call printrest(tkey,gen)
}

/* Recursively print the rest of the line back to the source.
If the current person is linked through both parents, also print
the alternate line starting from here. */

proc printrest(key,gen) {
set(gen,add(1,gen))
set(new,lookup(link1,key))
if(eq(0,strcmp(key,new))) { return() }
d(gen) ". " call do_person(father(indi(key)))
"    & " call do_person(mother(indi(key)))
if(alt,save(lookup(link2,key))) { "* " }        /* mark a branch point */
call printrest(new,gen)
if(alt) {
        nl()
        call printrest(alt,gen)
}}

/* Link a new child (indi) back to a parent (key).
If the new child has already been linked once, use alternate table.
A truly new child is added to the list of chain ends */

proc include(key,indi) {

set(dots,add(dots,1))
if(eq(25,dots)) {
        set(dots,0)
        print(".")
}

set(new,save(key(indi)))
if(lookup(link1,key(indi))) {
        insert(link2,new,key)
} else {
        insert(link1,new,key)
        enqueue(elist,new)
}}

/* Print name and dates for a given person */

proc do_person(p) {
name(p) " ("
set(e,birth(p))
if(not(e)) {set(e,baptism(p))}
if(e) {date(e)}
" - "
set(e,death(p))
if(not(e)) {set(e,burial(p))}
if(e) {date(e)}
")\n"
}
