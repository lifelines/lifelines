/*
 * @progname       paf-export.ll
 * @version        1994-11-12
 * @author         Kurt Baudendistel (baud@research.att.com)
 * @category       
 * @output         GEDCOM
 * @description
 *
 *      Convert lifelines-standard gedcom to paf gedcom.
 *      This report generates paf-compatible gedcom from a lines-compatible
 *      database, including the conversion of SOUR entries into the bang-
 *      tagged NOTEs used by paf for documentation (see 5).  This produces
 *      paf 2.2 output -- you can convert to 2.1 by loading and unloading
 *      it using paf 2.2.
 *
 *      First, some silly truncation and format conformance stuff:
 *
 *      1. NAMEs are truncated to four fields (3 given and 1 surname) of
 *         16 characters each. The 3rd given name field is filled with
 *         multiple names concatenated by underscores, up to the 16 character
 *         limit.  Characters trailing the surname are inserted as a TITL
 *         entry, taking precendence over or being subverted by another TITL
 *         entry (according to the order of the two -- the first takes
 *         precedence).
 *      2. PLACes are truncated to four fields of 16 characters each.
 *         Leading commas are inserted to fill to four fields.
 *      3. SEX is set to M, F, or blank.
 *      4. DATEs are truncated to 23 characters. Date format checking is
 *         not (yet) performed. If you've done this elsewhere, please let
 *         me know and I'll stick it in.
 *      5. No effort is made to conform to the 80 character per line limit.
 *
 *         Let's define "contify" to mean read a line, check its length, and
 *         line break it using CONTs at a space so that the maximum line length
 *         is approached but not violated.  Contification is best handled in
 *         a post-processing phase that simply reads in the file, contify's it,
 *         and outputs it again. This could be done, but is not.
 *
 *         Should this simply check line lengths and contify those over 80
 *         characters, or should the system concatenate and then contify all
 *         lines? The latter is much more elegant and suitable for systems
 *         that assume post-processing, as with LaTeX, but the former is
 *         required to maintain "formatting" in ascii text while providing
 *         the automatic capability for producing paf-compatible files. I
 *         would argue that if the former is the case, that no contification
 *         should take place at all -- if the user wants some control over
 *         the formatting, then s/he should take full responsibility to
 *         maintain the formatting completely. And that's where we leave
 *         it, no contification is done.
 *
 *      Next, only a restricted subset of the entries are output:
 *
 *      6. Only the following entries are output:
 *         - Level 1 records, only the first of multiple is output:
 *              NAME, TITL, SEX, BIRT, CHR, DEAT, BURI
 *         - Level 2 records, only the first of multiple is output:
 *              DATE, PLAC
 *         - Level 1 records, multiple outputs allowed:
 *              NOTE, FAMS, FAMC, AFN, REFN, HUSB, WIFE, CHIL, MARR,
 *              BAPL, ENDL, TEMP, SLGC, SLGS
 *         - Level 1 DIV, DIVF, and ANUL records are translated into DIV Y
 *           along with bang-tagged NOTEs (notes are not yet supported),
 *           multiple outputs are allowed.
 *         - Level 1 OCCU are converted to NOTEs.
 *         - Level 2 SOUR records are translated into bang-tagged NOTEs
 *           attached to the individual or to the head of the family,
 *           husband or wife if there is no husband, for marriage/divorce
 *           sources, multiple outputs are allowed.
 *
 *           The format of the NOTEs is as described in the 1993 Edition of
 *           of the PAF Documentation Guidelines produced by the Silicon
 *           Valley Users Group, where the text of each gedcom record is
 *           inserted as shown:
 *
 *              m SOUR text -> !event: text
 *              m @xx@ SOUR -> !event: AUTHor or NAME, TITLe; PERIod;
 *                                     PUBRisher and publication information,
 *                                     ADDR, DATE; PAGEs; REPOsitory; NOTEs
 *              m SOUR @xx@ -> !event: See xx.
 *
 *           Generally, source references must be converted to definitions
 *           before they can be used to produce legal NOTEs according to the
 *           PAF DC (I use an awk script for this in lieu of real lifelines
 *           support for sources in 2.3.6).
 *
 *           TITL is replaced by "TITL," PUBL when the PUBL record exists --
 *           this structure is used to give the TITLe of an article in a
 *           PUBLication.
 *
 *           Actually, this is not quite correct:
 *           . The PDG does not require the bang, but rather uses it to signal
 *             ``public'' notes -- we assume that all notes are public, though,
 *             and so require it.
 *           . The PDG requires ;;;;; before text in a plain note, but this
 *             seems like overkill.
 *
 *         No other entries are output!
 *
 *      8. CONTs are only handled correctly for NOTEs and SOURs.
 *
 *      Finally, some output formatting is available:
 *
 *      9. Submitter information can be optionally included. If used, this
 *         should be a file of the form
 *
 *              0 @xx@ SUBM
 *              1 NAME Kurt Baudendistel
 *              1 ADDR 420 River Rd, Apt D7
 *              2 CONT Chatham, NJ  07928
 *              2 CONT baud@research.att.com
 *              1 PHON (908) 582-2168
 *
 *         Note that errors in this file format will not be checked -- it
 *         is simply inserted in the gedcom output.
 *
 *      Possible future upgrades:
 *
 *      A. When multiple records, such as BIRT are found, output the later
 *         ones as NOTEs.
 *      B. Contify.
 *      C. Convert date formats to legal ones, including bumping non-date
 *         information, such as "See Notes" into NOTEs.
 *      D. Output submitter information that is stored in the database.
 *
 *      This capability is easy to use inside any other program that
 *      generates a restricted set of families/individuals. Simply include
 *      the pafX functions given below main and use pafindi/paffam instead
 *      of the standard outindi/outfam given in simpleged.
 *
 *      From:   simpleged               ttw@beltway.att.com
 *              pafcompat               eggertj@ll.mit.edu
 *
 *      12 NOV 1994 (2.3.6)             baud@research.att.com
 */

