/*
 * @progname       register-rtf.ll
 * @version        1.6
 * @author         Doug McCallum
 * @category       
 * @output         RTF
 * @description
 *
 * RTF based Register Report Generator.
 * This program has many options but basically takes a person
 * and generates an RTF document that can be read by a number
 * of word processors.  The document can optionally be cross-indexed
 * and footnoted.  The format is close to the NEHGS Register Form.
 *
 * Options are set by setting variables at the beginning of main
 * The options are:
 *     option          values
 *     -------------   ----------------------------------------------------
 *     doindex         0 == no index, 1 == create index
 *     prefix          a string.  This is prefixed to the standard
 *                     numbers for people
 *     donotes         0 == don't output notes, 1 == output notes in-line
 *                     2 == output notes at end with a reference in text
 *     author          a string used in the author info field
 *     strictness      0 == all descendents, 1 == modified form, 2 = strict
 *     childby         0 == list children together, 1 == indicate parent by
 *     titlepage       0 == no title page, 1 == generate title page
 *     dosources       0 == don't output sources, 1 == output sources
 *     occupation      0 == don't output occupation, 1 == output occupation
 *
 * Not implemented yet:
 *     showrefn        0 == don't show user refn tags, 1 == show tags
 *
 *
 * Notes:
 *     when an index is created, it must be turned on in the word processor
 *     since only the indexing is actually done.  Each time a name is seen
 *     it is indexed.  If the name is a reference to the person as child of,
 *     then it is indexed in plain form numbers.  If the person is a spouse
 *     the page number is italic and if the person is the first entry in
 *     the family info, then the page number is bold.
 *
 *     The "prefix" is intended for use when doing multifamily reports.
 *     Further work needs to be done, but it can get you quite a ways
 *     toward that end.  A future version of this program may handle
 *     the multi-family case directly.
 *
 *     If notes are done in-line, each NOTE is a new paragraph and blank
 *     lines mark paragraphs.  If done as endnotes, the NOTES are indicated
 *     with ids in the form [id1, id2] and then a Notes page created to
 *     print all the notes.
 *
 *     All main lines and the Generation lines are treated as headings and
 *     can be put into a table of contents. This is always done but the
 *     table is not inserted into the document.
 *
 *     If a marriage would occur multiple times, it is only referred
 *     to and not duplicated after the first time.  This is complicated
 *     by an indi having multiple marriages and the duplicated spouse
 *     also having multiple marriages with only one marriage in the duplicate
 *     tree. These are called out as well.
 *
 *     I don't plan to implement the generation tags (superscript generation
 *     numbers) on individuals at this point.  They are easy enough to do
 *     since the current generation is always known but I find them not
 *     particularly useful in the text.
 *
 *     The program can take a long time since it manipulates a lot of data.
 *     Future versions may improve performance as better algorithms are
 *     found.
 *
 *     An individual's occupation(s) should be output into the report
 *     but isn't currently done.  This probably should be an option along
 *     with the specific format to use.
 *
 *     A future version should replace all occurrences of English
 *     text with variable references to make translation easier. Mostly
 *     this is the label stuff (b., d., bur., children, etc.).  There should
 *     also be an option to expand the abbreviations to the full word.
 *
 *     At some point, information such as the various baptism, christening,
 *     and other event information will be included as well.  This will
 *     most likely be done as hidden text of some form or perhaps as
 *     annotations.  This will make the text available but it would have
 *     to be turned on in the document.
 *
 *     Sources should eventually be turned into endnotes so Word can do
 *     any special processing.
 */
include("rtflib")
global(childby)
global(curr_index)
global(doindex)
global(donotes)
global(dosources)
global(endtags)
global(generation)
global(inum_set)
global(nextgen)
global(prev_fam_list)
global(strictness)
global(taglist)
global(titlepage)
global(all_sources)
global(occupation)

