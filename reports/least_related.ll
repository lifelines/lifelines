/*
least_related - a LifeLines relation computing program
        by Jim Eggert (eggertj@atc.ll.mit.edu)
        Version 1,  5 June 1994

This program calculates the pair of individuals in a database that are
both related to a seed person, but are least related to each other.
Note that the pair is really not unique, but this program only
produces one pair.  If the database contains mutually unrelated
partitions, then only the partition containing the seed person is
really used.

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
change the language_table to have it print in any language you like.

*/

global(plist)
global(hlist)
global(mark)
global(keys)
global(do_names)
global(language)
global(language_table)
global(token)
global(untoken)

proc include(person,hops,keypath,path,pathend)
{
    if (person) {
        set(pkey,key(person))
        if (not(lookup(mark,pkey))) {
            enqueue(plist,save(pkey))
            enqueue(hlist,hops)
            insert(mark,save(pkey),save(concat(path,pathend)))
            insert(keys,save(pkey),save(concat(concat(keypath,"@"),pkey)))
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
    set(untoken,save(input))
    set(first_delim,index(untoken,"@",1))
    while (eq(first_delim,1)) {
        set(untoken,save(substring(untoken,2,strlen(untoken))))
        set(first_delim,index(untoken,"@",1))
    }
/* get token and untoken */
    if (not(first_delim)) {
        set(token,save(untoken))
        set(untoken,save(""))
    }
    else {
        set(token,save(substring(untoken,1,sub(first_delim,1))))
        set(untoken,save(
            substring(untoken,add(first_delim,1),strlen(untoken))))
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

    getintmsg(language,
        "Enter 0 for brief, 1 for English-language relationships:")
    getintmsg(do_names,
        "Enter 0 to omit, 1 to output names of all intervening relatives:")
    getindimsg(from_person,
        "Enter seed person:")

    set(dot,"-")
    set(iterations,2)
    while (gt(iterations,0)) {
        table(mark)
        table(keys)
        list(plist)
        list(hlist)
        set(iterations,sub(iterations,1))
        set(from_key,save(key(from_person)))
        set(hopcount,0)
        set(prev_hopcount,neg(1))
        call include(from_person,hopcount,"","","")
        while (pkey,dequeue(plist)) {
            set(person,indi(pkey))
            set(hopcount,dequeue(hlist))
            set(path,lookup(mark,pkey))
            set(keypath,lookup(keys,pkey))
            if (ne(hopcount,prev_hopcount)) {
                print(dot)
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
        if (eq(iterations,1)) {
            set(from_person,person)
            set(first_key,save(key(person)))
            print("\n")
            set(dot,"+")
        }
    }

    "Longest relationship chain is from "
    first_key " " name(indi(first_key)) " to "
    set(last_key,save(key(person)))
    last_key " " name(person) "\n"

    first_key " " name(indi(first_key)) " "
    if (path,lookup(mark,last_key)) {
        call parse_relation(save(path),lookup(keys,last_key))
        "\n"
    }
}

