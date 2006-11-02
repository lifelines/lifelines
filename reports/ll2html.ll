/*
 * @progname       ll2html.ll
 * @version        2005-11-19
 * @author         JRE Jim Eggert
 * @category       
 * @output         HTML
 * @description
 *
 *  This report program converts a LifeLines database into html documents.
 *  Family group records are created for each selected individual in
 *  the database.  These records are written in files containing clumps
 *  of individuals of a user-selected size.  Index files are generated
 *  for an index document.  Or, optionally, all output is sent to
 *  one file.
 *
 *  You will need to change the contents of proc html_address() and to
 *  set the value of HREF appropriately to your server.
 *  You need to set the value of PATH to point to the directory to put
 *  the files into. If you have 1000 individuals in your database this
 *  program will create up to 1027 files, one for each individual and
 *  up to 27 index files, if you set the clump size to one.
 *
 *  This program will also generate three pedigree charts for the root
 *  individual and descendants charts for selected individuals.
 *
 *  You also need to set the value of HOST to be the http server and
 *  path where you will server these files from.
 *
 *  History
 *   01-07-94  sew; Created.
 *   11-18-94  jre; Added clump capability.
 *   02-16-95  jre; Added privacy option.
 *   03-06-95  jre; Added pedigree table, better sorting.
 *   05-10-95  jre; Added descendants charts.
 *   05-02-97  jre; Added ISO8859 encoding in GENDEX.txt file.
 *   07-09-99  jre; Added background decorations, improved HTML.
 *   01-15-00  jre; Fixed quicksort bug
 *   11-19-05  jre; Updated released version to rev 12.  Many changes.
 *
 */

global(INDEX)
global(INDEXTABLE)
global(HREF)
global(PATH)
global(PEDIGREE_NAME)
global(INDEX_NAME)
global(TITLE)
global(ADDRESS)
global(FB)
global(nl)
global(qt)
global(CURRENTCLUMPFILE)
global(root_person)
global(root_key)
global(separate_clumps)
global(PRIVTABLE)
global(privacytern)
global(sort_xlat)
global(html_xlat)
global(ISO8859_xlat)

/* These globals are for descendant reports */
global(grouped_henry)
global(comma_separation)
global(first_comma)
global(generations)

/* This is for descendant and ancestor reports */
global(written_people)
global(tree)
global(ancestors)
global(qt)
global(deltax)
global(deltay)
global(html_xlat)

/* These constants are for estimating birth years */
global(years_between_kids)
global(mother_age)
global(father_age)

/* These globals are for time limits on privacy */
global(hundred_years_ago)
global(eighty_years_ago)

/* Decoration globals */
global(male_gif)
global(female_gif)
global(unknown_gif)
global(logo_gif)
global(background_gif)

proc main()
{

/* Change these to suit your needs */

    set(TITLE,"Eggert Family Genealogy") /* Title of main genealogy page */
    set(PEDIGREE_NAME,"Eggert Family Ancestry") /* Pedigree chart title */
    set(INDEX_NAME,"Eggert Family Genealogy Home ") /* Index title */
    set(DESC_NAME,"Eggert Family Descendant List") /* Descendant list title */
    set(PATH, "") /* path for file references */
    set(HREF, "") /* host and path */

    set(qt, qt())
    set(male_gif,concat(qt,"7m.gif",qt," HEIGHT=68 WIDTH=68"))
    set(female_gif,concat(qt,"7f.gif",qt," HEIGHT=80 WIDTH=50"))
    set(unknown_gif,concat(qt,"5U.GIF",qt))
    set(logo_gif,concat("<A HREF=",qt,"index.html",qt,"><IMG SRC=",qt,"7e.gif",qt,
	" HEIGHT=80 WIDTH=57 ALIGN=",qt,"MIDDLE",qt," ALT=",qt,"?",qt,"></A>"))
    set(background_gif,concat(qt,"oldyellow.gif",qt))

    set(FB, 0)
    set(nl, nl())
    list(INDEX)
    table(INDEXTABLE)
    table(PRIVTABLE)
    table(sort_xlat)
    table(html_xlat)
    table(ISO8859_xlat)
    call init_xlat()
    call init_years()

    indiset(people)
    getindimsg(root_person,"Enter root individual:")
    set(root_key,key(root_person))
    set(clumpsize,0)
    while (le(clumpsize,0))
    {
	getintmsg(clumpsize,"Enter number of individuals per file:")
    }
/*    getintmsg(separate_clumps,
 *	      "Do you want clumps in separate files (0=no,1=yes)?")
 */
    set(separate_clumps,1)
    list(choices)
    enqueue(choices,"all")
    enqueue(choices,"deceased individuals only")
    enqueue(choices,"none")
    set(privacytern,sub(menuchoose(choices,"Include notes and dates for:"),1))
    list(nonprivates)
    if (privacytern) {
	set(person,1)
	while(person) {
	    set(person,0)
	    getindimsg(person,"Enter non-private person:")
	    if (person) { enqueue(nonprivates,key(person)) }
	}
    }
    list(desc_roots)
    set(person,1)
    while(person) {
	set(person,0)
	getindimsg(person,"Enter root for descendant list:")
	if (person) { enqueue(desc_roots,key(person)) }
    }

    print("Finding ancestry... ")
    addtoset(people, root_person, 0)
    set(people,union(ancestorset(people),descendantset(people)))
    addtoset(people, root_person, 0)
    set(people,union(people,spouseset(people)))
/*    set(people,union(people,childset(people))) */

    set(indicount,0)
    set(clumpcount,1)

    print("done\nCollating index... 1")
    forindiset(people,me,val,num)
    {
	/* print(".") */
	incr(indicount)
	if (ge(indicount,clumpsize))
	{
	    incr(clumpcount)
	    set(indicount,0)
	    print(" ", d(clumpcount))
	}
	set(k,key(me))
	enqueue(INDEX,k)
	insert(INDEXTABLE,k,clumpcount)
	if (eq(privacytern,1)) { insert(PRIVTABLE,k,privacy(me)) }
	elsif (eq(privacytern,0)) { insert(PRIVTABLE,k,0) }
	else { insert(PRIVTABLE,k,1) }
    }

    if (privacytern) {
	while (pkey,dequeue(nonprivates)) {
	    insert(PRIVTABLE,pkey,0)
	}
    }

    print(" done\nWriting index(slow)...")
/* */
    call create_index_file(desc_roots)
/* */
    print(" done\nWriting name files...")
    call start_clumpfile(1)
    forindiset(people, me, val, num)
    {
	call write_indi(me)
    }
    call end_clumpfile()
/* */
/* */
    print(" done\nWriting pedigree chart...")
    call pedigree_chart(indi(root_key))
/* */
/* Disable privacy checks for protected access reports */
    set(privacyternsave,privacytern)
    set(privacytern,0)
    print(" done\nWriting descendant lists...")
    call descendant_lists(desc_roots)
    print("done\n")
    set(privacytern,privacyternsave)
}