proc main()
{
       /* program options */
       set(doindex, 1)         /* set to zero if you don't want an index */
       set(prefix, "")         /* string to prefix indi number with */
       set(donotes, 1)         /* 0 = no notes, 1 = inline, 2 = end */
       set(dosources, 1)       /* 0 = no sources, 1 = sources */
       set(childby, 1)         /* 0 = all children, 1 = children by spouse */
       set(strictness, 0)      /* all descendents, including female lines */
       set(occupation, 1)
       set(author, "Author/Compiler Name") /* default author name */
       set(titlepage, 1)       /* want a title page */

       set(now, gettoday())
       dayformat(1)
       monthformat(1)
       dateformat(8)
       set(created, stddate(now))

       /* program proper */

       /* initialize the variables used */

       indiset(inum_set)       /* this set keeps the family numbers */
       list(prev_fam_list)     /* keeps track of marriages to avoid dups */
       list(generation)        /* current generation being processed */
       list(nextgen)           /* next generation to process */
       table(endtags)          /* if notes at end, keep track of tags */
       list(taglist)           /* the tags in order created */
       set(curr_index, 0)      /* current index for indi/family */
       set(curr_gen, 0)        /* current generation counter */
       set(indi,0)
       list(all_sources)       /* contains list of all sources referenced */

       getindi(indi)
       if (not(indi)) {
               /* if no one selected, exit */
               return (0)
       }
       /*
        * initialize the RTF file for standard paper (default)
        */
       call rtf_open(0)
       set(title, concat("Descendents of ", fullname(indi, 0, 1, 128)))
       call rtf_set_info(title, name(indi), author, author, created)
       /*
        * define default table size  of 6in with 4 columns of
        * .125in, .375in, .375in, 5.125in
        */
       call rtf_set_row_width(4, 8640) /* 6in * 1440 */
       call rtf_set_col_width(540)     /* .45in * 1440 */
       call rtf_set_col_width(720)     /* .500in * 1440 */
       call rtf_set_col_width(540)     /* .45in * 1440 */
       /* want a footer with page number centered */
       call rtf_footer(0, 2)
       if (dosources) {
               /* if doing sources, what footnote type */
               call rtf_ftn_type(1, 0) /* everything at end */
       }

       /* add the first person to the list of people to process */
       /* this primes the pump, so to speak */
       call next_indi(indi)
       enqueue(nextgen, indi)
       if (titlepage) {
               call dotitle(author, title)
       }

       /*
        * all is setup to go down the descendency list.
        * continue until all are individuals are processed.
        * note that nextgen is the next generation to process
        * and generation is the current one.
        * both are queues so we keep order.  Basically, all
        * children of the person being processed are appended
        * to the end of the nextgen list.
        */

       while (or(length(generation), length(nextgen))) {
               /*
                * if no current generation but a nextgen exists,
                * start a new paragraph with header for the Generation
                * and make the nextgen the current generation
                */
               if (empty(generation)) {
                       call rtf_para_indent(0, 0)
                       call rtf_pstart(1)
                       call rtf_para_keepnext()
                       call rtf_hstart()
                       set(curr_gen, add(curr_gen, 1))
                       capitalize(ord(curr_gen))
                       " Generation\n"
                       call rtf_hend()
                       call rtf_pend()
                       set(generation, nextgen)
                       list(nextgen)
               }
               /*
                * the real work is done in out_register
                * get the next person to process and then
                * let out_register do the work.
                */
               set(indi, dequeue(generation))
               call out_register(indi)
       }
       if (dosources) {
               call dump_sources()
       }
       if (eq(donotes, 2)) {
               /* end notes need to be dumped if they exist */
               call endnotes()
       }
       call rtf_close()
}

/*
 * out_register( indi )
 *     outputs the standard register format for the
 *     individual.  Any children get added to nextgen
 *     if they have families.  Global variables are used
 *     to modify the exact output.
 */
