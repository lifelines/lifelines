/*
 * @progname       html.dn.ll
 * @version        3.0
 * @author         Dennis Nicklaus nicklaus@fnal.gov
 * @category       
 * @output         HTML
 * @description
 *
 *   Selects a person and writes html pages for that person
 *   and all their descendents through a specified number of generations.
 *   Actually, you get to specify a set of individuals.  It probably
 *   works nicest if you select people of the same generation, e.g.
 *   all your ggg-grandparents.
 *   (I also recommend that you start with the "top of the line" ancestor
 *   or else you'll have unresolved links in the pedigree chart.)
 *
 *   Output is a set of ASCII HTML files, one for each person in the set.
 *   In addition, it writes a surname index file named <db>index.html,
 *   and one named <db>-gendex, which is a GENDEX format index text file.
 *   <db> is the database name.
 *
 *   Note that I APPEND to the index files! This is necessary if you're building
 *   up a set of pages by multiple runs of html.dn.  But you want to remove
 *   the old ones, or go to a different directory, if you're starting a whole
 *   new set of pages. 
 *
 *   Why would you want to run it multiple times?  Suppose I wanted all my 
 *   relatives through my generation. On my dad's side I know all my great-grandparents
 *   but on mom's side, only my grandparents.  To create my set of pages,
 *   I'd first run this program, specifying all of my g-gparents (on dad's side)
 *   and number of generations =4.  Then run it a second time, specifying
 *   only my 2 grandparents on mom's side and # generations=3.

 *   The individual's HTML files are named <db>I<nnn>.html where
 *   <nnn> is replaced with the individual's key ID number.
 *   Since it can re-write the same individual html files multiple times,
 *   I usually sort the two index files with something like this (suppose db=dad):
 *   sort -u -t \> -k 2 dadindex.html > dadindex.sort; mv dadindex.sort dadindex.html
 *   (the individual html files expect the index to be named dadindex.html.)
 *   For the GENDEX index:
 *   sort -t \| -k 2 -u dad-gendex >gendex.txt
 *   (note that I use the -u flag to take out duplicate entries)
 *
 *   Actually, before sorting, I usually run the indexes and (all the indi html files)
 *   through some big "sed" scripts to take things from the LaTex notation
 *   I use to either HTML notation or plain text.
 *   e.g. for the gendex file, which doesn't want html special chars:
 *   (and you may have to use csh for this quoting convention here to work.)
 *           sed -e 's/\\"u/ue/g'  <gendex.in > gendex.out
 *   
 *   For the html files, it looks like 
 *           sed -e 's/\\"u/\&uuml;/g' <input > output
 *   I also use -e 's/\\begin{enumerate}/\<OL\>/g' -e 's/\\end{enumerate}/\<\/OL\>/g'  
 *   and similar things to go from LaTex to HTML.
 *
 *   The bibliography: The html generator looks for SOUR xrefs used on each indi
 *   and prints  a list of them at the bottom of each HTML file, and references a file
 *   <db>bib.html which is the expected bibliography.
 *   But this program doesn't generate the bibliography.
 *   I personally use book-latex to generate the bibliography then munge
 *   it around into a nicer HTML format. (I have a C program for this.)
 *   You can use whatever you want.
 * 
 *   SHORTCOMINGS:
 *   1. Takes time to do two pretty much identical pages for direct ancestors
 *   (or anyone else who is descended from more than one of the same starting people.
 *   (That's OK, the most recent one written will just overwrite the first.)
 *
 *   A previous verion had lots of wasted white space.
 *   I don't like the way pagemode works at all.
 *   I had to fill in lots of extra whitespace, e.g. at the top of the pedigree
 *   chart to make it not overwrite some of the text which was already output.
 *   It may still fail somewhere along the way.
 *   I completely changed around the way the pedigree chart is drawn to get 
 *   rid of this problem.  The result is a more compact html page, but with
 *   a few less details on the pedigree chart.
 *   I left all the old code in there (commented out), so it is confusing to read.
 *
 *   If you look closely, you will notice a few things like: concat(dbname,"")
 *   These are in there so I can fill in a numeral or something between the
 *   empty quote marks so that I can have a dad2index.html, e.g.
 *
 *   DETAILS:
 *   1. I have a special "1 NOTE FILE:" construct that I use to refer to external files.
 *   2. I also use the "1 OBJE" to link to external images (e.g. .GIF)
 *   relevant to the person.
 *   3. change the info in init() before you use this!!
  *
 *   EXAMPLE:
 *   See http://www.geocities.com/dnicklaus/dadindex.html
 *   and just pick a name from that index to see what it looks like.
 *   dadI3.html might be a good one to start at.
 *
 *   Version 3.0 July 1998 
 *         What's different from Version 2?
 *         Don't make a page and a link for the child if we know nothing
 *         about the child except his name and one simple fact.
 *   Version 2.0 July 1998 (Version 1.0 was Feb. 1998)
 *         What's different from Version 1?  Simpler, more compact pedigree chart.
 *
 *   This report works only with the LifeLines Genealogy program
 *
 */
