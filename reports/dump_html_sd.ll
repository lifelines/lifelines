/*
 * @progname       dump_html_sd.ll
 * @version        1.0
 * @author         Scott McGee, Steve Dum
 * @category       sample
 * @output         HTML
 * @description

dump_html_sd.ll dumps an entire database to static HTML files.

by Scott McGee (smcgee@microware.com)
                        1.4 13/06/02
lots of modifications by Steve Dum (stephen.dum@verizon.net)

This report basically generate a
master index and write a series of html files for everyone in
your database.  It generates both decendant and pedigree charts for
each individual.    There are lots of assumptions in how you store
data in your database inbeded in the script -- or should I say how I
store info. SD.

*/

/* customization globals - customize values assigned in main */
global(db_owner)       /* name of database owner */
global(owner_addr)     /* url of database owner (mailto or homepage) */
global(use_image)      /* flag to indicate whether to use genweb image */
global(genweb_image)   /* name of genweb image to place on each page */
global(use_page)       /* flag to add link to genweb page or homepage */
global(genweb_page)    /* URL of base genweb (or homepage) web page */
global(page_name)      /* name of base genweb (or homepage) web page */
global(LDS)            /* display LDS Ordinances? (1=yes 0=no) */
global(html_index)     /* put HTML <ISINDEX> tag in INDEX file (1=yes 0=no) */


proc set_static_html_globals(){
/* customize these globals to customize the output to your site */
  set(db_owner, getproperty("user.fullname"))
  set(owner_addr, getproperty("user.email"))
  set(use_image, 0)                /* 1 to use image, 0 to not use image */
  set(genweb_image, "../../pics/genweb.gif")
  set(use_page, 0)                 /* 1 to use link to page, 0 if not */
  set(genweb_page, "../genweb.html")
  set(page_name, "genweb page")    /* might change to "my homepage" */
  set(LDS, 0)
  set(html_index, 0)               /* use 1 to add <ISINDEX> to INDEX file */
}
/* end of customization globals - customize values assigned in main */

/* other globals */
global(found)          /* external file to inline image found flag */
global(per_file)       /* number of people per file to write */
global(first)          /* first person shouldn't be asked about */

global(RVAL)           /* ?? (part of borrowed code) */
global(last_surname)   /* last surname in index - used for anchors */
global(first_indi)     /* starting person */
global(sour_count)     /* count of source records */
global(sourcnt)        /* count of source records when doing birth/death */
global(href_table)     /* table that has filenames individuals are in */

global(linecount)	/* globals for descendent chart */
global(MAXLINES)
global(gens)
global(mainpre)
global(spousepre)
global(indentpre)

proc main () {

  set(gens, 3)
  set(mainpre, "-")
  set(spousepre, " s-")
  set(indentpre, "  |")
  set(MAXLINES, 500)

  set(sour_count,0)
  indiset(out_set)

  call set_static_html_globals()
  set(per_file, 1)

  print("Reading data...\n")
  forindi(j, n) {
    addtoset(out_set, j, n)
  }
  print("Working...\n")
  call html_out(out_set)
}

proc html_out (o) {
  /*
  set(s, concat("There are ", d(lengthset(o)),
                " people in your list, how many per file?"))
  getstr(a,s)
  set(per_file, atoi(a))
  */
  set(per_file,50)

  if (not(per_file)) {
    set(per_file, 1)
  }
  set(loop_count, 0)
  set(file_count, 0)
  call init_href(o)
  forindiset(o, i, j, n) {
    set(indi, i)
    if (eq(loop_count, 0)) {
      incr(file_count)
      call write_head(file_count)
    }
    incr(loop_count)
    call genhtml(indi, o)
    if (or(eq(loop_count, per_file), eq(n, lengthset(o)))) {
      call write_tail()
      set(loop_count, 0)
    }
  }

  call do_index(o)

}

