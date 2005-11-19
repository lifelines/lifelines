/*
 * @progname       net-ped.ll
 * @version        2.1
 * @author         Rafal T. Prinke
 * @category       
 * @output         Netscape HTML
 * @description

    This program generates a set of files covering all known ancestors
    with hypertextual links from the top and bottom persons for
    easy on-line browsing using Netscape.

    The amount and format of data can be edited in the "vitals" procedure
    and "mylong" function.

    Net-Ped -- Pedigree in Netscape HTML table -- bottom to top
    Rafal T. Prinke
    v.1 -- 25 JUN 1996 - one file/tafel
    v.2 -- 25 JUN 1996 (later at night than v.1)
           - multiple files/tafels for all ancestors
    v.2.1 -- 28 JUN 1996
           - changed table holding last filename to list
           - no files for top persons without a parent
           - cross-links for related ancestors
           - nicer layout
    v.2.2  - misc cleanup - tighten checking for non-existent persons
           - make html slightly more readable -- by Stephen Dum.

    "proc vitals" code stolen from Tom Wetmore's "ahnentafel" 1995
    output uses some Polish specific abbreviations
*/

global(mindi)
global(toplist)
global(fillist)
global(t)
global(a)
global(lastfile)
global(onelist)
global(all)
global(mdupli)
global(fdupli)

proc main()
{
list(toplist)
list(fillist)
list(onelist)
table(all)
table(mdupli)
table(fdupli)

getindimsg(mindi, "Whose Ahnentafel do you want?")
enqueue(toplist, mindi)
set(t,1)
set(t2,concat("t",d(t),".html"))
enqueue(fillist, t2)
set(t,add(t,1))

while(indi, dequeue(toplist)) {
        set(nf,dequeue(fillist))
        print("file: ") print(nf) print("\n")
        newfile(nf,0)
        call tafel(indi)
        }
}

proc tafel(indi)
{
table(quart)
insert(quart,"1",indi)
set(a,1)
while(lt(a,16)) {
        if(person,lookup(quart,d(a))) {
	    if (par,father(person)) {
                set(before, lookup(all, key(par)))
                if (before) {
                        insert(fdupli, key(person),1)
                } else {
                        insert(all, key(par), outfile())
                        insert(quart,d(mul(a,2)),par)
                }
	    }
	    if (par,mother(person)) {
                set(before, lookup(all, key(par)))
                if (before) {
                        insert(mdupli, key(person),1)
                } else {
                        insert(all, key(par), outfile())
                        insert(quart,d(add(1,mul(a,2))),par)
                }
	    }
	}
        set(a,add(a,1))
        }

"<html>\n<body>\n<table border>\n"
set(a,16)
"<tr>"
while(lt(a,32)) {
        if (lookup(quart,d(a))) {
if (or( father(lookup(quart,d(a))), mother(lookup(quart,d(a))) )) {
                set(t2,concat("t",d(t),".html"))
                "<td valign=top>"
                "<sup>" d(a) ".</sup> <p align=center>"
                "<a href=" t2 ">"
                call vitals(lookup(quart,d(a))) nl() "</a></td>\n"
                enqueue(toplist,lookup(quart,d(a)))
                enqueue(onelist,outfile())
                set(t,add(t,1))
                enqueue(fillist, t2)
} else {
                "<td valign=top>"
                "<sup>" d(a) ".</sup> <p align=center>"
                call vitals(lookup(quart,d(a))) nl() "</td>\n"
}



        } else { "<td valign=top><sup>" d(a) ".</sup></td>\n" }
        set(a,add(a,1))
        } "</tr>\n"
set(a,8)
"<tr>"
while(lt(a,16)) {
        "<td valign=top colspan=2>"
        "<sup>" d(a) ".</sup><p align=center>"
        call dup(lookup(quart,d(a)))
        call vitals(lookup(quart,d(a))) nl() "</td>\n"
        set(a,add(a,1))
        } "</tr>\n"
set(a,4)
"<tr>"
while(lt(a,8)) {
        "<td valign=top colspan=4>"
        "<sup>" d(a) ".</sup><p align=center>"
        call dup(lookup(quart,d(a)))
        call vitals(lookup(quart,d(a))) nl() "</td>\n"
        set(a,add(a,1))
        } "</tr>\n"
set(a,2)
"<tr>"
while(lt(a,4)) {
        "<td valign=top colspan=8>"
        "<sup>" d(a) ".</sup><p align=center>"
        call dup(lookup(quart,d(a)))
        call vitals(lookup(quart,d(a))) nl() "</td>\n"
        set(a,add(a,1))
        } "</tr>\n"

if(nestr(key(lookup(quart,d(1))),key(mindi))) {
        "<tr><td valign=top colspan=16>"
        "<sup>" d(1) ".</sup><p align=center>"
        call dup(lookup(quart,d(1)))
        call vitals(lookup(quart,d(1))) nl()
        "<hr><a href=" dequeue(onelist)
        "><font color=FF0000><blink>BACK</blink></font></a></td>\n</tr>\n"
} else {
        "<tr><td valign=top colspan=16>"
        "<sup>" d(1) ".</sup><p align=center>"
        call vitals(lookup(quart,d(1))) nl() "</td>\n</tr>\n"
}


"</table>\n</body>\n</html>\n"

}

