/*
verify - a LifeLines database verification report program
	by Jim Eggert (eggertj@atc.ll.mit.edu)
	Version 1,  3 November 1992 (unreleased, first simple try)
	Version 2,  7 November 1992 (added lots of checks, verbose mode)
	Version 3, 12 November 1992 (minor bugfix, parameter tuning,
		added mrbbpm, unkgen, and hommar checks,
		and more heuristics for birth and marriage years)
	Version 4, 17 November 1992 (minor bugfix,
		added femhus, malwif, cbspan, lngwdw, oldunm checks)
	Version 5,  2 December 1992 (added mmnsnk check, improved morder)
	Version 6, 26 March 1993 (improved paternity checks,
		added hermaf check)
	Version 7,  2 September 1993 (added noprnt check,
		bug fix for parentless families)
	Version 8,  2 December 1994 (added mulpar check)
	Version 9, 28 April    1995 (added nofams check)

This LifeLines report program generates a text file which lists
exceptions to assertions or checks about the database.	There are two
forms of the output report, terse or verbose, selectable at runtime.

In the terse report, the assertions tested are labeled with a
six-character label at the beginning of each line, followed by the
instance of the exception.  The assertions tested, with the terse
syntax of the report output, are of four types, and are listed below.

The verbose report is more English-like, and requires less
explanation.  It contains the same information in the same order as
the terse report, with words added to make it read easier.  The lines
are often longer than 80 characters.

Before using this report, you may want to edit the first bunch of
set() calls in the main() procedure to adjust the various parameters.

Only one individual key is printed out per line, anyone else is
given by relation to this first person.

All age and date checks use only the year field of the date.  Thus
they are a bit inaccurate.  In particular, any use of ABT (about), BEF
(before), AFT (after), split dates, or similar devices will be lost on
this program.  Sorry.

Normal and Irish twins are indistinguishable.  For the cspace
assertion, intervening children with no known birthyear are treated
intelligently.	Unknown birthyears and deathyears are assumed equal to
the baptism years and burial years, respectively, if those are
available.  Unknown marriage years are estimated from the children's
birth or baptism years.

For a large database, this program will likely generate a lot of
output.	 I suggest that you go through this large output once to make
sure that everything is correct in the database.  After correcting any
errors, run this program again, creating a reference report, which
will still contain a lot of messages.  Then use diff to compare any
later reports to the reference report to catch any new errors.

You can sort the output report against the first field to sort against
the check type.	 This actually makes the report look nicer too.	 Then
you can sort subsets of the output against other fields to look for
the oldest person or the youngest father in the database, for example.

Parameters
Settable parameters are denoted by _parameter_ in this documentation.
These parameters are set in the first few lines of the procedure
main(), and can be changed by editing the program before running.

Assertions or checks
individual checks:
person's age at death is older than _oldage_
   oldage key name birth death age
person is baptized before birth
   bpbef  key person birth baptism
person dies before birth
   dbefb  key person birth death
person is buried before birth
   bubefb key person birth burial
person dies before baptism
   dbefbp key person baptism death
person is buried before baptism
   bubfbp key person baptism burial
person is buried before death
   bubefd key person death burial
person is baptised after birth year
   bpspac key person birth baptism
person is buried after death year
   buspac key person death burial
person has unkown gender
   unkgen key person
person has ambiguous gender
   hermaf key person
person has multiple parentage
   mulpar key person familynum familynum
person has no family pointers
   nofams key person

marriage checks:
person marries before birth
   unbmar key person birth marriage spouse
person marries after death
   dedmar key person death marriage spouse
person has more than _wedder_ spouses
   wedder key person nspouses
person marries someone more than _jundec_ years older
   jundec key person birth family spouse spouse_birth
person marries younger than _yngmar_
   yngmar key person age spouse
person marries older than _oldmar_
marriage out of order
   morder key person spouse
marriage before birth from previous marriage
   mrbbpm key person marriage spouse previous_birth
homosexual marriage
   hommar key person marriage spouse
person is a female husband
   femhus key person marriage
person is a male wife
   malwif key person marriage
person was a widow(er) longer than _lngwdw_ years
   lngwdw key person years
person lived more than _oldunm_ years and never married
   oldunm key person years
person has multiple marriages, this one with no spouse and no children
   mmnsnk key person family
person has same surname as spouse
   samnam key person marriage spouse

parentage checks:
mother has more than _fecmom_ children
   fecmom key person nkids nfamilies
mother is older than _oldmom_ at time of birth of child
   oldmom key person age familynum childnum child
child is born before mother
   unbmom key person birth familynum childnum child child_birth
mother is younger than _yngmom_
   yngmom key person age familynum childnum child
mother is dead at birth of child
   dedmom key person death familynum childnum child birth
same as above, but for father
   [fecdad, olddad, unbdad, yngdad, deddad]
child doesn't inherit father's surname
   nonpat key person familynum childnum child

children checks:
child is born out of order with respect to a previous child
   corder key person familynum childnum child child_birth prev_child_birth
child is born in the same year as a previous child
   ctwins key person familynum childnum child child_birth
child is born more than _cspace_ years after previous child
   cspace key person familynum childnum child birthspace
children's births span more than _cbspan_ years
   cbspan key person birthspan

family checks:
family has no parents
   noprnt fkey firstchild nchildren

*/

