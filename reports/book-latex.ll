/*
** @progname       book-latex.ll
** @author         Nicklaus
** @version        3.1
** @category       
** @output         LaTeX
** @description    
**
** Generates really spiffy register reports for formatting with LaTex. Reports
** read like a book. Includes source citation, footnotes, etc. Register
** reports are either descendant or ancestor/ahnentafel style.

** SourceForge Versions:
**
** Revision 1.14  2005/11/19 05:30:30  memmerto
** - Add missing </PARA> tags, as per SF Patch 551968.
** - Add "usepackage{isolatin1}" to register-tex.ll, as per SF Patch # 402021.
** - Add comments about how to change between A4/Letter paper.
** - Add -v (version) option; clean up -h/-? (usage) options, as per SF Feature Request # 1310390.
** - Add "temp" and "random" hints to more file operations.
**
** Revision 1.13  2005/10/26 04:40:45  dr_doom
** make user property usage more consistent
**
** Revision 1.12  2005/06/18 00:29:39  dr_doom
** Update formatted docs, minor cleanup in merge families
**
** Revision 1.11  2004/09/02 23:11:04  rsimms
**         Enhanced book-latex.ll to filter text to render harmless those
**         characters meaningful to the LaTeX system.  Also added an option
**         to suppress e-mail addresses of source authors when they're specified
**         as EMAI subnodes of AUTH nodes.  Also indented all of the code with
**         spaces (instead of tabs/spaces mix) to make the code easier to follow.
**
** Revision 1.10 2004/07/19 05:54:54 dr_doom
** Merge Vincent Broman Changes to reports
**
** Revision 1.9  2003/01/19 02:50:23  dr_doom
**
** Revision 1.8  2001/10/03 02:58:55  dabright
**  Restored some previous additions to this report.
**  The update from Dennis Nicklaus on 12 Aug 2001
**  had deleted them and he asked me to re-add them. So:
**  Add  CREM (cremated) tag processing;
**  modified OCCU tag processing so that it can recognize date ranges
**  (and so avoid saying "xx is a yy and a zz and a ..."); modified
**  OCCU tag processing to recognize a subordinate AGNC tag indicating
**  employer; modified onDate to recognize date ranges (FROM dd mmm
**  yyyy TO dd mmm yyyy) - this still has some rough edges.
**
** Revision 1.7  2001/08/12 20:53:59  nozell
** Update by Dennis Nicklaus to his book-latex.ll
**
** Revision 1.5  2000/11/28 21:39:45  nozell
** Add keyword tags to all reports
** Extend the report script menu to display script output format
**
** Revision 1.4  2000/11/11 07:52:06  pere
** Use ISO 8859/1 charset in LaTeX.  Add meta-information in header.
**
** Revision 1.3  2000/11/11 07:46:47  pere
** Include index even when there is no bibliography.
**
** Revision 1.2  2000/11/11 04:07:37  dabright
**
** reports/book-latex.ll: Added processing for BAPM tag, corrected
** error in referencing "spouse" rather than "s" in longvitals,
** added processing for the TYPE tag (modifier for EVENT), corrected
** setDayNumber so that it only uses text phrases (e.g., "on the same
** day") when both previous date and current date are fully
** specified, preserve line breaks represented by blank CONT/CONC
** tags, ensure "cn" variable in sourceIt is initialized before
** referenced, and miscellaneous typographical corrections.
**
**
** Revision 1.1  2000/11/10 dabright
** Initial revision - copy of Version 2.5 from Dennis Nicklaus
**
** Pre-SourceForge history:
**
## Dennis Nicklaus (dnicklaus at yahoo.com)
** Version 2.5     Feb. 2000
**
** Requires LifeLines version 2.3.3 or later
** Requires tree.tex (for formatting)   (tex macros for tree drawing).
**    (found in TUGboat, vol. 6 (1985) and online in various places,
**    including with the desc-tex lifelines report)
**
** based on work by David Olsen (dko@cs.wisc.edu) which in turn was
** based on work originally done by Tom Wetmore (ttw@cbnews1.att.com).
** also work by Kurt Baudendistel (baud@research.att.com).
** A few others, such as Ed Sternin (edik@brocku.ca) made other suggestions.
** and corrections.
**
** This report prints, in register/book format, information about all 
** descendants of a person (or persons) and all of their spouses.  
** It tries to understand as many different
** GEDCOM tags as possible.  All source information (SOUR lines) is in the
** bibliography and footnotes.
**
** An alternate usage (new in version 2) lets you print out sort of a
** combination ahnentafel and register report going through the ancestors of
** the persons chosen.
**
** The output is in LaTeX format.  Therefore, the name of the output file
** should end in ".tex".  To print (assuming the name of the output file is
** "out.tex"):
**      latex out      < ignore lots of warnings about underfull \hboxes >
**      makeindex out  < not all systems have makeindex available
**                       if yours is one, just remove the \input{file.ind} line
**                       from the LaTeX output and skip the 'makeindex'>
**      latex out      < repeat latex-ing to get cross-references resolved>
**      latex out      < needed to get the index into the TOC>
**                     < you may need to repeat more if Latex
**                       says so, e.g. if page refs change. >
**      dvips -o out   < without the -o, dvips will likely print to your
**                     < default printer instead of creating a .ps file.
**      lpr out.ps
**                     < the last three commands here may be replaced by >
**                       pdflatex out    -- if you have 'pdflatex' and a PDF is
**                       the desired final product >
**
** I admit that this is lot of post-processing, but the results are worth it.
**
** NOTE ON PAPER SIZES:
** Paper sizes (A4 or letter) can be specified within the LaTeX output,
** but this requires editing by folks who don't like the default.
**
** Since dvips (a neccessary processing step) can take a paper-size
** argument on the command line, it's much simpler to let the user
** specify the desired page size when running dvips (outlined above)
** instead of editing the report/LaTeX output.
**
** Example:
**   dvips -t letter out [ for US Letter-sized paper, 8.5x11" ]
**   dvips -t a4 out [ for ISO/European A4-sized paper, 8.3x11.7" ]
**
** A special note about indexing.  If you have names with double quotes in
** them, e.g. Forrest "Foggy" Morrison, not the nice Latex quotes style:
** Forrest ``Foggy'' Morrison, then the " marks will screw up the index.

  If you'd like to credit me & this program in your
  introduction if it's something you're really going to publish
  that'd be nice (but not required).
  Something like this could be used:
     "This document was prepared using LifeLines v." version() " genealogical database program\n"
     "by Thomas T.~Wetmore~IV, {\\tt ttw@beltway.att.com}. The script {\\tt book-latex}\n"
     "by Dennis Nicklaus {\\tt dnicklaus at yahoo.com} was used to generate the \\LaTeX\\ code.\n"

*/
/* WHAT DENNIS NICKLAUS DID:
  I expanded this program greatly, mostly based on a "book" report
  done by Kurt Baudendistel (baud@research.att.com).
  I combined what I liked best about register-tex and the book report.

  numbering:
    Register-tex had modified register numbering, where book
    had no numbering, and just always referred to people by
    page number, so I took the mod. reg. numbering.
  sources/bibliography:
    book had really nice SOUR support, so I took that
    and modified it a little bit, so that it supports
    a more std. gedcom usage of the SOUR definitions
    (according to my reading of the std).
  nothing needed in database:
    book required that you have various things like CHAP
    and PART additions to your lifelines gedcom database
    in order to find who to include in the book.
    I didn't include that
  multiple-person selection:
    On the other hand, maybe you want to include the
    descendants of more than one person. I included the
    ability (which was sort of there in book) of specifying
    multiple people.  For Instance, you might specify your
    maternal grandfather and your paternal grandfather to
    get all your first cousins on both sides into the same
    book.  This report asks you to keep selecting as many
    people as you want.  It does the complete descendancy
    for each person selected.  Each person so selected
    starts a new chapter in the book.  I make up a title
    of the book based on the surnames of indi's chosen.
    When you don't want to select any more people, just
    hit return at the "select indi" prompt.
  english sentences:
    The book report was very good at automatically making
    real sentences instead of just fragments. I used that.
    I try to make compound sentences using "and" whenever
    possible, and this makes for a lot of rules in the
    code to try to handle a lot of cases. I probably missed a 
    few where the English will still come out poorly.
  placename smarts:
    Also from book is the ability to recognize a place which
    is used multiple times. For instance, the first time
    it sees Keswick,Keokuk Co.,Iowa, it'll print the whole
    thing.  But then every subsequent time, it'll just print
    Keswick.  Makes things VERY readable, but it can leave
    some ambiguous things, like if you have two Keswick's,
    it might be hard to figure out which is meant.
    Likewise, I picked up ADDR support from book.
    if you have: 
      2 PLAC thattown
      3 ADDR thisplace
    It'll say "at thisplace in thattown".  Simply having
    2 PLAC results in "in thattown".
    One thing I did change from book was making sure 
    it always says "in thattown" after saying "at thisplace".
    Book was happy just saying "at thisplace" and assuming
    you know what it meant.
    But if you have several of
      2 PLAC Town,County,State
      3 ADDR his home
    Then just saying "at his home" doesn't do much, so I made
    sure it always says at least "at his home in Town".
    (the first time, it'd say, "at his home in Town,County, State".)
    (also useful if you have lots of different St. Mary's, e.g.)
    I also made it watch for words like "near, north,..."
    for the "town" part of the place, so that it doesn't say
    something icky like "in near Mytown".
  many events:
    register-tex supported a lot more GEDCOM fields, so 
    I tried to include all of them.
    But I personally don't use all of them, so some may look ugly.
  charts: I added a feature that makes it draw 3 gen desc. tree charts
    for any indi who heads up a chapter.  I took code
    from desc-tex to do this. (and modified it slightly
    because it didn't work for some cases).
    So you need the tree.tex macros, which this will
    try to include.
    Note:  I modified the tree.tex macro a little bit
    to scrunch things up because I have some ancestors
    with huge families, and all their 10 kids had big families,
    so I had to scrunch the spacing to fit 3 gen. onto one page.

    I dropped most of the pedigree chart capabilities in book,
    but I did add one thing to look for the something like:
      1 NOTE BOOKPEDIGREE
    on each indi. If a note like that is found for an indi,
    that indi's pedigree (8 gen, I think) will be printed
    as a latex figure in the book.  I find this useful,
    for instance, when I do a book of all the desc. of
    all the grandparents of my grandmother.  I have
    a BOOKPEDIGREE note on my grandmother so you can see
    how all these lines fit together.

    If you have
      1 NOTE BOOKDESCENDENT 
    on an individual, it'll draw a 3-gen desc. chart for that person.


  excursions: These are for when you are following one family
    down, say SMITH, and one of the SMITHs marries a JONES.
    If you want to include the JONES ancestry in this book,
    but don't really want to follow every JONES descendant,
    then if you put a note on the JONES person:
      1 NOTE BOOKEXCURSION
    That causes this report to wait until the end of the
    chapter, then make a subsection which goes to the
    farthest male JONES ancestor, and document
    the direct line between that JONES ancestor and the
    JONES who married the SMITH.  It doesn't follow
    every JONES line down, but includes info about each
    child of each direct JONES ancestor.

  intro
    I include the possibility that you might want to put
    your own introduction before most of the book.
    You can input your own intro file if you want.
    It should contain all the Latex directives you want
    also, such as \chapter{Introduction}.

  grandchild divisions
    Starting with the 4th generation down from each chapter head,
    I group together sets of grandchildren with over and
    under braces. (It doesn't make any sense to do it for
    generations 1-3 because they'd all be in the same group.)
    The idea is that if patriarch has kids A,B,C...
    Then by generation 4, you'll get a grouping of A's grandkids
    followed by B's grandkids, followed by C's grandkids.
    Lots of intermarrying might occasionally confuse the 
    code which does this.

  Chapter splits
    If you put a
      1 NOTE BOOKCHAPSPLIT
    on the person who is a chapter head, 
    then each of his children will head up their own chapters with
    the children of that head as Generation 1 in their respective chapters.
    You should be careful not to have a BOOKCHAPSPLIT on anyone who
    is not the head of a line, or it'll probably come out ugly.

    Okay, now that you know how to produce a report, here are the formatting
    conventions you must follow to get a good one. All records shown here are 
    optional, and all other records are okay -- they'll just be ignored by 
    the book report:

     1 NAME - Multiple name records allowed. First is ``true name.''
            - Later ones, with given but no surname, are ``nicknames''
              or familiars. 
            - Later ones, with surname but no given, are aliases or
              alternate spellings.
            - Later ones, with both surname and given, are aliases.
            - Post-titles, such as MD, should be included in this name.
     2 SOUR ... - Source for name.
     1 TITL ... - Pre-titles, such as Reverend. (but I don't call people
                  by their titl much. I mean after all, a person
                  isn't Captain John yet when they are born.
                  So I currently ignore this.)
     1 SEX  ...
     1 SOUR ... - Source for parentage if no BIRT or CHR is given. This 
                  produces better output than BIRT-SOUR records with no
                  DATE or PLAC given.
                  I also use this when I have a general source
                  which tells me everything about the person and
                  I don't want to mess things up by citing it
                  separately 8 times for birth, death, marr,...
     1 evnt ... - BIRT, CHR, DEAT, BURI, CREM, MARR, DIV, DIVF, or ANUL,...
     2 DATE ... - Date should be of format
                  [ABT|BEF|AFT|BET] [day] [JAN|...|DEC] [year] [-year for BET]
     2 PLAC ... - Comma separated list of localities appropriate for the
                  expression ``in ...''.
     3 ADDR ... - Location appropriate for the expression ``at ... in ...''.
     3 CEME ... - Location appropriate for the expression ``at ... in ...''.
     2 AGE  ... - Age appropriate for the expression ``at age ...''.
     2 CAUS ... - Cause of death appropriate for the expression
                  ``died of ...''.
     2 SOUR ... - Source for event.
     2 NOTE ... - Text to be inserted in book following technical details
                  of the event. (I use this instead of TEXT)
     3 SOUR ... - Source for text.
     1 OCCU ... - Description (title) of an occupation (job).
     2 AGNC ... - Employer (produces "worked [or became] a <OCCU> with <AGNC>").
     1 TEXT ... - Text to be inserted in book about the person.
                  I toyed with putting this before the death info,
                  but decided I like having all the vital stuff first,
                  then the more interesting text stuff.
     2 SOUR ... - Source for text.
     1 NCHI ... - Number of children (family records only).

     n CONT ... - Appropriate for TEXT and SOUR.

    You have the option of selecting either 1 TEXT, 1 NOTE or (not and)
    only those 1 NOTE records which start with an ! (exclamation)
    to include as the main body of text for each individual.

    From the GEDCOM std, TEXT "contains information from the source document."
    One might argue that I'm misusing it here.  But it depends on what
    kinds of things you use 1 TEXT for. It might be nicely readable and
    technically appropriate if, for example, you copy a bio. of someone from
    an old book and want it included in your printout.  Also, in sort of
    a self-referential way, you're including text from the book you
    publish with this. :-)

    I personally don't print out the "1 NOTE" records because I personally
    (and feel it is common that most people) have a lot of garbage in their
    notes, either general reminders to themselves, or PAF-style source notes
    (e.g. 1 NOTE BIRTH-DEATH: whatever source)
    However, with version 2.2, I give the option to print out either
    all 1 TEXT, all 1 NOTE, or all 1 NOTE !-tagged notes. (where if the
    1st char of the note is !, then the 1 NOTE gets included).

    Something that I consider a typical 2 NOTE usage might be:
      1 BURI
        2 DATE when
        2 PLAC somewhere
        2 NOTE (with her parents)
    But you do have to be a little aware of what this report is going
    to generate if you want to make it a grammatically correct sentence

    

  SOURCE records are complicated, but they produce great output. 

    To document a fact with a simple footnote, use

       n SOUR ...
      +1 CONT ...

    This can get more complicated (all records optional):
    (actually, I'm not sure what all is supported for footnotes
    any more, I mostly use bibliography entries.)

       n SOUR ...
      +1 CONT ...
      +1 PAGE ...
      +1 VOLU ...
      +1 NOTE ...
      +2 CONT ...
      +1 SOUR @id@

    The +1 SOUR @id@ produces a citation to an entry in the bibliography, 
    not a footnote, that is attached to the text of the footnote.  To get 
    a citation in the text itself, use

       n SOUR
      +1 SOUR @id@

    or the more simple

       n SOUR @id@

    To create a bibliographic entry that can be referenced as a citation,
    include a cross-reference definition. This can be included at the point
    where a citation is wanted. However, since you will like re-use the
    same citation many times and you'll want to be consist, define the
    cross-reference definition separately.

    A cross-reference definition takes this form (start at level 0)

      n @id@ SOUR   - no text allowed here
     +1 AUTH ...    - author of source
       +2 EMAI ...  - e-mail address of author (useful for fellow genealogists)
                      there is now an option to block e-mail addresses from
                      showing in the output (in their place is a mention that
                      one is on file)
       +2 ADDR ...  - postal address of author
     +1 TITL ...    - title of article or book
     +1 PUBL        - publishing info record
   note that all these +2 things have to be under it for a bib. entry.
   (rules are diff. for footnotes).
   This is the way I read the 5.3 GEDCOM std.
     +2 NAME ...   - name of publication,e.g. journal
     +2 PUBR ...   - publisher name
     +2 ADDR ...   - address
     +2 PHON ...   - phone number
     +2 DATE ...   - date of pub.
     +2 VOLU ...   - volume or list or range
     +2 NUM  ...   - number or list or range

     +1 FILM ...   - LDS film number
     +1 FICH ...   - LDS fiche number
     +1 PAGE ...   - page or list or range
     +1 REPO ...   - library name
     +1 NOTE ...   - free form text
     +2 CONT ...
     +1 SOUR @id@  - cite another source from bibliographic entry
     +1 TEXT ...   - free form text to print
     +2 CONT ...

    Note that the id can be most text that begins with an alphanumeric 
    character -- check your gedcom spec! Using a descriptive name of 
    the source that it represents, such as kurts-death-record or 0996198 
    (for film numbers) is a good idea.
    (but Lifelines eats the nice names when it reads them in)

SEMI-BROKEN STUFF

  A few problems you might notice:
  I'm not really happy with the desc. trees in several other cases,
  such as multiple spouses, and had to fix desctex a little bit
  to make it work better.  It still isn't perfect, I don't think.
  
  The sentence structure may come out badly in some odd cases I haven't
  encountered/tested yet.  It's been a real pain to get it as
  far as it is, and I'm still not happy with it.  (It gets complicated
  because I try to make compound sentences and use pronouns so
  it isn't so choppy.)

  The "test for common grandparents" thing was just the simplest
  way of doing it I could think of. Not perfect, but works 99%.

  I really want to put some smarts in so placename ambiguity is 
  less of a problem. For instance to distinguish the town of 
  Washington, Iowa from the state of Washington.

  My LaTex is sort of rusty, and some things aren't really properly
  done, possibly. (Like the spacing on the overbraces designed to
  fill one column.) Also, it's line-filling is pretty weird sometimes,
  such as just putting one or two words in the first line of a 
  person's description. I don't understand why it does that.

FOR VERSION 2:  
  I also now support a book which goes through an individual's
  ancestors in ascending order.  Numbering is ahnentafel style.
  A new chapter for each generation.

FOR VERSION 2.2
  (Several of these were suggested by Ed Sternin (edik@brocku.ca)

   a.  Fixed problem where in the descendant trees if a spouse's 
       last name isn't known.  It now puts in escaped underscores.
       (Also fixed printfirstname [now printablefirstname] to print
       underscores when no given name is known.)

   b.  Added the BOOKCHAPSPLIT option 
       If you put a 
       1 NOTE BOOKCHAPSPLIT on the person who is a chapter head, 
       then each of his children will head up their own chapters with
       the children of that head as Generation 1 in their respective chapters.
       I added this because I have a family where the father had 3 sons.
       I want to include the info on the father, but I also want each
       of the three sons to have their own chapter.  So I put a 
       BOOKCHAPSPLIT note on the father, and it all comes out automatically.
       You should be careful not to have a BOOKCHAPSPLIT on anyone who
       is not the head of a line, or it'll probably come out ugly.
       (It causes subsequently members of the generation of the noted 
       person to have separate chapters, as well as the children of 
       the noted person.)

   c.  I give the option to print out either all 1 TEXT, all 1 NOTE, or all
       1 NOTE !-tagged notes (where if the 1st char of the note is !, then the
       1 NOTE gets included).

   d.  Fixed mistake with running headers in ancestor format books so the
       running header is now "<name> Ancestors", not "Descendents".

   e.  Added option (query turned off by default to match old way since it is
       confusing to a beginner) to reset the placenametable at each generation.
       If this option is selected, then the effect is that any placename
       will have the fully specified (long) name printed out once in each 
       generation. I find this useful sometimes, just to remind people
       where you're talking about since it might have been a long time
       since the place was introduced.

   f.  if there is a 1 Event, 2 DATE or 2 PLAC record where there is 
       no value on the 2 DATE or 2 PLAC (but the record is present),
       it'll no fill in an underscore where the date/place would be, e.g.,
         Joe was born in ____ in ____ .
       Previously if the 2 DATE or 2 PLAC were present and empty, it 
       would have said "born in in ." which isn't so nice.

   g.  Changed from supporting 3 SITE records to 3 ADDR records because
       SITE isn't supposed to be part of std. GEDCOM anymore, ADDR is.
       I hope this is as easy of a global replace for you as
       it was for me. (of a gedcom file in a text editor).

   h.  Fixed A back-reference number (explaining that children were shown
       earlier) in ancestor mode.

   i.  Fixed problem in ancestor mode that would print out the long text for
       some individuals more than once.
  
  New For Version 2.3
  Not much is new:
    Don't do excursions in ancestor mode
    Added a couple LaTex macros that I use for pictures.

  New For Version 2.4
  Some contributions by Dave Steiner (steiner@bakerst.rutgers.edu)
    Some very minor typesetting fixups to make some spacing more
    consistent. Some  spaces added in certain parts, spaces taken
    out of other places.
    Says a couple "have no children" instead of "had no children" if
    it looks like the couple are still married and living.
    The bibliography filename will now be <outfile>-bib.tex
    instead of <database>-bib.tex.

  Supports CONC as well as CONT continuation lines in most places.

  I've made a couple other similarly minor cosmetic changes,
  one of which forces a new paragraph during excursions in an oddball case.

  Made a change in ancestor mode so that if cousins marry, it won't
  print the information about the common ancestors twice.

  Also an ancestor mode change so that ref. numbers for a person's
  parents appear in the text in the longvitals() description.

  New For Version 2.5.
  Fixed a mistake in check_print_divinfo which made it not work at all.

  New For Version 3.0
  Several miscellaneous fixes.
  Now part of lifelines sourceforge distribution. See sourceforge history.
  Uses documentclass instead of documentstyle
  Added ability to print a limited number of generations in
  descendant-style books. At the terminal generation, if a person
  is not dead, it will only print the birth year, not full date.
  Fixed the titlepage generation.
  Some fixes to support latest version of lifelines (or maybe its
  just differences for lifelines under solaris):
     Intersect() can't be used inside an if() call.
         It has to call intersect separately and use the result in a set()
     The childnum argument of children() is now only valid
         within the scope of the children() loop.
*/

