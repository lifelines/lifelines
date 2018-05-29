/* From Seppo Sippu */
/* Designed to validate behaviour of various functions with multi-byte character strings */
/* Seppo indicates that these are 2-byte unicode, but on my system they are a mix of 2-byte and 4-byte unicode. */
proc main()
{
  set(s,"A쎥쎤쎶B쎅쎄쎖C")
  "string: " s " (9 characters, 15 bytes)\n"
  "index of the 1st character, A: " d(index(s,"A",1)) "\n"
  "index of the 2nd character, 쎥: " d(index(s,"쎥",1)) "\n"
  "index of the 5th character, B: " d(index(s,"B",1)) "\n"
  "index of the 8th character, 쎖: " d(index(s,"쎖",1)) "\n"
  "index of the 9th character, C: " d(index(s,"C",1)) "\n"
  list(lis)
  extracttokens(s,lis,cnt,"B")
  "extracttokens with separator B: " getel(lis,1) " " getel(lis,2) "\n"
  extracttokens(s,lis,cnt,"쎖")  /* does not work */
  "extracttokens with separator 쎖: " getel(lis,1) " " getel(lis,2) "= \n"
  "substring [1:5]: " substring(s,1,5)
    ", strlen " d(strlen(substring(s,1,5))) "\n"
  "substring [1:6]: " substring(s,1,6)   /* splits a two-byte character */
    ", strlen " d(strlen(substring(s,1,6))) "\n"
  "substring [1:7]: " substring(s,1,7)
    ", strlen " d(strlen(substring(s,1,7))) "\n"
  "substring [1:8]: " substring(s,1,8)
    ", strlen " d(strlen(substring(s,1,8))) "\n"
}
