/*
 * @progname       pedigree_html.ll
 * @version        1.3
 * @author         Scott McGee (smcgee@microware.com)
 * @category       
 * @output         HTML
 * @description
 *
 *   Select and produce an ancestor report for the person selected.
 *
 *   @(#)pedigree_html.ll	1.3 10/4/95
 *
 *   By Scott McGee (smcgee@microware.com)
 *   Based on pedigreel by Tom Wetmore, ttw@cbnewsl.att.com
 *   And Cliff Manis
 */

include("cgi_html.li")


proc main () {
  call set_cgi_html_globals()

  set (nl,nl())
  getindi(indi)

  call do_chart_head(indi, "Pedigree")
  "<PRE>\n"
  call pedigree(0, indi)
  "</PRE>\n"
  call do_tail(indi)
}

proc pedigree (level, indi) {
  set(has_parent, or(father(indi), mother(indi)))
  if(and(lt(level, 4), has_parent)) {
    set(par, father(indi))
    call pedigree(add(1,level), par)
  }
  if(indi) {
    col(mul(4,level))
    href(indi, "Pedigree")
    if (evt, birth(indi)) {
      ", b. "
      if(gt(level, 3)) {
        short(evt)
      }
      else {
        long(evt)
      }
    }
    nl()
  }
  else {
  	col(mul(4,level))
  	"(Spouse not known)"
    nl()
  }
  if(and(lt(level, 4), has_parent)) {
    set(par, mother(indi))
    call pedigree(add(1,level), par)
  }
}