global(dbname)
global(extension)
global(email)
global(personalname)
global(homepage)
global (gotValue)
global (gottenNode)
global (gottenValue)
global (global_gp_var)
proc init ()
{
	set(dbname,save(database()))
	set(extension,".html")
	set(personalname,getproperty("user.fullname"))
	set(email,getproperty("user.email"))
	set(homepage,"http://www.yourplace.here")
/*e.g.:	set(homepage,"http://www.geocities.com/dnicklaus/index.html")*/
}
proc main ()
{
	call init()
    getindimsg(person,"Enter person to output HTML for Descendents")
    indiset(thisgen)
    indiset(allgen)
    indiset(newgen)
    while (person){
	addtoset(thisgen, person, 0)
	addtoset(allgen, person, 0)

	set(person,0)
        getindimsg(person,"Enter next person to output HTML for Descendents")
     }

    getintmsg (ngen,
               "Enter number of generations for complete info")

	/* collect descendents */

    set(gen,2)  /* this code has to do at least 2 generations */
    while(lt(gen,ngen)){
	set(thisgen,childset(thisgen))
        set(allgen,union(allgen,thisgen))
        set(gen,add(gen,1))
     }
     set(newgen,childset(thisgen))

	/* print out individual html files */
	
     forindiset(allgen,person,val,thisgensize) { 
       if (gt(worth_doing(person),1)) {
  	  newfile(save(concat (concat(dbname,key(person)),extension)),0)
	  call do_it_all(person,1 ) 
	}
      }
	/* list last newgen will not have any hyperlinks from the children
	   in this set */
     forindiset(newgen,person,val,thisgensize) { 
       if (gt(worth_doing(person),1)) {
	 newfile(save(concat (concat(dbname,key(person)),extension)),0)
	 call do_it_all(person,0) 
	}
     }

	/* write out a GENDEX format index */
    newfile(save(concat(concat(dbname,""),"-gendex")),1)
     forindiset(allgen,person,val,thisgensize) { 
       if (gt(worth_doing(person),1)) {
	call write_gendex_line(person)
       }  
     }
     forindiset(newgen,person,val,thisgensize) { 
       if (gt(worth_doing(person),1)) {
	call write_gendex_line(person)
       }
     }
	/* write out a normal index for HTML use */
    newfile(save(concat(concat(dbname,""),"index.html")),1)
     forindiset(allgen,person,val,thisgensize) { 
       if (gt(worth_doing(person),1)) {
	call write_regular_index_line(person)
       }
     }
     forindiset(newgen,person,val,thisgensize) { 
       if (gt(worth_doing(person),1)) {
	call write_regular_index_line(person)
       }
     }
}
proc do_it_all(indi,linkit)

