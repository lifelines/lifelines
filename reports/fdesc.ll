/* 
 * @progname       fdesc.ll
 * @version        1.0
 * @author         Wetmore
 * @category       
 * @output         GEDCOM
 * @description    


   this funny little program is based on Tom Wetmore's "genancc1"
   and generates a GEDCOM file with descendants of a chosen individual
   who have the same surname (usually this means male line descendants
   plus illegitimate children of daughters) plus their spouses.

   a truely good program would need to exclude cases of daughters'
   marriages with guys of the same surname but not related and include
   male line descendants who changed surnames */

proc main ()
{
                indiset(set1)
                indiset(set2)
                getindi(indi)
                addtoset(set2, indi, n)
                set(set1, descendantset(set2))
                set(set1, union(set1, set2))
                getindiset(set2)
                set(set1, intersect(set1, set2))
                set(set2, spouseset(set1))
                set(set1, union(set1, set2))
                gengedcom(set1)

}
