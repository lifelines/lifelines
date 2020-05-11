/*
 * @progname    related.ll
 * @version     2010-05-10
 * @author      Eugene Reimer (ereimer@shaw.ca)
 * @category
 * @output      Text
 * @description

Shows all the ways two individuals are related (genetically), and their 
Relatedness (as defined by Sewall Wright) which is the expected fraction 
of their DNA that's identical by descent; for example, parent and child 
have relatedness 0.5, grandparent and grandchild 0.25, half-siblings 
0.25, full-siblings 0.5.

Each A-to-B path through a common-ancestor consisting of N parent-child
links contributes 0.5^N;  contributions are summed over all such paths.

Other similar Lifelines reports:
genetics: same definition of relatedness, but wrong/incomplete results;
genetics2: same definition of relatedness, correct, extremely slow;
cons: slightly different definition, often gets same answer;
  (cons applies an "inbreeding correction-factor" for a common-ancestor 
  whose parents are related, this program doesn't);
cousins: no relatedness-metric, slow;
--all of the above produce less complete and/or less readable output;
relate: very different notion of relatedness;
relation: very different notion of relatedness.

For examples comparing the above report-programs on several testcases, 
and more info, see: http://ereimer.net/genealogy/index.htm.

See lines containing "OPTIONS" below for instructions on configuring the 
options, and eliminating the first prompt.

Although based on cons.ll by Arthur.Teschler@uni-giessen.de, this 
program reflects my decisions, and the bugs are my fault.

Copyright (c) 2006,2010 Eugene Reimer;  can be used, modified, copied, 
distributed under the terms of either the MIT-License or the GPL; see 
http://www.opensource.org/licenses/mit-license.php and/or 
http://www.gnu.org/licenses for the details of these terms.
/**/

global(Aancset)			/*set of A's ancestors*/
global(Bancset)			/*set of B's ancestors*/
global(ABancset)		/*set of A&B's common ancestors*/
global(Path)			/*current path from A to B, as a list*/
global(Pathstack)		/*stack of paths for later output*/
global(Cancstack)		/*stack of common-ancestors, companion to Pathstack*/
global(PDS)global(OPB)global(OPK)
global(UNSAFE)

