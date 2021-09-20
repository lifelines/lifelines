/* From Seppo Sippu */
/* Designed to validate behaviour of various functions with multi-byte character strings */
/* Seppo indicates that these are 2-byte unicode, but on my system they are a mix of 2-byte and 4-byte unicode. */
/* Stephen Dum -- and by the time I saw this they are all 3 byte UTF-8 codes
 * it is now unicode characters encoded as UTF-8
 * I added some 2 byte latin characters (utf-8 encoded) to the test and a 
 * lot more testing of extracttokens.
*/

proc extract(str, sep) {
    list(lis)
    extracttokens(str,lis,cnt,sep)
    "extracttokens from '" str "' with separator '" sep "' found " d(cnt) " tokens:\n"
    forlist(lis,val,cnt) { 
        "   '" val "'" 
    } 
    nl()
}


proc main()
{
  /* NOTE: index() is broken because it is not UTF-8 aware, searching on bytes rather than characters */
  "TEST: index()\n"
  set(s,"A쎥쎤쎶B쎅쎄쎖C")
  "string: " s " (9 characters, 15 bytes)\n"
  "index of the 1st character, A: "  d(index(s,"A",1))  "\n"
  "index of the 2nd character, 쎥: " d(index(s,"쎥",1)) "\n"
  "index of the 3rd character, 쎶: " d(index(s,"쎤",1)) "\n"
  "index of the 4th character, 쎥: " d(index(s,"쎥",1)) "\n"
  "index of the 5th character, B: "  d(index(s,"B",1))  "\n"
  "index of the 6th character, 쎅: " d(index(s,"쎅",1)) "\n"
  "index of the 7th character, 쎄: " d(index(s,"쎄",1)) "\n"
  "index of the 8th character, 쎖: " d(index(s,"쎖",1)) "\n"
  "index of the 9th character, C: "  d(index(s,"C",1))  "\n"
  "\n"

  call extract(s,"B")
  call extract(s,"쎖")
  "\n"
   "check sugstring and strlen\n"
  "substring [1:1]: " substring(s,1,1) "  len=" d(strlen(substring(s,1,1))) "\n"
  "substring [1:2]: " substring(s,1,2) "  len=" d(strlen(substring(s,1,2))) "\n"
  "substring [1:3]: " substring(s,1,3) "  len=" d(strlen(substring(s,1,3))) "\n"
  "substring [1:4]: " substring(s,1,4) "  len=" d(strlen(substring(s,1,4))) "\n"
  "substring [1:5]: " substring(s,1,5) "  len=" d(strlen(substring(s,1,5))) "\n"
  "substring [1:6]: " substring(s,1,6) "  len=" d(strlen(substring(s,1,6))) "\n"
  "substring [1:7]: " substring(s,1,7) "  len=" d(strlen(substring(s,1,7))) "\n"
  "substring [1:8]: " substring(s,1,8) "  len=" d(strlen(substring(s,1,8))) "\n"
  "substring [1:9]: " substring(s,1,9) "  len=" d(strlen(substring(s,1,9))) "\n"
  "substring [2:9]: " substring(s,2,9) "  len=" d(strlen(substring(s,2,9))) "\n"
  "substring [3:9]: " substring(s,3,9) "  len=" d(strlen(substring(s,3,9))) "\n"
  "substring [4:9]: " substring(s,4,9) "  len=" d(strlen(substring(s,4,9))) "\n"
  "substring [5:9]: " substring(s,5,9) "  len=" d(strlen(substring(s,5,9))) "\n"
  "substring [6:9]: " substring(s,6,9) "  len=" d(strlen(substring(s,6,9))) "\n"
  "substring [7:9]: " substring(s,7,9) "  len=" d(strlen(substring(s,7,9))) "\n"
  "substring [8:9]: " substring(s,8,9) "  len=" d(strlen(substring(s,8,9))) "\n"
  "substring [9:9]: " substring(s,9,9) "  len=" d(strlen(substring(s,9,9))) "\n"

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
  call extract("AÄa","ÄA")
  call extract("aÄa","ÄA")
  call extract("ÄAa","ÄA")
  call extract(s,"A")
  call extract(s,"Ë")
  call extract(s,"I")
  call extract(s,"ϴ")
  call extract(s,"C")
  call extract(s,"c")

  set(w,"one,two,:three,:=end")
  call extract(w,",:=")
  call extract("Hi there folks!",",")
  call extract("Hi there folks!","!")
  call extract("one,two,,three,,,end,",",")
  "\n"
  "\n"

  /* NOTE: substring() is broken because it is not UTF-8 aware, searching on bytes rather than characters */
  "TEST: substring() of string " s "\n"
  "substring [1:1]: " substring(s,1,1) "  len=" d(strlen(substring(s,1,1)))"\n"
  "substring [1:2]: " substring(s,1,2) "  len=" d(strlen(substring(s,1,2)))"\n"
  "substring [1:3]: " substring(s,1,3) "  len=" d(strlen(substring(s,1,3)))"\n"
  "substring [1:4]: " substring(s,1,4) "  len=" d(strlen(substring(s,1,4)))"\n"
  "substring [1:5]: " substring(s,1,5) "  len=" d(strlen(substring(s,1,5)))"\n"
  "substring [1:6]: " substring(s,1,6) "  len=" d(strlen(substring(s,1,6)))"\n"
  "substring [1:7]: " substring(s,1,7) "  len=" d(strlen(substring(s,1,7)))"\n"
  "substring [1:8]: " substring(s,1,8) "  len=" d(strlen(substring(s,1,8)))"\n"
  "substring [1:9]: " substring(s,1,9) "  len=" d(strlen(substring(s,1,9)))"\n"
  "substring [2:9]: " substring(s,2,9) "  len=" d(strlen(substring(s,2,9)))"\n"
  "substring [3:9]: " substring(s,3,9) "  len=" d(strlen(substring(s,3,9)))"\n"
  "substring [4:9]: " substring(s,4,9) "  len=" d(strlen(substring(s,4,9)))"\n"
  "substring [5:9]: " substring(s,5,9) "  len=" d(strlen(substring(s,5,9)))"\n"
  "substring [6:9]: " substring(s,6,9) "  len=" d(strlen(substring(s,6,9)))"\n"
  "substring [7:9]: " substring(s,7,9) "  len=" d(strlen(substring(s,7,9)))"\n"
  "substring [8:9]: " substring(s,8,9) "  len=" d(strlen(substring(s,8,9)))"\n"
  "substring [19:20]: " substring(s,19,20) "  len=" d(strlen(substring(s,19,20))) "\n"
  "substring [20:20]: " substring(s,20,20) "  len=" d(strlen(substring(s,20,20))) "\n"
  "string: " s " (20 characters, 35 bytes)\n"


  "\n"
  set(s,"eéée")
  "TEST: substring() '" s "' -- letter e, e with acute, e + composing acute, e\n"
  "      7 bytes, 5 UTF-8 chars, 4 grapheme's (preceived visable characters \n"
  "substring [1:1]: " substring(s,1,1) "  len=" d(strlen(substring(s,1,1)))"\n"
  "substring [1:2]: " substring(s,1,2) "  len=" d(strlen(substring(s,1,2)))"\n"
  "substring [1:3]: " substring(s,1,3) "  len=" d(strlen(substring(s,1,3)))"\n"
  "substring [1:4]: " substring(s,1,4) "  len=" d(strlen(substring(s,1,4)))"\n"
  "substring [1:5]: " substring(s,1,5) "  len=" d(strlen(substring(s,1,5)))"\n"
  "substring [1:6]: " substring(s,1,6) "  len=" d(strlen(substring(s,1,6)))"\n"
  "substring [1:7]: " substring(s,1,7) "  len=" d(strlen(substring(s,1,7)))"\n"
  "substring [1:8]: " substring(s,1,8) "  len=" d(strlen(substring(s,1,8)))"\n"
}