proc write_head(count) {
  set(filename, concat("html/", database(), "/",database(),"_", d(count), ".html"))
  print("Writing ", filename, "\n")
  newfile(filename, 0)
  "<HTML><HEAD>\n"
  "<TITLE> " database() "_" d(count) ".html </TITLE>\n"
  "</HEAD><BODY>\n"
  "<style type=\"text/css\">\n"
  "p.hindent { margin-top: 0.2em; margin-bottom:0em;\n"
  "            text-indent: -2em; padding-left: 2em;}\n"
  "p.indent { margin-top: 0.2em; margin-bottom:0em;\n"
  "            text-indent: 0em; padding-left: 2em;}\n"
  "</style>\n"
  if (use_image) {
    "<IMG SRC=\""
    genweb_image
    "\" ALT = \"\"><BR><BR>\n"
  }
}

proc write_tail() {
  "<BR><HR><ADDRESS>\n"
  date(gettoday())
  "<BR>\n"
  "Database maintained by "
  "<A HREF=\"" owner_addr "\">\n"
  db_owner
  "</A></ADDRESS>\n"
  "</BODY></HTML>\n"
}

proc genhtml (i, o) {
/*  print("        ", fullname(i,0,1,300), "\n") */
  "<A NAME=\"" key(i) "\"></A>\n"
  /* was <H1>...</H1> but fudged to get key in slightly smaller font */
  "<P><FONT size=+2><B>"
  givens(i) " " surname(i)
  "  <Font size=-1>(" key(i) ")</Font></B></FONT><P>\n"

  set(path, get_picture(i))
  if (found) {
    "<IMG width=30% SRC =\""
    if (eq(found,2)) { "../../" }
    path
    "\" ALT = \"\"><BR>\n"
  }
  "<H3>" date(birth(i)) " - " date(death(i)) "</H3>" nl()
  call afn(i)
  call scan_events(i,0)
  if (LDS) {
    /* LDS ordinances */
    set(started, 0)
    fornodes(inode(i), node) {
      if (eq(0, strcmp(tag(node), "BAPL"))) {
        if (not(started)) {
          set(started, 1)
          "<BR>LDS Ordinances: B "
        }
      }
      /* determine if endowed */
      if (eq(0, strcmp(tag(node), "ENDL"))) {
        if (not(started)) {
          set(started, 1)
          "<BR>LDS Ordinances: "
        }
        "E "
      }
    }
    /* determine if sealed to parents */
    set(fam, fnode(parents(i)))
    set(ind, inode(i))
    if (fam) {
      fornodes(fam, node) {
        if (and(eqstr("CHIL", tag(node)), eqstr(xref(ind), value(node)))) {
          fornodes(node, next) {
            if (eqstr(tag(next), "SLGC")) {
              if (not(started)) {
                set(started, 1)
                "<BR>LDS Ordinances: "
              }
              "SC "
            }
          }
        }
      }
    }
    if (started){
      "<BR>\n"
    }
  }
  call othernames(i)
  call print_html(i)
  "<HR>\n"
  if (p, father(i)) {
    "<P class=\"hindent\">\n"
    "<EM>" "Father</EM> : "
    call name_href(p,1)
    "<BR>\n"
  }
  if (p, mother(i)) {
    "<P class=\"hindent\">\n"
    "<EM>" "Mother</EM> : "
    call name_href(p,1)
    "<BR>\n"
  }
  families(i, f, s, n) {
    "<P class=\"hindent\">\n"
    "<EM>" "Spouse"
    if (gt(nfamilies(i), 1)) {
      " "
      d(n)
    }
    "</EM> : \n"
    if (s) {    /* family has a spouse */
      call name_href(s,1)
      "<BR>\n"
    }
    if (e, marriage(f)) {
      "<P class=\"indent\">\n"
      "<EM>Married</EM> "
      long(e)
      "<BR>\n"
    }
    if (e, divorced(f)) {
      "<P class=\"indent\">\n"
      "<EM>Divorced</EM> "
      long(e)
      "<BR>\n"
    }
    if (LDS) {
      fornodes(fnode(f), node) {
        if (eq(0, strcmp(tag(node), "SLGS"))) {
          "<BR>LDS Ordinances: SS\n"
        }
      }
    }
    if (nchildren(f)) {
	"<P class=\"hindent\">\n"
	"<EM>Children</EM>\n"
	"<OL style=\"margin-top: 0.2em;\">\n"
	children(f, c, nn) {
	  "<LI>"
	  call name_href(c,1)
	  "</LI>\n"
	}
	"</OL>\n"
    }
  }

  call print_notes(inode(i))

  call print_indi_sources(i)

  if (parents(i)) {
    "<P>\nPedigree Chart for " fullname(i,0,1,300) "<BR>\n<PRE>\n"
     call pedigree(0,i)
     "</PRE>\n"
  }
  if (nfamilies(i)) {
    "Descendent Chart for " fullname(i,0,1,300) "<BR>\n<PRE>\n"
    set(linecount,0)
    call dofam(i,"",1,0)
    "</PRE>\n"
  }

  /* scan events for sources */
  call scan_events(i,1)
  "<HR>\n"

/*  Insert code here for Pedigree and Descendant charts
  if (parents(i)) {
    "Pedigree Chart<BR>\n"
  }
  if (nfamilies(i)) {
    set(hasChildren, 0)
    families(i, f, s, n) {
      if (nchildren(f)) {
        set(hasChildren, 1)
      }
    }
    if (hasChildren) {
      "Descendant Chart\n"
    }
  }
  "<HR>\n"
*/
  "<BR>\n"
  "[<A HREF=\"index.html\">"
  "Index</A>]<BR>\n"
  if (use_page) {
    "[<A HREF=\"" genweb_page "\">"
    "Return to "
    page_name
    " </A>]<BR> \n"
  }
  "<BR><HR><BR>\n"
}