/* main function */

proc main ()
{
  "0 HEAD \n"
  "1 SOUR LIFELINES\n"
  "2 VER 2.3.6\n"
  "2 NAME PAF-EXPORT REPORT\n"
  "1 DEST PAF\n"
  "2 VER 2.2\n"
  "1 DATE " date (gettoday ()) "\n"

  getstrmsg (submitterFile,
    "What is the name of the submitter information file (null okay)?")
  if (strcmp ("", submitterFile)) {
    "1 COPR Copyright " date (gettoday ()) ". Permission is granted to repro"
      "duce any subset\n2 CONT of the data contained herein under the condit"
      "ion that this copyright\n2 CONT notice is preserved, that the origina"
      "l source citations referenced\n2 CONT in the subset are included, and"
      " that the submitter of this file is\n2 CONT credited with original au"
      "thorship as appropriate.\n"
    copyfile (submitterFile)
  }
  "1 CHAR ASCII\n"

  print ("Processing nodes (x10) ...\n")
  forindi (indi, num) {
    if (eq (mod (num, 10), 0)) {
      print ("i")
    }
    call pafindi (indi)
  }

  forfam (fam, num) {
    if (eq (mod (num, 10), 0)) {
      print ("f")
    }
    call paffam (fam)
  }

  "0 TRLR \n"
}

/* pafX functions */

global (paftitl)

