/*
 * @progname       relation.ll
 * @version        5.0
 * @author         Eggert
 * @category       
 * @output         Text
 * @description    

This program calculates the relationship between individuals in a
database.  It does so in three modes.  Mode 1 just does one pair of
individuals and then exits.  Mode 2 does any number of pairs with a
common "from" person.  Mode 3 does all pairs with a common "from"
person.  In general, mode 1 is fastest for simple relationships, but
if you want one complicated relationship, you may as well do them all.

relation - a LifeLines relation computing program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  21 November 1992
        Version 2,  23 November 1992 (completely revamped)
        Version 3,  (changed format slightly, modified code somewhat)
        Version 4,   6 July 1993 (added English language)
        Version 5,   6 September 1993 (generified language)

Each computed relation is composed of the minimal combination of
parent (fm), sibling (bsS), child (zdC), and spouse (hw) giving the
relational path from the "from" person to the "to" person.  Each
incremental relationship (or hop) is coded as follows, with the
capital letters denoting a person of unknown gender:
        father  f
        mother  m
        parent  P (not used)
        brother b
        sister  s
        sibling S
        son     z (sorry)
        daughtr d
        child   C
        husband h
        wife    w
        spouse  O (sorry again, but usually not possible)

The report gives the steps required to go from the first person to
the second person.  Thus the printout
        I93 John JONES fmshwz I95 Fred SMITH
means that John Jones' father's mother's sister's husband's wife's son
is Fred Smith.  Notice in this case, the sister's husband's wife is
not the same as the sister, and the husband's wife's son is not the
same as the husband's son.  Thus in more understandable English, John
Jones' paternal grandmother's sister's husband's wife's son from
another marriage is Fred Smith.

The program will do a trivial parsing of the path string.  You can
change the language_table to have it print in different languages, as
long as the word order is unchanged.

If there is no relation, the program says so.  That at least should be
easy to explain.  Mode 3 only prints out those individuals who are
related to the "from" individual.
*/

global(plist)
global(hlist)
global(mark)
global(keys)
global(found)
global(do_names)
global(language)
global(language_table)
global(token)
global(untoken)

proc include(person,hops,keypath,path,pathend)
{
    if (and(person,eq(found,0))) {
        set(pkey,key(person))
        if (entry,lookup(mark,pkey)) {
            if (eq(strcmp(entry,"is not related to"),0)) {
                set(found,1)
                list(plist)
                list(hlist)
                insert(mark,pkey,concat(path,pathend))
                insert(keys,pkey,concat(concat(keypath,"@"),pkey))
            }
        }
        else {
            enqueue(plist,pkey)
            enqueue(hlist,hops)
            insert(mark,pkey,concat(path,pathend))
            insert(keys,pkey,concat(concat(keypath,"@"),pkey))
        }
    }
}

proc get_token(input) {
/*  Parse a token from the input string.
    Tokens are separated by one or more "@"s.
    Set global parameter token to the first token string.
    Set global parameter untoken to the rest of the string after first token.
*/
/* strip leading @s */
    set(untoken,input)
    set(first_delim,index(untoken,"@",1))
    while (eq(first_delim,1)) {
        set(untoken,substring(untoken,2,strlen(untoken)))
        set(first_delim,index(untoken,"@",1))
    }
/* get token and untoken */
    if (not(first_delim)) {
        set(token,untoken)
        set(untoken,"")
    }
    else {
        set(token,substring(untoken,1,sub(first_delim,1)))
        set(untoken,
            substring(untoken,add(first_delim,1),strlen(untoken)))
    }
}

proc parse_relation(relation,keypath) {
    if (not(language)) {
        " " relation
        if (do_names) {
            set(untoken,keypath)
            call get_token(untoken)
            while(strlen(untoken)) {
                call get_token(untoken)
                " " token " " name(indi(token))
            }
        }
        " "
    }
    else {
        set(charcounter,1)
        set(untoken,keypath)
        call get_token(untoken)
        while (le(charcounter,strlen(relation))) {
            lookup(language_table,substring(relation,charcounter,charcounter))
            if (do_names) {
                call get_token(untoken)
                " " token " " name(indi(token))
            }
            set(charcounter,add(charcounter,1))
        }
        " is "
    }
}

