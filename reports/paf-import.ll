/*
 * @progname       paf-import.ll
 * @version        1994-11-12
 * @author         Kurt Baudendistel (baud@research.att.com)
 * @category       
 * @output         GEDCOM
 * @description
 *
 *        Convert paf gedcom to lifelines-standard gedcom,
 *        transforming name formats and notes.
 *
 *        First, some silly formating:
 *
 *        1. _'s in NAMEs are converted to spaces.
 *        2. Leading commas are stripped from PLACes
 *        3. Recognizable posttitles are moved from TITL entries to NAME
 *           entries.
 *
 *        Then, the meat of the problem
 *
 *        4. Bang-tagged NOTEs of the form
 *
 *                1 NOTE !BIRTH-CHRISTENING: ...
 *                2 CONT ...
 *
 *           are converted to SOURs in the appropriate event, and the
 *           original NOTE is deleted. The following NOTEs are recognized and
 *           translated into the corresponding event (an event is created
 *           if it does not exist):
 *
 *              NAME            -> NAME
 *              BIRTH           -> BIRT
 *              PARENTS         -> BIRT
 *              FATHER          -> BIRT
 *              MOTHER          -> BIRT
 *              ADOPTION        -> ADOP
 *              CHRISTENING     -> CHR
 *              DEATH           -> DEAT
 *              BURIAL          -> BURI
 *              MARRIAGE        -> MARR (in first associated family)
 *              MARRIAGE(N)     -> MARR (in numbered associated family)
 *              MARRIAGES       -> MARR (in all associated families)
 *              DIVORCE         -> DIV  (in first associated family)
 *              DIVORCE(N)      -> DIV  (in numbered associated family)
 *              DIVORCES        -> DIV  (in all associated families)
 *              DIVORCEFINAL    -> DIVF (in first associated family)
 *              DIVORCEFINAL(N) -> DIVF (in numbered associated family)
 *              DIVORCEFINALS   -> DIVF (in all associated families)
 *              ANNULMENT       -> ANUL (in first associated family)
 *              ANNULMENT(N)    -> ANUL (in numbered associated family)
 *              ANNULMENTS      -> ANUL (in all associated families)
 *
 *           The NOTE is not deleted if any of the components are not
 *           recognized. Plain bang-tagged NOTEs are converted to TEXT.
 *
 *           Multiple NOTEs produce multiple SOURs, just as you would expect.
 *
 *        5. Non-bang-tagged NOTEs of the form
 *
 *                1 NOTE BIRTH: ...
 *                2 CONT ...
 *
 *           are converted to NOTEs in the appropriate event for those
 *           events listed above, and the original NOTE is deleted. Note
 *           multiple NOTE targets (as in BIRTH-CHRISTENING) are not allowed
 *           for non-bang-tagged NOTEs, and that containing nodes (like
 *           PLAC) are not created if they do not exist -- the NOTE is simply
 *           lost.
 *
 *           For the following NOTEs, a record is created of the
 *           indicated type (death here can be replaced by any event):
 *
 *              DEATHSITE       -> DEAT - PLAC - SITE
 *              DEATHAGE        -> DEAT - AGE
 *              DEATHCAUSE      -> DEAT - CAUS
 *              CEMETERY        -> (same as BURIALSITE)
 *              EDITOR          -> SOUR (at level 1)
 *              RESEARCHER      -> SOUR (at level 1)
 *              OCCUPATION      -> OCCU
 *
 *           Of course, the original note is deleted.
 *
 *      From:   paf                     baud@research.att.com
 *
 *      12 NOV 1994 (2.3.6)             baud@research.att.com
 */

global (tTagTranslation)
global (tTitleTransformation)
global (sourceListTable)
global (siteListTable)
global (ageListTable)
global (causeListTable)
global (noteListTable)
global (tIndiEvents)
global (tFamEvents)
global (tNotesToDelete)

