/*
 * @progname       igi-merge.ll
 * @version        4.0
 * @author         Eggert
 * @category       
 * @output         GEDCOM
 * @description    

This program helps you merge IGI data.  IGI data consists of "families"
which are actually events.  A real family may be documented by several
events in the IGI database, and thus be represented by several "families".
This program will help you to locate and merge those "families" back into
real families again.

igi-merge - a LifeLines IGI database merging program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  2 February 1993  Requires LifeLines 2.3.3 or later.
        Version 2, 17 February 1993  bug fix, better suggestions
        Version 3, 15 March    1993  more tunable suggestions,
                                     added windup and restart
        Version 4,  9 November 1994  Requires LifeLines 3.0.1 or later.
                                     Minor windup bug fix.

The program locates candidate "families" to be merged by comparing the
soundex, double initials of husband to husband and wife to wife, and
event years.  (Double initials are like "JO" for John.  Any double
initials are allowed to match.  Hence George Michael will match Mike,
and Betty Lou will match Lois Amelia.)  If the comparison indicates a
possible match, the program prompts you for approval to merge.  If you
answer with y or Yes or yazbotchit, the program will merge those two
families, otherwise it will not.  The two husbands will be merged
together, as will the two wives.

The comparison is made following a strictness code entered at the
beginning of the program execution.  The user is prompted for the
strictness level, which means more precisely:
1  soundexes and double initials must match
2  soundexes must match
3  soundexes and double initials must match, but empty soundexes match anything
4  soundexes must match, but empty soundexes match anything
5  double intitials must match
6  nothing needs match
I usually use the strictness in a multiple-pass method:  First I run
igi-merge with strictness 1.  I import the resulting GEDCOM file into
an empty LL database, and run igi-merge with strictness 2 or 3.  And so
on until I am satisfied.

In any case, the event years are always used for declaring matches.
Two families match only if their event years are within forty years of
each other.  If the events are marriage events, however, they must be
within five years of each other.  If a family has more than one event
associated with it (for partially merged IGI data, for example), any
marriage event has precedence.

After families are merged, the program puts the children in birth order,
and attempts to locate children who are really the same.  It prompts you
for approval to merge any two children born or christened in the same year.

Any merged individuals will retain one copy of each name variant and of
each variant sex.  (There shouldn't be any of the latter!)  Other data,
such as birth and marriage events etc, are simply copied; duplicate
information may therefore be retained after the merge.  You are urged
to edit the resulting file to look for and possibly delete such
duplication.

The resulting data is written to a GEDCOM file.  You may read this back
into a LifeLines database if you wish.

If you run out of time (because, for example, you are merging ten
thousand families), you can invoke windup.  If you answer the family
merge question with w (or windup, or Whatever), the program will act as
if you answered all the remaining queries negatively.

If you want to be really fancy, before you issue the w command, write
down the families that you are being queried about.  Then later you
can read the resulting GEDCOM file into a new LifeLines database, and
start up igi-merge again.  When asked what comparison strictness to
use, answer with zero.  You will be prompted for the two families to
restart the program at.  Make sure that you enter the top family
first, otherwise it won't work.  The program will then resume your
previous igi-merge session where you had left off.

With windup and restart, you don't really have to have a single block
of time to be able to merge a large dataset.

The program does some rudimentary checking to see if the source data
really is IGI-like.  If not, it complains, but keeps on running
anyway.  Because it only writes a GEDCOM file, this program can't
corrupt your database, so don't worry.

The user interface depends on the size of the LifeLines screen,
I have marked the sensitive lines with a commented #
 */

global(famged)
global(gedsex)
global(names)
global(ptable)
global(event_year)
global(event_string)
global(event_type) /* 1=birth, 2=baptism, 3=marriage,
                        and in the future 4=death, 5=burial */
global(compare_level)
global(windup)

func get_yesno(prompt) {
    set(yes,0)
    set(windup,0)
    getstrmsg(yesno,prompt)
    if (gt(strlen(yesno),0)) {
        if (not(strcmp(upper(trim(yesno,1)),"Y"))) { set(yes,1) }
        if (not(strcmp(upper(trim(yesno,1)),"W"))) { set(windup,1) }
    }
    return(yes)
}

