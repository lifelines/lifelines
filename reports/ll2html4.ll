/*
 * @progname       ll2html4.ll
 * @version        1995-03-06
 * @author         JRE
 * @category       
 * @output         HTML
 * @description
 *
 *  This report program converts a LifeLines database into html documents.
 *  Family group records are created for each selected individual in
 *  the database.  These records are written in files containing clumps
 *  of individuals of a user-selected size.  Index files are generated
 *  for an index document.  Or, optionally, all output is sent to
 *  one file.
 *
 *  You will need to change the contents of proc html_address() and to
 *  set the value of HREF appropriately to your server.
 *  You need to set the value of PATH to point to the directory to put
 *  the files into. If you have 1000 individuals in your database this
 *  program will create up to 1027 files, one for each individual and
 *  up to 27 index files.
 *
 *  You also need to set the value of HOST to be the http server and
 *  path where you will server these files from.
 *
 *  History
 *   01-07-94  sew; Created.
 *   11-18-94  jre; Added clump capability.
 *   02-16-95  jre; Added privacy option.
 *   03-06-95  jre; Added pedigree table, better sorting.
 *
 */

global(INDEX)
global(INDEXTABLE)
global(HREF)
global(PATH)
global(PEDIGREE_NAME)
global(INDEX_NAME)
global(TITLE)
global(ADDRESS)
global(FB)
global(nl)
global(CURRENTCLUMPFILE)
global(root_person)
global(separate_clumps)
global(PRIVTABLE)
global(sort_xlat)
global(html_xlat)

/* These constants are for estimating birth years */
global(years_between_kids)
global(mother_age)
global(father_age)

proc main()
{

/* Change these to suit your needs */

    set(TITLE,"Eggert Family Genealogy") /* Title of main genealogy page */
    set(PEDIGREE_NAME,"Eggert Family Pedigree") /* Pedigree chart title */
    set(INDEX_NAME,"Eggert Family Genealogy Index ") /* Index title */
    set(ADDRESS,"Jim Eggert, eggertj@ll.mit.edu") /* Your name and address */
    set(PATH, "") /* path for file references */
/*    set(HOST, "http://vulcan.cv.com") */ /* host name of server, if other */
/*    set(HREF, save(concat(HOST, PATH)))  */
    set(HREF, "") /* host and path */

    set(FB, 0)
    set(nl, nl())
    list(INDEX)
    table(INDEXTABLE)
    table(PRIVTABLE)
    table(sort_xlat)
    table(html_xlat)
    call init_xlat()
    call init_years()

    indiset(people)
    getindimsg(root_person,"Enter root individual:")
    set(root_key,save(key(root_person)))
    set(clumpsize,0)
    while (le(clumpsize,0))
    {
        getintmsg(clumpsize,"Enter number of individuals per file:")
    }
/*    getintmsg(separate_clumps,
 *            "Do you want clumps in separate files (0=no,1=yes)?")
 */
    set(separate_clumps,1)
    getintmsg(privacy,
              "Delete notes and dates for living people (0=no, 1=yes)?")
    list(nonprivates)
    if (privacy) {
        set(person,1)
        while(person) {
            getindimsg(person,"Enter non-private person:")
            if (person) { enqueue(nonprivates,save(key(person))) }
        }
    }

    print("Finding ancestry... ")
    addtoset(people, root_person, 0)
    set(people,union(people,ancestorset(people),descendantset(people)))
    set(people,union(people,spouseset(people)))
    set(people,union(people,childset(people)))

    set(indicount,0)
    set(clumpcount,1)

    print("done\nCollating index... 1")
    forindiset(people,me,val,num)
    {
              /* print(".") */
        incr(indicount)
        if (ge(indicount,clumpsize))
        {
            incr(clumpcount)
            set(indicount,0)
            print(" ", d(clumpcount))
        }
        set(k,save(key(me)))
        enqueue(INDEX,k)
        insert(INDEXTABLE,k,clumpcount)
        if (privacy) { insert(PRIVTABLE,k,privacy(me)) }
        else { insert(PRIVTABLE,k,0) }
    }

    if (privacy) {
        while (pkey,dequeue(nonprivates)) {
            insert(PRIVTABLE,pkey,0)
        }
    }

    print(" done\nWriting index(slow)...")
    call create_index_file()

    print(" done\nWriting name files...")
    call start_clumpfile(1)
    forindiset(people, me, val, num)
    {
        call write_indi(me)
    }
    call end_clumpfile()
    print(" done\nWriting pedigree chart...")
    call pedigree_chart(indi(root_key))
    print(" done\n")
}

