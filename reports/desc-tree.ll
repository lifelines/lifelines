/*
 * @progname    desc-tree.ll
 * @version     8
 * @author      Dick Knowles, knowles@inmet.camb.inmet.com
 * @category
 * @output      Text
 * @description
 *
        This report prints a descendant tree for an individual.  A
        line is printed for every spouse and child including name,
        database key number, birth, marriage, and death information.
        The user can set the number of generations or they can all be
        done (up to a maximum of 20).  The user can also, optionally,
        include step children and family database numbers.  There are
        two slightly different output styles, tree and numbered.  Here
        are examples of each:


        Dick Knowles, knowles@inmet.camb.inmet.com
        18 Feb 1993
        18 Mar 1993 ver. 2  Add date to heading
        19 Dec 1993 ver. 3  (partial) Changes for Cliff Manis
        30 Dec 1993 ver. 4  Updates suggested by Cliff Manis
        07 Mar 1994 ver. 5  Make 0 generations max at 20.
                            Add message when stopping for gen count.
        10 Mar 1994 ver. 6  Add max line count to limit output.
        10 Aug 1994 ver. 7  Bugfix (by Jim Eggert).
        31 Aug 1997 ver. 8  Added old bugfix for incorrect printing of step
                            children. (Source of fix unknown at this time.)

----------------- numbered:

Tree of descendants for Thomas Leo SARJEANT (19)

Dated: 30 Dec 1993

1- Thomas Leo SARJEANT (19)      b. 27 Mar 1916  d.  1 Oct 1978
s- Rita LACROIX (59)     b. 28 Aug 1918  m.        1936  d. 28 Sep 1974
   2- Thomas Leo SARJEANT (60)   b. 26 Feb 1936
   s- Joan MERRIAM (69)  m. 13 May 1961
      3- Thomas John SARJEANT (70)
      3- Marjorie SARJEANT (71)
      3- James SARJEANT (72)
   2- John Bernard SARJEANT (61)         b.  8 Nov 1939
   s- Bettye MCPHERSON (504)     b. 18 Apr 1932  m.  2 Sep 1973
   2- Beverly Ann SARJEANT (62)  b. 28 Jul 1942
   s- Steven JOHNSON (73)        m. 10 Nov 1960
   s- Joseph COSTA (74)  b. 20 Apr 1926  m.  8 Oct 1963
      3- Michael Angelo COSTA (75)       b. 30 Jun 1965
      s- Elaine CARTER (319)     b.  4 Feb 1966  m. 26 May 1990
s- Charlotte Lois BENJAMIN (20)  b. 29 Nov 1923  m. 12 Oct 1949
   2- Kathleen SARJEANT (14)     b. 23 Jan 1950
   s- Richard James KNOWLES (3)  b. 20 Nov 1949  m. 14 Aug 1971
      3- Jennifer Danielle KNOWLES (15)  b. 28 Oct 1974
      3- Kevin Scott KNOWLES (16)        b. 14 May 1976
      3- James Michael KNOWLES (17)      b. 13 Oct 1979
      3- Brenda Marie KNOWLES (18)       b.  7 Oct 1981


----------------- tree (with stepchildren and family numbers):

Tree of descendants for Thomas Leo SARJEANT (19)

Dated: 30 Dec 1993

-Thomas Leo SARJEANT (19)        b. 27 Mar 1916  d.  1 Oct 1978
 s-Rita LACROIX (59)     b. 28 Aug 1918  m.        1936 (17)     d. 28 Sep 1974
  |-Thomas Leo SARJEANT (60)     b. 26 Feb 1936
  | s-Joan MERRIAM (69)  m. 13 May 1961 (20)
  |  |-(ST)Deborah CONNORS (295)
  |  |-(ST)Diane CONNORS (296)
  |  | s-John LIPSEY (482)       b.      m.  (160)
  |  |  |-John LIPSEY (479)      b. 27 Oct 1979  d.  8 Mar 1993
  |  |-(ST)Gayle CONNORS (297)
  |  |  |-Jennifer (483)         b.
  |  |-Thomas John SARJEANT (70)
  |  |-Marjorie SARJEANT (71)
  |  |-James SARJEANT (72)
  |-John Bernard SARJEANT (61)   b.  8 Nov 1939
  | s-Bettye MCPHERSON (504)     b. 18 Apr 1932  m.  2 Sep 1973 (168)
  |  |-(ST)Tammarra Victoria WALL (505)  b.  7 Jul 1963
  |-Beverly Ann SARJEANT (62)    b. 28 Jul 1942
  | s-Steven JOHNSON (73)        m. 10 Nov 1960 (21)
  | s-Joseph COSTA (74)  b. 20 Apr 1926  m.  8 Oct 1963 (22)
  |  |-Michael Angelo COSTA (75)         b. 30 Jun 1965
  |  | s-Elaine CARTER (319)     b.  4 Feb 1966  m. 26 May 1990 (89)
 s-Charlotte Lois BENJAMIN (20)  b. 29 Nov 1923  m. 12 Oct 1949 (3)
  |-Kathleen SARJEANT (14)       b. 23 Jan 1950
  | s-Richard James KNOWLES (3)  b. 20 Nov 1949  m. 14 Aug 1971 (2)
  |  |-Jennifer Danielle KNOWLES (15)    b. 28 Oct 1974
  |  |-Kevin Scott KNOWLES (16)  b. 14 May 1976
  |  |-James Michael KNOWLES (17)        b. 13 Oct 1979
  |  |-Brenda Marie KNOWLES (18)         b.  7 Oct 1981

*/

