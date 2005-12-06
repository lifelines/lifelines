/* 
 * @progname       novel.ll
 * @version        none
 * @author         Wetmore, Manis, Stringer
 * @category       
 * @output         nroff
 * @description    
 *
 *   It will produce a report of all descendents and ancestors of a person
 *   in book form. It understands a wide variety of gedcom records and
 *   tries hard to produce a readable, personalised document.
 *
 *   It prints a sorts listing of names, at the end of the report
 *   of everyone in the report.  All NOTE and CONT lines will
 *   be printed in the this report.  This report will produced
 *   a paginated output.
 * 
 *   This report produces a nroff output, and to produce the
 *   output, use:  nroff -mm filename > filename.out
 *                 groff -mgm -Tascii filename >filename.out
 *                 groff -mgm filename >filename.ps       [PostScript output]
 *
 *  The report uses one additional file as input.
 *      novel.intro is included at the beginning of the report and is where
 *                 you can put a general intoductory text.  If you don't
 *                 provide this, it is skipped.  A prototype is provided
 *                 along with this report.
 *
 *   Original code by Tom Wetmore, ttw@cbnewsl.att.com
 *   with modifications by Cliff Manis
 *   Extensively re-written by Phil Stringer P.Stringer@mcc.ac.uk
 *   Modified by Stephen Dum to remove external file dependencies and
 *        to fix a y2k bug.
 *
 *   This report works only with the LifeLines Genealogy program
 *
 */
 
global(idex)
global(curgen)
global(glist)
global(ilist)
global(in)
global(out)
global(ftab)
global(sid)
global(lvd)
global(enqc)
global(enqp)
global(stack)
global(fac)			/* First item after children */
global(itab)
proc main () {
	getindi(indi)
	dayformat(2)
	monthformat(6)
	output_head()
	list(ilist)
	list(glist)
	list(stack)			/* To hold function return values */
	table(ftab)
	indiset(idex)
	table(sid)
	table(lvd)
	table(itab)
	enqueue(ilist,indi)
	enqueue(glist,0)
	set(curgen,0)
	set(out,1)
	set(in,2)
	".ds iN " name(indi) nl()
	".PH " qt() "''\\s+3\\fB" name(indi) sp() call fromto(indi) "\\s-3\\fR" qt() nl()

	if (test("f","novel.intro")) {
	    copyfile("novel.intro")
	}

	print ("Descendants") print(nl())
	".HU " qt() name(indi) " and " pn(indi,3) " descendants" qt() nl()
	set(enqc,1) set(enqp,0)
	call scan()

	print ("Ancestors") print(nl())
	".HU " qt() "The ancestors of " name(indi) qt() nl()
	set(curgen,0)
	set(enqc,0) set(enqp,1)
	call enqpar(indi)
	call scan()

	call prindex()
}

proc enqpar(indi) {
	set(dad,father(indi))
	if (dad) {
		set(g,sub(curgen,1))
		enqueue(ilist,dad)
		enqueue(glist,g)
		insert(sid,key(dad),in)
		set(in,add(in,1))
	}
	set(mom,mother(indi))
	if (mom) {
		set(g,sub(curgen,1))
		enqueue(ilist,mom)
		enqueue(glist,g)
		insert(sid,key(mom),in)
		set(in,add(in,1))
	}
}

proc scan () {
    while (indi,dequeue(ilist)) {
	print(name(indi)) print(nl())
        set(thisgen,dequeue(glist))
        if (ne(curgen,thisgen)) {
            ".GN " d(thisgen) nl()
            set(curgen,thisgen)
        }
	if (enqp) {
		call enqpar(indi)
	}
        ".IN" nl() d(out) ". "
        call longvitals(indi,1,1)
        set(out,add(out,1))
    }
}