/* Note that marriage events have priority! */
proc get_event(family) {
    set(event_year,0)
    if (e,marriage(family)) {
        extractdate(e,day,month,event_year)
        set(event_string,"m. ")
        set(event_type,3)
    }
    else {
        children(family,child,cnum) {
            if (eq(cnum,1)) {
                if (e,birth(child)) {
                    set(event_string,concat(name(child)," b. "))
                    extractdate(e,day,month,event_year)
                    set(event_type,1)
                }
                elsif (e,baptism(child)) {
                    set(event_string,concat(name(child)," c. "))
                    extractdate(e,day,month,event_year)
                    set(event_type,2)
                }
            }
        }
    }
    if (not(event_year)) {
/*      print("Event year not found.\n") */
        set(event_string,concat(event_string,"date unknown"))
    }
    else {
        set(event_string,concat(event_string,d(event_year)))
    }
    if (p,place(e)) {
        set(event_string,concat(event_string," "))
        set(event_string,concat(event_string,p))
    }
    set(event_string,save(trim(event_string,73))) /*#*/
}

proc write_ged_indi(person,newperson,ftag,famkey) {
    if (not(person)) { break() }
    if (lookup(ptable,key(person))) {
        print("Database doesn't look like IGI data.\n")
        print("  -  ", key(person), " ", name(person), "\n")
    }
    insert(ptable,save(key(person)),famkey)
    set(n,inode(person))
    if (newperson) {
        while (dequeue(names)) { "" }
        set(gedsex,"X")
    }
    traverse(n,node,level) {
        if (level) {
            set(t,tag(node))
            if (and(strcmp(t,"FAMS"),strcmp(t,"FAMC"))) {
                set(write_line,1)
                if (not(strcmp(t,"NAME"))) {
                    if (newperson) {
                        enqueue(names,save(value(node)))
                    }
                    else {
                        set(thisname,save(value(node)))
                        forlist(names,prevname,pnum) {
                            if (not(strcmp(thisname,prevname))) {
                                set(write_line,0)
                            }
                        }
                        if (write_line) { enqueue(names,thisname) }
                    }
                }
                if (not(strcmp(t,"SEX"))) {
                    if (newperson) { set(gedsex,save(value(node))) }
                    elsif (not(strcmp(gedsex,value(node)))) {
                        set(write_line,0)
                    }
                    else { set(gedsex,save(value(node))) }
                }
                if (write_line) { d(level) " " t " " value(node) "\n" }
            }
            elsif (newperson) {
                d(level) " " t " @" famkey "@\n"
            }
        }
        elsif (newperson) { "0 " xref(node) " INDI\n" }
    }
    if (newperson) {
        enqueue(famged,ftag)
        enqueue(famged,save(key(person)))
        enqueue(famged,"@\n")
    }
}

proc write_ged_fam(fam) {
    if (not(fam)) { break() }
    set(n,fnode(fam))
    traverse(n,node,level) {
        if (not(level)) { continue() }
        if (eq(level,1)) { set(levelonetag,save(tag(node))) }
        if (and(and(strcmp(levelonetag,"HUSB"),
                    strcmp(levelonetag,"WIFE")),
                    strcmp(levelonetag,"CHIL"))) {
            enqueue(famged,save(d(level)))
            enqueue(famged," ")
            enqueue(famged,save(tag(node)))
            enqueue(famged," ")
            enqueue(famged,save(value(node)))
            enqueue(famged,"\n")
        }
    }
}

func compare(aint,bint) {
    if (lt(aint,bint)) { return(neg(1)) }
    elsif (gt(aint,bint)) { return(1) }
    else { return(0) }
}

proc bubblesort(alist,ilist)
{
/*    print("bubblesorting list of length ") print(d(length(alist))) */
/*    print(" entries.\n") */
    while (dequeue(ilist)) { "" }
    forlist(alist,ael,index) { enqueue(ilist,index) }
    while (gt(index,0)) {
        set(bubblepos,index)
        set(bubbleindex,getel(ilist,bubblepos))
        set(abubble,getel(alist,bubbleindex))
        set(movedup,0)
        set(comparison,neg(1))
        while (and(gt(bubblepos,1),lt(comparison,0))) {
            set(bubbleupindex,getel(ilist,sub(bubblepos,1)))
            set(bubbleup,getel(alist,bubbleupindex))
            set(comparison,compare(abubble,bubbleup))
            if (lt(comparison,0)) {
                setel(ilist,bubblepos,bubbleupindex)
                decr(bubblepos)
                set(movedup,1)
            }
        }
        if (eq(movedup,0)) {
            set(comparison,1)
            while(and(lt(bubblepos,length(alist)),gt(comparison,0))) {
                set(bubbledownindex,getel(ilist,add(bubblepos,1)))
                set(bubbledown,getel(alist,bubbledownindex))
                set(comparison,compare(abubble,bubbledown))
                if (gt(comparison,0)) {
                    setel(ilist,bubblepos,bubbledownindex)
                    incr(bubblepos)
                }
            }
        }
        setel(ilist,bubblepos,bubbleindex)
        if (eq(movedup,0)) { decr(index) }
    }
}

