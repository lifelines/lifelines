/*
 * @progname       test_forindi_it
 * @version        1
 * @author         Stephen Dum
 * @category       self-test
 * @output         text
 * @description
 * 
 * test indi iterator: forindi
 * Iterate over some data, printing results, so we can
 * compare the output with exected results.
 */
proc main() {
    print(nl())
    forindi(i,c) { print(d(c),": ",key(i),nl()) }
}