proc vitals(persn) {
        set(e,marriage(fam))
        if (and(e,long(e))) { mylong(e) }
        "<strong>" name(persn,0) "</strong><br>\n<i>"

        set(e,birth(persn))
        if(and(e,long(e)))  { "*&nbsp;" mylong(e) "<br>\n" }
        set(e,death(persn))
        if(and(e,long(e)))  { "+&nbsp;" mylong(e) "</i><br>\n" }
     "<small>"
       set(srd,0)
       if (gt(nspouses(persn),1)) {
           spouses(persn,ind2,fm,nsp) {
                        set(dad,father(ind2))
                        set(mom,mother(ind2))
               if (srd) { "; " }
               " x&nbsp;(" d(nsp) ") "
               set (es,marriage(fm))
               if (and(es,long(es))) { mylong(es) " " }
               name(ind2,0)

                   if (or(dad,mom))   {
                      ", "
                      if (male(ind2))      { "s.&nbsp;" }
                      elsif (female(ind2)) { "c.&nbsp;" }
                      else                 { "dz.&nbsp;" }
                   }

                   if (dad) {
                      name(dad,0)
                      fornodes(inode(dad), ok) {
                         if (eqstr(tag(ok),"OCCU")) { ", " value(ok) }
                      }
                   }
                   else     { name(mom,0) }

               set(srd,1)
       }  }
       if (eq(nspouses(persn),1)) {
              if (male(persn)) {
           spouses(persn,ind2,fm,nsp) {
               set (es,marriage(fm))
               if (and(es,long(es))) { "x&nbsp;" mylong(es) }
               } }
        }
"</small>"
}

proc dup(persn) {
if (persn) {
if(lookup(fdupli,key(persn))) {
        set(yest,lookup(all,key(father(persn))))
        "<small><a href=" yest "><font color=FF0000><blink>FATHER:</blink></font><font color=0000FF> " name(father(persn)) "</font></a></small><br>\n"
        }

if(lookup(mdupli,key(persn))) {
        set (yest,lookup(all,key(mother(persn))))
        "<small><a href=" yest "><font color=FF0000><blink>MOTHER:</blink></font><font color=0000FF> " name(mother(persn)) "</font></a></small><br>\n"
        }
}
}

func mylong(ev) {
   set(nic,0)
   if (ne(index(date(ev),"/",1),0)) { date(ev) set(nic,1) }
   if (ne(index(date(ev),"BEF",1),0)) {
        "p.&nbsp;" substring(date(ev),5,strlen(date(ev))) set(nic,1) }
   if (ne(index(date(ev),"AFT",1),0)) {
        "po&nbsp;" substring(date(ev),5,strlen(date(ev))) set(nic,1) }
   if (ne(index(date(ev),"ABT",1),0)) {
        "ok.&nbsp;" substring(date(ev),5,strlen(date(ev))) set(nic,1) }
   if (eq(nic,0)) { date(ev) }
   if (place(ev)) { ", " place(ev) }
}