proc descendant_lists(desc_roots) {
    set(grouped_henry,0)
    set(comma_separation,3)
    set(first_comma,0)
    set(generations,0)
    while (desc_key,dequeue(desc_roots)) {
	print(desc_key," ")
	set(desc_root,indi(desc_key))
	list(henry_list)
	table(written_people)
	push(henry_list,substring(mysurname(desc_root),1,1))
	set(fn, concat(PATH, "onlyfamilydesc",desc_key,".html"))
	if (separate_clumps) { newfile(fn, FB) }
	call html_header(DESC_NAME, 0)
	"<PRE>\n"
	call do_header(desc_root)
	call desc_sub(desc_root,henry_list)
	call do_trailer(desc_root)
	"</PRE>"
	call html_trailer("","Genealogy%20descendant%20lists")
    }
}

proc pedigree_chart(person) {
    set(fn, concat(PATH, "pedigree.html"))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(PEDIGREE_NAME, 0)
    "Go to <A HREF=" qt "pedigreeg.html" qt ">graphic version</A> or sort by <A HREF="
    qt HREF "pedigreen.html" qt ">generation</A> or "
    "<A HREF=" qt HREF "pedigreea.html" qt ">name</A>.<p>\n<PRE>\n"
    table(written_people)
    call pedigree(0, 1, person)
    "</PRE>\n"
    call html_trailer("","Pedigree%20list")

    set(fn, concat(PATH, "pedigreen.html"))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(PEDIGREE_NAME, 0)
    "Go to <A HREF=" qt "pedigreeg.html" qt ">graphic version</A> or sort by <A HREF="
    qt HREF "pedigree.html" qt ">lineage</A> or "
    "<A HREF=" qt HREF "pedigreea.html" qt ">name</A>.<p>\n"
    call ahnen(person)
    "\n"
    call html_trailer("","Ahnentafel%20list")

    set(fn, concat(PATH, "pedigreea.html"))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(PEDIGREE_NAME, 0)
    "Go to <A HREF=" qt "pedigreeg.html" qt ">graphic version</A> or sort by <A HREF="
    qt HREF "pedigree.html" qt ">lineage</A> or "
    "<A HREF=" qt HREF "pedigreen.html" qt ">generation</A>.<p>\n<PRE>\n"
    call ahnensort(person)
    "</PRE>\n"
    call html_trailer("","Ancestor%20list")

    set(fn, concat(PATH, "pedigreeg.html"))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header_graphic(PEDIGREE_NAME, 0)
    "Go to <A HREF=" qt "pedigree.html" qt ">text version</A> or sort by <A HREF="
    qt "pedigreen.html" qt ">generation</A> or <A HREF=" qt "pedigreea.html" qt ">name</A>."
    call tableau(person)
    call html_trailer_graphic("","Pedigree%20graph")
}

proc pedigree(in, ah, indi) {
    if (didah,lookup(written_people,key(indi))) {
	rjustify(d(ah),add(1,mul(in,2))) " " call href(indi,neg(1)) " (see " d(didah) ")" nl
    } else {
	if (par, father(indi)) { call pedigree(add(1,in), mul(2,ah), par) }
	rjustify(d(ah),add(1,mul(in,2))) " " call href(indi,neg(1)) nl
	insert(written_people,key(indi),ah)
	if (par, mother(indi)) { call pedigree(add(1,in), add(1,mul(2,ah)), par) }
    }
}

proc ahnen(person) {
    table(written_people)
    list(plist)
    list(nlist)
    enqueue(plist,person)
    enqueue(nlist,1)
    set(twotothen,1)
    set(greatcount,neg(2))
    while(p,dequeue(plist)) {
	set(n,dequeue(nlist))
	while (ge(n,twotothen)) {
	    if    (eq(twotothen,1)) { set(label,"Self") }
	    elsif (eq(twotothen,2)) { set(label,"Parents") }
	    elsif (eq(twotothen,4)) { set(label,"Grandparents") }
	    elsif (eq(twotothen,8)) { set(label,"Great-Grandparents") }
	    else { set(label,concat("Great(x",d(greatcount),")-Grandparents")) }
	    "<HR SIZE=4 NOSHADE><H2>" label "</H2>\n"
	    set(twotothen,add(twotothen,twotothen))
	    incr(greatcount)
	}
	d(n) " " call href(p,neg(1))
	if (other,lookup(written_people,key(p))) {
	    " (see " d(other) " above)"
	} else {
	    insert(written_people,key(p),n)
	    if (f,father(p)) {
		enqueue(plist,f)
		enqueue(nlist,mul(2,n))
	    }
	    if (m,mother(p)) {
		enqueue(plist,m)
		enqueue(nlist,add(1,mul(2,n)))
	    }
	}
	"<BR>\n"
    }
}

proc ahnensort(person) {
    list(plist)
    list(nlist)
    list(klist)
    list(nklist)
    table(written_people)
    enqueue(plist,person)
    enqueue(klist,key(person))
    enqueue(nlist,1)
    enqueue(nklist,1)
    while(p,dequeue(plist)) {
	set(n,dequeue(nlist))
	if (f,father(p)) {
	    if (didit,lookup(written_people,key(f))) { "" }
	    else {
		insert(written_people,key(f),n)
		enqueue(plist,f)
		enqueue(klist,key(f))
		set(nf,add(n,n))
		if (gt(nf,nmax)) { set(nmax,nf) }
		enqueue(nlist,nf)
		enqueue(nklist,nf)
	    }
	}
	if (m,mother(p)) {
	    if (didit,lookup(written_people,key(m))) { "" }
	    else {
		insert(written_people,key(m),n)
		enqueue(plist,m)
		enqueue(klist,key(m))
		set(nm,add(n,n,1))
		if (gt(nm,nmax)) { set(nmax,nm) }
		enqueue(nlist,nm)
		enqueue(nklist,nm)
	    }
	}
    }
    list(sortindex)
    list(transindex)
    call translate(klist,transindex)
    call quicksort(transindex,sortindex)
    set(maxspacecount,strlen(d(nmax)))
    forlist(sortindex,sindex,counter)
    {
	set(p,indi(getel(klist,sindex)))
	set(n,getel(nklist,sindex))    
	set(spacecount,sub(maxspacecount,strlen(d(n))))
	while(spacecount) { " " decr(spacecount) }
	d(n) " " call href(p,neg(1)) nl
    }
}

proc do_header(indi_root)
{
    "desc-henry:  Descendant report for " fullname(indi_root,0,1,80)
    "\nGenerated by the LifeLines Genealogical System on "
    stddate(gettoday()) ".\n\n"
}

proc do_trailer(indi_root)
{
    "\nEnd of Report\n"
}