proc pafindi (indi)
{
  set (root, inode (indi))
  set (noname, 1)
  set (notitl, 1)
  set (nosex, 1)
  set (nobirt, 1)
  set (nobapt, 1)
  set (nodeat, 1)
  set (noburi, 1)
  if (eq (nfamilies (indi), 1)) {
    set (fams_counter, 0)
  } else {
    set (fams_counter, 1)
  }
  "0 " xref (root) " " tag (root) "\n"
  fornodes (root, node) {
    if (and (noname, not (strcmp ("NAME", tag (node))))) {
      "1 NAME" call pafname (value (node)) "\n"
      if (and (notitl, strlen (paftitl))) {
        "1 TITL" paftitl "\n"
        set (notitl, 0)
      }
      set (noname, 0)
    } elsif (and (notitl, not (strcmp ("TITL", tag (node))))) {
      "1 TITL " value (node) "\n"
      set (notitl, 0)
    } elsif (and (nosex, not (strcmp ("SEX", tag (node))))) {
      "1 SEX " call pafsex (value (node)) "\n"
      set (nosex, 0)
    } elsif (and (nobirt, not (strcmp ("BIRT", tag (node))))) {
      call pafevent (node, 1, 1, 0, 0)
      set (nobirt, 0)
    } elsif (and (nobapt, not (strcmp ("CHR", tag (node))))) {
      call pafevent (node, 1, 1, 0, 0)
      set (nobapt, 0)
    } elsif (and (nodeat, not (strcmp ("DEAT", tag (node))))) {
      call pafevent (node, 1, 1, 0, 0)
      set (nodeat, 0)
    } elsif (and (noburi, not (strcmp ("BURI", tag (node))))) {
      call pafevent (node, 1, 1, 0, 0)
      set (noburi, 0)
    } elsif (not (strcmp ("BAPL", tag (node)))) {
      "1 BAPL" call pafevent (node, 1, 1, 0, 0)"\n"
    } elsif (not (strcmp ("ENDL", tag (node)))) {
      "1 ENDL" call pafevent (node, 1, 1, 0, 0)"\n"
    } elsif (not (strcmp ("TEMP", tag (node)))) {
      "1 TEMP" call pafevent (node, 1, 1, 0, 0)"\n"
    } elsif (not (strcmp ("SLGC", tag (node)))) {
      "1 SLGC" call pafevent (node, 1, 1, 0, 0)"\n"
    } elsif (not (strcmp ("SLGS", tag (node)))) {
      "1 SLGS" call pafevent (node, 1, 1, 0, 0)"\n"
    } elsif (not (strcmp ("FAMC", tag (node)))) {
      "1 FAMC " value (node) "\n"
    } elsif (not (strcmp ("FAMS", tag (node)))) {
      "1 FAMS " value (node) "\n"
      set (f, fam (value (node)))
      if (or (not (husband (f)), eq (husband (f), indi))) {
        call pafevent (marriage (f), 0, 1, 0, fams_counter)
        fornodes (fnode (f), subnode) {
          if (or (or (not (strcmp ("DIV", tag (subnode))),
                      not (strcmp ("DIVF", tag (subnode)))),
                      not (strcmp ("ANUL", tag (subnode))))) {
            call pafevent (subnode, 0, 1, 1, fams_counter)
          }
        }
      }
      incr (fams_counter)
    } elsif (not (strcmp ("OCCU", tag (node)))) {
      "1 NOTE OCCUPATION: " call values (node) "\n"
    } elsif (not (strcmp ("NOTE", tag (node)))) {
      "1 NOTE " call values (node) "\n"
    } elsif (not (strcmp ("AFN", tag (node)))) {
      "1 AFN" value (node) "\n"
    } elsif (not (strcmp ("REFN", tag (node)))) {
      "1 REFN" value (node) "\n"
    }
  }
}

proc paffam (fam)
{
  set (root, fnode (fam))
  "0 " xref (root) " " tag (root) "\n"
  fornodes (root, node) {
    if (not (strcmp ("HUSB", tag (node)))) {
      "1 HUSB " value (node) "\n"
    } elsif (not (strcmp ("WIFE", tag (node)))) {
      "1 WIFE " value (node) "\n"
    } elsif (not (strcmp ("CHIL", tag (node)))) {
      "1 CHIL " value (node) "\n"
    } elsif (not (strcmp ("MARR", tag (node)))) {
      call pafevent (node, 1, 0, 0, 0)
    } elsif (not (strcmp ("DIV", tag (node)))) {
      "1 DIV Y\n"
    } elsif (not (strcmp ("DIVF", tag (node)))) {
      "1 DIV Y\n"
    } elsif (not (strcmp ("ANUL", tag (node)))) {
      "1 DIV Y\n"
    }
  }
}

