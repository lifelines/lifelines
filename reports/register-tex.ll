/*
 * @progname       register-tex
 * @version        2.0
 * @author         Wetmore, Olsen
 * @category       
 * @output         LaTeX
 * @description    
**
** register-tex
**
** Version 2  24 Feb 1993
** Version 1     Nov 1992
**
** Requires LifeLines version 2.3.3 or later
**
** David Olsen (dko@cs.wisc.edu)
** based on work originally done by Tom Wetmore (ttw@cbnews1.att.com).
**
** This report prints, in book format, information about all descendants of a
** person and all of their spouses.  It tries to understand as many different
** GEDCOM tags as possible.  All source iformation (SOUR lines) is in the
** footnotes.
**
** The output is in LaTeX format.  Therefore, the name of the output file
** should end in ".tex".  To print (assuming the name of the output file is
** "out.tex"):
**      latex out  <ignore lots of warnings about underfull \hboxes>
**      dvips out
**      lpr out.ps
**
** Indexing commands are placed within the LaTeX output.  To include an index
** in the document do the following:
**      latex out
**      makeindex out           <not all systems have makeindex available>
**      <edit out.tex, add line \input{out.ind} just before \end{document}.>
**      latex out
**      dvips out
**      lpr out.ps
**
** I admit that this is lot of post-processing, but the results are worth it.
**
** LaTeX interprets $, &, %, #, _, {, }, ~, ^, and \ as special characters.
** If you have any of these characters in your database, you will need to
** convert them to \$, \&, \%, \#, \_, \{, \}, \verb|~|, \verb|^|, and
** $\backslash$ (or \verb|\|) respectively after the report is generated and
** before running through LaTeX.  $, &, _, ~, and ^ are easy to find, since
** they are not used in the report.  % and # are only used near the beginning.
** {, }, and \ will be more difficult to find since they are used as part of
** the LaTeX commands throughout the report.
*/


