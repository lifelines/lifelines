/*
 * @progname       index_mm.ll
 * @version        4.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program lists everyone in a database, with women listed by both
maiden name and married name.  It assumes that all women take the surname
of their husbands, which is not always correct.

index_mm - a LifeLines database listing program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  25 November 1992
        Version 2,  29 November 1992 (gave up on bubblesort)
        Version 3,  11 January  1993 (added deathdate and marriage)
        Version 4,  18 April    1993 (bug fix, made namewidth variable)

To sort the resulting report by name, enter the Unix command
        sort -b +1 report > sorted.report
*/


proc main ()
{
    list(names)
    list(keys)
    list(indices)

    set(namewidth,40)  /* change this value as needed */

    ". ." col(8) "LAST, First Middle [MAIDEN]"
    set(bcol,add(8,namewidth))
    col(bcol) "Birthdate"
    set(dcol,add(22,namewidth))
    col(dcol) "Deathdate"
    set(mcol,add(36,namewidth))
    col(mcol) "Marriage"

    set(marriednum,0)
    print("Writing names...")
    set(nextrep,0)
    forindi(person,num) {
        if (ge(num,nextrep)) {
            print(d(num)) print(" ")
            set(nextrep,add(nextrep,100))
        }
        if (b,birth(person)) { set(bdate,date(b)) }
        else { set(bdate,date(baptism(person))) }
        if (d,death(person)) { set(ddate,date(d)) }
        else { set(ddate,date(burial(person))) }
        key(person) col(8) fullname(person,1,0,namewidth)
        col(bcol) bdate col(dcol) ddate
        families(person,fam,spouse,fnum) {
            if (eq(fnum,1)) {
                col(mcol) date(marriage(fam))
            }
        }
        nl()
        if (female(person)) {
            set(maidenname,save(concat(", ",fullname(person,1,1,100))))
            spouses(person,spouse,fam,fnum) {
                if (spousesurname,surname(spouse)) {
                    set(mdate,date(marriage(fam)))
                    key(person) col(8)
                    trim(concat(upper(spousesurname),maidenname),namewidth)
                    col(bcol) bdate col(dcol) ddate col(mcol) mdate nl()
                    set(marriednum,add(marriednum,1))
                }
            }
        }
    }
    print("\nWrote ") print(d(num)) print(" database names and ")
    print(d(marriednum)) print(" married names.\n")
}
