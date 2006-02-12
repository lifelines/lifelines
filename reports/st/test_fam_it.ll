/*
 * @progname       test_fam_it
 * @version        1
 * @author         Stephen Dum
 * @category       self-test
 * @output         text
 * @description
 * 
 * test family iterators: forfam, children and spouses
 * Iterate over some data, printing results, so we can
 * compare the output with exected results.
 */


proc main() {
    print(nl())
    forfam(f,c) {
	print(d(c),": ",key(f),nl()) 
	print("    children\n")
	children(f,i,c2) {
	    print("    ",d(c2),": ",key(i),nl()) 
	}
	print("    spouses\n")
	spouses(f,s,c2) {
	    print("    ",d(c2),": ",key(s),nl()) 
	}
    }
}