proc out_register(indi)
{
       /*
        * We always start a new paragraph with hanging indent for the number.
        * It is then tagged to be kept intact to avoid splitting across pages.
        * The indivual's number if found and printed and then the name
        * is output followed by marriage(s), birth, death, etc.
        */
       call rtf_pstart(3)
       call rtf_para_indent(neg(540), 540)
       call rtf_para_keepintact()
       d(inum(indi)) "."
       call rtf_tab(0)
       call rtf_bold(1)
       fullname(indi, 0, 1, 128)
       /* the individual's name is entered as a level 2 TOC header */
       call rtf_toc_entry(2, fullname(indi, 0, 1, 128))
       call rtf_bold(0)
       if (doindex) {
               /*
                * optional indexing, main entry is bold
                * It would be nice to have index same text
                * as the output name but the format is different.
                * Need to find a better way to tie together so edit
                * will do the right thing.
                */
               rtf_index(surname(indi), givens(indi), 1)
       }
       /*
        * it is important to not print duplicate marriages since in some
        * families this can lead to excessive information.  In my own,
        * there were 5 children of one ancestor married 5 children of
        * another ancestor.  Over seven generations there have been
        * additional crossings of the lines and a non-pruned tree is HUGE
        *
        * There are two types of pruning of this type that need to be
        * considered.  The first is the simple case of a single marriage
        * that is duplicated.  It can be handled with a simple reference
        * to the first occurrence. The second type is more complex where
        * there are multiple marriages.  Some of the marriages may be
        * duplicates and need to be pruned, but some may be ones that
        * haven't been seen yet.  This occurs when person A marries person B.
        * The implication that B is married to A.  If B is also married to C
        * the multiple case occurs.  In this case if B is seen as a child
        * and the family info is about to be output, the marriage to A is
        * a duplicate but the one to C is not.
        */
       if (check_marriages(indi)) {
               /*
                * duplicate marriages (common ancestors)
                * are not duplicated but referred to the only
                * instance.  If there were multiple marriages,
                * then the duplicates can be referred to but the
                * non-duplicate ones need expansion.
                */
               call rtf_italic(1)
               set(prev, prevmarr(indi))
               "(See marriage to "
               set(f, getel(prev_fam_list, prev))
               if (male(indi)) { set(s, wife(f)) }
               else            { set(s, husband(f)) }
               fullname(s, 0, 1, 128)
               if (and(doindex, surname(s))) {
                       rtf_index(surname(s), givens(s), 2)
               }
               ", number " prefix d(inum(s)) ")"
               call rtf_italic(0)
               call rtf_para_space(0, 120)
               call rtf_pend()
       } else {
               /* not a complete duplicate so generate lots of text */
               call rtf_para_keepnext()
               if(e, birth(indi)) {
                       " b. " long(e)
                       if (dosources) {
                               call do_sources(e)
                       }
               }
               /*
                * this is an attempt to track duplicate marriages.
                * It needs to be looked at more carefully.
                */
               set(nmarr, nfamilies(indi))
               /*
                * run through all of this person's families
                */
               families (indi, famvar, spvar, cnt) {
                  if (spvar) {
                       set(prev, check_prev(famvar))
                       if (not(prev)) {
                               /* save for future reference */
                               enqueue(prev_fam_list, famvar)
                       }
                       /*
                        * basic format of marriage is
                        * m. [date][[,] place] [(mnum)] spouse
                        * [b. [date][, place]]
                        * ([daughter|son] of spouse's parents)
                        * [d. [date][, place]] [bur. [date][, place]].
                        */
                       /* If first spouse, use a ';' but ',' for rest */
                       if (eq(1, cnt)) {
                               "; m."
                       } elsif (ne(0, cnt)) {
                               ", m."
                       }
                       if (not(prev)) {
                               if (e, marriage(famvar)) {
                                       " " long(e)
                                       if (dosources) {
                                               call do_sources(e)
                                       }
                                       ","
                               }
                       }
                       if (gt(nmarr, 1)) {
                               " (" d(cnt) ")"
                       }
                       if (not(prev)) {
                               " " fullname(spvar, 0, 1, 128)
                               if (and(doindex, surname(spvar))) {
                                       rtf_index(surname(spvar),
                                       givens(spvar), 2)
                               }
                               set(items, 0)
                               if (e, birth(spvar)) {
                                       " b. " long(e)
                                       if (dosources) {
                                               call do_sources(e)
                                       }
                                       set(items, 1)
                               }
                               /*
                                * we know parents so give a referral.
                                * in a future version, this should be updated
                                * to determine if spouses had common ancestor
                                * and give the family number cross-reference.
                                * This would apply to my own genealogy.
                                */
                               if (f, parents(spvar)) {
                                       call rtf_italic(1)
                                       if (male(spvar)) {
                                               " (son of "
                                       } else {
                                               " (daughter of "
                                       }
                                       set(j, "")
                                       if (f, father(spvar)) {
                                               set(j, " and ")
                                               fullname(f, 0, 1, 128)
                                       }
                                       if (f, mother(spvar)) {
                                               j fullname(f, 0, 1, 128)
                                       }
                                       ")"
                                       call rtf_italic(0)

                                       /* spouse's death info */
                                       if (e, death(spvar)) {
                                               if (eq(items, 1)) {
                                                       ", d. "
                                               } else {
                                                       " d. "
                                               }
                                               long(e)
                                               if (dosources) {
                                                       call do_sources(e)
                                               }
                                               set(items, 1)
                                       }

                                       /* spouse's burial info */
                                       if (e, burial(spvar)) {
                                               if (eq(1, items)) {
                                                       ", bur. "
                                               } else {
                                                       " bur. "
                                               }
                                               long(e)
                                               if (dosources) {
                                                       call do_sources(e)
                                               }
                                               set(items, 1)
                                       }
                               }
                       } else {
                               fullname(spvar, 0, 1, 128)
                               call rtf_italic(1)
                               " (see marriage to number " d(inum(spvar))
                               ")"
                               call rtf_italic(0)
                       }
                   }
               }
               if (gt(cnt,0)) {
                       ".\n"
               }
               /* indi's remaining information */
               if (e, death(indi)) {
                       " " givens(indi) " died " long(e)
                       if (dosources) {
                               call do_sources(e)
                       }
                       if (e, burial(indi)) {
                               " and was buried " long(e)
                               if (dosources) {
                                       call do_sources(e)
                               }
                       }
                       ".\n"
               } elsif (e, burial(indi)) {
                       " " givens(indi) " was buried " long(e)
                       if (dosources) {
                               call do_sources(e)
                       }
                       ".\n"
               }
               /*
                * all occupations are given if any are found.
                */
               if (occupation) {
                       call do_occupation(indi)
               }
               /* if doing notes, make sure we get them now */
               if (donotes) {
                       call do_notes(indi, donotes, 0)
                       families(indi, famvar, spvar, cnt) {
                               if (spvar) {
                                       call do_notes(spvar, donotes, 0)
                               }
                       }
               }
               /*
                * now the children
                * starting a table is a new paragraph.  Keep it all together
                * and put in a label cell in first row.  Then dump
                * each child into a row.
                */
               call rtf_tstart(4)
               call rtf_para_keepnext()
               if (or(eq(nmarr, 1), not(childby))) {
                       call rtf_cstart()
                       call rtf_cstart()
                       call rtf_cstart()
                       call rtf_cstart()
                       " Children:"
                       call rtf_cend()
               }
               set(numchildren, 1)
               set(tsize, totalchildren(indi))
               set(fcnt, 0)
               families(indi, famvar, spvar, cnt) {
                       /*
                        * if childby is set, then put spouse info
                        * out to identify which family children
                        * came from.  Skip families with no children
                        */
                       if (and(childby, gt(nmarr, 1))) {
                               if (not(nchildren(famvar))) {
                                       continue()
                               }
                               incr(fcnt)
                               if (gt(fcnt, 1)) {
                                       call rtf_endrow()
                                       call rtf_endrow()
                               }
                               call rtf_cstart()
                               call rtf_cstart()
                               call rtf_cstart()
                               call rtf_cstart()
                               " Children with "
                               fullname(spvar, 0, 1, 128)
                               ":"
                               call rtf_cend()
                       }
                       children (famvar, ch, num) {
                               /* want to know if this is someone to expand */
                               set(ival, determine(ch, indi))
                               call rtf_endrow()
                               if (lt(numchildren, tsize)) {
                                       call rtf_para_keepnext()
                               }
                               /* note that nothing goes in cell 1 */
                               call rtf_cstart()
                               /* start the cell where we do a number */
                               call rtf_cstart()
                               call rtf_para_rightjust()

                               /* if the indi is non-zero, then tag it */
                               if (ne(ival, 0)) {
                                       person_prefix d(ival) "."
                               }

                               /* the roman numeral/child order cell */
                               call rtf_cstart()
                               call rtf_para_rightjust()
                               roman(numchildren) "."

                               /* the name and info cell */
                               call rtf_cstart()
                               call rtf_para_leftjust()
                               fullname(ch, 0, 1, 128)
                               if (doindex) {
                                       rtf_index(surname(ch), givens(ch), 0)
                               }

                               /* we always give birth info */
                               if (b, birth(ch)) {
                                       if (gt(ival, 0)) {
                                               if (strlen(date(b))) {
                                                       " b. "
                                                       date(b)
                                               }
                                       } else {
                                               " b. "
                                               long(b)
                                               if (dosources) {
                                                       call do_sources(e)
                                               }
                                       }
                               }

                               /*
                                * if a non-expanded indi, give more info
                                * such as death, marriages, etc.  If
                                * expanded, don't since the full record
                                * will contain it.
                                */
                               if (eq(ival, 0)) {
                                       if (e, death(ch)) {
                                               if (b) { "," }
                                               " d. " long(e)
                                               if (dosources) {
                                                       call do_sources(e)
                                               }
                                       }
                                       set(nsp, nfamilies(ch))
                                       /* all known spouses */
                                       spouses(ch, sp, fm, cnt) {
                                               "; m. "
                                               if (gt(nsp, 1)) {
                                                       "(" d(cnt) ") "
                                               }
                                               if (e, marriage(fm)) {
                                                       long(e)
                                                       if (dosources) {
                                                               call do_sources(e)
                                                       }
                                                       ", "
                                               }
                                               fullname(sp, 0, 1, 128)
                                               if (doindex) {
                                                       rtf_index(surname(sp),
                                                               givens(sp), 2)
                                               }
                                       }
                                       ". "
                                       if (donotes) {
                                               call do_notes(ch, donotes, 1)
                                       }
                               } else {
                                       "."
                               }
                               call rtf_cend()
                               incr(numchildren)
                       }
               }
               call rtf_tend()
               call rtf_pend()
       }
       call rtf_para_space(0, 0)
}

