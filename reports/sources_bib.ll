/*
 * @progname       sources_bib.ll
 * @version        1999-02
 * @author         Dennis Nicklaus (nicklaus@fnal.gov)
 * @category       
 * @output         LaTeX
 * @description

   Lifelines report program.
   Write out a LaTex bibliography entry line for each source referenced
   by an indi or family record in the whole database.
   This is pretty slow.
   The bibliography printed out is useful for the html.dn programs
   (if you first run it through my bib2html.c program)
   or for a Latex document.

   The bibliography is pretty much the same as that generated as part of
   the book-latex code.  (But book-latex generates its own bibliography,
   so you don't need this for that.)

   Most of the code for this report program was copied directly from book-latex.

   Deficiency?: May not generate an entry for a source which is only referenced
   from within another source. I don't know.
*/

global (bibList)
global (bibTable)
global (sourceList)
global (gotValue)
global (gottenNode)
global (gottenValue)
global(sour_list)
global(sour_table)

proc main ()
{
  list (bibList)
  table (bibTable)
  list (sourceList)
  table(sour_table)
  list(sour_list)

  call sour_addset()
  call sour_ged()

 while (b, dequeue (bibList)) { b }

}




proc sour_addset()
{
    forindi(person, number) {
	print(".")
	traverse(root(person),m,l) {
		call print_sources(m)
	}
        families(person, f, sp, m) {
		traverse(root(f),m,l) {
			call print_sources(m)
		}
        }
    }

}

/* sour_ged() outputs the current source list in GEDCOM format */

proc sour_ged()
{
        table(other_table)
        list(other_list)

        forlist(sour_list, k, n) {
                set(r, dereference(k))
                traverse(r, s, l) {
                        d(l)
                        if (xref(s)) { " " xref(s) }
                        " " tag(s)
                        if (v, value(s)) {
                          " " v
                          if(reference(v)) {
                            if (ne(0, lookup(other_table, v))) { continue() }
                            if (ne(0, lookup(sour_table, v))) { continue() }
                            set(v, save(v))
                            insert(other_table, v, 1)
                            enqueue(other_list, v)
                          }
                        }
                        "\n"
                }
        }
        forlist(other_list, k, n) {
                set(r, dereference(k))
                traverse(r, s, l) {
                        d(l)
                        if (xref(s)) { " " xref(s) }
                        " " tag(s)
                        if (v, value(s)) { " " v }
                        "\n"
                }
        }
}
/* print_sources (root)
   Prints all sources (SOUR lines) associated with the given GEDCOM line.  The
   sources are formated as LaTeX footnotes.  This routine prints each SOUR line
   as a separate footnote, which is not correct.  This should be corrected so
   that all sources are combined into a single footnote. */

proc print_sources (root)
{
       enqueue(sourceList,root)
	call sourceIt(sourceList)
}

proc getValue (root, t) {
  set (gotValue, 0)
  if (root) {
    fornodes (root, node) {
      if (and (not (gotValue), not (strcmp (tag (node), t)))) {
        set (gotValue, 1)
        set (gottenNode, node)
        set (gottenValue, save (value (node)))
      }
    }
  }
}

proc getValueCont (root, t) {
  set (gotValue, 0)
  if (root) {
    fornodes (root, node) {
      if (and (not (gotValue), not (strcmp (tag (node), t)))) {
        set (gotValue, 1)
        set (gottenNode, node)
        set (gottenValue, save (value (node)))
        fornodes (node, subnode) {
          if (not (strcmp ("CONT", tag (subnode)))) {
            if (strlen (value (subnode))) {
	      set (gottenValue, 
	        save (concat (gottenValue, concat ("\n", value (subnode)))))
            }
          } elsif (not (strcmp ("CONC", tag (subnode)))) {
            if (strlen (value (subnode))) {
	      set (gottenValue, 
	        save (concat (gottenValue, value (subnode))))
            }
          }
        }
      }
    }
  }
}

