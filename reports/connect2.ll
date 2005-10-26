/* 
 * @progname       connect2.ll
 * @version        2.1
 * @author         Simms
 * @category       
 * @output         Text
 * @description
 *                 Describes the family line connecting an ancestor/descendant

   Written by: Robert Simms, 19 Sep 1997
               rsimms@math.clemson.edu, http://www.math.clemson.edu/~rsimms

   Asks for a descendant and an ancestor then produces, for the line
   connecting the two persons,
   an indented report on an individual and all families associated
   with that individual.  Details on individuals include NOTE lines.
   Line wrapping is done with indenting maintained.

   This program does not check to make sure that the descendant given is really
   a descendant of the ancestor.  An error will result if not.

   At the beginning of main() is provided the means to easily change page width,
   tab size, left margin, and whether or not to include notes in output.

   Revisions:  2: Robert Simms, 17 Feb 2000, made line-wrapping code more
                  consistent in its use of the parameters page_width,
                  and left_margin.
             2.1: Robert Simms, 30 May 2001, fixed the concatenation of
                  multiple notes so that two spaces are inserted before
                  every note after the first note.
                  Thanks to M.W. Poirier for pointing this out.
                  
*/

global(page_width)
global(tab_size)
global(left_margin)
global(note_flag)
global(plist)

proc main() {
   set(page_width, 80)
   set(tab_size, 3)
   set(left_margin, 0)
   set(note_flag, 1) /*set equal to 1 to include notes, 0 NOT to include notes*/
   list(plist)


   getindi(indi1, "Descendant")
   getindi(indi2, "Ancestor")
   set(connects, 0)
   if(connect(indi1, indi2)) {

      forlist(plist, person, pnum) {
         if(ne(pnum, 1)) {
            nl() nl()
         }
         set(x, 0)
         set(skip, left_margin)
         set(x, outfam(person, skip, x))
      }
      nl()
      " -------------------------------------"
      nl()
   }
}

func connect(person, target) {
   set(connects, 0)
   if(eq(person, target)) {
      set(connects, 1)
   } else {
      if(dad, father(person)) {
         if(connect(dad, target)) {
            set(connects, 1)
         } else {
            if(mom, mother(person)) {
               if(connect(mom, target)) {
                  set(connects, 1)
               }
            }
         }
      }
   }
   if(connects) {
      enqueue(plist, person)
   }
   return(connects)
}


func outfam(indi, skip, x) {
   set(x, outpers(indi, skip, x))
   if(gt(nfamilies(indi), 0)) {
      set(skip, add(skip, tab_size))
      families(indi, fam, sp, num) {
         set(x, 0)
         set(x, outline(concat("Family #", d(num)), skip, x))
         if(date(marriage(fam))) {
            set(x, outline(concat(", ", date(marriage(fam))), skip, x))
         }
         set(x, 0)
         set(skip, add(skip, tab_size))
         set(x, outpers(sp, skip, x))
         if(gt(nchildren(fam), 0)) {
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
   if(indi) {
      print(name(indi), nl())
      set(x, 0)
      set(x, outline(name(indi), skip, x))
      set(skip, add(skip, tab_size))
      set(s, "")
      if(birth(indi)) {
         set(s, concat(", b. ", long(birth(indi))))
      }
      if(death(indi)) {
         set(s, concat(s, ", d. ", long(death(indi))))
      }
      if(burial(indi)) {
         set(s, concat(s, ", buried at ", place(burial(indi))))
      }
      set(s, concat(s, ". "))
      set(x, outline(s, skip, x))
      if(note_flag) {
         set(s, "")
         set(note_separator, "")
         fornotes(inode(indi), note) {
            set(s, concat(s, note_separator, note))
            set(note_separator, "  ")
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
   if(gt(strlen(txt), 0)) {
      set(x, outline(txt, skip, x))
   }
   return(x)
}

func outline(text, skip, x) {
   if(eq(x, 0)) {
      col(add(skip, 1))
      set(x, skip)
   }
   set(max, sub(page_width, x))
   if(gt(strlen(text), max)) {
      set(space, breakpoint(text, max))
      if(eq(space, 0)) {
         if(eq(x, skip)) {
            set(text, strsave(text))
            substring(text, 1, sub(max, 1)) "-"
            set(x, 0)
            set(text, substring(text, max, strlen(text)))
            set(x, outline(text, skip, x))
         } else {
            set(x, 0)
            set(x, outline(text, skip, x))
         }
      } else {              /* space gt 0 -- good break point found*/
         set(text, strsave(text))
         substring(text, 1, sub(space, 1))
         set(x, 0)
         set(text, strsave(substring(text, add(space, 1), strlen(text))))
         while(eqstr(" ", substring(text, 1, 1))) { /* strip leading spaces */
            set(text, strsave(substring(text, 2, strlen(text))))
         }
         set(x, outline(text, skip, x))
      }
   } else {
      text
      set(x, add(x, strlen(text)))
   }
   return(x)
}

func breakpoint(text, max) {
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