func sound_compare(asound,bsound) {
    if (and(strlen(asound),strlen(bsound))) {
        return(strcmp(asound,bsound))
    }
    elsif (ge(compare_level,3)) { return(0) }
    else { return(1) }
}

func initial_compare(namelist1,namelist2,len2) {
    forlist(namelist2,this2,n) {
        if (ge(n,len2)) { break() }
        set(init2,save(upper(trim(this2,2))))
        forlist(namelist1,init1,m) {
            if (not(strcmp(init1,init2))) {
                return(0)
            }
        }
    }
    return(1)
}

global(hsound)
global(wsound)
global(hnamelist)
global(wnamelist)
global(hexists)
global(wexists)

func names_compare(fam2) {
    list(namelist)
    set(hsound2,"")
    if (and(hexists,husband(fam2))) {
        if (lt(compare_level,5)) {
            set(hsound2,save(soundex(husband(fam2))))
            if (not(strcmp(hsound2,"Z999"))) { set(hsound2,"") }
            if (s,sound_compare(hsound,hsound2)) { return(s) }
        }
        if (mod(compare_level,2)) {
            extractnames(inode(husband(fam2)),namelist,n1,n2)
            if (s,initial_compare(hnamelist,namelist,n2)) { return(s) }
        }
    }
    elsif (le(compare_level,2)) { return(1) }
    if (and(wexists,wife(fam2))) {
        if (lt(compare_level,5)) {
            set(wsound2,save(soundex(wife(fam2))))
            if (not(and(strcmp(wsound2,"Z999"),
                        strcmp(wsound2,hsound2)))) { set(wsound2,"") }
            if (s,sound_compare(wsound,wsound2)) { return(s) }
        }
        if (mod(compare_level,2)) {
            extractnames(inode(wife(fam2)),namelist,n1,n2)
            if (n,initial_compare(wnamelist,namelist,n2)) { return(n) }
        }
    }
    elsif (le(compare_level,2)) { return(1) }
    return(0)
}