proc longvitals(i,showc,showp) {
	if ( and(i,lookup(lvd,key(i))) ) {
	/*	call shortvitals(i)*/
		call nicename(i) "." nl()
	} else {
		set (fac,1)
        	"\\fB" call nicename(i) "\\fR." nl()
		insert(sid,key(i),out)
		insert(lvd,key(i),out)
		call add_to_ix(i)
		call dobirth(i,showp)
		call doeduc(i)
		call domarr(i,showc)
		call dooccu(i)
		call doresi(i)
		call donotes(inode(i),1)
		call dotext(inode(i),1)
		call othernodes(inode(i))
		call doreti(i)
		call dodeath(i)
	}
}

proc shortvitals(indi) {
        call nicename(indi)
        set(b,birth(indi)) set(d,death(indi))
        if (and(b,short(b))) { ", b. " short(b) }
        if (and(d,short(d))) { ", d. " short(d) }
	"." nl()
}

proc famvitals (indi,fam,spouse,nfam,showc) {
	if (eq(0,nchildren(fam))) {
		call firstname(indi)
		if (spouse) {
			" and " call firstname(spouse)
		} 
		" had no children"
		if (not(spouse)) {
			" from this marriage"
		}
		"." nl()
	} elsif (and(fam,lookup(ftab,key(fam)))) {
		set(par,indi(lookup(ftab,key(fam))))
		"Children of " call firstname(indi) " and " call firstname(spouse) " are shown "
		"under " call nicename(par) "." nl()
	} elsif (showc) {
		"Children of " call firstname(indi)
		if (spouse) {
			" and " call firstname(spouse)
		}
		":" nl()
		".VL 0.4i" nl()
        	insert(ftab,save(key(fam)),key(indi))
		children(fam,child,nchl) {
			".LI " roman(nchl) nl()
			set(childhaschild,0)
			families(child,cfam,cspou,ncf) {
				if(ne(0,nchildren(cfam))) { set(childhaschild,1) }
			}
			".CH " nl()
			if (and(enqc,childhaschild)) {
				call enqch(child)
                        	call shortvitals(child)
			} else {
				call longvitals(child,0,0)
			}
			set(fac,1)
		}
		".LE" nl()
		".IN" nl()
	} else {
		call firstname(indi)
		if (spouse) {
			" and " call firstname(spouse)
		}
		" had " card(nchildren(fam))
		if(eq(1,nchildren(fam))) {
			" child,"
			set(andn,0)
		} else {
			" children,"
			set(andn,sub(nchildren(fam),1))
		}

		children(fam,child,nchl) {
			" "
			call firstname(child)
			call doadopts(child)
			call add_to_ix(child)
			if(ne(nchl,nchildren(fam))) {
				if(eq(nchl,andn)) {
					" and"
				} else {
					","
				}
			}
		}
		"." nl()
	}
}

proc enqch (child) {
	enqueue(ilist,child)
	enqueue(glist,add(1,curgen))
	insert(sid,key(child),in)
	set (in, add (in, 1))
}

proc spousevitals (sp,fam) {
	if(e,marriage(fam)) {
		if (place(e)) {
			call wherewhen(e) ","
		}
	}
	" "
	call add_to_ix(sp)
	if (and(sp,lookup(sid,key(sp)))) {
		/*call shortvitals(sp)*/ call nicename(sp) "." nl()
	} else {
		call nicename(sp)
        	set(e,birth(sp))
        	if(and(e,long(e)))  { "," nl() "born" call wherewhen(e) }
        	set(e,death(sp))
        	if(and(e,long(e)))  { "," nl() pn(sp,1) " died" call wherewhen(e) }
		"." nl()
		call showparents(sp)
	}
}