proc tableau(indi_root)
{
    set(deltax,80)
    set(deltay,16)

    list(tree)  /* this will be a list of generations, most recent first */
		/* each generation will be a list of ancestors, most paternal first */
		/* each ancestor will be a list containing their data:
key (can be duplicate), generation, ahnentafel, y position, father ancestor, mother ancestor, duplicate boolean */
    table(ancestors) /* keys are ancestors, entries are lowest ahnentafel numbers */
    list(plist)
    list(ancestor)
    enqueue(ancestor,key(indi_root))
    enqueue(ancestor,1)
    enqueue(ancestor,1)
    enqueue(plist,ancestor)
/* Generate basic pedigree tree */
    while (ancestor,dequeue(plist)) {
	set(key,getel(ancestor,1))  /* get basic information */
	set(gen,getel(ancestor,2))
	set(ahn,getel(ancestor,3))
	set(person,indi(key))
	if (lt(length(tree),gen)) { /* make another generation if we need it */
	    list(generation)
	    enqueue(tree,generation)  /* Note:  can't skip a generation! */
	}
	set(generation,getel(tree,gen))  /* get the generation */
	enqueue(generation,ancestor)     /* put this ancestor on it */
	if (oldahn,lookup(ancestors,key)) {  /* if we have already done this ancestor ... */
	    setel(ancestor,7,oldahn)   /* mark it as a duplicate */
	} else {
	    setel(ancestor,7,0)	/* mark it as a non-duplicate */
	    insert(ancestors,key,ahn)  /* put it in the table of ancestors */
	    if (par,father(person)) {  /* and look for a father to enqueue */
		list(father)
		enqueue(father,key(par)) enqueue(father,add(gen,1)) enqueue(father,add(ahn,ahn))
		enqueue(plist,father)
		setel(ancestor,5,father)
	    }
	    if (par,mother(person)) {  /* and look for a mother to enqueue */
		list(mother)
		enqueue(mother,key(par)) enqueue(mother,add(gen,1)) enqueue(mother,add(ahn,ahn,1))
		enqueue(plist,mother)
		setel(ancestor,6,mother)
	    }
	}
    }
/* Make the geometry of the tree */
    call make_geometry()
/* Write the output */
    call write_tree()
}

proc make_geometry() {  /* figure out y positions of all the ancestors */
	list(tofix)
	set(gennum,length(tree))
	while (gennum) { /* for each generation, oldest generation first */
		set(generation,getel(tree,gennum))
		set(lasty,0)
		forlist(generation,ancestor,ancnum) { /* for each ancestor within the generation, patrilineal first */
			if(and(getel(ancestor,5),getel(ancestor,6))) { /* has father and mother */
				set(thisy,div(add(getel(getel(ancestor,5),4),getel(getel(ancestor,6),4)),2))
			} elsif (getel(ancestor,5)) { /* has father */
				set(thisy,getel(getel(ancestor,5),4))
			} elsif (getel(ancestor,6)) { /* has mother */
				set(thisy,getel(getel(ancestor,6),4))
			} else {
				set(thisy,add(lasty,deltay))
			}
			setel(ancestor,4,thisy)
			set(fix,add(lasty,deltay,neg(thisy)))
			if (gt(fix,0)) { /* too close to previous ancestor within the generation, fix this person */
				/* and all his/her ancestors */
				enqueue(tofix,ancestor)
				/* plus all parents of those persons below this one and their ancestors */
				set(found,0)
				forlist(generation,ancestor2,ancnum2) {
					if (found) {
						if (getel(ancestor2,5)) { enqueue(tofix,getel(ancestor2,5)) }
						if (getel(ancestor2,6)) { enqueue(tofix,getel(ancestor2,6)) }
					} elsif (eq(ancestor,ancestor2)) { set(found,1) }
				}
				while(fixee,dequeue(tofix)) {
					setel(fixee,4,add(fix,getel(fixee,4)))
					if (getel(fixee,5)) { enqueue(tofix,getel(fixee,5)) }
					if (getel(fixee,6)) { enqueue(tofix,getel(fixee,6)) }
				}
			}
			set(lasty,getel(ancestor,4))
		}
		decr(gennum)
	}
}

proc write_tree() { /* this procedure destroys (recycles?) the tree and all its generations */
	set(x,8)
	set(maxx,add(x,mul(deltax,length(tree))))
	set(maxy,0)
	forlist(tree,generation,gennum) { 
		set(thismaxy,getel(getel(generation,length(generation)),4))
		if (gt(thismaxy,maxy)) { set(maxy,thismaxy) }
	}
	set(maxy,add(maxy,deltay))
	"<div style=" qt "position:absolute; left:0; top:0;"
/* "width:" d(maxx) "px; height:" d(maxy) "px;" */
	qt ">\n"
	while(generation,dequeue(tree)) {
		while(ancestor,dequeue(generation)) {
			set(person,indi(getel(ancestor,1)))
	/* first write the person in a box */
			"<p class="
			if (male(person)) { "man" } else { "vrw" }
			" style=" qt "top:" d(getel(ancestor,4)) "px; left:" d(x) "px;"
			if (getel(ancestor,7)) { " border-style:dotted;" }
/*			elsif (not(or(father(person),mother(person)))) { " border-width:2px;" } */
			set(myclump,lookup(INDEXTABLE,getel(ancestor,1)))
			set(private,0)
			if (privacytern) { set(private,lookup(PRIVTABLE,getel(ancestor,1))) }
			qt "><a href=" qt "clump" d(myclump) ".html#" getel(ancestor,1) qt
			" title=" qt d(getel(ancestor,3)) ". " strxlat(html_xlat,fullname(person, 0, 1, 99))
			if (getel(ancestor,7)) {
				" (=" d(lookup(ancestors,getel(ancestor,1))) ")"
			} else {
			    if(not(private)) {
				set(paren,0)
				set(bdate,date(birth(person)))
				set(byear,year(birth(person)))
				if (not(byear)) {
					set(bdate,date(baptism(person)))
					set(byear,year(baptism(person)))
				}
				set(ddate,date(death(person)))
				set(dyear,year(death(person)))
				if (not(dyear)) {
					set(ddate,date(burial(person)))
					set(dyear,year(burial(person)))
				}
				if (or(byear,dyear)) {
					" ("
					call print_fix_year(bdate,byear)
					" - "
					call print_fix_year(ddate,dyear)
					")"
				}
			    }
			}
			qt " target=" qt "new" qt ">"
			strxlat(html_xlat,surname(person))
			"</a></p>\n"
	/* then draw any connectors to his/her parents */
			set(top,add(getel(ancestor,4),5))
			set(left,add(x,deltax,neg(18)))
			if(getel(ancestor,7)) { /* duplicate */
				if(or(father(person),mother(person))) { /* draw a short line */
					"<table class=h0 style=" qt "top:" d(top) "px; left:" d(left) "px;" qt
					"><tr><td></td></tr></table>\n"
				}
			} elsif(and(getel(ancestor,5),getel(ancestor,6))) { /* has father and mother */
				"<table class=h1 style=" qt "top:" d(top) "px; left:" d(left) "px;" qt
				"><tr><td></td></tr></table>\n"
				set(topdad,add(getel(getel(ancestor,5),4),5))
				set(topmom,add(getel(getel(ancestor,6),4),5))
				"<table style=" qt "top:" d(topdad) "px; left:" d(add(left,9))
				"px; height:" d(sub(topmom,add(topdad,2))) "px;" qt "><tr><td></td></tr></table>\n"
			} elsif (or(getel(ancestor,5),getel(ancestor,6))) { /* has one parent */
				"<table class=h2 style=" qt "top:" d(top) "px; left:" d(left) "px;" qt
				"><tr><td></td></tr></table>\n"
			}
		}
		set(x,add(x,deltax))
	}
	"</div>\n"
}