{
	print(key(indi)) print(" ")
	call do_chart_head(indi)
	"<Center> <H2>" name(indi,0) "</H2></Center>\n"
	"<HR>\n"
	call getGif(inode(indi))
	"<HR>\n"
	"<DL>\n"
	if (e,birth(indi))   {  "<DT>     born: " long(e) call doAddr(e) "\n" }
	if (e,baptism(indi))  { "<DT>     bapt: " long(e) call doAddr(e) "\n" }
	if (e,death(indi))   {  "<DT>     died: " long(e) call doAddr(e) "\n" }
	if (e,burial(indi))   { "<DT>     bur.: " long(e) call doAddr(e) "\n" }
	call getValueCont(inode(indi),"OCCU")
	if (gotValue){          "<DT>     occu: " gottenValue "\n"}
	call getValueCont(inode(indi),"WILL")
	if (gotValue){          "<DT>     Will: " long(gottenNode) call doAddr(gottenNode)"\n"}
	call getValueCont(inode(indi),"PROB")
	if (gotValue){          "<DT> Probated: " long(gottenNode) call doAddr(gottenNode)"\n"}

        families(indi,fam,sp,spi) {
		if(sp){	        "<DT>  spouse:  <A HREF=\"" dbname key(sp) extension "\">" name(sp) "</a>\n" 
			if (e,marriage(fam)){ "<DD>       marr: " long(e) call doAddr(e) "\n" }
			if (e,birth(sp))   { "<DD>       born: " long(e) call doAddr(e) "\n" }
			if (e,baptism(sp))  { "<DD>       bapt: " long(e) call doAddr(e) "\n" }
			if (e,death(sp))   { "<DD>       died: " long(e) call doAddr(e) "\n" }
			if (e,burial(sp))  { "<DD>       bur.: " long(e) call doAddr(e) "\n" }
			call getValueCont(inode(sp),"OCCU")
			if (gotValue){          "<DD>     occu: " gottenValue "\n"}

		}
		if (gt(nchildren(fam),0)){		 "<DT>     Children:\n"}
                children (fam,ch,famchi) {   "       "

			if (linkit){ 
			  if (gt(worth_doing(ch),1)) {
	"<DD> <A HREF=\"" dbname key(ch) extension "\">" name(ch) "</a>\n"
                          }
			  else { 
			    /* here worth_doing must = exactly 0 or 1 
			       print out the name and the one thing we know, 
			       but no link */
			    "<DD>" name(ch,0) 
			if (e,birth(ch))   { " -- born: " long(e) call doAddr(e) "." }
			if (e,baptism(ch)) { " -- bapt: " long(e) call doAddr(e) "." }
			if (e,death(ch))   { " -- died: " long(e) call doAddr(e) "." }
			if (e,burial(ch))  { " -- bur.: " long(e) call doAddr(e) "." }
			      "\n"
			    }
			}
			else { "<dd>" name(ch,0) "<br>\n" }
		}
	}
	"</DL>\n"
  if (parents(indi)){
	call has_grandparents(indi)
	if (eq(0,global_gp_var)){
		set(fath,father(indi))
		if (fath){
			"Father: "
			"<A HREF=\"" dbname key(fath) extension"\">"
			name(fath) "</a>"
			" ("	
			if (e,birth(fath))   { year(e)}
			"-"
			if (e,death(fath))   { year(e)}
			")"
			"<BR>\n"

		}
		set(moth,mother(indi))
		if (moth){
			"Mother: "
			"<A HREF=\"" dbname key(moth) extension"\">"
			name(moth) "</a>"
			" ("	
			if (e,birth(moth))   { year(e)}
			"-"
			if (e,death(moth))   { year(e)}
			")"
			"<BR>\n"

		}
	}
   else{

  "<CENTER><H3> Pedigree Chart</H3></CENTER>"
/*	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
	"                                                                     "
	"                                                                     \n"
*/
	/* all this extra crappy space is apparently necessary because
	of the way the pagemode feature gobbles up the output buffer for
	lines which haven't been written out yet. It gets gobbled as whitespace*/
/*	pagemode(64,120)
	pos(1,1)*/
	 "<PRE>"
/*	call pedout(indi,1,4,1,64)
	print(nl())
	pos(64,1)
	pageout()
	linemode()
*/
	call pedout2(indi)
	"</PRE>\n"
	}
  }
	"<HR>\n"
	call getText(inode(indi),1)
	"<HR>\n"
	call getCensus(inode(indi))
	"<HR>\n"
	call do_file_notes(indi)


	"<p>\nSources for this individual:  <a href=\"" dbname "bib.html\">"
	call sour_addind(indi) "</a> <br>\n"
	"<HR>\n"
"<center>\n"
"<a href=\"" homepage "\"> Homepage </a> | \n"
"<a href=\"genealogy.html\"> Genealogy Home </a> | \n"
"<a href=\"" dbname  "index.html\"> Index </a> | \n"
"<a href=\"" dbname  "about.html\"> Explanations </a><br>\n"
"</center>\n"
"<a href=\"mailto:" email "\">" personalname " " email "</a><br>" 

}