/*
 * next_indi(indi)
 *     find the next unique number for this individual
 *     the global curr_index keeps the current value
 *     the inum_set keeps track of the indi/number pairs
 */
proc next_indi(indi)
{
       set(curr_index, add(curr_index, 1))
       addtoset(inum_set, indi, curr_index)
}

/*
 * inum(indi)
 *     find the unique number for this indi
 *     if there is one it is in inum_set
 *     zero is returned if there isn't a mapping
 */
func inum(indi)
{
       forindiset(inum_set, indvar, inumval, cnt) {
               if (eq(indvar, indi)) {
                       return (inumval)
               }
       }
       return (0)
}

/*
 * find_fam(indi, spouse)
 *     find the family (fam) indi and spouse create
 */
func find_fam(indi, sps)
{
       spouses (indi, s, f, c) {
               if (eq(sps, s)) {
                       return (f)
               }
       }
}

/*
 * check_marriages(indi)
 *     check to see if an individual has any marriages and return
 *     the inum of the first spouse that has one
 */
func check_marriages(indi)
{
       set(res, 0)
       set(notyet, 0)
       families (indi, f, s, c) {
               if (x, check_prev(f)) {
                       incr(res)
               } else {
                       incr(notyet)
               }
       }
       if (and(res, not(notyet))) {
               return (1)
       } else {
               return (0)
       }
}

