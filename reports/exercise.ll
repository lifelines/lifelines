/*
 * @progname       exercise
 * @version        0.86 (2001/12/28)
 * @author         Perry Rapp
 * @category       test
 * @output         mixed
 * @description    Perry's test program.

Exercises lots of functions
cycling through most of the database
(this was adapted from my scrubLiving report)
and ending with a gengedcomstrong dump.

The exerciseStrings proc (although not complete)
actually tests the results & prints if a wrong
result is found.

I run this report before checking in code changes.

*/

option("explicitvars") /* Disallow use of undefined variables */
global(dead)
global(cutoff_yr)
global(true)
global(undef) /* variable with no set value, used in string tests */
global(dbuse)
global(dategood)
global(datebad)

proc main()
{
	getint(dbuse, "Exercise db functions ? (0=no)")
	set(true,1)
	set(cutoff_yr, 1900) /* assume anyone born before this is dead */


	call testStrings()
	call testDates()

	if (dbuse) {
	  call exerciseDb()
	}
}

proc exerciseDb()
{
	"database: " database() nl()
	"version: " version() nl()

	set(N, 5) /* output this many of each type of record */

	set(living,0)
	set(dead,0)

	/* count up # of living & dead indis, and output first 10 of each */
	nl() nl() "*** PERSONS ***" nl() nl()
	indiset(iset)
	forindi (person, pnum) {
		/* exercise indi stuff with the first person */
		if (lt(add(living,dead),1)) {
			call exerciseIndi(person)
		}
		/* output the first five living & first N dead people */
		if (isLivingPerson(person)) {
			set(living,add(living,1))
			if (lt(living,N)) {
				call outputLivingIndi(person)
				addtoset(iset,person,1)
			}
		} else {
			set(dead,add(dead,1))
			if (lt(dead,N)) {
				call outputRec(person)
				addtoset(iset,person,0)
			}
		}
	}
	nl() "Live INDI: " d(living) nl()
	"Dead INDI: " d(dead) nl()

	set(living,0)
	set(dead,0)
	/* count up # of living & dead fams, and output first 10 of each */

	nl() nl() "*** FAMILIES ***" nl() nl()
	forfam (fam, fnum) {
		/* output the first N living & first five dead families */
		if (isLivingFam(fam)) {
			set(living,add(living,1))
			if (lt(living,N)) {
				call outputLivingFam(fam)
			}
		} else {
			set(dead,add(dead,1))
			if (lt(dead,N)) {
				call outputRec(fam)
			}
		}
	}
	nl() "Live FAM: " d(living) nl()
	"Dead FAM: " d(dead) nl()

	nl() nl() "*** SOURCES ***" nl() nl()
	forsour (sour,snum) {
		if (lt(snum,N)) {
			call outputRec(sour)
		}
	}
	
	nl() nl() "*** EVENTS ***" nl() nl()
	foreven (even,enum) {
		if (lt(enum,N)) {
			call outputRec(even)
		}
	}
	
	nl() nl() "*** OTHERS ***" nl() nl()
	forothr (othr,onum) {
		if (lt(onum,N)) {
			call outputRec(othr)
		}
	}

	nl() nl() "*** GENGEDCOMSTRONG *** " nl() nl()
	gengedcomstrong(iset)

}

/* Output entire record, except filter out SOUR & NOTE sections */
proc outputRec(record)
{
  traverse (root(record), node, level) {
    if (or(eq(level,0),and(ne(tag(node),"SOUR"),ne(tag(node),"NOTE")))) {
      d(level) " " xref(node) " " tag(node) " " value(node)
      nl()
    }
  }
}

proc outputLivingIndi(indi)
{
  "0 @" key(indi) "@ INDI" nl()
  "1 NAME " fullname(indi,0,1,50) nl()
  fornodes(inode(indi), node) {
    if (isFamilyPtr(node)) {
      "1 " xref(node) " " tag(node) " " value(node)
      nl()
    }
  }
}


proc outputLivingFam(fam)
{
  "0 @" key(fam) "@ FAM" nl()
  fornodes(root(fam), node) {
    if (isMemberPtr(node)) {
      "1 " xref(node) " " tag(node) " " value(node)
      nl()
    }
  }
}

func isLivingFam(fam)
{
  fornodes(root(fam), node) {
    if (isMemberPtr(node)) {
      if (isLivingPerson(indi(value(node)))) { return (1) }
    }
  }
  return (0)
}

func isLivingPerson(indi)
{
  if (death(indi)) { return (0) }
  if (birth(indi)) {
    extractdate(birth(indi),day,mon,yr)
    if (and(gt(yr,300),lt(yr,cutoff_yr))) { return (0) }
  }
  return (1)
}