proc pedout (indi, gen, max, top, bot)
{
	if (and(indi,le(gen,max))) {
		set(gen,add(1,gen))
		set(fath,father(indi))
		set(moth,mother(indi))
		set(height,add(1,sub(bot,top)))
		set(offset,div(sub(height,8),2))
		call block(indi,add(top,offset),mul(10,sub(gen,2)))
		set(half,div(height,2))
		call pedout(fath,gen,max,top,sub(add(top,half),1))
		call pedout(moth,gen,max,add(top,half),bot)
	}
}

proc do_chart_head(indi){
  "<HTML><HEAD>"
  "<TITLE>"
  name(indi,0) 
  " Family"
  "</TITLE>"
  "</HEAD>\n"
}
proc block (indi, row, col)
{
	print(".")
	set(row,add(3,row))


	set(col,add(3,col))
	pos(row,col)
	"<A HREF=\"" dbname key(indi) extension"\">"
	name(indi)
	"</a>"
	set(row,add(row,1))
	pos(row,col)
	set (e,birth(indi))
	if (e){ " b. " long(e)}
	set(row,add(row,1))
	pos(row,col)
	set (e,death(indi))
	if (e){ " d. " long(e)}
	set(row,add(row,1))
	pos(row,col)

}
proc doname (indi)
{
  if(indi){
	"<A HREF=\"" dbname key(indi) extension"\">"
	name(indi)
	"</a> ("
	set (e,birth(indi))
	if (e){short(e)}
	" - "
	set (e,death(indi))
	if (e){short(e)}
	")"
  }
}

proc getText (root, paragraph) {
  if (root) {
    fornodes (root, node) {
      if (not (strcmp ("TEXT", tag (node)))) {
	if (paragraph) { "\n\n" set (paragraph, 0) }
        call values (node)
	"\n\n<p>"
      }
    }
  }
}
proc getGif (root) {

  if (root) {
    fornodes (root, node) {
      if (not (strcmp ("OBJE", tag (node)))) {
	"<center>\n"
        call getValueCont (node,"TITL")
	if (gotValue){ set (title,save(gottenValue)) }
        call getValueCont (node,"FILE")
	if (gotValue){ set (file,save(gottenValue)) }
	"<IMG src=" file " alt=\"" title "\"> <br>"
	title "<br>"
	"</center>\n"
	
      }
    }
  }
}
proc getCensus (root) {
  if (root) {
    fornodes (root, node) {
      if (not (strcmp ("CENS", tag (node)))) {
        "Census: " long(node)
	"<br>\n"
      }
    }
  }
}

proc values (root) {
  if (root) {
    if (strlen (value (root))) { "\n" value (root) }
    fornodes (root, node) {
      if (not (strcmp ("CONT", tag (node)))) {
        if (strlen (value (node))) { "\n" value (node) }
      }
    }
  }
}
proc doAddr(event)
{
  fornodes(event, subnode) {
    if (eq(0,strcmp("PLAC", tag(subnode)))) {
      fornodes(subnode, subnode2) {
        if (eq(0,strcmp("ADDR", tag(subnode2)))) {	
            ", at " value(subnode2) 
  }}}}
}

proc getValueCont (root, t) {
  set (gotValue, 0)
  if (root) {
    fornodes (root, node) {
      if (and (not (gotValue), not (strcmp (tag (node), t)))) {
        set (gotValue, 1)
        set (gottenNode, node)
        set (gottenValue, save (value (node)))
        fornodes (node, subnode) {
          if (not (strcmp ("CONT", tag (subnode)))) {
            if (strlen (value (subnode))) {
	      set (gottenValue, 
	        save (concat (gottenValue, concat ("\n", value (subnode)))))
            }
          }
        }
      }
    }
  }
}