proc init_href(outset){
  table(href_table)

  forindiset(outset, indi, j, number) {
    insert(href_table, save(key(indi)), number)
  }
}

func get_href(indi) {
  set(path, "")
  set(found, 0)
  set(value, lookup(href_table, key(indi)))
  if (value){
    set(number, add(div(sub(value, 1), per_file), 1))
    set(path, concat(database(),"_", d(number), ".html"))
    set(found, 1)
  }
  return(path)
}

proc print_notes(i){
    set(first, 1)
    fornodes(i, n) {
	set(hdr,"")
	if (eqstr(tag(n), "NOTE")) { set(hdr,"Note") }
	elsif(eqstr(tag(n),"NOTE_L")) { set(hdr,"Note_L") }
	elsif(eqstr(tag(n),"NOTE_Q")) { set(hdr,"Note_Q") }
	elsif(eqstr(tag(n),"NOTE_E")) { set(hdr,"Note_E") }
	if (strcmp(hdr,"")) {
	    set(s, value(n))
	    if (and(strcmp("",s),reference(s))) {
		set(n,dereference(s))
		set(hdr, save(concat(hdr," ",substring(s,2,sub(strlen(s),1)))))
	    }	
	    print_note(n,hdr)
	}
    }
}

proc print_indi_sources(indi) {
    fornodes(inode(indi), node) {
	set(hdr,"")
	set(ntag,tag(node))
	set(first,0)
	if (eqstr("SOUR", ntag)) { set(hdr,"Source") }
	elsif(eqstr("ADDR", ntag)) { set(hdr,"Address") }
	elsif(eqstr("_MDCL", ntag)) { set(hdr,"Medical") }
	elsif(eqstr("OCCU", ntag)) { set(hdr,"Occupation") }
	elsif(eqstr("_FA", trim(ntag,3))) { set(hdr,"Fact") }
	if (strcmp("",hdr)) {
	    if (reference(value(node))) { set(node,dereference(value(node))) }
	    "<P class=\"hindent\">\n"
	    "<EM>" hdr ":</EM>: "
	    if (nestr("",value(node))) {
	        incr(first)
	        value(node) "<BR>\n"
	    }
	    fornodes(node, next) {
		if (nestr("",value(next))) {
		    /* if(eq(first,1)) { "<BR>" } */
		    incr(first)
		    value(next) "<BR>\n"
		}
		fornodes(next, nn) {
		    if (nestr("",value(next))) {
			/* if(eq(first,1)) { "<BR>" } */
			incr(first)
			value(nn) "<BR>\n"
		    }
		}
	    }
	    if(gt(first,1)) { "\n" }
	}
    }
}