proc do_name(person,henry_list,marr)
{
    set(h,"")
    if (grouped_henry) {
	set(c,sub(first_comma,1)) /* one for the root symbol */
	forlist(henry_list,l,li) {
	    if (not(strcmp(trim(l,1),"s"))) {
		set(h,concat(h,".",l))
	    }
	    else {
		if (ge(c,comma_separation)) {
		    set(h,concat(h,","))
		    set(c,mod(c,comma_separation))
		}
		if (and(gt(strlen(l),1),gt(li,1))) {
		    set(h,concat(h,"(",l,")"))
		} else {
		    set(h,concat(h,l))
		}
	    }
	    incr(c)
	}
    } else {
	forlist(henry_list,l,li) { set(h,concat(h,l,".")) }
    }
    h " "
    if (person) { call href(person,neg(1)) } else { "<SPOUSE>" }
    if (l,lookup(written_people,key(person))) {
	" appears above as " l "\n"
    }
    else {
	if (person) { insert(written_people,key(person),h) }
	"\n"
    }
}

proc desc_sub(person,henry_list)
{
    call do_name(person,henry_list,0)
    set(nfam,nfamilies(person))
    set(chi,0)
    families(person,fam,sp,spi) {
	if (gt(nfam,1)) { push(henry_list,concat("s",d(spi))) }
	else { push(henry_list,"s") }
	call do_name(sp,henry_list,marriage(fam))
	set(junk,pop(henry_list))
	if (or(eq(generations,0),
	       lt(length(henry_list),generations))) {
	    children (fam,ch,famchi) {
		set(chi,add(1,chi))
		push(henry_list,d(chi))
		call desc_sub(ch,henry_list)
		set(junk,pop(henry_list))
	    }
	}
    }
}

func privacy(person) {
    if (living(person)) { return(1) }
    set(sib,person)
    while (sib,nextsib(sib)) { if (living(sib)) { return(1) } }
    set(sib,person)
    while (sib,prevsib(sib)) { if (living(sib)) { return(1) } }
    if (f,father(person)) { if (living(f)) { return(1) } }
    if (m,mother(person)) { if (living(m)) { return(1) } }
    return(0)
}

func living(person) {
    if (death(person)) { return(0) }
    if (burial(person)) { return(0) }
    if (b,birth(person)) {
	extractdate(b, da, mo, yr)
	if (gt(yr,hundred_years_ago)) { return(1) }
    }
    if (b,baptism(person)) {
	extractdate(b, da, mo, yr)
	if (gt(yr,hundred_years_ago)) { return(1) }
    }
    families(person,fam,spouse,nfam) {
	if (m,marriage(fam)) {
	    extractdate(m, day, mo, yr)
	    if (gt(yr,eighty_years_ago)) { return(1) }
	}
    }
    return(0)
}

