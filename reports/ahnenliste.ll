/*
 * @progname    ahnenliste.ll
 * @version     6
 * @author      Jim Eggert
 * @category
 * @output      Text
 * @description
 *
 * Generate an Ahnenliste, an ancestral report for an individual
 *
ahnenliste - a LifeLines report program to aid in the generation of
an Ahnenliste (German ancestral report).

Given a person, this generates an Ahnenliste for that person and
his/her ancestors.

BEFORE YOU RUN THE PROGRAM:

Change the routine write_header() to use your submitter tag, name,
and address.

Version 1, 14 July 1994, by Jim Eggert, EggertJ@verizon.net
Version 2, 18 Aug  1998, added HTML
Version 3, 17 Feb  1999, added surnames to location list
Version 4, 15 Jan  2000, fixed quicksort bug
Version 5, 26 Jan  2000, added sorting translation
Version 6, 22 Jun  2000, improved handling of intersecting lines

*/

global(locationsurname_list)
global(locationsurname_table)
global(i_list)
global(i_table)
global(a_list)
global(g_list)
global(ahn_table)

global(html)
global(sep)
global(par)
global(br)
global(bold)
global(unbold)
global(gt)
global(lt)
global(born)
global(bapt)
global(died)
global(burd)
global(marr)

global(sort_xlat)
global(html_xlat)
global(ISO8859_xlat)

/* write_header writes a little header */

proc write_header(person) {
  sep
  if (html) { "<H1>" }
  "Ahnenliste " autohtml(mysurname(person)) "\n"
  if (html) { "</H1>" }
  sep "\n"
  bold "Proband:" unbold " " autohtml(fullname(person,0,1,80)) br
  bold "Autor:" unbold "   James R. Eggert" par
  bold "Inhalt:" unbold br
  if (html) { "   Erl&auml;uterungen," }
  else { "   Erl\"auterungen," }
  br "   Landschaften," br
  "   Orte," br
  "   Namen" par "\n"
  if (html) { "Erg&auml;nzungen" } else { "Erg\"anzungen" }
  ", Berichtigungen, Anfragen oder Kommentare werden als\n"
  "eMail erbeten an:" br
  if (html) {
    "<A HREF=\"MAILTO:EggertJ@verizon.net\">Jim Eggert</A>"
    "(EggertJ@verizon.net)"
  } else {
    "Jim Eggert (EggertJ@verizon.net)"
  }
  par
  dayformat(1)
  monthformat(4)
  dateformat(0)
  bold "Stand:" unbold " " stddate(gettoday()) par

  if (html) { call section("Erl&auml;uterungen") }
  else { call section("Erl\"auterungen") }

  "Die genealogischen Zeichen wurden durch folgende Satzzeichen ersetzt:" br

  "   " born " -- geboren" br
  "   " bapt " -- getauft" br
  "   " died " -- gestorben" br
  "   " burd " -- begraben" br
  "   " marr " -- verheiratet" br br

  "Die zweiteilige Nummer vor jeder Zahl setzt sich zusammen auf die\n"
  "Generation (bezogen auf den Probanden) und der Ordnungszahl im\n"
  "Kekule'schen System.  Nach diesem System ist die Ahnenzahl des Vaters\n"
  "einer Person immer doppelt so gross wie deren Zahl, die der Mutter\n"
  if (html) {
    "um einen Wert h&ouml;her als die des Vaters. Daraus ergibt sich,
da&szlig;\n"
    "(mit Ausnahme eines m&auml;nnlichen Probanden) gerade Ordnungszahlen\n"
    "immer zu M&auml;nnern, ungerade immer zu Frauen geh&ouml;ren." par
  } else {
    "um einen Wert h\"oher als die des Vaters. Daraus ergibt sich, dass\n"
    "(mit Ausnahme eines m\"annlichen Probanden) gerade Ordnungszahlen\n"
    "immer zu M\"annern, ungerade immer zu Frauen geh\"oren." par
  }
  "---" gt " Bezugsperson = n, Vater = 2n, Mutter = 2n+1" par

  call section("Landschaften")

  "deutsche Gebiete:  Hannover (Amt Dannenberg), Schaumburg-Lippe,\n"
  "  Provinz Posen, Westpreussen, Pfalz" br
  "US-amerikanische Bundesstaate:  Illinois, Kansas, Massachusetts,\n"
  "  Minnesota, Nebraska, New Jersey" br
  "Syrien" par
}


