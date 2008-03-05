/*
 * @progname       afg.ll
 * @version        1.0
 * @author         Tom Wetmore
 * @category
 * @output         Text
 * @description
 
  Shows simple family groups starting at a person and extending
  out in ancestry.
  
  * Tom Wetmore -- 1 March 2008
  */
global(iset)

proc main ()
{
	list(fams)    /* families queued for possible processing */
	table(fset)   /* families that have been processed */
	table(iset)   /* table of ancestors that have been given numbers */
	set(n, 1)     /* counter that assigns numbers to ancestors */

	getindi(p, "enter person to build the family groups for")
	if (not(p)) { return() }
	set(k, save(key(p)))
	insert(iset, k, rjustify(d(n), 3))
	incr(n)
	set(f, parents(p))
	if (not(f))  { return() }

	enqueue(fams, f)
	while (f, dequeue(fams)) {
		set(k, key(f))
		if (lookup(fset, k)) { continue() }
		insert(fset, save(k), 1)
		if (h, husband(f)) {
			if (g, parents(h)) { enqueue(fams, g) }
			insert(iset, save(key(h)), rjustify(d(n), 3))
			incr(n)
		}
		if (w, wife(f)) {
			if (g, parents(w)) { enqueue(fams, g) }
			insert(iset, save(key(w)), rjustify(d(n), 3))
			incr(n)
		}
		call showfamily(f)
	}
}

proc showfamily (f)
{
	if (p, husband(f)) { call showperson(p, 0) }
	if (p, wife(f)) { call showperson(p, 0) }
	if (e, marriage(f)) {
		if (long(e)) { "    m. " long(e) nl() }
	}
	children (f, c, i) { call showperson(c, 1) }

	"----------------------------------------" nl()
}

proc showperson (p, child)
{
	if (child) { "    " }
	set(i, lookup(iset, key(p)))
	if (i) { i " " } else { "    " }
	name(p) nl()
	if (e, birth(p)) {
		if (child) { "    " }
		"    b. " long(e) nl()
	}
	if (e, death(p)) {
		if (child) { "    " }
		"    d. " long(e) nl()
	}
         fornotes (root(p), n) {
                 if (child) { "    " }
                 "    n: " n nl()
         }
}
