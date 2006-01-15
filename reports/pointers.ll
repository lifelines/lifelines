/*
 * @progname       pointers
 * @version        1.0
 * @author         Chandler
 * @category
 * @output         Text
 * @description

Test a database for reciprocity of pointers between persons and families.

Report any failures, primarily the following:

 Person Inn is a spouse/child in Fnn, but Fnn has no corresponding pointer.
 Family Fnn has HUSB/WIFE/CHIL Inn, but Inn has no corresponding pointer.

Some failures are supposed to be impossible, but are covered here
nonetheless:

 Family Fnn has HUSB/WIFE/CHIL Inn, but Inn does not exist.
 Family Fnn has a null HUSB/WIFE/CHIL line.
 Person Inn is a spouse/child in Fnn, but Fnn does not exist.
 Person Inn has a null FAMS/FAMC line.

Version 1.0 - 2003 Jul 2 - John F. Chandler

This program works only with LifeLines.

*/

global(pointers)

proc main() {
table(spou)	/* each entry is the list of spouses in the keyed family */
table(chil)	/* each entry is the list of children in the keyed family */

"Testing database " qt() database() qt() " for pointer reciprocity\n"

set(pointers,0)

/* loop through persons and note all the families they belong to */
forindi(i,n) {
	set(k,save(key(i)))
	fornodes(root(i),node) {
		set(type,tag(node))
		if(eqstr(type,"FAMC")) {
			call tally(type,"child",chil,node,k)
		} elsif(eqstr(type,"FAMS")) {
			call tally(type,"spouse",spou,node,k)
		}
	}
}
/* loop through families and compare the members against the list
   compiled by scanning persons -- flag any mismatches */
forfam(f,n) {
	set(id,save(key(f)))
	set(cl,lookup(chil,id))
	set(sl,lookup(spou,id))
	fornodes(root(f),node) {
		set(type,tag(node))
		if(eqstr(type,"CHIL")) { call checkoff(type,cl,id,node) }
		elsif(or(eqstr(type,"HUSB"),eqstr(type,"WIFE"))) {
			call checkoff(type,sl,id,node)
		}
	}
/* any remaining list elements are errors */
	if(sl) {
		while(k,dequeue(sl)) {
			"\nPerson " k " is a spouse in " id
			", but " id " has no corresponding pointer."
		}
	}
	if(cl) {
		while(k,dequeue(cl)) {
			"\nPerson " k " is a child in " id
			", but " id " has no corresponding pointer."
		}
	}
}
"\n\nFinished after checking " d(pointers) " pointers.\n"

}

/* check a family member against the expected list.
   anyone not on the list is an error.
   remove each person from the list when found here. */
proc checkoff(type,list,id,node) {
	incr(pointers)
	if(eq(mod(pointers,500),0)) { print(".") }
	if(k,value(node)) {
		set(key,substring(k,2,sub(strlen(k),1)))
		if(list) {
			set(count,length(list))
			while(gt(count,0)) {
				decr(count)
				set(c,dequeue(list))
				if(eqstr(c,key)) { set(count,-1) }
				else { enqueue(list,c) }
			}
		}
		if(eq(count,0)) {
			"\nFamily " id " has " type " " key ", but " key
			if(reference(k)) { " has no corresponding pointer." }
			else { " does not exist." }
		}
	} else { "\nFamily " id " has a null " type " line." }
}

/* build a list of persons who belong to families */
proc tally(type,member,table,node,k) {
	incr(pointers)
	if(eq(mod(pointers,500),0)) { print(".") }
	set(id,value(node))
	if(reference(id)) {
		set(id,save(substring(id,2,sub(strlen(id),1))))
		if(l,lookup(table,id)) { enqueue(l,k) }
		else {
			list(l)
			enqueue(l,k)
			insert(table,id,l)
		}
	} elsif(id) {
		"\nPerson " k " is a " member " in " id
		", but " id " does not exist."
	} else { "\nPerson " k " has a null " type " line." }
}

