/*
 * @progname    igi-import.ll
 * @version     1.0
 * @author      baud@research.att.com
 * @category
 * @output      GedCom
 * @description
 *
 *      Convert igi gedcom to lifelines-standard gedcom
 *
 *      Igi gedcom contains a single NOTE record for each source, either
 *      on the INDI for a BIRT/CHR or on one of the spouses for a MARR.
 *      This report rearranges the source information into a SOUR record
 *      with the associated NOTE text attached to the appropriate source.
 *
 *      Convert UPPERCASE surnames to upper- and lowercase. Try to
 *      figure out von's and such, but otherwise simply capitalize the
 *      first letters. Remove periods used for abbreviations.
 *
 *      Dates of the form <ABT ...> have the angle brackets stripped.
 *
 *      Note that this report converts INDI/FAM records to INDI/FAM
 *      records, providing *conclusions* for your database. It would
 *      be quite easy, but not of interest to me, to produce a similar
 *      report that produces EVEN records, providing *evidence* for your
 *      database.
 *
 *      -> Use this report on igi gedcom data *before* igi-merge. <-
 *
 *      12 NOV 1994 (3.0.1)             baud@research.att.com
 */

proc main ()
{
  getstrmsg (msg, "IGI Version [default 3.02/1994]?")
  if (streq (msg, "")) {
    set (igiversion, "3.02")
    set (igidate, "1994")
  } else {
    if (i, index (msg, "/", 1)) {
      set (igiversion, save (trim (msg, sub (i, 1))))
      set (igidate, save (cut (msg, add(i, 1))))
    } else {
      set (igiversion, save (msg))
      set (igidate, "")
    }
  }

  "0 HEAD \n"
  "1 SOUR LIFELINES\n"
  "2 VER 3.0.1\n"
  "2 NAME IGI-IMPORT REPORT\n"
  "1 DEST LIFELINES\n"
  "2 VER 3.0.1\n"
  "1 DATE " date (gettoday ()) "\n"
  "1 COPR Copyright " date (gettoday ()) ". Permission is granted to repro"
    "duce any subset\n2 CONT of the data contained herein under the condit"
    "ion that this copyright\n2 CONT notice is preserved, that the origina"
    "l source citations referenced\n2 CONT in the subset are included, and"
    " that the submitter of this file is\n2 CONT credited with original au"
    "thorship as appropriate.\n"
  "1 CHAR ASCII\n"

  "0 @S1@ SOUR\n"
  "1 NAME International Genealogical Index\n"
  "1 PUBR The Church of Jesus Christ of Latter-day Saints\n"
  if (strlen (igiversion)) {
    "1 VER " igiversion "\n"
  }
  if (strlen (igidate)) {
    "1 DATE " igidate "\n"
  }

  print ("Processing nodes ...\n")
  forfam (fam, fn) {
    print ("f")
    igiimport (fam)
  }

  "0 TRLR \n"
}

func igiimport (fam)
{
  if (marriage (fam)) {
    if (note, subnode (inode (husband (fam)), "NOTE")) {
      deletenode (note)
    }
    elsif (note, subnode (inode (wife (fam)), "NOTE")) {
      deletenode (note)
    }
    set (parentsourcetext, "See marriage record.")
    catnode (marriage (fam), sourcifyNote (note))
  } else {
    children (fam, indi, nc) {
      if (note, subnode (inode (indi), "NOTE")) {
        deletenode (note)
      }
      set (childsourcetext, 0)
      if (birth (indi)) {
        set (childsourcetext, "See birth record.")
        catnode (birth (indi), sourcifyNote (note))
      }
      if (baptism (indi)) {
        set (childsourcetext, "See christening record.")
        catnode (baptism (indi), sourcifyNote (note))
      }
      reformatnames (inode (indi), childsourcetext)
      reformatdates (inode (indi))
      gedcomnode (inode (indi))
      set (parentsourcetext,
        save (concat ("See ",
                concat (cond (female (indi), "daughter", "son"),
                  concat (cond (strlen (givens (indi)), " ", ""),
                    concat (givens (indi),
                      concat ("'s ",
                        cond (birth (indi), "birth record.",
                          "christening record."))))))))
    }
  }
  if (indi, husband (fam)) {
    reformatnames (inode (indi), parentsourcetext)
    reformatdates (inode (indi))
    gedcomnode (inode (indi))
  }
  if (indi, wife (fam)) {
    reformatnames (inode (indi), parentsourcetext)
    reformatdates (inode (indi))
    gedcomnode (inode (indi))
  }
  reformatdates (fnode (fam))
  gedcomnode (fnode (fam))
  return (0)
}