proc create_index_file(desc_roots)
{
    list(initials)
    list(initialcounters)
    list(sortindex)

  getintmsg(sortit,"Sort the indexes? (0=no, 1=yes)")
  if (sortit) {

    print("sorting...")
    list(transindex)
    call translate(INDEX,transindex)
    call quicksort(transindex,sortindex)
    print("writing letter indices...")

    set(initial,"no-initial")
    set(counter,1)
    forlist(sortindex,sindex,counter)
    {
	set(me,indi(getel(INDEX,sindex)))
	set(myinitial,trim(strxlat(sort_xlat,trim(mysurname(me),1)),1))
	if (strcmp(myinitial,initial))
	{
	    if (strcmp(initial,"no-initial"))
	    {
		"</UL>\n"
		call html_trailer("",concat("Name%20list%20",initial))
		enqueue(initials, initial)
		enqueue(initialcounters, initialcounter)
		set(initial, myinitial)
	    }
	    else
	    {
		set(initial, myinitial)
	    }
	    set(initialcounter,0)
	    print("-", initial, "-")
	    set(fn, concat(PATH, "index", initial, ".html"))
	    if (separate_clumps) { newfile(fn, FB) }
	    call html_header(
		concat(INDEX_NAME,initial)
		, 0)
	    "<UL>\n"
	}
	"<LI>" call href(me,neg(1)) nl
	incr(initialcounter)
    }
    "</UL>\n"
    call html_trailer("",concat("Name%20list%20",initial))
    enqueue(initials, initial)
    enqueue(initialcounters, initialcounter)

    print("writing master_index...")
    set(fn, concat(PATH, "master_index", ".html"))
    if (separate_clumps) { newfile(fn, FB) }
    forlist(sortindex,sindex,counter) {
	set(me,indi(getel(INDEX,sindex)))
	"<LI>" call href(me,neg(1)) nl
    }

    print("writing main index...")
    set(fn, concat(PATH, "index.html"))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(INDEX_NAME, 0)

    "<P><IMG SRC=" qt "jim_s2.jpg" qt
    " ALIGN=RIGHT HEIGHT=419 WIDTH=284 ALT=" qt "Jim Eggert" qt ">\n"
    "This database contains the families of the ancestors of my children.\n"
    "Most of them are German, German-American,\n"
    "Syrian, and Syrian-American.\n"
    "This list contains about a twelfth of\n"
    "my entire genealogical database. If you would like to see more,\n"
    "please <A HREF="
    qt "mailto:Jim%20Eggert%20%3CEg%67ertJ%40verizon%2Enet%3E?subject=Genealogy%20query" qt
    ">send e-mail</A>."

    "<P>For more information about German genealogy in general, try the\n"
    "<A HREF=" qt "http://www.genealogy.net/gene/" qt
    ">German genealogy website</A>, where I manage\n"
    "the <A HREF=" qt "http://www.genealogy.net/gene/faqs/sgg.html" qt
    ">soc.genealogy.german FAQ</A>\n"
    "and the\n"
    "<A HREF=" qt
    "http://www.genealogy.net/gene/reg/NSAC/schaumburg-lippe.html" qt
    ">Schaumburg-Lippe</A> pages.\n"

    "<HR SIZE=4 NOSHADE>\n<P>Start with me <A HREF=" qt "clump1.html#I1" qt
    ">Eggert, James Robert</A> (1957-?)\n"

    "<P>Look at my <A HREF=" qt HREF "pedigreeg.html" qt
    "> ancestry chart.</A>\n"

    if (length(desc_roots)) {
      "<P>Here are some descendant reports:\n<UL>\n"
      forlist(desc_roots,desckey,dnum) {
	"<LI><A HREF=" qt HREF "onlyfamilydesc" desckey ".html" qt "> "
	mysurname(indi(desckey)) " Family Descendant List</A>\n"
      }
      "</UL>\n"
    }

    "<P>Examine my <A HREF=" qt
    "http://rsl.rootsweb.com/cgi-bin/rslsql.cgi?op=submitter&amp;user=eggertj"
    qt ">RootsWeb Surname List (RSL) entries</A> and my <A HREF="
    qt "http://worldconnect.rootsweb.com/cgi-bin/igm.cgi?db=eggertj" qt
    ">RootsWeb WorldConnect database</A>.\n"
    "<P>Here is my <A HREF=" qt "Eggert_Records.html" qt
    ">list of sources</A>.\n"
    "<P>There is also a <A HREF=" qt "Eggertr11.pdf" qt
    ">PDF file</A> (~400KB, 162 pages) of the entire ancestry.\n"
    "<P>Here are some of my <A HREF=" qt "special.html" qt
    ">special projects</A>.\n"

    indiset(baseset)
    addtoset(baseset,indi(root_key),1)
    indiset(addset)
    addtoset(addset,indi(root_key),1)
    set(generations,4)
    while(gt(generations,0)) {
      set(addset,parentset(addset))
      forindiset(addset,addperson,pval,pnum) {
	if (female(addperson)) { addtoset(baseset,addperson,1) }
      }
      decr(generations)
    }

    namesort(baseset)
    forindiset(baseset,person,pval,pnum) {
      if (eq(pnum,1)) {
	"<P>These are the base surnames in this ancestry:<BR>\n"
      }
      "<A HREF=" qt "clump" d(lookup(INDEXTABLE,key(person)))
      ".html#" key(person) qt ">" mysurname(person) "</A>"
      if (eq(pnum,sub(lengthset(baseset),1))) { ", and\n" }
      elsif (eq(pnum,lengthset(baseset)))   { ".\n" }
      else  { ",\n" }
    }
    "<P>You can also find surnames alphabetically by their first letter:<br>\n"
    set(first_dash,1)
    while (initial,dequeue(initials))
    {
	set(count,dequeue(initialcounters))
	if (first_dash) {
	    set(first_dash,0)
	} else {
	    " - "
	}
	"<A HREF=" qt HREF "index" initial ".html" qt ">"
	initial "</A>"
    }
    "\n"
    "<p>There are " d(length(INDEX))
    " main entries in this website, from "
    set(pcount,0)
    forindi(person,pnum) { set(pcount,pnum) }
    d(pcount) " in my database, last updated "
    dayformat(2) monthformat(6) dateformat(0)
    stddate(gettoday()) ".\n"

    "<HR SIZE=4 NOSHADE>\n"
    "<P ALIGN=" qt "LEFT" qt ">"
    "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=2 SUMMARY=" qt "FreeFind search box and RSS link" qt ">\n"
    "<TR>\n"
    "<TD ALIGN=CENTER BGCOLOR=" qt "#A5CEA5" qt "><B><A HREF=" qt
    "http://search.freefind.com/find.html?id=4752315" qt
    ">Search this site</A></B></TD>\n"
    "<TD>&nbsp;&nbsp;&nbsp;</TD>\n"
    "<TD ALIGN=CENTER BGCOLOR=" qt "#A5CEA5" qt ">Point your feedreader to my</TD>\n"
    "</TR>\n"
    "<TR>\n"

    "<TD ALIGN=CENTER BGCOLOR=" qt "#A5CEA5" qt "><FORM ACTION=" qt
    "http://search.freefind.com/find.html" qt " METHOD=" qt "GET" qt
    "><INPUT TYPE=" qt "HIDDEN" qt " NAME=" qt "id" qt " SIZE=" qt "-1" qt
    " VALUE=" qt "4752315" qt "><INPUT TYPE=" qt "HIDDEN" qt " NAME=" qt
    "pageid" qt " SIZE=" qt "-1" qt " VALUE=" qt "r" qt "><INPUT TYPE=" qt
    "HIDDEN" qt " NAME=" qt "mode" qt " SIZE=" qt "-1" qt " VALUE=" qt
    "ALL" qt "><INPUT TYPE=" qt "TEXT" qt " NAME=" qt "query" qt " SIZE="
    qt "19" qt ">\n"
    "<BR><INPUT TYPE=" qt "SUBMIT" qt " VALUE=" qt " Find " qt
    "><SMALL>powered by <A HREF="
    qt "http://www.freefind.com/" qt
    ">FreeFind</A></SMALL></FORM></TD>\n"
    "<TD></TD>\n"
    "<TD ALIGN=CENTER BGCOLOR=" qt "#A5CEA5" qt "><a title=" qt "RSS 2.0" qt
    " href=" qt "http://mysite.verizon.net/eggertj/rss.xml" qt " style=" qt
    "border:1px solid;border-color:#FC9 #630 #330 #F96;padding:0 3px;font:bold 10px verdana,sans-serif;color:#FFF;background:#F60;text-decoration:none;margin:0;" qt ">RSS</a></TD>\n"
    "</TR>\n"
    "</TABLE>\n"

    call html_trailer("<BR>\neggertjkey\n","Genealogy%20query")

/* make GENDEX index file */
/* */
    print("writing GENDEX...")
    set(fn, concat(PATH, "GENDEX.txt"))
    if (separate_clumps) { newfile(fn, FB) }
    forlist(sortindex,sindex,counter) {
	set(mykey,getel(INDEX,sindex))
	set(me,indi(mykey))
	set(private,lookup(PRIVTABLE,mykey))
	"clump" d(lookup(INDEXTABLE,mykey)) ".html#" mykey
	"|" strxlat(ISO8859_xlat,mysurname(me))
	"|" strxlat(ISO8859_xlat,mygivens(me))
	" /" strxlat(ISO8859_xlat,mysurname(me)) "/"
	"|"
	if (evt, birth(me)) {
	    if (not(private)) { date(evt) }
	    "|" strxlat(ISO8859_xlat,place(evt))
	} else { "|" }
	"|"
	if (evt, death(me)) {
	    if (not(private)) { date(evt) }
	    "|" strxlat(ISO8859_xlat,place(evt))
	} else { "|" }
	"|\n"
    }
/* */
  } else {
    print("writing master_index...")
    set(fn, concat(PATH, "master_index", ".html"))
    if (separate_clumps) { newfile(fn, FB) }
    forlist(INDEX,mykey,counter) {
	"<LI>" call href(indi(mykey),neg(1)) nl
    }
/* */
    print("writing GENDEX...")
    set(fn, concat(PATH, "GENDEX.txt"))
    if (separate_clumps) { newfile(fn, FB) }
    forlist(INDEX,mykey,counter) {
	set(me,indi(mykey))
	set(private,lookup(PRIVTABLE,mykey))
	"clump" d(lookup(INDEXTABLE,mykey)) ".html#" mykey
	"|" strxlat(ISO8859_xlat,mysurname(me))
	"|" strxlat(ISO8859_xlat,mygivens(me))
	" /" strxlat(ISO8859_xlat,mysurname(me)) "/"
	"|"
	if (evt, birth(me)) {
	    if (not(private)) { date(evt) }
	    "|" strxlat(ISO8859_xlat,place(evt))
	} else { "|" }
	"|"
	if (evt, death(me)) {
	    if (not(private)) { date(evt) }
	    "|" strxlat(ISO8859_xlat,place(evt))
	} else { "|" }
	"|\n"
    }
/* */
  }
    print("done\n")
}