/*
 * check_prev(fam)
 *     check to see if a previous marriage and return non-zero
 *     if there was one and zero if none.
 */
func check_prev(fam)
{
       forlist(prev_fam_list, f, cnt) {
               if (eq(fam, f)) {
                       return (cnt)
               }
       }
       return (0)
}

/*
 * determine(indi, par)
 *     determine if the indi is one to expand.
 *     The par is the parent descended from so that
 *     female lines can be skipped if strictness is
 *     set.
 */
func determine(indi, par)
{
       if (and(eq(strictness, 2), female(indi))) {
               /* strictest form doesn't follow female lines */
               return (0)
       }
       if (and(eq(strictness, 1), female(par))) {
               /* modified form gives one generation from a female line */
               return (0)
       }
       set(nchil, 0)
       families (indi, fm, sp, cnt) {
               set(nchil, add(nchil, nchildren(fm)))
       }
       if (gt(nchil, 0)) {
               enqueue(nextgen, indi)
               call next_indi(indi)
               return (inum(indi))
       }
       return (0)
}

/*
 * do_notes(indi, where, type)
 *     where is inline vs. end
 *     type is in or out of table
 */
proc do_notes(indi, where, type)
{
       /* where == 1 is inline */
    if (eq(where, 1)) {
       set(didpara, 0)
       set(innote, 0)
       set(root, inode(indi))
       traverse(root, node, level) {
               if (and(innote, le(level, innote))) {
                       set(innote, 0)
               }
               if (eqstr(tag(node), "NOTE")) {
                       if (not(type)) {
                               call rtf_pstart(3)
                       } else {
                               call rtf_cpar()
                       }
                       set(innote, level)
                       call fixstring(value(node))
                       set(didpara, 1)
               } elsif (eqstr(tag(node), "CONT")) {
                       if (innote) {
                               if (eq(0, strlen(value(node)))) {
                                   if (not(type)) {
                                       call rtf_pstart(3)
                                   } else {
                                       call rtf_cpar()
                                   }
                               } else {
                                       " \n"
                                       call fixstring(value(node))
                               }
                       }
               }
       }
    } elsif (eq(where, 2)) {   /* where == 2 is at end */
       set(found, 0)
       set(tagprefix, 0)
       set(root, inode(indi))
       traverse(root, node, level) {
               if (eqstr(tag(node), "NOTE")) {
                       if (not(found)) {
                               " ["
                       } else {
                               ", "
                       }
                       incr(found)
                       if (not(tagprefix)) {
                               set(tagprefix, tagname(indi))
                       }
                       tagprefix d(found)
               }
       }
       if (found) {
               "]"
       }
    }
}