func isFamilyPtr (node) {
  if (eq(tag(node),"FAMC")) { return (1) }
  if (eq(tag(node),"FAMS")) { return (1) }
  return (0)
}

func isMemberPtr (node) {
  if (eq(tag(node),"HUSB")) { return (1) }
  if (eq(tag(node),"WIFE")) { return (1) }
  if (eq(tag(node),"CHIL")) { return (1) }
  return (0)
}

/* Uses a lot of function calls */
proc exerciseIndi(indi)
{
  list(lst)
  set(em, empty(lst))
  enqueue(lst, indi)
  push(lst, father(indi))
  requeue(lst, mother(indi))
  set(junk,pop(lst))
  setel(lst, 1, nextsib(indi))
  forlist(lst, el, count)
  {
    name(el) " " d(count) nl()
  }
  table(tbl)
  insert(tbl, "bob", indi)
  set(thing, lookup(tbl, "bob"))
  indiset(iset)
  addtoset(iset,indi,"bob")
  set(iset,union(iset,parentset(iset)))
  addtoset(iset,indi,"jerry")
  addtoset(iset,father(indi), "dad")
  addtoset(iset,mother(indi), "mom")
  addtoset(iset,nextsib(indi), "bro")
  spouses(indi,spouse,fam,num)
  {
    addtoset(iset,spouse,fam)
	"spouse: " fullname(spouse, true, true, 20) nl()
  }
  families(indi,fam,spouse,num)
  {
    addtoset(iset,spouse,num)
	"family: " key(fam) nl()
  }
  addtoset(iset,nextindi(indi),"next")
  addtoset(iset,previndi(indi),"prev")
  set(p,99)
  "name: " name(indi) nl()
  "title: " title(indi) nl()
  "key: " key(indi) nl()
  parents(indi) nl()
  "fullname(12): " fullname(indi,true,true,12) nl()
  "surname: " surname(indi) nl()
  "givens: " givens(indi) nl()
  "trimname(8): " trimname(indi,8) nl()
  lock(indi)
  call dumpnode("birth", birth(indi))
  call dumpnodetr("death", death(indi))
  unlock(indi)
}

proc dumpnode(desc, node)
{
  if (node)
  {
    desc ": " xref(node) " " tag(node) " " value(node)
    fornodes(node, child)
    {
      call dumpnode2(child)
    }
  }
}

proc dumpnode2(node)
{
  xref(node) " " tag(node) " " value(node)
  fornodes(node, child)
  {
    call dumpnode2(child)
  }
}

proc dumpnodetr(desc, node)
{
  if (node)
  {
    desc ": " xref(node) " " tag(node) " " value(node) nl()
    traverse(node, child,lvl)
    {
      xref(node) " " tag(node) " " value(node) nl()
    }
  }
}

/* report failure to screen, as well to to output */
proc reportfail(str)
{
	print(str)
	print("\n")
	if (dbuse) {
		str nl()
	}
}

