/*
 * @progname    af-import.ll
 * @version     4.12
 * @author      baud@research.att.com
 * @category
 * @output      GedCom
 * @description
 *
 * convert ancestral-file gedcom to lifelines-standard gedcom
 *
 *      AF gedcom has the following defects that must be corrected:
 *
 *      NAME    - Delete name LIVING (Actually, 3.0.1 requires some kind
 *                of name, so use "/").
 *              - Convert UPPERCASE surnames to upper- and lowercase. Try to
 *                figure out von's and such, but otherwise simply capitalize
 *                the first letters.
 *              - Remove periods used for abbreviations.
 *              - Delete given name "Stillborn".
 *              - Alternate surnames (given in parentheses) are converted
 *                to subsequent NAME records.
 *      DATE    - LIVING -- delete associated event.
 *              - Convert "<ABT X>" to "ABT X".
 *              - Marriage date with trailing (DIV) indicates divorce --
 *                strip string and convert to DIV record.
 *      PLAC    - Burial place "Cremated" converted to NOTE record.
 *              - Strip leading commas.
 *      events  - Add SOUR cross-reference to "Ancestral File" to all.
 *
 *      12 NOV 1994 (3.0.1)             baud@research.att.com
 */

proc main ()
{
  getstrmsg (msg, "AF Version [default 4.12/1992]?")
  if (streq (msg, "")) {
    set (afversion, "4.12")
    set (afdate, "1992")
  } else {
    if (i, index (msg, "/", 1)) {
      set (afversion, save (trim (msg, sub (i, 1))))
      set (afdate, save (cut (msg, add(i, 1))))
    } else {
      set (afversion, save (msg))
      set (afdate, "")
    }
  }

  "0 HEAD \n"
  "1 SOUR LIFELINES\n"
  "2 VER " version() "\n"
  "2 NAME AF-IMPORT REPORT\n"
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
  "1 NAME Ancestral File\n"
  "1 PUBR The Church of Jesus Christ of Latter-day Saints\n"
  if (strlen (afversion)) {
    "1 VER " afversion "\n"
  }
  if (strlen (afdate)) {
    "1 DATE " afdate "\n"
  }

  print ("Processing nodes ...\n")
  forindi (indi, in) {
    print ("i")
    afimportindi (indi)
  }
  forfam (fam, fn) {
    print ("f")
    afimportfam (fam)
  }

  "0 TRLR \n"
}

func afimportindi (indi)
{
  set (root, inode (indi))

  if (streq (name (indi), "LIVING")) {
    replacenode (createnode ("NAME", "/"), subnode (root, "NAME"))
  } elsif (index (name (indi), "Stillborn ", 1)) {
    set (namenode, subnode (root, "NAME"))
    replacenode (createnode ("NAME", save (cut (value (namenode), 11))),
      namenode)
  }
  reformatnames (root, "@S1@")

  if (streq (date (birth (indi)), "LIVING")) {
      deletenode (birth (indi))
  }
  if (streq (date (baptism (indi)), "LIVING")) {
      deletenode (baptism (indi))
  }
  reformatdates (root)

  fornodes (root, node) {
    if (eventP (node)) {
      if (place (node)) {
        if (streq (place (node), "Cremated")) {
          replacenode (createnode ("NOTE", "Cremated."), subnode (node, "PLAC"))
        } else {
          list (placelist)
          extractplaces (node, placelist, placenumber)
          replacenode (createnode ("PLAC", strjoin (denull (placelist), ",")),
            subnode (node, "PLAC"))
        }
      }
      catnode (node, createnode ("SOUR", "@S1@"))
    }
  }

  gedcomnode (root)
  return (0)
}

func eventP (root) {
  if (root) {
    if (streq (tag (root), "BIRT")) { return (1) }
    if (streq (tag (root), "CHR"))  { return (1) }
    if (streq (tag (root), "DEAT")) { return (1) }
    if (streq (tag (root), "BURI")) { return (1) }
  }
  return (0)
}

func afimportfam (fam)
{
  set (root, fnode (fam))

  reformatdates (root)

  if (node, marriage (fam)) {
    if (i, index (date (node), " (DIV)", 1)) {
      replacenode (createnode ("DATE", save (trim (date (node), sub (i, 1)))),
        subnode (node, "DATE"))
      set (divorcenode, createnode ("DIV", ""))
      catnode (divorcenode, createnode ("SOUR", "@S1@"))
      catnode (root, divorcenode)
    }
    if (place (node)) {
      list (placelist)
      extractplaces (node, placelist, placenumber)
      replacenode (createnode ("PLAC", strjoin (denull (placelist), ",")),
        subnode (node, "PLAC"))
    }
    catnode (node, createnode ("SOUR", "@S1@"))
  }

  gedcomnode (root)
  return (0)
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