/*
 * fixstring(str)
 *     fix the string to not break RTF output
 *     Any {, }, or \ characters must be escaped.
 *     Then output the string
 */
proc fixstring(str)
{
       if (i, index(str, "{", 1)) {
               call fixstring(substring(str, 1, i))
               "\\{"
               incr(i)
               call fixstring(substring(str, i, sub(strlen(str), i)))
       } elsif (i, index(str, "}", 1)) {
               call fixstring(substring(str, 1, i))
               "\\}"
               incr(i)
               call fixstring(substring(str, i, sub(strlen(str), i)))
       } elsif (i, index(str, "\\", 1)) {
               call fixstring(substring(str, 1, i))
               "\\\\"
               incr(i)
               call fixstring(substring(str, i, sub(strlen(str), i)))
       } else {
               str
       }
}

/*
 * prevmarr(indi)
 *     determine if an indi had a previously output marriage.
 */
func prevmarr(indi)
{
       spouses (indi, s, f, c) {
               forlist (prev_fam_list, fm, cnt) {
                       if (eq(f, fm)) {
                               return (cnt)
                       }
               }
       }
       return (0)
}

/*
 * dotitle(author, title)
 */
proc dotitle(author, title)
{
       "\\titlepg"
       "\\pvmrg\\posy2880\\qc\\fs48 "
               title
               "\\line\\line "
               "\\fs32 by\\line "
               author
       "\\par\\sect\\pgnrestart\n"
}

/*
 * tagname(indi)
 *     from an indi, create a unique tag to use for notes references
 *     for endnote form.
 */
func tagname(indi)
{
       list(parts)
       /*
        * the algorithm is:
        *      first 3 letters of surname
        *      first letter of first and any middle name
        *      if conflict, try adding "a", "b", etc. until unique.
        */
       extractnames(inode(indi), parts, nparts, surpart)
       set(surnm, substring(getel(parts, surpart), 1, 3))
       set(firstp, substring(getel(parts, 1), 1, 1))
       if (gt(nparts, 2)) {
               set(midp, substring(getel(parts, 2), 1, 1))
               if (not(strcmp(midp, "\""))) {
                       set(midp, substring(midp, 2, 2))
               }
       } else {
               set(midp, "")
       }
       set(tagvar, concat(surnm, firstp, midp))
       set(suffix, "")
       set(v, 0)
       while (lookup(endtags, concat(tagvar, suffix))) {
               incr(v)
               set(suffix, substring("abcdefghijklmnopqrstuvwxyz",
                               v, v))
       }
       insert(endtags, tagvar, indi)
       call sorttag(tagvar)
       set(tagvar, concat(tagvar, suffix))
       return (tagvar)
}

/*
 * sorttag(str)
 *     do an insertion sort of str into the taglist list of notes
 */
