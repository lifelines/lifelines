/*
 * @progname       test_indiit
 * @version        1
 * @author         Stephen Dum
 * @category       self-test
 * @output         text
 * @description
 * 
 * test indi iterators: forindi, Parents, families, spouses, mothers, fathers
 * Iterate over some data, printing results, so we can
 * compare the output with exected results.
 */

proc main() {
    print(nl())
    forindi(i,c) { 
	print(d(c),": ",key(i),nl())
	print("    Parents\n")
	Parents(i,f,c2) {
	    print("    ",d(c2),": ",key(f),nl())
	}
	print("    families\n")
	families(i,f,s,c2) {
	    print("    ",d(c2),": ",key(f)," ",key(s),nl())
	}
	print("    spouses\n")
	spouses(i,s,f,c2) {
	    print("    ",d(c2),": ",key(s)," ",key(f),nl())
	}
	print("    mothers\n")
	mothers(i,m,f,c2) {
	    print("    ",d(c2),": ",key(m)," ",key(f),nl())
	}
	print("    fathers\n")
	fathers(i,fa,f,c2) {
	    print("    ",d(c2),": ",key(fa)," ",key(f),nl())
	}
    }
}