proc main() {
  table(sort_xlat)
  table(html_xlat)
  table(ISO8859_xlat)
  call init_xlat()

  getintmsg(html,"Enter 0 for text, 1 for HTML output:")
  set(born,"*")
  set(bapt,"=")
  set(died,"+")
  set(marr,"oo")
  if (html) {
    set(sep,"<HR>")
    set(par,"<P>\n")
    set(br,"<BR>\n")
    set(bold,"<B>")
    set(unbold,"</B>")
    set(gt,"&gt;")
    set(lt,"&lt;")
    set(burd,"&plusmn;")
  } else {
    set(sep,
    "_________________________________________________________________\n")
    set(par,"\n")
    set(br,"\n")
    set(bold,"")
    set(unbold,"")
    set(gt,">")
    set(lt,"<")
    set(burd,"±")
  }

  getindi(person)

  call write_header(person)

  table(locationsurname_table)
  list(i_list)       /* holds all root ancestors, just once */
  table(i_table)     /* lookup mechanism for i_list */
  list(a_list)       /* ahnentafel numbers for i_list */
  list(g_list)       /* generation numbers for i_list */
  list(s_list)       /* child one down in ancestry */
  list(work_i_list)
  list(work_a_list)
  list(work_g_list)
  list(work_c_list)
  list(locationsurname_list)
  table(ahn_table)  /* holds all ancestors once, with ahnentafel numbers */

  enqueue(work_i_list, person)
  enqueue(work_a_list, 1)
  enqueue(work_g_list, 1)
  enqueue(work_c_list, 0)

/* Traverse ancestry twice, first pass to collect places, surnames,
   and keys, second pass to produce ancestral lines.
 */
  set(curgen,0)
  set(done,0)
  while(person,dequeue(work_i_list)) {
    set(ahnen,dequeue(work_a_list))
    set(gen,dequeue(work_g_list))
    set(child,dequeue(work_c_list))

    if (not(lookup(ahn_table,key(person)))) { /* only do a person once */
      insert(ahn_table,key(person),ahnen)
      call locations(person)
/* test for inclusion of this individual as a root */
      set(include,0)
      if (child) {
        if (strcmp(soundex(person),soundex(child))) {
          set(include,1)
        } elsif (and(female(person),father(child))) {
            set(include,1)
        }
      } else { set(include,1) }
      if (include) {
        enqueue(i_list,person)
        insert(i_table,save(key(person)),ahnen)
        enqueue(a_list,ahnen)
        enqueue(g_list,gen)
        enqueue(s_list,save(mysurname(person)))
      }

/* iterate into working lists */
      incr(gen)
      set(ahnen,mul(ahnen,2))
      if (f,father(person)) {
        enqueue(work_i_list,f)
        enqueue(work_a_list,ahnen)
        enqueue(work_g_list,gen)
        enqueue(work_c_list,person)
      }
      if (m,mother(person)) {
        enqueue(work_i_list,m)
        enqueue(work_a_list,add(ahnen,1))
        enqueue(work_g_list,gen)
        enqueue(work_c_list,person)
      }
    }
  }

  call section("Orte")
  list(index_list)
  list(trans_locsur_list)
  call translate(locationsurname_list,trans_locsur_list)
  call quicksort(trans_locsur_list,index_list)
  set(prevplace,"zzznowhere")
  set(prevsurname,"zzznoone")
  set(prevfirstplace,"zzznothere")
  set(yearfrom,9999)
  set(yearto,0)
  forlist(index_list,index,i) {
         set(locationsurname,getel(locationsurname_list,index))
         list(ls)
         extracttokens(locationsurname,ls,nls,":")
         set(location,getel(ls,1))
         set(surname,getel(ls,2))
         set(years,lookup(locationsurname_table,locationsurname))
    if (strcmp(location,prevplace)) {
      if (strcmp(prevplace,"zzznowhere")) {
        if (yearto) {
          " ("
          if (ne(yearfrom,yearto)) {
            d(yearfrom) "-"
          }
          d(yearto) ")"
        }
        br
      }
      set(yearfrom,getel(years,1))
      set(yearto,getel(years,2))
      list(placenamelist)
      extracttokens(location,placenamelist,nplaces,",")
      set(name,getel(placenamelist,1))
      if (not(strcmp(name,prevfirstplace))) {
        print("Warning: ambiguous placename initial element: ",
                  name,"\n")
      }
      set(prevfirstplace,save(name))
      forlist(placenamelist,placename,np) {
        autohtml(placename)
        if (lt(np,nplaces)) {
          if (eq(np,1)) { " - " } else { ", " }
        }
      } ":  " autohtml(surname)
      set(prevplace,save(location))
      set(prevsurname,save(surname))
    } else {
      if (strcmp(surname,prevsurname)) {
        ", " autohtml(surname)
        set(prevsurname,save(surname))
      }
      if (thisyearfrom,getel(years,1)) {
        if (or(lt(thisyearfrom,yearfrom),eq(yearfrom,0))) {
          set(yearfrom,thisyearfrom)
        }
      }
      if (gt(getel(years,2),yearto)) {
        set(yearto,getel(years,2))
      }
    }
  }
  if (yearto) {
    " ("
    if (ne(yearfrom,yearto)) {
      d(yearfrom) "-"
    }
    d(yearto) ")"
  }
  br

  call section("Namen")
  list(index_list)
  list(trans_s_list)
  call translate(s_list,trans_s_list)
  call quicksort(trans_s_list,index_list)
  set(ni,length(index_list))
  set(prevname,"zzxxyyzz")
  set(comma,0)
  forlist(index_list,index,i) {
    set(name,getel(s_list,index))
    if (strcmp(name,prevname)) {
      if (comma) { ", " } autohtml(name)
      set(comma,1)
      set(prevname,save(name))
    }
  }
  par

/* Second traversal of ancestry, in surname order, but proband first. */

  call doline(1)

  forlist(index_list,index,i) {
    if (ne(index,1)) {
      call doline(index)
    }
  }
}

