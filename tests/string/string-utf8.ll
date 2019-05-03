/* From Stephen Dum */
/* Designed to validate behaviour of various functions with multi-byte UTF-8 
   character strings */
proc main()
{
  /* NOTE: index() is not UTF-8 aware, searching on bytes not characters*/
  "TEST: index()\n"
  /* following string should be
   * A, A grave, A acute, A circumflx, A tilde, A diaeresis(aka umlaut), AE
   * E, E grave, E acute, E circumflx, A diaeresis
   * I, I grave, I acute, I circumflex, I diaresis
   * small ϴf with hook
   */
  set(s,"AÀÁÂÃÄÅÆEÈÊËIÌÍÎÏƒϴC") 
  "string: " s " (20 characters, 35 bytes)\n"
       "index of the 1st character, A:" d(index(s,"A",1)) "\n"
       "index of the 2nd character, À:" d(index(s,"À",1)) "\n"
       "index of the 3nd character, Á:" d(index(s,"Á",1)) "\n"
       "index of the 4th character, Â:" d(index(s,"Â",1)) "\n"
       "index of the 5th character, Ã:" d(index(s,"Ã",1)) "\n"
       "index of the 6th character, Ä:" d(index(s,"Ä",1)) "\n"
       "index of the 7th character, Å:" d(index(s,"Å",1)) "\n"
       "index of the 8th character, Æ:" d(index(s,"Æ",1)) "\n"
       "index of the 9th character, E:" d(index(s,"E",1)) "\n"
       "index of the 10th character, È:" d(index(s,"È",1)) "\n"
       "index of the 11th character, Ê:" d(index(s,"Ê",1)) "\n"
       "index of the 12th character, Ë:" d(index(s,"Ë",1)) "\n"
       "index of the 13th character, I:" d(index(s,"I",1)) "\n"
       "index of the 14th character, Ì:" d(index(s,"Ì",1)) "\n"
       "index of the 15th character, Í:" d(index(s,"Í",1)) "\n"
       "index of the 16th character, Î:" d(index(s,"Î",1)) "\n"
       "index of the 17th character, Ï:" d(index(s,"Ï",1)) "\n"
       "index of the 18th character, ƒ:" d(index(s,"ƒ",1)) "\n"
       "index of the 19th character, ϴ:" d(index(s,"ϴ",1)) "\n"
       "index of the 20th character, C:" d(index(s,"C",1)) "\n"
  "\n"

  /* NOTE: extracttokens() is not UTF-8 aware, searching on bytes not characters*/
  "TEST: extracttokens()\n"
  list(lis)
  extracttokens(s,lis,cnt,"E")
  "extracttokens with separator E: '" getel(lis,1) "' and '" getel(lis,2) "'\n"
  extracttokens(s,lis,cnt,"I")
  "extracttokens with separator I: '" getel(lis,1) "' and '" getel(lis,2) "'\n"
  "\n"

  /* NOTE: substring() is broken because it is not UTF-8 aware, searching on bytes rather than characters */
  "TEST: substring()\n"
  "substring [1:1]: " substring(s,1,1) "\n"
  "substring [1:2]: " substring(s,1,2) "\n"
  "substring [1:3]: " substring(s,1,3) "\n"
  "substring [1:4]: " substring(s,1,4) "\n"
  "substring [1:5]: " substring(s,1,5) "\n"
  "substring [1:6]: " substring(s,1,6) "\n"
  "substring [1:7]: " substring(s,1,7) "\n"
  "substring [1:8]: " substring(s,1,8) "\n"
  "substring [1:9]: " substring(s,1,9) "\n"
  "substring [2:9]: " substring(s,2,9) "\n"
  "substring [3:9]: " substring(s,3,9) "\n"
  "substring [4:9]: " substring(s,4,9) "\n"
  "substring [5:9]: " substring(s,5,9) "\n"
  "substring [6:9]: " substring(s,6,9) "\n"
  "substring [7:9]: " substring(s,7,9) "\n"
  "substring [8:9]: " substring(s,8,9) "\n"
  "substring [20:20]: " substring(s,20,20) "\n"
}
