/*
 * @progname       givens_gender_finder.ll
 * @version        1
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program finds all persons with a particular given name and gender.
It is really meant to be a companion to the givens_gender program.

givens_gender_finder - a LifeLines database given name & gender finder program
        by Jim Eggert (eggertj@ll.mit.edu)
        Version 1 (19 April 1995) requires LifeLines 3.0.1 or later.

*/

proc main() {
    list(names)
    getstrmsg(nseek,"Enter name to be found") set(nseek,save(nseek))
    getstrmsg(gseek,"Enter gender to be found") set(gseek,save(gseek))
    forindi(person,pnum) {
        if (not(strcmp(gseek,sex(person)))) {
            extractnames(inode(person),names,nnames,isurname)
            forlist(names,name,iname) {
                if (ne(iname,isurname)) {
                    if (not(strcmp(name,nseek))) {
                        print(key(person)," ",name(person),"\n")
                    }
                }
            }
        }
    }
}