proc start_clumpfile(clumpnum)
{
    print(" ", d(clumpnum))
    set(CURRENTCLUMPFILE, clumpnum)
    set(fn, concat(PATH, "clump", d(CURRENTCLUMPFILE), ".html"))
    if (separate_clumps) { newfile(fn, FB) }
    call html_header(TITLE, 0)
    "<HR SIZE=4 NOSHADE>\n"
}

proc end_clumpfile()
{
    "<A HREF=" qt HREF "index.html" qt "> [Home] </A>"
    call html_trailer("",concat("Genealogy%20query%20",d(CURRENTCLUMPFILE)))
}

proc write_indi(me)
{
    set(private,lookup(PRIVTABLE,key(me)))
    set(myclump,lookup(INDEXTABLE,key(me)))
    if (ne(myclump,CURRENTCLUMPFILE))
    {
	call end_clumpfile()
	call start_clumpfile(myclump)
    }
    "<H1><A NAME=" qt key(me) qt "><IMG SRC="
    if (male(me)) { male_gif } elsif (female(me)) { female_gif }
    else { unknown_gif }
    " ALIGN=" qt "MIDDLE" qt " ALT=" qt "[" upper(sex(me)) "]" qt
    ">\n"
    "&nbsp;&nbsp;&nbsp;&nbsp;"
    call print_name(me, 1) "</A></H1>\n"
    "<PRE>"
    nl
    if(e, birth(me))   { "Birth:     " privlong(e,private) nl }
    if(e, baptism(me)) { "Baptism:   " privlong(e,private) nl }
    if(e, death(me))   { "Death:     " privlong(e,private) nl }
    if(e, burial(me))  { "Burial:    " privlong(e,private) nl }
    nl
    if (f,father(me)) { "Father:    " call href(f,myclump) nl }
    if (m,mother(me)) { "Mother:    " call href(m,myclump) nl }
    set(nfam,nfamilies(me))
    families(me, fam, sp, nsp)
    {
	nl
	"Married"
	if (gt(nfam,1)) { "(" d(nsp) ") " } else { "    " }
	call href(sp,myclump)
	if(e, marriage(fam)) { "\n           " privlong(e,private) }
	fornodes(fnode(fam),thisnode) {
	    if (not(strcmp(tag(thisnode),"DIV")))
	    {
		if (not(private)) { ", Divorced" }
	    }
	}
	nl
	if(nchildren(fam))
	{
	    "Children:\n"
	    children(fam, ch, nch)
	    {
		rjt(nch, 5) ". "
		call href(ch,myclump) nl
	    }
	}
    }
    nl
    if (not(private)) { call print_notes(me) }
    "</PRE><HR SIZE=4 NOSHADE>\n"
}

func privlong(event,private) {
    if (private) { strxlat(html_xlat,place(event)) }
    else { strxlat(html_xlat,long(event)) }
/*    if (private) { place(event) }
    else { long(event) } */
}

proc print_notes(me)
{
    set(first, 1)
    fornodes( inode(me), node)
    {
	if (not(strcmp("NOTE", tag(node))))
	{
	    if(first) { "<EM>Notes: </EM>" nl nl set(first, 0) }
	    strxlat(html_xlat,value(node)) nl
/*	    value(node) nl */
	    fornodes(node, next)
	    {
		strxlat(html_xlat,value(next)) nl
/*		value(next) nl */
	    }
	    nl
	}
    }
    fornodes( inode(me), node)
    {
	if (not(strcmp("REFN", tag(node))))
	{
	    if(first) { "<EM>Notes: </EM>" nl nl set(first, 0) }
	    "SOURCE: " strxlat(html_xlat,value(node)) nl
/*	    "SOURCE: " value(node) nl */
	    nl
	}
    }
}

proc href(me,fromclump)
{
    if(me)
    {
	set(private,0)
	set(myclump,lookup(INDEXTABLE,key(me)))
	if (myclump)
	{
	    if (eq(fromclump,myclump))
	    {
		"<A HREF=" qt "#" key(me) qt ">"
	    }
	    else
	    {
		"<A HREF=" qt HREF "clump" d(myclump) ".html#" key(me) qt ">"
	    }
	    if (privacytern) { set(private,lookup(PRIVTABLE,key(me))) }
	}
	elsif (privacytern) { set(private,privacy(me)) }
	call print_name(me, 1)
	if (myclump) { "</A>" }
	" ("
	if (print_year_place(birth(me),baptism(me),"*",private)) {
	    set(j,print_year_place(death(me),burial(me)," +",private))
	} else {
	    set(j,print_year_place(death(me),burial(me),"+",private))
	}
	")"
    }
/*    else { "_____" } */
}

func print_year_place(event,secondevent,symbol,private)
{
    set(noyear,1)
    set(noplace,1)
    if (not(private)) {
	if (event) {
	    set(d, date(event))
	    set(y, year(event))
	    if (strlen(y)) { 
		symbol call print_fix_year(d,y) set(noyear,0)
	    }
	}
	if (noyear) {
	    if (secondevent) {
		set(d, date(secondevent))
		set(y, year(secondevent))
		if (strlen(y)) {
		    symbol call print_fix_year(d,y) set(noyear,0)
		}
	    }
	}
    }
    if (noyear) { set(space,symbol) } else { set(space," ") }
    if (event) {
	set(p, place(event))
	if (strlen(p)) { space strxlat(html_xlat,p) set(noplace,0) }
/*	if (strlen(p)) { space p set(noplace,0) } */
    }
    if (noplace) {
	if (secondevent) {
	    set(p, place(secondevent))
	    if (strlen(p)) { space strxlat(html_xlat,p) set(noplace,0) }
/*	    if (strlen(p)) { space p set(noplace,0) } */
	}
    }
    return(not(and(noyear,noplace)))
}