/* Stuff I haven't implemented:
   adler@math.toronto.edu (Jeffrey D. Adler)
     One suggestion came from "Denis B. Roegel" <Denis.Roegel@loria.fr>
  regarding the over/under braces.
  He suggested using something like:
  \begin{minipage}{\columnwidth}
  $\overbrace{\hspace*{3in}}$
  \vspace{3ex}
  \begin{center}{\large\bf 24\ Henry REGLE}\end{center}
  \end{minipage}
  \nobreak
  Henry {\sc REGLE}\index{REGLE, Henry|bfit}, ...
  and For \underbrace, try to put a \nobreak before:
  \nobreak
  $\underbrace{\hspace*{3in}}$

  I haven't tried this out to see if I like it better.
  But I did include some of the \nobreaks before the underbrace
  and after the big center-ed person's name.
*/

global(maxgenprint)      /* number of generations to print full info for.*/
global(atmax_generation) /* used so we don't print full birthdates of living
                            individuals at the MAX-th generation */
global(ancestormode)
global(notes_text_mode)
global(eventPlaceTable)
global(atAddrValue)
global(eventNameTable)
global(in)
global(out)
global(idex)
global(stab)
global(powValue)
global(namereturn)
global(excurlist)
global(sourceList)
global(bibList)
global(bibTable)
global(figureCiteList)
global(figureNodeList)
global(gotValue)
global(gottenNode)
global(gottenValue)
global(dayNumber)
global(previousDayNumber)
global(daysToMonthList)
global(not_married_flag)
global(dumpplacetable_each_gen)
global(hadsplitnote)
global(force_desc_chart)
global(pedigreeFigureLabel)
global(global_dead)/* used at the MAX-th generation so we know if indi is dead*/
global(tex_xlat)   /* table used to escape characters meaningful to TeX */
global(opt_xlat)
global(opt_email)  /* flag indicating whether source author's
                      e-mail should be shown */