proc main(){			/*==OPTIONS and CONFIGURABLE CONSTANTS==*/
  set(PDS,"G")			/*Path-Description-Style:  "G" for Great-aunt/Great-grandparent style;  "R" for Removed style;  "N" for Numeric style*/
  set(OPB,0)			/*Whether to show Date-of-Birth for each person in addition to name;  0 for false;  1 for true  (is "B" in option-prompt)*/
  set(OPK,0)			/*Whether to show KEY           for each person in addition to name;  0 for false;  1 for true  (is "K" in option-prompt)*/
  set(WID,40)			/*Width in characters of each column in the two-column output, to which are added adjustments for the OPB and OPK options*/
  set(ADB,10)			/*Additional Width when OPB in effect*/
  set(ADK,8)			/*Additional Width when OPK in effect*/
  set(FRAC,0.5)			/*Fraction of WDTH by which to indent the topmost person, the common-ancestor (WDTH is WID+ADB*OPB+ADK*OPK)*/
  set(DATEFMT,"1,4,2,11,0,7")	/*Format for date as 6 comma-separated-numbers for: day, month, year, dateformat, eraformat, complex;  see section-2.17 in reportmanual*/
  set(UNSAFE," !\"#%&'()*+,./:;<=>?@[\\]^`|~")		/*characters illegal in filenames in Windows-NTFS, Windows-FAT32, Windows-FAT16, Mac-OSX, or Unix*/
  set(OP,"")
  if(1){			/*==OPTIONS-PROMPT: USE 1 TO BE PROMPTED; 0 TO USE THE DEFAULTS ABOVE==*/
    print("\nYou can supply several one-letter options:\nG or R or N for the path-description style, G for the default great aunt etc style, R for removed style, ",
      "N for numeric;\nB to have date-of-birth shown for each person;\nK to have the key shown for each person;\nhint: enter nothing to get the default style.\n",
      "(See the script for instructions on configuring the options you want and eliminating this prompt.)\n")
    getstr(OP,"enter [G/R/N] for path-style [B][K] for optional info (see above)")
  }
  set(J,1)while(le(J,strlen(OP))){					/*for each character in OP-string*/
    set(C,upper(substring(OP,J,J)))
    if(index("GRN",C,1)){set(PDS,C)} elsif(eq(C,"B")){set(OPB,1)} elsif(eq(C,"K")){set(OPK,1)} elsif(eq(C,"-")){set(OPB,0)set(OPK,0)} 
    else{print("unknown option:",C," ignored",nl())}
    incr(J)
  }
  set(WDTH,add(WID,mul(ADB,OPB),mul(ADK,OPK)))				/*compute width in characters of each column in the two-column output*/
  dateformat6(DATEFMT)							/*set format to be used for date (date-of-birth if OPB in effect)*/
  getindi(A,"1st person:")  set(nmA, deSpaceEtc(name(A,0)))		/*Get person A*/
  getindi(B,"2nd person:")  set(nmB, deSpaceEtc(name(B,0)))		/*Get person B*/
  newfile(concat("LLrelated",OP,"-",nmA,"-",nmB,".txt"), 0)		/*filename with options and despaced names*/
  OUT "Results for: " name(A,0) nl()
  OUT "             " name(B,0) nl() nl()
  list(Pathstack)  list(Cancstack)
  indiset(Aancset)  addtoset(Aancset,A,0)  set(Aancset,ancestorset(Aancset))  addtoset(Aancset,A,0)
  indiset(Bancset)  addtoset(Bancset,B,0)  set(Bancset,ancestorset(Bancset))  addtoset(Bancset,B,0)
  set(ABancset, intersect(Aancset,Bancset))
  if(lengthset(ABancset)){						/*2010-03-06: safer to use lengthset despite its being deprecated*/
    list(Path)  call FindFullPaths(A,0,B)				/*collect all A-to-B paths on Pathstack*/
  }
  list(L)  forlist(Pathstack,P,C){					/*prepare L to sort by path-length||keys-of-persons-in-path*/
    set(K,rjustify(d(length(P)),6))
    forlist(P,p,c) {set(K,concat(K,"-",key(p)))}
    push(L,K)
  }
  set(L2,dup(L))  rsort(Pathstack,L)  rsort(Cancstack,L2)		/*sort by L;  using rsort since will be removing with pop*/
  print(nl(),nl(),"Related-Report written to: ", outfile(), nl())	/*msg-to-screen showing filename*/
  set(sum, Rat(0,0))							/*print and sum Paths from Pathstack*/
  set(pathcnt, 0)
  while(length(Pathstack)){
    incr(pathcnt)
    set(Canc, pop(Cancstack))
    set(Path, pop(Pathstack))
    set(len,  sub(length(Path),1))					/*length one too long was cause of spurious halving*/
    set(term, Rat(1,len))						/*relatedness for this path*/
    set(CA,indlist(Path,Canc)) set(anc2A,revlist(sublist(Path,1,sub(CA,1)))) set(anc2B,sublist(Path,add(CA,1),length(Path)))	/*split Path into Half-Paths*/
    set(lenA,length(anc2A)) set(lenB,length(anc2B))  set(NL,max(lenA,lenB))
    set(W,0) set(J,1) while(le(J,lenA)) {set(W,max(W,strlen(PersJ(anc2A,J)))) incr(J)}	/*get length of longest name in first column*/
    set(W, max(add(W,2), WDTH))								/*set W to larger of WDTH or 2 more than longest*/
    set(INDA,int(add(mul(W,FRAC),0.5)))							/*compute common-ancestor-indentation*/
    OUT rat2str(term,0) ": " HowRelated(lenA,lenB,male(A)) ":" nl()			/*show contribution plus HowRelated as Mth cousin N-times removed*/
    OUT pad("",INDA) Pers(Canc) nl()							/*show common-ancestor roughly centered*/
    set(J,1) while(le(J,NL)) {OUT pad(PersJ(anc2A,J),W) PersJ(anc2B,J) nl() incr(J)}	/*show half-paths side-by-side*/
    nl()
    set(sum, addrat(sum,term))
  }
  OUT   "============" nl()
  OUT   "Relatedness: "  rat2str(sum,1) "  (" d(pathcnt) " different paths)" nl() nl()	/*show Relatedness, and number of paths*/
  print("Relatedness: ", rat2str(sum,1),"  (",d(pathcnt)," different paths)",nl(),nl())	/*msg-to-screen showing Relatedness, and number of paths*/
}