proc show_path (node){
  list(path)
  while (node) {
    push(path, tag(node))
    set(node, parent(node))
  }
  "("
  while (s, pop(path)) {
    if (eqstr(lower(s), "indi")){
      "Individual "
    }elsif (eqstr(lower(s), "fam")){
      "Family "
    }elsif (eqstr(lower(s), "famc")){
      "family "
    }elsif (eqstr(lower(s), "fams")){
      "family "
    }elsif (eqstr(lower(s), "note")){
      "note"
    }elsif (eqstr(lower(s), "birt")){
      "birth "
    }elsif (eqstr(lower(s), "deat")){
      "death "
    }elsif (eqstr(lower(s), "buri")){
      "burial "
    }elsif (eqstr(lower(s), "plac")){
      "place "
    }else{
      lower(s)
      " "
    }
  }
  ")\n"
}

proc do_index(indi_set) {
  set(last_surname, "ZZ")
  list(RVAL)
  indiset(index)

  set(index, indi_set)
  namesort(index)
  print("Writing index.html\n")
  call create_index_file(index)
  print("Writing gendex.txt\n")
  call create_gendex_file(index)
}

proc create_gendex_file(index) {
  set(fn, save(concat("html/", database(), "/gendex.txt")))
  newfile(fn, 0)
  forindiset(index, me, v, n)
  {
    set(path, concat(save(get_href(me)), "#", key(me)))
    path
    "|"
    surname(me)
    "|"
    givens(me) " /"
    surname(me) "/"
    "|"
    if (evt, birth(me)) {
      date(evt)
    }
    "|"
    if (evt, birth(me)) {
      place(evt)
    }
    "|"
    if (evt, death(me)) {
      date(evt)
    }
    "|"
    if (evt, death(me)) {
      place(evt)
    }
    "|\n"
  }
}

proc create_index_file(index) {
  set(fn, save(concat("html/", database(), "/index.html")))
  newfile(fn, 0)
  call html_header(concat("Index for ",database()," Database"), html_index)
  "<BODY>\n"
  if (use_image) {
    "<IMG SRC=\""
    genweb_image
    "\" ALT = \"\"><BR><BR>\n"
  }
  "<H1> Index </H1>\n"
  "<UL>\n"
  forindiset(index, me, v, n)
  {
    call href(me)
    "\n"
  }
  "</UL>\n"
  call write_tail()
}

/* href generates html link reference for name in form last,first */
proc href(indi) {
  if (indi) {
    call print_name(indi, 1)
    if (ne(strcmp(upper(surname(indi)), last_surname), 0)) {
       print("        ", upper(surname(indi)), "\n")
       set(last_surname, save(upper(surname(indi))))
       "<A NAME=" qt() last_surname qt() "></A>\n"
    }
    "<LI>"
    set (path, get_href(indi))
    if (found) {
      "<A HREF=\"" path "#" key(indi) "\">\n"
    }
    pop(RVAL)
    if (found) {
      "</A>"
    }
    do_info(indi,1)
  }
}

/* name_href generates html link reference for name */
proc name_href(indi,long) {
    set (path, get_href(indi))
    if (found) {
      "<A HREF=\"" path "#" key(indi) "\">"
    }
    if (t,title(indi)) { t " " }
    fullname(indi,0,1,300)
    if (found) {"</A>"}
    do_info(indi,long)
}

proc html_header(str, isindex) {
  "<HTML>\n"
  "<HEAD>\n"
  if (isindex) {
    "<ISINDEX>\n"
  }
  "<TITLE> "
  str
  " </TITLE>\n"
  "</HEAD>\n"
 }