/* test some string functions against defined & undefined strings */
proc testStrings()
{
	set(str,"hey")
	set(str2,upper(str))
	if (ne(str2,"HEY")) {
		call reportfail("upper FAILED")
	}
	set(str4,capitalize(str))
	if (ne(str4,"Hey")) {
		call reportfail("capitalize FAILED")
	}
	set(str6,concat(str2,str4))
	if (ne(str6,"HEYHey")) {
		call reportfail("concat FAILED")
	}
	set(str3,upper(undef))
	set(str5,capitalize(undef))
	set(str7,concat(str3,str5))
	if (ne(str7,undef)) {
		call reportfail("concat FAILED on undefs")
	}
	set(str7,strconcat(str3,str5))
	if (ne(str7,undef)) {
		call reportfail("strconcat FAILED on undefs")
	}
	set(str8,lower(str4))
	if (ne(str8,"hey")) {
		call reportfail("lower FAILED")
	}
	set(str9,lower(undef))
	if (ne(str9,undef)) {
		call reportfail("lower FAILED on undef")
	}
	set(str10,alpha(3))
	if(ne(str10,"c")) {
		call reportfail("alpha(3) FAILED")
	}
	set(str10,roman(4))
	if(ne(str10,"iv")) {
		call reportfail("roman(4) FAILED")
	}
	set(str11,d(43))
	if(ne(str11,"43")) {
		call reportfail("d(43) FAILED")
	}
	set(str12,card(4))
	if(ne(str12,"four")) {
		call reportfail("card(4) FAILED")
	}
	set(str13,ord(5))
	if(ne(str13,"fifth")) {
		call reportfail("ord(5) FAILED")
	}
	if (ne(strcmp("alpha","beta"),-1)) {
		call reportfail("strcmp(alpha,beta) FAILED")
	}
	if (ne(strcmp("gamma","delta"),1)) {
		call reportfail("strcmp(gamma,delta) FAILED")
	}
	if (ne(strcmp("zeta","zeta"),0)) {
		call reportfail("strcmp(zeta,zeta) FAILED")
	}
	if (ne(strcmp(undef,""),0)) {
		call reportfail("strcmp(undef,) FAILED")
	}
	if (ne(substring("considerable",2,4),"ons")) {
		call reportfail("substring(considerable,2,4) FAILED")
	}
	if (ne(substring(undef,2,4),0)) {
		call reportfail("substring(undef,2,4) FAILED")
	}
	if (ne(rjustify("hey",5), "  hey")) {
		call reportfail("rjustify(hey,5) FAILED")
	}
	if (ne(rjustify("heymon",5), "heymo")) {
		call reportfail("rjustify(heymon,5) FAILED")
	}
	/* eqstr returns bool, which may be compared to 0 but no other number */
	if (ne(eqstr("alpha","beta"),0)) {
		call reportfail("eqstr(alpha,beta) FAILED")
	}
	if (not(eqstr("alpha","alpha"))) {
	call reportfail("eqstr(alpha,alpha) FAILED")
	}
	if (ne(strtoint("4"), 4)) {
	call reportfail("strtoint(4) FAILED")
	}
	if (ne(strsoundex("pat"),strsoundex("pet"))) {
		call reportfail("soundex(pat) FAILED")
	}
	if (ne(strlen("pitch"),5)) {
		call reportfail("strlen(pitch) FAILED")
	}
	set(str14,"the cat in the hat put the sack on the rat and the hat on the bat ")
	if (ne(index(str14,"at",1),6)) {
		call reportfail("index(str14,at,1) FAILED")
	}
	if (ne(index(str14,"at",2),17)) {
		call reportfail("index(str14,at,2) FAILED")
	}
	if (ne(index(str14,"at",3),41)) {
		call reportfail("index(str14,at,3) FAILED")
	}
	if (ne(index(str14,"at",4),53)) {
		call reportfail("index(str14,at,4) FAILED")
	}
	if (ne(index(str14,"at",5),64)) {
		call reportfail("index(str14,at,5) FAILED")
	}
	if (ne(strlen(str14),66)) {
		call reportfail("strlen(str14) FAILED")
	}
	set(str15,strconcat(str14,str14))
	if (ne(strlen(str15),132)) {
		call reportfail("strlen(str15) FAILED")
	}
	if (ne(index(str15,"at",10),130)) {
		call reportfail("index(str15,at,10) FAILED")
	}
	set(str16,strconcat(str15,str15))
	if (ne(strlen(str16),264)) {
		call reportfail("strlen(str16) FAILED")
	}
	if (ne(index(str16,"at",20),262)) {
		call reportfail("index(str16,at,20) FAILED")
	}
	if (ne(substring(str16,260,262)," ba")) {
		call reportfail("substring(str16,260,262) FAILED")
	}
}

/* 
  Using specified formats, check stddate(src) against dests
  and complexdate(src) against destc
  tdfb = test date format both (simple & complex)
  */
proc tdfb(src, dayfmt, monfmt, yrfmt, sfmt, ofmt, dests, destc)
{
	dayformat(dayfmt)
	monthformat(monfmt)
	yearformat(yrfmt)
	dateformat(sfmt)
	originformat(ofmt)
	set(result, stddate(src))
	if (ne(result, dests)) {
		set(orig, concat(src,", ", d(dayfmt), ", ", d(monfmt)))
		set(orig, concat(orig, ", ", d(yrfmt), ", ",d(sfmt)))
		set(orig, concat(orig, ", ", d(ofmt)))
		call reportfail(concat("stddate failure: '", dests, "'<>'", result, "'", " from ", orig))
		incr(datebad)
	} else {
		incr(dategood)
	}
	set(result, complexdate(src))
	if (ne(result, destc)) {
		set(orig, concat(src,", ", d(dayfmt), ", ", d(monfmt)))
		set(orig, concat(orig, ", ", d(yrfmt), ", ",d(sfmt)))
		set(orig, concat(orig, ", ", d(ofmt)))
		call reportfail(concat("complexdate failure: '", destc, "'<>'", result, "'", " from ", orig))
		incr(datebad)
	} else {
		incr(dategood)
	}
}

