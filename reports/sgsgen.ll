/*
 * @progname       sgsgen.ll
 * @version        1
 * @author         Jim Eggert (eggertj@ll.mit.edu)
 * @category       
 * @output         Text
 * @description

A LifeLines report program to aid in the generation of
soc.genealogy.surnames (sgs) submissions.

Given a person, this generates a sgs-like submission for that person and
his/her ancestors.  The output will likely need considerable hand editing,
but that is how it is.  If you need to know what sgs is, I have enclosed
a description at the end of this file.

Here's what you will need to do by hand (you can consider this a list
of desired features for future versions of this report program):

BEFORE YOU RUN THE PROGRAM:

Change the routines write_sgs_entry() and write_sgs_body() to
customize your address and standard message.

AFTER YOU RUN THE PROGRAM:

Eliminate or fix empty or useless surname lines.
Use sgs-standard abbreviations for placenames.
  Get FAMILY ABBREV as per instructions at end of this file.
Combine duplicate surname lines where appropriate.
Check check check.  Note I've put a checklist at the head of the
  report.  Until you edit the report by hand, it cannot be used for
  autosubmission via the telnet command as below.
Send the final product to the sgs maintainer.  To autosubmit,
  telnet your.news.host 119 < checked.sgs.report

Version 1, 13 January 1997, by Jim Eggert, eggertj@ll.mit.edu

*/

global(year_min)
global(year_max)
global(submitter_tag)
global(location)
global(last_location)
global(location_list)
global(location_table)
global(see_surname_table)
global(sgs_entry_count)

proc main() {
    list(ilist)
    table(see_surname_table)

    getindi(person)

    set(sgs_entry_count,0)

    enqueue(ilist,person)

    dayformat(1)  monthformat(4)  dateformat(5)

    "QUIT\n"
    "------------ edit checklist\n"
    "change placenames to rsl-type places in the subject lines\n"
    "ensure countries exist in migration components in the subject lines\n"
    "eliminate / in places in the subject lines\n"
    "fix special characters as needed\n"
    "look for repeated submissions\n"
    "delete sensitive info if desired\n"
    "delete this checklist, including the top QUIT command\n"
    "------------ end of edit checklist\n"

    while (person,dequeue(ilist)) {
        table(location_table)
        list(location_list)

        set(year_min,9999) set(year_max,0)
        call locations_and_years(person)
        set(final_key,save(key(person)))
        set(final_surname,save(surname(person)))

        if (m,mother(person)) { enqueue(ilist,m) }

        while (person,father(person)) {
            if (m,mother(person)) { enqueue(ilist,m) }
            call locations_and_years(person)
            set(this_surname,surname(person))
            if (strcmp(this_surname,final_surname)) {
                set(see_name,0)
                if (other_surname,lookup(see_surname_table,this_surname)) {
                    if (strcmp(final_surname,other_surname)) {
                        set(see_name,1)
                    }
                }
                else {
                    set(see_name,1)
                    set(other_surname,final_surname)
                }
                if (see_name) {
                    insert(see_surname_table,
                                save(this_surname),save(other_surname))
                }
            }
        }
        call write_sgs_entry(final_key)
    }
    if (gt(sgs_entry_count,100)) {
        print("Warning:  Number of lines produced (")
        print(d(sgs_entry_count))
        print(")\nexceeds recommended submission limit (100)\n")
    }
    "QUIT\n"
}


/* write_sgs_entry writes one submission to soc.genealogy.surnames*/

