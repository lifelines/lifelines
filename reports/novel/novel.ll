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
 *  The report uses 2 additional files as input.
 *      novel.head contains nroff headers and macros in an attempt to
 *                 separate formatting from the ll reporting.
 *      novel.intro is included at the beginning of the report and is where
 *                 you can put a general intoductory text.
 *
 *   Original code by Tom Wetmore, ttw@cbnewsl.att.com
 *   with modifications by Cliff Manis
 *   Extensively re-written by Phil Stringer P.Stringer@mcc.ac.uk
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
	copyfile("novel.head")
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

	copyfile("novel.intro")

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
        ".IN" nl() d(out) "."
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
        set(e,baptism(i))
        if(and(e,long(e))) {
		if(not(birth(i))) {".P" nl()}
		call fn0(i)
		" was christened" call wherewhen(e) "." nl()
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
                if (eq(0,strcmp("FILE", tag(node)))) {
			copyfile(value(node))
		} elsif (eq(0,strcmp("BIRT", tag(node)))) {
			set(null,0)
		} elsif (eq(0,strcmp("BURI", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("CHIL", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("CHR", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("CNAM", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("CONF", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("DEAT", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("DIVI", tag(node)))) {
			"The marriage ended in divorce." nl()
                } elsif (eq(0,strcmp("EDUC", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("FAMC", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("FAMS", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("HUSB", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("MARR", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("NAME", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("NOTE", tag(node)))) {
			set(null,0)
		} elsif (eq(0,strcmp("RESI", tag(node)))) {
			set(null,0)
		} elsif (eq(0,strcmp("RETI", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("OCCU", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("SEX", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("TEXT", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("WIFE", tag(node)))) {
			set(null,0)
                } elsif (eq(0,strcmp("WITN", tag(node)))) {
			set(null,0)
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
                if (eq(0,strcmp("ADDR", tag(subn)))) {
			set(null,0)
                } elsif (eq(0,strcmp("AGE", tag(subn)))) {
			set(null,0)
                } elsif (eq(0,strcmp("CORP", tag(subn)))) {
			set(null,0)
                } elsif (eq(0,strcmp("DATE", tag(subn)))) {
			set(null,0)
                } elsif (eq(0,strcmp("PERI", tag(subn)))) {
			set(null,0)
                } elsif (eq(0,strcmp("PLAC", tag(subn)))) {
			set(null,0)
                } elsif (eq(0,strcmp("SITE", tag(subn)))) {
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