proc main ()
{
    getindi(indi)  /* Get the individual to start with */

    /* Print preamble.  Feel free to change this to suit your tastes. */
    "\\documentstyle[twocolumn]{article}\n"
    "\\pagestyle{myheadings}\n\n"
    "% Shrink the margins to use more of the page.\n"
    "% This is taken from fullpage.sty, which is on some systems.\n"
    "\\topmargin 0pt\n"
    "\\advance \\topmargin by -\\headheight\n"
    "\\advance \\topmargin by -\\headsep\n"
    "\\textheight 8.9in\n"
    "\\oddsidemargin 0pt\n"
    "\\evensidemargin \\oddsidemargin\n"
    "\\textwidth 6.5in\n\n"
    "\\newcounter{childnumber}\n\n"
    "% The \\noname command is needed because TeX doesnt like underscores.\n"
    "\\newcommand{\\noname}{\\underline{\\ \\ \\ \\ \\ }}\n\n"
    "% Environment for printing the list of children.\n"
    "\\newenvironment{childrenlist}"
        "{\\begin{small}\\begin{list}{\\sc\\roman{childnumber}.}"
        "{\\usecounter{childnumber}\\setlength{\\leftmargin}{0.5in}"
        "\\setlength{\\labelsep}{0.07in}\\setlength{\\labelwidth}{0.43in}}}"
        "{\\end{list}\\end{small}}\n\n"
    "% The following commands are used to create the index.\n"
    "\\newcommand{\\bold}[1]{{\\bf #1}}\n"
    "\\newcommand{\\bfit}[1]{{\\bf\\it #1}}\n"
    "\\newcommand{\\see}[2]{{\\it see #1}}\n\n"
    "% Command to use at the beginning of each new generation.\n"
    "\\newcommand{\\generation}[1]"
        "{\\newpage\\begin{center}{\\huge\\bf Generation #1}\\end{center}"
        "\\vspace{3ex}\\setcounter{footnote}{0}"
        "\\markright{Descendants of " fullname(indi, 0, 1, 40)
        "\\hfill Generation #1\\hfill\\ }}\n\n"
    "\\makeindex\n\n"
    "\\begin{document}\n\n"
    "\\title{Descendants of " fullname(indi, 0, 1, 40) "}\n"

    getstrmsg(author, "Enter the author(s) of this document:")
    "\\author{" author "}\n"
    "\\date{\\today}\n"
    "\\maketitle\n"

    getstrmsg(intro, "File that contains introduction (if any):")
    if (ne(strcmp(intro, ""), 0)) {
        "\\input{" intro "}\n"
    }


    list(ilist)    /* List of individuals */
    list(glist)    /* List of generation for each individual */
    table(stab)    /* Table of numbers for each individual */
    indiset(idex)

    enqueue(ilist, indi)
    enqueue(glist, 1)
    set(curgen, 0)
    set(out, 1)
    set(in, 2)

    while (indi, dequeue(ilist)) {

        set(thisgen, dequeue(glist))
        if (ne(curgen, thisgen)) {
            print("Generation ") print(d(thisgen)) print("\n")
            "\n\n\\generation{" d(thisgen) "}\n"
            set(curgen, thisgen)
        }

        print(d(out)) print(" ") print(name(indi)) print("\n")

        "\n\\vspace{3ex}\\ \\\\\\begin{center}{\\large\\bf " d(out) ".\\ "
        name(indi) "}\\end{center}\n"

        insert(stab, save(key(indi)), out)

        call longvitals(indi, 1, 2)

        addtoset(idex, indi, 0)
        set(out, add(out, 1))

        families(indi, fam, spouse, nfam) {
            "\n\n"
            if (eq(0, nchildren(fam))) {
                call texname(inode(indi), 0) "\\ and "
                if (spouse) {
                  call texname(inode(spouse), 0)
                } else {
                  "\\noname"
                }
                "\\ had no children.\n"
            } elsif (and(spouse, lookup(stab, key(spouse)))) {
                "Children of " call texname(inode(indi), 0) "\\ and "
                call texname(inode(spouse), 0) "\\ are shown under "
                call texname(inode(spouse), 0)
                "\\ (" d(lookup(stab, key(spouse))) ").\n"
            } else {
                "Children of " call texname(inode(indi), 0) "\\ and "
                if (spouse) {
                  call texname(inode(spouse), 0)
                } else {
                  "\\noname"
                }
                ":\n\\begin{childrenlist}\n"
                children(fam, child, nchl) {
                    set(haschild, 0)
                    families(child, cfam, cspou, ncf) {
                        if (ne(0, nchildren(cfam))) { set(haschild, 1) }
                   }
                   if (haschild) {
                        enqueue(ilist, child)
                        enqueue(glist, add(1, curgen))
                        "\n\\item[{\\bf " d(in) "}\\ \\hfill"
                        "\\addtocounter{childnumber}{1}"
                        "{\\sc\\roman{childnumber}}.]"
                        set (in, add (in, 1))
                        call shortvitals(child)
                    } else {
                        "\n\\item "
                        call longvitals(child, 0, 1)
                        addtoset(idex, child, 0)
                    }
                }
                "\\end{childrenlist}\n"
            }
        }
    }
    "\n\n\\end{document}\n"
}


/* shortvitals(indi):  Displays the short form of the vital statistics (birth
   and death only) of an individual. */

proc shortvitals(indi)
{
        call texname(inode(indi), 1)
        set(b, birth(indi))
        set(d, death(indi))
        if (and(b, long(b))) { ", b.\\ " long(b) }
        if (and(d, long(d))) { ", d.\\ " long(d) }
        "\n"
}


/* longvitals(i, name_parents, name_type)
   Prints out the complete vital statistics of the individual (i).  If
   name_parents is not 0, then the names of the parents of the individual will
   be printed.  The parameter name_type is passed to texname.  The GEDCOM tags
   are divided into ones that would likely occur before getting married and
   ones that would likely occur after getting married.  Within the two sets
   they are printed in the order in which they appear in the database.  I
   haven't yet figured out a convenient way of indicating the sex. */

