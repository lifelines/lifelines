/*
 * @progname       pdesc2.r
 * @version        1.0
 * @author         Wetmore, Manis, Jones, Eggert, Simms
 * @category       
 * @output         Text
 * @description    
 *
 *   pdesc2
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *   With modifications by:  James P. Jones
 *   With modifications by:  Jim Eggert (unknown spouse bugfix)
 *   With modifications by:  Robert Simms (indented line wrap) Mar '96
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990.
 *
 *   It will select and produce a descendant report for the person
 *   selected.   Children of each spouse are printed below that spouse.
 *
 *   Descendants report format, which print the date in long format.
 *
 *   Output is an ASCII file.
 */

global(page_size)
global(tab_size)
proc main () {
        set(page_size, 80)
        set(tab_size, 3)
        getindi(indi)
        call pout(0, indi)
        "\n===============================================================================\nReport generated with <pdesc2> under LifeLines\n"
}
proc pout(gen, indi)
{
/*        print(name(indi)) print("\n")*/
        set(skip, mul(4,gen))
        col(skip)
        set(x, skip)
        set(s, concat(rjustify(d(add(gen, 1)), 2), "--"))
        s
        set(x, add(x, tab_size))
        set(skip, x)
        call outp(indi, skip, x)
        set(next, add(1, gen))
        families(indi,fam,sp,num) {
                set(skip, add(2,mul(4,gen)))
                col(skip)
                set(x, skip)
                " sp-"
                set(x, add(x, 4))
                set(skip, x)
                call outp(sp, skip, x)
                if (lt(next,15)) {
                        children(fam, child, no) {
                                call pout(next, child)
                        }
                }
        }
}

proc outp(indi, skip, x)
{
        set(s, concat(fullname(indi, 1, 1, 40),
               " (",
               long(birth(indi)),
               " - ",
               long(death(indi)),
               ")"))
        set(x, outline(s, add(tab_size, skip), x))
        "\n"
}

func outline(text, skip, x) {
   if (eq(x, 0)) {
      col(skip)
      set(x, skip)
      }
   set(max, sub(page_size, x))
   if (gt(strlen(text), max)) {
      set(space, breakit(text, max))
      if (eq(space, 0)) {
         if (eq(x, skip)) {
            set(text, strsave(text))
            substring(text, 1, sub(max, 1)) "-"
            set(x, 0)
            set(text, substring(text, max, strlen(text)))
            set(x, outline(text, skip, x))
            } else {
            set(x, 0)
            set(x, outline(text, skip, x))
            }
         } else {              /* space gt 0 */
         set(text, strsave(text))
         substring(text, 1, sub(space, 1))
         set(x, 0)
         set(x, outline(substring(text, add(space, 1), strlen(text)), skip, x))
         }
      } else {
      text
      set(x, add(x, strlen(text)))
      }
   return(x)
   }

func breakit(text, max) {
   set(space, 0)
   set(occ, 1)
   set(next, index(text, " ", occ))
   incr(occ)
   while ( and(le(next, add(max, 1)), ne (next, 0))) {
      set(space, next)
      set(next, index(text, " ", occ))
      incr(occ)
      }
   return(space)
   }