proc main() {
  list(headlist)
  table(stab)    /* Table of numbers for each individual */
  list(bibList)
  list(excurlist)
  table(bibTable)
  list(figureCiteList)
  list(figureNodeList)

  list (sourceList)

  set(opt_xlat, 1)
  table(tex_xlat)
  insert(tex_xlat, "$", "\\$")
  insert(tex_xlat, "&", "\\&")
  insert(tex_xlat, "%", "\\%")
  insert(tex_xlat, "#", "\\#")
  insert(tex_xlat, "_", "\\_")
  insert(tex_xlat, "{", "\\{")
  insert(tex_xlat, "}", "\\}")
  insert(tex_xlat, "~", "\\verb|~|")
  insert(tex_xlat, "^", "\\verb|^|")
  insert(tex_xlat, "\\", "\\verb|\\|")
  insert(tex_xlat, "<", "$<$")  /* out of math mode, < and > produce */
  insert(tex_xlat, ">", "$>$")  /*   upsidedown ! and ? marks        */

  list(daysToMonthList)
  setel(daysToMonthList, 1, 0)
  setel(daysToMonthList, 2, 31)
  setel(daysToMonthList, 3, 59)
  setel(daysToMonthList, 4, 90)
  setel(daysToMonthList, 5, 120)
  setel(daysToMonthList, 6, 151)
  setel(daysToMonthList, 7, 181)
  setel(daysToMonthList, 8, 212)
  setel(daysToMonthList, 9, 243)
  setel(daysToMonthList, 10, 273)
  setel(daysToMonthList, 11, 304)
  setel(daysToMonthList, 12, 334)
  set(force_desc_chart, 0)

  getindimsg(indi, "Enter the first person for the report")
  if(not(indi)) {
    return(0)  /* assume the user wants to quit */
  }
  set(familycount, 0)
  while(indi) {
    set(familycount, add(familycount, 1))
    enqueue(headlist, indi)
    set(indi, 0)
    getindimsg(indi, "Enter another root person or none to proceed")
  }

  if(1) {
    getintmsg(ancestormode, "Enter 0 for descendant, 1 for ancestor report")
  } else {
    set(ancestormode, 0)
  }

  set(maxgenprint, 999)
  if(eq(0, ancestormode)) {
    if(1) {
      getintmsg(maxgenprint, "Max generations for descendancy books")
    } else {
      set(maxgenprint, 999)
    }
  }

  if(1) {
    getintmsg(notes_text_mode,
      "Book Text:1='1 TEXT'; 2= all '1 NOTE'; 3= !-tag '1 NOTE's")
  } else {
    set(notes_text_mode, 1)
  }

  if(0) {  /* change this to  "if (1)" to turn this query on. */
    getintmsg(dumpplacetable_each_gen,
      "Enter 1 to reset place name table each generation, 0 to not")
  } else {
    set(dumpplacetable_each_gen, 0)
  }
  
  if(1) {
    getintmsg(opt_email,
      "Enter 1 to show source authors' e-mail addresses, 0 to not")
  } else {
    set(opt_email, 0)
  }

  /* Print preamble.  Feel free to change this to suit your tastes. */
  "\\documentclass[twocolumn,twoside,titlepage]{book}\n"  /* LaTeX 2e */
  /*"\\documentstyle[twocolumn,makeidx]{book}\n"*/
  "\\pagestyle{myheadings}\n\n"
  "% Enable ISO 8859/1 charset" nl()
  "\\usepackage{isolatin1}" nl()
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
  "\\newcommand{\\nodate}{\\underline{\\ \\ \\ \\ }}\n\n"
  "% Environment for printing the list of children.\n"
  "\\newenvironment{childrenlist}"
    "{\\begin{small}\\begin{list}{\\sc\\roman{childnumber}.}"
    "{\\usecounter{childnumber}\\setlength{\\leftmargin}{0.5in}"
    "\\setlength{\\labelsep}{0.07in}\\setlength{\\labelwidth}{0.43in}}}"
    "{\\end{list}\\end{small}}\n\n"
  "% The following commands are used to create the index.\n"
  "\\newcommand{\\bold}[1]{{\\bf #1}}\n"
  "\\newcommand{\\bfit}[1]{{\\bf\\it #1}}\n"
  "%%\\newcommand{\\see}[2]{{\\it see #1}} %not needed with makeidx.sty\n\n" 
  "% Command to use at the beginning of each new generation.\n"
  if(ancestormode) {
    "\\newcommand{\\generation}[2]"
      "{\\newpage\\begin{center}{\\huge\\bf Generation #1}\\end{center}"
      "\\vspace{3ex}\\setcounter{footnote}{0}"
      "\\markright{#2 Ancestors" "\\hfill Generation #1\\hfill\\ }"
      "}\n\n"
  } else {
    "\\newcommand{\\generation}[2]"
      "{\\newpage\\begin{center}{\\huge\\bf Generation #1}\\end{center}"
      "\\vspace{3ex}\\setcounter{footnote}{0}"
      "\\markright{#2 Descendants" "\\hfill Generation #1\\hfill\\ }"
      "}\n\n"
  }
  "\\newcommand{\\image}[4]"
    "{\\begin{figure}\n\\centerline{\\psfig{figure=#1,height=#4}}\n"
    "\\label{#3}\n"
    "\\caption{#2}\n\\end{figure}}\n"
  "\\newcommand{\\imwide}[4]"
    "{\\begin{figure*}\n\\centerline{\\psfig{figure=#1,height=#4}}\n"
    "\\label{#3}\n"
    "\\caption{#2}\n\\end{figure*}}\n"

  "\\makeindex\n\n"
  "\n\\input{tree}\n" /* needed for making descendant trees */
  "\\begin{document}\n\n"
  
  /*******************************************/
  /* Make the title */
  /*******************************************/
  "\\title{ The " 
  forlist(headlist,head,localcount) {
    if(gt(localcount,1)) {
      if(gt(familycount,2)) {    /* don't say "a, and b." */
        ", "
      }
      if(eq(localcount,familycount)) { " and " }
    }
    strxlat(tex_xlat, surname(head))
  }
  if(eq(1,familycount)) {
    " Family}\n"
  } else {
    " Families}\n"
  }

  getstrmsg(author, "Enter the author(s) of this document:")
  "\\author{" strxlat(tex_xlat, author) "}\n"
  "\\date{\\today}\n"
  "\\maketitle\n"

  "\\clearpage\n"
  "\\onecolumn\n"
  "\\pagestyle{empty}\n"
  "\\mbox{ }\n"
  "\\vfill\n"
  "\\begin{center}\n"
  "Copyright \\copyright \\ \\today \\  " strxlat(tex_xlat, author) "\\\\" 
  getstrmsg(copyplace, "Enter the place for the copyright notice:")
  strxlat(tex_xlat, copyplace) "\n"

  "\\end{center}\n"
  "\\clearpage\n"
  "\\pagestyle{myheadings}\n"
  "\\twocolumn\n"

  "\\setcounter{page}{1}\n"
  "\\tableofcontents"

  getstrmsg(intro, "File that contains introduction (if any):")
  if(ne(strcmp(intro, ""), 0)) {
    "\\input{" intro "}\n"
  }



  table(eventNameTable)
  table(eventPlaceTable)
  table(eventNameTable)
  insert(eventNameTable, "BIRT", "was born")
  insert(eventNameTable, "ADOP", "was adopted")
  insert(eventNameTable, "BAPM", "was baptized")
  insert(eventNameTable, "CHR",  "was baptized")
  insert(eventNameTable, "DEAT", "died")
  insert(eventNameTable, "BURI", "was buried")
  insert(eventNameTable, "CREM", "was cremated")
    /* GRAD left blank since it is done as a separate case */
  insert(eventNameTable, "GRAD", "")
  insert(eventNameTable, "NATU", "was naturalized")
  insert(eventNameTable, "CHRA", "was christened (as an adult)")
  insert(eventNameTable, "CENS", "was listed in the census")
  insert(eventNameTable, "ORDN", "was ordained")
  insert(eventNameTable, "RELI", "")
  insert(eventNameTable, "RESI", "lived")
  insert(eventNameTable, "CONL", "was confirmed")
  insert(eventNameTable, "CONF", "was confirmed")
  insert(eventNameTable, "BLES", "was blessed")
  insert(eventNameTable, "BASM", "was bat mitzvah-ed")
  insert(eventNameTable, "BARM", "was bar mitzvah-ed")
  /* these two Will related things come out kind of icky because
     I just always use plain pronouns like he, she, not
     possessive ones, so I have to say "he wrote  a will"
     instead of "his will was dated"
  */
  insert(eventNameTable, "PROB", "had a will probated")
  insert(eventNameTable, "WILL", "wrote a will")
  insert(eventNameTable, "RETI", "retired")


  indiset(idex)


  set(out, 1)
  set(in, 1)

  dayformat(2)
  monthformat(6)
  dateformat(1)
  while(indi, dequeue(headlist)) {
    if(ancestormode) {
      call  ancestor_chapterproc(indi)
    } else {
      call chapterproc(indi)
    }
  }

  set(basename, 
    save(substring(outfile(), 1, sub(index(outfile(), ".", 1), 1))))

  /* Output bibliography commands */
  if(not(empty(bibList))) {
    "\n\n\\onecolumn"
    "\n\\cleardoublepage"
    "\n\\label{Bibliography}"
    "\n\\addcontentsline{toc}{chapter}{Bibliography}"
    "\n\\begin{thebibliography}{9.99}"
    "\n\\input{" basename "-bib.tex}"
    "\n\\end{thebibliography}"
  }
  "\n\n\\cleardoublepage"
  "\n\\label{Index}"
  "\n\\addcontentsline{toc}{chapter}{Index}"
  "\n\\input{" basename ".ind}"

  "\n\n\\end{document}\n"

  /* Output bibliography file */
  print("\n\nCreating support files ...")
  if( not(empty(bibList)) ) {
    newfile(concat(basename, "-bib.tex"), 0)
    print("writing to : ")
    print("\n")
    print(concat(basename, "-bib.tex"))
    print("\n")
    while(b, dequeue(bibList)) { b }
  }
}

proc chapterproc(topguy) {
  list(ilist)    /* List of individuals */
  list(glist)    /* List of generation for each individual */
  set(last_grandparents,0)
  indiset(grandparentset)
  indiset(hisset)
  indiset(last_grandparentset)
  set(chapterTitle, 
  save(concat("The ", 
    concat(fullname(topguy, 0, 1, 99), "\ Family"))))
  
  "\n\\chapter{" chapterTitle "}" "\n"

  enqueue(ilist, topguy)
  enqueue(glist, 1)
  set(curgen, 0)
  set(printed_brace,0)
  set(just_printed_brace,0) 

  /* we have to do this add1 once for the topguy of each chapter
     We used to start out with "in" initialized to 2, but that made
     the numberings bad when there were multiple chapters, so now
     we init to 1 and do this +1 here
  */
  set(in, add(in, 1))

  set(hadsplitnote,0)
  set(this_level_hadsplitnote,0)

  while(indi, dequeue(ilist)) {
    /* This is where we implement the "Chapter Split".  The idea is,
       that if you put a BOOKCHAPSPLIT note on the head of a line, then
       each of his children will head up their own chapters with
       the children of that line as Generation 1 in their respective chapters.
    */
    if(eq(1,this_level_hadsplitnote)) {
      call chapterproc(indi)
    } else {
      set(thisgen, dequeue(glist))
      if(ne(curgen, thisgen)) {
        /* If we are starting a new generation, close off brace
           from previous gen. if necessary
        */
        if(printed_brace) {
          "\n\\nobreak"
          "\n" "$\\underbrace{\\hspace*{3in}}$" "\n\n"
          set(printed_brace,0)
        }

        if(dumpplacetable_each_gen) {
          table(eventPlaceTable)
        }
        print("Generation ") print(d(thisgen)) print("\n")
        "\n\n\\generation{" d(thisgen) "}" "{"
        strxlat(tex_xlat, surname(topguy)) "}" "\n"
        "\n\\addcontentsline{toc}{section}{Generation " d(thisgen) "}\n"  
        set(curgen, thisgen)
        set(last_grandparents,0)
        indiset(last_grandparentset)
        set(printed_brace,0)
        if(eq(curgen,maxgenprint)) {
          set(atmax_generation,1)
        } else {
          set(atmax_generation,0)
        }
      }
      /* decide if we have the same grandparents or not */
      /* I try to group people  together with over/under braces
         for people descended from the same grandparent in this descendancy */
      /* rather than remember who belongs to what line, I just look
         at all their grandparents and if they overlap with the grandparents
         of the previous person, then I assume I'm on the same line.
         This isn't always true, but it is a start, at least.
      */
      if(gt(curgen,3)) {
        if(eq(0,last_grandparents)) {
           "\n" "$\\overbrace{\\hspace*{3in}}$" "\n"
           set(printed_brace,1) 
           set(just_printed_brace,1) 
           indiset(hisset)
           addtoset(hisset,indi,1)  
           set(grandparentset,parentset(parentset(hisset)))
           set(last_grandparentset,grandparentset)
           set(last_grandparents,1)
         } else {
           indiset(hisset)
           addtoset(hisset,indi,1)  
           set(grandparentset,parentset(parentset(hisset)))
           set(doit,1)
           indiset(extraSet)
           set(extraSet,intersect(grandparentset,last_grandparentset))
           forindiset(extraSet, joe, a, b) {
             set(doit,0)
           }
           if(doit) {
             if(printed_brace) {
               "\n\\nobreak"
               "\n" "$\\underbrace{\\hspace*{3in}}$" "\n\n"
             }
             "\n" "$\\overbrace{\\hspace*{3in}}$" "\n\n"
             set(printed_brace,1) 
             set(just_printed_brace,1) 
             /* also reset the place table after each set of grandchildren.
                This makes it repeat the whole location name the next time
                it sees any location. Otherwise, it can get too far from the
                introduction of the place for my liking.
             */
            if(dumpplacetable_each_gen) {
              table(eventPlaceTable)
            }
          }
          set(last_grandparentset,grandparentset)
          set(last_grandparents,1)
        }
      }

      print(d(out)) print(" ") print(name(indi)) print("\n")

      /* only do the vspace between people if there was no overbrace printed.
         Otherwise there is too much white space and it looks icky.
      */
      if(eq(0,just_printed_brace)) {
        "\n\\vspace{3ex}\\ \\\\"
      }
      set(just_printed_brace,0) 

      "\\begin{center}{\\large\\bf " d(out) "\\ "
      strxlat(tex_xlat, name(indi)) "}\\end{center}\n"
      "\\nobreak\n"
      insert(stab, save(key(indi)), out)

      call longvitals(indi, 1, 2)
      if(hadsplitnote) {
        set(this_level_hadsplitnote,1)
      }
  
      addtoset(idex, indi, 0)
      set(out, add(out, 1))
      /* check whether the children we are about to print are at the
         Max generation
      */
      set(save_atmax_generation,atmax_generation)
      if(eq(add(curgen,1),maxgenprint)) {
        set(atmax_generation,1)
      }

      families(indi, fam, spouse, nfam) {
        "\n\n"
        if(eq(0, nchildren(fam))) {
          call texname(inode(indi), 0) "\\ and "
          if(spouse) {
            call texname(inode(spouse), 0)
          } else {
            "\\noname"
          }
          call havehadchildren(indi, spouse)
        } elsif(and(spouse, lookup(stab, key(spouse)))) {
          "Children of " call texname(inode(indi), 0) "\\ and "
          call texname(inode(spouse), 0) "\\ are shown under "
          call texname(inode(spouse), 0)
          "(" d(lookup(stab, key(spouse))) ").\n"
        } else {
          "Children of " call texname(inode(indi), 0) "\\ and "
          if(spouse) {
            call texname(inode(spouse), 0)
          } else {
            "\\noname"
          }
          ":\n\\begin{childrenlist}\n"
          children(fam, child, nchl) {
            set(haschild, 0)
            families(child, cfam, cspou, ncf) {
              if(ne(0, nchildren(cfam))) {
                set(haschild, 1)
              }
            }
            if(and(haschild,lt(curgen,maxgenprint))) {
              if(not(lookup(stab, key(child)))) {
                enqueue(ilist, child)
                enqueue(glist, add(1, curgen))
                "\n\\item[{\\bf " d(in) "}\\ \\hfill"
                "\\addtocounter{childnumber}{1}"
                "{\\sc\\roman{childnumber}}.]"
                set(in, add(in, 1))
                call shortvitals(child)
              } else {
                "\n\\item[{\\bf " d(lookup(stab, key(child))) "}\\ \\hfill"
                "\\addtocounter{childnumber}{1}"
                "{\\sc\\roman{childnumber}}.]"
                call shortvitals(child)
                "  Details of " pn(child, 3) " family were shown earlier."
              }
            } else {
              if(haschild) {
                set(force_desc_chart, 1)
              }
              "\n\\item "
              call longvitals(child, 0, 1)
              set(force_desc_chart,0)
              addtoset(idex, child, 0)
            }
          }
          "\\end{childrenlist}\n"
        }
      }
      set(atmax_generation, save_atmax_generation)

      if(eq(indi,topguy)) {
        set(descFigureLabel, save(concat(key(indi), "-figure-desc")))
        "\nA brief chart of the descendants of this line is contained in "
        "Figure~\\ref{"   descFigureLabel   "}."
        "\n\\begin{figure*}\n"
        "\\centering\n"
        call desc_chart_main3(indi)
        "\n\\caption{Descendents of " fullname(indi,0,1,99) "({\\bf "
        d(lookup(stab, key(indi))) "})}" nl()
        "\\label{" descFigureLabel "}"
        "\\end{figure*}\n"
      }
    }
  }

  /* Close off the last braces if necessary */
  if(printed_brace) {
    "\n\\nobreak"
    "\n" "$\\underbrace{\\hspace*{3in}}$" "\n\n"
  }
  set(printed_brace,0)


  while(indi, dequeue(excurlist)) {
    call excursion(indi)
  }
}


