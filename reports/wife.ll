/*
 * @progname       wife.ll
 * @version        1995-02-10
 * @author         Kurt Baudendistel <baud@RESEARCH.ATT.COM>
 * @category       
 * @output         Text
 * @description

		A report to find wives.
 */

proc main () {
    getstrmsg (str, "Name [hit enter for help]")

    if (eq (strlen (str), 0)) {

        print ("Enter a name in the browser format ...\n")
        print ("- case insensitive.\n")
        print ("- given name(s) [/optional surname]\n")
        print ("\n")
        print ("... and this program will give a list of women who have this\n")
        print ("    as their married name on the screen here.\n")
        print ("\n")
        print ("The matching mechanism, however, is not that used by the\n")
        print ("browser, but simply uses the \"index\" function to create a\n")
        print ("match separately in the given name(s) and surname. While\n")
        print ("this makes partial matching easier, it does not allow for\n")
        print ("abbreviations of multiple given names.\n")
        print ("\n")

    } else {

        if (i, index (str, "/", 1)) {
                set (Givens, save (substring (str, 1, sub (i, 1))))
                set (givens, save (Givens))
            set (j, index (str, "/", 2))
            if (not (j)) {
                set (j, add (strlen (str), 1))
            }
                set (Surname, save (substring (str, add (i, 1), sub (j, 1))))
                set (surname, save (Surname))
        } else {
                set (Givens, save (str))
                set (givens, save (Givens))
                set (Surname, "")
                set (surname, "")
        }

        print ("Possible identities of ")
        print (Givens)
        if (strlen (Surname)) {
            print (" (")
            print (Surname)
            print (")")
        }
        print (":\n\n")

        forindi (indi, n) {
            families (indi, fam, spouse, m) {
                if (and (strlen (surname (indi)),
                     and (index (lower (surname (indi)), surname, 1),
                                  and (strlen (givens (spouse)),
                               index (lower (givens (spouse)), givens, 1))))) {
                if (not (male (spouse))) {
                    print (name (spouse))
                    print (" (")
                    print (key (spouse))
                    print (")")
                    if (date (birth (spouse))) {
                        print (" ")
                        print (date (birth (spouse)))
                    } elsif (date (baptism (spouse))) {
                        print (" ")
                        print (date (baptism (spouse)))
                    }
                    print (" -")
                    if (date (death (spouse))) {
                        print (" ")
                        print (date (death (spouse)))
                    } elsif (date (burial (spouse))) {
                        print (" ")
                        print (date (burial (spouse)))
                    }
                    print ("\n")
                    }
                }
            }
        }

    }
}
