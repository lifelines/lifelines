/*
 * @progname       rslgen.ll
 * @version        1.1
 * @author         Eggert
 * @category       
 * @output         RSL format
 * @description    Generate a Roots Surname List (RSL) submission

rslgen - a LifeLines report program to aid in the generation of
Roots Surname List (RSL) submissions.

Given a person, this generates a RSL-like submission for that person and
his/her ancestors.  The output will likely need considerable hand editing,
but that is how it is.  If you need to know what the RSL is, I have enclosed
a description at the end of this file.

Here's what you will need to do by hand (you can consider this a list
of desired features for future versions of this report program):

BEFORE YOU RUN THE PROGRAM:

Change the routine write_rsl_header() to use your submitter tag, name,
and address.

AFTER YOU RUN THE PROGRAM:

Sort the surname portion of the output file.
Eliminate empty or useless surname lines.
Use RSL-standard abbreviations for placenames.
  - get FAMILY ABBREV as per instructions at end of this file.
Combine duplicate surname lines where appropriate.
Check check check.
Send the final product to the RSL maintainer.
  - see the end of this file.

Version 1, 14 July 1994, by Jim Eggert, eggertj@ll.mit.edu

*/

global(year_min)
global(year_max)
global(submitter_tag)
global(location)
global(last_location)
global(location_list)
global(location_table)
global(see_surname_table)
global(rsl_entry_count)

/* write_rsl_header sets the submitter tag and
   writes a little header for the RSL list maintainer
 */

proc write_rsl_header(person) {
    set(submitter_tag,"dummy_tag")

    "Roots Surname List (RSL) submission of " date(gettoday())
    " submitted by John Q. Public\n\n"
    submitter_tag
    col(12) "John Q. Public noname@nowhere.nohow\n"
    col(12) "1234 56th Street, Anytown AM 54321\n\n"
}


proc main() {
    list(ilist)
    table(see_surname_table)

    getindi(person)

    call write_rsl_header(person)
    set(rsl_entry_count,0)

    enqueue(ilist,person)
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
                    this_surname " - see " final_surname
                    " (" submitter_tag ")\n"
                    insert(see_surname_table,
                                save(this_surname),save(other_surname))
                }
            }
        }
        call write_rsl_entry(final_key)
    }
    if (gt(rsl_entry_count,100)) {
        print("Warning:  Number of lines produced (")
        print(d(rsl_entry_count))
        print(")\nexceeds recommended submission limit (100)\n")
    }
}


/* write_rsl_entry writes one line in the rsl submission */