/* sour_addind() adds the sources referenced for this individual */
proc sour_addind(i)
{
        table(sour_table)
        list(sour_list)

	/* first get all the sources in the INDI record */
         traverse(root(i), m, l) {
                if (nestr("SOUR", tag(m))) { continue() }
                set(v, value(m))
                if (eqstr("", v)) { continue() }
                if(reference(v)) {
                          if (ne(0, lookup(sour_table, v))) { continue() }
                          set(v, save(v))
                          insert(sour_table, v, 1)
                          v " "
                }
         }
	/* now get all the sources in the FAM records where this person is a spouse */
        families(i,fam,sp,spi) {
         traverse(root(fam), m, l) {
                if (nestr("SOUR", tag(m))) { continue() }
                set(v, value(m))
                if (eqstr("", v)) { continue() }
                if(reference(v)) {
                          if (ne(0, lookup(sour_table, v))) { continue() }
                          set(v, save(v))
                          insert(sour_table, v, 1)
                          v " "
                }
         }
       }
}

proc write_gendex_line(person)
{
	set(separator,"|")
	dbname key(person) extension
	separator
	surname(person)
	separator
	givens(person) " /" surname(person) "/" /*	fullname(person,0,1,110)*/
	separator
	if (e,birth(person))   { date(e)}
	separator	
	if (e,birth(person))   { place(e)}
	separator	
	if (e,death(person))   { date(e)}
	separator	
	if (e,death(person))   { place(e)}
	separator	
	"\n"
}
proc write_regular_index_line(person)
{
	"<A HREF=\"" dbname key(person) extension "\">"
	fullname(person,0,0,110) "</A>"
	" ("	
	if (e,birth(person))   { year(e)}
	"-"
	if (e,death(person))   { year(e)}
	")"
	"<BR>\n"
}
proc has_grandparents(indi)
{
	set(global_gp_var,0)
	set(fath,father(indi))
	if (parents(fath)){
		set(global_gp_var,1)
	}
	set(moth,mother(indi))
	if (parents(moth)){
		set(global_gp_var,1)
	}
}
proc do_file_notes(person)
{
    set (done_once,0)
    fornotes(inode(person),note){
	set (i, index(note,"FILE:",1))
	if (gt(i,0)){
		set(what,save(substring(note,add(i,6),strlen(note))))
		set (i, index(what," ",1))
		set(descrip,save(substring(what,1,i)))

		/* now get and flatten the file name */
		set (i, index(what,"FAMHIST/",1))
		set (fname,save(substring(what,add(i,strlen("FAMHIST/")),strlen(what))))o
		set (slash, index(fname,"/",1))
		while (gt(slash,0)){
		set(fnameb,save(concat(concat(substring(fname,1,sub(slash,1)),"_"),
					substring(fname,add(slash,1),strlen(fname)))))
			set(fname,fnameb)
			set (slash, index(fname,"/",1))
		}
		set (slash, index(fname,".",1))
		while (gt(slash,0)){
			set(fnameb,save(concat(concat(substring(fname,1,sub(slash,1)),"_"),
						substring(fname,add(slash,1),strlen(fname)))))
			set(fname,fnameb)
			set (slash, index(fname,".",1))
		}
	if (done_once) { /* this isn't the first time */
		" | " /* simple separator */
	}
        else { "More information: " set(done_once,1)}
	"<a href=\"" fname ".txt\">"
	descrip
	"</a>\n"
	}

   }
}
		

/* this function helps us to not print out a whole www page for someone
   that I don't know anything about except for maybe the name.
   return a 1 if I know one simple fact, and a 0 if I don't know anything,
   and a bigger number if I know something more complicated about them.*/