proc showparents(sp) {
        set(dad,father(sp))
        set(mom,mother(sp))
        if (or(dad,mom)) {
                pn(sp,0) " "
		if (death(sp)) { "was the " } else { "is the " }
		if (male(sp))      { "son of " }
                elsif (female(sp)) { "daughter of " }
                else               { "child of " }
        	if (dad)          { call nicename(dad) }
        	if (and(dad,mom)) { nl() "and " }
        	if (mom)          { call nicename(mom) }
        	if (dad) { call add_to_ix(dad) }
        	if (mom) { call add_to_ix(mom) }
		set(nch,nchildren(parents(sp)))
		decr(nch)
		if (gt(nch,0)) {
			" who had " card(nch) " other "
			if (eq(1,nch)) {
				"child,"
				set(andn,0)
			} else {
				"children,"
				set(andn,sub(nch,1))
			}
			set(cp,0)
			children(parents(sp),child,nchl) {
				if (ne(key(child),key(sp))) {
					" "
					call firstname(child)
					call doadopts(child)
					call add_to_ix(child)
					set(cp,add(cp,1))
					if(ne(nch,cp)) {
						if(eq(cp,andn)) {
							" and"
						} else {
							","
						}
					}
				}
			}
			". " nl()
		}
		"." nl()
        }
}

proc dobirth(i,showp) {
        set(e,birth(i))
        if(and(e,long(e))) {
		".P" nl()
		call firstname(i)
		set(fac,0) 
		" was born" call wherewhen(e) "." nl()
	}
	if(showp) { call showparents(i) }
        set(e,get_baptism(i))
        if(and(e,long(e))) {
		if(not(birth(i))) {".P" nl()}
		call fn0(i)
		if (eqstr(tag(e),"BAPM")) { " was baptized" }
		elsif (eqstr(tag(e),"BAPL")) { " was baptized" }
		elsif (eqstr(tag(e),"CHR")) { " was christened" }
		elsif (eqstr(tag(e),"CHRA")) { " was christened" }
		call wherewhen(e) "." nl()
	}
}

proc domarr(i,showc) {
        set(j,1)
        families(i,f,s,n) {
		".P" nl()
		call fn0(i)
                if (or(not(s),marriage(f))) {
                        " married"
                } else {
                        " lived with"
                }
		if (ne(1,nfamilies(i))) { " " ord(j) ", " }
                set(j,add(j,1))
		if (s) {
			call spousevitals(s,f)
		} else {
			if (male(i)) {
				" but his wife's name is not known. "
			} else {
				" but her husband's name is not known. "
			}
			nl()
		}
		call dowitness(fnode(f))
		call donotes(fnode(f),1)
		call othernodes(fnode(f))
		call famvitals(i,f,s,n,showc)
		set(fac,1)
	}
}

proc dodeath(i) {
        set(e,death(i))
        if(and(e,long(e))) {
		".P" nl()
		call fn0(i)
		" died"
		call wherewhen(e) "." nl()
		call addtostack(e,"CAUS")
		if(not(empty(stack))) {
			"The cause of death was "
			dequeue(stack) "." nl()
		}
		call donotes(e,0)
	}
        set(e,burial(i))
	if(and(e,long(e))) {
		if(not(long(death(i)))) {".P" nl()}
		call fn0(i)
                if (p,place(e)) {
                        if( ne(0,index(upper(p),"CREMAT",1)) ) {
                                " was laid to rest"
                        } else {
                                " was buried"
                        }
                }
                else {
                        " was buried"
                }
		call wherewhen(e) "." nl()
		call donotes(e,0)
		call dotext(e,1)
	}
}

proc donotes(in,subpara) {
        fornodes(in, node) {
		if (eq(0,strcmp("NOTE", tag(node)))) {
			if (subpara) {
				".P" nl()
			}
			value(node) nl()
			call addtostack(node,"CONT")
			while(it,dequeue(stack)) {
				it nl()
			}
		}
        }
}

proc dotext(in,subpara) {
        fornodes(in, node) {
		if (eq(0,strcmp("TEXT", tag(node)))) {
			if (subpara) {
				".P" nl()
			}
			call addtostack(node,"SOUR")
			if(not(empty(stack))) {
				"The following information was found in "
				while(it,dequeue(stack)) {
					it nl()
				}
				".I :" nl()
				".P" nl()
			}
			value(node) nl()
			call addtostack(node,"CONT")
			while(it,dequeue(stack)) {
				it nl()
			}
			".R" nl()
		}
        }
}

