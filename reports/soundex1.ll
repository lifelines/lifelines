/*
 * @progname       soundex1.ll
 * @version        1.0
 * @author         Jones
 * @category       
 * @output         Text
 * @description    
 *
 *   Produces a chart of all surnames in database with corresponding 
 *   SOUNDEX codes.
 *   It is designed for 10 or 12 pitch, HP laserjet III, or any
 *   other printer.
 *
 *   soundex1
 *
 *   Code by James P. Jones, jjones@nas.nasa.gov
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by James P. Jones, 28 Sep 1992
 *
 *
 *   Output is an ASCII file.
 *
 *   An example of the output may be seen at end of this report.
 *
 */

proc main ()
{
        indiset(idx)
        forindi(indi,n) {
                addtoset(idx,indi,n)
                print(".")
        }
        print(nl()) print("indexed ") print(d(n)) print(" persons.")
        print(nl())
        print(nl())
        print("begin sorting") print(nl())
        namesort(idx)
        print("done sorting") print(nl())

        col(11) "SOUNDEX CODES OF ALL SURNAMES IN DATABASE" nl()
        col(1) " " nl()
        col(1) " " nl()
        col(16) "   Surname      Soundex Code" nl()
        col(16) " -------------  ------------" nl()

        set(last, " ")
        forindiset(idx,indi,v,n) {
                if(strcmp(surname(indi), last)) {
                    col(20) upper(surname(indi))
                    col(36) soundex(indi)
                }
                set(last,surname(indi))
                print(".")
        }
        nl()
        print(nl())
}

/* Sample output of this report:

          SOUNDEX CODES OF ALL SURNAMES IN DATABASE
 
 
                  Surname      Soundex Code
                -------------  ------------
                   ABERNATHY       A165
                   AHMADVAND-S     A531
                   ANDERSON        A536
                   ANDREWS         A536
                   BAILEY          B400
                   BARBIE          B610
                   BENNET          B530

*/

/* End of Report */