proc pedigree_chart(person) {
    if (separate_clumps) { newfile("pedigree.html", FB) }
    call html_header(PEDIGREE_NAME, 0)
    "Sort by <A HREF=" qt() "pedigreen.html" qt() ">number</A>.<p>\n<PRE>\n"
    call pedigree(0, 1, person)
    "</PRE>\n"
    call html_trailer()
    if (separate_clumps) { newfile("pedigreen.html", FB) }
    call html_header(PEDIGREE_NAME, 0)
    "Sort by <A HREF=" qt() "pedigree.html" qt() ">lineage</A>.<p>\n<PRE>\n"
    call ahnen(person)
    "</PRE>\n"
    call html_trailer()
}

proc pedigree(in, ah, indi) {
    if (par, father(indi)) { call pedigree(add(1,in), mul(2,ah), par) }
    set(spacecount,add(1,sub(mul(in,2),strlen(d(ah)))))
    while(spacecount) { " " decr(spacecount) }
    d(ah) " " call href(indi,neg(1)) nl
    if (par, mother(indi)) { call pedigree(add(1,in), add(1,mul(2,ah)), par) }
}

proc ahnen(person) {
    list(plist)
    list(nlist)
    enqueue(plist,person)
    enqueue(nlist,1)
    while(p,dequeue(plist)) {
        set(n,dequeue(nlist))
        d(n) " " call href(p,neg(1)) nl
        if (f,father(p)) {
            enqueue(plist,f)
            enqueue(nlist,mul(2,n))
        }
        if (m,mother(p)) {
            enqueue(plist,m)
            enqueue(nlist,add(1,mul(2,n)))
        }
    }
}

func privacy(person) {
    if (living(person)) { return(1) }
    set(sib,person)
    while (sib,prevsib(sib)) { if (living(sib)) { return(1) } }
    set(sib,person)
    while (sib,nextsib(sib)) { if (living(sib)) { return(1) } }
    if (f,father(person)) { if (living(f)) { return(1) } }
    if (m,mother(person)) { if (living(m)) { return(1) } }
    return(0)
}

func living(person) {
    if (death(person)) { return(0) }
    if (burial(person)) { return(0) }
    if (b,birth(person)) {
        extractdate(b, da, mo, yr)
        if (gt(yr,1900)) { return(1) }
    }
    if (b,baptism(person)) {
        extractdate(b, da, mo, yr)
        if (gt(yr,1900)) { return(1) }
    }
    families(person,spouse,fam,nfam) {
        if (m,marriage(fam)) {
            extractdate(m, day, mo, yr)
            if (gt(yr,1920)) { return(1) }
        }
    }
    return(0)
}

proc create_index_file()
{
    list(initials)
    list(initialcounters)
    list(sortindex)

    call quicksort(INDEX,sortindex)

    set(initial,"no-initial")
    set(counter,1)
    forlist(sortindex,sindex,counter)
    {
        set(me,indi(getel(INDEX,sindex)))
        set(myinitial,trim(strxlat(sort_xlat,trim(surname(me),1)),1))
        if (strcmp(myinitial,initial))
        {
            if (strcmp(initial,"no-initial"))
            {
                enqueue(initials, initial)
                enqueue(initialcounters, initialcounter)
                set(initial, save(myinitial))
                "</UL>" nl
                call html_trailer()
            }
            else
            {
                set(initial, save(myinitial))
            }
            set(initialcounter,0)
            print(" ", initial)
            set(fn, save(concat(PATH, "index", initial, ".html")))
            if (separate_clumps) { newfile(fn, FB) }
            call html_header(
                concat(INDEX_NAME,initial)
                , 0)
            "<BODY>" nl
            "<H1> INDEX " initial " </H1>" nl
            "<UL>" nl
        }
        "<LI>" call href(me,neg(1)) nl
        incr(initialcounter)
    }
    "</UL>" nl
    call html_trailer()
    enqueue(initials, initial)
    enqueue(initialcounters, initialcounter)

    set(fn, save(concat(PATH, "master_index", ".html")))
    if (separate_clumps) { newfile(fn, FB) }
    forlist(sortindex,sindex,counter) {
        set(me,indi(getel(INDEX,sindex)))
        "<LI>" call href(me,neg(1)) nl
    }

    set(fn, save(concat(PATH, "index.html")))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(INDEX_NAME, 0)
    "<BODY>" nl
    "<H1> INDEX </H1>" nl
    "<UL>" nl

    "<A HREF=" qt() "pedigree.html" qt() "> Pedigree chart</A>" nl

    "<L1>" call href(root_person,neg(1)) nl

    while (initial,dequeue(initials))
    {
        "<LI><A HREF=" qt() "index" initial ".html" qt() "> "
        initial " (" d(dequeue(initialcounters)) " entries)</A>" nl
    }
    "</UL>" nl
    "There are in total " d(length(INDEX))
    " people in this database, last updated "
    dayformat(2) monthformat(6) dateformat(0)
    stddate(gettoday()) nl
    call html_trailer()
}