global(birthyear)  /* calculated by get_birthyear */

proc get_birthyear(someone)
{
    set(birthyear,0)
    if (bth,birth(someone)) {
	extractdate(bth,birthday,birthmonth,birthyear)
    }
    if (eq(birthyear,0)) {
	if (bap,baptism(someone)) {
	    extractdate(bap,bapday,bapmonth,birthyear)
	}
    }
}


proc main ()
{
/* Main settable parameters */
    set(oldage,90)  /* maximum approximate age */
    set(jundec,30)  /* maximum husband-wife age difference */
    set(yngmar,17)  /* minimum age to marry */
    set(oldmar,50)  /* maximum age to marry */
    set(fecmom,11)  /* maximum number of children for a woman */
    set(oldmom,49)  /* maximum age for a woman to bear a child */
    set(yngmom,17)  /* minimum age for a woman to bear a child */
    set(fecdad,15)  /* maximum number of children for a man */
    set(olddad,65)  /* maximum age for a man to father a child */
    set(yngdad,18)  /* minimum age for a man to father a child */
    set(wedder,3)   /* maximum number of spouses for a person */
    set(cspace,8)   /* maximum number of years between children */
    set(cbspan,25)  /* maximum span of years for all children */
    set(nonpat,0)   /* 0 = compare child=father surnames by Soundex code,
		       1 = require strict surname equality */
    set(oldunm,99)  /* maximum age at death for unmarried person */
    set(lngwdw,30)  /* maximum number of consecutive years of widowhood */

    getintmsg(verbose,"Enter 0 for terse, 1 for verbose output")
    if (verbose) {
	set(oldagestr,"Old age ")
	set(namestr," ")
	set(bornstr," born ")
	set(bapstr," baptized ")
	set(buriedstr," buried ")
	set(diedstr," died ")
	set(agestr," age ")
	set(dbefbstr,"Died before birth ")
	set(bpbefbstr,"Baptized before birth ")
	set(bpspacstr,"Baptized late ")
	set(bubefdstr,"Buried before death ")
	set(buspacstr,"Buried late ")
	set(dbefbpstr,"Death before baptism ")
	set(bubfbpstr,"Buried before baptism ")
	set(bubefbstr,"Buried before birth ")
	set(wedderstr,"Married often ")
	set(marriedstr," married ")
	set(timesstr," times")
	set(momstr,"mother ")
	set(dadstr,"father ")
	set(oldstr,"Old ")
	set(unbstr,"Unborn ")
	set(deadstr,"Dead ")
	set(yngstr,"Young ")
	set(fecundstr,"Fecund ")
	set(hadstr," had ")
	set(kidsinstr," children in ")
	set(familystr," family")
	set(familiesstr," families")
	set(jundecstr,"June-December marriage ")
	set(unbmarstr,"Married before birth ")
	set(dedmarstr,"Married after death ")
	set(yngmarstr,"Young marriage ")
	set(oldmarstr,"Old marriage ")
	set(marriedagestr," married at age ")
	set(tostr," to ")
	set(morderstr,"Marriage out of order ")
	set(familynostr," family ")
	set(illegstr,"Possible illegitimate birth ")
	set(corderstr,"Child out of order ")
	set(childnostr," child ")
	set(prevbornstr," previous child born ")
	set(ctwinstr,"Possible twin ")
	set(cspacestr,"Widely spaced births ")
	set(laterstr," years later")
	set(nonpatstr,"Nonpatronymic inheritance ")
	set(mrbbpmstr,"Marriage before birth from previous family ")
	set(prevmarbirthstr," previous birth ")
	set(unkgenstr,"Unknown gender ")
	set(hermafstr,"Ambiguous gender ")
	set(mulparstr,"Multiple parentage ")
	set(nofamsstr,"No family ")
	set(parentstr,"parent ")
	set(hommarstr,"Homosexual marriage ")
	set(femhusstr,"Female husband ")
	set(malwifstr,"Male wife ")
	set(cbspanstr,"Widely spanning births ")
	set(childrenspanstr," children's births span ")
	set(yearsstr," years")
	set(oldunmstr,"Old and unmarried ")
	set(diedunmarriedstr," died unmarried aged ")
	set(lngwdwstr,"Long widowhood ")
	set(waswidowstr," was a widow ")
	set(waswidowerstr," was a widower ")
	set(beforefamilynostr," before family ")
	set(mmnsnkstr,"Multiple marriages no spouse no kids ")
	set(samnamstr,"Husband and wife with same surname ")
	set(noprntstr,"Family with no parents ")
	set(firstchildstr," first child ")
	set(numchildrenstr," of ")
	set(nl,".\n")
    }
    else {
	set(oldagestr,"oldage ")
	set(namestr," ")
	set(bornstr," ")
	set(bapstr," ")
	set(buriedstr," ")
	set(diedstr," ")
	set(agestr," ")
	set(dbefbstr, "dbefb  ")
	set(bpbefbstr,"bpbefb ")
	set(bpspacstr,"bpspac ")
	set(bubefdstr,"bubefd ")
	set(buspacstr,"buspac ")
	set(dbefbpstr,"dbefbp ")
	set(bubfbpstr,"bubfbp ")
	set(bubefbstr,"bubefb ")
	set(wedderstr,"wedder ")
	set(marriedstr," ")
	set(timesstr,"")
	set(momstr,"mom ")
	set(dadstr,"dad ")
	set(oldstr,"old")
	set(unbstr,"unb")
	set(deadstr,"ded")
	set(yngstr,"yng")
	set(fecundstr,"fec")
	set(hadstr," ")
	set(kidsinstr," ")
	set(familystr,"")
	set(familiesstr,"")
	set(jundecstr,"jundec ")
	set(unbmarstr,"unbmar ")
	set(dedmarstr,"dedmar ")
	set(yngmarstr,"yngmar ")
	set(oldmarstr,"oldmar ")
	set(marriedagestr," ")
	set(tostr," ")
	set(morderstr,"morder ")
	set(familynostr," ")
	set(illegstr,"illeg  ")
	set(corderstr,"corder ")
	set(childnostr," ")
	set(prevbornstr," ")
	set(ctwinstr,"ctwins ")
	set(cspacestr,"cspace ")
	set(laterstr,"")
	set(nonpatstr,"nonpat ")
	set(mrbbpmstr,"mrbbpm ")
	set(prevmarbirthstr," ")
	set(unkgenstr,"unkgen ")
	set(hermafstr,"hermaf ")
	set(mulparstr,"mulpar ")
	set(nofamsstr,"nofams ")
	set(parentstr,"par ")
	set(hommarstr,"hommar ")
	set(femhusstr,"femhus ")
	set(malwifstr,"malwif ")
	set(cbspanstr,"cbspan ")
	set(childrenspanstr," ")
	set(yearsstr,"")
	set(oldunmstr,"oldunm ")
	set(diedunmarriedstr," ")
	set(lngwdwstr,"lngwdw ")
	set(beforefamilynostr,"")
	set(waswidowstr," ")
	set(waswidowerstr," ")
	set(mmnsnkstr,"mmnsnk ")
	set(samnamstr,"samnam ")
	set(noprntstr,"noprnt ")
	set(firstchildstr," ")
	set(numchildrenstr," ")
	set(nl,"\n")
    }

    forindi(person, number) {
	set(idstr,save(concat(concat(key(person),namestr),
			 name(person))))
/* individual checks */
	set(byear,0)
	if (bth,birth(person)) {
	    extractdate(bth,bday,bmonth,byear)
	}
	set(bapyear,0)
	if (bap,baptism(person)) {
	    extractdate(bap,bapday,bapmonth,bapyear)
	}
	set(dyear,0)
	if (dth,death(person)) {
	    extractdate(dth,dday,dmonth,dyear)
	}
	set(buryear,0)
	if (bur,burial(person)) {
	    extractdate(bur,burday,burmonth,buryear)
	}
	if (and(byear,bapyear)) {
	    if (gt(byear,bapyear)) {
		bpbefbstr idstr
		bornstr d(byear) bapstr d(bapyear) nl
	    }
	    if (lt(byear,bapyear)) {
		bpspacstr idstr
		bornstr d(byear) bapstr d(bapyear) nl
	    }
	}
	if (and(dyear,buryear)) {
	    if (gt(dyear,buryear)) {
		bubefdstr idstr
		diedstr d(dyear) buriedstr d(buryear) nl
	    }
	    if (lt(dyear,buryear)) {
		buspacstr idstr
		diedstr d(dyear) buriedstr d(buryear) nl
	    }
	}
	if (and(dyear,gt(byear,dyear))) {
	    dbefbstr idstr bornstr
	    d(byear) diedstr d(dyear) nl
	}
	if (and(dyear,gt(bapyear,dyear))) {
	    dbefbpstr idstr bapstr
	    d(bapyear) diedstr d(dyear) nl
	}
	if (and(buryear,gt(bapyear,buryear))) {
	    bubfbpstr idstr bapstr
	    d(bapyear) buriedstr d(buryear) nl
	}
	if (and(buryear,gt(byear,buryear))) {
	    bubefbstr idstr bornstr
	    d(byear) buriedstr d(buryear) nl
	}

	if (eq(byear,0)) { set(byear,bapyear) } /* guess baptism = birth */
	if (eq(dyear,0)) { set(dyear,buryear) } /* guess burial = death */
	if (and(byear,dyear)) { set(ageatdeath,sub(dyear,byear)) }
	else { set(ageatdeath,0) }
	if (gt(ageatdeath,oldage)) {
	    oldagestr idstr
	    bornstr d(byear) diedstr d(dyear)
	    agestr d(ageatdeath) nl
	}

/* gender checks */
	if (female(person)) {
	    set(parstr,momstr)
	    set(oldpar,oldmom)
	    set(yngpar,yngmom)
	    set(fecpar,fecmom)
	    set(waswidstr,waswidowstr)
	}
	if (male(person)) {
	    set(parstr,dadstr)
	    set(oldpar,olddad)
	    set(yngpar,yngdad)
	    set(fecpar,fecdad)
	    set(waswidstr,waswidowerstr)
	}
	if (not(or(female(person),male(person)))) {
	    unkgenstr idstr nl
	    set(parstr,parentstr)
	    set(oldpar,olddad)
	    set(yngpar,yngdad)
	    set(fecpar,fecdad)
	    set(waswidstr,waswidowstr)
	}
	if (and(male(person),female(person))) {
	    hermafstr idstr nl
	    set(parstr,parentstr)
	    set(oldpar,olddad)
	    set(yngpar,yngdad)
	    set(fecpar,fecdad)
	    set(waswidstr,waswidowstr)
	}

/* multiple parentage check */
	set(nfamc,0)
	set(famstr,"")
	fornodes(inode(person),node) {
	    if (not(strcmp(tag(node),"FAMC"))) {
		set(nfamc,add(nfamc,1))
		if (eq(nfamc,1)) { set(famstr,save(value(node))) }
		elsif (eq(nfamc,2)) { mulparstr idstr famstr }
		if (ge(nfamc,2)) { " " value(node) }
	    }
	}
	if (gt(nfamc,1)) { nl }

/* no families check */
	if (and(not(parents(person)), eq(0,nfamilies(person)))) {
	    nofamsstr idstr nl
	}

	set(nkids,0)

/* marriage checks */
	set(nfam,nfamilies(person))
	if (gt(nfam,wedder)) {
	    wedderstr idstr
	    marriedstr d(nfam) timesstr nl
	}
	if (and(gt(ageatdeath,oldunm),eq(nfam,0))) {
	    oldunmstr idstr diedunmarriedstr d(ageatdeath) yearsstr nl
	}
	set(first_cbyear,99999)
	set(last_cbyear,0)
	set(prev_cbyear,0)
	set(prev_cbyfnum,0)
	set(prev_cbyind,0)
	set(prev_maryear,0)
	set(prev_sdyear,0)
	families(person,fam,spouse,fnum) {
	    if (eq(strcmp(sex(person),sex(spouse)),0)) {
		hommarstr idstr familynostr d(fnum)
		namestr name(spouse) nl
	    }
	    if (and(eq(person,husband(fam)),female(person))) {
		femhusstr idstr familynostr d(fnum) nl
	    }
	    if (and(eq(person,wife(fam)),male(person))) {
		malwifstr idstr familynostr d(fnum) nl
	    }
	    if (spouse) {
		if (and(male(person),
			not(strcmp(surname(person),surname(spouse))))) {
		    samnamstr idstr familynostr d(fnum)
		    namestr name(spouse) nl
		}
	    }
	    if (and(byear,spouse)) {
		call get_birthyear(spouse)
		if (gt(sub(birthyear,byear),jundec)) {
		    jundecstr idstr
		    bornstr d(byear) familynostr d(fnum)
		    namestr name(spouse)
		    bornstr d(birthyear) nl
		}
	    }
	    set(sdyear,0)
	    if (sdth,death(spouse)) {
		extractdate(sdth,sdthday,sdmonth,sdyear)
	    }
	    if (eq(sdyear,0)) {
		if (sbur,burial(spouse)) {
		    extractdate(sbur,sburday,sburmonth,sdyear)
		}
	    }
	    set(maryear,0)
	    if (mar,marriage(fam)) {
		extractdate(mar,marday,marmonth,maryear)
	    }
	    if (eq(maryear,0)) { /* estimate marriage year */
		children(fam,child,cnum) {
		    if (eq(maryear,0)) {
			call get_birthyear(child)
			if (birthyear) {
			    set(maryear,sub(birthyear,cnum))
			}
		    }
		}
	    }
	    if (or(and(maryear,lt(maryear,prev_maryear)),
		       and(sdyear,lt(sdyear,prev_maryear)))) {
		morderstr idstr
		tostr name(spouse) nl
	    }
	    if (maryear) {
		if (byear) {
		    set(marage,sub(maryear,byear))
		    if (lt(marage,0)) {
			unbmarstr idstr
			bornstr d(byear) marriedstr d(maryear)
			tostr name(spouse) nl
		    }
		    elsif (lt(marage,yngmar)) {
			yngmarstr idstr
			marriedagestr d(marage) tostr
			name(spouse) nl
		    }
		    elsif (gt(marage,oldmar)) {
			oldmarstr idstr
			marriedagestr d(marage) tostr
			name(spouse) nl
		    }
		}
		if (and(dyear,gt(maryear,dyear))) {
		    dedmarstr idstr
		    diedstr d(dyear) marriedstr d(maryear)
		    tostr name(spouse) nl
		}
		if (gt(prev_cbyear,maryear)) {
		    mrbbpmstr idstr
		    marriedstr d(maryear)
		    tostr name(spouse)
		    prevmarbirthstr d(prev_cbyear) nl
		}
		set(prev_maryear,maryear)
	    }
	    else { set(maryear,prev_maryear) }
	    if (and(maryear,prev_sdyear)) {
		set(wdwyear,sub(maryear,prev_sdyear))
		if (gt(wdwyear,lngwdw)) {
		    lngwdwstr idstr waswidstr
		    d(wdwyear) yearsstr
		    beforefamilynostr d(fnum) nl
		}
	    }
	    if (and(eq(fnum,nfam),and(dyear,sdyear))) {
		set(wdwyear,sub(dyear,sdyear))
		if (gt(wdwyear,lngwdw)) {
		    lngwdwstr idstr waswidstr
		    d(wdwyear) yearsstr nl
		}
	    }
	    if (and(and(gt(nfam,1),not(spouse)),eq(nchildren(fam),0))) {
		mmnsnkstr idstr familynostr d(fnum) nl
	    }
	    children(fam,child,cnum) {
		set(nkids,add(nkids,1))
		call get_birthyear(child)
		set(cbyear,birthyear)
		if (and(cbyear,lt(cbyear,first_cbyear)))
		    { set(first_cbyear,cbyear) }
		if (gt(cbyear,last_cbyear)) { set(last_cbyear,cbyear) }
/* parentage checks */
		if (and(byear,cbyear)) {
		    set(bage,sub(cbyear,byear))
		    if (gt(bage,oldpar)) {
			oldstr parstr idstr
			agestr d(bage) familynostr d(fnum)
			childnostr d(cnum) namestr
			name(child) nl
		    }
		    elsif (lt(bage,0)) {
			unbstr parstr idstr
			bornstr d(byear)
			familynostr d(fnum) childnostr d(cnum)
			namestr name(child)
			bornstr d(cbyear) nl
		    }
		    elsif (lt(bage,yngpar)) {
			yngstr parstr idstr
			agestr d(bage)
			familynostr d(fnum) childnostr d(cnum)
			namestr name(child) nl
		    }
		}
		if (and(dyear,gt(cbyear,dyear))) {
		    deadstr parstr idstr
		    diedstr d(dyear)
		    familynostr d(fnum) childnostr d(cnum)
		    namestr name(child)
		    bornstr d(cbyear) nl
		}
		if (male(person)) {
		    if (or(and(eq(nonpat,0),
			   strcmp(save(soundex(person)),
				  save(soundex(child)))),
			   and(eq(nonpat,1),
			       strcmp(save(surname(person)),
				      save(surname(child)))))) {
			nonpatstr idstr familynostr d(fnum)
			childnostr d(cnum) namestr
			name(child) nl }
		}
/* children checks */
		if (cbyear) {
		    set(main_parent,or(female(person),not(wife(fam))))
		    if (main_parent) {
			if (gt(maryear,cbyear)) {
			    illegstr idstr
			    familynostr d(fnum) marriedstr d(maryear)
			    childnostr d(cnum) namestr
			    name(child)
			    bornstr d(cbyear) nl
			}
		    }
		    if (and(prev_cbyear,
			    or(main_parent,ne(fnum,prev_cbyfnum)))) {
			if (gt(prev_cbyear,cbyear)) {
			    corderstr idstr
			    familynostr d(fnum) childnostr d(cnum)
			    namestr name(child)
			    bornstr d(cbyear) prevbornstr d(prev_cbyear) nl
			}
			elsif (eq(cbyear,prev_cbyear)) {
			    ctwinstr idstr
			    familynostr d(fnum) childnostr d(cnum)
			    namestr name(child)
			    bornstr d(cbyear) nl
			}
			elsif (gt(cbyear,
				    add(prev_cbyear,
				     mul(cspace,sub(nkids,prev_cbyind))))) {
			    cspacestr idstr
			    familynostr d(fnum) childnostr d(cnum)
			    namestr name(child)
			    bornstr d(sub(cbyear,prev_cbyear)) laterstr nl
			}
		    }
		    set(prev_cbyear,cbyear)
		    set(prev_cbyind,nkids)
		    set(prev_cbyfnum,fnum)
		}
	    }
	}
	set(prev_sdyear,sdyear)

/* other parentage checks */
	set(cbdiff,sub(last_cbyear,first_cbyear))
	if (gt(cbdiff,cbspan)) {
	    cbspanstr idstr childrenspanstr d(cbdiff) yearsstr nl
	}
	if (gt(nkids,fecpar)) {
	    fecundstr parstr idstr
	    hadstr d(nkids) kidsinstr d(nfam)
	    if (eq(nfam,1)) { familystr } else { familiesstr } nl
	}
    }
/* handle families with no parents */
    forfam(fam,fnum) {
	if (not(or(husband(fam),wife(fam)))) {
	    set(first_cbyear,99999)
	    set(last_cbyear,0)
	    set(prev_cbyear,0)
	    set(prev_cbyind,0)
	    set(prev_maryear,0)
	    set(maryear,0)
	    if (mar,marriage(fam)) {
		extractdate(mar,marday,marmonth,maryear)
	    }
	    children(fam,child,cnum) {
		if (eq(cnum,1)) { set(firstchild,child) }
		call get_birthyear(child)
		set(cbyear,birthyear)
		if (and(cbyear,lt(cbyear,first_cbyear)))
		    { set(first_cbyear,cbyear) }
		if (gt(cbyear,last_cbyear)) { set(last_cbyear,cbyear) }
		if (cbyear) {
		    if (gt(maryear,cbyear)) {
			illegstr
			key(fam) marriedstr d(maryear)
			childnostr d(cnum) namestr
			name(child)
			bornstr d(cbyear) nl
		    }
		    if (prev_cbyear) {
			if (gt(prev_cbyear,cbyear)) {
			    corderstr
			    key(fam) childnostr d(cnum)
			    namestr name(child)
			    bornstr d(cbyear) prevbornstr d(prev_cbyear) nl
			}
			elsif (eq(cbyear,prev_cbyear)) {
			    ctwinstr
			    key(fam) childnostr d(cnum)
			    namestr name(child)
			    bornstr d(cbyear) nl
			}
			elsif (gt(cbyear,
				    add(prev_cbyear,
				     mul(cspace,sub(cnum,prev_cbyind))))) {
			    cspacestr
			    key(fam) childnostr d(cnum)
			    namestr name(child)
			    bornstr d(sub(cbyear,prev_cbyear)) laterstr nl
			}
		    }
		    set(prev_cbyear,cbyear)
		    set(prev_cbyind,cnum)
		}
	    }
	    set(cbdiff,sub(last_cbyear,first_cbyear))
	    if (gt(cbdiff,cbspan)) {
		cbspanstr key(fam)
		childrenspanstr d(cbdiff) yearsstr nl
	    }
	    noprntstr key(fam)
	    firstchildstr key(firstchild) namestr name(firstchild)
	    numchildrenstr d(cnum) nl
	}
    }
}
