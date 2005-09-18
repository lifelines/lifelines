/*
 * @progname       est_life_span.ll
 * @version        none
 * @author         Rafal Prinke
 * @category       
 * @output         Text
 * @description

The program below finds the widest possible span of life, estimating
the uncertain and some unknown dates on the basis of user defined
values in the table for various combinations of date modifiers etc.
Then it compares the midpoints of this with midpoints of others'
life spans and if they fall within a user defined range, the two
are considered "possibly identical". This is actually just one procedure
of several that should be in a program like that - but perhaps the
most important one as people may change names, occupations, etc.
but they cannot change the time period. I added two arbitrary tags:
FAPP and LAPP for "first/last appearance" in sources.

At present the program compares only the life midpoints and surnames
(exactly as they are recorded). Adding other elements for comparison
should not be a problem - but there should be a more complicated
procedure for comparing names. I do not like soundex as it is misleading
(and my bias is founded on the fact that two spellings of my own
surname - Prinke and Brinke - never match under the standard soundex :-)
Perhaps a soundex that would INCLUDE conversion of the first character
into number would nee to be considered? That does not solve many problems
of non-English name, either. So perhaps a user defined soundex? This
might be possible with user defined functions!

The procedure below does not deal with BETWEEN dates for death yet.
Also people who have no dates at all are not dealt with properly -
they should be compared with everyone (generating a lot of rubbish
output) or their life span should be found out from children or spouses.
 */

global(year1)  /* earliest possible */
global(year2)  /* latest possible */
global(diffs)
global(someone)
global(another)

proc life_span(someone)
{
        set(year1,0)
        if (bt,birth(someone)) {
                set(year1,atoi(year(bt)))
                if (ne(index(upper(date(bt)),"BEF",1),0)) {
                        set(year1,sub(year1,lookup(diffs,"bef_birt")))
                }
                if (and(ne(index(upper(date(bt)),"ABT",1),0),ne(index(upper(date(bt)),"EST",1),0)))
		{
                        set(year1,sub(year1,lookup(diffs,"abt_birt")))
                }
        }
        if (eq(year1,0)) {
                if (bp,baptism(someone)) {
                        set(year1,atoi(year(bp)))
                        if (ne(index(upper(date(bp)),"BEF",1),0)) {
                                set(year1,sub(year1,lookup(diffs,"bef_birt")))
                        }
                        if (and(ne(index(upper(date(bp)),"ABT",1),0),ne(index(upper(date(bp)),"EST",1),0)))
			{
                                set(year1,sub(year1,lookup(diffs,"abt_birt")))
                        }
                }
        }
        if (eq(year1,0)) {
                set(r, inode(someone))
                fornodes (r, n) {
                        if (eq(0, strcmp("FAPP", tag(n)))) {
                                extractdate(n,da,mo,ye)
                                set(year1,ye)
                                set(year1,sub(year1,lookup(diffs,"app1")))
                        }
                }
        }
        if (and(eq(year1,0),ne(nfamilies(someone),0))) {
                set(myear,2000)
                families(someone,fm,sp,mnr) {
                        set(fyear,atoi(year(marriage(fm))))
                        if (lt(fyear,myear)) {
                                set (myear,fyear)
                                set(mar,marriage(fm))
                        }
                }
                if (and(ne(myear,2000),ne(myear,0))) {
                        set(year1,sub(myear,lookup(diffs,"f_marr")))
                        if (ne(index(upper(date(mar)),"BEF",1),0)) {
                                set(year1,sub(year1,lookup(diffs,"bef_marr")))
                        }
                        if (and(ne(index(upper(date(mar)),"ABT",1),0),ne(index(upper(date(bp)),"EST",1),0)))
			{
                                set(year1,sub(year1,lookup(diffs,"abt_marr")))
                        }
                }
        }

        set(year2,0)
        if (dt,death(someone)) {
                set(year2,atoi(year(dt)))
                if (ne(index(upper(date(dt)),"AFT",1),0)) {
                        set(year2,add(year2,lookup(diffs,"aft_deat")))
                }
                if (and(ne(index(upper(date(dt)),"ABT",1),0),ne(index(upper(date(dt)),"EST",1),0)))
		{
                        set(year2,add(year2,lookup(diffs,"abt_deat")))
                }
        }
        if (eq(year2,0)) {
                if (br,burial(someone)) {
                        set(year2,atoi(year(br)))
                        if (ne(index(upper(date(br)),"AFT",1),0)) {
                                set(year2,add(year2,lookup(diffs,"aft_deat")))
                        }
                        if (and(ne(index(upper(date(br)),"ABT",1),0),ne(index(upper(date(br)),"EST",1),0)))
			{
                                set(year2,add(year2,lookup(diffs,"abt_deat")))
                        }
                }
        }
        if (eq(year2,0)) {
                set(r, inode(someone))
                fornodes (r, n) {
                        if (eq(0, strcmp("LAPP", tag(n)))) {
                                extractdate(n,da,mo,ye)
                                set(year2,ye)
                                set(year2,add(year2,lookup(diffs,"app2")))
                        }
                }
        }
        if (and(eq(year2,0),ne(nfamilies(someone),0))) {
                set(myear,0)
                families(someone,fm,sp,mnr) {
                        set(lyear,atoi(year(marriage(fm))))
                        if (gt(lyear,myear)) {
                                set (myear,lyear)
                                set(mar,marriage(fm))
                        }
                }
                if (ne(myear,0)) {
                        set(year2,add(myear,lookup(diffs,"l_marr")))
                        if (ne(index(upper(date(mar)),"AFT",1),0)) {
                                set(year2,add(year2,lookup(diffs,"aft_marr")))
                        }
                        if (and(ne(index(upper(date(mar)),"ABT",1),0),ne(index(upper(date(mar)),"EST",1),0)))
			{
                                set(year2,add(year2,lookup(diffs,"abt_marr")))
                        }
                }
        }
        if (ne(add(year1,year2),0)) {
        if (and(eq(year1,0),ne(year2,0))) {
                set(year1,sub(year2,lookup(diffs,"birt_deat")))
        }
        if (and(eq(year2,0),ne(year1,0))) {
                set(year2,add(year1,lookup(diffs,"birt_deat")))
        }
        }
}

