/* 
 * @progname       cid.ll
 * @version        1.0
 * @author         Wetmore, Chandler
 * @category       
 * @output         Text
 * @description    

   Generates Pete Cook's CID (Chronological Identifier) for a person
   in a LifeLines database. 

   The program first computes the C-Vector, a
   seven element array of the birth years of a person and his/her parents
   and grandparents in ahnentafel order.  The program then subtracts the
   base person's birth year from those of the others.  Those differences
   are converted to the follow letters:

         Char   Parent Age   Grandparent Age
          0        0-15           0-30
          1-9     16-24          31-39
          A-Z     25-50          40-65
          a-y     51-75          66-90
          z     over 75        over 90
          -     unknown        unknown

    Version 1, 15 Mar 1995, Tom Wetmore, modified by J.F.Chandler
*/

proc main ()
{
        getindi(i, "Compute CID for what person?")
        if (eq(0, i)) { return() }
        set(b, getyear(birth(i)))
        if (lt(b, 1000)) {
                print("Base person has no birth year")
                return()
        }
        set(f,  father(i))
        set(m,  mother(i))
        set(ff, father(f))
        set(fm, mother(f))
        set(mf, father(m))
        set(mm, mother(m))

        set(bf,  getyear(birth(f)))
        set(bm,  getyear(birth(m)))
        set(bff, getyear(birth(ff)))
        set(bfm, getyear(birth(fm)))
        set(bmf, getyear(birth(mf)))
        set(bmm, getyear(birth(mm)))

        set(bf,  sub(sub(b, bf), 15))
        set(bm,  sub(sub(b, bm), 15))
        set(bff, sub(sub(b, bff), 30))
        set(bfm, sub(sub(b, bfm), 30))
        set(bmf, sub(sub(b, bmf), 30))
        set(bmm, sub(sub(b, bmm), 30))

        print("The CID for ", name(i), " is:  ", d(b), letter(bf),
            letter(bm), letter(bff), letter(bfm), letter(bmf),
            letter(bmm), "\n")
}

func getyear(event)
{
        set(mod, trim(date(event),3))
        if (and( strcmp(mod,"BEF"),
                 strcmp(mod,"AFT"),
                 strcmp(mod,"ABT") )) { return(atoi(year(event))) }
        return(0)
}

func letter (yr)
{
        if (gt(yr, 500)) { return("-") }
        if (lt(yr, 0))  { return("0") }
        if (le(yr, 9))  { return(d(yr)) }
        if (eq(yr, 10)) { return("A") }
        if (eq(yr, 11)) { return("B") }
        if (eq(yr, 12)) { return("C") }
        if (eq(yr, 13)) { return("D") }
        if (eq(yr, 14)) { return("E") }
        if (eq(yr, 15)) { return("F") }
        if (eq(yr, 16)) { return("G") }
        if (eq(yr, 17)) { return("H") }
        if (eq(yr, 18)) { return("I") }
        if (eq(yr, 19)) { return("J") }
        if (eq(yr, 20)) { return("K") }
        if (eq(yr, 21)) { return("L") }
        if (eq(yr, 22)) { return("M") }
        if (eq(yr, 23)) { return("N") }
        if (eq(yr, 24)) { return("O") }
        if (eq(yr, 25)) { return("P") }
        if (eq(yr, 26)) { return("Q") }
        if (eq(yr, 27)) { return("R") }
        if (eq(yr, 28)) { return("S") }
        if (eq(yr, 29)) { return("T") }
        if (eq(yr, 30)) { return("U") }
        if (eq(yr, 31)) { return("V") }
        if (eq(yr, 32)) { return("W") }
        if (eq(yr, 33)) { return("X") }
        if (eq(yr, 34)) { return("Y") }
        if (eq(yr, 35)) { return("Z") }
        if (eq(yr, 36)) { return("a") }
        if (eq(yr, 37)) { return("b") }
        if (eq(yr, 38)) { return("c") }
        if (eq(yr, 39)) { return("d") }
        if (eq(yr, 40)) { return("e") }
        if (eq(yr, 41)) { return("f") }
        if (eq(yr, 42)) { return("g") }
        if (eq(yr, 43)) { return("h") }
        if (eq(yr, 44)) { return("i") }
        if (eq(yr, 45)) { return("j") }
        if (eq(yr, 46)) { return("k") }
        if (eq(yr, 47)) { return("l") }
        if (eq(yr, 48)) { return("m") }
        if (eq(yr, 49)) { return("n") }
        if (eq(yr, 50)) { return("o") }
        if (eq(yr, 51)) { return("p") }
        if (eq(yr, 52)) { return("q") }
        if (eq(yr, 53)) { return("r") }
        if (eq(yr, 54)) { return("s") }
        if (eq(yr, 55)) { return("t") }
        if (eq(yr, 56)) { return("u") }
        if (eq(yr, 57)) { return("v") }
        if (eq(yr, 58)) { return("w") }
        if (eq(yr, 59)) { return("x") }
        if (eq(yr, 60)) { return("y") }
        return("z")
}