proc dowitness(snode) {
	set(mult,0)
	call addtostack(snode,"WITN")
	if (not(empty(stack))) {
		"Witnessed by "
		while(it,dequeue(stack)) {
			if (mult) {
				" and "
			} else {
				set(mult,1)
			}
			it
		}
	"." nl()
	}
}

proc dooccu(in) {
	set(first,1)
	fornodes(inode(in), node) {
		if (eq(0,strcmp("OCCU", tag(node)))) {
			if(first) {
				".P" nl()
				call fn2(in)
				" occupation was "
				set(first,0)
			} else {
				"Then "
			}
			value(node)
			call wherewhen(node)
			"." nl()
		}
        }
}

proc doresi(in) {
	set(first,1)
	fornodes(inode(in), node) {
		if (eq(0,strcmp("RESI", tag(node)))) {
			if(first) {
				".P" nl()
				call fn0(in)
				" lived"
				set(first,0)
			} else {
				"Subsequently"
			}
			call wherewhen(node)
			"." nl()
		}
        }
}

proc doeduc(in) {
	set(first,1)
        fornodes(inode(in), node) {
		if (eq(0,strcmp("EDUC", tag(node)))) {
			if(first) {
				".P" nl()
				call fn0(in)
				" was educated"
				set(first,0)
			} else {
				"Also"
			}
			call wherewhen(node)
			"." nl()
		}
        }
}

proc doreti(in) {
        fornodes(inode(in), node) {
		if (eq(0,strcmp("RETI", tag(node)))) {
			".P" nl()
			call fn0(in)
			" retired"
			call wherewhen(node)
			"." nl()
		}
        }
}

/* Short version of adoption */
proc doadopts(in) {
        fornodes(inode(in), node) {
		if (eq(0,strcmp("ADOP", tag(node)))) {
			" (adopted)"
		}
	}
}

proc addtostack(stnode,ntype) {
	fornodes(stnode, subnode) {
		if (eq(0,strcmp(ntype, tag(subnode)))) {
			enqueue(stack,value(subnode))
		}
	}
}

proc addtostackc(stnode,ntype) {
	fornodes(stnode, subnode) {
		if (eq(0,strcmp(ntype, tag(subnode)))) {
			enqueue(stack,value(subnode))
			call addtostack(subnode,"CONT")
		}
	}
}

proc stackaddr(e) {
	call addtostackc(e,"ADDR")
}

proc stackplace(stnode) {
	fornodes(stnode, subnode) {
		if (eq(0,strcmp("PLAC", tag(subnode)))) {
			call stackaddr(subnode)
			enqueue(stack,value(subnode))
		}
	}
}

proc fromto(indi) {
	set(e,birth(indi))
	set(f,death(indi))
	if (or(year(e),year(f))) {
		"("
		if (year(e)) {year(e)} else { "?" }
		"-"
		year(f)
		")"
	}
}

proc when(e) {
	if(d,stddate(e)) {
		set(i,index(d," ",1))
		if(eq(0,i)) {
			" in "
		} elsif(eq(i,1)) {
			" in"
		} elsif(lt(i,4)) {
			" on "
		} else { " in " }
		d
	}
	call doperi(e)
	call addtostack(e,"AGE")
	if (not(empty(stack))) {
		", at the age of " dequeue(stack)
	}	
}

proc where(e) {
	call addtostack(e,"CORP")
	call addtostack(e,"SITE")
	call stackaddr(e)
	call stackplace(e)
	if (not(empty(stack))) {
		" at " dequeue(stack)
		while (elem,dequeue(stack)) {
			", "
			elem
		/*	if (not(empty(stack))) {
				", "
			}*/
		}
	}
}

proc wherewhen(e) {
	call where(e)
	call when(e)
}

proc whenwhere(e) {
	call when(e)
	call where(e)
}

proc doperi(node) {
	call addtostack(node,"PERI")
	if(not(empty(stack))) {
		" from "
		set(notfirst,0)
/*		if(not(getel(stack,2))) {
			dequeue(stack)
		} else {*/
			while(it,dequeue(stack)) {
				if(getel(stack,1)) {
					it ", "
				} else {
					if(notfirst) {"and " set(notfirst,1)}
					it
				}
			}
/*		}*/
	}
}