func sourcifyNote (node) {
  if (node) {
    set (text, values (node))
    while (i, index (text, "#:", 1)) {
      set (text, save (concat3 (trim (text, sub (i, 1)),
                                "Number",
                                cut (text, add (i, 2)))))
    }
    if (streq (substring (text, sub (strlen (text), 5), strlen (text)),
               "Number")) {
      set (text, save (concat (text, " unknown")))
    }
    set (text, save (concat3 ("International Genealogical Index, ", text, ".")))
    set (node, createnodes ("SOUR", text))
    catnode (node, createnode ("SOUR", "@S1@"))
  } else {
    set (node, createnode ("SOUR", "@S1@"))
  }
  return (node)
}

/* common import/export functions */

func cond (x, a, b) {
  if (x) {
    return (a)
  } else {
    return (b)
  }
}

func gedcomnode (root) {
  traverse (root, node, level) {
    d (level)
    if (x, xref (node))  { " " x }
    if (x, tag (node))   { " " x }
    if (x, value (node)) { " " x }
    "\n"
  }
  return (0)
}

func denull (alist) {
  list (blist)
  forlist (alist, a, an) {
    if (a) { enqueue (blist, a) }
  }
  return (blist)
}

func reformatdates (root) {
  traverse (root, node, level) {
    if (streq (tag (node), "DATE")) {
      if (v, value (node)) {
        if (and (eq (index (v, "<", 1), 1),
                 eq (index (v, ">", 1), strlen (v)))) {
          replacenode
            (createnode ("DATE", save (substring (v, 2, sub (strlen (v), 1)))),
             subnode (node, "DATE"))
        }
      }
    }
  }
  return (0)
}

func reformatnames (root, sourcetext) {
  list (namelist)
  list (surnamelist)
  list (choppedsurnamelist)
  list (newchoppedsurnamelist)
  if (namenode, subnode (root, "NAME")) {
    extractnames (namenode, namelist, nameN, surnameN)
    set (lastnamenode, namenode)
    forlist (namelist, s, sn) {
      set (s, strremove (s, "."))
      set (s, strremove (s, "_"))
      setel (namelist, sn, s)
    }
    enqueue (surnamelist, getel (namelist, surnameN))
    while (surname, dequeue (surnamelist)) {
      set (choppedsurnamelist, strchop (surname, " "))
      forlist (choppedsurnamelist, s, sn) {
        if (streq ("VON", s)) {
          enqueue (newchoppedsurnamelist, s)
        } elsif (streq ("DER", s)) {
          enqueue (newchoppedsurnamelist, s)
        } elsif (and (eq (index (s, "(", 1), 1),
                      eq (index (s, ")", 1), strlen (s)))) {
          enqueue (surnamelist, save (substring (s, 2, sub (strlen (s), 1))))
        } else {
          enqueue (newchoppedsurnamelist, save (capitalize (lower (s))))
        }
      }
      set (newsurname, strjoin (newchoppedsurnamelist, " "))
      if (strlen (newsurname)) {
        if (i, index (newsurname, "Mc ", 1)) {
          set (newsurname, save (concat (trim (newsurname, add (i, 1)),
                                         cut (newsurname, add (i, 3)))))
        }
        set (newsurname, save (concat3 ("/", newsurname, "/")))
      }
      setel (namelist, surnameN, newsurname)
      set (newnamenode, createnode ("NAME", strjoin (namelist, " ")))
      addnode (newnamenode, parent (lastnamenode), lastnamenode)
      if (sourcetext) {
        catnode (newnamenode, createnode ("SOUR", sourcetext))
      }
      set (lastnamenode, newnamenode)
    }
    deletenode (namenode)
  }
  return (0)
}

