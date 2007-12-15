/*
 * @progname       gedmin.ll
 * @version        2007-12-15
 * @author         Perry Rapp
 * @category       
 * @output         GEDCOM
 * @description    Output only specified tags of a database to GEDCOM
 *
 * (Adapted from gedall.ll by Paul B. McBride)
 * Output only specific nodes of a database to GEDCOM
 *
 *   The gedcom header is generated in main() using property's
 *   obtained from the lifelines config file (~/.linesrc on unix else
 *   lines.cfg - or from properties set in the database) with values from
 *   the user defined properties
 *   user.fullname
 *   user.email
 *   user.address
 *
 * Note: The tag info is appended to the output GEDCOM file if is is chosen
 *  so remember to cut it out to make output valid GEDCOM
 *
 */
option("explicitvars") /* Disallow use of undefined variables */

global(print_tag_list)
global(print_tag_table)
global(print_tag_count)

global(removed_tag_table)
global(removed_tag_list)

global(removed_udt_table)
global(removed_udt_list)

global(removed_tag_count)
global(removed_udt_count)

proc main ()
{
  /* tags to be output */
  /* table to filter with, list to display afterwards */
  list(print_tag_list)
  table(print_tag_table)
  set(print_tag_count, 0)

  /* keep track of all distinct tags removed for display */
  /* table for uniqueness, list to build before and display after */
  table(removed_tag_table)
  list(removed_tag_list)

  /* keep track of all distinct UDTs removed for display */
  /* table for uniqueness, list to build before and display after */
  table(removed_udt_table)
  list(removed_udt_list)

  /* count # items removed, not only distinct but all */
  set(removed_udt_count, 0)
  set(removed_tags_count, 0)

  /* top-level records */
  call keep_tag("INDI")
  call keep_tag("SOUR")
  call keep_tag("FAM")
  /* not keeping EVEN/events */
  call keep_tag("SOUR")
  call keep_tag("NOTE")
  /* lineage-links */
  call keep_tag("HUSB")
  call keep_tag("WIFE")
  call keep_tag("CHIL")
  call keep_tag("FAMS")
  call keep_tag("FAMC")
  /* basic person info */
  call keep_tag("NAME")
  call keep_tag("SEX")
  call keep_tag("BIRT")
  call keep_tag("DEAT")
  call keep_tag("PLAC")
  call keep_tag("DATE")
  call keep_tag("MARR")
  /* basic source info */
  call keep_tag("AUTH")
  call keep_tag("TITL")

  set(removed_udt_count, 0)
  set(removed_line_count, 0)

  while(1) {
    getstrmsg(keeptag, "Enter any other tag to be kept")
    if(gt(strlen(keeptag),0)) {
      call keep_tag(keeptag)
    }
    else { break() }
  }

  call print_header()

  set(icnt, 0)
  forindi(p, n) {
    call traverse_node_subtree(root(p))
    set(icnt, add(icnt,1))
  }
  print(d(icnt), " INDI records (I*)...\n")
  set(fcnt, 0)
  forfam(f, n) {
    call traverse_node_subtree(root(f))
    set(fcnt, add(fcnt,1))
  }
  print(d(fcnt), " FAM records (F*)...\n")
  
  set(ecnt, 0)
  foreven(e, n) {
    call traverse_node_subtree(root(e))
    set(ecnt, add(ecnt,1))
  }
  print(d(ecnt), " EVEN records (E*)...\n")

  set(scnt, 0)
  forsour(s, n) {
    call traverse_node_subtree(root(s))
    set(scnt, add(scnt,1))
  }
  print(d(scnt), " SOUR records (S*)...\n")
  set(ocnt, 0)
  forothr(o, n) {
    call traverse_node_subtree(root(o))
    set(ocnt, add(ocnt,1))
  }
  print(d(ocnt), " other level 0 records (X*)\n")

  if(gt(removed_udt_count, 0)) {
    print(d(removed_udt_count), " user defined tag structures were removed.\n")
  }
  if(gt(removed_line_count, 0)) {
    print(d(removed_line_count), " lines were removed, as requested.\n")
  }

  call print_trailer()

  if (askyn("Add tag lists at end of file")) {
    call print_tags_info()
  }
}