proc nicename(i) {
	if(eq(0,strlen(givens(i)))) { "____" } else { givens(i) }
	sp()
	if(surname(i)) {upper(surname(i))} else { "____" }
	if(sect,lookup(sid,key(i))) { 
		if(ne(sect,out)) {" [" d(sect) "]"}
	}
}

/* Print the firstname or He/She depending whether the fac flag is set */
proc fn0(i) {
	if (fac) {
		call firstname(i)
		set(fac,0)
	} else {
		pn(i,0)
	}
}

/* Print the firstname or His/Her depending whether the fac flag is set */
proc fn2(i) {
	if (fac) {
		call firstname(i) "'s"
		set(fac,0)
	} else {
		pn(i,2)
	}
}

proc firstname(i) {
	if (i) {
		call addtostack(inode(i),"CNAM")
		if (not(empty(stack))) { 
			dequeue(stack)
		} else {
			list(parts)
			extractnames(inode(i),parts,elems,sn)
			if(eq(1,elems)) {
				"____ " pop(parts)
			} else {
				set(nf,1)
				forlist(parts,it,n) {
					if(ne(sn,n)) {
						if(nf) { set(ans,it) set(nf,0) }
					/*	if( ne(0,index(it,qt(),1)) ) {
							set(ans,substring(it,2,sub(strlen(it),1)))
						}*/
					}
				}
				ans
			}
		}
	}
}

proc othernodes(i) {
        fornodes(i, node) {
		if (index(" BAPM BAPL BIRT BURI CHIL CHR CHRA CNAM CONF DEAT DIVI EDUC FAMC FAMS HUSB MARR NAME NOTE RESI RETI OBJE OCCU SEX TEXT WIFE WITN ",
		        concat(" ",upper(tag(node))," "),1)) {
			set(null,0)  /* lifelines noop */
                } elsif (eq(0,strcmp("FILE", tag(node)))) {
			copyfile(value(node))
                } elsif (eq(0,strcmp("DIVI", tag(node)))) {
			"The marriage ended in divorce." nl()
		} else {
			".P" nl()
                        tag(node) sp() value(node)
			call wherewhen(node) nl()
			call subnode(node)
                }
        }
}

proc subnode(i) {
	fornodes(i, subn) {
		if (index(" ADDR AGE CORP DATE PERI PLAC SITE ",
		        concat(" ",upper(tag(subn))," "),1)) {
			set(null,0)
		} else {
			".br" nl()
			tag(subn) sp() value(subn) nl()
			call subnode(subn)
		}
	}
}

proc prindex () {
	print("Index") print(nl())
	namesort(idex)
	monthformat(4)
	".IX" nl()
	forindiset(idex,indi,v,n) {
		".br" nl()
		fullname(indi,1,0,24)
		" "
		call fromto(indi) " "
		lookup(itab,key(indi))
		nl()
		set(tp,n) 
	}
	print(d(tp)) print(" individuals were mentioned in this report") print(nl()) 
	".P" nl() "There are " d(tp) " individuals mentioned in this report."
	nl()
}

proc add_to_ix(i) {
/*	print("IX ") print(name(i)) print(" ") print(d(out)) print(nl())*/
	addtoset(idex,i,d(out))
	if (l,lookup(itab,key(i))) {
/*		print(" - already got ") print(l) print(nl())*/
		insert(itab,key(i),save(concat(concat(l,","),d(out))))
	} else {
		insert(itab,key(i),save(d(out)))
	}
}

func get_baptism(indi) {
    list(ev)
    fornodes(indi,node) {
        if (index(" BAPM BAPL CHR CHRA ",concat(" ",upper(tag(node))," "),1)) {
	    return(node)
	}
     }
     return(0)
}

func get_tags(indi,str) {
    list(ev)
    fornodes(indi,node) {
        if (index(str,concat(" ",upper(tag(node))," "),1)) {
	    push(ev,node)
	}
     }
     return(ev)
}

