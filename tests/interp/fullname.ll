/*
@progname fullname.ll
@author Matt Emmerton
@description Test fullname() on invalid INDIs
*/

proc main ()
{
  "Starting Test" nl()

  /* for each family */
  forfam(fam,fnum)
  {
    set (h, husband(fam))
    set (w, wife(fam))

    "Valid INDI (DOSURCAP, REGORDER): " "'" fullname(h,1,1,50) "'" nl()
    "Valid INDI (NOSURCAP, REGORDER): " "'" fullname(h,0,1,50) "'" nl()
    "Valid INDI (DOSURCAP, SURFIRST): " "'" fullname(h,1,0,50) "'" nl()
    "Valid INDI (NOSURCAP, SURFIRST): " "'" fullname(h,0,0,50) "'" nl()

    "Valid INDI (NOSURCAP, REGORDER truncation, varying lengths):" nl()
    "Partial First Name:   '" fullname(h,1,1,4) "'" nl()
    "Full First Name:      '" fullname(h,1,1,5) "'" nl()
    "Full including Space: '" fullname(h,1,1,6) "'" nl()
    "Full including Last:  '" fullname(h,1,1,7) "'" nl()

    "Valid INDI (DOSURCAP, SURFIRST truncation, varying lengths):" nl()
    "Partial Last Name     '" fullname(h,1,0,4) "'" nl()
    "Full Last Name:       '" fullname(h,1,0,5) "'" nl()
    "Full including Comma: '" fullname(h,1,0,6) "'" nl()
    "Full including Space: '" fullname(h,1,0,7) "'" nl()
    "Full including First: '" fullname(h,1,0,8) "'" nl()

    /* invalid indi */
    "Invalid INDI: "
    "'" fullname(w,1,1,50) "'" nl()

  "Ending Test" nl()
  }
}
