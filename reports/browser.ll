/*
 * @progname    browser.ll
 * @version     1.0
 * @author      Prinke
 * @category
 * @output      onscreen
 * @description

   browsing via all kinds of links, especially in non-standard
   or experimental GEDCOM structures

 browser.ll v.1.0   Rafal T. Prinke -- 19 APR 1997 -- rafalp@hum.amu.edu.pl

*/


proc main()
{
        list(back)
        list(backhdr)
        getindi(p, "Person to start with: ")
        if(not(p)) { break() }
        set(i, savenode(root(p)))
        set(hdr, concat("***  INDI: ", name(p,0),"  ***"))
        set(bh, "----- BACK")
        while(i) {
                list(mnu)
                list(gto)
                enqueue(gto,0)
                enqueue(gto,0)
                enqueue(mnu,"----- STOP")
                enqueue(mnu,bh)

                traverse (i, node, x) {
                        if (reference(value(node))) {
                                set (n, dereference(value(node)))
                                enqueue(gto, savenode(n))

if(eq(substring(value(node),1,2),"@I")) {
        set(show,concat("INDI: ", name(indi(value(node)),0))) }
elsif(eq(substring(value(node),1,2),"@S")) {
        set(show,concat("SOUR: ", value(child(n)) )) }
elsif(eq(substring(value(node),1,2),"@E")) {
        set(show,concat("EVEN: ", value(child(n)) )) }
elsif(eq(substring(value(node),1,2),"@F")) {
        set(show,concat("FAM:  ", name(husband(fam(value(node))),0),
         " & ", name(wife(fam(value(node))),0))) }
else { set(show, concat("OTHER:",value(child(node)))) }

                                enqueue(mnu, show)
                        }
                }
                set(why, menuchoose(mnu, hdr))

                if(eq(why, 1)) { break() }

                elsif(eq(why, 2)) {
                        if(empty(back)) {
                        set(bh, "-- THIS IS THE FIRST RECORD - CAN'T GO BACK --")
                                push(back, savenode(i))
                                push(backhdr, hdr)
                        }
                        set(i, pop(back))
                        set(hdr, pop(backhdr))
                }
                else {
                        push(back, savenode(i))
                        push(backhdr, hdr)
                        set(nd, getel(gto, why))
                        set(hdr, concat("***  ",getel(mnu, why),"  ***"))
                        set(i, nd)
                        set(bh, "----- BACK")

                }
        }
}