proc start_clumpfile(clumpnum)
{
    print(" ", d(clumpnum))
    set(CURRENTCLUMPFILE, clumpnum)
    set(fn, save(concat(PATH, "clump", d(CURRENTCLUMPFILE), ".html")))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(TITLE, 0)
    "<BODY>" nl
}

proc end_clumpfile()
{
    "<A HREF=" qt() HREF "index.html" qt() "> [Index To Database] </A>"
    call html_trailer()
}

proc write_indi(me)
{
    set(private,lookup(PRIVTABLE,key(me)))
    set(myclump,lookup(INDEXTABLE,key(me)))
    if (ne(myclump,CURRENTCLUMPFILE))
    {
        call end_clumpfile()
        call start_clumpfile(myclump)
    }
    "<A NAME = " qt() key(me) qt() ">"
    "<H1>" call print_name(me, 1) "</H1>" "</A>" nl
    "<PRE>"
    nl
    if(e, birth(me))   { "Birth:     " privlong(e,private) nl }
    if(e, baptism(me)) { "Baptism:   " privlong(e,private) nl }
    if(e, death(me))   { "Death:     " privlong(e,private) } nl
    if(e, burial(me))  { "Burial:    " privlong(e,private) } nl
    nl
    if (f,father(me)) { "Father:    " call href(f,myclump) nl }
    if (m,mother(me)) { "Mother:    " call href(m,myclump) nl }
    set(nfam,nfamilies(me))
    families(me, fam, sp, nsp)
    {
        nl
        "Married"
        if (gt(nfam,1)) { "(" d(nsp) ") " } else { "    " }
        call href(sp,myclump)
        if(e, marriage(fam)) { "\n           " privlong(e,private) }
        fornodes(fnode(fam),thisnode) {
            if (not(strcmp(tag(thisnode),"DIV")))
            {
                if (not(private)) { ", Divorced" }
            }
        }
        nl
        if(nchildren(fam))
        {
            "Children:" nl
            children(fam, ch, nch)
            {
                rjt(nch, 5) ". "
                call href(ch,myclump) nl
            }
        }
    }
    nl
    "<P>" nl
    if (not(private)) { call print_notes(me) }
    "</PRE><HR>" nl
}

func privlong(event,private) {
    if (private) { strxlat(html_xlat,place(event)) }
    else { strxlat(html_xlat,long(event)) }
}

proc print_notes(me)
{
    set(first, 1)
    fornodes( inode(me), node)
    {
        if (not(strcmp("NOTE", tag(node))))
        {
            if(first) { "<B>Notes: </B>" nl nl set(first, 0) }
            strxlat(html_xlat,value(node)) nl
            fornodes(node, next)
            {
                strxlat(html_xlat,value(next)) nl
            }
            nl
        }
    }
    fornodes( inode(me), node)
    {
        if (not(strcmp("REFN", tag(node))))
        {
            if(first) { "<B>Notes: </B>" nl nl set(first, 0) }
            "SOURCE: " strxlat(html_xlat,value(node)) nl
            nl
        }
    }
}

proc href(me,fromclump)
{
    if(me)
    {
        set(myclump,lookup(INDEXTABLE,key(me)))
        if (myclump)
        {
            if (eq(fromclump,myclump))
            {
                "<A HREF=" qt() "#" key(me) qt() ">"
            }
            else
            {
                "<A HREF=" qt() "clump" d(myclump) ".html#" key(me) qt() ">"
            }
            set(private,lookup(PRIVTABLE,key(me)))
        }
        else
        {
            set(private,privacy(me))
        }
        call print_name(me, 1)
        if (myclump) { "</A>" }
        " ("
        if (print_year_place(birth(me),baptism(me),"*",private)) {
            set(j,print_year_place(death(me),burial(me)," +",private))
        } else {
            set(j,print_year_place(death(me),burial(me),"+",private))
        }
        ")"
    }
/*    else { "_____" } */
}