proc main ()
{
    table(mark)
    table(keys)
    list(plist)
    list(hlist)

    table(language_table)
    insert(language_table,"f","'s father")
    insert(language_table,"m","'s mother")
    insert(language_table,"P","'s parent")
    insert(language_table,"b","'s brother")
    insert(language_table,"s","'s sister")
    insert(language_table,"S","'s sibling")
    insert(language_table,"z","'s son")
    insert(language_table,"d","'s daughter")
    insert(language_table,"C","'s child")
    insert(language_table,"h","'s husband")
    insert(language_table,"w","'s wife")
    insert(language_table,"O","'s spouse")

    getindimsg(from_person,
        "Enter person to compute relation from:")
    set(from_key,key(from_person))
    set(hopcount,0)
    set(prev_hopcount,neg(1))
    set(found,0)
    call include(from_person,hopcount,"","","")
    getintmsg(mode,"Enter 1 for a single relation, 2 for several, 3 for all:")
    getintmsg(language,
        "Enter 0 for brief, 1 for English-language relationships:")
    getintmsg(do_names,
        "Enter 0 to omit, 1 to output names of all intervening relatives:")
    if (eq(mode,1)) {
        getindimsg(to_person,
            "Enter one person to compute relation to:")
        set(to_key,key(to_person))
        if (strcmp(from_key,to_key)) {
            insert(mark,to_key,"is not related to")
        }
        else {
            list(plist)
            list(hlist)
        }
    }
    while (pkey,dequeue(plist)) {
        set(person,indi(pkey))
        set(hopcount,dequeue(hlist))
        set(path,lookup(mark,pkey))
        set(keypath,lookup(keys,pkey))
        if (ne(hopcount,prev_hopcount)) {
            print(".")
            set(prev_hopcount,hopcount)
        }
        set(hopcount,add(hopcount,1))
        call include(father(person),hopcount,keypath,path,"f")
        call include(mother(person),hopcount,keypath,path,"m")
        children(parents(person),child,cnum) {
            if (male(child)) { set(pathend,"b") }
            elsif (female(child)) { set(pathend,"s") }
            else { set(pathend,"S") }
            call include(child,hopcount,keypath,path,pathend)
        }
        families(person,fam,spouse,pnum) {
            if (male(spouse)) { set(pathend,"h") }
            elsif (female(spouse)) { set(pathend,"w") }
            else { set(pathend,"O") }
            call include(spouse,hopcount,keypath,path,pathend)
            children(fam,child,cnum) {
                if (male(child)) { set(pathend,"z") }
                elsif (female(child)) { set(pathend,"d") }
                else { set(pathend,"C") }
                call include(child,hopcount,keypath,path,pathend)
            }
        }
    }
    if (eq(mode,1)) {
        from_key " " name(indi(from_key))
        call parse_relation(lookup(mark,to_key),lookup(keys,to_key))
        to_key   " " name(indi(to_key)) "\n"
    }
    if (eq(mode,2)) {
        set(want_another,1)
        while (want_another) {
            getindimsg(to_person,"Enter person to compute relation to:")
            set(to_key,key(to_person))
            from_key " " name(indi(from_key))
            if (path,lookup(mark,to_key)) {
                call parse_relation(path,lookup(keys,to_key))
            }
            else { " is not related to " }
            to_key  " "  name(to_person) "\n"
            getintmsg(want_another,
                  "Enter 0 if done, 1 if you want another to person:")
        }
    }
    if (eq(mode,3)) {
        from_key " " name(indi(from_key)) " --->\n"
        forindi(to_person,num) {
            set(to_key,key(to_person))
            if (path,lookup(mark,to_key)) {
                call parse_relation(path,lookup(keys,to_key))
                to_key " " name(to_person) "\n"
            }
        }
    }
}