func worth_doing(child) {
  
  set (worth, 0)
  if (birth(child))   { set (worth,add(worth,1)) }
  if (baptism(child))  { set (worth,add(worth,1)) }
  if (death(child))   { set (worth,add(worth,1)) }
  if (burial(child))   {set (worth,add(worth,1)) }
  if (gt(nfamilies(child),0)) {  return(10) }
  call getValueCont(inode(child),"OCCU")
    if (gotValue){   return(10) }
  call getValueCont(inode(child),"TEXT")
    if (gotValue){  return(10) }
  call getValueCont(inode(child),"WILL")
    if (gotValue){  return(10) }
  call getValueCont(inode(child),"PROB")
    if (gotValue){  return(10) }
  call getValueCont(inode(child),"OBJE")
    if (gotValue){  return(10) }
  call getValueCont(inode(child),"FILE")
    if (gotValue){  return(10) }
  call getValueCont(inode(child),"CENS")
    if (gotValue){  return(10) }

  return(worth)
}



proc pedout2 (indi)
{
/* I actually collect enough names here to do a 5generation pedigree chart,
   but I only print out 4 generations. */
	set(fath,father(indi))
	set(moth,mother(indi))
	if (fath) {
	   set(ffath,father(fath))
	   set(mfath,mother(fath))
	   if (ffath) {
	   	set(fffath,father(ffath))
	   	set(mffath,mother(ffath))
		if (fffath) {
	   		set(ffffath,father(fffath))
	   		set(mfffath,mother(fffath))
		}
		if (mffath) {
	   		set(fmffath,father(mffath))
	   		set(mmffath,mother(mffath))
		}
  	   }			
	   if (mfath) {
	   	set(fmfath,father(mfath))
	   	set(mmfath,mother(mfath))
		if (fmfath) {
	   		set(ffmfath,father(fmfath))
	   		set(mfmfath,mother(fmfath))
		}
		if (mmfath) {
	   		set(fmmfath,father(mmfath))
	   		set(mmmfath,mother(mmfath))
		}
  	   }			
	}
	if (moth) {
	   set(fmoth,father(moth))
	   set(mmoth,mother(moth))
	   if (fmoth) {
	   	set(ffmoth,father(fmoth))
	   	set(mfmoth,mother(fmoth))
		if (ffmoth) {
	   		set(fffmoth,father(ffmoth))
	   		set(mffmoth,mother(ffmoth))
		}
		if (mfmoth) {
	   		set(fmfmoth,father(mfmoth))
	   		set(mmfmoth,mother(mfmoth))
		}
  	   }			
	   if (mmoth) {
	   	set(fmmoth,father(mmoth))
	   	set(mmmoth,mother(mmoth))
		if (fmmoth) {
	   		set(ffmmoth,father(fmmoth))
	   		set(mfmmoth,mother(fmmoth))
		}
		if (mmmoth) {
	   		set(fmmmoth,father(mmmoth))
	   		set(mmmmoth,mother(mmmoth))
		}
  	   }			
	}
	
"                      |--------" call doname(fffath) "\n"
"                      |\n"
"           |---------" call doname(ffath) "\n"
"           |          |\n"
"           |          |--------" call doname(mffath) "\n"
"           |\n"
"  |------" call doname(fath) "\n"
"  |        |\n"
"  |        |          |--------" call doname(fmfath) "\n"
"  |        |          |\n"
"  |        |---------" call doname(mfath) "\n"
"  |                   |\n"
"  |                   |--------" call doname(mmfath) "\n"
"  |\n"
call doname(indi) "\n"
"  |\n"
"  |                   |--------" call doname(ffmoth) "\n"
"  |                   |\n"
"  |        |---------" call doname(fmoth) "\n"
"  |        |          |\n"
"  |        |          |--------" call doname(mfmoth) "\n"
"  |        |\n"
"  |------" call doname(moth) "\n"
"           |\n"
"           |          |--------" call doname(fmmoth) "\n"
"           |          |\n"
"           |---------" call doname(mmoth) "\n"
"                      |\n"
"                      |--------" call doname(mmmoth) "\n"
}

/*
                      |--------bill
                      |
           |---------joe
           |          |
           |          |--------mary
           |
  |------sam
  |        |
  |        |          |--------fred
  |        |          |
  |        |---------sue
  |                   |
  |                   |--------sally
bill
  |                   |--------john
  |                   |
  |        |---------jack
  |        |          |
  |        |          |--------mary
  |        |
  |------jane
           |
           |          |--------fred
           |          |
           |---------sue
                      |
                      |--------sally


*/
/* End of Report */
