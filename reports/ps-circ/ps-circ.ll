/*
 * @progname       ps-circ.ll
 * @version        2
 * @author         Jim Eggert (eggertj@ll.mit.edu)
 * @category       
 * @output         PostScript
 * @description    

This program generates a basic five-generation ancestry circle chart.
Its output is a Postscript file specifying the chart.  This program
uses a slightly modified version of the CIRC.PS program written by
David Campbell (campbell@cse.uta.edu) and John Dunn (dunn@pat.mdc.com).
You must have the file ps-circ.ps in your default directory for this to work.

Version 1 of this program only writes two lines to the circular chart,
with the first line being the name and the second line being the
birth-death years.  The ps-circ.ps file allows, however, up to four
lines of data per person.  By changing the putperson routines, you can
put additional information in the circular chart, but I was too lazy.

The four strlen global lists contain the recommended maximum string
lengths for each of the four strings at each of five levels.

ps-circ - a LifeLines five-generation ancestry circle chart program
        by Jim Eggert (eggertj@ll.mit.edu)
        Version 1, 15 Sep 1993
        Version 2,  3 May 1995, added postscript escape codes
*/

global(strlen1)
global(strlen2)
global(strlen3)
global(strlen4)

global(ps_xlat)

proc putperson1(person, level, ahnen) {
    "(" strxlat(ps_xlat,fullname(person,0,1,getel(strlen1,level)))
    ") " d(ahnen) " 1 info1\n"
    "(" strxlat(ps_xlat,year(birth(person)))
    "-"
    strxlat(ps_xlat,year(death(person)))
    ") " d(ahnen) " 2 info1\n"
}

proc putperson2(person, level, ahnen) {
    "(" strxlat(ps_xlat,fullname(person,0,1,getel(strlen1,level)))
    ") " d(ahnen)
    if (lt(level,5)) { " 2 info2\n" }
    else { " 1 info2\n" }
    "(" strxlat(ps_xlat,year(birth(person)))
    "-"
    strxlat(ps_xlat,year(death(person)))
    ") " d(ahnen)
    if (lt(level,5)) { " 1 info2\n" }
    else { " 2 info2\n" }
}

proc semicirc(person, level, ahnen, info) {
    if (and(person,le(level,5))) {
        if (eq(info,1)) { call putperson1(person,level,ahnen) }
        else { call putperson2(person,level,ahnen) }
        set(nextlevel, add(level,1))
        set(nextahnen, mul(ahnen,2))
        call semicirc(father(person), nextlevel, nextahnen, info)
        call semicirc(mother(person), nextlevel, add(nextahnen,1), info)
    }
}

proc main() {
    list(strlen1)
    list(strlen2)
    list(strlen3)
    list(strlen4)

/* load up the max string length lists */
    enqueue(strlen1,30)
    enqueue(strlen2,24)
    enqueue(strlen3,18)
    enqueue(strlen4,9)

    enqueue(strlen1,30)
    enqueue(strlen2,26)
    enqueue(strlen3,23)
    enqueue(strlen4,20)

    enqueue(strlen1,22)
    enqueue(strlen2,20)
    enqueue(strlen3,18)
    enqueue(strlen4,16)

    enqueue(strlen1,14)
    enqueue(strlen2,13)
    enqueue(strlen3,12)
    enqueue(strlen4,11)

    enqueue(strlen1,22)
    enqueue(strlen2,22)
    enqueue(strlen3,22)
    enqueue(strlen4,22)

/*  Initialize postscript translation table to escape characters */
    table(ps_xlat)
    insert(ps_xlat,"(","\\(")
    insert(ps_xlat,")","\\)")
    insert(ps_xlat,"\\","\\\\")

/* now find out who we are generating this chart for */

    getfam(fam)

    getintmsg(circtype,"Enter 1 for regular, 2 for bggb format:")

    copyfile("ps-circ.ps")
    "/circtype " d(sub(circtype,1)) " def\n\n"
    "fan\n\n"

    call semicirc(husband(fam),1,1,1)
    call semicirc(wife(fam),1,1,2)
    "\nshowpage\n"
}

/* translate string according to xlat table */
func strxlat(xlat,string) {
    set(fixstring,"")
    set(pos,1)
    while(le(pos,strlen(string))) {
        set(char,substring(string,pos,pos))
        if (special,lookup(xlat,char)) {
            set(fixstring,concat(fixstring,special))
        }
        else { set(fixstring,concat(fixstring,char)) }
        incr(pos)
    }
    return(save(fixstring))
}