proc longvitals(i, name_parents, name_type)
{
        call texname(inode(i), name_type) "." call print_sources(inode(i)) "\n"

        set(dad, father(i))
        set(mom, mother(i))
        if (and(name_parents, or(dad, mom))) {
                if (male(i))       { "Son of " }
                elsif (female(i)) { "Daughter of " }
                else               { "Child of " }
                if (dad)           { call texname(inode(dad), 0) }
                if (and(dad, mom)) { "\nand " }
                if (mom)           { call texname(inode(mom), 0) }
                ".\n"
        }

        set(name_found, 0)
        fornodes (inode(i), n) {
                if (eq(strcmp(tag(n), "ADOP"), 0)) {
                        call process_event(n, "Adopted")
                }
                if (eq(strcmp(tag(n), "BAPL"), 0)) {
                        call process_event(n, "Baptized")
                }
                if (eq(strcmp(tag(n), "BAPM"), 0)) {
                        call process_event(n, "Baptized")
                }
                if (eq(strcmp(tag(n), "BARM"), 0)) {
                        call process_event(n, "Bar mitzvah")
                }
                if (eq(strcmp(tag(n), "BASM"), 0)) {
                        call process_event(n, "Bat mitzvah")
                }
                if (eq(strcmp(tag(n), "BIRT"), 0)) {
                        call process_event(n, "Born")
                }
                if (eq(strcmp(tag(n), "BLES"), 0)) {
                        call process_event(n, "Blessed")
                }
                if (eq(strcmp(tag(n), "CAST"), 0)) {
                        "Caste: " call valuec(n) "."
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "CHR"), 0)) {
                        call process_event(n, "Christened")
                }
                if (eq(strcmp(tag(n), "CONF"), 0)) {
                        call process_event(n, "Confirmed")
                }
                if (eq(strcmp(tag(n), "CONL"), 0)) {
                        call process_event(n, "Confirmed")
                }
                if (eq(strcmp(tag(n), "GRAD"), 0)) {
                        call process_event(n, "Graduated")
                }
                if (eq(strcmp(tag(n), "NAME"), 0)) {
                    if (eq(name_found, 0)) {
                        set(name_found, 1)
                    } else {
                        "Also known as " call texname(n, 3) "."
                        call print_sources(n) "\n"
                    }
                }
                if (eq(strcmp(tag(n), "NAMR"), 0)) {
                        "Religious name: " call valuec(n) "."
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "NATI"), 0)) {
                        "Nationality: " call valuec(n) "."
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "ORDN"), 0)) {
                        call process_event(n, "Ordained")
                }
                if (eq(strcmp(tag(n), "RELI"), 0)) {
                        "Religious affiliation: " call valuec(n) "."
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "TITL"), 0)) {
                        "Title: " value(n) "."
                        call print_sources(n) "\n"
                }
        }
        if (eq(1, nfamilies(i))) {
                families(i, f, s, n) {
                        "Married" call print_sources(fnode(f))
                        call spousevitals(s, f)
                }
        } else {
                families(i, f, s, n) {
                        "Married " ord(n) "," call print_sources(fnode(f))
                        call spousevitals(s, f)
                }
        }
        fornodes (inode(i), n) {
                if (eq(strcmp(tag(n), "BURI"), 0)) {
                        call process_event(n, "Buried")
                }
                if (eq(strcmp(tag(n), "CENS"), 0)) {
                        call process_event(n, "Listed in census")
                }
                if (eq(strcmp(tag(n), "CHRA"), 0)) {
                        call process_event(n, "Christened (as an adult)")
                }
                if (eq(strcmp(tag(n), "DEAT"), 0)) {
                        call process_event(n, "Died")
                }
                /* One part of the GEDCOM standard says the tag should be DSCR,
                   another part says DESR. */
                if (eq(strcmp(tag(n), "DESR"), 0)) {
                        "Description: " call valuec(n)
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "EVEN"), 0)) {
                        call process_event(n, value(n))
                }
                if (eq(strcmp(tag(n), "NATU"), 0)) {
                        call process_event(n, "Naturalized")
                }
                if (eq(strcmp(tag(n), "OCCU"), 0)) {
                        "Occupation: " call valuec(n) "."
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "PROB"), 0)) {
                        call process_event(n, "Will probated")
                }
                if (eq(strcmp(tag(n), "PROP"), 0)) {
                        "Possessions: " call valuec(n) "."
                        call print_sources(n) "\n"
                }
                if (eq(strcmp(tag(n), "RETI"), 0)) {
                        call process_event(n, "Retired")
                }
                if (eq(strcmp(tag(n), "WILL"), 0)) {
                        call process_event(n, "Will dated")
                }
        }
        call print_notes(inode(i), "\n\n")
}


/* spousevitals (spouse, fam)
   Prints out information about a marriage (fam) and about a spouse in the
   marriage (spouse). */

