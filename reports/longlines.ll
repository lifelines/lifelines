/*
 * @progname       longlines
 * @version        1.0
 * @author         Chandler
 * @category       
 * @output         Text
 * @description    
 
longlines

Version 1 - 1994 May 19 - John F. Chandler

Find the maximal-length male and female lineages in the database.

This program works only with LifeLines.

*/
global(len)     /* current lineage length */
global(lenmax)  /* longest lineage found */
global(ends)    /* keys of last persons */
global(linsex)  /* sex of lineage desired */

proc main(){
"Longest lineages in database\n\n   Male"
set(linsex,"M")
call dumplines()
"\n   Female"
set(linsex,"F")
call dumplines()
}

/* scan all offspring matching the sex of the input person, and
   return the longest lineage(s) from those -- if no matching
   offspring, just return the input person as a lineage */
proc getline(indi)
{
set(len,add(len,1))
families(indi,fam,spou,num) {
        children(fam,child,numc) {
                if(eq(0,strcmp(linsex,sex(child)))) {
                        set(found,1)
                        call getline(child)
                }
        }
}
set(len,sub(len,1))
if(and(not(found),ge(len,lenmax))) {
        if(gt(len,lenmax)) {list(ends)}
        enqueue(ends,save(key(indi)))
        set(lenmax,len)
}}

proc dumplines()
{
set(lenmax,0)
print("Starting ", linsex, " ...\n")

/* find all eligible starting points */
/* assume that a nameless person doesn't count */
forindi (indi, num) {
        set(skip,"")
        if(eq(0,strcmp(linsex,"M"))) {set(par,father(indi))}
        else {set(par,mother(indi))}
        if(par) {set(skip,name(par))}
        if(and(eq(0,strcmp(linsex,sex(indi))),eq(0,strcmp("",skip)))) {
                set(len,1)
                call getline(indi)
        }
}
/* report results */
"\n   Maximal length " d(lenmax) "\n"
/* dump each lineage, starting with most recent person */
while(end, dequeue(ends)) {
        "\n"
        set(count, lenmax)
        set(line,indi(end))
        while(gt(count,0)) {
                set(count,sub(count,1))
                name(line) " (" key(line) ")"
                if(x, birth(line)) {" b. " year(x)}
                if(y, death(line)) {
                        if(x) {","}
                        " d. " year(y)
                }
                "\n"
                if(eq(0,strcmp(linsex,"M"))) {set(line,father(line))}
                else {set(line,mother(line))}
        }
}}