proc main ()
{
  "0 HEAD \n"
  "1 SOUR LIFELINES\n"
  "2 VER 2.3.6\n"
  "2 NAME PAF-IMPORT REPORT\n"
  "1 DEST LIFELINES\n"
  "1 DATE " date (gettoday ()) "\n"
  "1 CHAR ASCII\n"

  table (tTagTranslation)
  insert (tTagTranslation, "NAME", "NAME")
  insert (tTagTranslation, "BIRTH", "BIRT")
  insert (tTagTranslation, "PARENTS", "BIRT")
  insert (tTagTranslation, "FATHER", "BIRT")
  insert (tTagTranslation, "MOTHER", "BIRT")
  insert (tTagTranslation, "ADOPTION", "ADOP")
  insert (tTagTranslation, "CHRISTENING", "CHR")
  insert (tTagTranslation, "DEATH", "DEAT")
  insert (tTagTranslation, "BURIAL", "BURI")
  insert (tTagTranslation, "MARRIAGE", "MARR")
  insert (tTagTranslation, "DIVORCE", "DIV")
  insert (tTagTranslation, "DIVORCEFINAL", "DIVF")
  insert (tTagTranslation, "ANNULMENT", "ANUL")
  insert (tTagTranslation, "EDITOR", "SOUR")
  insert (tTagTranslation, "RESEARCHER", "SOUR")
  insert (tTagTranslation, "OCCUPATION", "OCCU")

  table (tTitleTransformation)
  insert (tTitleTransformation, "Jr", "")
  insert (tTitleTransformation, "Sr", "")
  insert (tTitleTransformation, "I", "")
  insert (tTitleTransformation, "II", "")
  insert (tTitleTransformation, "III", "")
  insert (tTitleTransformation, "IV", "")
  insert (tTitleTransformation, "V", "")
  insert (tTitleTransformation, "MD", "Dr")
  insert (tTitleTransformation, "DDS", "Dr")
  insert (tTitleTransformation, "PhD", "Dr")
  insert (tTitleTransformation, "SJ", "Father")
  insert (tTitleTransformation, "SM", "Brother")

  table (sourceListTable)
  table (siteListTable)
  table (ageListTable)
  table (causeListTable)
  table (noteListTable)

  table (tIndiEvents)
  insert (tIndiEvents, "NAME", 1)
  insert (tIndiEvents, "BIRT", 1)
  insert (tIndiEvents, "ADOP", 1)
  insert (tIndiEvents, "CHR", 1)
  insert (tIndiEvents, "DEAT", 1)
  insert (tIndiEvents, "BURI", 1)
  insert (tIndiEvents, "SOUR", 1)
  insert (tIndiEvents, "OCCU", 1)

  table (tFamEvents)
  insert (tFamEvents, "MARR", 1)
  insert (tFamEvents, "DIV", 1)
  insert (tFamEvents, "DIVF", 1)
  insert (tFamEvents, "ANUL", 1)

  table (tNotesToDelete)

  print ("Scanning for sources and event notes (x10) ...\n")
  forindi (indi, num) {
    if (eq (mod(num,10),0)) {
      print ("i")
    }
    call unpafSources (indi)
    call unpafOthers (indi, "SITE", siteListTable)
    call unpafOthers (indi, "AGE", ageListTable)
    call unpafOthers (indi, "CAUSE", causeListTable)
    call unpafOthers (indi, "", noteListTable)
  }

  print ("\n\nProcessing nodes (x10) ...\n")
  forindi (indi, num) {
    if (eq (mod(num,10),0)) {
      print ("i")
    }
    call unpafNode (key (indi), inode (indi))
  }

  forfam (fam, num) {
    if (eq (mod(num,10),0)) {
      print ("f")
    }
    call unpafNode (key (fam), fnode (fam))
  }

  "0 TRLR \n"
}

