/*
 * @progname       test_forfam_it
 * @version        1
 * @author         Stephen Dum
 * @category       self-test
 * @output         text
 * @description
 * 
 * test family iterator: forfam
 * Iterate over some data, printing results, so we can
 * compare the output with exected results.
 */
proc main() {
    print(nl())
    forfam(f,c) { print(d(c),": ",key(f),nl()) }
}