proc print_fix_year(d,y)
{
    if (index(d,"BEF",1)) { "&lt;" }
    if (index(d,"AFT",1)) { "&gt;" }
    if (index(d,"ABT",1)) { "c" }
    y
/* Handle PAF slash years */
    set(yp,index(d,y,1))
    set(d2,substring(d,add(yp,4),strlen(d)))
    if (d2) {
	if (eq(index(d2,"/",1),1)) {
	    substring(d2,1,5)
	}
    }
}

proc html_header(str, isindex)
{
    "<!DOCTYPE html PUBLIC " qt "-//W3C//DTD HTML 4.01 Transitional//EN" qt ">\n"
    "<HTML>\n"
    "<HEAD>\n"
    "<META HTTP-EQUIV=" qt "Content-Type" qt " CONTENT=" qt "text/html; charset=ISO-8859-1" qt ">\n"
    if(isindex) { "<ISINDEX>" nl }
    "<TITLE> " str " </TITLE>\n"
    "<link rel=" qt "shortcut icon" qt " href=" qt "favicon.ico" qt " type=" qt "image/x-icon" qt ">\n"
    "<link rel=" qt "icon" qt " href=" qt "favicon.ico" qt " type=" qt "image/x-icon" qt ">\n"
    "<link rel=" qt "alternate" qt " type=" qt "application/rss+xml" qt 
    " title=" qt "RSS" qt " href=" qt "http://mysite.verizon.net/eggertj/rss.xml" qt ">\n"
    "</HEAD>\n"
    "<BODY BACKGROUND=" background_gif ">\n"
    "<H2>" logo_gif "&nbsp;&nbsp;" str "</H2>\n"
 }

proc html_header_graphic(str, isindex)
{
    "<!DOCTYPE html PUBLIC " qt "-//W3C//DTD HTML 4.01 Transitional//EN" qt ">\n"
    "<html>\n"
    "<head>\n"
    "<meta http-equiv=" qt "Content-Type" qt " content=" qt "text/html; charset=ISO-8859-1" qt ">\n"
    "<meta name=" qt "Creator" qt " content=" qt "html_tableau.ll under LifeLines" qt ">\n"
    "<meta name=" qt "Subject" qt " content=" qt "Pedigree chart; ancestor tableau" qt ">\n"
    "<meta name=" qt "Version" qt " content=" qt "1.0" qt ">\n"
    "<meta name=" qt "Date" qt " content=" qt stddate(gettoday()) qt ">\n"
    "<title> " str " </title>\n"
    "<style type=" qt "text/css" qt ">\n"
    "<!--\n"
    "p          { font-family:'Arial'; font-size:7pt; position: absolute; width:60px; height:10px; left: 0; text-align: Center;border-style: solid; border-width: 1pt; margin-top: 0pt; margin-bottom: 1pt }\n"
    "p.man      { background-color:#C5D6FE}\n"
    "p.vrw      { background-color:#FEC7D0}\n"
    "table      { position: absolute; border: 1pt solid black; border-right: 0pt; border-spacing: 0; width: 9px }\n"
    "table.r    { border-color: red }\n"
    "table.h1   { border-left: 0pt; border-bottom: 0pt}\n"
    "table.h1r  { border-left: 0pt; border-bottom: 0pt; border-color: red}\n"
    "table.h2   { border-left: 0pt; border-bottom: 0pt; width:18px }\n"
    "table.h2r  { border-left: 0pt; border-bottom: 0pt; width:18px; border-color: red }\n"
    "table.h0   { border-left: 0pt; border-bottom: 0pt; width:5px }\n"
    "table.v    { border-right: 0pt; border-top:0pt; border-bottom: 0pt}\n"
    "a:link     {color:blue; font-weight:normal; text-decoration:none;}\n"
    "a:visited  {color:purple; font-weight:normal; text-decoration:none;}\n"
    "a:active   {color:red; font-weight:normal; text-decoration:none;}\n"
    "-->\n"
    "</style>\n"
    "<base target=" qt "_self" qt ">\n"
    "</head>\n"
    "<BODY BACKGROUND=" background_gif ">\n"
    "<H2>" logo_gif "&nbsp;&nbsp;Eggert Family Ancestry</H2>\n"
    "<p class=" qt "man" qt " style=" qt "top:160px; left:30px;" qt ">male</p>\n"
    "<p class=" qt "vrw" qt " style=" qt "top:180px; left:30px;" qt ">female</p>\n"
    "<div style=" qt "position:absolute; top:200px; left:10px; width:110px; height:44px; text-align:center" qt ">\n"
    "  <small>\n"
    "Hover over a name to see the full name.  Click on a name for more information.\n"
    "<br>\n"
    "  </small>\n"
    "</div>\n"
}


proc html_trailer(tag,subject)
{
    "<HR SIZE=4 NOSHADE>\n"
    "=Jim Eggert<BR>\n"
    "Email:&nbsp;&nbsp;<A HREF="
    qt "mailto:Jim%20Eggert%20%3CEg%67ertJ%40verizon%2Enet%3E"
    if (strlen(subject)) { "?subject=" subject }
    qt "><IMG SRC=" qt "email2.gif" qt " HEIGHT=17 WIDTH=117 ALIGN=TOP ALT="
    qt "Eggert J (all one word) at verizon dot net" qt "></A><BR>\n"
    "Home Page:&nbsp;&nbsp;<A HREF=" qt "http://mysite.verizon.net/eggertj/"
    qt ">http://mysite.verizon.net/eggertj/</A><BR>\n"
    "Copyright &copy; " year(gettoday())
    " by James R. Eggert, All Rights Reserved.\n"
    tag
    "</BODY>\n"
    "</HTML>\n"
}

proc html_trailer_graphic(tag,subject)
{
    "</body>\n"
    "</html>\n"
}

proc print_name (me, last)
{
    strxlat(html_xlat,fullname(me, 0, not(last), 45))
/*    if (last) {
	mysurname(me) ", " mygivens(me)
    } else {
	mygivens(me) " " mysurname(me)
    } */
/*    fullname(me, 0, not(last), 45) */
    fornodes(inode(me), node)
    {
	if (not(strcmp("TITL", tag(node)))) { set(n, node) }
    }
    if (n) { " " strxlat(html_xlat,value(n)) }
/*    if (n) { " " value(n) } */
}

