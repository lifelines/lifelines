/*
 * @progname       name2html.ll
 * @version        1.5
 * @author         Scott McGee
 * @category       
 * @output         HTML
 * @description

Converts the selected indi record to an HTML file.

This program is based primarily on my version of indi2html with additions
(based on suggestions by Tom Westmore) to handle name lookup and possible
multiple matches.

@(#)name2html.ll	1.5 10/6/95
*/

include("cgi_html.li")
include("tools.li")

global(found)  /* external file to inline found */
global(path)   /* path to external file to inline */
global(LDS)    /* report LDS ordinaces (1=yes 0=no) */

proc main (){
  call set_cgi_html_globals()
  set(is_indi_html, 1)

  set(LDS, 1)
  getindiset(iset, "What name to you want an HTML file for?")
  if (eq(1, lengthset(iset))) {
	forindiset(iset, i, v, n) {
		set(indi, i)
	}
	call genhtml(indi)
  } elsif (ne(0, lengthset(iset))) {
    call list_to_html(iset)
  }
  else {
    "<HTML><HEAD>\n"
    "<TITLE>No Match</TITLE>\n"
    "</HEAD><BODY>\n"
    if(use_image){
      "<IMG SRC=\""
      genweb_image
      "\" ALT = \"\"><BR><BR>\n"
    }
    "<H1>No Match</H1>\n"
    "Sorry, no match was found for the requested name!\n"
    "</BODY></HTML>\n"
  }
}

proc genhtml (i){
  html_head(i)

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
    "<EM>Buried</EM> : " long(e) "<BR>	\n"
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
    set(fam, parents(i))
    if(fam){
      set(val, concat("@", key(i), "@"))
      fornodes(fnode(fam), node) {
        if (eq(0, strcmp(tag(node), "CHIL"))) {
          if (eq(0, strcmp(value(node), val))) {
            fornodes(node, next) {
              if (eq(0, strcmp(tag(next), "SLGC"))) {
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
    }
    if(started){
      "<BR>\n"
    }
  }
  call othernames(i)
  call print_html(i)
  set(hasChildren, 0)
  if(nfamilies(i)){
    families(i, f, s, n){
      if(nchildren(f)){
        set(hasChildren, 1)
      }
    }
  }
  "<HR>\n"
  if (p, father(i)) {
    "<EM>" "Father</EM> : \n"
    href(p, "Lookup")
    do_info(p)
    "<BR>\n"
  }
  if (p, mother(i)) {
    "<EM>" "Mother</EM> : "
    href(p, "Lookup")
    do_info(p)
    "<BR>\n"
  }
  families(i, f, s, n) {
    "<P><EM>" "Spouse"
    if (gt(nfamilies(i), 1)){
      " "
      d(n) 
    }
    "</EM> : \n"
    if (s) {	/* family has a spouse */
      href(s, "Lookup")
      do_info(s)
      "<BR>\n"
    }else{
      "(unknown)<BR>"
    }
    if (e, marriage(f)) {"<EM>Married</EM> " long(e) "<BR>\n"}
    if (e, divorced(f)) {"<EM>Divorced</EM> " long(e) "<BR>\n"}
    if(LDS) {
      /* determine if sealed to parents */
      fornodes(fnode(f), node) {
        if (eq(0, strcmp(tag(node), "SLGS"))) {
          "LDS Ordinances: SS <BR>"
        }
      }
    }
    if(nchildren(f)){
    "<EM>Children</EM> :<OL>\n"
    children(f, c, nn) {
      "<LI>"
      href(c, "Lookup")
      do_info(c)
    }
    "</OL>\n"
    }else{
      "(no children)<BR>\n"
    }
  }
  call print_notes(i)
  "<HR>\n"
  if(parents(i)){
    "[<A HREF=\""
    cgi_script
    "/DB="
    database()
    "/INDEX=" 
    key(i) 
    "/?PedigreeInternal\">" 
    "Pedigree Chart</A>]<BR>\n"
  }
  if(hasChildren){
    "[<A HREF=\""
    cgi_script
    "/DB="
    database()
    "/INDEX=" 
    key(i) 
    "/?DescendantInternal\">" 
    "Descendant Chart</A>]<BR>\n"
  }
  call do_tail(i)
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
    if(not(strcmp(lower(s), "indi"))){
      "Individual "
    }elsif(not(strcmp(lower(s), "fam"))){
      "Family "
    }elsif(not(strcmp(lower(s), "famc"))){
      "family "
    }elsif(not(strcmp(lower(s), "fams"))){
      "family "
    }elsif(not(strcmp(lower(s), "note"))){
      "note"
    }elsif(not(strcmp(lower(s), "birt"))){
      "birth "
    }elsif(not(strcmp(lower(s), "deat"))){
      "death "
    }elsif(not(strcmp(lower(s), "buri"))){
      "burial "
    }elsif(not(strcmp(lower(s), "plac"))){
      "place "
    }else{
      lower(s)
      " "
    }
  }
  ") "
}

proc list_to_html (iset) {
  "<HTML><HEAD>\n"
  "<TITLE>Multiple Matches</TITLE>\n"
  "</HEAD><BODY>\n"
  "<H1>Multiple Matches</H1>\n"
  "More than one person in the database matched the name search. They are:\n"
  "<BR><HR>\n"
  forindiset(iset, i, v, n) {
    href(i, "Lookup")
    do_info(i)
    "<BR>\n"
  }
  call do_tail(0)
}

func do_info(me){
  if(not(me)){
    return("")
  }else{
    set(out, " -")
    if (evt, birth(me)) {
      set(out, concat(out, " born ", short(evt)))
    }
    else {
      if (evt, baptism(me)) {
        set(out, concat(out, " baptised ", short(evt)))
      }
      else {
        if (evt, bapt(me)) {
          set(out, concat(out, " baptised ", short(evt)))
        }
      }
    }
    if (evt, death(me)) {
      set(out, concat(out, " died ", short(evt)))
    }
    return(out)
  }
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

proc nameval(namenode){
  list(np)
  extractnames(namenode, np, nc, sc)
  forlist(np, v, i){
    v
    " "
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

func divorced(fam) {
  fornodes(fnode(fam), node) {
    if (eq(0, strcmp(tag(node), "DIV"))) {
      return(node)
    }
  }
  return(0)
}

proc print_html(indi){
  fornodes(inode(indi), node) {
    if (not(strcmp("REPORT", tag(node)))) {
      set(m, child(node))
      if (not(strcmp("TYPE", tag(m)))) {
        if (or(not(strcmp("HTML", value(m))), 
              not(strcmp("HTML-CGI", value(m))))) {
          "<BR>\n"
          fornodes(m, o) {
            if (not(strcmp("DATA", tag(o)))) {
              value(o)
              "\n"
            }
          }
        }
      }
    }
  }
}


func html_head(i){
  "<HTML><HEAD>\n"
  "<TITLE>" key(i) ": " name(i,0) "\n</TITLE>\n" "</HEAD><BODY>\n"
  if(use_image){
    "<IMG SRC=\""
    genweb_image
    "\" ALT = \"\"><BR><BR>\n"
  }
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
            localhost
          }
        }
      }
    }
    path
    "\" ALT = \"\"><BR><BR>\n"
  }
}