proc FindFullPaths(current,Canc,target){	/*find all Paths from current to target, saving them in Pathstack;  uses precalculated sets ABancset and Bancset*/
  print(".")					/*debug progress-indicator*/
  push(Path,current)
  if(eq(current,target)){			/*saving when a complete to-target path found*/
    /*print("!")				/*debug progress-indicator*/
    if(not(Canc)){if(inset(ABancset,current)){set(Canc,current)}}	/*so Canc is set even when it is the target!!*/
    push(Pathstack,dup(Path))			/*now pushing list-of-INDIs -- was list-of-KEYs-as-separate-elements*/
    push(Cancstack,Canc)			/*now pushing INDI -- was KEY*/
    pop(Path)
    return()
  }
  if(not(Canc)){				/*We are ascending*/
    if(father(current)) {call FindFullPaths(father(current),0,target)}
    if(mother(current)) {call FindFullPaths(mother(current),0,target)}
    if(inset(ABancset,current)) {set(Canc, current)}
  }
  if(Canc){					/*We have found a common ancestor, now we check for descendants*/
    families(current,curfam,spouse,cnt){
      children(curfam,curchild,cnt){
        if(not(inlist(Path,curchild))){
          if(inset(Bancset,curchild)){		/* <- speeds up!*/
            call FindFullPaths(curchild,Canc,target)
          }
        }
      }
    }
  }
  pop(Path)
}

func HowRelatedN(lenA,lenB,mA){					/*N-style HowRelated-description as 2 numbers, the half-path lengths*/
  return(concat("pathlength:",d(lenA),"+",d(lenB)))
}
func HowRelatedR(lenA,lenB,mA){					/*R-style How THEY're Related as (emptystring / siblings / Mth cousins) [N-times removed]*/
  set(m,min(lenA,lenB))  set(n,sub(max(lenA,lenB),m))		/*m is the shorter half-length, n the difference between that and the longer*/
  if(and(eq(m,0),eq(n,0))){set(R,"identity")}			/*identity only when there's no removed*/
  elsif(eq(m,0)){set(R,"")}					/*emptystring with removed*/
  elsif(eq(m,1)){set(R,"siblings")}				/*siblings even with removed*/
  else          {set(R,concat(ordn(sub(m,1))," cousins"))}	/*Mth cousins, using terse ord*/
  if(ge(n,1)){if(nestr(R,"")){set(R,concat(R," "))}}		/*add a space but not onto emptystring*/
  if(ge(n,1)){set(R,concat(R,multiplicative(n)," removed"))}	/*add N-times removed*/
  return(R)
}
func HowRelatedG(lenA,lenB,mA){					/*G-style How A Related-to B in common language with great grandparent, great aunt, etc (2010-03-05)*/
  set(m,min(lenA,lenB))  set(n,sub(max(lenA,lenB),m))		/*m is the shorter half-length, n the difference between that and the longer*/
  if(and(eq(m,0),eq(n,0))){					/*identity*/
    set(R,"identity")
  }elsif(eq(m,0)){						/*[Nth] [great] [grand]parent/child*/
    if(gt(lenA,lenB)){set(R,"child")}else{set(R,"parent")}
    if(ge(n,2)){set(R,concat("grand",R))}
    if(ge(n,3)){set(R,concat("great ",R))}
    if(ge(n,4)){set(R,concat(ordn(sub(n,2))," ",R))}
  }elsif(and(eq(m,1),eq(n,0))){					/*sibling*/
    set(R,"sibling")
  }elsif(eq(m,1)){						/*[Nth] [great] aunt/uncle/nephew/niece*/
    if(gt(lenA,lenB)) {if(mA){set(R,"nephew")}else{set(R,"niece")}}
    else              {if(mA){set(R,"uncle" )}else{set(R,"aunt" )}}
    if(ge(n,2)){set(R,concat("great ",R))}
    if(ge(n,3)){set(R,concat(ordn(sub(n,1))," ",R))}
  }else{							/*Mth cousin [N-times removed]*/
    set(R,concat(ordn(sub(m,1))," cousin"))			/*which ord??*/
    if(ge(n,1)){set(R,concat(R," ",multiplicative(n)," removed"))}
  }
  return(R)
}
func HowRelated(A,B,C){if(eq(PDS,"N")){return(HowRelatedN(A,B,C))} if(eq(PDS,"R")){return(HowRelatedR(A,B,C))} return(HowRelatedG(A,B,C))}	/*PDS-option:N/R/G*/
func PersJ(L,J){if(le(J,length(L))){return(Pers(getel(L,J)))} return("")}
func Pers(P){							/*format person-info with name, and extra fields as indicated by OPB and OPK*/
  set(S,name(P,0))						/*always show NAME of person*/
  if(OPK){set(S,concat(S," ",key(P)))}				/*OPK-option to also show KEY of person*/
  if(OPB){set(S,concat(S," ",stddate(birth(P))))}		/*OPB-option to also show DOB of person*/
  return(S)
}
func deSpace(S)   {return(subst(S," ",""))}			/*remove spaces from string*/
func deSpaceEtc(S){						/*remove spaces, dots, and other "unsafe" (in filename) characters from string*/
  set(J,1)while(le(J,strlen(UNSAFE))) {set(S,subst(S,substring(UNSAFE,J,J),"")) incr(J)}	/*for each character in UNSAFE-string, remove it from S*/
  return(S)
}