proc pafevent (event, eventflag, sourceflag, noteflag, count)
{
  table (tagnotes)
  insert (tagnotes,"BIRT","BIRTH")
  insert (tagnotes,"CHR", "CHRISTENING")
  insert (tagnotes,"DEAT","DEATH")
  insert (tagnotes,"BURI","BURIAL")
  insert (tagnotes,"MARR","MARRIAGE")
  insert (tagnotes,"DIV", "DIVORCE")
  insert (tagnotes,"DIVF", "DIVORCEFINAL")
  insert (tagnotes,"ANUL", "ANNULMENT")

  if (event) {
    set (tagname, lookup (tagnotes, tag (event)))
    if (not (strcmp ("", tagname))) { set (tagname, tag (event)) }

    if (eventflag) {
      "1 " tag (event) "\n"
      set (datecount, 1)
      set (placecount, 1)
      fornodes (event, evt) {
        if (not (strcmp ("DATE", tag (evt)))) {
          if (eq (datecount, 1)) {
            "2 DATE " call pafdate (value (evt)) "\n"
          }
          incr (datecount)
        } elsif (not (strcmp ("PLAC", tag (evt)))) {
          if (eq (placecount, 1)) {
            "2 PLAC " call pafplac (value (evt)) "\n"
          }
          incr (placecount)
        }
      }
    }

    if (noteflag) {
      set (countlimit, 0)
    } else {
      set (countlimit, 1)
    }

    if (sourceflag) {
      set (datecount, 1)
      set (placecount, 1)
      fornodes (event, evt) {
        if (not (strcmp ("DATE", tag (evt)))) {
          if (gt (datecount, countlimit)) {
            "1 NOTE " tagname "DATE"
            if (count) { "(" d (count) ")" }
            ": " call pafdate (value (evt)) "\n"
          }
          incr (datecount)
        } elsif (not (strcmp ("PLAC", tag (evt)))) {
          if (gt (placecount, countlimit)) {
            "1 NOTE " tagname "PLACE"
            if (count) { "(" d (count) ")" }
            ": " call pafplac (value (evt)) "\n"
          }
          if (or (not (strcmp ("SITE", tag (child (evt)))),
                  not (strcmp ("CEME", tag (child (evt)))))) {
            "1 NOTE "
            if (not (strcmp (tagname, "BURIAL"))) {
              "CEMETERY"
            } else {
              tagname
              "SITE"
            }
            if (count) { "(" d (count) ")" }
            ": "
            call values (child (evt)) "\n"
          }
          incr (placecount)
        } elsif (not (strcmp ("CAUS", tag (evt)))) {
          "1 NOTE " tagname "CAUSE: " call values (evt) "\n"
        } elsif (not (strcmp ("AGE", tag (evt)))) {
          "1 NOTE " tagname "AGE: " call values (evt) "\n"
        } elsif (not (strcmp ("SOUR", tag (evt)))) {
          "1 NOTE !" tagname
          if (count) { "(" d (count) ")" }
          ": " call pafsour (evt) "\n"
        } elsif (not (strcmp ("NOTE", tag (evt)))) {
          "1 NOTE " tagname
          if (count) { "(" d (count) ")" }
          "NOTE:\n2 CONT " call values (evt) "\n"
        }
      }
    }
  }
}

proc pafname (name)
{
  set (c, 1)
  set (i, 1)
  set (k1, index (name,"/", 1))
  set (k2, index (name,"/", 2))
  set (n, 16)
  set (m, 0)
  while (lt (i, k1)) {
    set (j, index (name," ", c))
    if (or (eq (j, 0), gt (j, k1))) {
      set (j, k1)
    }
    if (lt (c, 4)) {
      " "
    } else {
      "_"
      set (n, sub (sub (n, m), 1))
      if (lt (n, 0)) { set (n, 0) }
    }
    trim (substring (name, i, sub (j, 1)), n)
    set (m, sub (j, i))
    set (i, add (j, 1))
    set (c, add (c, 1))
  }
  " "
  substring (name, k1, k2)
  set (paftitl, substring (name, add (k2, 1), strlen (name)))
}

proc pafsex (name)
{
  if (or (not (strcmp ("M", name)), not (strcmp ("F", name)))) { name }
  else { " " }
}

proc pafdate (name)
{
  trim (name, 23)
}