/* Run this routine if you want an ahnentafel style report for the
   individuals named
*/
proc ancestor_chapterproc(topguy) {
  list(ilist)    /* List of individuals */
  list(glist)    /* List of generation for each individual */
  list(alist)
  set(chapterTitle, 
  save(concat(fullname(topguy, 0, 1, 99), "\ Ancestors")))
  "\n\\chapter{" chapterTitle "}" "\n"

  enqueue(ilist, topguy)
  enqueue(alist,1)
  enqueue(glist, 1)
  set(curgen, 0)
  insert(stab, save(key(topguy)), 1)

  while(indi, dequeue(ilist)) {
    set(ahnen, dequeue(alist))
    set(thisgen, dequeue(glist))
    if(ne(curgen, thisgen)) {
      print("Generation ") print(d(thisgen)) print("\n")
      "\n\n\\generation{" d(thisgen) "}" "{"
      strxlat(tex_xlat, surname(topguy)) "}" "\n"
      "\n\\addcontentsline{toc}{section}{Generation " d(thisgen) "}\n"  
      set(curgen, thisgen)
      /* reset the place table at each generation if asked to. */
      if(dumpplacetable_each_gen) {
        table(eventPlaceTable)
      }
    }
    print(d(ahnen)) print(" ") print(name(indi)) print("\n")

    "\n\\vspace{3ex}\\ \\\\"

    "\\begin{center}{\\large\\bf " d(ahnen) "\\ "
    strxlat(tex_xlat, name(indi)) "}\\end{center}\n"

    /****************************************************************/
    /* first, enqueue his parents onto the lists so their numbers
       will be printed out in the description of "indi" */
    /* also includes a check to see if indi's parents are already
       there such as will happen when cousins marry.
    */
    set(print_dad_note,0)
    set(print_mom_note,0)

    if(par,father(indi)) {
      if(not(lookup(stab,key(par)))) {
        enqueue(ilist, par)
        enqueue(alist, mul(2,ahnen))
        enqueue(glist, add(curgen, 1))
        insert(stab, save(key(par)), mul(2,ahnen))
      } else {
        set(print_dad_note,key(par))
      }
    }
    if(par, mother(indi)) {
      if(not(lookup(stab,key(par)))) {
        enqueue(ilist, par)
        enqueue(alist, add(1,mul(2,ahnen)))
        enqueue(glist, add(curgen, 1))
        insert(stab, save(key(par)), add(1,mul(2,ahnen)))
      } else {
       set(print_mom_note,key(par))
      }
    }

    /****************************************************************/
    /* now to print out info about this person */

    call longvitals(indi, 1, 2)
  
    addtoset(idex, indi, 0)
    families(indi, fam, spouse, nfam) {
      "\n\n"
      if(eq(0, nchildren(fam))) {
        call texname(inode(indi), 0) "\\ and "
        if(spouse) {
          call texname(inode(spouse), 0)
        } else {
          "\\noname"
        }
        call havehadchildren(indi, spouse)
      } elsif( and(female(indi), spouse, lookup(stab, key(spouse))) ) {
        /* note that the form of that if is different here than in
           descendant reports.  It is different because we explicitly
           form the queue by adding the father before the mother.
           Thus, for the parent-set, the children will be printed 
           under the father. We don't check to see if the father's
           spouse had them previously printed because that would
           only happen if a different (non-ancestor) wife also
           happened to be an ancestor from a different branch, which
           could happen, but has to be pretty rare.  (E.g. your
           dad's dad marries your mom's mom for the 2nd marriage for both
           of them, something like that. Ick.)
        */
        "Children of " call texname(inode(indi), 0) "\\ and "
        call texname(inode(spouse), 0) "\\ are shown under "
        call texname(inode(spouse), 0)
        "(" d(lookup(stab, key(spouse))) ").\n"
      } else {
        "Children of " call texname(inode(indi), 0) "\\ and "
        if(spouse) {
          call texname(inode(spouse), 0)
        } else {
          "\\noname"
        }
        ":\n\\begin{childrenlist}\n"
        children(fam, child, nchl) {
          set(haschild, 0)
          families(child, cfam, cspou, ncf) {
            if(ne(0, nchildren(cfam))) {
              set(haschild, 1)
            }
          }
          "\n\\item "
          if(not(lookup(stab, key(child)))) {
            call longvitals(child, 0, 1)
            addtoset(idex, child, 0)
          } else {
            call shortvitals(child)
            "  Details of " pn(child,3) " family were shown earlier "
            "({\\bf "
            d(lookup(stab, key(child))) "})" "."
          }
        }
        "\\end{childrenlist}\n"
      }
    }  /* END families loop */

    /* if his parents are not numbered as expected, tell 'em so. */
    if(ne(0,print_dad_note)) {
      "\n Note that " pn(indi,3) " father "
      "({\\bf "
      d(lookup(stab, print_dad_note)) "})"
      " is not found in the usual "
      "ahnentafel-style numbering place due to intermarriages.\n"
    }
    if(ne(0,print_mom_note)) {
      "Note that " pn(indi,3) " mother "
      "({\\bf "
      d(lookup(stab, print_mom_note)) "})"
      " is not found in the usual "
      "ahnentafel-style numbering place due to intermarriages.\n"
      "\n"
    }

    if(eq(indi,topguy)) {
      call pedigreeFigure(indi)
      if(strcmp(pedigreeFigureLabel, "")) {
        pn(indi, 2)
        " pedigree is illustrated in Figure \\protect\\ref{" 
        pedigreeFigureLabel "}."
      }
    }
  }
}

/* shortvitals(indi):  Displays the short form of the vital statistics (birth
   and death only) of an individual.
*/
proc shortvitals(indi) {
  call resetdayplace()
  call texname(inode(indi), 1)
  set(b, birth(indi))
  set(d, death(indi))

  set(local_dead,global_dead) /* save for restore */
  set(global_dead,d)

  if(and(b, long(b))) {
    call process_event(b) 
    if(and(d, long(d))) {
      " and " pn(indi,1)
      call process_event(d)
    }
  } else {    /* know death info, not birth*/
    if(and(d, long(d))) {
      call process_event(d)
    }
  }
  "."
  set(global_dead,local_dead) /* restore */
}


