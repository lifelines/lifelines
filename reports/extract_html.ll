/*
 * @progname       extract_html.ll
 * @version        1.4
 * @author         Scott McGee (smcgee@microware.com)
 * @category       
 * @output         HTML
 * @description

This program allows the user to select a group of individuals from a database
and generate a set of HTML files for them. It allows writing multiple people 
per HTML file, and will create an index file and a GENWEB.txt file for genweb
indexing of the resulting data.

Before running this program, you will want to customize some global values
for your site. In the original release, they are set as follow:

  set(db_owner, getproperty("user.fullname"))
  set(owner_addr, getproperty("user.email"))
  set(use_image, 1)               
  set(genweb_image, "../../pics/genweb.gif")
  set(use_page, 1)                
  set(genweb_page, "../genweb.html")
  set(page_name, "genweb page")   
  set(html_index, 0)

The first two sets will get your fullname and email address from the 
corresponding user properties.  They do not require editing this file.
The other customizations require editing of this file.
It also says to put an image at the top of each HTML file and specifies
that the image is called genweb.gif. Next, it specifies that a link to my
base page be added to each HTML file, that the location of the base page is
genweb.html, and that the text for the link be "genweb page". It also says
not to use and <ISINDEX> tag in the INDEX.html file.

The program, when run, will request a person to start with. It then allows 
selection of additional people by following family links. It then allows 
addition of all ancestors of the selected set or of the first individual, 
and then all descendants of the selected set or of the orignal individual. 
It also allows addition of all persons with a specified number of relations 
to any individual in any of the groups added above.

For each person asked about, you will be given some information on them to
aid in deciding if they are the one you want or not. This is similar to a
person display when browsing with LifeLines. 

Note: This program will assume that you have a directory called genweb in your
output directory (as specified by LL_REPORTS) and will write all output files
in that directory. If the genweb directory does NOT exist (at least, with
LL302) you will be prompted for the name of each output file. Be aware that
if you use this to name the files diffently, the references within the files
will NOT be changed to reflect the new file name!

Future Enhancements (Let me know if you want to do one of these for me!):
  A hierarchical index would be a nice option.
  Need to add descendant and ancestor (pedigree) charts.
  Add seperate page(s) for sources and generate hyperlinks to them.

Thanks to Tom Wetmore for many small routines that have been addapted for
use in this program as well as LifeLines itself.

Scott McGee

@(#)extract_html.ll	1.4 10/1/95

*/

include("extract_set.li")
include("tools.li")

/* customization globals - customize values assigned in main */
global(db_owner)       /* name of database owner */
global(owner_addr)     /* url of database owner (mailto or homepage) */
global(use_image)      /* flag to indicate whether to use genweb image */
global(genweb_image)   /* name of genweb image to place on each page */
global(use_page)       /* flag to add link to genweb page or homepage */
global(genweb_page)    /* URL of base genweb (or homepage) web page */
global(page_name)      /* name of base genweb (or homepage) web page */
global(LDS)            /* display LDS Ordinances? (1=yes 0=no) */
global(html_index)     /* add <ISINDEX> tag to INDEX.html file (1=yes 0=no) */

/* other globals */
global(found)          /* external file to inline image found flag */
global(per_file)       /* number of people per file to write */
global(first)          /* first person shouldn't be asked about */

global(RVAL)           /* ?? (part of borrowed code) */
global(last_surname)   /* last surname in index - used for anchors */
global(first_indi)     /* starting person */

proc main () {

  indiset(out_set)

/* customize these globals to customize the output to your site */
  set(db_owner, getproperty("user.fullname"))
  set(owner_addr, getproperty("user.email"))
  set(use_image, 1)                /* 1 to use image, 0 to not use image */
  set(genweb_image, "../../pics/genweb.gif")
  set(use_page, 1)                 /* 1 to use link to page, 0 if not */
  set(genweb_page, "../genweb.html")
  set(page_name, "genweb page")    /* might change to "my homepage" */
  set(LDS, 1)
  set(html_index, 0)               /* 1 to use <ISINDEX>, 0 if not */

  set(per_file, 1)

  getindi(indi)
  if (indi) {
    set(first_indi, indi)
    set(out, extract_set(indi))
    call html_out(out)
  }
  else {
    print("No one identified -- terminating\n")
  }
}

