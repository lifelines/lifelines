/*
 * @progname       fam_ged.ll
 * @version        1.1 of 1994-06-08
 * @author         Wetmore and Prinke
 * @category       
 * @output         GEDCOM
 * @description

This program extracts a gedcom file of all male line descendants
of a specified person, with their spouses and parents (including
those of the specified person and of all spouses). Also included
are possibly illegitimate children of females - when they have
the same surname as the mother but different than the father (also
if there is no father recorded).

-------------------------------------------------------------------
fam_ged - a LifeLines family gedcom extraction program

Version 1,  18 May  1994 by Thomas Wetmore IV, ttw@petrel.att.com
  modified   8 June 1994 by Rafal T. Prinke, rafalp@plpuam11.bitnet

*/

proc main ()
{
    list(ilist)
    indiset(idex)
    getindi(indi)
    enqueue(ilist, indi)
    set(out,1)  set(in,2)
    while (indi, dequeue(ilist)) {
        print("OUT: ", d(out), " ", name(indi), "\n")
        addtoset(idex, indi, 0)
        set(out, add(out, 1))
        if (male(indi)) {
            families(indi, fam, spouse, nfam) {
                children(fam, child, nchl) {
                    print("IN: ", d(in), " ", name(child), "\n")
                    set(in, add(in, 1))
                    enqueue(ilist, child)
                }
            }
        }
        if (female(indi)) {
            families(indi, fam, spouse, nfam) {
                children(fam, child, nchl) {
                  if (eq(strcmp(surname(indi), surname(child)), 0)) {
                   if (ne(strcmp(surname(indi), surname(spouse)), 0)) {
                    print("INfem: ", d(in), " ", name(child), "\n")
                    set(in, add(in, 1))
                    enqueue(ilist, child)
                   }
                  }
                }
            }
        }
    }
    set(idex, union(idex, spouseset(idex)))
    set(idex, union(idex, parentset(idex)))
    gengedcom(idex)
}
