/*
 * @progname       findmissing.ll
 * @version        1.0
 * @author         
 * @category       
 * @output         Text
 * @description    
 *
 * find persons that are 'isolated' in your database - no parents and not
 * in any families..
 */
proc main ()
{
        "THE FOLLOWING PERSONS ARE 'ISOLATED' IN YOUR DATABASE" nl() nl()
        forindi(indi, num) {
                if (and(not(parents(indi)), eq(0,nfamilies(indi)))) {
                        name(indi) " (" key(indi) ")" nl()
                }
        }
}