proc print_name (me, last) {
  call get_title(me)
  set(junk, pop(RVAL))
  push(RVAL, save(concat(fullname(me, 1, not(last), 45), junk)))
}

proc get_title (me) {
  fornodes(inode(me), node) {
    if (not(strcmp("TITL", tag(node)))) {
      set(n, node)
    }
  }
  if (n) {
    push(RVAL, save(concat(" ", value(n))))
  }
  else {
    push(RVAL, "")
  }
}

proc rjt(n, w) {
  if (lt(n, 10)) {
    set(d, 1)
  }
  elsif (lt(n, 100)) {
    set(d, 2)
  }
  elsif (lt(n, 1000)) {
    set(d, 3)
  }
  elsif (lt(n, 10000)) {
    set(d, 4)
  }
  else  {
    set(d, 5)
  }
  if (lt(d, w)) {
    set(pad, save( trim("      ", sub(w, d))))
  }
  else{
    set(pad, "")
  }
  push(RVAL, save( concat(pad, save(d(n)))))
}

proc othernames(indi){
  if (indi){
    set(count, 0)
    fornodes(inode(indi), subnode){
      if (eqstr(tag(subnode), "NAME")){
        incr(count)
        if (eq(count, 2)){
          "<BR><EM>Other Names</EM>: \n<UL>"
          "<LI>" call nameval(subnode) "</LI>"
        } elsif (gt(count, 2)){
          "<LI>" call nameval(subnode) "</LI>\n"
        }
      } elsif (eqstr(tag(subnode),"ALIA")){
        incr(count)
        if (eq(count, 2)){
          "<BR><EM>Other Names</EM>: \n<UL>"
          "<LI>" value(subnode) "</LI>"
        } elsif (gt(count, 2)){
          "<LI>" value(subnode) "</LI>\n"
        }
      }
    }
    if (gt(count, 1)){
      "</UL>\n"
    }
  }
}

proc afn(indi){
  if (indi){
    fornodes(inode(indi), subnode){
      if (eqstr(tag(subnode), "AFN")){
        "AFN "
        value(subnode)
        "<BR><BR>\n"
      }
    }
  }
}

func do_info(me,long){
    if(not(me)){
	return("")
    }
    set(out, " -")
    if (evt, birth(me)) {
      if (long) {
	  set(out, concat(out, " b. ", long(evt)))
      } else {
	  set(out, concat(out, " b. ", short(evt)))
      }
    } else {
      if (evt, baptism(me)) {
	if (long) {
	    set(out, concat(out, " bapt. ", long(evt)))
	} else {
	    set(out, concat(out, " bapt. ", short(evt)))
	}
      } else {
	if (evt, bapt(me)) {
	    if (long) {
		set(out, concat(out, " bapt. ", long(evt)))
	    } else {
		set(out, concat(out, " bapt. ", short(evt)))
	    }
	}
      }
    }
    if (evt, death(me)) {
      if (long) {
	  set(out, concat(out, " d. ", long(evt)))
      } else {
	  set(out, concat(out, " d. ", short(evt)))
      }
    }
    return(out)
}

func bapt (indi) {
  fornodes(inode(indi), node) {
    if (eq(0, strcmp(tag(node), "BAPL"))) {
      return(node)
    }
    if (eq(0, strcmp(tag(node), "BAPM"))) {
      return(node)
    }
    if (eq(0, strcmp(tag(node), "BAPT"))) {
      return(node)
    }
  }
  return(0)
}

proc nameval(namenode){
  list(np)
  extractnames(namenode, np, nc, sc)
  forlist(np, v, i){
    v
    " "
  }
}

