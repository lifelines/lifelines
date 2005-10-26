/*
 * @progname    cons.ll
 * @version     1.0
 * @author      Teschler
 * @category
 * @output      Text
 * @description

Calculates coefficient of inbreeding F(A,B) for the offspring
of two individuals A and B.

The consanguity (blood in common) C(A,B) is 2*F(A,B)
F(A,B) = sum(0.5^(n(i)+p(i)) * (1+F(J(i))/2)
The sum extends over the number of distinct chains of relationship
connecting A and B. The ith chain has n(i)+p(i) links ascending
from A and B to the common ancestor J(i), whose coefficient of
inbreeding is f(J(i)).
A chain of relationship consists of all links leading from A and
B to a common ancestor J, and has no other point in common except
J. Two chains are considered distinct if they differ in at least
one link.

Result goes to file /tmp/t1
This is one of my first LL programs so please do not look
for elegance ;-)

Arthur.Teschler@uni-giessen.de
*/

global(anc_line)        /* holds the current way from A towards B */
global(to_anc)          /* B's ancestors */
global(from_anc)        /* A's ancestors */
global(common_anc)      /* A's and B's common ancestors */
global(common_stack)    /* holds J(i) for later inbreed check */
global(anc_line_stack)  /* holds lines for later output */
global(coefftab)        /* holds inbreed coefficients for J(i) */

func coanc(A,B)
  {
  indiset(from_anc)
  addtoset(from_anc,A,0)
  set(from_anc,ancestorset(from_anc))
  addtoset(from_anc,A,0)

  indiset(to_anc)
  addtoset(to_anc,B,0)
  set(to_anc,ancestorset(to_anc))
  addtoset(to_anc,B,0)

  indiset(common_anc)
  set(common_anc,intersect(from_anc,to_anc))

  list(anc_line)
  if (lengthset(common_anc))
    {
    push(anc_line_stack,"--") /*Marker*/
    if (gt(lengthset(from_anc),lengthset(to_anc)))
      {
      call swap(from_anc,to_anc)
      call swap(A,B)
      }
    call iter(A,0,B)
    /*
       At this point we have collected all paths
       leading from A to B on anc_line_stack
       Now we have to calculate f(J(i)) for all
       common ancestors that are in our list of
       paths (saved on common_stack), then we
       can sum up things.
    */
    while(pers,dequeue(common_stack)) {
      if (not(lookup(coefftab,key(pers,0)))) {
        set(pc,coanc(father(pers),mother(pers)))
        insert(coefftab,key(pers),pc)
        }
      }
    print "Results for :"
    print fullname(A,0,0,50) sp()
    print fullname(B,0,0,50) nl()
    set(result,sum_up())
    }
  else
    {
    set(result,"0 1")
    }
  return(result)
  }

proc iter(current,common,target)
  /* Recursively traverses the tree (better hedge)
     to find all paths leading from current to
     target. Makes use of precalculated sets
     common_anc and to_anc.
     Fills up a list of paths
  */
  {
  print (".")
  push(anc_line,current)
  if (eq(current,target)) {
    call found(common)
    pop(anc_line)
    return()
    }
  if (not(common)) {
    /* We are ascending */
    if (father(current)) {
      call iter(father(current),0,target)
      }
    if (mother(current)) {
      call iter(mother(current),0,target)
      }
    if (iselement(current,common_anc)) {
      set(common,current)
      }
    }
  if (common) {
    /* We have found a common ancestor
       now we check for descendants */
    families(current,curfam,spouse,cnt) {
      children(curfam,curchild,cnt) {
        if (notchecked(curchild)) {
          if(iselement(curchild,to_anc)) {    /* <- speeds up! */
            call iter(curchild,common,target)
            } /* iselement */
          } /* notchecked */
        } /* children */
      } /* families */
    } /* common */
  pop(anc_line)
  }

proc found(common) {
  /* Unfortunately LL pushes references.
     I had liked to push values.
     Now I have to do my own special stack handling.
     Not very elegant, though :(
  */
  print("!")
  push(anc_line_stack,"-")  /*Marker*/
  forlist(anc_line,pers,cnt) {
    push(anc_line_stack,key(pers))
    }
  push(anc_line_stack,key(common))
  push(common_stack,common)
}