proc write_rsl_entry(key_entry) {
    if (key_entry) {
        set(surname_entry,save(surname(indi(key_entry))))
        if (strlen(surname_entry)) {
            if (strcmp(trim(surname_entry,1),"_")) {
    set(rsl_entry_count,add(rsl_entry_count,1))
    set(not_first,0)
    surname_entry col(14) d(year_min)
    col(20) d(year_max) " "
    while (loc,dequeue(location_list)) {
        if (not_first) { ">" } else { set(not_first,1) }
        loc
    }
                " " submitter_tag " " key_entry "\n"
            }
        }
    }
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
                        ###   WHAT IS IT?   ###

The Roots Surname List (RSL) helps genealogical researchers share and
compare data.  Genealogists with Internet access are welcome to submit
surnames that they are researching for inclusion in the Roots Surname
List if they are willing to share their data with others who may be
doing parallel research.  Enough information should be provided so that
others can decide whether a link with their own lines is possible or
probable.  The assumption is that you have SOME data to share.  You
needn't be on the verge of writing the definitive genealogy for the
family in question.

If you see a surname listed that interests you, contact the person who
submitted the surname.  To do that, just look up their nametag (listed
at the end of each surname entry) in the list of submitters.  The
FAMILY INDEX, described below, has instructions on how to obtain the
list of submitters.  Unless the submitter happened to be me, I won't be
of much help.

The =update= to the RSL is posted to ROOTS-L (the Internet genealogy mailing
list) and to soc.roots (the USENET genealogy newsgroup)
on the first Sunday of the month.  At the same time, the entire
new RSL is stored on the listserver.  See below for instructions on
obtaining a copy from the listserver or via mail if you don't have access
to the listserver.  The update and sometimes the full RSL also propagates
after that to the genealogy libraries on CompuServe, GEnie, and Delphi.
Included in the posted update is contact information for the submitters of
the new and updated info.

                 ###   OBTAINING THE ENTIRE LIST   ###


To obtain a copy of the full Roots Surname List (RSL) from the listserver,
the first step is to send e-mail to LISTSERV@vm1.nodak.edu or
LISTSERV@NDSUVM1, with the one-line message (not in the subject line):

        GET FAMILY INDEX

You will receive by e-mail a list of the names of the various files
comprising the current RSL.  The files named in FAMILY INDEX may change
each month, so be sure to use a current one!  The files listed there may
be obtained in the same manner as you obtained FAMILY INDEX. (That is, by
sending e-mail to the LISTSERV address with the GET message.) If you have
access to FTP, you can instead use anonymous FTP to vm1.nodak.edu
(134.129.111.1) and do "cd ROOTS-L" then "get FAMILY.INDEX" to retrieve
the file.  Don't FTP it in binary mode, but instead in text mode.

If you are unable to retrieve the RSL via e-mail or via FTP, you can
receive a copy by sending $5 to:
        Brian Leverich
        27991 Caraway Lane
        Saugus, CA 91350
Requests MUST include a description of the computer you use (DOS or Mac)
and the highest capacity diskette you can read (360kb, 720kb, 1.2mb,
1.44mb, etc.).  If you need something other than DOS or Mac, inquire
first: if Brian can't handle your particular format, maybe someone else
here can.

                   ###   SUBMITTING NEW ENTRIES   ###

Please READ THIS SECTION ***CAREFULLY*** BEFORE SUBMITTING.  I receive
submissions and handle correspondence about the RSL over long distance
telephone lines, and I cannot afford the time and money wasted by
improperly formatted or otherwise inappropriate submissions.

 ** BASIC GUIDELINES **

Send new entries to me at one of the addresses listed at the end of this
note.  Entries received will be included in the next list.  See below
for format information.  PLEASE follow these guidelines:

        o Send ordinary text files.  Please do not compress, zip,
          uuencode, or MIME encode your file.

        o Be sure to submit "how to reach you" information as well as
          surnames.

        o Do not submit more than 100 surnames.

        o Do =not= put your surnames in CAPS.

        o Follow the formatting rules below with religious fervor.

        o AND NO TINY TAFELS.  They don't conform to the RSL format, and
          they don't contain the sort of information needed for the RSL.
          If you have a Tiny Tafel and want to put it to good use, I
          believe Brian Mavrogeorge, will enter it in the Fidonet TMS
          (Tafel Matching System) database if you post it to soc.roots
          or Roots-L.  (If you don't know what a Tiny Tafel is, you're
          probably in no danger of sending me one accidentally.)

 ** FORMATTING YOUR "HOW TO REACH YOU" INFORMATION **

For each submitter, I must receive one or two lines of address
information which tell readers how they can reach the person who
submitted the entry.  The format is fairly flexible, but must include a
short nametag (less than eight characters, all lower case) and should
typically include all your e-mail and postal addresses.  (If you're
nervous about broadcasting your postal address to the universe, though,
feel free to leave it off.)  If the nametag you select has already been
taken, I'll conjure up a new one for you.  Or feel free to suggest
alternates at the time of your submission.

The lines for karen (me) are:
karen    Karen Isaacson, karen@rand.org, Prodigy:  XRBV26B, GEnie:  KRENA
         27991 Caraway Lane, Saugus, CA 91350              Delphi:  KRENA

 ** FORMATTING YOUR SURNAME INFORMATION **

What should the surname entries look like?  Don't worry too much about
format -- I end up reformatting them anyhow for my sort routine.  But
please do conform to the general guidelines below.  Also, despite all
genealogical advice to the contrary, DO NOT put the surnames all in
capital letters.  The RSL does not use surnames in all CAPs.  (Don't
put them all in lower case, either, though, just do them like in the
example below.)

Each entry should be on one line, and consists of five parts:
1. The name of the family.
2. The earliest date for which you have information about the family.
   (For instance, the birthdate of the founder of the family, or the year
   he or she first showed up in the records.)
3. The latest date for which you have information about the family.  (When
   the last person with that surname died or skipped town, for instance.
   Or "now" if you know people of this surname that are still around --
   yourself, for example.  It's up to you whether a woman is considered
   under her maiden surname, married surname, or both.)
4. The migration of the family.  For instance, if my ancestors started out
   in Virginia, moved to Kentucky, then on to Missouri, this would be
   VA>KY>MO,USA.  If the surname was common, and I still had room (remember,
   all four fields should fit on one line), then I might add some county
   information to further distinguish them: OrangeCo,VA>KY>GentryCo,MO,USA.
   There is a list of most of the abbreviations that are in use.  It is in
   a file called FAMILY ABBREV, and can be obtained in the same manner as
   FAMILY INDEX, discussed above.  Or just spell out the location, and I'll
   put in the proper abbreviation, if any.
5. The nametag of the submitter.  This is so you can be found in the address
   list.  See discussion above for how to select one.

The Roots Surname Index is rather oddly computerized.  There aren't any
firm restrictions on the presentation of the data, but do try to use
something like the format suggested above and illustrated below.

Here are a few (genuine!) sample entries (my own, funny thing):

 Bell         1780  1940 OrangeCo,VA>KY>GentryCo,MO,USA karen
 Carr - see Kerr (karen)
 Keithley    c1750  1923 DEU>PA?>RowanCo,NC>BathCo,KY>FloydCo,IN,USA karen
 Keithley    c1750  1923 DEU>PA?>RowanCo,NC>KY>StCharlesCo,MO,USA karen
 Kerr         1760   now HuntingdonCo,PA>VenangoCo,PA>IA,USA karen
 Kicheli - see Keithley (karen)

 ** WHEN TO SUBMIT **

Try to get your additions or modifications in to me by the Thursday before
the first Sunday of each month, when the monthly update is published.
If you miss a deadline, not to worry: your surnames will have arrived in
time for the next deadline and will be included in the next month's list.

 ** WHERE TO SUBMIT **

How can I be reached?  At one of the following addresses:

        Internet:        karen@rand.org         <- preferred
                         krena@genie.geis.com
                         xrbv26b@prodigy.com
                         bi254@cleveland.freenet.edu
                         kisaacson@mcimail.com
        GEnie or Delphi: KRENA
        Prodigy:         XRBV26B
        Postal:          Karen Isaacson
                         27991 Caraway Lane
                         Saugus, CA  91350


*/
