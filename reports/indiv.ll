/* 
 * @progname       indiv.ll
 * @version        3.2
 * @author         Simms
 * @category       
 * @output         Text
 * @description
 *
 * Report on individual with all his families

   Written by: Robert Simms, 27 Mar 1996
               rsimms@math.clemson.edu, http://www.math.clemson.edu/~rsimms

   Produces an indented report on an individual and all families associated
   with that individual.  Details on individuals include NOTE lines.
   Linewrapping is done with indenting maintained.

   At the beginning of main() is provided the means to easily change page width,
   tab size, left margin, and whether or not to include notes in output.
   ______________

   Version 2:  5 April 96 --  Unknown spouses can be returned by the family
               function, so a check had to be added to make sure that
               individuals exist before trying to print information on them.
               Now it's fixed to return _____ _____ as the name of an
               unknown person. -- Robert Simms

   Version 3:  16 Feb 2000 -- Two spaces at the end of a sentence could result
               in a leading space after line-wrap.  Added a loop to
               eliminate leading spaces after line-wrap.  Care had to be taken
               to use the strsave() function to get it working correctly.
               Also fixed it so that page_width really is the maximum
               line length, not one less.
               -- Robert Simms

   Version 3.1: 30 May 2001, fixed the concatenation of multiple notes
                so that two spaces are inserted before every note
                after the first.  Thanks to M.W. Poirier for pointing this out.

   To-do:     Option to maintain blank lines (paragraphing) in notes.
              Once that is done, it will be possible to separate multiple
              notes with a blank line, easily.
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
   set(skip, left_margin)
   set(x, outfam(indi, skip, x))

   nl()
   " -------------------------------------"
   nl()
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
         /* if multiple spouses in a marriage, this will only pick up
            the first one
          */
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

/* outtxt -- removes new line chars from text and sends it to output
             via the outline function
*/
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

/* outline -- buffered text output with linewrapping and and indentation
              preservation
   the vars:  x -- the column up to which text has been written
                   on the current line
              skip -- current indentation, added to x at the start of a new
                      line
*/
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
