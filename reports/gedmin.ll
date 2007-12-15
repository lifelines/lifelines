/*
 * @progname       gedmin.ll
 * @version        2007-12-15b
 * @author         Perry Rapp
 * @category       
 * @output         GEDCOM
 * @description    Output only specified tags of a database to GEDCOM
 *
 * (Adapted from gedall.ll by Paul B. McBride)
 * Output only specific nodes of a database to GEDCOM
 *
 * See proc specify_tags below to adjust what tags are displayed
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
 * TechNote: occurrence counts in tables are +1, so that 0 is stored as 1
 *  (b/c lookup cannot distinguish 0 from not present)
 *
 */
option("explicitvars") /* Disallow use of undefined variables */

  /* tags to output for anyone & anything */
global(output_tag_list)
global(output_tag_table)

  /* tags to output for dead persons & marriages of dead persons */
global(output_tag_dead_list)
global(output_tag_dead_table)

  /* remaining globals are all just for tracking stats of what we did */
global(removed_tag_table)
global(removed_tag_list)

global(removed_tag_dead_table)
global(removed_tag_dead_list)

global(removed_udt_table)
global(removed_udt_list)

global(removed_udt_dead_table)
global(removed_udt_dead_list)

global(removed_tag_count)
global(removed_tag_dead_count)
global(removed_udt_count)
global(removed_udt_dead_count)
global(output_tag_count)
global(output_tag_dead_count)
global(living_indi_count)
global(dead_indi_count)
global(living_fam_count)
global(dead_fam_count)


  /* This is the settings for what to output */
proc specify_tags()
{
    /* top-level records */
  call keep_tag("INDI")
  call keep_tag("FAM")
  call keep_tag("SOUR")
    /* not keeping EVEN/events */
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
  call keep_tag_dead("BIRT")
  call keep_tag_dead("DEAT")
  call keep_tag_dead("PLAC")
  call keep_tag_dead("DATE")
  call keep_tag_dead("MARR")
    /* basic source info */
  call keep_tag("AUTH")
  call keep_tag("TITL")
}

proc main()
{
    /* tags to be output */
    /* table to filter with, list to display afterwards */
  list(output_tag_list)
  table(output_tag_table)

    /* tags to be output */
    /* table to filter with, list to display afterwards */
  list(output_tag_dead_list)
  table(output_tag_dead_table)

    /* keep track of all distinct tags removed for display */
    /* table for uniqueness, list to build before and display after */
  table(removed_tag_table)
  list(removed_tag_list)
  table(removed_tag_dead_table)
  list(removed_tag_dead_list)

    /* keep track of all distinct UDTs removed for display */
    /* table for uniqueness, list to build before and display after */
  table(removed_udt_table)
  list(removed_udt_list)
  table(removed_udt_dead_table)
  list(removed_udt_dead_list)

    /* count # items removed (all items, not just distinct ones) */
  set(removed_udt_count, 0)
  set(removed_tags_count, 0)
  set(living_indi_count, 0)
  set(dead_indi_count, 0)
  set(living_fam_count, 0)
  set(dead_fam_count, 0)

  call specify_tags()

    /* max width of lines when outputting tag lists */
  set(linewid, 70)

  set(removed_udt_count, 0)
  set(removed_line_count, 0)

    /* Allow user to add tags to keep */
  while(1) {
    getstrmsg(keeptag, "Enter any other tag to be kept")
    if(gt(strlen(keeptag),0)) {
      call keep_tag(keeptag)
    }
    else { break() }
  }

    /* Allow user to add tags to keep for dead people */
  while(1) {
    getstrmsg(keeptag, "Enter any other tag to be kept for dead")
    if(gt(strlen(keeptag),0)) {
      call keep_tag_dead(keeptag)
    }
    else { break() }
  }

  call print_header()

  call traverse_database()

  call print_trailer()

  if (askyn("Add tag lists at end of file")) {
    call print_tags_info(linewid)
  }
}

  /* call traverse_node_subtree for all nodes in subtree */
  /* (except ones chopped off higher up) */