func rjt(n, w)
{
    set(d, strlen(d(n)))
    if (lt(d, w))
	{ set(pad, trim("      ", sub(w, d))) }
    else
	{ set(pad, "") }
    return(concat(pad, d(n)))
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
print(".")
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

/* translate a whole key list via sort_xlat to a sortable list */
proc translate(listin,listout) {
    forlist(listin,pkey,i) {
	set (p,indi(pkey))
	enqueue(listout,
	  concat(strxlat(sort_xlat,mysurname(p))," ",
		 strxlat(sort_xlat,mygivens(p)), " ",
	  d(estimate_byear(p))))
    }
}

/* compare indis referred to by strings constructed as in translate() */
func compare(str1,str2) {
    return(strcmp(str1,str2))
}

/* compare indis referred to by keys */
func keycompare(pkey1,pkey2) {
    if(not(strcmp(pkey1,pkey2))) { return(0) }
    if (s,strcmp(strxlat(sort_xlat,mysurname(indi(pkey1))),
		 strxlat(sort_xlat,mysurname(indi(pkey2))))) { return(s) }
    if (s,strcmp(strxlat(sort_xlat,mygivens(indi(pkey1))),
		 strxlat(sort_xlat,mygivens(indi(pkey2))))) { return(s) }
    return(intcompare(estimate_byear(indi(pkey1)),estimate_byear(indi(pkey2))))
}

func intcompare(i1,i2) {
    if(lt(i1,i2)) { return(neg(1)) }
    if(eq(i1,i2)) { return(0) }
    return(1)
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
    insert(sort_xlat,"ö","oe")
    insert(sort_xlat,"ü","ue")
    insert(sort_xlat,"ä","ae")
    insert(sort_xlat,"ß","ss")
    insert(sort_xlat,"Ä","Ae")
    insert(sort_xlat,"Ö","Oe")
    insert(sort_xlat,"Ü","Ue")
    insert(sort_xlat,"ë","e")
    insert(sort_xlat,"ÿ","y")
    insert(sort_xlat,"é","e")
    insert(sort_xlat,"ñ","n~")
    insert(sort_xlat,"œ","oe")
    insert(sort_xlat,"<","")
    insert(sort_xlat,">","")

/* For the full list of HTML encodings for special characters, see
   http://info.cern.ch/hypertext/WWW/MarkUp/ISOlat1.html
*/
    insert(html_xlat,"ö","&ouml;")
    insert(html_xlat,"ü","&uuml;")
    insert(html_xlat,"ä","&auml;")
    insert(html_xlat,"ß","&szlig;")
    insert(html_xlat,"Ä","&Auml;")
    insert(html_xlat,"Ö","&Ouml;")
    insert(html_xlat,"Ü","&Uuml;")
    insert(html_xlat,"ë","&euml;")
    insert(html_xlat,"ÿ","&yuml;")
    insert(html_xlat,"é","&eacute;")
    insert(html_xlat,"è","&igrave;")
    insert(html_xlat,"´","`")
    insert(html_xlat,"&","&amp;")
    insert(html_xlat,"ñ","&ntilde;")
    insert(html_xlat,"œ","&oelig;")
    insert(html_xlat,"<","&lt;")
    insert(html_xlat,">","&gt;")

/* ISO 8859 translation for the GENDEX.txt file
*/
    insert(ISO8859_xlat,"ö","ˆ")
    insert(ISO8859_xlat,"ü","¸")
    insert(ISO8859_xlat,"ä","‰")
    insert(ISO8859_xlat,"ß","ﬂ")
    insert(ISO8859_xlat,"Ä","ƒ")
    insert(ISO8859_xlat,"Ö","÷")
    insert(ISO8859_xlat,"Ü","‹")
    insert(ISO8859_xlat,"ë","Î")
    insert(ISO8859_xlat,"ÿ","ˇ")
    insert(ISO8859_xlat,"é","È")
    insert(ISO8859_xlat,"è","ö")
    insert(ISO8859_xlat,"´","'")
    insert(ISO8859_xlat,"ñ","Ò")
    insert(ISO8859_xlat,"œ","ú")
}

proc init_years() {
    set(years_between_kids,2)
    set(mother_age,23)
    set(father_age,25)
    set(hundred_years_ago,sub(atoi(year(gettoday())),100))
    set(eighty_years_ago, sub(atoi(year(gettoday())),80))
}

func estimate_byear(person) {
    set(byear_est,0)
    if(byear,get_byear(person)) { return(byear) }
    set(older,person)
    set(younger,person)
    set(yeardiff,0)
    set(border,0)
    while (or(older,younger)) {
	set(older,prevsib(older))
	set(younger,nextsib(younger))
	set(yeardiff,add(yeardiff,years_between_kids))
	if (older) {
	    incr(border)
	    if (byear,get_byear(older)) {
		return(add(byear,yeardiff))
	    }
	}
	if (younger)  {
	    if(byear,get_byear(younger)) {
		return(sub(byear,yeardiff))
	    }
	}
    }
/* estimate from parents' marriage */
    set(my,0)
    if (m,marriage(parents(person))) { extractdate(m,bd,bm,my) }
    if (my) {
	return(add(add(my,mul(years_between_kids,border)),1))
    }
/* estimate from first marriage */
    families(person,fam,spouse,fnum) {
	if (gt(fnum,1)) { break() }
	if (m,marriage(fam)) { extractdate(m,bd,bm,my) }
	if (my) {
	    if (female(person)) { return(sub(my,mother_age)) }
	    else { return(sub(my,father_age)) }
	}
	children(fam,child,cnum) {
	    if (byear,get_byear(child)) {
		if (female(person)) {
		    return(sub(sub(byear,
				mul(sub(cnum,1),years_between_kids)),
					mother_age))
		}
		else {
		    return(sub(sub(byear,
				mul(sub(cnum,1),years_between_kids)),
				father_age))
		}
	    }
	}
    }
/* estimate from parents' birthyear */
    set(older,person) set(byear_addend,0)
    while(older,prevsib(older)) {
	set(byear_addend,add(byear_addend,years_between_kids))
    }
    if (byear,get_byear(mother(person))) {
	return(add(byear,mother_age,byear_addend))
    }
    if (byear,get_byear(father(person))) {
	return(add(byear,father_age,byear_addend))
    }
    return(0)
}

func get_byear(person) {
    set(byear,0)
    if (person) {
	if (b,birth(person)) { extractdate(b,day,month,byear) }
	if (byear) { return(byear) }
	if (b,baptism(person)) { extractdate(b,day,month,byear) }
    }
    return(byear)
}

func mysurname(person) {
    set(s,surname(person))
    if (not(strlen(s))) { set(s,"____") }
    if (not(strcmp(s,"<unknown>"))) { set(s,"____") }
    return(s)
}

func mygivens(person) {
    set(s,givens(person))
    if (not(strlen(s))) { set(s,"____") }
    return(s)
}