/*RATIONAL DATATYPE -- stored as floating-point*/
func Rat(T,B)   {return(div(T,exp(2.0,B)))}			/*Construct rational T / 2^B */
func addrat(A,B){return(add(A,B))}				/*Add rationals*/
func mulrat(A,B){return(mul(A,B))}				/*Multiply rationals*/
func rat2str(A,BOTH){						/*Rational-to-string; 2nd param TRUE for both vulgar and decimal-fraction-approximation*/
  set(T,A)  set(B,1.0)  while(ne(T,int(T))) {set(T,mul(T,2.0)) set(B,mul(B,2.0))}	/*convert float to rational (tho w/o taking log2 of B)*/
  if(or(ne(B,int(B)),eq(int(B),0))){				/*showing as vulgar-fraction won't work, so show only as decimal-fraction*/
    set(str,g(A))						/*decimal-fraction in "g" meaning either "f" or "e" format*/
  }else{							/*show as vulgar-fraction, optionally also as decimal-fraction*/
    set(T,int(T))  set(B,int(B))
    set(L,6) if(lt(A,0.0001)){set(L,10)}			/*nbr of fractional-digits either 6 or 10*/
    set(str, d(T))						/*numerator*/
    if(T)    {set(str,concat(str,"/",d(B)))}			/*add slash and denominator when nonzero*/
    if(BOTH) {set(str,concat(str," = ",f(A,L)))}		/*BOTH => also as decimal-fraction; (avoid percent-sign to print-routine??)*/
  }
  return(str)
}