proc traverse_database()
{
  set(icnt, 0)
  forindi(p, n) {
    set(dead, is_indi_dead(p))
    call traverse_node_subtree(root(p), dead)
    if (dead) { incr(dead_indi_count) } else { incr(living_indi_count) }
    incr(icnt, 1)
  }
  print(d(icnt), " INDI (I*) records (L:", d(living_indi_count), ", D:", d(dead_indi_count), ").\n")

  set(fcnt, 0)
  forfam(f, n) {
    set(dead, is_fam_dead(f))
    call traverse_node_subtree(root(f), dead)
    if (dead) { incr(dead_fam_count) } else { incr(living_fam_count) }
    incr(fcnt, 1)
  }
  print(d(fcnt), " FAM (F*) records (L:", d(living_fam_count), ", D:", d(dead_fam_count), ").\n")
  
  set(ecnt, 0)
  foreven(e, n) {
    set(dead, 0)
    call traverse_node_subtree(root(e), dead)
    incr(ecnt, 1)
  }
  print(d(ecnt), " EVEN (E*) records.\n")

  set(scnt, 0)
  forsour(s, n) {
    set(dead, 0)
    call traverse_node_subtree(root(s), dead)
    incr(scnt, 1)
  }
  print(d(scnt), " SOUR (S*) records.\n")

  set(ocnt, 0)
  forothr(o, n) {
    set(dead, 0)
    call traverse_node_subtree(root(o), dead)
    incr(ocnt, 1)
  }
  print(d(ocnt), " other level 0 (X*) records.\n")

  print(d(add(output_tag_count, output_tag_dead_count))
    , " items output (", d(output_tag_count), "/", d(output_tag_dead_count), "\n")

  print(d(add(removed_tag_count, removed_tag_dead_count, removed_udt_count, removed_udt_dead_count))
    , "  items removed (", d(removed_tag_count), "/", d(removed_tag_dead_count)
	, "/", d(removed_udt_count), "/", d(removed_udt_dead_count), "\n")
}

func is_indi_dead(p)
{
  set(dt, death(p))
  if (not(dt)) {
    return(0) /* no death event */
  }
  if (eqstr(value(dt), "Y")) {
    return(1) /* "DEAT Y" mean dead */
  }
  if (child(dt)) {
    return(1) /* if DEAT node has children, we'll call it dead */
  }
  return(0) /* apparently placeholder empty DEAT line */
}

func is_fam_dead(f)
{
  set(spct, 0)
  spouses(f, sp, ord) {
    incr(spct, 1)
    if (not(is_indi_dead(sp))) {
      return(0) /* has living spouse */
    }
  }
  if (spct) {
    return(1) /* has at least one dead spouse */
  } else {
    return(0) /* no spouses */
  }
}

proc print_tags_info(linewid)
{
  nl()
  "---------------" nl()
  "PRESERVED ITEMS INFO" nl()
  "---------------" nl()

  if(length(output_tag_list)) {
    "PRESERVED TAGS (" d(length(output_tag_list)) "):" nl()
	call print_list(output_tag_list, output_tag_table, linewid)
  } else {
    "NO PRESERVED TAGS" nl()
  }
  d(output_tag_count) " lines preserved" nl()
  nl()

  if(length(output_tag_dead_list)) {
    "PRESERVED TAGS (DEAD)(" d(length(output_tag_dead_list)) "):" nl()
	call print_list(output_tag_dead_list, output_tag_dead_table, linewid)
  } else {
    "NO PRESERVED TAGS (DEAD)" nl()
  }
  d(output_tag_dead_count) " additional dead lines preserved" nl()
  nl()

  d(add(output_tag_count, output_tag_dead_count)) " total lines preserved" nl()
  nl()


  "---------------" nl()
  "REMOVED ITEMS INFO" nl()
  "---------------" nl()

  if(length(removed_tag_list)) {
    "REMOVED TAGS (NON-DEAD)(" d(length(removed_tag_list)) "):" nl()
	call print_list(removed_tag_list, removed_tag_table, linewid)
  } else {
    "NO REMOVED TAGS (NON-DEAD)" nl()
  }
  d(removed_tag_count) " regular tags removed (non-dead)" nl()
  nl()
  
  if(length(removed_udt_list)) {
    "REMOVED UDTS (NON-DEAD)(" d(length(removed_udt_list)) "):" nl()
	call print_list(removed_udt_list, removed_udt_table, linewid)
  } else {
    "NO REMOVED UDTs (NON-DEAD)" nl()
  }
  d(removed_udt_count) " udts removed (non-dead)" nl()
  nl()

  if(length(removed_tag_dead_list)) {
    "REMOVED TAGS (DEAD)(" d(length(removed_tag_dead_list)) "):" nl()
	call print_list(removed_tag_dead_list, removed_tag_dead_table, linewid)
  } else {
    "NO REMOVED TAGS (DEAD)" nl()
  }
  d(removed_tag_dead_count) " regular tags removed (dead)" nl()
  nl()
  
  if(length(removed_udt_dead_list)) {
    "REMOVED UDTS (DEAD)(" d(length(removed_udt_dead_list)) "):" nl()
	call print_list(removed_udt_dead_list, removed_udt_dead_table, linewid)
  } else {
    "NO REMOVED UDTs (DEAD)" nl()
  }
  d(removed_udt_dead_count) " udts removed (dead)" nl()
  nl()

  d(add(removed_tag_count, removed_tag_dead_count)) " total regular tags removed (dead & non-dead)" nl()
  d(add(removed_udt_count, removed_udt_dead_count)) " total udts removed (dead & non-dead)" nl()
  nl()

  d(add(removed_tag_count, removed_tag_dead_count, removed_udt_count, removed_udt_dead_count))
     " total tags and udts removed (dead & non-dead)" nl()
  nl()

  "---------------" nl()
  "TRAVERSED ITEMS INFO" nl()
  "---------------" nl()

  "INDI: living=" d(living_indi_count) ", dead=" d(dead_indi_count) ", total="
    d(add(living_indi_count, dead_indi_count)) "\n"

  "FAMI: living=" d(living_fam_count) ", dead=" d(dead_fam_count) ", total="
    d(add(living_fam_count, dead_fam_count)) "\n"

}

  /* dump contents of a list to output, multiple items per line */