proc main() {
    list(husbands)
    list(wives)
    list(childlist)
    list(childyear)
    list(childindex)
    list(childevent)
    list(names)
    list(hnamelist)
    list(wnamelist)
    table(ftable)
    table(ptable)
    list(famged)

    set(compare_level,neg(1))
    while(or(lt(compare_level,0),gt(compare_level,6))) {
        getintmsg(compare_level,
            "Enter comparison strictness (0=restart,1=very...6=not at all)")
        if (eq(compare_level,0)) {
            set(restart,1)
            print("Select first family to restart from")
            getfam(rfam)
            set(rfkey,key(rfam))
            print("Select second family to restart from")
            getfam(rfam)
            set(rfkey2,key(rfam))
            getintmsg(compare_level,
                "Enter real comparison strictness (1=very...6=not at all)")
        }
    }
    set(num_merged,0)
    forfam(fam,fnum) { "" }
    print("Trying to merge ", d(fnum), " families.\n")
    set(next_fnum,0)
    set(windup,0)
    forfam(fam,fnum) {
        if (ge(fnum,next_fnum)) {
            print(d(fnum), " ")
            incr(next_fnum)
        }
        set(famkey,save(key(fam)))
        if (lookup(ftable,famkey)) { continue() }
        insert(ftable,famkey,1)
        enqueue(famged,"0 @")
        enqueue(famged,famkey)
        enqueue(famged,"@ FAM\n")
        if (h,husband(fam)) {
            enqueue(husbands,save(key(h)))
            set(hexists,1)
        }
        else { set(hexists,0) }
        if (w,wife(fam)) {
            enqueue(wives,save(key(w)))
            set(wexists,1)
        }
        else { set(wexists,0) }
        children(fam,child,cnum) {
            enqueue(childlist,save(key(child)))
        }
        call write_ged_fam(fam)
        if (and(not(windup),
                or(and(restart,not(strcmp(key(fam),rfkey))),
                   and(not(restart),or(h,w))))) {
            set(hname,save(name(h)))
            set(wname,save(name(w)))
            set(hsound,save(soundex(h)))
            if (not(strcmp(hsound,"Z999"))) { set(hsound,"") }
            set(wsound,save(soundex(w)))
            if (not(and(strcmp(hsound,wsound),
                        strcmp(wsound,"Z999")))) { set(wsound,"") }
            if (mod(compare_level,2)) {
                if (h) {
                    extractnames(inode(h),hnamelist,n1,n2)
                    forlist(hnamelist,this,n) {
                        setel(hnamelist,n,save(upper(trim(this,2))))
                    }
                    while (le(n2,n1)) {
                        set(junk,pop(hnamelist))
                        incr(n2)
                    }
                }
                if (w) {
                    extractnames(inode(w),wnamelist,n1,n2)
                    forlist(wnamelist,this,n) {
                        setel(wnamelist,n,save(upper(trim(this,2))))
                    }
                    while (le(n2,n1)) {
                        set(junk,pop(wnamelist))
                        incr(n2)
                    }
                }
            }
            call get_event(fam)
            set(year,event_year)
            set(fevent_type,event_type)
            set(fevent,event_string)
            forfam(fam2,fnum2) {
                if (or(windup,
                       and(or(not(restart),strcmp(key(fam2),rfkey2)),
                           or(restart,le(fnum2,fnum))))) {
                    continue() }
                set(restart,0)
                set(famkey2,save(key(fam2)))
                if (not(lookup(ftable,famkey2))) {
                    if (not(names_compare(fam2))) {
                        call get_event(fam2)
                        set(ydiff,sub(event_year,year))
                        if (lt(ydiff,0)) { set(ydiff,neg(ydiff)) }
/* if soundexes are identical, and event years are close, ask user */
                        set(askit,0)
                        if (and(eq(event_type,3),eq(event_type,3))) {
                            if (lt(ydiff,5)) { set(askit,1) }
                        }
                        elsif (lt(ydiff,40)) { set(askit,1) }
                        if (askit) {
                            print("\n\n\n\n", hname, "\n", wname, "\n") /*#*/
                            print(fevent, "\n\n\n\n\n\n\n\n\n") /*#*/
                            print(name(husband(fam2)), "\n")
                            print(name(wife(fam2)), "\n")
                            print(event_string)
                        set(yes,get_yesno("Merge these families? (y/n/w)"))
                            print("\n")
                            if (yes) {
                                incr(num_merged)
                                insert(ftable,famkey2,2)
                                children(fam2,child,cnum) {
                                    enqueue(childlist,save(key(child)))
                                }
                                call write_ged_fam(fam2)
                                enqueue(husbands,save(key(husband(fam2))))
                                enqueue(wives,save(key(wife(fam2))))
                            }
                            elsif (windup) {
                                print("Winding up...")
                            }
                        }
                    }
                }
            }
        }
/* write out the parents */
        if (hkey,dequeue(husbands)) {
            set(h,indi(hkey))
            call write_ged_indi(h,1,"1 HUSB @",famkey)
            while (hkey,dequeue(husbands)) {
                set(h,indi(hkey))
                call write_ged_indi(h,0,"",famkey)
            }
        }
        if (wkey,dequeue(wives)) {
            set(w,indi(wkey))
            call write_ged_indi(w,1,"1 WIFE @",famkey)
            while (wkey,dequeue(wives)) {
                set(w,indi(wkey))
                call write_ged_indi(w,0,"",famkey)
            }
        }
/* collect children birthyears */
        forlist(childlist,childkey,cnum) {
            set(child,indi(childkey))
            if (e,birth(child)) {
                extractdate(e,day,month,year)
                enqueue(childevent," b. ")
            }
            elsif (e,baptism(child)) {
                extractdate(e,day,month,year)
                enqueue(childevent," c. ")
            }
            enqueue(childyear,year)
        }
/* sort by birthyear */
        call bubblesort(childyear,childindex)
        set(prev_year,neg(1))
/* merge and write children */
        forlist(childindex,index,inum) {
            set(child,indi(getel(childlist,index)))
            set(merge_child,0)
            if (and(not(windup),
                    eq(getel(childyear,index),prev_year))) {
                set(merge_child,1)
                print("\n\n\n\n\n") /*#*/
                print(prev_name, prev_event, d(prev_year))
                print("\n\n\n\n\n\n\n\n\n\n\n") /*#*/
            }
            set(prev_name,save(name(child)))
            set(prev_event,getel(childevent,index))
            set(prev_year,getel(childyear,index))
            if (merge_child) {
                print(prev_name, prev_event, d(prev_year))
                set(yes,get_yesno("Merge these children? (y/n)"))
                print("\n")
                if (yes) { set(merge_child,2) }
            }
            if (eq(merge_child,2)) {
                call write_ged_indi(child,0,"",famkey)
            }
            else {
                call write_ged_indi(child,1,"1 CHIL @",famkey)
            }
        }
/* empty out children data */
        while (not(empty(childlist))) {
            set(e,dequeue(childlist))
            set(e,dequeue(childevent))
            set(e,dequeue(childyear))
            set(e,dequeue(childindex))
        }
/* write out the family part of the GEDCOM file */
        while(not(empty(famged))) { dequeue(famged) }
    }
    "0 TRLR\n"
    print("\nMerged ", d(num_merged), " of ", d(fnum), " families.\n")
}