proc unpafSources (indi)
{
  fornodes (inode (indi), node) {
    if (not (strcmp (tag (node), "NOTE"))) {
      set (note, value (node))
      if (eq (index (note, "!", 1), 1)) {
        if (colon, index (note, ":", 1)) {
          set (nTag, save (concat (substring (note, 2, sub (colon, 1)), "-")))
          set (deleteFlag, 1)
          while (strcmp (nTag, "")) {
            set (mark, index (nTag, "-", 1))
            set (bTag, save (substring (nTag, 1, sub (mark, 1))))
            set (nTag, save (substring (nTag, add (mark, 1), strlen (nTag))))
            set (openLoc, add (index (bTag, "("/*)*/, 1), 1))
            set (closLoc, sub (index (bTag, /*(*/")", 1), 1))
            if (le (openLoc, closLoc)) {
              if (bNum, atoi (substring (bTag, openLoc, closLoc))) {
                set (bTag, save (trim (bTag, sub (openLoc, 2))))
              } else {
                set (bNum, 1)
              }
            } else {
              set (bNum, 1)
            }
            if (evt, lookup (tTagTranslation, bTag)) {
              set (sourceKey, "")
              if (lookup (tIndiEvents, evt)) {
                if (eq (bNum, 1)) {
                  set (sourceKey, save (concat (key (indi), evt)))
                }
              } elsif (lookup (tFamEvents, evt)) {
                set (foundFlag, 0)
                families (indi, fvar, svar, num) {
                  if (eq (bNum, num)) {
                    set (sourceKey, save (concat (key (fvar), evt)))
                    set (foundFlag, 1)
                  }
                }
                if (not (foundFlag)) {
                  set (deleteFlag, 0)
                }
              }
              if (strcmp (sourceKey, "")) {
                call insertListTable (sourceListTable, sourceKey, node)
              }
            } else {
              set (deleteFlag, 0)
            }
          }
          if (deleteFlag) {
            insert (tNotesToDelete, save (value (node)), 1)
          }
        }
      }
    }
  }
}

proc unpafOthers (indi, kind, otherListTable)
{
  set (tail, save (concat (kind, ":")))
  fornodes (inode (indi), node) {
    if (not (strcmp (tag (node), "NOTE"))) {
      set (note, value (node))
      if (eq (index (note, "CEMETERY:", 1), 1)) {
        set (note,
          save (concat ("BURIALSITE", substring (note, 9, strlen (note)))))
      }
      set (tailIndex, index (note, tail, 1))
      set (spaceIndex, index (note, " ", 1))
      if (or (lt (tailIndex, spaceIndex),
              and (eq (spaceIndex, 0),
                   gt (tailIndex, 0)))) {
        set (bEnd, sub (tailIndex, 1))
        set (bTag, save (trim (note, bEnd)))
        set (bNum, atoi (substring (bTag, bEnd, bEnd)))
        if (ne (bNum, 0)) {
          decr (bEnd)
          set (bTag, save (trim (bTag, bEnd)))
        }
        incr (bNum)
        if (evt, lookup (tTagTranslation, bTag)) {
          set (otherKey, "")
          if (lookup (tIndiEvents, evt)) {
            if (eq (bNum, 1)) {
              set (otherKey, save (concat (key (indi), evt)))
            }
          } elsif (lookup (tFamEvents, evt)) {
            families (indi, fvar, svar, num) {
              if (eq (bNum, num)) {
                set (otherKey, save (concat (key (fvar), evt)))
              }
            }
          }
          if (strcmp (otherKey, "")) {
            call insertListTable (otherListTable, otherKey, node)
            insert (tNotesToDelete, save (value (node)), 1)
          }
        }
      }
    }
  }
}

proc insertListTable (listTable, tableKey, node) {
  list (evtList)
  set (note, value (node))
  set (first,
    save (substring (note, add (index (note, ":", 1), 1), strlen (note))))
  if (not (strcmp (trim (first, 1), " "))) {
    set (first, save (substring (first, 2, strlen (first))))
  }
  if (strlen (first)) {
    enqueue (evtList, first)
  }
  fornodes (node, n) {
    if (not (strcmp ("CONT", tag (n)))) {
      enqueue (evtList, save (value (n)))
    }
  }
  set (entryList, lookup (listTable, tableKey))
  if (not (entryList)) { list (entryList) }
  enqueue (entryList, evtList)
  insert (listTable, tableKey, entryList)
}