proc write_sgs_entry(key_entry) {
    if (key_entry) {
        set(surname_entry,save(surname(indi(key_entry))))
        if (strlen(surname_entry)) {
            if (strcmp(trim(surname_entry,1),"_")) {
                if (father(indi(key_entry))) {
                    incr(sgs_entry_count)
                    set(not_first,0)
                    "POST\n"
/*---*/             "From: your@email.address\n"
                    "Newsgroups: soc.genealogy.surnames\n"
                    "Subject: " upper(surname_entry) "; "
                    while (loc,dequeue(location_list)) {
                        if (not_first) { ">" } else { set(not_first,1) }
                        loc
                    }
                    "; " d(year_min) "-" d(year_max) "\n"
                    "Message-ID: <sgs" d(sgs_entry_count)
/*---*/             "@your.host.name>\n"
                    "\n"
                    "Organization: none\n"
                    call write_sgs_body(key_entry)
                    ".\n"
                }
            }
        }
    }
}


/* write_sgs_body writes the body of a message */

proc write_sgs_body(key_entry) {
/*---*/
    "I am offering information on the following paternal ancestral line,\n"
    "and am soliciting the sharing of genealogical data with any interested\n"
    "party.  The numbers preceeding the person's name are the generation\n"
    "number, counting from the most recent person.  Further information\n"
    "can be found in my web pages at\n"
    "    <http://your.web.page/>\n"
    "Please direct any communications to your@email.address\n"
/*---*/

    set(person, indi(key_entry))
    set(number,1)
    while (person) {
        call ahnenreport(person,number)
        incr(number)
        set(person,father(person))
    }
}

proc ahnenreport(person,number) {
    "\n" d(number) ". " fullname(person,0,1,80) "\n"
    if (e, birth(person))   { "    born: " long(e) "\n" }
    if (e, baptism(person)) { "baptized: " long(e) "\n" }
    set(nfam,nfamilies(person))
    families(person, fam, spouse, fnum) {
        set(e, marriage(fam))
        if (or(e,spouse)) {
            if (gt(nfam,1)) {
                "married" d(fnum) ": "
            }
            else { " married: " }
        }
        if (e) { long(e) "\n" }
        if (spouse) {         "      to  " fullname(spouse,0,1,80) "\n" }
    }
    if (e, death(person))   { "    died: " long(e) "\n" }
    if (e, burial(person))  { "  buried: " long(e) "\n" }
/*    fornotes(inode(person), note) {
        note "\n"
    }
    fornodes(inode(person),node) {
        if (not(strcmp(tag(node),"REFN"))) {
            "SOURCE: " value(node) "\n"
        }
    }
*/
}


/* locations_and_years figures out unique locations and min/max years
   for a person with respect to other persons already figured.
   Note that the events are processed in reverse chronological order,
   because the ancestry is traced bottom up.  Oh well...
 */

proc locations_and_years(person) {
    list(marriage_list)

    call one_location_and_year(burial(person))
    call one_location_and_year(death(person))
    families(person,family,spouse,fnum) {
        push(marriage_list,marriage(family))
    }
    while (event,pop(marriage_list)) { call one_location_and_year(event) }
    call one_location_and_year(baptism(person))
    call one_location_and_year(birth(person))
}


/* one_location_and_year appends the event location to the global
   location list if it is new, and figures the event year into the
   global min and max values.
 */

proc one_location_and_year(event) {
    if (event) {
        set(loc,place(event))
        if (strlen(loc)) {
            if (not(lookup(location_table,loc))) {
                requeue(location_list,save(loc))
                insert(location_table,save(loc),1)
            }
        }
        if (yr,atoi(year(event))) {
            if (lt(yr,year_min)) { set(year_min,yr) }
            if (gt(yr,year_max)) { set(year_max,yr) }
        }
    }
}