/* longvitals(i, name_parents, name_type)
   Prints out the complete vital statistics of the individual (i).  If
   name_parents is not 0, then the names of the parents of the individual will
   be printed.  The parameter name_type is passed to texname.  The GEDCOM tags
   are divided into ones that would likely occur before getting married and
   ones that would likely occur after getting married.  Within the two sets
   they are printed in the order in which they appear in the database.  I
   haven't yet figured out a convenient way of indicating the sex.
*/
proc longvitals(i, name_parents, name_type) {
  call resetdayplace()

  set(local_dead,global_dead) /* save for restore */
  set(global_dead,death(i))

  call texname(inode(i), name_type)  call print_sources(inode(i)) 
  call getValue(inode(i),"NAME")
  if(gotValue) {
    call print_sources(gottenNode) 
  }

  /* remember the value so it doesn't affect spousevitals */
  set(save_force_chart,force_desc_chart)
  set(force_desc_chart,0)

  set(dad, father(i))
  set(mom, mother(i))
  if(and(name_parents, or(dad, mom))) {
    ", "
    if(male(i))      { "the son of " }
    elsif(female(i)) { "the daughter of " }
    else             { "the child of " }
    if(dad)          {
      call texname(inode(dad), 0) 
      if(lookup(stab, key(dad))) {
        "({\\bf "
        d(lookup(stab, key(dad)))
        "})"
      }
    }
    if(and(dad, mom)) {
      "\nand "
    }
    if(mom) {
      call texname(inode(mom), 0) 
      if(lookup(stab, key(mom))) {
        "({\\bf "
        d(lookup(stab, key(mom)))
        "})"
      }
    }
    ",\n"
  }

  set(name_found, 0)
  set(needname,0)
  set(canUseAnd,1)
  set(printedOne,1) /* at the start, we have just printed his fullname*/
  set(pronoun," ")
  set(anythingprinted,0)
  set(putPeriod,0)
  /* there is a mistake in this for right now, if we don't have any
     pre-marriage info, it'll be ugly for the death.
  */
  fornodes(inode(i), n) {
    if(not(printedOne)) {
      if(needname) {
        set(pronoun, printablefirstname(i))
        set(needname,0)
        set(canUseAnd,1)
        set(printedOne,1)
        set(putPeriod,0)
        set(pronoun," ")
      } else {
        if(canUseAnd) {
          set(pronoun," and ")
          set(canUseAnd,0)
          set(printedOne,1)
          set(putPeriod,1)
        } else {    /* set up for "He" this time, "and" next time. */
                    /* put the period in for the first time */
          if(not(anythingprinted)) {
            set(putPeriod,1)
          } else {
            set(putPeriod,0)
          }
          set(pronoun,pn(i,0))
          set(canUseAnd,1)
          set(printedOne,1)
        }
      }
      set(anythingprinted,1)
    }/* end if not printedone */

    if(
      or(eq(strcmp(tag(n), "ADOP"), 0),
        eq(strcmp(tag(n), "BAPL"), 0),
        eq(strcmp(tag(n), "BAPM"), 0),
        eq(strcmp(tag(n), "BARM"), 0),
        eq(strcmp(tag(n), "BASM"), 0),
        eq(strcmp(tag(n), "BIRT"), 0),
        eq(strcmp(tag(n), "BLES"), 0),
        eq(strcmp(tag(n), "CONF"), 0),
        eq(strcmp(tag(n), "CONL"), 0),
        eq(strcmp(tag(n), "ORDN"), 0),
        eq(strcmp(tag(n), "CHR" ), 0)
      )) {
      pronoun set(printedOne,0) 
      call process_event(n)
      if(putPeriod) {". "}
    }
    if(eq(strcmp(tag(n), "GRAD"), 0)) {
      pronoun set(printedOne,0) 
      " graduated from "
      call valuec(n)
      call process_event(n)
      if(putPeriod) {". "}
    }

    if(eq(strcmp(tag(n), "CAST"), 0)) {
      pronoun set(printedOne,0) 
      " was a member of Caste: " call valuec(n) 
      call process_event(n)
      if(putPeriod) {". "}
    }
    if(eq(strcmp(tag(n), "NAME"), 0)) {
      if(eq(name_found, 0)) {
        set(name_found, 1)
      } else {
        pronoun set(printedOne,0) 
        " was also known as " call texname(n, 3)
        call print_notes(n, " ") 
        call print_sources(n) 
        if(putPeriod) {". "}
        "\n"
      }
    }
    if(eq(strcmp(tag(n), "NAMR"), 0)) {
      pronoun set(printedOne,0) 
      " had the religious name of: " call valuec(n)
      call process_event(n)
      if(putPeriod) {". "}
    }
    if(eq(strcmp(tag(n), "RELI"), 0)) {
      pronoun set(printedOne,0) 
      " was a " call valuec(n)
      call process_event(n)     
      if(putPeriod) {". "}
    }
/*
    if(eq(strcmp(tag(n), "TITL"), 0)) {
      pronoun set(printedOne,0) 
      " held the title of " value(n)
      call print_sources(n) 
      if(putPeriod) {". "} "\n"
    }
*/
  }  /*  END fornodes(inode(i), n)  */

  if(and(putPeriod,anythingprinted)) { ". " }
  set(inhibit_text_charts,0)
  if(eq(1, nfamilies(i))) {
    families(i, f, s, n) {
      if(s) {
        if(anythingprinted) { pn(i,0) }
        set(anythingprinted,1)
        call illegit_check(f)
        if(not_married_flag) {
          " had a child with"
        } else {
          " married"
        }
        /* it is OK to assume they had children if the
           Not married flag is raised. Cuz if they
           weren't married and didn't have kids, there
           isn't much point in the family existing
           (as far as this program is concerned).
        */
        call print_sources(fnode(f))
        call spousevitals(s, f)
      }
    }  /* END families(i, f, s, n) */

  } else {  /* individual 'i' had more than one family */
    families(i, f, s, n) {
      if(anythingprinted) {
        /* print "he" (or "she") for first marriage, and his
           first name for all later marriages
        */
        if(gt(n,1)) {
          printablefirstname(i)
        } else {
          pn(i,0)
        }      
      }
      set(anythingprinted, 1)
      if(s) {
        call illegit_check(f)
        if(not_married_flag) {
          " had child(ren) with "
        } else {
          " married "
        }
        ord(n) "," call print_sources(fnode(f))
        call spousevitals(s, f)
        /* make sure we don't print this (i) persons
           text and charts a second time. The theory here
           being that if the spouse is in the stab,
           then this person (i) has already had their
           notes/charts done as part of their spouse.
           We don't want to print a 2nd time
        */
        if(not(ancestormode)) {
          if(lookup(stab, key(s))) {
            set(inhibit_text_charts,1)
          }
        }
      } else {
        " " ord(n) " had a child with an unknown spouse.  "
      }
    }  /*  END families(i, f, s, n)  */
  }
  "  "
/*
  if(anythingprinted) {
    call getText(inode(i),0)
  }
*/
  /* otherwise hold off on getText until after the death info & all */
  /* I changed my mind. I like getText at the end. */
  set(needname,1)
  set(canUseAnd,0)
  set(putPeriod,0)
  if(anythingprinted) {
    set(printedOne,0) /* haven't printed a name here after the spouse info */
                      /* this works because pronoun is still a blank */
  } else {
    set(printedOne,1) /* we still have his name from way at the start */
    set(putPeriod,1) /* JUST ADDED THIS LINE 5/24/96. Don't know if works */
    /* test it on Galbraith kids where know only death.*/
  }

  fornodes(inode(i), n) {  /* process DEATH related gedcom nodes */
    if(not(printedOne)) {
      if(needname) {
        set(pronoun, printablefirstname(i))
        set(needname,0)
        set(canUseAnd,1)
        set(printedOne,1)
        set(putPeriod,0)
      } else {
        if(canUseAnd) {
          set(pronoun," and ")
          set(canUseAnd,0)
          set(printedOne,1)
          set(putPeriod,1)
        } else {
          set(pronoun,pn(i,0))
          set(canUseAnd,1)
          set(printedOne,1)
          set(putPeriod,0)
        }
      }
    }
    if(
      or(
        eq(strcmp(tag(n), "BURI"), 0),
        eq(strcmp(tag(n), "CREM"), 0),
        eq(strcmp(tag(n), "CENS"), 0),
        eq(strcmp(tag(n), "CHRA"), 0),
        eq(strcmp(tag(n), "DEAT"), 0),
        eq(strcmp(tag(n), "NATU"), 0),
        eq(strcmp(tag(n), "RETI"), 0),
        eq(strcmp(tag(n), "RESI"), 0),
        eq(strcmp(tag(n), "PROB"), 0),
        eq(strcmp(tag(n), "WILL"), 0)
      )) {
      pronoun set(printedOne,0) 
      call process_event(n)
      if(putPeriod) {". "}
    }

    /* One part of the GEDCOM standard says the tag should be DSCR,
       another part says DESR.
    */
    if(eq(strcmp(tag(n), "DESR"), 0)) {
      pronoun set(printedOne,0) 
      "Description: " call valuec(n)
      call print_sources(n) 
      if(putPeriod) {". "}
    }
    if(eq(strcmp(tag(n), "EVEN"), 0)) {
      pronoun set(printedOne,0) 
      value(n)
      call process_event(n)
      if(putPeriod) {". "}
    }
    if(eq(strcmp(tag(n), "OCCU"), 0)) {
      pronoun set(printedOne,0) 
      /* should also check for a RETIred node
         and always say WAS if it exists
      */
/* DAB - replace with do_occu; delete this when do_occu accepted
      call getValue(inode(i),"RETI")
      if(gotValue) { 
        " was"
      } else {
        call iswas(i)
      }
      " "
      call aAn(value(n)) " "
      call valuec(n)
      call process_event(n)
*/
      call do_occu(n, i)
      if(putPeriod) {". "}
    }
    if(eq(strcmp(tag(n), "PROP"), 0)) {
      pronoun set(printedOne,0) 
      "had possessions: " call valuec(n) "."
      call print_sources(n) 
      if(putPeriod) {". "}
    }
  }
  if(putPeriod) {". "}
  /* restore forcing of desc. charts */
  set(force_desc_chart,save_force_chart)
  if(not(inhibit_text_charts)) {
    call getText(inode(i),0)
    call process_book_notes(i)
  }
  set(global_dead,local_dead) /* restore */
}


/* isRange(d) - Indicate if a date node is a range
 * 
 * d - DATE node(could be NIL)
 *
 * Returns: 1 if <d> is of the form "[BET] date1-date2"; 0 otherwise
 *
 */
func isRange(d) {
  set(r, 0)
  if(d) {
    if(i, index(d, "-", 1)) { 
      set(r, 1)
    } elsif(i, index(d, "FROM", 1)) { 
      set(r, 1)
    }
  }
  return(r)
}


/* do_occu(n, i) - Process an OCCU node
 * 
 * n - OCCU node
 * i - INDI containing <n>
 *
 * An OCCU node will produce text saying "<name> is/was a <occu> with <agnc> ...."
 * It is assumed that the <name> was printed before this routine was called.
 * If the person is (likely) deceased, if the OCCU node has a subordinate RETI node,
 * or if the DATE tag subordinate to the OCCU node is a range, then "was" is used in
 * the sentence; otherwise, "is" is used. The "with <agnc>" clause is added if a
 * AGNC node is subordinate to the OCCU node; it is taken to be the name of the 
 * employer.
 *
 */

proc do_occu(n, i) {
  /* Check for date range or RETI node and use "was" if either present. */
  set(d, date(n))
  call getValue(inode(i), "RETI")
  if(or(gotValue, isRange(d))) {
    " was"
  } else {
    call iswas(i)
  }
  " "
  call aAn(value(n)) " "
  call valuec(n)
  call getValue(n, "AGNC")
  if(gotValue) {
    " with " gottenValue
  }
  call process_event(n)
  /*    
    if(putPeriod) {". "}
  */
}


/* spousevitals(spouse, fam)
   Prints out information about a marriage (fam) and about a spouse in the
   marriage (spouse).
*/

proc spousevitals(spouse, fam) {
  call texname(inode(spouse), 3)
  if(spouse) {
    call print_sources(inode(spouse))
    call getValue(inode(spouse),"NAME")
    if(gotValue) {
      call print_sources(gottenNode) 
    }
  }

  if(e, marriage(fam)) {
    call process_event(e) 
  }
  ". " 
  call check_print_divinfo(fam)

  if(spouse) {
    set(bir, birth(spouse))
    set(chr, baptism(spouse))
    set(dea, death(spouse))
    set(bur, burial(spouse))
    set(dad, father(spouse))
    set(mom, mother(spouse))

    set(local_dead,global_dead) /* save for restore */
    set(global_dead,dea)


    if(or(bir, chr, dea, bur, mom, dad)) {
      printablefirstname(spouse)
      if(or(mom, dad)) {
        ", "
        if(male(spouse)) {
          "the son of "
        } elsif(female(spouse)) {
          "the daughter of "
        } else {
          "the child of "
        }
        if(dad) { 
          call texname(inode(dad), 3) 
          if(lookup(stab, key(dad))) {
            "({\\bf "
            d(lookup(stab, key(dad)))
            "})"
          }
        }
        if(and(mom, dad)) {
          " and "
        }
        if(mom) {
          call texname(inode(mom), 3) 
          if(lookup(stab, key(mom))) {
            "({\\bf " d(lookup(stab, key(mom))) "})"
          }
          ", "
        }
      }

      if(or(or(or(bir, chr), dea), bur)) {
        if(bir) {
          call vitalEvent(bir,1)
          call print_sources(bir)
          if(chr) {
            " and"
            call vitalEvent(chr,0)
            call print_sources(chr)
            ". "
          }
          if(dea) {
            if(chr) {
              pn(spouse,0)
            } else {
              " and"
            }
            call vitalEvent(dea,1)
            call print_sources(dea)
            ". "
          } else {
            if(not(chr)) {
              ". "
            }
          } /* born, but nothing more */
        }
        if(and(chr, not(bir))) {
          call vitalEvent(chr,1)
          call print_sources(chr)
          ". "
        }
        if(and(dea,not(bir))) { /* if bir, then dea is already handled */
          if(chr) { /* then need to print pronoun, otherwise don't
                       since we still have the name standing there, 
                       not finishing a sentence
                    */
            pn(spouse,0) 
          } 
          call vitalEvent(dea,1)
          call print_sources(dea)
          ". "
        }
        if(bur) {
          if(or(or(bir,dea),chr)) {
            pn(spouse,0)
          }
          call vitalEvent(bur,0)
          call print_sources(bur)
          ".\n"
        }
      }
    }
    if(gt(nfamilies(spouse), 1)) {
      set(beforefam,1)
      families(spouse, newfam, newspouse, n) {
        if(ne(newfam, fam)) {
          printablefirstname(spouse)
          if(beforefam) {
            " had "
            if(gt(n, 1)) {
              "also "
            }
            "previously married "
          } else {
            " later remarried "
          }
          if(newspouse) {
            call texname(inode(newspouse), 3)
          }
          set(e,marriage(newfam))
          if(e) { call process_event(e) }
          if(gt(nchildren(newfam), 0)) {
            ", and had "
            d(nchildren(newfam))
            if(gt(nchildren(newfam), 1)) {
              " children "
            } else {
              " child"
            }
            " by that marriage: "
            children(newfam, stepchild, numerical) {
              if(gt(numerical,1)) {
                ", "
              }
              printablefirstname(stepchild)
            }       
          }
          ". "
        } else {  /* newfam = fam */
          set(beforefam,0) /* we see the current family */
        }
      }  /*  END families(spouse, newfam, newspouse, n)  */
    }  /*  END if(gt(nfamilies(spouse), 1))  */

    if(not(lookup(stab, key(spouse)))) { /* don't print a 2nd time */
      call getValue(inode(spouse),"OCCU")
      if(gotValue) {
        set(savenode,gottenNode)
        printablefirstname(spouse)
        /* should also check for a RETIred node and always say WAS if it exists */
/* DAB - replace with do_occu; delete this when do_occu is accepted
        call getValue(inode(spouse),"RETI")
        if(gotValue) {
          " was"
        } else {
          call iswas(spouse)
        }
        " "
        call aAn(value(savenode)) " "
        call valuec(savenode)
        call process_event(savenode)
        ". "
*/
        call do_occu(savenode, spouse) ". "
      }
      call getText(inode(spouse),0)
      call process_book_notes(spouse)
    }
    set(global_dead,local_dead) /* restore */
  } else {
    "\\noname" ".\n"
  }  /*  END if(spouse) block  */
}


/* texname(i, type)
   Prints an individual's name in LaTeX format, with the surname in small caps.
   For example, "David Kenneth /Olsen/ Jr." would be printed as
   "David Kenneth {\sc Olsen} Jr.".  The type argument determines how the name
   will appear in the index.
        type = 0: no index
        type = 1: page number appears in bold
        type = 2: page number appears in bold-italics
        type = 3: page number appears in normal text
   The parameter i can be either an INDI node (NOT an individual) or a
   NAME node.
*/
proc texname(i, type) {
  list(name_list)
  set(sname, "")
  extractnames(i, name_list, num_names, surname_no)
  forlist(name_list, nm, num) {
    if(eq(num, surname_no)) {
      if(eq(strcmp(nm, ""), 0)) {
        " \\noname"
        set(sname, "\\noname")
      } else {
        if(eq(strcmp(nm, "____"), 0)) {
          set(sname, "\\noname")
        } else {
          " {\\sc "
          strxlat(tex_xlat, save(nm))
          "}"
          set(sname, nm)
        }
      }
    } else {
      " " strxlat(tex_xlat, nm)
    }
  }
  if(gt(type, 0)) {
    "\\index{"
    strxlat(tex_xlat, sname)
    if(gt(num_names, 1)) {
      ","
    }
    forlist(name_list, nm, num) {
      if(ne(num, surname_no)) {
        " "
        strxlat(tex_xlat, nm)
      }
    }
    if(eq(type, 1)) {
      "|bold"
    } elsif(eq(type, 2)) {
      "|bfit"
    }
    "}"
  }
}


