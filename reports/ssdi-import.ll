/*
 * @progname       ssdi-import.ll
 * @version        1994-11-12
 * @author         Kurt Baudendistel (baud@research.att.com)
 * @category       
 * @output         GEDCOM
 * @description
 *
 * Convert ssdi gedcom to lifelines-standard gedcom
 *
 *      12 NOV 1994 (3.0.1)             baud@research.att.com
 *                                      Derived from import-igi.
 */

proc main ()
{
  getstrmsg (msg, "SSDI Version [default X/1992]?")
  if (streq (msg, "")) {
    set (ssdiversion, "X")
    set (ssdidate, "1992")
  } else {
    if (i, index (msg, "/", 1)) {
      set (ssdiversion, save (trim (msg, sub (i, 1))))
      set (ssdidate, save (cut (msg, add(i, 1))))
    } else {
      set (ssdiversion, save (msg))
      set (ssdidate, "")
    }
  }

  "0 HEAD \n"
  "1 SOUR LIFELINES\n"
  "2 VER 3.0.1\n"
  "2 NAME SSDI-IMPORT REPORT\n"
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
  "1 NAME Social Security Death Index\n"
  if (strlen (ssdiversion)) {
    "1 VER " ssdiversion "\n"
  }
  if (strlen (ssdidate)) {
    "1 DATE " ssdidate "\n"
  }

  print ("Processing nodes ...\n")
  forindi (indi, in) {
    print ("i")
    ssdiimport (indi)
  }

  "0 TRLR \n"
}

func ssdiimport (indi)
{
  set (number, 0)
  set (residences, 0)
  set (root, inode (indi))
  forlist (subnodes (root, "NOTE"), note, nn) {
    if (streq (trim (value (note), 24), "Social Security Number: ")) {
      set (number, save (cut (value (note), 25)))
    } elsif (streq (value (note), "Death Residence Localities")) {
      set (residences, localities (note))
    }
    deletenode (note)
  }

  reformatnames (root, "@S1@")

  if (number) {
    set (ssn, createnode ("SSN", number))
    if (birthplace, subnode (birth (indi), "PLAC")) {
      if (streq (value (birthplace), "Not Identified")) {
        set (ssnsour,
          createnodes ("SOUR",
            concat ("Issued to ",
              concat (fullname (indi, 0, 1, 999),
                ", but no location of issuance was identified."))))
      } else {
        set (ssnsour,
          createnodes ("SOUR",
            concat ("Issued in ",
              concat (value (birthplace),
                concat (" to ",
                  concat (fullname (indi, 0, 1, 999), "."))))))
      }
      catnode (ssnsour, createnode ("SOUR", "@S1@"))
      catnode (ssn, ssnsour)
    } else {
      catnode (ssn, createnode ("SOUR", "@S1@"))
    }
    addnode (ssn, root, subnode (root, "NAME"))
  }

  if (birth (indi)) {
    if (birthplace, subnode (birth (indi), "PLAC")) {
      deletenode (birthplace)
    }
    catnode (birth (indi), createnode ("SOUR", "@S1@"))
  }

  if (death (indi)) {
    set (deathplace, subnode (death (indi), "PLAC"))
    set (zip, "an unknown")
    if (code, dequeue (residences)) {
      if (streq (trim (code, 10), "Zip Code: ")) {
        set (zip, save (concat ("the ", cut (code, 11))))
      } else {
        requeue (residences, zip)
      }
    }
    if (rn, residences) {
      forlist (residences, res, rn) {
        catnode (death (indi), createnode ("PLAC", res))
        if (and (deathplace, index (res, value (deathplace), 1))) {
          deletenode (deathplace)
          set (deathplace, 0)
        }
      }
    }
    if (and (deathplace, not (value (deathplace)))) {
      deletenode (deathplace)
    }
    if (rn) {
      if (eq (rn, 1)) {
        set (trailer, " zip code.")
      } else {
        set (trailer, " zip code, which encompasses the named localities.")
      }
      set (sour, createnodes ("SOUR",
        concat3 ("The residence at the time of death was in ", zip, trailer)))
      catnode (sour, createnode ("SOUR", "@S1@"))
      catnode (death (indi), sour)
    } else {
      catnode (death (indi), createnode ("SOUR", "@S1@"))
    }
  }

  gedcomnode (root)
  return (0)
}

func localities (root) {
  list (residences)
  if (root) {
    fornodes (root, node) {
      enqueue (residences, value (node))
    }
  }
  return (residences)
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
