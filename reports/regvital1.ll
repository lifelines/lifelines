/* 
 * @progname       regvital1
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         nroff
 * @description    
 *
 *   regvital1
 *
 *   This report does have a footer and header
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   Modifications by Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *   and it has been modified many times since.
 *
 *   It will produce a report of all descendents of a person,
 *   and is presently designed for 12 pitch, HP laserjet III,
 *   for printing a book about that in ancestor.
 *
 *   It prints a sorts listing of names, at the end of the report
 *   of everyone in the report.  All NOTE and CONT lines will
 *   be printed in the this report.  This report will produced
 *   a paginated output.
 * 
 *   This report produces a nroff output, and to produce the
 *   output, use:  nroff filename > filename.out
 *
 */
 
global(idex)
proc main ()
{
    set (nl,nl())
    getindi(indi)
    monthformat(4)
    set(tday, gettoday())
    ".de hd" nl()
    ".ev 1" nl()
    ".sp" nl()
    /*  ".ce" nl()  */
    /*------------------------------------------------*/
    "Manes - Manis - Maness  Family  History" nl()
    col(60)stddate(tday) nl()
    ".sp" nl()
    "" nl()
    "'sp .8i" nl()
    ".ev" nl()
    ".." nl()
    ".de fo" nl()
    ".ev 1" nl()
    ".sp" nl()
    ".tl  'Cliff Manis, P. O. Box 33937, San Antonio, TX 78265, 1-(512)-654-9912'''" nl ()
    ".sp" nl()
    ".ev" nl()
    "'bp" nl()
    ".." nl()
    ".wh 0 hd" nl()
    ".wh -.8i fo" nl()
    ".de CH" nl()
    ".sp" nl()
    ".in 16n" nl()
    ".ti 0" nl()
    "\h'5n'\h'-\w'\\$1'u'\\$1\h'8n'\h'-\w'\\$2'u'\\$2\h'2n'" nl()
    ".." nl()
    ".de IN" nl()
    ".sp" nl()
    ".in 0" nl()
    ".." nl()
    ".de GN" nl()
    ".br" nl()
    ".ne 2i" nl()
    ".sp 2" nl()
    ".in 0" nl()
    ".ce" nl()
    ".." nl()
    ".de P" nl()
    ".sp" nl()
    ".in 0" nl()
    ".ti 5" nl()
    ".." nl()
    ".ev 1" nl()
    ".po 9" nl()
    ".ll 8i" nl()
    ".ev" nl()
    ".ls 1" nl()
    ".na" nl()
    list(ilist) list(glist)
    table(stab) indiset(idex)
    enqueue(ilist,indi)  enqueue(glist,1)
    set(curgen,0)  set(out,1)  set(in,2)
    while (indi,dequeue(ilist)) {
        print("OUT: ") print(d(out))
        print(" ") print(name(indi)) print(nl())
        set(thisgen,dequeue(glist))
        if (ne(curgen,thisgen)) {
            ".GN" nl() "GENERATION " d(thisgen) nl() nl()
            set(curgen,thisgen)
        }
        ".IN" nl() d(out) ". "
        insert(stab,save(key(indi)),out)
        call longvitals(indi)
        addtoset(idex,indi,0)
        set(out,add(out,1))
        families(indi,fam,spouse,nfam) {
            ".P" nl()
            if (spouse) { set(sname, save(name(spouse))) }
            else        { set(sname, "_____") }
            if (eq(0,nchildren(fam))) {
                name(indi) " and " sname
                " had no children." nl()
            } elsif (and(spouse,lookup(stab,key(spouse)))) {
                "Children of " name(indi) " and " sname " are shown "
                "under " sname " (" d(lookup(stab,key(spouse))) ")." nl()
            } else {
                "Children of " name(indi) " and " sname":" nl()
                children(fam,child,nchl) {
                    set(haschild,0)
                    families(child,cfam,cspou,ncf) {
                        if (ne(0,nchildren(cfam))) { set(haschild,1) }
                   }
                   if (haschild) {
                        print("IN:  ") print(d(in))
                        print(" ") print(name(child)) print(nl())
                        enqueue(ilist,child)
                        enqueue(glist,add(1,curgen))
                        ".CH " d(in) " " roman(nchl) nl()
                        set (in, add (in, 1))
                        call shortvitals(child)
                    } else {
                        ".CH " qt() qt() " " roman(nchl) nl()
                        call longvitals(child)
                        addtoset(idex,child,0)
                    }
                }
            }
        }
    }
    print("begin sorting") print(nl())
    namesort(idex)
    print("done sorting") print(nl())
    ".bp" nl()
    ".in 0" nl()
    "INDEX OF ALL PERSONS IN THIS REPORT" nl() nl()
    forindiset(idex,indi,v,n) {
        ".br" nl()
        fullname(indi,1,0,30) 
        col(40) stddate(birth(indi)) 
        col(55) stddate(death(indi)) nl()
        print(".")
    }
    nl()
    print(nl())
}
proc shortvitals(indi)
{
        name(indi)
        set(b,birth(indi)) set(d,death(indi))
        if (and(b,short(b))) { ", b. " short(b) }
        if (and(d,short(d))) { ", d. " short(d) } nl()
}

