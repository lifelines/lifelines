/*
 * @progname       6gen1.ll
 * @version        1.0
 * @author         Wetmore, Manis
 * @category       
 * @output         Text, 80 cols
 * @description    
 *
 *   select and produce a 6 generation ancestor report for 
 *   the person selected.
 *   Output is an ASCII file, and will probably need to be printed
 *   using 10 or 12 pitch.
 *
 *   6gen1
 *
 *   Code by Tom Wetmore, ttw@cbnewsl.att.com
 *   With modifications by:  Cliff Manis
 *
 *   This report works only with the LifeLines Genealogy program
 *
 *   version one of this report was written by Tom Wetmore, in 1990,
 *
 *   select and produce a 6 generation ancestor report for 
 *   the person selected.
 *
 *   Output is an ASCII file, and will probably need to be printed
 *   using 10 or 12 pitch.
 *
 *   An example of the output may be seen at end of this report.
 */

proc main ()
{
        getindi(indi)
        set (nl,nl())
        pagemode(70,80)
        call pedout(indi,1,6,1,64)
        pageout()
	print(nl())
}

proc pedout (indi, gen, max, top, bot)
{
        if (le(gen,max)) {
                set(gen,add(1,gen))
                set(fath,father(indi))
                set(moth,mother(indi))
                set(height,add(1,sub(bot,top)))
                set(offset,div(sub(height,1),2))
                call block(indi,add(top,offset),mul(8,sub(gen,2)))
                set(half,div(height,2))
                call pedout(fath,gen,max,top,sub(add(top,half),1))
                call pedout(moth,gen,max,add(top,half),bot)
        }
}

proc block (indi, row, col)
{
	print(".")
        set(row,add(3,row))
        set(col,add(3,col))
        pos(row,col)
        if (indi) { name(indi) }
        else      { "_______________" }
}

/*  Sample output of the 6gen1 report for: a c /manis 
 
 
                                          John MANESS
                                  Samuel P. MANES
                                          _______________
                          William Thomas MANES
                                          _______________
                                  Fanny (MANES)
                                          _______________
                  William Bowers MANES
                                          James BOWERS
                                  Anderson BOWERS
                                          Martha
                          Martha A. BOWERS
                                          Christopher Columbus COWAN
                                  Lurina Viney "Vina" COWAN
                                          Mary BOYD
          Fuller Ruben MANES
                                          _______________
                                  Henry B. CANTER
                                          _______________
                          James H. CANTER
                                          _______________
                                  Polina (CANTER)
                                          _______________
                  Cordelia "Corda" F. CANTER
                                          _______________
                                  James WHITEHORN
                                          _______________
                          Martha Marie WHITEHORN
                                          Kennedy "Kan" Powell FOSTER
                                  Martha "Patsy" FOSTER
                                          Rebecca KERSAWN
  Alda Clifford MANIS
                                          _______________
                                  Amos MANIS
                                          _______________
                          Thomas D.A.F.S. MANIS
                                          David FRANCIS
                                  Mary Elizabeth FRANCIS
                                          Mary CROCKETT
                  William Loyd MANIS
                                          Jacob BIRD
                                  John BIRD
                                          Mrs. (BIRD)
                          Frances Amanda BIRD
                                          G. Christopher SHRADER
                                  Elizabeth SHRADER
                                          Mary WEBB
          Edith Alberta MANIS
                                          John NEWMAN
                                  Aaron NEWMAN
                                          Nancy FRANKLIN
                          John Franklin NEWMAN
                                          Thomas B. RANKIN
                                  Sinea RANKIN
                                          Jennet BRADSHAW
                  Lillie Caroline "Carolyn" NEWMAN
                                          James CORBETT
                                  John Williams CORBETT
                                          Polly GRESHAMS
                          Mary Jean CORBETT
                                          _______________
                                  Betsy EUDAILY
                                          _______________

*/

/* End of Report */


 
 
 
 