proc section(header) {
  sep
  if (html) { "<H2>" } else { "\n" }
  header
  if (html) { "</H2>" }
  par par
}

proc doline(person_index) {
  set(person,getel(i_list,person_index))
  sep
  if (html) { "<H3>" } else { "\n" }
/* First pass to print out appropriate surnames */
  table(prev_surname_table)
  autohtml(mysurname(person))
  insert(prev_surname_table,save(mysurname(person)),1)
  while (person,father(person)) {
    if (lookup(i_table,key(person))) {
      set(person,0)
    } else {
      set(s,save(mysurname(person)))
      if (not(lookup(prev_surname_table,s))) {
        ", " autohtml(s)
        insert(prev_surname_table,s,1)
      }
    }
  }
  if (html) { "</H3>" }
  "\n\n"

/* Second pass to print out detailed information */
  set(person,getel(i_list,person_index))
  set(gen,getel(g_list,person_index))
  set(ahn,getel(a_list,person_index))
  call doperson(person,gen,ahn)
  while (person,father(person)) {
    incr(gen)
    set(ahn,add(ahn,ahn))
    set(prev_ahn,lookup(i_table,key(person)))    /* stop if person is a key... */
    if (not(prev_ahn)) {
      set(prev_ahn,lookup(ahn_table,key(person)))
      if (eq(prev_ahn,ahn)) { set(prev_ahn,0) }  /* or if we did them already */
    }
    if (prev_ahn) {
      bold if (lt(gen,10)) { "0" } d(gen) " " d(ahn) unbold  " "
      autohtml(fullname(person,0,1,80)) " siehe "
      set(gen2,ahn2gen(prev_ahn))
      if (lt(gen2,10)) { "0" } d(gen2) " " d(prev_ahn) "." br
      set(person,0)
    } else {
      call doperson(person,gen,ahn)
    }
  }
}