/*==============================================*/
/*CANDIDATES for inclusion in LL-Report-Language*/
/*==============================================*/
func type(X){					/*receives ANY, returns string naming argument's type, one of "INT", "STRING", "INDI", etc*/
  set(T,pvalue(X))  return(substring(T,3,sub(index(T,",",1),1)))
}
func EQ(X,Y){					/*compare ANY to ANY-of-same-type, using eqstr for STRING, eq for anything else*/
  if(nestr(type(X),type(Y))){return(0)}  if(eqstr(type(X),"STRING")){return(eqstr(X,Y))}  return(eq(X,Y))
}
func indlist(L,E){				/*index for lists; same as inlist except it returns the position if found; uses EQ to handle any type*/
  forlist(L,e,k){if(EQ(e,E)){return(k)}}  return(0)
}
func sublist(L,B,E){				/*substring for lists*/
  list(l)  forlist(L,e,k){if(and(ge(k,B),le(k,E))){push(l,e)}}  return(l)
}
func revlist(L){				/*reverse a list*/
  list(l)  forlist(L,e,k){requeue(l,e)}  return(l)
}
func max(A,B){if(ge(A,B)){return(A)} return(B)}	/*larger of two integers  (builtin could handle 2 to 32 arguments)*/
func min(A,B){if(le(A,B)){return(A)} return(B)}	/*smaller of two integers (builtin could handle 2 to 32 arguments)*/
func g(R){					/*floating-point-to-string in "f" format if it fits, resorting to "e" format for very big or very small number*/
  if(and(ge(R,0.000000001),lt(R,10000000000))){return(f(R,10))}	/*produce f-format for not-too-big not-too-small number*/
  set(S,1)if(lt(R,0.0)){set(S,-1)set(R,neg(R))}			/*remember the sign*/
  set(E,0)
  while(lt(R,0.5        )){decr(E)set(R,mul(R,10.0))}		/*normalize for very small number*/
  while(ge(R,10000000000)){incr(E)set(R,div(R,10.0))}		/*normalize for very big number*/
  return(concat(f(mul(R,S),6),"e",d(E)))			/*produce e-format*/
}
func dateformat6(FMT){				/*receives 6 comma-separated-numbers for: day, month, year, dateformat, eraformat, complex;  eg: 1,4,2,11,11,7*/
  list(A) while(strlen(FMT)) {push(A,atoi(FMT)) if(index(FMT,",",1)){set(FMT,substring(FMT,add(index(FMT,",",1),1),strlen(FMT)))} else{set(FMT,"")}}
  dayformat(getel(A,1))monthformat(getel(A,2))yearformat(getel(A,3))dateformat(getel(A,4))eraformat(getel(A,5))complexformat(getel(A,6))  /*eraformat(11) broken??*/
}						/*note: linesrc config-file has LongDisplayDate, ShortDisplayDate, but builtin date-functions don't honour those??*/
func pad(S,N){					/*pad string to specified length; should be named ljustify but wanted a short name*/
  while(lt(strlen(S),N)){set(S,concat(S," "))} return(S)
}
func subst(S,A,B){				/*replace in string S each occurence of A by B (sed-like regular-expression-based replacements also desirable)*/
  while(J, index(S,A,1)) {set(S, concat(substring(S,1,sub(J,1)), B, substring(S,add(J,strlen(A)),strlen(S))))}
  return(S)
}
func ordn(n){					/*nth ordinal-number in terse numeric style, and "corrected" for 21st, 22nd, 23rd, etc*/
  set(e,mod(n,10))set(ee,mod(n,100))
  set(E,"th") if(eq(e,1)){if(ne(ee,11)){set(E,"st")}} if(eq(e,2)){if(ne(ee,12)){set(E,"nd")}} if(eq(e,3)){if(ne(ee,13)){set(E,"rd")}}
  return(concat(d(n),E))
}
func ordx(n){					/*nth ordinal-number as words to twelfth, just like builtin ord but "corrected" for 21st, 22nd, 23rd, etc*/
  if(le(n,20)){return(ord(n))}  return(ordn(n))
}
func multiplicative(n){				/*nth multiplicative-number once, twice, thrice, etc;  to go with card, ord, ordn*/
  if   (eq(n,1)){return("once")}
  elsif(eq(n,2)){return("twice")}
  elsif(eq(n,3)){return("thrice")}
  else          {return(concat(d(n),"-times"))}	/*suspect "-times" more readable than "x"*/
}
/*string-comparisons to complete the set lifelines began with eqstr and nestr;  consider camelcase-names since neStr is more readable than nestr??*/
func ltstr(a,b) {return(lt(strcmp(a,b),0))}	/*string-comparison to go with eqstr and nestr*/
func lestr(a,b) {return(le(strcmp(a,b),0))}	/*string-comparison to go with eqstr and nestr*/
func gestr(a,b) {return(ge(strcmp(a,b),0))}	/*string-comparison to go with eqstr and nestr*/
func gtstr(a,b) {return(gt(strcmp(a,b),0))}	/*string-comparison to go with eqstr and nestr*/
