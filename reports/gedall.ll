/*
 * @progname       gedall.ll
 * @version        2000-02-20
 * @author         Paul B. McBride (pbmcbride@rcn.com)
 * @category       
 * @output         GEDCOM
 * @description
 *
 * This LifeLines report program produces a GEDCOM file containing
 * the entire LifeLines database, including header, trailer, and
 * submitter records. It also gives the option to keep or remove user defined
 * tags, and to remove any other tags.
 *
 * The default action is to remove all user defined tags. These are tags
 * which begin with an underscore, "_", character.
 *
 * When a tag line is removed, lines following it with higher level
 * numbers are also removed.
 *
 * This report program may require LifeLines 3.0.3 or later.
 *
 * Required files:
 *   header.ged, submit.ged (see samples at end of this file)
 *
 * This report program was tested on databases created from the Test Book
 * sample GEDCOM files at http://www.gentech.org
 *
 * 20 Feb 2000 Paul B. McBride (pbmcbride@rcn.com)
 */

global(REMOVEUSERTAGS)
global(REMOVELISTEDTAGS)
global(REMOVETAG_LIST)

global(removed_line_count)
global(removed_udt_count)

proc main ()
{
  list(REMOVETAG_LIST)		/* list of tags to be removed */
  set(REMOVELISTEDTAGS, 0)	/* set to 1 if there are tags to be removed */
  set(REMOVEUSERTAGS, askyn("Remove user defined tags (_*)"))
  set(removed_udt_count, 0)
  set(removed_line_count, 0)

  while(1) {
    getstrmsg(remtag, "Enter any other tag to be removed")
    if(gt(strlen(remtag),0)) {
    	set(REMOVELISTEDTAGS, 1)
    	enqueue(REMOVETAG_LIST, remtag)
    }
    else { break() }
  }

  copyfile("header.ged")	/* header file (references @SM1@) */

  "1 DATE " stddate(gettoday()) nl()

  copyfile("submit.ged")	/* submitter file (defines @SM1@) */

  set(icnt, 0)
  forindi(p, n) {
    call ged_write_node(root(p))
    set(icnt, add(icnt,1))
  }
  print(d(icnt), " INDI records (I*)...\n")
  set(fcnt, 0)
  forfam(f, n) {
    call ged_write_node(root(f))
    set(fcnt, add(fcnt,1))
  }
  print(d(fcnt), " FAM records (F*)...\n")
  set(ecnt, 0)
  foreven(e, n) {
    call ged_write_node(root(e))
    set(ecnt, add(ecnt,1))
  }
  print(d(ecnt), " EVEN records (E*)...\n")
  set(scnt, 0)
  forsour(s, n) {
    call ged_write_node(root(s))
    set(scnt, add(scnt,1))
  }
  print(d(scnt), " SOUR records (S*)...\n")
  set(ocnt, 0)
  forothr(o, n) {
    call ged_write_node(root(o))
    set(ocnt, add(ocnt,1))
  }
  print(d(ocnt), " other level 0 records (X*)\n")

  if(gt(removed_udt_count, 0)) {
    print(d(removed_udt_count), " user defined tag structures were removed.\n")
  }
  if(gt(removed_line_count, 0)) {
    print(d(removed_line_count), " lines were removed, as requested.\n")
  }

  "0 TRLR" nl()		/* trailer */
}

proc ged_write_node(n)
{
  set(remlevel, 10000)	/* this value is larger than the largest level number */
  traverse(n, m, level) {
    if(le(level, remlevel)) {
      set(remlevel, 10000) /* end of previous tag removal if any */
      if(REMOVEUSERTAGS) {
        if(t, tag(m)) {
	  if(eqstr(trim(t, 1), "_")) {
	    set(remlevel, level) /* remove line, and subordinate tag lines */
	    set(removed_udt_count, add(removed_udt_count, 1))
	  }
	}
      }
    }
    if(lt(level, remlevel)) {
      if(REMOVELISTEDTAGS) {
        if(t, tag(m)) {
	  forlist(REMOVETAG_LIST, rt, n) {
	    if(eqstr(t, rt)) {
	      set(remlevel, level)
	      break()
	    }
	  }
	}
      }
    }
    if(lt(level, remlevel)) {
      /* output this line to the GEDCOM file */
      d(level)
      if (xref(m)) { " " xref(m) }
      " " tag(m)
      if (v, value(m)) {
	" " v
      }
      "\n"
    }
    else {
      set(removed_line_count, add(removed_line_count, 1))
    }
  }
}

func askyn(msg)
{
  set(prompt, concat(msg, "? [y/n] "))
  getstrmsg(str, prompt)
  if(and(gt(strlen(str), 0),
     or(eq(strcmp(str, "n"),0), eq(strcmp(str, "N"),0)))) {
    return(0)
  }
  return(1)
}

/* same header.ged 
0 HEAD 
1 SOUR LIFELINES
2 VERS 3.0.5
2 NAME LifeLines
2 CORP T. T. Wetmore
3 ADDR ttw@shore.net
1 SUBM @SM1@
1 GEDC 
2 VERS 5.5
2 FORM Lineage-Linked
1 CHAR ASCII
* end of sample header.ged */

/* sample submit.ged
0 @SM1@ SUBM
1 NAME Your Name Here
1 ADDR Your Street Address
2 CONT Your City, State and Zip Code
2 CONT E-mail: your@email.address
* end of sample submit.ged */