proc longvitals(i)
{
        if (bold) { "\fB" }
        name(i)
        if (bold) { "\fP" }
        "." nl()
        set(e,birth(i))
        if(and(e,long(e))) { "Born " long(e) "." nl() }
        if (eq(1,nspouses(i))) {
                spouses(i,s,f,n) {
                        if (marriage(f)) {
                                "Married"
                        } else {
                                /* "Lived with " */
                                "Married" 
                        }
                        set(nocomma,1)
                        call spousevitals(s,f)
                }
        } else {
                set(j,1)
                spouses(i,s,f,n) {
                        if (marriage(f)) {
                                "Married " ord(j) ","
                                set(j,add(j,1))
                        } else {
                                "Married "
                        }
                        call spousevitals(s,f)
                }
        }
        set(e,death(i))
        if(and(e,long(e))) { "Died " long(e) "." nl() }

        fornodes(inode(i), node) {
                if (eq(0,strcmp("FILE", tag(node)))) {
                        copyfile(value(node))
                } elsif (eq(0,strcmp("NOTE", tag(node)))) {
                        value(node) nl()
                        fornodes(node, subnode) {
                                if (eq(0,strcmp("CONT", tag(subnode)))) {
                                        value(subnode) nl()
                                }
                        }
                }
        }
}

proc spousevitals (sp,fam)
{
        addtoset(idex,sp,0)
        set(e,marriage(fam))
        if (and(e,long(e))) { nl() long(e) "," }
        nl() name(sp)
        set(e,birth(sp))
        if(and(e,long(e)))  { "," nl() "born " long(e) }
        set(e,death(sp))
        if(and(e,long(e)))  { "," nl() "died " long(e) }
        set(dad,father(sp))
        set(mom,mother(sp))
        if (or(dad,mom)) {
                "," nl()
                if (male(sp))      { "son of " }
                elsif (female(sp)) { "daughter of " }
                else               { "child of " }
        }
        if (dad)          { name(dad) }
        if (and(dad,mom)) { nl() "and " }
        if (mom)          { name(mom) }
        "." nl()
        if (dad) { addtoset(idex,dad,0) }
        if (mom) { addtoset(idex,mom,0) }
        addtoset(idex,sp,0)
}

