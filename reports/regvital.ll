/*
 * @progname       regvital.ll
 * @version        3.0
 * @author         Wetmore, Manis, Chandler
 * @category       
 * @output         nroff
 * @description    

This program produces a report of all descendents of a given person,
and is presently designed for 12 pitch, HP laserjet III, for printing
a book about that person.  All NOTE and CONT lines are included in the
report, along with the vital statistics, occupations, immigrations,
attributes, and wills.
At the end of the report is a sorted listing of names of everyone
mentioned, with reference numbers giving the first occurrences of all
the names.


regvital

version 1 by Tom Wetmore
version 2 by Cliff Manis
version 3 by John Chandler, 1994

This program has paginated output with a footer and header.

This report produces a nroff output, and to produce the
output, use:  nroff filename > filename.out
         or:  troff -t filename | lpr -t

 */

global(bold)
global(idex)
global(srcs)
global(curref)
global(months)
global(dtform)
global(footn)
global(begnote)
global(endnote)

proc main () {

/* Customize the following: */
set(head,"Family History")
/* set(foot,"your name and address or whatever") */
set(foot,concat("Created ",stddate(gettoday())," by ",
                 getproperty("user.fullname"), " ",getproperty("user.email")))
set(ll,"8.5i")  /* line length for headers */
set(dtform,0)   /* date format: 0=dmy, 8=ymd, etc. */
set(footn,1)    /* if 1, then do footnote-style sources */
set(fancy,0)    /* if 1, then do superscript note refs */
set(bold,0)     /* if 1, use boldface for names */

list(months)
enqueue(months,"Jan") enqueue(months,"Feb")
enqueue(months,"Mar") enqueue(months,"Apr")
enqueue(months,"May") enqueue(months,"Jun")
enqueue(months,"Jul") enqueue(months,"Aug")
enqueue(months,"Sep") enqueue(months,"Oct")
enqueue(months,"Nov") enqueue(months,"Dec")

if(fancy){
        set(begnote,"\\u\\s-2") /* or use left-bracket for ASCII version */
        set(endnote,"\\s0\\d")  /* or use right-bracket */
} else {
        set(begnote,"[")
        set(endnote,"]")
}
dateformat(dtform)
if(or(eq(dtform,0),eq(dtform,8))){dayformat(0) monthformat(4)}
elsif(eq(dtform,1)){dayformat(2) monthformat(4)}
else{dayformat(1) monthformat(1)}

getindi(indi)
getintmsg(maxgen,"Enter max generations to include (0 if no limit)")
set(maxgen,sub(maxgen,1))

set(tday, gettoday())
".de hd\n"      /* header */
".ev 1\n"
".sp 2\n"
".tl '" head "''%'\n"
".tl ''" stddate(tday) "''\n"
"\n"
"'sp 3\n"
".ev\n"
"..\n"
".de fo\n"      /* footer */
".ev 1\n"
".sp\n"
".tl '" foot "'''\n"
".sp\n"
".ev\n"
"'bp\n"
"..\n"
".wh 0 hd\n"
".wh -.8i fo\n"
".de CH\n"      /* CHild number macro */
".sp\n"
".in 14n\n"
".ti 0\n"
"\\h'5n'\\h'-\\w'\\\\$1'u'\\\\$1\\h'6n'\\h'-\\w'\\\\$2'u'\\\\$2\\h'2n'\n"
"..\n"
".de II\n"      /* Index Item macro */
".br\n"
"\\\\$1\\h'-\\w'\\\\$1'u'\\h'35n'"
"\\\\$2\\h'-\\w'\\\\$2'u'\\h'13n'"
"\\\\$3\\h'-\\w'\\\\$3'u'\\h'13n'"
"\\\\$4\n"
"..\n"
".de IN\n"      /* Individual Number macro */
".sp\n"
".in 0\n"
"..\n"
".de GN\n"      /* Generation Number macro */
".br\n"
".ne 2i\n"
".sp 2\n"
".in 0\n"
".ce\n"
"..\n"
".de P\n"       /* Paragraph macro */
".sp\n"
".in 0\n"
".ti 5n\n"      /* indent 1st line */
"..\n"
".ev 1\n"
".ll " ll nl()  /* line length */
".ev\n"
".po 9\n"       /* left margin */
".ls 1\n"
".na\n"
list(ilist) list(glist)
table(stab) indiset(idex)
enqueue(ilist,indi) enqueue(glist,1)
set(curgen,0) set(out,1) set(in,2)
if(footn) {list(srcs)}

while (indi,dequeue(ilist)) {
    print("OUT: ") print(d(out))
    print(" ") print(name(indi)) print(nl())
    set(thisgen,dequeue(glist))
    if (ne(curgen,thisgen)) {
        if(or(lt(maxgen,0),gt(maxgen,1))){".GN\nGENERATION " d(thisgen) "\n\n"}
        set(curgen,thisgen)
    }
    ".IN\n" d(out) ". "
    insert(stab,save(key(indi)),out)
    set(curref,out)
    call longvitals(indi,curgen)
    addtoset(idex,indi,curref)
    set(out,add(out,1))
    families(indi,fam,spouse,nfam) {
        ".P\n"
        if (spouse) { set(sname, save(name(spouse))) }
        else        { set(sname, "_____") }
        if(eq(0,strcmp("",sname))) { set(sname, "_____") }
        if (eq(0,nchildren(fam))) {
            name(indi) " and " sname
            " had no children.\n"
        } elsif (and(spouse,lookup(stab,key(spouse)))) {
            "Children of " name(indi) " and " sname " are shown "
            "under " sname " (" d(lookup(stab,key(spouse))) ").\n"
        } else {
            "Children of " name(indi) " and " sname ":\n"
            children(fam,child,nchl) {
                set(haschild,0)
                families(child,cfam,cspou,ncf) {
                    if (ne(0,nchildren(cfam))) { set(haschild,1) }
                }
                if(and(haschild,or(gt(maxgen,curgen),lt(maxgen,0)))) {
                    print("IN: ") print(d(in))
                    print(" ") print(name(child)) print(nl())
                    enqueue(ilist,child)
                    enqueue(glist,add(1,curgen))
                    ".CH " d(in) " " roman(nchl) nl()
                    set (in, add (in, 1))
                    call shortvitals(child)
                } else {
                    ".CH " qt() qt() " " roman(nchl) nl()
                    call longvitals(child,0)
                    addtoset(idex,child,curref)
                }
            }
        }
    }
}
if(and(footn,length(srcs))){
        "\n.in 0\n.sp 2\n---------------\n.sp\nSources of information:\n"
        forlist(srcs,s,n){ if(gt(n,1)){";\n"} "[" d(n) "] " s }
        ".\n"
}
if(or(lt(maxgen,0),gt(maxgen,1))){
    print("begin sorting\n")
    namesort(idex)
    print("done sorting\n")
    ".bp\n"
    ".in 0\n"
    "Index of Persons in this Report (first occurrence)\n\n"
    ".II Name Birth Death #\n\n"
    forindiset(idex,indi,v,n) {
        ".II " qt()fullname(indi,1,0,30)qt()
        " " qt()stddate(birth(indi))qt()
        " " qt()stddate(death(indi))qt()
        " " d(v) nl()
        print(".")
    }
    nl()
    print(nl())
}}