proc main()
{
        table(diffs)   /* values for range of date modifiers etc. */
        insert(diffs,"bef_birt", 10)
        insert(diffs,"abt_birt", 10)
        insert(diffs,"aft_deat", 10)
        insert(diffs,"abt_deat", 10)
        insert(diffs,"f_marr",25)
        insert(diffs,"l_marr",13)
        insert(diffs,"bef_marr", 10)
        insert(diffs,"abt_marr", 8)
        insert(diffs,"aft_marr", 2)
        insert(diffs,"app1", 10)
        insert(diffs,"app2", 10)
        insert(diffs,"birt_deat", 88)
        insert(diffs,"dist", 50)

        forindi(someone,n1) {
                call life_span(someone)
                set(pb1,surname(someone))
                set(pb1,save(pb1))
          set(midspan1,div(add(year1,year2),2))
                forindi(another,n2) {
                print(d(n1)," ",d(n2),"\n")
                    if (gt(n2,n1)) {
                        call life_span(another)
                        set(pb2,surname(another))
                        set(pb2,save(pb2))
                        set(midspan2,div(add(year1,year2),2))
                        set(cont,0)
if (ge(midspan1,midspan2)) {
        if (lt(sub(midspan1,midspan2),lookup(diffs,"dist"))) {
                set(cont,1)
        }
}
if (ge(midspan2,midspan1)) {
        if (lt(sub(midspan2,midspan1),lookup(diffs,"dist"))) {
                set(cont,1)
        }
}
 if (and(eq(cont,1),eq(strcmp(pb1,pb2),0))) {
                " possibly contemporary: " nl()
          key(someone) " " name(someone) " " d(midspan1) " " pb1 nl()
                key(another) " " name(another) " " d(midspan2) " " pb2 nl()

 }
                    }
                }
        }
}