proc pafplac (name)
{
  set (c, 1)
  set (i, 1)
  set (I, add (strlen (name), 1))
  set (plac,"")
  while (and (lt (i,I), lt (c, 5))) {
    set (j, index (name,",", c))
    if (eq (j, 0)) {
      set (j,I)
    }
    set (plac, concat (plac, trim (substring (name, i, sub (j, 1)), 16)))
    set (plac, concat (plac,","))
    set (i, add (j, 1))
    set (c, add (c, 1))
  }
  while (lt (c, 5)) {
    set (plac, concat (",", plac))
    set (c, add (c, 1))
  }
  substring (plac, 1, sub (strlen (plac), 1))
}

proc pafsour (root) {
  fornodes (root, n) {
    if (not (strcmp ("NAME", tag (n)))) { set (auth, n) }
    elsif (not (strcmp ("AUTH", tag (n)))) { set (auth, n) }
    elsif (not (strcmp ("TITL", tag (n)))) { set (titl, n) }
    elsif (not (strcmp ("PUBL", tag (n)))) { set (publ, n) }
    elsif (not (strcmp ("PERI", tag (n)))) { set (peri, n) }
    elsif (not (strcmp ("PUBR", tag (n)))) { set (pubr, n) }
    elsif (not (strcmp ("ADDR", tag (n)))) { set (addr, n) }
    elsif (not (strcmp ("PHON", tag (n)))) { set (phon, n) }
    elsif (not (strcmp ("DATE", tag (n)))) { set (date, n) }
    elsif (not (strcmp ("VOLU", tag (n)))) { set (vol, n)  }
    elsif (not (strcmp ("VOL",  tag (n)))) { set (vol, n)  }
    elsif (not (strcmp ("NUM",  tag (n)))) { set (num,  n) }
    elsif (not (strcmp ("PAGE", tag (n)))) { set (page, n) }
    elsif (not (strcmp ("REPO", tag (n)))) { set (repo, n) }
    elsif (not (strcmp ("SOUR", tag (n)))) { set (sour, n) }
    elsif (not (strcmp ("FILM", tag (n)))) { set (film, n) }
    elsif (not (strcmp ("NOTE", tag (n)))) { set (note, n) }
  }
  set (any, or (auth, or (titl, or (publ, or (peri, or (pubr, or (addr,
               or (phon, or (date, or (vol, or (num, or (page, or (repo,
               note)))))))))))))
  if (any) {
    if (auth) { call values (auth) }
    if (publ) {
      if (auth) { "," }
      if (titl) { "\n2 CONT \"" call values (titl) ",\"" }
      "\n2 CONT " call values (publ)
    } elsif (titl) {
      if (auth) { "," }
      "\n2 CONT " call values (titl)
    }
    ";" if (peri) { "\n2 CONT " call values (peri) }
    ";" if (pubr) { "\n2 CONT " call values (pubr) }
        if (addr) { if (pubr) { "," } "\n2 CONT " call values (addr) }
        if (phon) {
          if (or (pubr, addr)) { "," }
          "\n2 CONT " call values (phon)
        }
        if (date) {
          if (or (pubr, or (addr, phon))) { "," }
          "\n2 CONT " call values (date)
        }
    ";" if (film) { "\n2 CONT " "Film Number " call values (film) }
        if (vol)  { "\n2 CONT " "Volume " call values (vol) }
        if (num)  { "\n2 CONT " "Number " call values (num) }
        if (page) { "\n2 CONT " "Page(s) " call values (page) }
    ";" if (repo) { "\n2 CONT " call values (repo) }
        if (and (film, not (repo))) {
          "\n2 CONT Church of Jesus Christ of Latter Day Saints, "
            "Salt Lake City, UT"
        }
    ";" if (note) { "\n2 CONT " call values (note) }
  }
  if (v, value (root)) {
    if (and (eq (index (v, "@", 1), 1), eq (index (v, "@", 2), strlen (v)))) {
      if (not (any)) {
        "See " substring (v, 2, sub (strlen (v), 1)) "."
      }
    } else {
      "\n2 CONT " call values (root)
    }
  }
  if (sour) {
    "\n2 CONT " call pafsour (sour)
  }
}

proc values (node)
{
  value (node)
  fornodes (node, n) {
    if (not (strcmp ("CONT", tag (n)))) {
      "\n2 CONT " value (n)
    }
  }
}