proc getValueCommaCont (root, t) {
  set (gotValue, 0)
  if (root) {
    fornodes (root, node) {
      if (and (not (gotValue), not (strcmp (tag (node), t)))) {
        set (gotValue, 1)
        set (gottenNode, node)
        set (gottenValue, save (value (node)))
        fornodes (node, subnode) {
          if (not (strcmp ("CONT", tag (subnode)))) {
            if (strlen (value (subnode))) {
	      set (gottenValue, 
	        save (concat (gottenValue, concat (",\n", value (subnode)))))
            }
          } elsif (not (strcmp ("CONC", tag (subnode)))) {
            if (strlen (value (subnode))) {
	      set (gottenValue, 
	        save (concat (gottenValue, value (subnode))))
            }
          }
        }
      }
    }
  }
}

 


proc sourceIt (sourceList) {
  list (cList)
  list (fList)
  while (root, dequeue (sourceList)) {
    fornodes (root, node) {
      if (not (strcmp (tag (node), "SOUR"))) {
	set (footnote, 1)
	set (val, value (node))
	if (val) {
  	  if (reference(val)){
	  call bibliographize (dereference(val))
	 }
        }
	if (xref (node)) {
	  call bibliographize (node)
	  set (val, xref (node))
	}
	if (val) {
	  set (a1, index (val, "@", 1))
	  set (a2, index (val, "@", 2))
	  if (and (eq (a1, 1), eq (a2, strlen (val)))) { 
	    set (c, save (substring (val, 2, sub (strlen (val), 1))))
	    enqueue (cList, c)
	    incr (cn)
	    set (footnote, 0)
	  }
        } else {
	  set (subnodecount, 0)
	  fornodes (node, subnode) {
            if (strcmp (tag (subnode), "SOUR")) {
	      incr (subnodecount)
	    }
	  }
	  if (eq (subnodecount, 0)) {
	    fornodes (node, subnode) {
	      set (val, value (subnode))
	     /* With loadsources, this is needed here. It is technically
		 illegal gedcom. */
	      if (xref (subnode)) {
	        call bibliographize (subnode)
	        set (val, xref (subnode))
	      }
	      if (val) {
	        set (a1, index (val, "@", 1))
	        set (a2, index (val, "@", 2))
	        if (and (eq (a1, 1), eq (a2, strlen (val)))) { 
	          set (c, save (substring (val, 2, sub (strlen (val), 1))))
	          enqueue (cList, c)
	          incr (cn)
	        }
              } 
	    }
	    set (footnote, 0)
	  }
	}
	if (footnote) {
	  enqueue (fList, node)
	}
      }
    }
  }
  while (cn) {
    forlist (cList, c, n) {
      if (and (ne (n, cn), not (strcmp (c, getel(cList, cn))))) {
	setel (cList, cn, "")
      }
    }
    decr (cn)
  }
}