/*   Sample output of this report, it is paginated but I have not shown
     that in this example.


         Manes - Manis - Maness  Family  History
                                                              27 Sep 1992

                                   GENERATION 1

         1. Fuller Ruben MANES.  Born 19 Nov 1902, Union Valley, Sevier
         Co, TN.  Married 17 OCT 1936, Knoxville, TN, Edith Alberta MANIS,
         born 8 APR 1914, Dandridge, Jefferson Co, TN, died 18 JUN 1992,
         Knoxville, Knox Co, TN, daughter of William Loyd MANIS and Lillie
         Caroline "Carolyn" NEWMAN.  Died 20 Jun 1980, Knoxville, Knox Co,
         TN.  Fuller's first fifteen years were growing up on a farm.  By
         the time he was 10 years old, he had 9 other brothers and sisters
         to help feed and care for, play with, and the many facets of work
         which had to be done each day.  "Clifford" and "Snowball" were
         some of his nicknames. Pictures show him (many times) in a
         three-piece suit and a man of many places.  As most men, during
         his youth, he was photographed in the presence with several
         different females.  He attended school at Harrison Chilhowee
         Baptist Academy, which a walk of about 5 or 6 miles each way from
         his home.  He boarded at the school dormitory for an unknown
         period of time.

              Children of Fuller Ruben MANES and Edith Alberta MANIS:

                     i   Ellsworth Howard MANIS.  Born 11 MAR 1939,
                         Knoxville, Knox Co, TN.  Died 13 MAR 1939,
                         Knoxville, TN,.  Was the first born of twins,
                         birth two-forty PM, at Harrison-Henderson
                         Hospital.  Ellsworth died at age 44 hours, was a
                         twin to Alda Clifford MANIS.  Buried 13 Mar 1939
                         at Seven Islands Cem, NE Knox County, TN (near
                         Jefferson and Sevier County line).

             2      ii   Alda Clifford MANIS, b. 1939, TN


                                   GENERATION 2


         2. Alda Clifford MANIS.  Born 11 MAR 1939, Knoxville, Knox Co,
         TN.  Married first, 8 SEP 1962, Knoxville, Knox Co, TN, Joyce
         Fern OWENS, born 1 APR 1942, Knoxville, Knox Co, TN, daughter of
         Guy Hixon OWENS and Bertha Mae TURNER.  Married second, 13 FEB
         1984, San Antonio, Texas, Marianne Florence KRAMER, born 19 MAY
         1943, Los Angeles, CA, daughter of Anthony Leo KRAMER and
         Florence Rita BOSSO.  Born at two-forty five PM, Harrison-
         Henderson Hospital.  Twin of Elsworth Howard MANIS.  Clifford was
         born second.

              Children of Alda Clifford MANIS and Joyce Fern OWENS:

             3       i   Gregory Scott MANIS, b. 1963, VA

                    ii   Sheila Ann MANIS.  Born 7 APR 1968, Mexico City,
                         Mexico DF.

              Alda Clifford MANIS and Marianne Florence KRAMER had no
         children.


                                   GENERATION 3


         3. Gregory Scott MANIS.  Born 15 Sep 1963, Warrenton, Fauquier
         Co, VA.  Married San Antonio, Tx, Vicky Lynn LAMB BLOOMER, born
         18 JAN 1963, Henderson, KY, daughter of Richard Graham LAMB and
         Annette M. ST. PIERRE.  Birthday: Sunday Scott started his
         international traveling at the early age of 9 weeks, then he flew
         with his parents to Teheran, Iran.  2 days after arriving there,
         President John F. Kennedy, was killed in Dallas, Texas.

         Attended school in Mexico City, Mexico, where he started the
         first grade.  Later getting the rest of his early education at
         public schools in the following cities:  Vienna, Austria and
         Ankara, Turkey, and attending school in several states in the US
         before finally graduating high school at Holmes in San Antonio,
         Texas.

         An important part of his teenage years included being in the Boy
         Scouts of America, where he achieved the honor of being an Eagle
         Scout through his hard work.

         He then graduated the University of Texas at San Antonio, TX, B.
         S. Biology, August 1985.

         In 1992, a Research Specialist and Lab Manager, Immunogenetics
         Laboratory, Trinity University, San Antonio, May 1989-present.


              Children of Gregory Scott MANIS and Vicky Lynn LAMB BLOOMER:

                     i   Kayla Marie MANIS.  Born 23 NOV 1988, San
                         Antonio, Tx.

                    ii   Gregory Paul MANIS.  Born 16 JUN 1990, San
                         Antonio, Tx.

         Cliff Manis, P. O. Box 33937, San Antonio, TX 78265, 1-(512)-654-9912

         INDEX OF ALL PERSONS IN THIS REPORT

         BLOOMER, Vicky Lynn LAMB               18 Jan 1963
         BOSSO, Florence Rita                   16 Dec 1916
         KRAMER, Anthony Leo                     2 Apr 1913    28 May 1981
         KRAMER, Marianne Florence              19 May 1943
         LAMB, Richard Graham
         MANES, Fuller Ruben                    19 Nov 1902    20 Jun 1980
         MANIS, Alda Clifford                   11 Mar 1939
         MANIS, Edith Alberta                    8 Apr 1914    18 Jun 1992
         MANIS, Ellsworth Howard                11 Mar 1939    13 Mar 1939
         MANIS, Gregory Paul                    16 Jun 1990
         MANIS, Gregory Scott                   15 Sep 1963
         MANIS, Kayla Marie                     23 Nov 1988
         MANIS, Sheila Ann                       7 Apr 1968
         MANIS, William Loyd                     5 Sep 1872    15 Mar 1946
         NEWMAN, Lillie Caroline                13 Jun 1881    29 Sep 1949
         OWENS, Guy Hixon                       22 Jul 1908
         OWENS, Joyce Fern                       1 Apr 1942
         ST. PIERRE, Annette M.                 23 Jan 1944
         TURNER, Bertha Mae                     19 Jul 1914

         Cliff Manis, P. O. Box 33937, San Antonio, TX 78265, 1-(512)-654-9912

*/

/* end of report */