/* process_event(event_node, event_name)
   Prints information about a particular event (event_node, which is a GEDCOM
   node).  event_name is verb form of the text describing the event (such as
   "Born", "Died", etc.).
*/
proc process_event(event_node) {
  call vitalEvent(event_node,0)
  call print_sources(event_node)
  call print_notes(event_node, " ") 
}


proc inPlace(event) {
  if(place(event)) {
    if(eq(strcmp(place(event),""),0)) {
      " in \\noname"
    } else {
      call atAddr(event)
      if(not(strcmp(atAddrValue,""))) {
        set(fullSpecCompare,1)
      } else {
       set(fullSpecCompare,2)
      }  /* there was an ADDR. We want to 
            say at X in Y
         */
      list(placeList)
      list(placeTextList)
      list(placeTagList)
      extractplaces(event, placeList, nPlaces)
      requeue(placeList, atAddrValue)
      while(placeText, dequeue(placeList)) {
        enqueue(placeTextList, placeText)
        set(placeTag, placeText)
        forlist(placeList, place, placeN) {
          set(placeTag, save(concat(placeTag, concat("-", place))))
        }
        enqueue(placeTagList, placeTag)
      }

      set(there, getel(placeTagList, 1))
      if(not(strcmp(there, lookup(eventPlaceTable, "@there@")))) {
        "\nthere"
      } else {
        insert(eventPlaceTable, "@there@", there)
        set(fullySpecified, 0)
        forlist(placeTextList, place, placeN) {
          set(placeTag, dequeue(placeTagList))
          if(not(eq(fullySpecified,fullSpecCompare))) {
            if(eq(placeN, 2)) {
              /* if the name of the place doesn't start with "near", say "in" */
              if(and(
                strcmp(substring(place,1,4), "near"),
                strcmp(substring(place,1,2), "in"),
                strcmp(substring(place,1,5), "south"),
                strcmp(substring(place,1,5), "north"),
                strcmp(substring(place,1,4), "west"),
                strcmp(substring(place,1,4), "from"),
                strcmp(substring(place,1,4), "east"))) {
                /* note that case matters a lot in that comparison */
                /* a town might be named North English, but one should
                   always have written, "north of English" if
                   you just want to say it is outside of town
                */
                "\nin "
              } else {
                if(not(strcmp(substring(place,1,4), "from"))) {
                  " and was\n"
                  /* Actually, that isn't quite what I want because if
                     it has someone b. PLAC from there but no date, it'll wind
                     up outputting "He was born and was from there".
                     But oh well.
                  */
                } else {
                  "\n"    /* put nothing there if it says "near" */
                }
              }
            } elsif(gt(placeN, 2)) {
              ", "
            }
            place
            if(strlen(place)) {
              if(not(lookup(eventPlaceTable, placeTag))) {
                insert(eventPlaceTable, placeTag, 1)
                /* this next fiddling with fullSpecCompare is because
                   the FIRST time we see an "at ADDR in a,b,c"
                   since ADDR-a-b-c isn't in the table, it would
                   try to print "at ADDR in a,b". We just want it to 
                   print "at ADDR in a" if a,b,c was previously defined.
                */
                set(fullSpecCompare,1)
              } else {
                set(fullySpecified, add(fullySpecified,1))
              }
              if(gt(index(place,"Twp",1),0)) {
                /* then it is just the name
                   of a township, which I don't think is much
                   use without the county name, so I'm always
                   going to force it to print the county name
                   if it gives a township
                */
                set(fullSpecCompare,add(1,fullSpecCompare))
              }
            }
          }/* end if not fullySpecified */
        }
      }
    }/* matches the else for place being non-null */
  }
}

/* Possible customization I have chosen not to implement:
  Suggested by: "John F. Chandler"   <JCHBN@CUVMB.CC.COLUMBIA.EDU>
  if(gt(nPlaces,1)) { set(inWord,"at") }
   else { set(inWord,"in") }
   if(not(strcmp(tag(event),"IMMI"))) { set(inWord,"to") }
   if(not(strcmp(tag(event),"GRAD"))) {
     if(eq(nPlaces,1)) { set(inWord,"from") }
     else { set(inWord,"-") }
   }
   inWord " "
  instead of the original "in ".  This assumes that the institution is
  recorded in the PLAC
   I (DN) Think there is a little more to it than that because of the way
   I currently use atAddr, but I'm sure you can figure all that out.
   I currently assume that the institution name is the value of the
   1 GRAD line, e.g. 1 GRAD Univ. of Iowa and I handle it as a special case.
*/  


proc atAddr(root) {
  set(atAddrValue, "")
  if(root) {
    fornodes(root, node) {
      if(not(strcmp(tag(node), "PLAC"))) {
        fornodes(node, subnode) {
          if(and(not(strcmp(atAddrValue, "")),
              or(not(strcmp(tag(subnode), "ADDR")),
                 not(strcmp(tag(subnode), "CEME"))))) {
            if(val, value(subnode)) {
              set(atAddrValue, save(concat("\nat ", val)))
            }
          }
        }
      }
    }
  }
}


proc check_print_divinfo(fam) {
  call getValue(fnode(fam), "DIV")
  if(gotValue) {
    " They divorced"
    call process_event(gottenNode)
    ".  "
  }
}


/* print_notes(root, sep):  Prints all the notes (NOTE nodes) associated with
   the GEDCOM line root, separated by the given separator.
*/
proc print_notes(root, sep) {
  fornotes(root, note) {
    sep
    strxlat(tex_xlat, note)
    /* " "*/
  }
}


proc process_book_notes(indi) {
  set(hadpednote,0)
  set(haddescnote,0)
  set(doexcursion,0)

  fornotes(inode(indi), note) {
    set(i, index(note, "BOOKPEDIGREE", 1))
    if(gt(i, 0)) {
      set(hadpednote, 1)
    }
    set(i, index(note, "BOOKDESCENDENT", 1))
    if(gt(i, 0)) {
      set(haddescnote, 1)
    }
    set(i, index(note, "BOOKEXCURSION", 1))
    if(gt(i, 0)) {
      set(doexcursion, 1)
    }
    set(i, index(note, "BOOKCHAPSPLIT", 1))
    if(gt(i, 0)) {
      set(hadsplitnote, 1)
    }
  }
  if(eq(hadpednote,1)) {
    call pedigreeFigure(indi)
    if(strcmp(pedigreeFigureLabel, "")) {
      pn(indi, 2)
      " pedigree is illustrated in Figure \\protect\\ref{" 
      pedigreeFigureLabel "}."
    }
  }
  if(force_desc_chart) {
    set(haddescnote,1)
  }
  if(eq(haddescnote,1)) {
    set(descFigureLabel, save(concat(key(indi), "-figure-desc")))
    "\nA brief chart of the descendants of "
    call texname(inode(indi), 3)
     " is contained in "
    "Figure~\\ref{"   descFigureLabel   "}."
    "\n\\begin{figure*}\n"
    "\\centering\n"
    call desc_chart_main3(indi)
    "\n\\caption{Descendents of " strxlat(tex_xlat, fullname(indi,0,1,99)) "({\\bf "
    d(lookup(stab, key(indi))) "})}" nl()
    "\\label{" descFigureLabel "}"
    "\\end{figure*}\n"
  }
  if(eq(0, ancestormode)) {
    if(eq(doexcursion, 1)) {
      pn(indi, 2) " ancestors will be discussed in depth on page~\\pageref{"
      key(indi) "-excur-ref}" 
      " in this chapter.\n\n"
      enqueue(excurlist, indi)
    }
  }
}

proc pedigreeFigure(i) {
  indiset(iSet)
  addtoset(iSet, i, 1)
  set(max, 0)
  indiset(extraSet)
  set(extraSet,ancestorset(iSet))
  forindiset(extraSet, indi, val, num) {
    if(gt(val, max)) {
      set(max, val)
    }
  }
  if(gt(max, 1)) {
    set(pedigreeFigureLabel, save(concat(key(i), "-figure-pedigree")))
    if(gt(max, 5)) {
      set(max, 5)
    }
    call figPed(max, i)
  } else {
    set(pedigreeFigureLabel, "")
  }
}

proc figPed(n, indi) {
  "\n\\begin{figure*}"
  "\n\\centering"
  "\n\\small"
  "\n\\setlength{\\unitlength}{"
  if(eq(n, 5)) { ".8" } else { ".9" }
  "\n\\baselineskip}"

  call pow(2, n)
  "\n\\begin{picture}("
  d(add(mul(6, n), 12)) 
  ","
  d(sub(mul(powValue, 2), 1))
  ")(0,.5)"
  
  call ped6(indi, 0, powValue, powValue) 

  "\n\\end{picture}"
  "\n\\caption{Pedigree of " call scname(indi) "}"
  "\n\\label{" pedigreeFigureLabel "}"
  "\n\\end{figure*}" 
}

proc ped6(indi, x, y, z) {
  "\n\\put(" d(mul(x, 6)) "," d(y) "){\\makebox(0,0)[l]{"
  call scname(indi) 
  "}}"
  if(x) {
    "\n\\put(" d(sub(mul(x, 6), 3)) "," d(y) "){\\line(1,0){" d(3) "}}"
    "\n\\put(" d(sub(mul(x, 6), 3)) "," d(y) "){\\line(0,"
    if(female(indi)) { "1" } else { "-1" }
    "){" d(sub(z, 1)) ".4}}"
  }
  if(z2, div(z, 2)) {
    if(f, father(indi)) { call ped6(f, add(x, 1), add(y, z2), z2) }
    if(m, mother(indi)) { call ped6(m, add(x, 1), sub(y, z2), z2) }
  }
}

proc scname(indi) {
  strxlat(tex_xlat, fullname(indi, 0, 1, 99))
  if(lookup(stab, key(indi))) {
  "({\\bf "
  d( lookup(stab, key(indi)))
  "})"
  }
}

proc pow(x, i) {
  set(powValue, 1)
  call powIt(x, i)
}

proc powIt(x, i) { 
  if(i) {
    set(powValue, mul(powValue, x))
    call powIt(x, sub(i, 1))
  }
}


/* print_sources(root)
   Prints all sources (SOUR lines) associated with the given GEDCOM line.  The
   sources are formatted as LaTeX footnotes.  This routine prints each SOUR line
   as a separate footnote, which is not correct.  This should be corrected so
   that all sources are combined into a single footnote.
*/
proc print_sources(root) {
  enqueue(sourceList,root)
  call sourceIt(sourceList)
}


/* valuec(n):  Prints the value of a GEDCOM node and the values of any CONT
   lines associated with it.
*/
proc valuec(n) {
  value(n)
  fornodes(n, n1) {
    if(eq(strcmp(tag(n1), "CONT"), 0)) {
      "\n" value(n1)
    } elsif(eq(strcmp(tag(n1), "CONC"), 0)) {
      value(n1)
    }
  }
}


proc resetdayplace() {
  call setDayNumber(0)
  insert(eventPlaceTable, "@there@", "")
}

proc vitalEvent(event, reset) {
  if(reset) {
    call setDayNumber(0)
    insert(eventPlaceTable, "@there@", "")
  }
  if(event) {
    if(eventName, lookup(eventNameTable, tag(event))) {
      " " eventName
      if(
        or(
          not(strcmp(tag(event), "ADOP")), 
          not(strcmp(tag(event), "CHR")), 
          not(strcmp(tag(event), "CREM")), 
          not(strcmp(tag(event), "BURI"))
        )) {
        set(previousDayNumber, dayNumber)
      } else {
        set(previousDayNumber, 0)
      }
    }
    if(not(eventName)) {
      call getValue(event, "TYPE")
      if(gotValue) {
        " " strxlat(tex_xlat, gottenValue)
      }
    }
    if(not(strcmp(tag(event), "DEAT"))) {
      call ofCause(event) 
    }
    call onDate(event)
    call atAge(event)
    call inPlace(event)
  }
}


proc setDayNumber(event) {
  set(dayNumber, 0)
  if(date(event)) {
    extractdate(event, day, month, year)
    /* DAB - Have to check day month and year, otherwise two events for which
       only the year is known
       are said to have occurred on "the same day"
    */
    if(and(and(year, month), day)) {
      set(yearNumber,
        add(mul(year, 365), div(year, 4),
             neg(div(year, 100)), div(year, 400)))
      set(monthNumber, getel(daysToMonthList, month))
      set(leapYear, and(eq(mod(year, 4), 0),
        not(and(eq(mod(year, 100), 0), ne(mod(year, 400), 0)))))
      if(and(leapYear, le(month, 2))) {
        decr(monthNumber)
      }
      set(dayNumber, add(yearNumber, monthNumber, day))
    }
  }
}


/* This was the old way, replaced by the above by Jim Eggert */
/*
proc setDayNumber(event) {
  set(dayNumber, 0)
  if(date(event)) {
    extractdate(event, day, month, year)
    if(year) {
      set(yearNumber, sub(add(mul(year, 365), div(year, 4)), div(year, 400)))
      set(monthNumber, getel(daysToMonthList, month))
      set(leapYear, and(eq(mod(year, 4), 0), 
        not(and(eq(mod(year, 100), 0), ne(mod(year, 400), 0)))))
      if(and(leapYear, gt(month, 2))) {
        incr(monthNumber)
      }
      set(dayNumber, add(yearNumber, monthNumber, day))
    }
  }
}
*/


proc atAge(event) {
  call getValueCont(event, "AGE")
  if(gotValue) {
    if(not(strcmp(gottenValue, "young"))) {
      "\nyoung"
    } elsif(not(strcmp(gottenValue, "0"))) {
      "\nas an infant"
    } elsif(not(strcmp(gottenValue, "infancy"))) {
      "\nas an infant"
    } else {
      "\nat age "
      strxlat(tex_xlat, gottenValue)
    }
  }
}