/*

soc.genealogy.surnames


This FAQ is presently in draft form. It may change without notice. Last
modification: 25 Aug 1995.

Other surname-related FAQs available here:

       Commonly Used Abbreviations
       Commonly Used German Abbreviations
       Examples of Queries: Good and Bad
       WWW Surname Archive


Welcome to soc.genealogy.surnames. The intent of this newsgroup is to
help genealogists researching related families to contact each
other. All surname queries are welcome here.

A surname query is in many ways like a classified ad in a
newspaper. You want to attract people who might be interested in
sharing information about your family to read and respond to your
post.  To help readers in finding articles of interest, writers are
requested to follow some simple guidelines in the subject lines of
their articles.

Articles in soc.genealogy.surnames fall into a few basic categories:

    1.general surname queries
    2.tiny tafels
    3.address changes
    4.follow-up articles
    5.Roots Surname List (RSL) articles
    6.moderators' announcements

1. General surname queries

(Style suggestions can be found in a companion FAQ, Surname Queries:
Good and Bad. This document discusses only what is absolutely
necessary for a post to soc.genealogy.surnames.)

General surname queries can be written in plain language, freely
formatted. The body of the article can include any information about a
family that you wish. We recommend including given names, spouses,
children, birth, death, and marriage dates and places, if you know
them. This will make your article more useful to people who might want
to contact you, as well as making it valuable to people who may find
your article in searching the surname archives later. Indicating what
additional information you are seeking is a good idea. Also include
how to contact you: e-mail address, snail-mail address, and phone
number, if you like.

Each surname query should have a subject line that gives one or more
surnames (in all capital letters), at least one place (using an
abbreviation from the RSL list of abbreviations), and an indication of
the time frame of interest. Examples:

  Subject: MILLS; NY,USA; 1800-1915
  Subject: MILLS Samuel D.; Williamsburg,Kings Co,NY,USA; 1796-1863
  Subject: ZAHM/PICARD/STEIS; LOT,FRA; 1680-1840
  Subject: ZAHM / PICARD / STEIS; LOT,FRA; 1680-1840
  Subject: ZAHM; LOT,FRA>IN,USA>IL,USA>KS,USA; 1650-
  Subject: ZAHM; LOT,FRA > IN,USA > KS,USA; 1650-
  Subject: CLOVER John; Lincoln,LIN,ENG>IL,USA; -1860
  Subject: LEGGETT; anywhere; anytime

The "anywhere anytime" indication should only be used by genealogists
who are making a comprehensive one-name study of everyone in the world
who bore that name in all of recorded history. If you are just
starting out in researching your family, do not use this form; please
read the first paragraph of this section on surname queries again, and
use one of the other examples. If you are making a comprehensive
world-wide collection, please tell us about the extent of your
database as a way to encourage people to share their research with
you.

If you find you want to include queries about more names than will fit
in the subject line, you may wish to post several queries.

Some of the abbreviations may seem unfamiliar at first. The advantage
of using standard abbreviations for place names is that it makes
searching the surname archives easier and more reliable. As an
example, consider searching the archives for Coles families in New
York. With standard abbreviations, you can look for subjects that
contain both "COLES" and "NY,USA" and be sure that you are finding all
the relevant archived queries. Without standardization, you'd have to
search for "NY" and "New York", and might miss articles that said
"Albany" or "Buffalo" but left out the state. Abbreviations also make
subject lines shorter, for writers whose software limits subject
length.

The place abbreviations are the same as used for the Roots Surname
List (RSL). They include United States and Canadian two-letter postal
codes, Chapman codes, three-letter ISO codes for nations, some other
standard codes, and a few codes invented for the RSL. The list was
compiled by Karen Isaacson with help from Christian Carey.

You may find more information on abbreviations that may be used in
soc.genealogy.surnames in the Commonly Used Abbreviations FAQ for
soc.genealogy.surnames. The complete list of codes is archived on
mail.eworld.com. To retrieve the file, send e-mail to
listserv@mail.eworld.com containing only the line:

  GET FAMILY ABBREV

The computer will then e-mail you the list of abbreviations (unless
you are using a system that blocks e-mail to and from listservs). You
can also retrieve the file by anonymous ftp to vm1.nodak.edu, in the
ROOTS-L directory; the file is named family.abbrev. The dates should
indicate when you are interested in the family in the area listed in
the subject line. The dates could be the earliest birth and latest
death dates for known ancestors, or periods for which you have
information, or the time for which you want more information. The date
range is approximate; no need to add "circa" or "?" if you are not
sure of dates.

*/