/* test some date functions with various GEDCOM dates */
/* These assume English output */
proc testDates()
{
	set(dategood, 0)
	set(datebad, 0)

/* NB: We do not test all possible combinations, as there are quite a lot
  (3 day formats, 3 month formats, 3 year formats, 14 combining formats,
  9 origin formats -- multiply out to 3402 combinations */

/* test simple 4 digit years dates */
	call tdfb("2 JAN 1953", 0, 0, 0, 0, 0, " 2  1 1953", " 2  1 1953")
	call tdfb("2 JAN 1953", 1, 0, 0, 0, 0, "02  1 1953", "02  1 1953")
	call tdfb("2 JAN 1953", 2, 0, 0, 0, 0, "2  1 1953", "2  1 1953")
	call tdfb("2 JAN 1953", 2, 1, 0, 0, 0, "2 01 1953", "2 01 1953")
	call tdfb("2 JAN 1953", 2, 2, 0, 0, 0, "2 1 1953", "2 1 1953")
	call tdfb("2 JAN 1953", 2, 3, 0, 0, 0, "2 JAN 1953", "2 JAN 1953")
	call tdfb("2 JAN 1953", 2, 4, 0, 0, 0, "2 Jan 1953", "2 Jan 1953")
	call tdfb("2 JAN 1953", 2, 5, 0, 0, 0, "2 JANUARY 1953", "2 JANUARY 1953")
	call tdfb("2 JAN 1953", 2, 6, 0, 0, 0, "2 January 1953", "2 January 1953")
	call tdfb("2 JAN 1953", 2, 6, 0, 0, 2, "2 January 1953 A.D.", "2 January 1953 A.D.")
	call tdfb("2 JAN 1953", 2, 6, 0, 0, 12, "2 January 1953 AD", "2 January 1953 AD")
	call tdfb("2 JAN 1953", 2, 6, 0, 0, 22, "2 January 1953 C.E.", "2 January 1953 C.E.")
	call tdfb("2 JAN 1953", 2, 6, 0, 0, 32, "2 January 1953 CE", "2 January 1953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 1, 32, "1 2, 1953 CE", "1 2, 1953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 2, 32, "1/2/1953 CE", "1/2/1953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 3, 32, "2/1/1953 CE", "2/1/1953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 4, 32, "1-2-1953 CE", "1-2-1953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 5, 32, "2-1-1953 CE", "2-1-1953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 6, 32, "121953 CE", "121953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 7, 32, "211953 CE", "211953 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 8, 32, "1953 1 2 CE", "1953 1 2 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 9, 32, "1953/1/2 CE", "1953/1/2 CE")
	call tdfb("2 JAN 1953", 2, 2, 0, 10, 32, "1953-1-2 CE", "1953-1-2 CE")

/* test simple 3 digit years dates */
	call tdfb("11 MAY 812", 0, 0, 0, 0, 0, "11  5  812", "11  5  812")
	call tdfb("11 MAY 812", 0, 1, 0, 0, 0, "11 05  812", "11 05  812")
	call tdfb("11 MAY 812", 0, 2, 0, 0, 0, "11 5  812", "11 5  812")
	call tdfb("11 MAY 812", 0, 3, 0, 0, 0, "11 MAY  812", "11 MAY  812")
	call tdfb("11 MAY 812", 0, 4, 0, 0, 0, "11 May  812", "11 May  812")
	call tdfb("11 MAY 812", 0, 5, 0, 0, 0, "11 MAY  812", "11 MAY  812" )
	call tdfb("11 MAY 812", 0, 6, 0, 0, 0, "11 May  812", "11 May  812")
	call tdfb("11 MAY 812", 1, 6, 0, 0, 0, "11 May  812", "11 May  812")
	call tdfb("11 MAY 812", 2, 6, 0, 0, 0, "11 May  812", "11 May  812")

/* test simple 2 digit years dates */
	call tdfb("2 JAN 53", 0, 0, 0, 0, 0, " 2  1   53", " 2  1   53")
	call tdfb("2 JAN 53", 1, 0, 0, 0, 0, "02  1   53", "02  1   53")
	call tdfb("2 JAN 53", 2, 0, 0, 0, 0, "2  1   53", "2  1   53")
	call tdfb("2 JAN 53", 2, 1, 0, 0, 0, "2 01   53", "2 01   53")
	call tdfb("2 JAN 53", 2, 1, 1, 0, 0, "2 01 0053", "2 01 0053")
	call tdfb("2 JAN 53", 2, 1, 2, 0, 0, "2 01 53", "2 01 53")

/* test simple 1 digit years dates */
	call tdfb("2 JAN 3", 0, 0, 0, 0, 0, " 2  1    3", " 2  1    3")
	call tdfb("2 JAN 3", 1, 0, 0, 0, 0, "02  1    3", "02  1    3")
	call tdfb("2 JAN 3", 2, 0, 0, 0, 0, "2  1    3", "2  1    3")
	call tdfb("2 JAN 3", 2, 1, 0, 0, 0, "2 01    3", "2 01    3")
	call tdfb("2 JAN 3", 2, 1, 1, 0, 0, "2 01 0003", "2 01 0003")
	call tdfb("2 JAN 3", 2, 1, 2, 0, 0, "2 01 3", "2 01 3")

/* test simple BC dates */
	call tdfb("15 MAR 30 B.C.", 0, 0, 0, 0, 0, "15  3   30", "15  3   30")
	call tdfb("15 MAR 30 B.C.", 0, 0, 0, 0, 1, "15  3   30 B.C.", "15  3   30 B.C.")
	call tdfb("15 MAR 30 B.C.", 0, 0, 1, 0, 1, "15  3 0030 B.C.", "15  3 0030 B.C.")
	call tdfb("15 MAR 30 B.C.", 0, 0, 2, 0, 1, "15  3 30 B.C.", "15  3 30 B.C.")
	call tdfb("15 MAR 30 B.C.", 0, 0, 2, 0, 2, "15  3 30 B.C.", "15  3 30 B.C.")
	call tdfb("15 MAR 30 B.C.", 0, 0, 2, 0, 11, "15  3 30 BC", "15  3 30 BC")
	call tdfb("15 MAR 30 B.C.", 0, 0, 2, 0, 21, "15  3 30 B.C.E.", "15  3 30 B.C.E.")
	call tdfb("15 MAR 30 B.C.", 0, 0, 2, 0, 31, "15  3 30 BCE", "15  3 30 BCE")

/* test simple dates in non-GEDCOM format */
	call tdfb("1930/11/24", 0, 0, 0, 0, 0, "24 11 1930", "24 11 1930")
	call tdfb("11/24/1930", 0, 0, 0, 0, 0, "24 11 1930", "24 11 1930")
	call tdfb("24/11/1930", 0, 0, 0, 0, 0, "24 11 1930", "24 11 1930")

/* Complex tests */
	call tdfb("AFT 3 SEP 1630", 0, 0, 0, 0, 0, " 3  9 1630", "after  3  9 1630")
	call tdfb("BEF 3 SEP 1630", 0, 0, 0, 0, 0, " 3  9 1630", "before  3  9 1630")
	call tdfb("BET 3 SEP 1630 AND OCT 1900", 0, 0, 0, 0, 0, " 3  9 1630", "between  3  9 1630 and    10 1900")
	call tdfb("FROM 3 SEP 1630", 0, 0, 0, 0, 0, " 3  9 1630", "from  3  9 1630")
	call tdfb("TO 3 SEP 1630", 0, 0, 0, 0, 0, " 3  9 1630", "to  3  9 1630")
	call tdfb("FROM 3 SEP 1630 TO 1700", 0, 0, 0, 0, 0, " 3  9 1630", "from  3  9 1630 to       1700")
	call tdfb("ABT 3 SEP 890", 0, 0, 0, 0, 0, " 3  9  890", "about  3  9  890")
	call tdfb("EST 3 SEP 890", 0, 0, 0, 0, 0, " 3  9  890", "estimated  3  9  890")
	call tdfb("CAL 3 SEP 890", 0, 0, 0, 0, 0, " 3  9  890", "calculated  3  9  890")

/* Complex tests with bad input */
	call tdfb("24 SEP 811 TO 29 SEP 811", 0, 0, 0, 0, 0, "24  9  811", "from 24  9  811 to 29  9  811")


/* Calendar tests (as of 2001/12, we fail most of these) */
	call tdfb("@#DGREGORIAN@ 1 JAN 1953", 2, 6, 0, 0, 0, "1 January 1953", "1 January 1953")
	call tdfb("@#DJULIAN@ 1 JAN 1953", 2, 6, 0, 0, 0, "1 January 1953", "1 January 1953")
	call tdfb("@#DFRENCH R@ 1 VEND 11", 2, 6, 0, 0, 0, "1 Vendemiare 11", "1 Vendemiare 11")
	call tdfb("@#DHEBREW R@ 1 TSH 11", 2, 6, 0, 0, 0, "1 Tishri 11", "1 Tishri 11")
	/* ROMAN would presumably be in AUC, and days counted before K,N,I */

	if (gt(datebad, 0)) {
		print(concat("Failed ", d(datebad), "/", d(add(dategood,datebad)), " date tests"))
	}
}