proc print_html(indi){
  fornodes(inode(indi), node) {
    if (eqstr("REPORT", tag(node))) {
      set(m, child(node))
      if (eqstr("TYPE", tag(m))) {
        if (eqstr("HTML", value(m))) {
          "<BR>\n"
          fornodes(m, o) {
            if (eqstr("DATA", tag(o))) {
              value(o)
              "\n"
            }
          }
        }
        else {
          if (eqstr("HTML-STATIC", value(m))) {
            "<BR>\n"
            fornodes(m, o) {
              if (eqstr("DATA", tag(o))) {
                value(o)
                "\n"
              }
            }
          }
        }
      }
    }
  }
}

func divorced(fam) {
  fornodes(fnode(fam), node) {
    if (eq(0, strcmp(tag(node), "DIV"))) {
      return(node)
    }
  }
  return(0)
}

proc scan_events(indi, flag) {
    set(sourcnt,sour_count)
    fornodes(inode(indi),e) {
	set(match,1)
	if (eq(0, strcmp(tag(e), "BIRT"))) {
	    set (type,"Birth")
	} elsif (eq(0, strcmp(tag(e), "DEAT"))) {
	    set (type,"Death")
	} elsif (eq(0, strcmp(tag(e), "BAPL"))) {
	    set (type,"Baptism")
	} elsif (eq(0, strcmp(tag(e), "CHR"))) {
	    set (type,"Christening")
	} elsif (eq(0, strcmp(tag(e), "BURI"))) {
	    set (type,"Burial")
	} elsif (eq(0, strcmp(tag(e), "BAPT"))) {
	    set (type,"Baptism")
	} elsif (eq(0, strcmp(tag(e), "BAPM"))) {
	    set (type,"Baptism")
	} else {
	    set(match,0)
	}
        if (eq(match,1)) {
	    if (flag) {
		/* flag == 1 print notes with sources */
		print_sources(e,type)
	    } else {
		"<EM>" type "</EM> : " long(e)
		fornodes(e, s) {
		   if (and(eqstr(tag(s), "SOUR"),nestr(value(s),""))) {
		       incr(sourcnt)
		       " [<A HREF=\"#" d(sourcnt) "\">" d(sourcnt) "</A>]"
		    }
		}
		"<BR>\n"
	    }
	}
   }
}

func print_sources(e,t) {
    fornodes(e, s) {
        if (and(eqstr(tag(s), "SOUR"),nestr(value(s),""))) {
           incr(sour_count)
	   "<P>\n<A NAME=\"" d(sour_count) "\">[<B>" d(sour_count)
	   "</B>]</A>[Source " t "]\n" value(s) nl()
        fornodes(s, n) {
	    set(hdr,"")
	    if (eqstr(tag(n), "NOTE")) { set(hdr,"Note") }
	    elsif(eqstr(tag(n),"NOTE_L")) { set(hdr,"Note_L") }
	    elsif(eqstr(tag(n),"NOTE_Q")) { set(hdr,"Note_Q") }
	    elsif(eqstr(tag(n),"NOTE_E")) { set(hdr,"Note_E") }
	    if (strcmp(hdr,"")) {
		set(s, value(n))
		if (and(strcmp("",s),reference(s))) {
		    set(n,dereference(s))
		    set(hdr,save(concat(hdr," ",s)))
		}
		print_note(n,hdr)
	    }
	}
	}
    }
}

func print_note(node,hdr) {
    "<P class=\"hindent\">\n"
    "<EM>" hdr "</EM>: "
    if (strcmp("",value(node))) {
	value(node) nl()
    }
    fornodes(node,next) {
        set(ctag,tag(next))
	if (or(eqstr("CONT",ctag),eqstr("CONC",ctag))) {
	    if (eqstr("CONT",ctag)) { "<BR>" nl() }
	    value(next)
	}
    }
    "</P>\n"
}