func print_year_place(event,secondevent,symbol,private)
{
    set(noyear,1)
    set(noplace,1)
    if (not(private)) {
        if (event) {
            set(d, date(event))
            set(y, year(event))
            if (strlen(y)) {
                symbol call print_fix_year(d,y) set(noyear,0)
            }
        }
        if (noyear) {
            if (secondevent) {
                set(d, date(secondevent))
                set(y, year(secondevent))
                if (strlen(y)) {
                    symbol call print_fix_year(d,y) set(noyear,0)
                }
            }
        }
    }
    if (noyear) { set(space,symbol) } else { set(space," ") }
    if (event) {
        set(p, place(event))
        if (strlen(p)) { space strxlat(html_xlat,p) set(noplace,0) }
    }
    if (noplace) {
        if (secondevent) {
            set(p, place(secondevent))
            if (strlen(p)) { space strxlat(html_xlat,p) set(noplace,0) }
        }
    }
    return(not(and(noyear,noplace)))
}

proc print_fix_year(d,y)
{
    if (index(d,"BEF",1)) { "&lt;" }
    if (index(d,"AFT",1)) { "&gt;" }
    if (index(d,"ABT",1)) { "c" }
    y
/* Handle PAF slash years */
    set(yp,index(d,y,1))
    set(d2,substring(d,add(yp,4),strlen(d)))
    if (d2) {
        if (eq(index(d2,"/",1),1)) {
            substring(d2,1,5)
        }
    }
}

proc html_header(str, isindex)
{
    "<HTML>" nl
    "<HEAD>" nl
    if(isindex) { "<ISINDEX>" nl }
    "<TITLE> " str " </TITLE>" nl
    "</HEAD>" nl
 }

proc html_trailer()
{
    "<HR>" nl
    "<ADDRESS> " ADDRESS " </ADDRESS>" nl
    "</BODY>" nl
    "</HTML>" nl
}

proc print_name (me, last)
{
    strxlat(html_xlat,fullname(me, 0, not(last), 45))
    fornodes(inode(me), node)
    {
        if (not(strcmp("TITL", tag(node)))) { set(n, node) }
    }
    if (n) { " " strxlat(html_xlat,value(n)) }
}

func rjt(n, w)
{
    set(d, strlen(d(n)))
    if (lt(d, w))
        { set(pad, trim("      ", sub(w, d))) }
    else
        { set(pad, "") }
    return(save( concat(pad, d(n))))
}

/*
   quicksort: Sort an input list by generating a permuted index list
   Input:  alist  - list to be sorted
   Output: ilist  - list of index pointers into "alist" in sorted order
   Needed: compare- external function of two arguments to return -1,0,+1
                    according to relative order of the two arguments
*/
proc quicksort(alist,ilist) {
    set(len,length(alist))
    set(index,len)
    while(index) {
        setel(ilist,index,index)
        decr(index)
    }
    call qsort(alist,ilist,1,len)
}

/* recursive core of quicksort */
proc qsort(alist,ilist,left,right) {
    if(pcur,getpivot(alist,ilist,left,right)) {
        set(pivot,getel(alist,getel(ilist,pcur)))
        set(mid,partition(alist,ilist,left,right,pivot))
        call qsort(alist,ilist,left,sub(mid,1))
        call qsort(alist,ilist,mid,right)
    }
}

/* partition around pivot */
func partition(alist,ilist,left,right,pivot) {
    while(1) {
        set(tmp,getel(ilist,left))
        setel(ilist,left,getel(ilist,right))
        setel(ilist,right,tmp)
        while(lt(compare(getel(alist,getel(ilist,left)),pivot),0)) {
            incr(left)
        }
        while(ge(compare(getel(alist,getel(ilist,right)),pivot),0)) {
            decr(right)
        }
        if(gt(left,right)) { break() }
    }
    return(left)
}

/* choose pivot */
func getpivot(alist,ilist,left,right) {
    set(pivot,getel(alist,getel(ilist,left)))
    set(left0,left)
    incr(left)
    while(le(left,right)) {
        set(rel,compare(getel(alist,getel(ilist,left)),pivot))
        if (gt(rel,0)) { return(left) }
        if (lt(rel,0)) { return(left0) }
        incr(left)
    }
    return(0)
}