/* ahn2gen converts ahnentafel number to generation number */
func ahn2gen(ahn) {
  set(gen,1)
  while (gt(ahn,1)) {
    incr(gen)
    set(ahn,div(ahn,2))
  }
  return(gen)
}

proc doperson(person,gen,ahn) {
  bold if (lt(gen,10)) { "0" } d(gen) " " d(ahn) unbold " "
  autohtml(mygivens(person)) set(comma,0)

  if (b,birth(person)) {
    " " born call doevent(b)
    set(comma,1)
  }
  if (b,baptism(person)) {
    if (comma) { "," }
    " " bapt call doevent(b)
    set(comma,1)
  }
  set(nfam,nfamilies(person))
  families(person,fam,spouse,fnum) {
    set(m,marriage(fam))
    if (or(m,spouse,gt(nfamilies,1))) {
      if (comma) { "," }
      " " marr if (gt(nfamilies,1)) { d(fnum) }
      if (m) { call doevent(m) }
      if (spouse) {
        " " autohtml(mygivens(spouse))
        " " bold autohtml(mysurname(spouse)) unbold
      }
      set(comma,1)
    }
  }
  if (b,death(person)) {
    if (comma) { "," }
    " " died call doevent(b)
    set(comma,1)
  }
  if (b,burial(person)) {
    if (comma) { "," }
    " " burd call doevent(b)
    set(comma,1)
  }
  "." br
}

func mygivens(person) {
  set(g,givens(person))
  if (strlen(g)) { return(g) }
  return("____")
}

func mysurname(person) {
  set(s,surname(person))
  if (strcmp(s,"<unknown>")) { return(s) }
  return ("____")
}

proc doevent(event) {
  list(placelist)
  extractplaces(event,placelist,nplaces)
  if (nplaces) {
    set(place,dequeue(placelist))
    if (strlen(place)) { " " } autohtml(place)
  }
  set(d,date(event))
  if (strlen(d)) { " " }
  set(lopoff,4)
  if    (eq (index (d, "AFT", 1), 1)) { gt }
  elsif (eq (index (d, "Aft", 1), 1)) { gt }
  elsif (eq (index (d, "BEF", 1), 1)) { lt }
  elsif (eq (index (d, "Bef", 1), 1)) { lt }
  elsif (eq (index (d, "ABT", 1), 1)) { "c" }
  elsif (eq (index (d, "Abt", 1), 1)) { "c" }
  else { set(lopoff,1) }
  set(d,substring(d,lopoff,strlen(d)))
  while (eq (index (d, " ", 1), 1)) {
    set(d,substring(d,2,strlen(d)))
  }
  if    (m, index (d, "JAN", 1)) { "" }
  elsif (m, index (d, "FEB", 1)) { "" }
  elsif (m, index (d, "MAR", 1)) { "" }
  elsif (m, index (d, "APR", 1)) { "" }
  elsif (m, index (d, "MAY", 1)) { "" }
  elsif (m, index (d, "JUN", 1)) { "" }
  elsif (m, index (d, "JUL", 1)) { "" }
  elsif (m, index (d, "AUG", 1)) { "" }
  elsif (m, index (d, "SEP", 1)) { "" }
  elsif (m, index (d, "OCT", 1)) { "" }
  elsif (m, index (d, "NOV", 1)) { "" }
  elsif (m, index (d, "DEC", 1)) { "" }
  if (gt(m,1)) {
    trim(d,sub(m,1))
  }
  if (m) { capitalize(lower(substring(d,m,strlen(d)))) }
  else { d }
}

proc locations(person) {
  call one_location(burial(person),mysurname(person),death(person))
  call one_location(death(person),mysurname(person),burial(person))
  if (female(person)) {
    families(person,family,husband,fnum) { set(lasthusband,husband) } /* find last husband */
    call one_location(burial(person),mysurname(lasthusband),death(person))
    call one_location(death(person),mysurname(lasthusband),burial(person))
  }
  families(person,family,spouse,fnum) {
    call one_location(marriage(family),mysurname(person),0)
    call one_location(marriage(family),mysurname(spouse),0)
  }
  call one_location(baptism(person),mysurname(person),birth(person))
  call one_location(birth(person),mysurname(person),baptism(person))
}

