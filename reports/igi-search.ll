/*
 * @progname    igi-search.ll
 * @version     1.1
 * @author      Vincent Broman
 * @category
 * @output      Text
 * @description
 *
 *     prints out a list of people to look up in the IGI,
 *     those who are closely enough related, who fall in a time range,
 *     and whose temple work is not done.
 */

include( "ldsgedcom.li")

/* TODO: needs a sort and handling "OF" places */

/*
 * indvordsdone and marrordsdone test whether all the individual (Bap/End/Sch)
 * or marriage (Ssp) ordinances are recorded for this individual
 * in the database.
 */

func indvordsdone( indi) {
    return( and( ldsbaptism( indi),
		 ldsendowment( indi),
		 or( ldschildsealing( indi),
		     not( father( indi)))))
}

func marrordsdone( indi) {
    families( indi, f, sp, ctm) {
	if( not( ldsspousesealing( f))) { return( 0) }
    }
    return( 1)
}

/* return as a string the birth/christening year if known, else 0 */
func knownbirthyear( indi) {
    set( b, birth( indi))
    if( not( b)) { set( b, baptism( indi)) }
    if( b) {
	return( save( year( b)))
    }
    return( 0)
}

global( thisyear)

/* isdead tests whether this individual is known to be dead
 * or can be assumed dead by the rules for LDS temple work submission.
 */
func isdead( indi) {
    if( death( indi)) { return( 1) }
    if( burial( indi)) { return( 1) }
    if( by, knownbirthyear( indi)) {
	return( le( strtoint( by), sub( thisyear, 110)))
    }
    families( indi, f, sp, ctd) {
	if( m, marriage( f)) {
	    if( md, year( m)) {
		if( le( strtoint( md), sub( thisyear, 95))) {
		    return( 1)
		}
	    }
	}
    }
    return( 0)
}

/* return the set of people who are wide-sense ancestors
 * of the given individual, plus the children of these wide-sense ancestors,
 * where a wide-sense ancestor is either the given individual himself/herself
 * a parent or step-parent of a wide-sense ancestor.
 */
func interestingforebearsof( indi) {
    indiset( res)
    addtoset( res, indi, key( indi))

    set( pf, parents( indi))
    if( not( pf)) { return( res) }

    if( h, husband( pf)) {
	set( res, union( res, interestingforebearsof( h)))
	families( h, f, sp, ctf) {
	    if( sp) {
		set( res, union( res, interestingforebearsof( sp)))
	    }
	    children( f, ch, ctc) {
		addtoset( res, ch, key( ch))
	    }
	}
	set( hk, key( h))
    } else {
	set( hk, "")
    }
    if( w, wife( pf)) {
	families( w, f, sp, cth) {
	    /* add only husbands not the father */
	    if( and( sp, nestr( hk, key( sp)))) {
		set( res, union( res, interestingforebearsof( sp)))
	    } else {
		children( f, ch, ctcc) {
		    addtoset( res, ch, key( ch))
		}
	    }
	}
    }
    return( res)
}

proc igiord( e) {
    if( e) {
	if( v, value( e)) {
	    v
	} else {
	    if( ed, date( e)) {
		ed
	    } else {
		"           "
	    }
	    " "
	    if( et, ldstemple( e)) {
		et
	    } else {
		"     "
	    }

	}
    }
}

proc printigientry( indi) {
    if( n, name( indi)) {
	n "  "
    } else {
	"<nameless>  "
    }
    "("
    if( fa, father( indi)) {
	if( fn, name( fa)) {
	    fn
	}
    }
    "/"
    if( mo, mother( indi)) {
	if( mn, name( mo)) {
	    mn
	}
    }
    ")  " nl()
    if( b, birth( indi)) {
        sex( indi)
	"B " long( b)
	if( c, baptism( indi)) {
	    " and "
	    sex( indi)
	    "C " long( c)
	}
    } elsif( c, baptism( indi)) {
        sex( indi)
	"C " long( c)
    } else {
        sex( indi)
	" no B/C event  "
    }
    nl()
    set( ba, ldsbaptism( indi))
    set( en, ldsendowment( indi))
    set( cs, ldschildsealing( indi))
    if( or( ba, en, cs)) {
	call igiord( ba)
	"/"
	call igiord( en)
	"/"
	call igiord( cs)
	nl()
    }
    nl()
}

proc main() {
    set( thisyear, strtoint( year( gettoday())))

    getindi( i, "Who's ancestors should be checked for IGI entries?")
    getint( fby, "First birth year of interest?")
    getint( lby, "Last  birth year of interest?")
    forindiset( interestingforebearsof( i), ai, k, ctb) {
	if( by, knownbirthyear( ai)) {
	    set( iby, strtoint( by))
	    if( and( isdead( ai),
		     not( indvordsdone( ai)),
		     le( fby, iby),
		     le( iby, lby))) {
		by nl()
		call printigientry( ai)
	    }
	}
    }
}