/* compare indis referred to by keys */
func compare(pkey1,pkey2) {
    if(not(strcmp(pkey1,pkey2))) { return(0) }
    set(indi1,indi(pkey1))
    set(surname1,save(strxlat(sort_xlat,surname(indi1))))
    set(indi2,indi(pkey2))
    if (s,strcmp(surname1,strxlat(sort_xlat,surname(indi2)))) { return(s) }
    set(givens1,save(strxlat(sort_xlat,givens(indi1))))
    if (s,strcmp(givens1,strxlat(sort_xlat,givens(indi2)))) { return(s) }
    return(intcompare(estimate_byear(indi1),estimate_byear(indi2)))
}

/* translate string according to xlat table */
func strxlat(xlat,string) {
    set(fixstring,"")
    set(pos,1)
    while(le(pos,strlen(string))) {
        set(char,substring(string,pos,pos))
        if (special,lookup(xlat,char)) {
            set(fixstring,concat(fixstring,special))
        }
        else { set(fixstring,concat(fixstring,char)) }
        incr(pos)
    }
    return(save(fixstring))
}

proc init_xlat() {
    insert(sort_xlat,"š","oe")
    insert(sort_xlat,"Ÿ","ue")
    insert(sort_xlat,"Š","ae")
    insert(sort_xlat,"§","ss")
    insert(sort_xlat,"€","Ae")
    insert(sort_xlat,"…","Oe")
    insert(sort_xlat,"†","Ue")
    insert(sort_xlat,"‘","e")
    insert(sort_xlat,"Ø","y")

    insert(html_xlat,"š","&ouml;")
    insert(html_xlat,"Ÿ","&uuml;")
    insert(html_xlat,"Š","&auml;")
    insert(html_xlat,"§","&szlig;")
    insert(html_xlat,"€","&Auml;")
    insert(html_xlat,"…","&Ouml;")
    insert(html_xlat,"†","&Uuml;")
    insert(html_xlat,"‘","&euml;")
    insert(html_xlat,"Ø","&yuml;")
}

func intcompare(i1,i2) {
    if(lt(i1,i2)) { return(neg(1)) }
    if(eq(i1,i2)) { return(0) }
    return(1)
}

proc init_years() {
    set(years_between_kids,2)
    set(mother_age,23)
    set(father_age,25)
}

func estimate_byear(person) {
    set(byear_est,0)
    if(byear,get_byear(person)) { return(byear) }
    set(older,person)
    set(younger,person)
    set(yeardiff,0)
    set(border,0)
    while (or(older,younger)) {
        set(older,prevsib(older))
        set(younger,nextsib(younger))
        set(yeardiff,add(yeardiff,years_between_kids))
        if (older) {
            incr(border)
            if (byear,get_byear(older)) {
                return(add(byear,yeardiff))
            }
        }
        if (younger)  {
            if(byear,get_byear(younger)) {
                return(sub(byear,yeardiff))
            }
        }
    }
/* estimate from parents' marriage */
    set(my,0)
    if (m,marriage(parents(person))) { extractdate(m,bd,bm,my) }
    if (my) {
        return(add(add(my,mul(years_between_kids,border)),1))
    }
/* estimate from first marriage */
    families(person,fam,spouse,fnum) {
        if (gt(fnum,1)) { break() }
        if (m,marriage(fam)) { extractdate(m,bd,bm,my) }
        if (my) {
            if (female(person)) { return(sub(my,mother_age)) }
            else { return(sub(my,father_age)) }
        }
        children(fam,child,cnum) {
            if (byear,get_byear(child)) {
                if (female(person)) {
                    return(sub(sub(byear,
                                mul(sub(cnum,1),years_between_kids)),
                                        mother_age))
                }
                else {
                    return(sub(sub(byear,
                                mul(sub(cnum,1),years_between_kids)),
                                father_age))
                }
            }
        }
    }
/* estimate from parents' birthyear */
    set(older,person) set(byear_addend,0)
    while(older,prevsib(older)) {
        set(byear_addend,add(byear_addend,years_between_kids))
    }
    if (byear,get_byear(mother(person))) {
        return(add(byear,mother_age,byear_addend))
    }
    if (byear,get_byear(father(person))) {
        return(add(byear,father_age,byear_addend))
    }
    return(0)
}

func get_byear(person) {
    set(byear,0)
    if (person) {
        if (b,birth(person)) { extractdate(b,day,month,byear) }
        if (byear) { return(byear) }
        if (b,baptism(person)) { extractdate(b,day,month,byear) }
    }
    return(byear)
}