func sum_up()
  {
  /*
  pops anc_lines from anc_line_stack and sums
  up their values.
  prints them as a side effect, otherwise there would
  be no need to save all those steps, the length would
  have been enough
  */
  set(sum,"0 1")
  set(lcnt,0)
  set(element,pop(anc_line_stack))
  while(strcmp(element,"--")) {
    incr(lcnt)
    set(common,element)
    print "Common ancestor: " fullname(indi(common),0,0,50) nl()
    set(factor,lookup(coefftab,common))
    if (strcmp(factor,"0 1")) {
       print "(Inbreeding coefficient: " showfrac(factor) ")" nl()
       }
    set(length,0)
    set(pers,pop(anc_line_stack))
    while(strcmp(pers,"-")) {
      incr(length)
      print "   " d(length) " " fullname(indi(pers),0,0,50) nl()
      set(pers,pop(anc_line_stack))
      }
    set(element,pop(anc_line_stack))
    set(factor,addfrac("1 0",factor))
    set(factor, mulfrac( factor,concat("1 ",d(length))))
    print "------------" nl()
    print showfrac(factor) nl() nl()
    set(sum,addfrac(sum,factor))
    }
  print "============" nl()
  print "Sum: " showfrac(sum) "  (" d(lcnt) " different lines)" nl()
  print nl()
  return(sum)
  }

/*
   Some functions to handle fractions follow here.
   Lifelines has no type fraction let's put nominator denominator
   as space separated strings.  As the denominator is always 2^x,
   we put just x
*/

func addfrac(A,B)
  {
  set(nomA,atoi(A))
  set(denA,atoi(substring(A,index(A," ",1),strlen(A))))
  set(nomB,atoi(B))
  set(denB,atoi(substring(B,index(B," ",1),strlen(B))))

  while (lt(denA,denB)) {
    incr(denA)
    set(nomA,mul(nomA,2))
    }
  while (lt(denB,denA)) {
    incr(denB)
    set(nomB,mul(nomB,2))
    }

  set(nomA,add(nomA,nomB))
  while (eq(0,mod(nomA,2))) {
    decr(denA)
    set(nomA,div(nomA,2))
    }

  set(result,concat(d(nomA)," "))
  return(concat(result,d(denA)))
  }

func mulfrac(A,B)
  {
  /* Multiply my funny fractions */
  set(nomA,atoi(A))
  set(denA,atoi(substring(A,index(A," ",1),strlen(A))))
  set(nomB,atoi(B))
  set(denB,atoi(substring(B,index(B," ",1),strlen(B))))
  set(nomA,mul(nomA,nomB))
  set(denA,add(denA,denB))
  while (eq(0,mod(nomA,2))) {
    decr(denA)
    set(nomA,div(nomA,2))
    }

  set(result,concat(d(nomA)," "))
  return(concat(result,d(denA)))
  }

func showfrac(A)
  {
  /* show my funny fractions */
  set(nomA,atoi(A))
  set(denA,atoi(substring(A,index(A," ",1),strlen(A))))
  return(concat(d(nomA),concat("/",d(exp(2,denA)))))
  }


proc swap(V1,V2)
  {
  set(help,V1)
  set(V1,V2)
  set(V2,help)
  }

/* I'm sure there are better ways to handle the following two ... */

func iselement(E,S)
  {
  indiset(test)
  addtoset(test,E,0)
  return (lengthset(intersect(test,S)))
  }

func notchecked(i)
  {
  forlist(anc_line,pers,cnt) {
    if (eq(key(pers,0),key(i,0))) { return (0) }
    }
  return (1)
  }

proc show_stack()
  {
  /* for debugging purposes */
  print "Current:" nl()
    forlist(anc_line,pers,cnt) {
      print "   " d(cnt) fullname(pers,0,0,50) nl()
      }
  }

proc main() {
  getindimsg(from,"1st :")
  getindimsg(to,"2nd :")
  list(common_stack)
  list(anc_line_stack)
  table(coefftab)
  newfile("/tmp/t1",0)
  set(cf,mulfrac("2 0",coanc(from,to)))
  print "Consanguity factor: " showfrac(cf) nl()
}
