/*
 * @progname       infant_mortality.ll
 * @version        1
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program finds families that have lost multiple children.
You give it the threshold for the number of young deaths, and the
threshold for the age at death, and it finds all the appropriate
families.

infant_mortality - a LifeLines program
         by Jim Eggert (eggertj@atc.ll.mit.edu)
         Version 1,  19 September 1994

*/


global(yob)
global(yod)

proc main() {
    getintmsg(numthresh,"Enter threshold for number of young deaths")
    getintmsg(agethresh,"Enter threshold for age at death")
    forfam(family,fnum) {
        if (ge(nchildren(family),numthresh)) {
            set(countdeaths,0)
            set(maxageatdeath,0)
            children(family,child,cnum) {
                call get_dyear(child)
                if (yod) {
                    call get_byear(child)
                    if (yob) {
                        set(ageatdeath,sub(yod,yob))
                        if (le(ageatdeath,agethresh)) {
                            set(countdeaths,add(countdeaths,1))
                            if (gt(ageatdeath,maxageatdeath)) {
                                set(maxageatdeath,ageatdeath)
                            }
                        }
                    }
                }
            }
            if (ge(countdeaths,numthresh)) {
                key(family) " "
                name(husband(family)) " and " name(wife(family))
                "\nlost " d(countdeaths)
                " children by the age of " d(maxageatdeath)
                ".\n"
                children(family,child,cnum) {
                    call get_byear(child)
                    call get_dyear(child)
                    name(child) " ("
                    if (yob) { d(yob) }
                    "-"
                    if (yod) { d(yod) }
                    ") "
                    if (and(yob,yod)) { d(sub(yod,yob)) }
                    "\n"
                }
                "\n"
            }
        }
    }
}

proc get_dyear(person) {
    set(yod,0)
    if (d,death(person)) { extractdate(d,dod,mod,yod) }
    if (not(yod)) {
        if (d,burial(person)) { extractdate(d,dod,mod,yod) }
    }
}

proc get_byear(person) {
    set(yob,0)
    if (b,birth(person)) { extractdate(b,dob,mob,yob) }
    if (not(yob)) {
        if (b,baptism(person)) { extractdate(b,dob,mob,yob) }
    }
}
