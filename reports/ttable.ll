/*
 * @progname       ttable.ll
 * @version        none
 * @author         anon
 * @category       
 * @output         Text
 * @description
 *
 * Compute Pete Cook's C-Table comparison vector, as modified by Tom.
 */

proc main ()
{
        getindi(i, "Compute C-Table for what person?")
        if (not(i)) { return() }
        set(b, getyear(birth(i)))
        set(f,  father(i)) set(m,  mother(i))
        set(ff, father(f)) set(fm, mother(f))
        set(mf, father(m)) set(mm, mother(m))
        set(bf,  getyear(birth(f))) set(bm,  getyear(birth(m)))
        set(bff, getyear(birth(ff))) set(bfm, getyear(birth(fm)))
        set(bmf, getyear(birth(mf))) set(bmm, getyear(birth(mm)))
        print( b, " ", bf, " ", bm, " ")
        print(bff, " ", bfm, " ", bmf, " ", bmm, " ")
        print(sex(i), " ", soundex(i), " ", trim(givens(i), 1), "\n")
}
func getyear(event)
{
        set(y, atoi(year(event)))
        if (and(ge(y, 1000), le(y, 2000))) {return(year(event))}
        return("0000")
}