proc sorttag(str)
{
       list(tmp)
       set(done, 0)
       set(any, 0)
       while (l, dequeue(taglist)) {
           set(any, 1)
           if (not(done)) {
               set(r, strcmp(str, l))
               if (le(r, 0)) {
                       set(done, 1)
                       enqueue(tmp, str)
               }
               if (ne(r, 0)) {
                       enqueue(tmp, l)
               }
           } else {
               enqueue(tmp, l)
           }
       }

       if (or(not(any), not(done))) {
               enqueue(tmp, str)
               set(any, 1)
       }

       /* set to null so we can copy the new list */
       list(taglist)
       if (any) {
               while (l, dequeue(tmp)) {
                       enqueue(taglist, l)
               }
       }
}

/*
 * endnotes()
 *     at end, dump the endnotes in a reasonable format
 */
proc endnotes()
{
       call rtf_newpage()
       call rtf_para_indent(0, 0)
       call rtf_pstart(1)
       call rtf_hstart()
       "Notes"
       call rtf_hend()
       call rtf_pend()
       while (l, dequeue(taglist)) {
               set(indi, lookup(endtags, l))
               if (indi) {
                       call dumpnote(indi, l)
               }
       }
}

/*
 * dumpnote(indi, tagstr)
 *     dump the notes for this indi, using tagstr as the prefix
 */
proc dumpnote(indi, tagstr)
{
       set(didpara, 0)
       set(innote, 0)
       set(root, inode(indi))
       set(which, 0)
       traverse(root, node, level) {
               if (and(innote, le(level, innote))) {
                       set(innote, 0)
               }
               if (nestr(tag(node), "NOTE")) {
                       call rtf_pstart(3)
                       call rtf_para_indent(neg(1440), 1440)
                       incr(which)
                       set(innote, level)
                       tagstr d(which)
                       rtf_tab(0)
                       value(node)
                       set(didpara, 1)
               } elsif (nestr(tag(node), "CONT")) {
                       if (innote) {
                               if (eq(0, strlen(value(node)))) {
                                       call rtf_pstart(3)
                               } else {
                                       " \n" value(node)
                               }
                       }
               }
       }
}

/*
 * totalchildren(indi)
 *     count all the children this indi had
 */
func totalchildren(indi)
{
       set(total, 0)
       families (indi, fam, sp, cnt) {
               set(total, add(total, nchildren(fam)))
       }
       return (total)
}

/*
 * do_sources(e)
 *     find all the sources associated with the event
 *     and create the footnote reference.  If dosources is
 *     greater than 1, just gather the footnotes to stick at
 *     the end of family rather than in-line for each event
 *     {mode 2 not implemented yet}
 */
proc do_sources(e)
{
       set(evlist, sources(e))
       list(taglist)
       while (s, dequeue(evlist)) {
               set(srcvar, fmt_source(s))
               set(taglist, source_process(srcvar))
       }
       if (not(empty(taglist))) {
               call rtf_super(1)
               set(pre, "")
               forlist(taglist, var, cnt) {
                       pre d(var)
                       set(pre, ", ")
               }
               call rtf_super(0)
       }
}

/*
 * fmt_source(s)
 *     for a source node, traverse it and put into a normalized
 *     reference/footnote format.  New forms should be added as
 *     necessary since there are lots of possibilities.
 */
func fmt_source(s)
{
       set(prefix, "")
       set(cont, "")
       set(result, "")
       set(title,0)
       set(sour, 0)
       set(dt, 0)
       set(text, 0)
       set(publ, 0)
       set(page, 0)
       traverse (s, node, l) {
               if (gt(l, 2)) {
                       continue()
               }
               if (reference(value(node))) {
                       set(indresult, fmt_source(dereference(value(node))))
               } else {
                       if (eq(l, 0)) {
                               continue()
                       } elsif (eqstr(tag(node), "SOUR")) {
                               set(sour, text_node(node))
                       } elsif (eqstr(tag(node), "TEXT")) {
                               set(text, text_node(node))
                       } elsif (eqstr(tag(node), "DATE")) {
                               set(dt, date(node))
                       } elsif (eqstr(tag(node), "TITL")) {
                               set(title, text_node(node))
                       } elsif (eqstr(tag(node), "PAGE")) {
                               set(page, concat("page ", value(node)))
                       }
               }
       }
       set(result, "")
       if (indresult) {
               set(result, indresult)
               set(prefix, ", ")
       }
       if (title) {
               set(result, concat(result, prefix, title))
               set(prefix, ", ")
       }
       if (sour) {
               set(result, concat(result, prefix, sour))
               set(prefix, ", ")
       }
       if (dt) {
               set(result, concat(result, prefix, dt))
               set(prefix, ", ")
       }
       if (text) {
               set(result, concat(result, prefix, text))
               set(prefix, ", ")
       }
       if (publ) {
               set(result, concat(result, prefix, publ))
               set(prefix, ", ")
       }
       if (page) {
               set(result, concat(result, prefix, page))
               set(prefix, ", ")
       }
       return (result)
}

