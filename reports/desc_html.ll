/*
 * @progname       desc_html.ll
 * @version        1.4
 * @author         Dick Knowles, Scott McGee, anon
 * @category       
 * @output         HTML
 * @description

This program is designed to be used in a cgi based genweb site to produce
a descendant chart for a specified individual. It is based on the desc-tree
program by Dick Knowles as modified by Scott McGee. A line is printed for 
every spouse and child including name, database key number, birth, marriage, 
and death information.

@(#)desc_html.ll	1.4 10/4/95
*/

include("cgi_html.li")

global(MAXGENS)
global(MAXLINES)
global(linecount)
global(gens)
global(mainpre)
global(spousepre)
global(indentpre)

proc main () {
  call set_cgi_html_globals()

  set(MAXGENS,20)             /* make "all" gens max at 20 */
  set(MAXLINES,500)           /* set max report lines */
  set(linecount,0)            /* initialize linecount */
  set(nm," ")
  getindi(nm)                 /* get individual */

  set(gens, 3)

  dayformat(0)
  monthformat(4)
  dateformat(0)

  set(mainpre, "-")
  set(spousepre, " s-")
  set(indentpre, "  |")

  call do_chart_head(nm, "Descendant")
  "<PRE>\n"
  call dofam(nm,"",1,0)               /* start with first person */
  "</PRE>\n"
  call do_tail(nm)
}


/* startfam:
   If we haven't reached the maximum or specified generation count,
   call dofam for each child in this family.
   Otherwise, print a message line if there are further descendants
   at this point.
*/
proc startfam (fam,prefix,level,isstep) {
    if (le(level,gens)) {               /* if not at last generation */
        children (fam,child,num) {      /* for each child */
            call dofam (child,          /* call dofam */
                        concat(prefix, indentpre),
                        add(level,1),
                        isstep)
        }
    } else {                            /* don't do this generation */
        if (gt(nchildren(fam),0)) {     /* but if there are children, */
                                        /* issue message */
            prefix "  [[Further descendants here"
            if (eq(isstep,1)) {
                " (stepchildren)"
            }
            ".]]\n"
            incr(linecount)
        }
    }
}

/* dofam:
   Write out a person and check for spouses and children.
   Each spouse is written, then this routine is called
   recursively for each child.  An incremented level is passed along
   in case the user specified a limited number of generations
*/

proc dofam (nm,prefix,level,isstep) {
  set(pre,mainpre)
  call printpers(nm,concat(prefix,pre),0,0)  /* print this person */
  if (and(ge(linecount,MAXLINES),gt(nfamilies(nm),0))) {
    prefix "  [[Reached line count max."
    "  May be further descendants here."
    "]]\n"
  } else {
    families(nm, fam, spouse, num) {   /* do for each family */
      if (ne(spouse,null)) {         /* if there is a spouse */
        /* print spouse */
        call printpers(spouse,concat(prefix,spousepre),1,fam)
        if (and(ge(linecount,MAXLINES),gt(nchildren(fam),0))) {
          prefix "  [[Reached line count max."
          "  May be further descendants here."
          "]]\n"
        } else {
          families (spouse, spsfam, ospouse, num2) {
            /* for each of the spouse families*/
            if(eq(fam,spsfam)){  /* only non-step families */
              call startfam (spsfam,prefix,level,0)
            }
          } /*end spouse's families*/
        } /* end spouse not ge MAXLINES */
      } else {                    /* there is no spouse */
        call startfam (fam,prefix,level,0)
      } /*end else no spouse*/
    } /*end 'families'*/
  } /* end MAXLINES else */
} /*end 'proc dofam'*/


/* printpers:
   Write output line for one person.
   Include birth and death dates if known.
   For a spouse, include marriage date if known.
*/
proc printpers (nm, prefix, spouse, fam) {
  prefix 
  if(nfamilies(nm)){
    set(hasChildren, 0)
    families(nm, f, s, n){
      if(nchildren(f)){
        set(hasChildren, 1)
      }
    }
  }
  href(nm, "Descendant")
  if(e, birth(nm)) {
    "\t b. " stddate(birth(nm))
  }
  if(e, marriage(fam)) {
    "\t m. " stddate(e)
  }
  if(e, death(nm)) {
    "\t d. " stddate(death(nm))
  }
  "\n"
  incr(linecount)
} /* end proc printpers */


/*
   key_no_char:
     Return string key of individual or family, without
     leading 'I' or 'F'.
*/
proc key_no_char (nm) {
    set(k, key(nm))
    substring(k,2,strlen(k))
} /* end proc key_no_char */
