/*
 * @progname       test_othr_it
 * @version        1
 * @author         Stephen Dum
 * @category       self-test
 * @output         text
 * @description
 * 
 * test iterators: forsour, foreven, forothr
 * Iterate over some data, printing results, so we can
 * compare the output with exected results.
 */
proc main() {
    print(nl())
    print("forsour\n")
    forsour(s,c) {
        print(d(c),": ",key(s),nl())
    }
    print("foreven\n")
    foreven(e,c) {
        print(d(c),": ",key(e),nl())
    }
    print("forothr\n")
    forothr(s,c) {
        print(d(c),": ",key(s),nl())
    }
}