proc ofCause(event) {
  call getValueCont(event, "CAUS")
  if(gotValue) {
    "\nof "
    strxlat(tex_xlat, gottenValue)
  }
}


proc onDate(event) {
  if(atmax_generation) {
    if(global_dead) {
      set(year_only,0)
    } else {
      set(year_only,1)
    }
  } else {
    set(year_only,0)  
  }
  call setDayNumber(event)
  if(d, date(event)) {
    if(strcmp(d, "Not married")) { 
      if(eq(strcmp(d, ""),0)) { "\nin \\nodate\\ " } 
      elsif(eq(index(d, "AFT", 1), 1)) { "\nsome time after " }
      elsif(eq(index(d, "Aft", 1), 1)) { "\nsome time after " }
      elsif(eq(index(d, "BEF", 1), 1)) { "\nsome time before " }
      elsif(eq(index(d, "Bef", 1), 1)) { "\nsome time before " }
      elsif(eq(index(d, "ABT", 1), 1)) { "\ncirca " }
      elsif(eq(index(d, "Abt", 1), 1)) { "\ncirca " }
      elsif(eq(index(d, "FROM", 1), 1)) { 
        set(t, index(d, "TO", 1))
/*
DAB - experimental (and not working) 
        set(fromDateEvent, createnode("EVEN", ""))
        set(fromDateNode, createnode("DATE", substring(d, add(1, strlen("FROM")), sub(t, 1)) ))
        addnode(fromDateNode, fromDateEvent, 0)
        set(toDateEvent, createnode("EVEN", ""))
        set(toDateNode, createnode("DATE", substring(d, add(t, strlen("TO")), strlen(d)) ))
        addnode(toDateNode, toDateEvent, 0)
DEBUG:
        "\n from date nodes: "
        traverse(fromDateEvent, xx, yy) {
          d(yy) ": " tag(xx) " " value(xx)
        }
        "\n to date nodes: "
        traverse(toDateEvent, xx, yy) {
          d(yy) ": " tag(xx) " " value(xx)
        }

        "\nfrom " stddate(fromDateEvent)
        " to " stddate(toDateEvent)
        deletenode(toDateNode)
        deletenode(toDateEvent)
        deletenode(fromDateNode)
        deletenode(fromDateEvent)
DAB - end of experimental
*/

/* DAB - This way work, but doesn't necessarily produce dates in the same
         format as stddate
*/
        "\nfrom " substring(d, add(1, strlen("FROM")), sub(t, 1)) 
        " to " substring(d, add(t, strlen("TO")), strlen(d)) 
        set(event, 0)
      } elsif(i, index(d, "-", 1)) { 
        "\nbetween " substring(d, 1, sub(i, 1)) 
        " and " substring(d, add(i, 1), strlen(d)) 
        set(event, 0)
      } elsif(and(dayNumber, eq(dayNumber, previousDayNumber))) {
        "\non the same day"
        set(event, 0)
      } elsif(and(dayNumber, eq(dayNumber, add(previousDayNumber, 1)))) {
        "\non the next day"
        set(event, 0)
      } elsif(and(dayNumber, eq(dayNumber, add(previousDayNumber, 2)))) {
        "\ntwo days later"
        set(event, 0)
      } elsif(and(dayNumber, eq(dayNumber, add(previousDayNumber, 7)))) {
       "\none week later"
        set(event, 0)
      } else {
        if(year_only) { "\nin "
        } else {
          extractdate(event, d, m, y)
          if(d) { "\non " } else { "\nin " }
        }
      }
      if(event) { 
        if(and(dayNumber, eq(dayNumber, previousDayNumber))) {
          "that day"
        } else {
          if(year_only) {
            year(event) 
          } else {
            stddate(event) 
          }
        }
      }
    }
  }
}

proc getValue(root, t) {
  set(gotValue, 0)
  if(root) {
    fornodes(root, node) {
      if(and(not(gotValue), not(strcmp(tag(node), t)))) {
        set(gotValue, 1)
        set(gottenNode, node)
        set(gottenValue, save(value(node)))
      }
    }
  }
}


proc getValueCont(root, t) {
  set(gotValue, 0)
  if(root) {
    fornodes(root, node) {
      if(and(not(gotValue), not(strcmp(tag(node), t)))) {
        set(gotValue, 1)
        set(gottenNode, node)
        set(gottenValue, save(value(node)))
        fornodes(node, subnode) {
          if(not(strcmp("CONT", tag(subnode)))) {
            /* If you want empty CONT tags to not leave a blank line, uncomment the following "if".
             * However, a blank line can be very useful (or even necessary) for some TeX formatting.
             */
            /*if(strlen(value(subnode))) {*/
              set(gottenValue, 
                save(concat(gottenValue, "\n", value(subnode))))
            /*}*/
          } elsif(not(strcmp("CONC", tag(subnode)))) {
            /* Same comment as above, this time for CONC tags */
            /*if(strlen(value(subnode))) {*/
              set(gottenValue, 
              save(concat(gottenValue, value(subnode))))
            /*}*/
          }
        }
      }
    }
  }
}


proc getValueCommaCont(root, t) {
  set(gotValue, 0)
  if(root) {
    fornodes(root, node) {
      if(and(not(gotValue), not(strcmp(tag(node), t)))) {
        set(gotValue, 1)
        set(gottenNode, node)
        set(gottenValue, save(value(node)))
        fornodes(node, subnode) {
          if(not(strcmp("CONT", tag(subnode)))) {
            if(strlen(value(subnode))) {
              set(gottenValue, 
                save(concat(gottenValue, ",\n", value(subnode))))
            }
          } elsif(not(strcmp("CONC", tag(subnode)))) {
            if(strlen(value(subnode))) {
              set(gottenValue, 
              save(concat(gottenValue, value(subnode))))
            }
          }
        }
      }
    }
  }
}


proc aAn(s) {
  set(s, save(trim(lower(s), 1)))
  if(not(strcmp(s, "a"))) { "an" }
  elsif(not(strcmp(s, "e"))) { "an" }
  elsif(not(strcmp(s, "i"))) { "an" }
  elsif(not(strcmp(s, "o"))) { "an" }
  elsif(not(strcmp(s, "u"))) { "an" }
  elsif(not(strcmp(s, "x"))) { "an" }
  else { "a" }
}


func printablefirstname(i) {
   set(firstname, givens(i))
   if(eq(strcmp(firstname, ""), 0)) {
      set(namereturn, save("\\noname"))
   } else {
      set(where, index(firstname, " ", 1))
      /* don't print out middle names */
      if(gt(where, 0)) {
         set(namereturn, save(substring(firstname, 1, sub(where, 1))))
      } else {
         set(namereturn, save(firstname))
      } /* if no middle names */
      set(namereturn, strxlat(tex_xlat, namereturn))
   }
   return(namereturn)
}

proc iswas(indi) {
  call setDayNumber(birth(indi))
  if(or(death(indi), or(not(dayNumber), lt(dayNumber, 693971)))) {
    "\nwas" 
  } else {
    "\nis"
  }
}


/* Check to see if this family might still have children at some point:
   If one spouse is dead, too old, or they are divorced; then they won't.
*/
proc havehadchildren(indi, spouse) {
  call setDayNumber(birth(indi))
  set(indiDayNumber, dayNumber)
  call setDayNumber(birth(spouse))
  set(divp, 0)
  spouses(indi, s, f, n) {
    if(eq(s, spouse)) {
      call getValue(fnode(f), "DIV")
      set(divp, gotValue)
      break()
    }
  }
  if(
    or(
      death(indi),
      or(not(indiDayNumber), lt(indiDayNumber, 693971)), death(spouse),
      or(not(dayNumber), lt(dayNumber, 693971)),
      divp
    )) {
    "\\ had no children.\n"
  } else {
    "\\ have no children.\n"
  }
}
 
/* illegit_check gives people a break.  If we have no marriage record,
   the assumption still is that the couple were married and that
   is the word we stick in the text.
   Only if it specifically says they were not married, do we
   state that they weren't
*/
proc illegit_check(fam) {
  set(not_married_flag, 0)
  if(e,marriage(fam)) { 
    if(d, date(e)) {  
      if(not(strcmp(d, "Not married"))) {
        set(not_married_flag, 1)
      }
    }
  }
}


proc sourceIt(sourceList) {
  list(cList)
  list(fList)
  set(cn, 0)
  while(root, dequeue(sourceList)) {
    fornodes(root, node) {
      if(not(strcmp( tag(node), "SOUR"))) {
        set(footnote, 1)
        set(val, value(node))
        if(val) {
          if(reference(val)) {
            call bibliographize(dereference(val))
          }
        }
        if(xref(node)) {
          call bibliographize(node)
          set(val, xref(node))
        }
        if(val) {
          set(a1, index(val, "@", 1))
          set(a2, index(val, "@", 2))
          if(and(eq(a1, 1), eq(a2, strlen(val)))) { 
            set(c, save(substring(val, 2, sub(strlen(val), 1))))
            enqueue(cList, c)
            incr(cn)
            set(footnote, 0)
          }
        } else {
          set(subnodecount, 0)
          fornodes(node, subnode) {
            if(strcmp(tag(subnode), "SOUR")) {
              incr(subnodecount)
            }
          }
          if(eq(subnodecount, 0)) {
            fornodes(node, subnode) {
              set(val, value(subnode))
              /* With loadsources, this is needed here. It is technically
                 illegal gedcom.
              */
              if(xref(subnode)) {
                call bibliographize(subnode)
                set(val, xref(subnode))
              }
              if(val) {
                set(a1, index(val, "@", 1))
                set(a2, index(val, "@", 2))
                if(and(eq(a1, 1), eq(a2, strlen(val)))) { 
                  set(c, save(substring(val, 2, sub(strlen(val), 1))))
                  enqueue(cList, c)
                  incr(cn)
                }
              } 
            }
            set(footnote, 0)
          }
        }
        if(footnote) {
          enqueue(fList, node)
        }
      }
    }
  }
  while(cn) {
    forlist(cList, c, n) {
      if(and(ne(n, cn), not(strcmp(c, getel(cList, cn))))) {
        setel(cList, cn, "")
      }
    }
    decr(cn)
  }
  if(not(empty(fList))) {
    "\n\\footnote{" 
    while(f, dequeue(fList)) { 
      set(first, 1)
      call getValueCont(f, "TITL") 
      if(gotValue) { 
        if(not(first)) { ", " } else { set(first, 0) }
        "\n"
        strxlat(tex_xlat, gottenValue)
      }
      call getValueCont(f, "DATE") 
      if(gotValue) { 
        if(not(first)) { ", " } else { set(first, 0) }
        "\n"
        strxlat(tex_xlat, gottenValue)
      }
      call getValueCont(f, "PLAC") 
      if(gotValue) { 
        if(not(first)) { ", " } else { set(first, 0) }
        "\n"
        strxlat(tex_xlat, gottenValue)
      }
      call getValueCont(f, "VOLU") 
      if(gotValue) { 
        if(not(first)) { ", " } else { set(first, 0) }
        if(
          or(index(gottenValue, "-", 1),
            index(gottenValue, ",", 1),
            index(gottenValue, "and ", 1)
          )) {
          "\nVolumes "
        } else {
          "\nVolume "
        }
        strxlat(tex_xlat, gottenValue)
      }
      call getValueCont(f, "PAGE") 
      if(gotValue) { 
        if(not(first)) { ", " } else { set(first, 0) }
        if(
          or(index(gottenValue, "-", 1),
            index(gottenValue, ",", 1),
            index(gottenValue, "and ", 1)
          )) {
          "\nPages "
        } else {
          "\nPage "
        }
        strxlat(tex_xlat, gottenValue)
      }
      call getValueCont(f, "FILM") 
      if(gotValue) { 
        if(not(first)) { ", " } else { set(first, 0) }
        "on Latter Day Saints Microfilm Number " 
        strxlat(tex_xlat, gottenValue)
      }

      call getValueCont(f, "TEXT") 
      if(gotValue) { 
        set(first, 0)
        "\n"
        strxlat(tex_xlat, gottenValue)
      }
      if(not(first)) { "\\@." }
      call getValueCont(f, "NOTE") 
      if(gotValue) { 
        set(first, 0)
        "\n"
        strxlat(tex_xlat, gottenValue)
      }
      if(and(first, not(value(f)))) { "\n" }
      call values(f)
    }
    "}"
  }
  if(not(empty(cList))) {
    "\\cite{"
    while(c, dequeue(cList)) {
      if(strlen(c)) {
        if(cn) { "," }
        c
        incr(cn)
      }
    }
    "}"
  }
}

