/*
 * @progname       fix_nameplac.ll
 * @version        1
 * @author         Eggert
 * @category       
 * @output         GEDCOM
 * @description    

This is a quicky to show how to fix name and place spacing.


fix_nameplac - a LifeLines names and places fixing program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  8 January 1993


*/

proc fixit(root) {
    list(components)
    traverse(root,node,level) {
        set(t,save(tag(node)))
        d(level) " " t " "
        if (not(strcmp(t,"PLAC"))) {
            extractplaces(node,components,nplaces)
            forlist(components,place,plnum) {
                if (gt(plnum,1)) { ", " }
                place
            }
        }
        elsif (not(strcmp(t,"NAME"))) {
            extractnames(node,components,nnames,nsurname)
            forlist(components,name,nnum) {
                if (gt(nnum,1)) { " " }
                if (eq(nnum,nsurname)) { "/" }
                name
                if (eq(nnum,nsurname)) { "/" }
            }
        }
        else {
            value(node)
        }
        "\n"
    }
}


proc main() {
    forindi(person,pnum) {
        call fixit(inode(person))
    }
    forfam(family,fnum) {
        call fixit(fnode(family))
    }
    "0 TRLR\n"
}