func get_picture (indi) {
/* Note: this code assumes the following tag sturcture
   return found==1 if url, found==2 if FILE

1 _PIC
  2 FILE pics/scott.gif
  2 DATE Jul 1989
1 _PIC
  2 URL http://www.someurl.net/~user/userpic.gif

   where the first defines an external file stored on the same file
   system and gives the path in the FILE record and the type in the
   FORM record. The second defines an external file stored on another
   site and provides a URL for referencing it. I have proposed this as
   an extension to GEDCOM, but nobody said very much.
*/

  set(found, 0)
  set(path, "")
  fornodes(inode(indi), node) {
    if (eqstr("_PIC", tag(node))) {
      set(m, child(node))
      /* files on local system  or file on remote system */
      if (or(eqstr("FILE", tag(m)),eqstr("URL", tag(m)))) {
        set(path, value(m))
	incr(found)
	if (eqstr("FILE",tag(m))) { incr(found) }
      }
    }
  }
  return(path)
}

proc pedigree (level, indi) {
    set(has_parent, or(father(indi), mother(indi)))
    if (and(lt(level, 4), has_parent)) {
	call pedigree(add(1,level), father(indi))
    }
    if (indi) {
	col(mul(4,level))
	call name_href(indi,le(level,3))
	nl()
    } else {
  	col(mul(4,level))
  	"(Spouse not known)"
	nl()
    }
    if (and(lt(level, 4), has_parent)) {
	call pedigree(add(1,level), mother(indi))
    }
}

/* startfam:
   If we haven't reached the maximum or specified generation count,
   call dofam for each child in this family.
   Otherwise, print a message line if there are further descendants
   at this point.
*/
proc startfam (fam,prefix,level,isstep) {
    if (le(level,gens)) {               /* if not at last generation */
        children (fam,child,num) {      /* for each child */
            call dofam (child,          /* call dofam */
                        concat(prefix, indentpre),
                        add(level,1),
                        isstep)
        }
    } else {                            /* don't do this generation */
        if (gt(nchildren(fam),0)) {     /* but if there are children, */
                                        /* issue message */
            prefix "  [[Further descendants here"
            if (eq(isstep,1)) {
                " (stepchildren)"
            }
            ".]]\n"
            incr(linecount)
        }
    }
}

/* dofam:
   Write out a person and check for spouses and children.
   Each spouse is written, then this routine is called
   recursively for each child.  An incremented level is passed along
   in case the user specified a limited number of generations
*/

proc dofam (nm,prefix,level,isstep) {
  set(pre,mainpre)
  call printpers(nm,concat(prefix,pre),0,0)  /* print this person */
  if (and(ge(linecount,MAXLINES),gt(nfamilies(nm),0))) {
    prefix "  [[Reached line count max."
    "  May be further descendants here."
    "]]\n"
  } else {
    families(nm, fam, spouse, num) {   /* do for each family */
      if (ne(spouse,null)) {         /* if there is a spouse */
        /* print spouse */
        call printpers(spouse,concat(prefix,spousepre),1,fam)
        if (and(ge(linecount,MAXLINES),gt(nchildren(fam),0))) {
          prefix "  [[Reached line count max."
          "  May be further descendants here."
          "]]\n"
        } else {
          families (spouse, spsfam, ospouse, num2) {
            /* for each of the spouse families*/
            if(eq(fam,spsfam)){  /* only non-step families */
              call startfam (spsfam,prefix,level,0)
            }
          } /*end spouse's families*/
        } /* end spouse not ge MAXLINES */
      } else {                    /* there is no spouse */
        call startfam (fam,prefix,level,0)
      } /*end else no spouse*/
    } /*end 'families'*/
  } /* end MAXLINES else */
} /*end 'proc dofam'*/


/* printpers:
   Write output line for one person.
   Include birth and death dates if known.
   For a spouse, include marriage date if known.
*/
proc printpers (nm, prefix, spouse, fam) {
  prefix
  if(nfamilies(nm)){
    set(hasChildren, 0)
    families(nm, f, s, n){
      if(nchildren(f)){
        set(hasChildren, 1)
      }
    }
  }
  call name_href(nm,1)
  if (e, marriage(fam)) {
    "\t m. " stddate(e)
  }
  "\n"
  incr(linecount)
} /* end proc printpers */