proc bibliographize(root) {
  set(val, xref(root))
  set(c, save(substring(val, 2, sub(strlen(val), 1))))

  if(not(lookup(bibTable, c))) {
    insert(bibTable, c, 1)

/*  
    call getValueCont(root, "TEXT")
    if(figureFlag, gotValue) {
      enqueue(figureCiteList, c)
      enqueue(figureNodeList, gottenNode)
    }
*/
    set(cref, save(concat("\\protect\\ref{", c, "}")))
    set(pref, save(concat("\\protect\\pageref{", c, "}")))

    set(b, "\\bibitem")
    if(figureFlag) {
      set(b, save(concat(b, "[", cref, "]")))
    }
    set(b, save(concat(b, "{", c, "} ")))
    call getValueCont(root, "TITL") 
    if(gotValue) { 
      set(b, save(concat(b, "{\\em ", strxlat(tex_xlat, gottenValue), "}, ")))
    }
    call getValueCont(root, "AUTH") 
    if(gotValue) { 
      set(b, save(concat(b, " ", strxlat(tex_xlat, gottenValue), ", ")))
      set(authnode, gottenNode)
      call getValueCont(authnode, "EMAI")
      if(gotValue) {
        if(opt_email) {
          set(b, save(concat(b, strxlat(tex_xlat, gottenValue), ", ")))
        } else {
          set(b, save(concat(b, "e-mail address on file, ")))
        }
      }
    }
    call getValueCont(root, "PUBL") 
    if(gotValue) { 
      set(pubnode,gottenNode)
      call getValueCont(pubnode, "NAME") 
      if(gotValue) { 
        set(b, save(
          concat(b, "in {\\em ", strxlat(tex_xlat, gottenValue), "}, ")))
      }
      call getValueCommaCont(pubnode, "ADDR") 
      if(gotValue) {
        set(b, save(
          concat(b, strxlat(tex_xlat, gottenValue), ": ")))
        }
      call getValueCont(pubnode, "PUBR") 
      if(gotValue) {
        set(b, save(
          concat(b, strxlat(tex_xlat, gottenValue), ", ")))
      }
      call getValueCont(pubnode, "PHON") 
      if(gotValue) {
        set(b, save(concat(b, strxlat(tex_xlat, gottenValue), ", ")))
      }
      call getValueCont(pubnode, "DATE") 
      if(gotValue) {
        set(b, save(
          concat(b, strxlat(tex_xlat, gottenValue), ", ")))
      }
      call getValueCont(pubnode, "VOLU") 
      if(gotValue) { 
        set(word, "Volume ")
        if(
          or(index(gottenValue, "-", 1),
            index(gottenValue, ",", 1),
            index(gottenValue, "and ", 1)
          )) {
          set(word, "Volumes ")
        }
        set(b, save(concat(b, word, strxlat(tex_xlat, gottenValue), ", ")))
      }
      call getValueCont(pubnode, "NUM") 
      if(gotValue) { 
        set(word, "Number ")
        if(
          or(index(gottenValue, "-", 1),
            index(gottenValue, ",", 1),
            index(gottenValue, "and ", 1)
          )) {
          set(word, "Numbers ")
        }
        set(b, save(concat(b, word, strxlat(tex_xlat, gottenValue), ", ")))
      }
      call getValueCont(root, "LCCN") 
      if(gotValue) { 
        set(b, save(
          concat(b, "Call Number ", strxlat(tex_xlat, gottenValue), ", ")))
      }
    }
    call getValueCont(root, "PAGE") 
    if(gotValue) { 
      set(word, "page ")
      if(
        or(index(gottenValue, "-", 1),
          index(gottenValue, ",", 1),
          index(gottenValue, "and ", 1)
        )) {
        set(word, "pages ")
      }
      set(b, save(concat(b, word, strxlat(tex_xlat, gottenValue), ", ")))
    }
    call getValueCont(root, "FILM") 
    if(gotValue) { 
      set(b, save(concat(b, 
         "Filmed by the Church of Jesus Christ of Latter Day Saints, ",
         "Microfilm Number ", 
         strxlat(tex_xlat, gottenValue), ", "))) 
    }
    call getValueCont(root, "FICH") 
    if(gotValue) { 
      set(b, save(concat(b, 
         "Filmed by the Church of Jesus Christ of Latter Day Saints, ",
         "Microfiche Number ", 
         strxlat(tex_xlat, gottenValue), ", "))) 
    }
    call getValueCont(root, "REPO") 
    if(gotValue) { 
      set(b, save(concat(b, "at ", strxlat(tex_xlat, gottenValue), ", ")))
    }

    if(index(b, ", ", 1)) {
      set(b, save(concat(save(substring(b, 1, sub(strlen(b), 2))), ".")))
    }
  
    call getValueCont(root, "NOTE") 
    if(gotValue) { set(b, save(concat(b, " ", strxlat(tex_xlat, gottenValue)))) }

    call getValueCont(root, "TEXT") 
    if(gotValue) { set(b, save(concat(b, " ", strxlat(tex_xlat, gottenValue)))) }
  
    call getValueCont(root, "HIDE") 
    if(gotValue) { set(b, save(concat(b, " ", strxlat(tex_xlat, gottenValue)))) }
  
    call getValueCont(root, "SOUR") 
    if(gotValue) { 
      set(bb, "?")
      if(gottenValue) {
        set(a1, index(gottenValue, "@", 1))
        set(a2, index(gottenValue, "@", 2))
        if(and(eq(a1, 1), eq(a2, strlen(gottenValue)))) { 
          set(bb, save(substring(gottenValue, 2, sub(strlen(gottenValue), 1))))
        }
      }
      set(b, save(concat(b, "\\cite{", strxlat(tex_xlat, bb), "}")))
    }
  
    if(figureFlag) {
      set(b, save(concat(b, " See figure on page~", pref, ".")))
    }
  
    /* This while loop undoes the line breaking in a CONT/CONC.
       Since those line breaks can be
       significant, it is commented out.
    */
/*
    while(i, index(b, "\n", 1)) {
      set(b, save(
        concat(
          substring(b, 1, sub(i, 1)),
          " ", 
          substring(b, add(i, 1), strlen(b))
        ) ))
    }
*/
    enqueue(bibList, save(concat(b, "\n")))
  }
}


proc getText(root, paragraph) {
  set(pronounOkay, 1)
  if(root) {
    if(eq(1,notes_text_mode)) {   /* only 1 TEXT records */
      fornodes(root, node) {
        if(not(strcmp("TEXT", tag(node)))) {
          set(pronounOkay, 0)
          if(paragraph) {
            "\n\n" set(paragraph, 0)
          }
          call values(node)
          "\n\n"
        }
      }
    } else {
      if(eq(2,notes_text_mode)) { /* all 1 NOTE records */
        fornotes(root, note) {
          if(paragraph) {
            "\n\n" set(paragraph, 0)
          }
          strxlat(tex_xlat, note)
          "\n\n"
          set(pronounOkay, 0)
        }
      } else {   /* only !-tagged 1 NOTE records (1st char must be !) */
        fornotes(root, note) {
          set(i, index(note,"!",1))
          if(eq(1,i)) {
            set(pronounOkay, 0)
            if(paragraph) {
              "\n\n" set(paragraph, 0)
            }
            strxlat(tex_xlat, substring(note, 2, strlen(note)))
            "\n\n"
          }
        }
      }
    }
  }
  /* if we printed any notes, then reset things so we don't use
     "there" in place names right after the notes.
  */
  if(not(pronounOkay)) {
    insert(eventPlaceTable, "@there@", "")
  }
}

proc values(root) {
  if(root) {
    if(strlen(value(root))) { "\n" strxlat(tex_xlat, value(root)) }
    fornodes(root, node) {
      if(not(strcmp("CONT", tag(node)))) {
        if(strlen(value(node))) {
          "\n" strxlat(tex_xlat, value(node))
        }
      } elsif(not(strcmp("CONC", tag(node)))) {
        if(strlen(value(node))) {
          strxlat(tex_xlat, value(node))
        }
      }
    }
    if(root) {
      enqueue(sourceList, root)
    }
    call sourceIt(sourceList)
  }
}


/********************************************************************/
/* below here are routines for printing descendant charts */
/* 
 * These are adapted from desc-tex
 * By Eric Majani (eric@elroy.jpl.nasa.gov)
 */

proc desc_chart_main3(indi) {
  "\\tree\n"
  call desc_chart_out(indi,3,1)
  "\\endtree\n"
}


proc descch_indi(indi) {
  "{\\bf " call desc_chart_name(indi) "}"
  if(or(birth(indi),death(indi))) {
    " "
    if(e, birth(indi)) {
      year(e)
    }
    "-"
    if(e, death(indi)) {
      year(e)
    }  
  }
  nl()
  families(indi,fam,sp,num) {
    if(e,marriage(fam)) {
      "   m. " short(e)  nl()
    } 
  }
}


proc desc_chart_name(i) {
  set(whole, givens(i))
  set(space,index(whole, " ", 1))
  if(gt(space, 0)) {
     strxlat(tex_xlat, substring(whole, 1, space))
  } else {
    strxlat(tex_xlat, whole) " "
  }
  if(eq(strcmp(surname(i), "____"), 0)) {
    " \\noname" 
  } else {
    strxlat(tex_xlat, surname(i))
  } 
}


proc descch_indinomar(indi) {
  "{\\bf " call desc_chart_name(indi) "}"
  if(or(birth(indi),death(indi))) {
    " "
    if(e, birth(indi)) {
      year(e)
    }
    "-"
    if(e, death(indi)) {
      year(e)
    }  
  }
  nl()
}


proc descch_prcouple(indi,fam,num) {
  if(eq(num,1)) {
    "{\\bf "
    call desc_chart_name(indi)
    "}" 
    if(e, birth(indi)) {
      " " year(e)"-"
    }
    if(e, death(indi)) {
      if(not(birth(indi))) {
        " -"
      }
      year(e)  
    }
  }
  nl()

/* I can't remember why I put this IF in here. I guess I'll take it out
   and see what breaks!
*/
/*
  if(eq(num,nfamilies(indi))) {
*/ 
    if(e,marriage(fam)) {
      " m. " year(e) " "
    } else {
      "m.\\ \\ \\ \\ \\ \\ "
    } /* space over without date */
  /*}*/
}


proc descch_printfam(indi,fam,sp) {
  "\\spouse{ " call desc_chart_name(sp) "}" nl()
}


proc desc_chart_out(indi,depth,level) {
  if(or(eq(0,nfamilies(indi)),eq(depth,level))) {
    call descch_indinomar(indi)
  }
  if(lt(level,depth)) {
    families(indi,fam,sp,num) {         
      call descch_prcouple(indi,fam,num)
      call descch_printfam(indi,fam,sp)
      set(level,add(level,1))
      set(num2,0)
      if(le(level,depth)) {
        children(fam,child,num2) {
          "\\subtree " nl()
          call desc_chart_out(child,depth,level)
          "\\endsubtree " nl()
        }
      }
      set(num2,nchildren(fam))
      set(level,sub(level,1))
      set(temp1,ne(num,nfamilies(indi)))
      set(temp2,gt(num2,0))
      if(and(temp1,temp2))
/*
      if(ne(num,nfamilies(indi)))
*/
      {
        if(eq(level,1)) {
          "\\endtree " nl()
          "\\tree " nl()
        } else {
          "\\endsubtree " nl()
           "\\subtree "  nl()
        }
      }
    }
  }
}

proc excursion(indi) {
  list(anclist)
  indiset(ancset)
  
  "\n\\section{"  strxlat(tex_xlat, surname(indi)) " Ancestors" "}" "\n"
  "\\label{" key(indi) "-excur-ref}" "\n"

  /* get us to the patriarch of the line.*/
  set(thisguy,indi)
  addtoset(ancset,thisguy,0)
  while(father(thisguy)) {
    set(thisguy,father(thisguy))
    push(anclist,thisguy)
    addtoset(ancset,thisguy,0)
  }
  "The " strxlat(tex_xlat, surname(indi)) " line has been traced back to "
  call texname(inode(thisguy), 0) "."

  while(indi,pop(anclist)) {
    print("Excursion: ") print(name(indi)) print("\n")
    call longvitals(indi,1,2)
      families(indi, fam, spouse, nfam) {
        "\n\n"
        if(eq(0, nchildren(fam))) {
          call texname(inode(indi), 0) "\\ and "
          if(spouse) {
            call texname(inode(spouse), 0)
          } else {
            "\\noname"
          }
          call havehadchildren(indi, spouse)
        } elsif(and(spouse, lookup(stab, key(spouse)))) {
          "Children of " call texname(inode(indi), 0) "\\ and "
          call texname(inode(spouse), 0) "\\ are shown under "
          call texname(inode(spouse), 0)
          "(" d(lookup(stab, key(spouse))) ").\n\n"
        } else {
          "Children of " call texname(inode(indi), 0) "\\ and "
          if(spouse) {
            call texname(inode(spouse), 0)
          } else {
            "\\noname"
          }
          ":\n\\begin{childrenlist}\n"
          children(fam, child, nchl) {
          "\n\\item "
          set(personIsAnc,0)
          forindiset(ancset,them,val,num) {
            if(eq(them, child)) {
              set(personIsAnc,1)
            }
          }
          if(personIsAnc) {
            "**" call shortvitals(child)
          } else {
            call longvitals(child, 0,2)
          }
          addtoset(idex, child, 0)
        }
        "\\end{childrenlist}\n"
      }
    }
  }
  "\n\n\n"
}

/*
**  function: strxlat
**
**  This idea was copied and/or adapted from Jim Eggert's modification
**  to the ps-circ(le) program for LifeLines.
**  A typical call would look like:
**      set(str, strxlat(tex_xlat, name(person)))
**  which would translate characters in person's name according to the
**  table called tex_xlat -- which escapes the special characters being
**  displayed as text via LaTeX.  The output is assigned to str.
**  The output of strxlat() can also be sent directly to output.
**
*/

func strxlat(xlat, string) {
  if(opt_xlat) {
    set(fixstring, "")
    set(pos, 1)
    while(le(pos, strlen(string))) {
      set(char, substring(string, pos, pos))
      if(special, lookup(xlat, char)) {
        set(fixstring, concat(fixstring, special))
      } else {
        set(fixstring, concat(fixstring, char))
      }
      incr(pos)
    }
  } else {
    set(fixstring, string)
  }
  return(save(fixstring))  /* save() is used for compatibilty with older LL */
}