proc one_location(event,surname,event2) {
  if (event) {
    set(loc,place(event))
    set(yr,atoi(year(event)))
    if (not(yr)) { set(yr,atoi(year(event2))) }
    if (not(yr)) { set(yr,0) }
    if (strlen(loc)) {
      set(loc,locfilter(loc))
      set(locsur,concat(loc,":",surname))
      if (not(lookup(locationsurname_table,locsur))) {
        list(locsuryears)
        setel(locsuryears,1,yr)
        setel(locsuryears,2,yr)
        insert(locationsurname_table,save(locsur),locsuryears)
        enqueue(locationsurname_list,save(locsur))
      } elsif (yr) {
        set(locsuryears,lookup(locationsurname_table,locsur))
        if (lt(yr,getel(locsuryears,1))) {
          setel(locsuryears,1,yr)
        } elsif (gt(yr,getel(locsuryears,2))) {
          setel(locsuryears,2,yr)
        }
        if (eq(getel(locsuryears,1),0)) {
          setel(locsuryears,1,yr)
        }
      }
    }
  }
}

/* remove unneeded location info from location name */
func locfilter(string) {
  set(string,strfilterstart(string,"near "))
  set(string,strfilter(string,"?"))
  return(string)
}

/* remove a string at the start of another string, if present */
func strfilterstart(string,start) {
  if (strcmp(substring(string,1,strlen(start)),start)) {
    return(string)
  }
  return(substring(string,add(strlen(start),1),strlen(string)))
}

/* remove a string from another string, multiple times if needed */
func strfilter(string,sub) {
  while (m,index(string,sub,1)) {
    set(string,concat(substring(string,1,sub(m,1)),
                      substring(string,add(m,strlen(sub)),strlen(string))))
  }
  return(string)
}

/* translate a string but only if html global is set */
func autohtml(string) {
  if (html) { return(strxlat(html_xlat,string)) }
  return(string)
}

/* translate a whole list via sort_xlat to a sortable list */
proc translate(listin,listout) {
    forlist(listin,element,i) {
	enqueue(listout,strxlat(sort_xlat,element))
    }
}

/* translate string according to xlat table */
func strxlat(xlat,string) {
    set(fixstring,"")
    set(pos,strlen(string))
    while(pos) {
	set(char,substring(string,pos,pos))
	if (special,lookup(xlat,char)) {
	    set(fixstring,concat(special,fixstring))
	}
	else { set(fixstring,concat(char,fixstring)) }
	decr(pos)
    }
    return(fixstring)
}