proc bibliographize (root) {
  set (val, xref (root))
  set (c, save (substring (val, 2, sub (strlen (val), 1))))

  if (not (lookup (bibTable, c))) {
    insert (bibTable, c, 1)

/*    call getValueCont (root, "TEXT")
    if (figureFlag, gotValue) {
      enqueue (figureCiteList, c)
      enqueue (figureNodeList, gottenNode)
    }*/
    set (cref, save (concat ("\\protect\\ref{", concat (c, "}"))))
    set (pref, save (concat ("\\protect\\pageref{", concat (c, "}"))))

    set (b, "\\bibitem")
    if (figureFlag) {
      set (b, save (concat (b, concat ("[", concat (cref, "]")))))
    }
    set (b, save (concat (b, concat ("{", concat (c, "} ")))))
    call getValueCont (root, "TITL") 
    if (gotValue) { 
      set (b, save (concat (b, concat ("{\\em ", concat (gottenValue, "}, ")))))
    }
    call getValueCont (root, "AUTH") 
    if (gotValue) { 
      set (b, save (concat (b, concat (" ", concat (gottenValue, ", ")))))
    }
    call getValueCont (root, "PUBL") 
    if (gotValue) { 
	set(pubnode,gottenNode)
        call getValueCont (pubnode, "NAME") 
        if (gotValue) { 
	   set (b, save (concat (b, concat ("in {\\em ", concat (gottenValue, "}, ")))))
        }
       call getValueCommaCont (pubnode, "ADDR") 
       if (gotValue) { set (b, save (concat (b, concat (gottenValue, ": ")))) }
       call getValueCont (pubnode, "PUBR") 
        if (gotValue) { set (b, save (concat (b, concat (gottenValue, ", ")))) }
       call getValueCont (pubnode, "PHON") 
       if (gotValue) { set (b, save (concat (b, concat (gottenValue, ", ")))) }
       call getValueCont (pubnode, "DATE") 
       if (gotValue) { set (b, save (concat (b, concat (gottenValue, ", ")))) }
       call getValueCont (pubnode, "VOLU") 
       if (gotValue) { 
         set (word, "Volume ")
         if (or (index (gottenValue, "-", 1),
            or (index (gottenValue, ",", 1),
                index (gottenValue, "and ", 1)))) {
          set (word, "Volumes ")
         }
         set (b, save (concat (b, concat (word, concat (gottenValue, ", ")))))
       }
       call getValueCont (pubnode, "NUM") 
       if (gotValue) { 
         set (word, "Number ")
         if (or (index (gottenValue, "-", 1),
             or (index (gottenValue, ",", 1),
                 index (gottenValue, "and ", 1)))) {
           set (word, "Numbers ")
         }
         set (b, save (concat (b, concat (word, concat (gottenValue, ", ")))))
       }
       call getValueCont (root, "LCCN") 
       if (gotValue) { 
          set (b, save (concat (b, concat ("Call Number ", concat (gottenValue, ", ")))))
       }

    }
    call getValueCont (root, "PAGE") 
    if (gotValue) { 
      set (word, "page ")
      if (or (index (gottenValue, "-", 1),
          or (index (gottenValue, ",", 1),
              index (gottenValue, "and ", 1)))) {
        set (word, "pages ")
      }
      set (b, save (concat (b, concat (word, concat (gottenValue, ", ")))))
    }
    call getValueCont (root, "FILM") 
    if (gotValue) { 
      set (b, save (concat (b, 
        concat ("Filmed by the Church of Jesus Christ of Latter Day Saints, Microfilm Number ", 
        concat (gottenValue, ", "))))) 
    }
    call getValueCont (root, "FICH") 
    if (gotValue) { 
      set (b, save (concat (b, 
        concat ("Filmed by the Church of Jesus Christ of Latter Day Saints, Microfiche Number ", 
        concat (gottenValue, ", "))))) 
    }
    call getValueCont (root, "REPO") 
    if (gotValue) { 
      set (b, save (concat (b, concat ("at ", concat (gottenValue, ", ")))))
    }


    if (index (b, ", ", 1)) {
      set (b, save (concat (save (substring (b, 1, sub (strlen (b), 2))), ".")))
    }
  
    call getValueCont (root, "NOTE") 
    if (gotValue) { set (b, save (concat (b, concat (" ", gottenValue)))) }

    call getValueCont (root, "TEXT") 
    if (gotValue) { set (b, save (concat (b, concat (" ", gottenValue)))) }
  
    call getValueCont (root, "SOUR") 
    if (gotValue) { 
      set (bb, "?")
      if (gottenValue) {
        set (a1, index (gottenValue, "@", 1))
        set (a2, index (gottenValue, "@", 2))
        if (and (eq (a1, 1), eq (a2, strlen (gottenValue)))) { 
	  set (bb, 
	    save (substring (gottenValue, 2, sub (strlen (gottenValue), 1))))
	}
      }
      set (b, save (concat (b, concat ("\\cite{", concat (bb, "}")))))
    }
  
    if (figureFlag) {
      set (b, 
        save (concat (b, concat (" See figure on page~", concat (pref, ".")))))
    }
  
    while (i, index (b, "\n", 1)) {
      set (b, save (concat (substring (b, 1, sub (i, 1)),
      			    concat (" ", 
				    substring (b, add (i, 1), strlen (b))))))
    }

    enqueue (bibList, save (concat (b, "\n")))
  }
}