proc print_tags_info()
{
  "---------------" nl()
  "PRESERVED ITEMS INFO" nl()
  "---------------" nl()

  if(length(removed_tag_list)) {
    "PRESERVED TAGS (" d(length(print_tag_list)) "):" nl()
	call print_list(print_tag_list, print_tag_table)
  } else {
    "NO PRESERVED TAGS" nl()
  }
  d(print_tag_count) " items preserved" nl()
  nl()

  "---------------" nl()
  "REMOVED ITEMS INFO" nl()
  "---------------" nl()

  if(length(removed_tag_list)) {
    "REMOVED TAGS (" d(length(removed_tag_list)) "):" nl()
	call print_list(removed_tag_list, removed_tag_table)
  } else {
    "NO REMOVED TAGS" nl()
  }
  d(removed_tag_count) " regular tags removed" nl()
  nl()
  
  if(length(removed_udt_list)) {
    "REMOVED UDTS (" d(length(removed_udt_list)) "):" nl()
	call print_list(removed_udt_list, removed_udt_table)
  } else {
    "NO REMOVED UDTs" nl()
  }
  d(removed_udt_count) " udts removed" nl()
}

proc print_list(alist, atable)
{
  set(wid, 0)
  forlist(alist, tag, ord) {
    set(item, concat(tag, " (", d(lookup(atable, tag)), ")"))
    /* output line return if needed */
    if (and(gt(wid, 0), gt(add(wid, strlen(item)), 70))) {
      "\n"
      set(wid, 0)
    }
    /* output , if not first item on line */
    if (gt(wid, 0)) {
      ", "
    }
    /* output tag item (tag and occurrence count) */
    item
    set(wid, add(wid, strlen(item)))
  }
  if (gt(wid,0)) {
    "\n"
  }
}

/* Add tag to collection of tags to output */
proc keep_tag(tag)
{
  if (not(lookup(print_tag_table, tag))) {
    insert(print_tag_table, tag, 1)
    enqueue(print_tag_list, tag)
  }
}

proc print_header()
{
  /* header file  */
  "0 HEAD " nl()
  "1 SOUR LIFELINES" nl()
  "2 VERS " version() nl()
  "2 NAME LifeLines" nl()
  /*
  "2 CORP ... "  nl()
  "3 ADDR .... " nl()
  */
  "1 SUBM @SM1@" nl()
  "1 GEDC " nl()
  "2 VERS 5.5" nl()
  "2 FORM Lineage-Linked" nl()
  "1 CHAR ASCII" nl()
  "1 DATE " stddate(gettoday()) nl()
  /* and referenced submitter */
  "0 @SM1@ SUBM" nl()
  "1 NAME " getproperty("user.fullname") nl()
  "1 ADDR " getproperty("user.address") nl()
  "2 CONT E-mail: " getproperty("user.email") nl()
}

proc print_trailer()
{
  "0 TRLR" nl()		/* trailer */
}

proc traverse_node_subtree(n)
{
  if (occur, lookup(print_tag_table, tag(n))) {
    incr(occur, 1)
    insert(print_tag_table, tag(n), occur)
    incr(print_tag_count, 1)
    call ged_write_node(n)
    fornodes(n, chil) {
      call traverse_node_subtree(chil)
    }
  } elsif (eqstr(trim(tag(n), 1), "_")) {
    set(occur, lookup(removed_udt_table, tag(n)))
    if (not(occur)) {
	  set(occur, 0)
      enqueue(removed_udt_list, tag(n))
    }
    incr(occur, 1)
    insert(removed_udt_table, tag(n), occur)
    incr(removed_udt_count, 1)
  } else {
    set(occur, lookup(removed_tag_table, tag(n)))
    if (not(occur)) {
	  set(occur, 0)
      enqueue(removed_tag_list, tag(n))
    }
    incr(occur, 1)
    insert(removed_tag_table, tag(n), occur)
    incr(removed_tag_count, 1)
  }
}

proc ged_write_node(n)
{
  /* output this line to the GEDCOM file */
  d(level(n))
  if (xref(n)) { " " xref(n) }
  " " tag(n)
  if (v, value(n)) {
    " " v
  }
  "\n"
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