func output_head()
{
    /* this is really ugly.  back slash is already overused in *roff code
     * but to include this here we have to escape the backslashs (that were
     * already escaped in the *roff code.  However, it is better here,
     * than the hassels of a separate file for the header info.
     */
    ".if t .pl 10.9i		\\\" Page length" nl()
    ".if n .pl 10.7i" nl()
    ".if t .ll 6.75i		\\\" Line length" nl()
    ".if n .ll 7.25i" nl()
    ".\\\".if t .lt 6.75i		\\\" Title length" nl()
    ".\\\".if n .lt 7.25i" nl()
    ".if t .lt 7.75i		\\\" Title length" nl()
    ".if n .lt 9.25i" nl()
    ".po 0.5i		\\\" Left margin" nl()
    ".ls 1			\\\" Line spacing" nl()
    ".\\\".nr Ej 1		\\\" New page before chapter headings" nl()
    ".nr Hb 1		\\\" Line break after all headings" nl()
    ".nr Hs 6		\\\" Blank line after all headings" nl()
    ".\\\".nr Hc 1		\\\" Centre chapter headings" nl()
    ".nr Hu 1		\\\" Un-numbered headings are at level 1" nl()
    ".nr Hi 1		\\\" Indent after head same as paras" nl()
    ".nr Pt 0		\\\" Don't indent paras" nl()
    ".nr Cl 6		\\\" Heads in table of contents up to level 6" nl()
    ".nr Yr \\n(yr+1900       \\\" the year for we are printing this" nl()
    ".if t .ds HF 3 3 3 3 3 3 2         \\\" Heading fonts" nl()
    ".ds HP +6 +6 +2 +2 +2 +2 +1  \\\" Heading point sizes" nl()
    ".ds pB " getproperty("user.fullname") nl()
    ".rm )k			\\\" Remove cut marks at top of page" nl()
    ".if \"\\nd\"0\" .nr m \\n(mo-1" nl()
    ".if \"\\nm\"0\" .ds mO January" nl()
    ".if \"\\nm\"1\" .ds mO February" nl()
    ".if \"\\nm\"2\" .ds mO March" nl()
    ".if \"\\nm\"3\" .ds mO April" nl()
    ".if \"\\nm\"4\" .ds mO May" nl()
    ".if \"\\nm\"5\" .ds mO June" nl()
    ".if \"\\nm\"6\" .ds mO July" nl()
    ".if \"\\nm\"7\" .ds mO August" nl()
    ".if \"\\nm\"8\" .ds mO September" nl()
    ".if \"\\nm\"9\" .ds mO October" nl()
    ".if \"\\nm\"10\" .ds mO November" nl()
    ".if \"\\nm\"11\" .ds mO December" nl()
    ".PF \"'\\fIProduced by \\*(pB\\fR'- \\\\\\\\nP -'\\fI\\n(dy \\*(mO \\n(Yr \\fR'\"" nl()
    ".PH" nl()
    ".de GN" nl()
    ".br" nl()
    ".ne 2i" nl()
    ".sp 2" nl()
    ".in 0" nl()
    ".ce" nl()
    ".if t \\s+3\\fHGENERATION \\\\$1\\fH\\s-3" nl()
    ".if n GENERATION \\\\$1" nl()
    ".." nl()
    ".de CH" nl()
    ".." nl()
    ".de IN" nl()
    ".sp" nl()
    ".in 0" nl()
    ".." nl()
    ".de IX" nl()
    ".SK" nl()
    ".HU Index" nl()
    "All the people mentioned in this report are given below. Please note that" nl()
    "the numbers printed after the name are not page numbers. They are the section" nl()
    "number(s) in which that person is mentioned. " nl()
    ".if t .2C" nl()
    ".." nl()
    ".de .I" nl()
    ".if t \\fI\\\\$1" nl()
    ".if n \\\\$1" nl()
    ".." nl()
    ".de .R" nl()
    ".if t \\fR" nl()
    ".." nl()
}