global(MAXGENS)
global(MAXLINES)
global(linecount)
global(gens)
global(style)
global(dofami)
global(dostep)
global(mainpre)
global(spousepre)
global(indentpre)

proc main () {
    set(MAXGENS,20)             /* make "all" gens max at 20 */
    set(MAXLINES,500)           /* set max report lines */
    set(linecount,0)            /* initialize linecount */
    set(nm," ")
    getindi(nm)                 /* get individual */
    getintmsg (gens,
        concat("How many generations (0 for all, max ",
          concat(d(MAXGENS),")?")))
    if (eq(gens,0)) {set(gens,MAXGENS)} /* if 0, set max */
    getintmsg (style,
               "Choose style: 0 for tree, 1 for numbered.")
    getintmsg (dofami,
               "Include family indices? 0 for no, 1 for yes.")
    getintmsg (dostep,
               "Show stepchildren? 0 for no, 1 for yes.")

    dayformat(0)
    monthformat(4)
    dateformat(0)

    /* Headers */
    "Tree of descendants for " name(nm) " (" call key_no_char(nm) ")\n\n"
    "Dated: " stddate(gettoday()) "\n\n"

    if (eq(style,0)) {          /* if tree */
        set(mainpre, "-")
        set(spousepre, " s-")
        set(indentpre, "  |")
    } else {                    /* if numbered */
        set(mainpre, "- ")
        set(spousepre, "s- ")
        set(indentpre, "   ")
    }
    call dofam(nm,"",1,0)               /* start with first person */

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
            prefix "  [[Reached gen count or max.  Further descendants here"
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
    if (eq(style,0)) {
        set(pre,mainpre)
    } else {
        set(pre,concat(d(level),mainpre))
    }
    if (eq(isstep,1)) {
        call printpers(nm,
          concat(prefix,concat(pre,"(ST)")),0,0)  /* print this person */
    } else {
        call printpers(nm,concat(prefix,pre),0,0)  /* print this person */
    }
    if (and(ge(linecount,MAXLINES),gt(nfamilies(nm),0))) {
        prefix "  [[Reached line count max."
        "  May be further descendants here."
        "]]\n"
    } else {
        families(nm, fam, spouse, num) {   /* do for each family */
            if (ne(spouse,null)) {         /* if there is a spouse */
                call printpers(            /* print spouse */
                    spouse,concat(prefix,spousepre),1,fam)
                if (and(ge(linecount,MAXLINES),gt(nchildren(fam),0))) {
                    prefix "  [[Reached line count max."
                    "  May be further descendants here."
                    "]]\n"
                } else {
                    families (spouse, spsfam, ospouse, num2) {
                                           /* for each of the spouse families*/
                        if (eq(fam,spsfam)) {/* this is main family */
                            call startfam (spsfam,prefix,level,0)
                        } else {                /* this is step-family*/
                            if (eq (dostep,1)) { /* if we're doing stepfams */
                                call startfam (spsfam,prefix,level,1)
                            } /*end if dostep */
                        } /*end else not stepfam*/
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
    prefix name(nm) " (" call key_no_char(nm) ")"
    if(e, birth(nm)) {
        "\t b. " stddate(birth(nm))
    }
    if(e, marriage(fam)) {
        if(eq(dofami,1)) {
            "\t m. " stddate(e) " (" call key_no_char(fam) ")"
        } else {
            "\t m. " stddate(e)
        }
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
