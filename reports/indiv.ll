/* 
 * @progname       indiv
 * @version        1.0
 * @author         Simms
 * @category       
 * @output         Text
 * @description    

   Produces an indented report on an individual and all families associated
   with that individual.  Details on individuals include NOTE lines.
   Linewrapping is done with indenting maintained.

   indiv2.r

   Written by: Robert Simms, 27 Mar 1996
               rsimms@math.clemson.edu, http://math.clemson.edu/~rsimms

   At the beginning of main() is provided the means to easily change page width,
   tab size, left margin, and whether or not to include notes in output.
   ______________
   Version 2:  5 April 96 --  Unknown spouses can be returned by the family
               function, so a check had to be added to make sure that
               individuals exist before trying to print information on them.
               Now it's fixed to return _____ _____ as the name of an
               unknown person.
*/

global(page_width)
global(tab_size)
global(left_margin)
global(note_flag)

proc main() {
   set(page_width, 80)
   set(tab_size, 3)
   set(left_margin, 1)
   set(note_flag, 1) /*set equal to 1 to include notes, 0 not to include notes*/


   getindi(indi)
   set(x, 0)
   set(skip, add(left_margin,1))
   set(x, outfam(indi, skip, x))

   nl()
   " -------------------------------------"
   nl()
   }

func outfam(indi, skip, x) {
   set(x, outpers(indi, skip, x))
   if (gt(nfamilies(indi),0)) {
      set(skip, add(skip, tab_size))
      families(indi, fam, sp, num) {
         set(x, 0)
         set(x, outline(concat("Family #", d(num)), skip, x))
         if (date(marriage(fam))) {
            set(x, outline(concat(", ", date(marriage(fam))), skip, x))
            }
         set(x, 0)
         set(skip, add(skip, tab_size))
         set(x, outpers(sp, skip, x))
         if (gt(nchildren(fam), 0)) {
            set(x, outline("Children", skip, x))
            set(x, 0)
            set(skip, add(skip, tab_size))
            children(fam, child, no) {
               set(x, outpers(child, skip, x))
               }
            set(skip, sub(skip, tab_size))
            }
         set(skip, sub(skip, tab_size))
         }
      }
   return(x)
   }

func outpers(indi, skip, x) {
   if (indi) {
      print(name(indi), nl())
      set(x, 0)
      set(x, outline(name(indi), skip, x))
      set(skip, add(skip, tab_size))
      set(s, "")
      if (birth(indi)) {
         set(s, concat(", b. ", long(birth(indi))))
         }
      if (death(indi)) {
         set(s, concat(s, ", d. ", long(death(indi))))
         }
      if (burial(indi)) {
         set(s, concat(s, ", buried at ", place(burial(indi))))
         }
      set(s, concat(s, ". "))
      set(x, outline(s, skip, x))
      if (note_flag) {
         set(s, "")
         fornotes(inode(indi), note) {
            set(s, concat(s, note))
            }
         set(x, outtxt(s, skip, x))
         set(skip, sub(skip, tab_size))
         }
      } else {
      print("_____ _____", nl())
      set(x, 0)
      set(x, outline("_____ _____", skip, x))
      }
   set(x, 0)
   return(x)
   }

func outtxt(txt, skip, x) {
   set(cr, index(txt, nl(), 1))
   while(ne(cr, 0)) {
      set(txt, save(txt))
      set(txt2, concat(substring(txt, 1, sub(cr, 1)), " "))
      set(x, outline(txt2, skip, x))
      set(txt, substring(txt, add(cr, 1), strlen(txt)))
      set(cr, index(txt, nl(), 1))
      }
   if (gt(strlen(txt),0)) {
      set(x, outline(txt, skip, x))
      }
   return(x)
   }

func outline(text, skip, x) {
   if (eq(x, 0)) {
      col(skip)
      set(x, skip)
      }
   set(max, sub(page_width, x))
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
