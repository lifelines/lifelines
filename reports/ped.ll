/*
 * @progname       ped.ll
 * @version        1996-08-09
 * @author         Bill.Alford@anu.edu.au
 * @category       
 * @output         HTML
 * @description

   html pedigree/ancestral chart part of an individuals report.
   This coding can be generalised to print many generations back but
   for html purposes we only need go back 2 generations because the
   width of the page becomes far too big otherwise. In this case I've
   hard coded out what the generalised coding would look like. I've
   used ideas from the output of the gedcom2html program.

   Typical output from this reports looks like:

<pre>
                     <a href="genweb_9.html">_William Alford___</a>
 <a href="genweb_1.html">_William Alford____</a>|
|                   |<a href="genweb_10.html">_Elizabeth Shore__</a>
|
|--<a href="genweb_5.html">William Alford</a>
|
|                    <a href="genweb_186.html">_Robert Goldsbrough__</a>
|<a href="genweb_2.html">_Jane Goldsbrough__</a>|
                    |<a href="genweb_187.html">_Hannah Browne_______</a>
</pre>
<p><hr>

   Bill.Alford@anu.edu.au 9 Aug 1996 */

global(line1)           /* position on line 1 */
global(line2)           /* position on line 2 */
global(line3)           /* position on line 3 */
global(line5)           /* position on line 5 */
global(line6)           /* position on line 6 */
global(line7)           /* position on line 7 */
global(nstrngff)        /* father's fathers name string */
global(nstrngf)         /* father's name string */
global(nstrngfm)        /* father's mothers name string */
global(nstrngi)         /* individual's name string */
global(nstrngmf)        /* mother's father name string */
global(nstrngm)         /* mother's name string */
global(nstrngmm)        /* mother's mother name string */

proc ped(indi, href_table)
{
        set(nstrngff,save(name(father(father(indi)),0)))
        set(nstrngf,save(name(father(indi),0)))
        set(nstrngfm,save(name(mother(father(indi)),0)))
        set(nstrngi,save(name(indi,0)))
        set(nstrngmf,save(name(father(mother(indi)),0)))
        set(nstrngm,save(name(mother(indi),0)))
        set(nstrngmm,save(name(mother(mother(indi)),0)))
        set(line1,strlen(nstrngff))
        set(line2,strlen(nstrngf))
        set(line3,strlen(nstrngfm))
        set(line5,strlen(nstrngmf))
        set(line6,strlen(nstrngm))
        set(line7,strlen(nstrngmm))
        set(dif1,0)
        set(dif2,0)
        set(dif3,0)
        set(dif5,0)
        set(dif6,0)
        set(dif7,0)
        if (ne(line2,line6)) {
           if (gt(line2,line6)) { set(dif6,sub(line2,line6)) }
           else { set(dif2,sub(line6,line2)) }
        }
        if (ne(line1,line3)) {
           if (gt(line1,line3)) { set(dif3,sub(line1,line3)) }
           else { set(dif1,sub(line3,line1)) }
        }
        if (ne(line5,line7)) {
           if (gt(line5,line7)) { set(dif7,sub(line5,line7)) }
           else { set(dif5,sub(line7,line5)) }
        }
        set(diff1,dif1)
        set(diff2,dif2)
        set(diff3,dif3)
        set(diff5,dif5)
        set(diff6,dif6)
        set(diff7,dif7)

/* Output the html */

        "<pre>\n"
        col(add(line2,6,dif2)) call ped_ahref(father(father(indi)),href_table)
        "_" nstrngff "__"
        while (gt(diff1,0)) { "_" decr(diff1) }
        "</a>" nl()
        " " call ped_ahref(father(indi),href_table) "_" nstrngf "__"
        while (gt(diff2,0)) { "_" decr(diff2) }
        "</a>|" nl()
        "|" col(add(line2,5,dif2)) "|"
        call ped_ahref(mother(father(indi)),href_table) "_" nstrngfm "__"
        while (gt(diff3,0)) { "_" decr(diff3) }
        "</a>" nl()
        "|" nl()
        "|--" call ped_ahref(indi,href_table) nstrngi "</a>" nl()
        "|" nl()
        "|" col(add(line6,6,dif6))
        call ped_ahref(father(mother(indi)),href_table) "_" nstrngmf "__"
        while (gt(diff5,0)) { "_" decr(diff5) }
        "</a>" nl()
        "|" call ped_ahref(mother(indi),href_table) "_" nstrngm "__"
        while (gt(diff6,0)) { "_" decr(diff6) }
        "</a>|" nl()
        col(add(line6,5,dif6)) "|"
        call ped_ahref(mother(mother(indi)),href_table) "_" nstrngmm "__"
        while (gt(diff7,0)) { "_" decr(diff7) }
        "</a>" nl()
        "</pre>\n"
        "<p><hr>\n"
}

proc ped_ahref(indi,href_table)
{
        "<a href=" qt()
        get_href(indi, href_table)
        qt() ">"
}
