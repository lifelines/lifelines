/*
 * @progname       drep2.ll
 * @version        2.1
 * @author         Robert Simms
 * @category       
 * @output         Text
 * @description

   Produces an indented report on an individual's families and all
   descendant families.  Details on individuals include NOTE lines, once.
   Line wrapping is done with indention maintained.

   At the beginning of main() is provided the means to easily change
   page width, tab size, and left margin.

   Written by: Robert Simms, 16 Feb 2000
               rsimms@math.clemson.edu, http://www.math.clemson.edu/~rsimms
   This is based on indiv3.ll, also by Robert Simms.

   Version 2.1:  30 May 2001, fixed the concatenation of multiple notes
                 so that two spaces are inserted before every note after
                 the first.  Thanks to M.W. Poirier for pointing this out.
   ______________
   TODO: Clean up trailing spaces in output.

*/

global(page_width)
global(tab_size)
global(left_margin)
global(gen)
global(genlim)
global(iparent)
global(ichild)

proc main() {
   set(page_width, 72)
   set(tab_size, 3)
   set(left_margin, 1)

   getindi(person)
   getintmsg (genlim, "Maximum number of generations")
   report(person, genlim)

   /* NOTE: this footer may need modifying if the page_width is changed */
   concat(" ______________________________ This report was produced on ",
      stddate(gettoday()), " _______", nl())
}

func report(person, genlim) {
	list(toPrint)
	list(toScan)
	set(gen, 1)
	set(iparent, 1)
        set(ichild, 2)
	enqueue(toPrint, person)
	while( and( le(gen,genlim), gt(length(toPrint), 0) ) ) {
		">> Generation " d(gen) nl()
		while(i1, dequeue(toPrint)) {
			nl() doFams(i1) nl()
			enqueue(toScan, i1)
		}
		while(i1, dequeue(toScan)) {
			families(i1, fp, sp, fn) {
				children(fp, i_x, n_x) {
					if(gt(nfamilies(i_x), 0)) {
						enqueue(toPrint, i_x)
					}
				}
			}
		}
		set(gen, add(gen, 1))
		if(gt(length(toPrint), 0)) {
			nl()
		}
	}
}

func doFams(indi) {
   set(x, 0)
   set(skip, left_margin)
   set(x, outfam(indi, skip, x))
}

func outfam(indi, skip, x) {
   set(x, outpers(indi, skip, x, 1, 1))
   if(gt(nfamilies(indi), 0)) {
      set(skip, add(skip, tab_size))
      families(indi, fam, sp, num) {
         set(x, 0)
         set(x, outline(concat("Family #", d(num)), skip, x))
         if(date(marriage(fam))) {
            set(s, concat(", ", date(marriage(fam))))
         }
         if(nestr("", place(marriage(fam)))) {
            set(s, concat(s, ", ", place(marriage(fam))))
         }
         set(x, outline(s, skip, x))
         set(x, 0)
         set(skip, add(skip, tab_size))
         set(x, outpers(sp, skip, x, 1, 0))
         if(gt(nchildren(fam), 0)) {
            set(x, outline("Children", skip, x))
            set(x, 0)
            set(skip, add(skip, tab_size))
            children(fam, child, no) {
               set(x, outpers(child, skip, x, or(not(nfamilies(child)),eq(gen,genlim)), 0 ))
            }
            set(skip, sub(skip, tab_size))
         }
         set(skip, sub(skip, tab_size))
      }
   }
   return(x)
}

func outpers(indi, skip, x, note_flag, parent) {
   if(indi) {
      print(name(indi), nl())
      set(x, 0)
      if(note_flag) {
         if(parent) {
            set(s, concat(d(iparent), ". ", name(indi)))
            set(iparent, add(iparent, 1))
         } else {
            set(s, name(indi))
         }
      } else {
         if(and(lt(gen,genlim), gt(nfamilies(indi), 0))) {
            set(s, concat(name(indi), " <",d(ichild),">" ))
            set(ichild, add(ichild, 1))
         } else {
            set(s, name(indi))
         }
      }
      set(x, outline(s, skip, x))
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
      } else {              /* space gt 0 */
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