proc html_out (o) {
  set(s, concat("There are ", d(lengthset(o)),
                " people in your list, how many per file?"))
  getstr(a,s)
  set(per_file, atoi(a))
  if(not(per_file)) {
    set(per_file, 1)
  }
  set(loop_count, 0)
  set(file_count, 0)
  set(href_table, init_href(o))
  forindiset(o, i, j, n) {
    set(indi, i)
    if(eq(loop_count, 0)) {
      incr(file_count)
      call write_head(file_count)
    }
    incr(loop_count)
    call genhtml(indi, o, href_table)
    if(or(eq(loop_count, per_file), eq(n, lengthset(o)))) {
      call write_tail()
      set(loop_count, 0)
    }
  }

  call do_index(o, href_table)

}

proc write_head(count) {
  set(filename, concat("genweb/", database(), "/genweb_", d(count), ".html"))
  print("Writing ", filename, "\n")
  newfile(filename, 0)
  "<HTML><HEAD>\n"
  "<TITLE> genweb_"
  d(count)
  ".html </TITLE>\n" "</HEAD><BODY>\n"
  if(use_image) {
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

proc genhtml (i, o, href_table) {
/*  print("        ", fullname(i,0,1,300), "\n") */
  "<A NAME=\""
  key(i)
  "\"></A>\n"
  "<H1>"
  set(vn,givens(i))
  set(vn1,save(vn))
  givens(i)
  " "
  set(nn,surname(i))
  set(nn1,save(nn))
  nn1
  "</H1>\n"
  set(path, get_picture(i))
  if (found) {
    "<IMG SRC =\""
    if(nestr(lower(trim(path, 4)), "http")) {
      if(nestr(lower(trim(path, 3)), "ftp")) {
        if(nestr(lower(trim(path, 5)), "file:")) {
          if(nestr(lower(trim(path, 6)), "gopher")) {
            "../"
          }
        }
      }
    }
    path
    "\" ALT = \"\"><BR><BR>\n"
  }
  call afn(i)
  if (e, birth(i)) {
    "<EM>Born</EM> : " long(e) "<BR>\n"
  }
  if (e, baptism(i)) {
    "<EM>Baptised</EM> : " long(e) "<BR>\n"
  }
  elsif (e, bapt(i)) {
    "<EM>Baptised</EM> : " long(e) "<BR>\n"
  }
  if (e, death(i)) {
    "<EM>Died</EM> : " long(e) "<BR>\n"
  }
  if (e, burial(i)) {
    "<EM>Buried</EM> : " long(e) "<BR>  \n"
  }
  if(LDS) {
    /* LDS ordinances */
    set(started, 0)
    fornodes(inode(i), node) {
      if (eq(0, strcmp(tag(node), "BAPL"))) {
        if(not(started)) {
          set(started, 1)
          "<BR>LDS Ordinances: B "
        }
      }
      /* determine if endowed */
      if (eq(0, strcmp(tag(node), "ENDL"))) {
        if(not(started)) {
          set(started, 1)
          "<BR>LDS Ordinances: "
        }
        "E "
      }
    }
    /* determine if sealed to parents */
    set(fam, fnode(parents(i)))
    set(ind, inode(i))
    if(fam) {
      fornodes(fam, node) {
        if(and(eqstr("CHIL", tag(node)), eqstr(xref(ind), value(node)))) {
          fornodes(node, next) {
            if(eqstr(tag(next), "SLGC")) {
              if(not(started)) {
                set(started, 1)
                "<BR>LDS Ordinances: "
              }
              "SC "
            }
          }
        }
      }
    }
    if(started){
      "<BR>\n"
    }
  }
/*  "<BR>" */
  call othernames(i)
  call print_html(i)
  "<HR>\n"
  if (p, father(i)) {
    "<EM>" "Father</EM> : "
    set (path, get_href(p, href_table))
    if(found) {
      "<A HREF=\""
      path
      "#"
      key(p)
      "\">"
    }
    if (t,title(p)) {t " "}
    fullname(p,0,1,300)
    if(found) {"</A>"}
    do_info(p)
    "<BR>\n"
  }
  if (p, mother(i)) {
    "<EM>" "Mother</EM> : "
    set (path, get_href(p, href_table))
    if(found) {
      "<A HREF=\""
      path
      "#"
      key(p)
      "\">"
    }
    if (t,title(p)) {t " "}
    fullname(p,0,1,300)
    if(found) {"</A>"}
    do_info(p)
    "<BR>\n"
  }
  families(i, f, s, n) {
    "<P><EM>" "Spouse"
    if (gt(nfamilies(i), 1)) {
      " "
      d(n)
    }
    "</EM> : \n"
    if (s) {    /* family has a spouse */
      set (path, get_href(s, href_table))
      if(found) {
        "<A HREF=\""
        path
        "#"
        key(s)
        "\">"
      }
      if (t,title(s)) {t " "}
      fullname(s,0,1,300)
      if(found) {"</A>"}
      do_info(s)
      "<BR>\n"
    }
    if (e, marriage(f)) {
      "<EM>Married</EM> "
      long(e)
      "<BR>\n"
    }
    if (e, divorced(f)) {
      "<EM>Divorced</EM> "
      long(e)
      "<BR>\n"
    }
    if(LDS) {
      fornodes(fnode(f), node) {
        if (eq(0, strcmp(tag(node), "SLGS"))) {
          "<BR>LDS Ordinances: SS\n"
        }
      }
    }
    "<OL>\n"
    children(f, c, nn) {
      "<LI>"
      set (path, get_href(c, href_table))
      if(found) {
        "<A HREF=\""
        path
        "#"
        key(c)
        "\">"
      }
      if (t,title(c)) {t " "}
      fullname(c,0,1,300)
      if(found) {"</A>"}
      do_info(c)
      "</LI>\n"
    }
    "</OL>\n"
  }
  call print_notes(i)
  "<HR>\n"

/*  Insert code here for Pedigree and Descendant charts
  if(parents(i)) {
    "Pedigree Chart<BR>\n"
  }
  if(nfamilies(i)) {
    set(hasChildren, 0)
    families(i, f, s, n) {
      if(nchildren(f)) {
        set(hasChildren, 1)
      }
    }
    if(hasChildren) {
      "Descendant Chart\n"
    }
  }
  "<HR>\n"
*/
  "<BR>\n"
  "[<A HREF=\"INDEX.html\">"
  "Index to database</A>]<BR>\n"
  if(use_page) {
    "[<A HREF=\"" genweb_page "\">"
    "Return to "
    page_name
    " </A>]<BR> \n"
  }
  "<BR><HR><BR>\n"
}

func init_href(outset){
  table(href_table)
 
  forindiset(outset, indi, j, number) {
    insert(href_table, save(key(indi)), number)
  }
  return(href_table)
}

func get_href(indi, href_table) {
  set(path, "")
  set(found, 0)
  set(value, lookup(href_table, key(indi)))
  if(value){
    set(number, add(div(sub(value, 1), per_file), 1))
    set(path, concat("genweb_", d(number), ".html"))
    set(found, 1)
  }
  return(path)
}

proc print_notes(indi){
  set(first, 1)
  traverse(inode(indi), node, l) {
    if (not(strcmp("NOTE", tag(node)))) {
      if(first) {
        "<EM>Notes</EM> : <BR>\n"
        set(first, 0)
      }
      "<P>"
      call show_path(node)
      value(node)
      "\n"
      fornodes(node, next) {
        value(next)
        "\n"
      }
      "</P>\n"
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
    if(eqstr(lower(s), "indi")){
      "Individual "
    }elsif(eqstr(lower(s), "fam")){
      "Family "
    }elsif(eqstr(lower(s), "famc")){
      "family "
    }elsif(eqstr(lower(s), "fams")){
      "family "
    }elsif(eqstr(lower(s), "note")){
      "note"
    }elsif(eqstr(lower(s), "birt")){
      "birth "
    }elsif(eqstr(lower(s), "deat")){
      "death "
    }elsif(eqstr(lower(s), "buri")){
      "burial "
    }elsif(eqstr(lower(s), "plac")){
      "place "
    }else{
      lower(s)
      " "
    }
  }
  ")\n"
}

proc do_index(indi_set, href_table) {
  set(last_surname, "ZZ")
  list(RVAL)
  indiset(index)

  set(index, indi_set)
  namesort(index)
  print("Writing INDEX.html\n")
  call create_index_file(index, href_table)
  print("Writing GENDEX.txt\n")
  call create_gendex_file(index, href_table)
}

proc create_gendex_file(index, href_table) {
  set(fn, save(concat("genweb/", database(), "/GENDEX.txt")))
  newfile(fn, 0)
  forindiset(index, me, v, n)
  {
    set(path, concat(save(get_href(me, href_table)), "#", key(me)))
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

proc create_index_file(index, href_table) {
  set(fn, save(concat("genweb/", database(), "/INDEX.html")))
  newfile(fn, 0)
  call html_header("Interactive Genealogical Server Index", html_index)
  "<BODY>\n"
  if(use_image) {
    "<IMG SRC=\""
    genweb_image 
    "\" ALT = \"\"><BR><BR>\n"
  }
  "<H1> INDEX </H1>\n"
  "<UL>\n"
  forindiset(index, me, v, n)
  {
    call href(me, href_table)
    "\n"
  }
  "</UL>\n"
  call write_tail()
}

proc href(me, href_table) {
  if(me) {
    call print_name(me, 1)
    if(ne(strcmp(upper(surname(me)), last_surname), 0)) {
       print("        ", upper(surname(me)), "\n")
       set(last_surname, save(upper(surname(me))))
       "<A NAME=" qt() last_surname qt() "></A>\n"
    }
    "<LI>"
    set (path, get_href(me, href_table))
    if(found) {
      "<A HREF=\""
      path
      "#"
      key(me)
      "\">\n"
    }
    pop(RVAL)
    if(found) {
      "</A>"
    }
    do_info(me)
  }
}

proc html_header(str, isindex) {
  "<HTML>\n"
  "<HEAD>\n"
  if(isindex) {
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
  if(indi){
    set(count, 0)
    fornodes(inode(indi), subnode){
      if(eqstr(tag(subnode), "NAME")){
        incr(count)
        if(eq(count, 2)){
          "<BR><EM>Other Names</EM>: \n<UL>"
          "<LI>"
          call nameval(subnode)
          "</LI>"
        }elsif(gt(count, 2)){
          "<LI>"
          call nameval(subnode)
          "</LI>\n"
        }
      }
    }
    if(gt(count, 1)){
      "</UL>\n"
    }
  }
}

proc afn(indi){
  if(indi){
    fornodes(inode(indi), subnode){
      if(eqstr(tag(subnode), "AFN")){
        "AFN "
        value(subnode)
        "<BR><BR>\n"
      }
    }
  }
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
    if (not(strcmp("REPORT", tag(node)))) {
      set(m, child(node))
      if (not(strcmp("TYPE", tag(m)))) {
        if (not(strcmp("HTML", value(m)))) {
          "<BR>\n"
          fornodes(m, o) {
            if (not(strcmp("DATA", tag(o)))) {
              value(o)
              "\n"
            }
            "\n"
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
              "\n"
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