proc spousevitals (spouse, fam)
{
  if (e, marriage(fam)) {
    call print_event(e) "," call print_sources(e) " "
  }
  "\n"
  if (spouse) {
    call texname(inode(spouse), 3)
    call print_sources(inode(spouse))
    set(bir, birth(spouse))
    set(chr, baptism(spouse))
    set(dea, death(spouse))
    set(bur, burial(spouse))
    set(dad, father(spouse))
    set(mom, mother(spouse))
    if (or(or(or(or(or(bir, chr), dea), bur), mom), dad)) {
      "\n("
      if (bir) {
        "born" call print_event(bir)
        if (or(or(or(dea, bur), mom), dad)) { "," }
        call print_sources(bir)
        if (or(or(or(dea, bur), mom), dad)) { "\n" }
      }
      if (and(chr, not(bir))) {
        "christened" call print_event(chr)
        if (or(or(or(dea, bur), mom), dad)) { "," }
        call print_sources(chr)
        if (or(or(or(dea, bur), mom), dad)) { "\n" }
      }
      if (dea) {
        "died" call print_event(dea)
        if (or(mom, dad)) { "," }
        call print_sources(dea)
        if (or(mom, dad)) { "\n" }
      }
      if (and(bur, not(dea))) {
        "buried" call print_event(bur)
        if (or(mom, dad)) { "," }
        call print_sources(bur)
        if (or(mom, dad)) { "\n" }
      }
      if (or(mom, dad)) {
        if    (male  (spouse)) { "son of " }
        elsif (female(spouse)) { "daughter of " }
        else                   { "child of " }
        if (dad)               { call texname(inode(dad), 3) }
        if (and(mom, dad))     { " and " }
        if (mom)               { call texname(inode(mom), 3) }
      }
      ")"
    }
  } else {
    "\\noname"
  }
  ".\n"
}


/* texname (i, type)
   Prints an individual's name in LaTeX format, with the surname in small caps.
   For example, "David Kenneth /Olsen/ Jr." would be printed as
   "David Kenneth {\sc Olsen} Jr.".  The type argument determines how the name
   will appear in the index.
        type = 0: no index
        type = 1: page number appears in bold
        type = 2: page number appears in bold-italics
        type = 3: page number appears in normal text
   The parameter i can be either an INDI node (NOT an individial) or a
   NAME node. */

proc texname (i, type)
{
        list(name_list)

        set(sname, "")
        extractnames(i, name_list, num_names, surname_no)
        forlist (name_list, nm, num) {
            if (eq(num, surname_no)) {
                if (eq(strcmp(nm, ""), 0)) {
                    " \\noname"
                    set(sname, "\\noname")
                } else {
                    " {\\sc " save(nm) "}"
                    set(sname, nm)
                }
            } else {
                " " nm
            }
        }
        if (gt(type, 0)) {
            "\\index{" sname
            if (gt(num_names, 1)) { "," }
            forlist (name_list, nm, num) {
                if (ne(num, surname_no)) {
                    " " nm
                }
            }
            if    (eq(type, 1)) { "|bold"}
            elsif (eq(type, 2)) { "|bfit"}
            "}"
        }
}


/* process_event (event_node, event_name)
   Prints information about a particular event (event_node, which is a GEDCOM
   node).  event_name is verb form of the text describing the event (such as
   "Born", "Died", etc.). */

proc process_event (event_node, event_name)
{
        event_name
        call print_event(event_node) "."
        call print_sources(event_node)
        call print_notes(event_node, " ") "\n"
}


/* print_event (event_node):  Prints the date and place of an event. */

proc print_event (event_node)
{
        if (date(event_node)) { " " date(event_node) }
        if (place(event_node)) { " at " place(event_node) }
}


/* print_notes (root, sep):  Prints all the notes (NOTE nodes) associated with
   the GEDCOM line root, separated by the given separator. */

proc print_notes (root, sep)
{
        fornotes (root, note) {
                sep note " "
        }
}


/* print_sources (root)
   Prints all sources (SOUR lines) associated with the given GEDCOM line.  The
   sources are formated as LaTeX footnotes.  This routine prints each SOUR line
   as a separate footnote, which is not correct.  This should be corrected so
   that all sources are combined into a single footnote. */

proc print_sources (root)
{
        fornodes (root, n) {
                if (eq(strcmp(tag(n), "SOUR"), 0)) {
                        "\\footnote{" call valuec(n) "}"
                }
        }
}


/* valuec(n):  Prints the value of a GEDCOM node and the values of any CONT
   lines associated with it. */

proc valuec(n)
{
        value(n)
        fornodes (n, n1) {
                if (eq(strcmp(tag(n1), "CONT"), 0)) {
                        "\n" value(n1)
                }
        }
}