/*
 * sources(e)
 *     for an event, look for all source nodes and make a list
 *     to return.
 */
func sources(ev)
{
       list(evs)
       if (not(ev)) {
               return (evs)
       }
       set(cnt, 0)
       traverse(ev, node, lev) {
               if (eqstr(tag(node), "SOUR"))  {
                       enqueue(evs, node)
                       incr(cnt)
               }
       }
       return (evs)
}

/*
 * source_process(src)
 *     look for the string src in the list of known sources
 *     if it exists, use that index.  If it doesn't add to list
 *     and use the new index.  Then remove duplicate entries
 *     and ultimately return the list of uniqe references.
 */
func source_process(src)
{
       list(taglist)
       set(found,0)
       forlist(all_sources, str, cnt) {
               if (eqstr(str, src)) {
                       set(found, cnt)
                       break()
               }
       }
       if (not(found)) {
               enqueue(all_sources, src)
               incr(cnt)
               set(taglist, addtolist(taglist, cnt))
       } else {
               set(taglist, addtolist(taglist, found))
       }
       return (taglist)
}

/*
 * addtolist(lst, num)
 *     add the value "num" to the list "lst" if
 *     it isn't already there.
 */
func addtolist(lst, num)
{
       set(found, 0)
       list(newlist)
       forlist(lst, val, cnt) {
          if (not(found)) {
               if (eq(val, num)) {
                       return (lst)    /* no change - a dup */
               } elsif (gt(val, num)) {
                       set(found, 1)
                       enqueue(newlist, num)
                       enqueue(newlist, val)
               }
          } else {
               enqueue(newlist, val)
          }
       }
       if (not(found)) {
               enqueue(newlist, num)
       }
       return (newlist)
}
/*
 * dump_sources()
 *     dump the entire list of reference sources with proper tags.
 */
proc dump_sources()
{
       if (not(empty(all_sources))) {
               call rtf_pend()
               call rtf_newpage()
               call rtf_para_indent(0, 0)
               call rtf_pstart(1)
               call rtf_hstart()
                       "References"
               call rtf_hend()
               call rtf_pend()
               forlist(all_sources, src, num) {
                       call rtf_pstart(3)
                       call rtf_para_indent(neg(540), 540)
                       d(num)
                       call rtf_tab(0)
                       src
                       call rtf_pend()
               }
       }
}

/*
 * text_node(node)
 *     convert a text type node (TEXT or SOUR) into a long
 *     string with CONT entries separated by space.
 */
func text_node(node)
{
       set(result, "")
       set(prefix, "")
       traverse(node, n, l) {
               set(result, concat(result, prefix, value(n)))
               set(prefix, " ")
       }
       return (result)
}

/*
 * do_occupation(ind)
 *     print out occupation(s) of the individual in
 *     a meaningful form.
 */
proc do_occupation(indi)
{
       list(occu)
       set(count, 0)
       traverse (inode(indi), node, lev) {
               if (eqstr(tag(node), "OCCU")) {
                       /* have an occupation */
                       enqueue(occu, value(node))
                       incr(count)
               }
       }
       if (not(empty(occu))) {
               " "
               pn(indi, 0)
               " was a "
               set(sep, "")
               forlist(occu, item, cnt) {
                       item sep
                       if (eq(count, add(cnt, 1))) {
                               set(sep, ", and ")
                       } else {
                               set(sep, ", ")
                       }
               }
               ". "
       }
}
