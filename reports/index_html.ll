/*
 * @progname       index_html.ll
 * @version        1.3
 * @author         Scott McGee (smcgee@microware.com)
 * @category       
 * @output         HTML
 * @description

This report program converts a LifeLines database into html index document.
You will need to change the contents of proc html_address() and to
set the value of HREF appropriately to your server.
You need to set the value of PATH to point to the directory to put
the file into.
You also need to set the value of HOST to be the http server and
path where you will server these files from.

@(#)index_html.ll	1.3 10/14/95
by Scott McGee (smcgee@microware.com)

*/

global(INDEX)
global(HREF)
global(PATH)
global(RVAL)
global(FB)
global(nl)
global(last_surname)
global(name_count)
global(surname_count)
global(owner_email)
global(db_owner)

proc main()
{
     set(db_owner, getproperty("user.fullname"))
     set(owner_email, getproperty("user.email"))
    set(FB, 0)
    set(nl, nl())
    set(last_surname, "ZZ")
    list(RVAL)
    indiset(INDEX)
    set(PATH, "/users/smcgee/www/")
    set(HOST, "http://www.emcee.com")
    set(HREF, concat(
                "http://www.emcee.com/~smcgee/cgi-bin/genweb_cgi/DB=",
                database(),
                "/INDEX="))
    print("processing database\n")
    set(count, 0)
    forindi(me,num)
    {
      if(eq(count, 100)){
        set(count, 0)
        print(".")
      }else{
        incr(count)
      }
      addtoset(INDEX,me,1)
    }
    print("\nwriting file\n")
    set(name_count, 0)
    set(surname_count, 0)
    call create_index_file()
    print("\n", d(name_count), " individuals, ", d(surname_count), " surnames\n")
}

proc create_index_file()
{
    namesort(INDEX)
    set(fn, save(concat(PATH, concat("genweb/", database(), "_idx.html"))))
    newfile(fn, FB)
    call html_header(0)
    "<BODY>" nl
    "<IMG SRC=\"http://www.emcee.com/~smcgee/pics/genweb.gif\" ALT = \"\"><BR><BR>\n"
    "<H1> INDEX </H1>" nl
    "<UL>" nl
    forindiset(INDEX, me, v, n)
    {
        call href(me) nl
    }
    "</UL>" nl
    call html_address()
    "</BODY>" nl
    "</HTML>" nl
}

proc href(me)
{
    if(me)
    {
        call print_name(me, 1)
        incr(name_count)
        if(ne(strcmp(surname(me), last_surname), 0)){
           incr(surname_count)
           print(surname(me))
           print("\n")
           set(last_surname, save(surname(me)))
           "<A NAME=" qt() last_surname qt() "></A>\n"
        }
        "<LI><A HREF=" qt() HREF key(me) "?LookupInternal" qt() ">\n"
        pop(RVAL)
        "</A> -"
        if (evt, birth(me)) {
          " born "
          short(evt)
        }
        else {
            if (evt, baptism(me)) {
                " baptised "
                short(evt)
            }
            else {
                if (evt, bapt(me)) {
                    " baptised "
                    short(evt)
                }
            }
        }
        if (evt, death(me)) {
          " died "
          short(evt)
        }
    }
}
proc html_header(isindex)
{
    "<HTML>" nl
    "<HEAD>" nl
    "<LINK rev=\"made\" HREF=\"mailto:" owner_email "\">"
    if(isindex) { "<ISINDEX>" nl }
    "<TITLE>Index of database - "
    database()
    "</TITLE>" nl
    "</HEAD>" nl
 }

proc html_address()
{
    "<HR>" nl
    "<ADDRESS>Last update : "
    date(gettoday())
    "<br>" db_owner "  //  " owner_email " </ADDRESS>" nl
}

proc print_name (me, last)
{
    call get_title(me)
    set(junk, pop(RVAL))
    push(RVAL, save(concat(fullname(me, 0, not(last), 45), junk)))
}

proc get_title (me)
{
    fornodes(inode(me), node)
    {
        if (not(strcmp("TITL", tag(node)))) { set(n, node) }
    }
    if (n) { push(RVAL, save(concat(" ", value(n)))) }
        else { push(RVAL, "") }
}

proc rjt(n, w)
{
    if (lt(n, 10)) { set(d, 1) }
    elsif (lt(n, 100)) { set(d, 2) }
    elsif (lt(n, 1000)) { set(d, 3) }
    elsif (lt(n, 10000)) { set(d, 4) }
    else  { set(d, 5) }
    if (lt(d, w))
        { set(pad, save( trim("      ", sub(w, d)))) }
    else
        { set(pad, "") }
    push(RVAL, save( concat(pad, save(d(n)))))
}
func bapt (indi) {
  fornodes(inode(indi), node) {
    if (eq(0, strcmp(tag(node), "BAPL"))) {
      return(node)
    }
    if (eq(0, strcmp(tag(node), "BAPM"))) {
      return(node)
    }
  }
  return(0)
}
