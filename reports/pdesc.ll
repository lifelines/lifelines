/*
 * @progname       pdesc.ll
 * @version        4.3
 * @author         Wetmore, Manis, Jones, Eggert, Simms
 * @category       
 * @output         Text
 * @description    
 *
 * Produces indented descendant list with line wrapping at 78 columns
 * (user-specifiable) while maintaining the indentation level. Enhancement
 * from version 2 is the addition of user-specified maximum number of
 * generations. Version 4 makes the page_width (not 1 less) the limit on
 * character a shift inplacement. Also eliminated an extra space at the
 * beginning of each line that was not controlled by a left_margin parameter.
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *   With modifications by:  James P. Jones
 *   With modifications by:  Jim Eggert (unknown spouse bugfix)
 *   With modifications by:  Robert Simms (indented line wrap) Mar '96
 *                                        (max number of generations) Jun '97
 *                                        (line wrap cleaned up) 16 Feb 2000
 *   With modifications by:  Vincent Broman (header cleanup) 2003-02
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
global(max_depth)
global(left_margin)

proc main () {
	set(page_size, 78)
	set(tab_size, 3)    /* extra indentation upon line-wrap */
	set(left_margin, 0)

	getindi(indi)
	getintmsg (max_depth, "Maximum number of generations")
	set(skip, left_margin)
	call pout(0, indi)

	"===============================================" nl()
}

proc pout(gen, indi) {
	set(skip, mul(4,gen))
	col(add(skip, 1))
	set(x, skip)
	set(s, concat(d(add(gen, 1)), "--"))
	s
	set(x, add(x, tab_size))
	set(skip, x)
	call outp(indi, skip, x)
	set(next, add(1, gen))
	families(indi,fam,sp,num) {
		set(skip, add(2,mul(4,gen)))
		col(add(skip, 1))
		set(x, skip)
		"sp-"
                /* Don't try to show a spouse name if none known */
                if (sp) {
                        set(x, add(x, 4))
                        set(skip, x)
                        call outp(sp, skip, x)
                } else {
                        "Unknown" nl()
                }
		if (lt(next,max_depth)) {
			children(fam, child, no) {
				call pout(next, child)
			}
		}
	}
}

proc outp(indi, skip, x) {
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
		col(add(skip, 1))
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
