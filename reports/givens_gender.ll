/*
 * @progname       givens_gender.ll
 * @version        1
 * @author         Jim Eggert (eggertj@ll.mit.edu)
 * @category       
 * @output         Text
 * @description

Given name gender report program.
This program prints a list of all given names of people, tagged by one
of the following:
M  Only males
F  Only females
B  Males and females
M? Males and persons of unknown gender
F? Females and persons of unknown gender
B? Males, females, and persons of unknown gender

Very few names should be marked as B.  Check them carefully and you
may find some database gender errors.  You may be able to help resolve
unknown genders for those names tagged M? and F?.

If you want to sort the report by name only, do
  sort +1b -2b report > report.sort
If you want to sort the report by gender and name, do
  sort report > report.sort

If you want to find a person with a specific given name and gender,
use givens_gender_finder.

        by Jim Eggert (eggertj@ll.mit.edu)
        Version 1 (19 April 1995) requires LifeLines 3.0.1 or later.
*/

proc main() {
    table(namestable)
    list(nameslist)
    list(codelist)
    list(names)
    print("Collecting names...")
    set(namescount,0)
    forindi(person,pnum) {
/* if (gt(pnum,300)) { break() } */
        if (male(person)) { set(a,15) set(m,2) }
        elsif (female(person)) { set(a,10) set(m,3) }
        else { set(a,6) set(m,5) }
        extractnames(inode(person),names,nnames,isurname)
        forlist(names,name,iname) {
            if (ne(iname,isurname)) {
                if (l,lookup(namestable,name)) {
                    if (not(mod(l,m))) {
                        insert(namestable,save(name),add(l,a))
                    }
                }
                else {
                    set(sname,save(name))
                    insert(namestable,sname,a)
                    enqueue(nameslist,sname)
                    incr(namescount)
                }
            }
        }
    }
    setel(codelist, 6,"?  ")
    setel(codelist,10,"F  ")
    setel(codelist,15,"M  ")
    setel(codelist,16,"F? ")
    setel(codelist,21,"M? ")
    setel(codelist,25,"B  ")
    setel(codelist,31,"B? ")
    print("done\nPrinting ", d(namescount)," names...")
    while(name,dequeue(nameslist)) {
        getel(codelist,lookup(namestable,name))
        name "\n"
    }
}