proc init_xlat() {
/* This initializes the various translation tables.
   Note that these use the Macintosh encoding scheme!
*/

/* Translation table for sorting purposes.
   Note that this is mostly to handle German characters.
*/
    insert(sort_xlat,"š","oe")
    insert(sort_xlat,"ö","oe")
    insert(sort_xlat,"Ÿ","ue")
    insert(sort_xlat,"ü","ue")
    insert(sort_xlat,"Š","ae")
    insert(sort_xlat,"ä","ae")
    insert(sort_xlat,"§","ss")
    insert(sort_xlat,"ß","ss")
    insert(sort_xlat,"€","Ae")
    insert(sort_xlat,"Ä","Ae")
    insert(sort_xlat,"…","Oe")
    insert(sort_xlat,"Ö","Oe")
    insert(sort_xlat,"†","Ue")
    insert(sort_xlat,"Ü","Ue")
    insert(sort_xlat,"‘","e")
    insert(sort_xlat,"ë","e")
    insert(sort_xlat,"Ø","y")
    insert(sort_xlat,"ÿ","y")
    insert(sort_xlat,"Ž","e")
    insert(sort_xlat,"é","e")
    insert(sort_xlat,"–","n~")
    insert(sort_xlat,"ñ","n~")
    insert(sort_xlat,"Ï","oe")
    insert(sort_xlat,"œ","oe")

/* For the full list of HTML encodings for special characters, see
   http://info.cern.ch/hypertext/WWW/MarkUp/ISOlat1.html
*/
    insert(html_xlat,"š","&ouml;")
    insert(html_xlat,"ö","&ouml;")
    insert(html_xlat,"Ÿ","&uuml;")
    insert(html_xlat,"ü","&uuml;")
    insert(html_xlat,"Š","&auml;")
    insert(html_xlat,"ä","&auml;")
    insert(html_xlat,"§","&szlig;")
    insert(html_xlat,"ß","&szlig;")
    insert(html_xlat,"€","&Auml;")
    insert(html_xlat,"Ä","&Auml;")
    insert(html_xlat,"…","&Ouml;")
    insert(html_xlat,"Ö","&Ouml;")
    insert(html_xlat,"†","&Uuml;")
    insert(html_xlat,"Ü","&Uuml;")
    insert(html_xlat,"‘","&euml;")
    insert(html_xlat,"ë","&euml;")
    insert(html_xlat,"Ø","&yuml;")
    insert(html_xlat,"ÿ","&yuml;")
    insert(html_xlat,"Ž","&eacute;")
    insert(html_xlat,"é","&eacute;")
    insert(html_xlat,"&","&amp;")
    insert(html_xlat,"–","&ntilde;")
    insert(html_xlat,"ñ","&ntilde;")
    insert(html_xlat,"Ï","&oelig;")
    insert(html_xlat,"œ","&oelig;")

/* ISO 8859 translation for the GENDEX.txt file
*/
    insert(ISO8859_xlat,"š","ö")
    insert(ISO8859_xlat,"Ÿ","ü")
    insert(ISO8859_xlat,"Š","ä")
    insert(ISO8859_xlat,"§","ß")
    insert(ISO8859_xlat,"€","Ä")
    insert(ISO8859_xlat,"…","Ö")
    insert(ISO8859_xlat,"†","Ü")
    insert(ISO8859_xlat,"‘","ë")
    insert(ISO8859_xlat,"Ø","ÿ")
    insert(ISO8859_xlat,"Ž","é")
    insert(ISO8859_xlat,"–","ñ")
    insert(ISO8859_xlat,"Ï","œ")
}

/*
   quicksort: Sort an input list by generating a permuted index list
   Input:  alist  - list to be sorted
   Output: ilist  - list of index pointers into "alist" in sorted order
   Needed: compare- external function of two arguments to return -1,0,+1
          according to relative order of the two arguments
*/
proc quicksort(alist,ilist) {
  set(len,length(alist))
  set(index,len)
  while(index) {
    setel(ilist,index,index)
    decr(index)
  }
  if (ge(len,2)) { call qsort(alist,ilist,1,len) }
}

/* recursive core of quicksort */
proc qsort(alist,ilist,left,right) {
  if(pcur,getpivot(alist,ilist,left,right)) {
    set(pivot,getel(alist,getel(ilist,pcur)))
    set(mid,partition(alist,ilist,left,right,pivot))
    call qsort(alist,ilist,left,sub(mid,1))
    call qsort(alist,ilist,mid,right)
  }
}

/* partition around pivot */
func partition(alist,ilist,left,right,pivot) {
  while(1) {
    set(tmp,getel(ilist,left))
    setel(ilist,left,getel(ilist,right))
    setel(ilist,right,tmp)
    while(lt(compare(getel(alist,getel(ilist,left)),pivot),0)) {
      incr(left)
    }
    while(ge(compare(getel(alist,getel(ilist,right)),pivot),0)) {
      decr(right)
    }
    if(gt(left,right)) { break() }
  }
  return(left)
}

/* choose pivot */
func getpivot(alist,ilist,left,right) {
  set(pivot,getel(alist,getel(ilist,left)))
  set(left0,left)
  incr(left)
  while(le(left,right)) {
    set(rel,compare(getel(alist,getel(ilist,left)),pivot))
    if (gt(rel,0)) { return(left) }
    if (lt(rel,0)) { return(left0) }
    incr(left)
  }
  return(0)
}

/* compare strings */
func compare(string1,string2) {
  return(strcmp(string1,string2))
}