proc shortvitals(indi){
name(indi)
set(b,birth(indi)) set(d,death(indi))
if (and(b,short(b))) { ", b. " short(b) }
if (and(d,short(d))) { ", d. " short(d) }
nl()
}

proc longvitals(i,flag){        /* all data and notes for individual */
if(not(footn)) {list(srcs)}
if (bold) { "\\f3" }
name(i)
if (bold) { "\\f1" }
set(e,birth(i))
if(and(e,long(e))) { ",\nborn " call displong(e) }
if(not(and(e,place(e)))) {
        set(e,baptism(i))
        if(and(e,long(e))) { ",\nbaptized " call displong(e) }
}
if(eq(flag,1)) { call printparents(i) }
".\n"
set(e,death(i))
if(and(e,long(e))) { "Died " call displong(e) ".\n" }
if(not(and(e,place(e)))) {
        set(e,burial(i))
        if(and(e,long(e))) {"Buried " call displong(e) ".\n"}
}
if (eq(1,nspouses(i))) {
        spouses(i,s,f,n) {
                if(e,marriage(f)) {
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
                if(e,marriage(f)) {
                        "Married " ord(j) ","
                        set(j,add(j,1))
                } else {
                        "Married"
                }
                call spousevitals(s,f)
        }
}
fornodes(inode(i), node) {
        set(ntag, save(tag(node)))
        if (eq(0,strcmp("FILE", ntag))) {
                copyfile(value(node))
        } elsif (eq(0,strcmp("NOTE", ntag))) {
                value(node)
                fornodes(node, subnode) {
                        if (eq(0,strcmp("CONT", tag(subnode)))) {
                                nl() value(subnode)
                        }
                }
                call setsrc(node) nl()
        } elsif (eq(0,strcmp("OCCU", ntag))) {
                "Occupation: " value(node)
                call setsrc(node)
                ".\n"
        } elsif (eq(0,strcmp("ATTR", ntag))) {
                "Attributes: " value(node)
                call setsrc(node)
                ".\n"
        } elsif (eq(0,strcmp("IMMI", ntag))) {
                if(long(node)) {
                        "Immigrated " call displong(node)
                        fornodes(node, subnode) {
                                if(eq(0,strcmp("NOTE",tag(subnode)))) {
                                        ",\n" value(subnode)
                                }
                        }
                        ".\n"
                }
        } elsif (eq(0,strcmp("WILL", ntag))) {
                if(long(node)) { "Made a will " call displong(node) ".\n" }
        } elsif (eq(0,strcmp("PROB", ntag))) {
                if(long(node)) { "Will proved " call displong(node) ".\n" }
        }
}
if(and(not(footn),length(srcs))){
        "\nSources of information:\n"
        forlist(srcs,s,n){ if(gt(n,1)){";\n"} s }
        ".\n"
}}

proc displong(e) {      /* display full date, place, and age for an event */
/* long(e) */
extractdate(e,da,mo,yr)
if(mod,date(e)){
        if(or(da,or(mo,yr))){
                set(mod,trim(mod,3))
                   if(eq(0,strcmp(mod,"ABT"))) {"about "}
                elsif(eq(0,strcmp(mod,"abo"))) {"about "}
                elsif(eq(0,strcmp(mod,"AFT"))) {"after "}
                elsif(eq(0,strcmp(mod,"aft"))) {"after "}
                elsif(eq(0,strcmp(mod,"BEF"))) {"before "}
                elsif(eq(0,strcmp(mod,"bef"))) {"before "}
                elsif(eq(0,strcmp(mod,"BET"))) {"beginning "}
                if(or(eq(1,dtform),le(8,dtform))){
                        if(yr){ d(yr) if(mo){" "}}
                        if(mo){ getel(months,mo) if(da){" "d(da)}}
                } else{
                        if(da){ d(da) if(mo){" "}}
                        if(mo){ getel(months,mo) if(yr){" "}}
                        if(yr){d(yr)}
                }
        } else { mod }
        if(place(e)){ ", "}
}
if(mod,place(e)) { mod }
fornodes(e,subnode) {
        if(eq(0,strcmp("AGE",tag(subnode)))) {
                ",\naged " value(subnode)
        }
}
call setsrc(e)
}

proc setsrc(node) {     /* collect source reference, if any */
fornodes(node,subnode){
        if(eq(0,strcmp("SOUR",tag(subnode)))){
                if(n,length(srcs)){
                        set(i,0)
                        while(lt(i,n)){
                                set(i,add(i,1))
                                if(eq(0,strcmp(getel(srcs,i),value(subnode)))){
                                        set(n,i) set(skip,1)
                                }
                        }
                }
                if(not(skip)){
                        enqueue(srcs,save(value(subnode)))
                        set(i,add(n,1))
                }
        if(footn){
                if(not(started)){begnote set(started,1)}
                else{","}
                d(i)}
        }
}
if(started){endnote}
}

proc spousevitals (sp,fam) {
list(names)
addtoset(idex,sp,curref)
set(e,marriage(fam))
if (and(e,long(e))) { nl() call displong(e) "," }
"\n"
if (bold) { "\\f3" }
if(strcmp("",name(sp))) {name(sp)} else {"_____"}
if (bold) { "\\f1" }
if(e){
        fornodes(e,subnode) {
                if(eq(0,strcmp("NAME",tag(subnode)))){
                        extractnames(subnode,names,n,s)
                        if(s) {"\n(under the name " getel(names,s) ")"}
                }
        }
}
set(e,birth(sp))
if(and(e,long(e))) { ",\nborn " call displong(e) }
set(e,death(sp))
if(and(e,long(e))) { ",\ndied " call displong(e) }
call printparents(sp)
}

proc printparents(ind) {        /* print only if non-blank */
if(dad,father(ind)) {if(ndad,name(dad)) {set(nbld,strcmp("",ndad))}}
if(mom,mother(ind)) {if(nmom,name(mom)) {set(nblm,strcmp("",nmom))}}
if (or(nbld,nblm)) {
        ",\n"
        if (male(ind))      { "son of " }
        elsif (female(ind)) { "daughter of " }
        else                { "child of " }
}
if (nbld) { name(dad) }
if (and(nbld,nblm)) { "\nand " }
if (nblm) { name(mom) }
".\n"
if (nbld) { addtoset(idex,dad,curref) }
if (nblm) { addtoset(idex,mom,curref) }
}

/*  Sample printout of the report, plus also prints a names index.

         Manes - Manis - Maness  Family  History
                                                                    14 Jan 1993

                                   GENERATION 1


         1. William Bowers MANES, born 6 Jan 1868, Hamblen Co, TN ?, died 5
         May 1933, Sevier Co, TN.  Married 13 Apr 1892, White Pine, TN,
         Cordelia "Corda" F. CANTER, born 7 Dec 1869, Jonesboro, Washington
         Co, TN, died 18 Apr 1960, Knoxville, Knox Co, TN, daughter of James
         H. CANTER and Martha Marie WHITEHORN.  He died of pneumonia at his
         homeplace in Union Valley, Sevier Co, TN He was buried at the Knob
         Creek Baptist Church cemetery in Sevier County, TN.

            Children of William Bowers MANES and Cordelia "Corda" F. CANTER:

             2     i   Nellie V. MANES, b. 1893, TN, d. 1984, TN

                  ii   Emery H. MANES, born 24 Oct 1894, White Pine,
                       Jefferson Co, TN, died 26 Jul 1926, Knob Creek,
                       Sevier Co., TN.  Died in auto accident, when he
                       and a brother were going in his truck with a load
                       of vegetables, and going to market in Knoxville.
                       He is buried at Knob Creek Cem. Sevier Co, TN.

             3   iii   Walter C. MANES, b. 1896, TN, d. 1989, TN

             4    iv   William Lee MANES, b. 1897, TN, d. 1969, TN

                   v   George MANES, born 29 Oct 1898, Union Valley,
                       Sevier Co, TN, died 17 Jun 1899, Knob Creek,
                       Sevier Co, TN.  Single, died as a infant, and is
                       buried at Knob Creek Cem, Sevier Co, TN.

             5    vi   Fuller Ruben MANES, b. 1902, TN, d. 1980, TN

             6   vii   Mabel E. MANES, b. 1905, TN

             7  viii   Lena G. MANES, b. 1906, TN, d. 1987, TN

             8    ix   Wade Preston MANES, b. 1910, TN

             9     x   Newman Clarence MANES, b. 1912, TN

//end of sample//    */

