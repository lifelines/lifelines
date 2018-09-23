/* From Seppo Sippu */
/* Designed to validate behaviour of various functions with multi-byte character strings */
/* Seppo indicates that these are 2-byte unicode, but on my system they are a mix of 2-byte and 4-byte unicode. */
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

  /* NOTE: extracttokens() is broken because it is not UTF-8 aware, searching on bytes rather than characters */
  "TEST: extracttokens()\n"
  list(lis)
  extracttokens(s,lis,cnt,"B")
  "extracttokens with separator B: '" getel(lis,1) "' and '" getel(lis,2) "'\n"
  extracttokens(s,lis,cnt,"쎖")
  "extracttokens with separator 쎖: '" getel(lis,1) "' and '" getel(lis,2) "'\n"
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
  "substring [9:9]: " substring(s,9,9) "\n"
}