proc print_list(alist, atable, linewid)
{
  set(wid, 0)
  forlist(alist, tag, ord) {
    set(item, concat(tag, " (", d(add(lookup(atable, tag),-1)), ")"))
    /* output line return if needed */
    if (and(gt(wid, 0), gt(add(wid, strlen(item)), linewid))) {
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
  if (lookup(output_tag_table, tag)) {
    print("keep_tag called again for tag: ", tag, "\n")
  } else {
    insert(output_tag_table, tag, 1)
    enqueue(output_tag_list, tag)
  }
}

  /* Add tag to collection of tags to output for dead people*/
proc keep_tag_dead(tag)
{
  if (lookup(output_tag_table, tag)) {
    print("keep_tag_dead called for tag after keep_tag: ", tag, "\n")
  } elsif (lookup(output_tag_dead_table, tag)) {
    print("keep_tag_dead called again for tag: ", tag, "\n")
  } else {
    insert(output_tag_dead_table, tag, 1)
    enqueue(output_tag_dead_list, tag)
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

proc traverse_node_subtree(n, dead)
{
    /* first see if list is on the always keep list */
  if (occur, lookup(output_tag_table, tag(n))) {
    incr(occur, 1)
    insert(output_tag_table, tag(n), occur)
    incr(output_tag_count, 1)
    call ged_write_node(n)
    fornodes(n, chil) {
      call traverse_node_subtree(chil, dead)
    }
   return(0)
  }
  if (dead) {
      /* is tag on the keep if dead list ? */
    if (occur, lookup(output_tag_dead_table, tag(n))) {
      incr(occur, 1)
      insert(output_tag_dead_table, tag(n), occur)
      incr(output_tag_dead_count, 1)
      call ged_write_node(n)
      fornodes(n, chil) {
        call traverse_node_subtree(chil, dead)
      }
    }
  }
  /* record being dropped, record stats */
  if (eqstr(trim(tag(n), 1), "_")) {
      /* udt being dropped */
    if (dead) {
      set(occur, lookup(removed_udt_dead_table, tag(n)))
      if (not(occur)) {
  	  set(occur, 1)
        enqueue(removed_udt_dead_list, tag(n))
      }
      incr(occur, 1)
      insert(removed_udt_dead_table, tag(n), occur)
      incr(removed_udt_dead_count, 1)
    } else {
      set(occur, lookup(removed_udt_table, tag(n)))
      if (not(occur)) {
  	  set(occur, 1)
        enqueue(removed_udt_list, tag(n))
      }
      incr(occur, 1)
      insert(removed_udt_table, tag(n), occur)
      incr(removed_udt_count, 1)
    }
  } else {
      /* regular tag being dropped */
    if (dead) {
      set(occur, lookup(removed_tag_dead_table, tag(n)))
      if (not(occur)) {
        set(occur, 1)
        enqueue(removed_tag_dead_list, tag(n))
      }
      incr(occur, 1)
      insert(removed_tag_dead_table, tag(n), occur)
      incr(removed_tag_dead_count, 1)
    } else {
      set(occur, lookup(removed_tag_table, tag(n)))
      if (not(occur)) {
        set(occur, 1)
        enqueue(removed_tag_list, tag(n))
      }
      incr(occur, 1)
      insert(removed_tag_table, tag(n), occur)
      incr(removed_tag_count, 1)
    }
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