func streq (x, y) {
  return (not (strcmp (x, y)))
}

func createnodes (tag, text) {
  set (text, trimspaces (text))
  if (le (strlen (text), 72)) {
    return (createnode (tag, text))
  } else {
    list (textlist)
    while (gt (strlen (text), 72)) {
      set (n, 1)
      if (i, index (text, " ", n)) {
        set (j, i)
      } else {
        set (j, add (strlen (text), 1))
      }
      while (and (i, lt (i, 73))) {
        incr (n)
        set (j, i)
        set (i, index (text, " ", n))
      }
      enqueue (textlist, save (trim (text, sub (j, 1))))
      set (text, save (cut (text, add (j, 1))))
    }
    if (gt (strlen (text), 0)) {
      enqueue (textlist, text)
    }
    set (root, createnode (tag, dequeue (textlist)))
    set (lastnode, 0)
    forlist (textlist, text, tn) {
      set (node, createnode ("CONT", text))
      addnode (node, root, lastnode)
      set (lastnode, node)
    }
    return (root)
  }
}

func trimspaces (text) {
  set (ss, 0)
  set (s0, 1)
  set (sn, strlen (text))
  while (and (le (s0, sn), streq (substring (text, s0, s0), " "))) {
    set (ss, 1)
    incr (s0)
  }
  while (and (le (s0, sn), streq (substring (text, sn, sn), " "))) {
    set (ss, 1)
    decr (sn)
  }
  if (ss) {
    return (save (substring (text, s0, sn)))
  } else {
    return (text)
  }
}

func catnode (root, newnode) {
  if (root) {
    set (lastnode, 0)
    fornodes (root, node) {
      set (lastnode, node)
    }
    addnode (newnode, root, lastnode)
  }
  return (0)
}

func strchop (s, d) {
  list (slist)
  set (dn, strlen (d))
  if (strlen (s)) {
    set (n, 1)
    set (s0, 1)
    while (sn, index (s, d, n)) {
      enqueue (slist, save (substring (s, s0, sub (sn, 1))))
      set (s0, add (sn, dn))
      incr (n)
    }
    enqueue (slist, save (cut (s, s0)))
  }
  return (slist)
}

func strjoin (slist, d) {
  forlist (slist, s, sn) {
    if (not (strlen (str))) {
      set (str, s)
    } elsif (strlen (s)) {
      set (str, save (concat3 (str, d, s)))
    }
  }
  return (str)
}

func subnode (root, tag) {
  if (root) {
    fornodes (root, node) {
      if (streq (tag (node), tag)) {
        return (node)
      }
    }
  }
  return (0)
}

func subnodes (root, tag) {
  list (nodelist)
  if (root) {
    fornodes (root, node) {
      if (streq (tag (node), tag)) {
        enqueue (nodelist, node)
      }
    }
  }
  return (nodelist)
}

func replacenode (newnode, oldnode) {
  if (newnode) {
    if (root, parent (oldnode)) {
      addnode (newnode, root, oldnode)
      deletenode (oldnode)
    }
  }
  return (0)
}

func concat3 (x, y, z) {
  return (concat (x, concat (y, z)))
}

func cut (s, n) {
  return (substring (s, n, strlen (s)))
}

func values (root) {
  if (root) {
    set (str, value (root))
    fornodes (root, node) {
      if (not (str)) {
        set (str, value (node))
      } elsif (strlen (value (node))) {
        set (str, save (concat3 (str, " ", value (node))))
      }
    }
    return (str)
  } else {
    return (0)
  }
}

func strremove (s, d) {
  if (strlen (s)) {
    while (i, index (s, d, 1)) {
      set (s, save (concat (trim (s, sub (i, 1)), cut (s, add (i, 1)))))
    }
  }
  return (s)
}