proc unpafNode (rootKey, root)
{
  set (sourceList, 0)
  set (noteList, 0)
  set (sawBIRT, 0)
  traverse (root, node, level) {
    set (sawBIRT, or (sawBIRT, not (strcmp (tag (node), "BIRT"))))
    if (eq (level, 0)) {
      set (deletingFlag, 0)
      set (listTableKey, save (concat (rootKey, tag (node))))
      set (sourceList, lookup (sourceListTable, listTableKey))
      if (unbangedSourceList, lookup (noteListTable, listTableKey)) {
        while (evt, dequeue (unbangedSourceList)) {
          enqueue (sourceList, evt)
        }
      }
    } elsif (eq (level, 1)) {
      if (sourceList) {
        while (evt, dequeue (sourceList)) {
          call reTagNote (add (level, 1), "SOUR", evt)
        }
        set (sourceList, 0)
      }
      if (noteList) {
        while (evt, dequeue (noteList)) {
          call reTagNote (add (level, 1), "TEXT", evt)
        }
        set (noteList, 0)
      }
      set (listTableKey, save (concat (rootKey, tag (node))))
      set (sourceList, lookup (sourceListTable, listTableKey))
      set (noteList, lookup (noteListTable, listTableKey))
      set (deletingFlag, and (not (strcmp (tag (node), "NOTE")),
                              lookup (tNotesToDelete, value (node))))
    }
    if (not (deletingFlag)) {
      d (level) " "
      if (xref (node)) { xref (node) " " }
      set (text, value (node))
      if (not (strcmp (tag (node), "NAME"))) {
        while (ind, index (text, "_", 1)) {
          set (text,
            save (concat (concat (substring (text, 1, sub (ind,1)), " "),
              substring (text, add (ind, 1), strlen (text)))))
        }
        "NAME " text "\n"
      } elsif (not (strcmp (tag (node), "TITL"))) {
        if (titl, lookup (tTitleTransformation, text)) {
          "NAME // " text "\n"
          if (strlen (titl)) {
            d (level) " TITL " titl "\n"
          }
        } else {
          "TITL " text "\n"
        }
      } elsif (not (strcmp (tag (node), "PLAC"))) {
        while (not (strcmp (trim (text, 1), ","))) {
          set (text, save (substring (text, 2, strlen (text))))
        }
        "PLAC " text "\n"
        if (siteList, lookup (siteListTable, listTableKey)) {
          while (evt, dequeue (siteList)) {
            call reTagNote (add (level, 1), "SITE", evt)
          }
        }
      } elsif (not (strcmp (tag (node), "NOTE"))) {
        if (not (strcmp (trim (text, 1), "!"))) {
          "TEXT "
        } else {
          "NOTE "
        }
        text "\n"
      } else {
        tag (node) " " text "\n"
        if (ageList, lookup (ageListTable, listTableKey)) {
          while (evt, dequeue (ageList)) {
            call reTagNote (add (level, 1), "AGE", evt)
          }
        }
        if (causeList, lookup (causeListTable, listTableKey)) {
          while (evt, dequeue (causeList)) {
            call reTagNote (add (level, 1), "CAUS", evt)
          }
        }
      }
    }
  }
  set (listTableKey, save (concat (rootKey, BIRT)))
  set (sourceList, lookup (sourceListTable, listTableKey))
  set (noteList, lookup (noteListTable, listTableKey))
  if (and (or (sourceList, noteList), not (sawBIRT))) {
    "1 BIRT\n"
    if (sourceList) {
      while (evt, dequeue (sourceList)) {
          call reTagNote (2, "SOUR", evt)
      }
    }
    if (noteList) {
      while (evt, dequeue (noteList)) {
        call reTagNote (2, "TEXT", evt)
      }
    }
  }
}

proc reTagNote (relevel, retag, revalueList) {
  set (contLevel, add (relevel, 1))
  forlist (revalueList, revalue, rv) {
    d (relevel) " " retag " " revalue "\n"
    set (relevel, contLevel)
    set (retag, "CONT")
  }
}
