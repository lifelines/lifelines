/*
 * @progname       fileindex.ll
 * @version        1999
 * @author         Dennis Nicklaus
 * @category       
 * @output         HTML
 * @description

   I have lines on indi's in my database which look like:
   1 NOTE FILE: BIOGRAPHY $FAMHIST/matthews/alkire.bio
   or maybe OBITUARY, MARRIAGE, WILL, NEWS ... instead of BIOGRAPHY.
   and the lowercase letters (matthews/alkire.bio in this example)
   will change to reflect the location of the file in question.

   The purpose of this report is to make an index for these files.
   Each entry looks something like:
   <a href=momI83.html> ALKIRE, James Denton</a>  : <a href=matthews_alkire_bio.txt> matthews_alkire_bio</a> <br>
   referencing my page for the individual and the file which has the article
   in it. (I had to change the file naming scheme from my local disk to the
   place where I have my files served to the WWW (geocities used to not allow
   subdirectories).

   The files are grouped by type (e.g. BIO, OBIT, ...) and then
   are sorted alphabetically by the individual's surname within each grouping.

   Probably not generally useful to anyone else, but shows one thing
   that can be done.  
*/



proc main()
{
   indiset(obitset)
   indiset(marrset)
   indiset(otherset)
   indiset(bioset)
   indiset(willset)

   table(obittab)
   table(marrtab)
   table(othertab)
   table(biotab)
   table(willtab)


   print("patience please, you have a lot of data\n")
    forindi (person, pnum) {
      fornotes(inode(person),note){
	set (i, index(note,"FILE:",1))
	if (gt(i,0)){


	/*   Get the filename. lifted from html.dn */

		set(what,save(substring(note,add(i,6),strlen(note))))
		set (i, index(what," ",1))
		set(descrip,save(substring(what,1,i)))

		/* now get and flatten the file name */
		set (i, index(what,"FAMHIST/",1))
		set (fname,save(substring(what,add(i,strlen("FAMHIST/")),strlen(what))))o
		set (slash, index(fname,"/",1))
		while (gt(slash,0)){
		set(fnameb,save(concat(concat(substring(fname,1,sub(slash,1)),"_"),
					substring(fname,add(slash,1),strlen(fname)))))
			set(fname,fnameb)
			set (slash, index(fname,"/",1))
		}
		set (slash, index(fname,".",1))
		while (gt(slash,0)){
			set(fnameb,save(concat(concat(substring(fname,1,sub(slash,1)),"_"),
						substring(fname,add(slash,1),strlen(fname)))))
			set(fname,fnameb)
			set (slash, index(fname,".",1))
		}
	/* filename is now complete except for adding .txt on the end of it */



	/* now figure out which table things go in */

  	  set (i, index(note,"OBITUARY",1))
	  if (gt(i,0)){
	    addtoset(obitset,person,0)
 	    list(temp)
	    if (lookup(obittab, key(person))){
		set(temp,lookup(obittab, key(person)))
	    	enqueue(temp,fname)
	    } else {
	    	enqueue(temp,fname)
	    }
	    insert(obittab, save(key(person)), temp)
          }
  	  set (j, index(note,"MARRIAGE",1))
	  if (gt(j,0)){
	    addtoset(marrset,person,0)
 	    list(temp)
	    if (lookup(marrtab, key(person))){
		set(temp,lookup(marrtab, key(person)))
	    	enqueue(temp,fname)
	    } else {
	    	enqueue(temp,fname)
	    }
	    insert(marrtab, save(key(person)), temp)
          }
  	  set (k, add(index(note,"BIOGRAPHY",1),index(note,"HISTORY",1)))
	  if (gt(k,0)){
	    addtoset(bioset,person,0)
 	    list(temp)
	    if (lookup(biotab, key(person))){
		set(temp,lookup(biotab, key(person)))
	    	enqueue(temp,fname)
	    } else {
	    	enqueue(temp,fname)
	    }
	    insert(biotab, save(key(person)), temp)
          }
  	  set (m, index(note,"WILL",1))
	  if (gt(m,0)){
	    addtoset(willset,person,0)
 	    list(temp)
	    if (lookup(willtab, key(person))){
		set(temp,lookup(willtab, key(person)))
	    	enqueue(temp,fname)
	    } else {
	    	enqueue(temp,fname)
	    }
	    insert(willtab, save(key(person)), temp)
          }
	  if (eq(add(add(add(i,j),k),m),0)){
	    addtoset(otherset,person,0)
 	    list(temp)
	    if (lookup(othertab, key(person))){
		set(temp,lookup(othertab, key(person)))
	    	enqueue(temp,fname)
	    } else {
	    	enqueue(temp,fname)
	    }
	    insert(othertab, save(key(person)), temp)
          }
        }
      }
    }
    /* now sort and print things out */
   print("uniquing\n")
/*    uniqueset(obitset)
    uniqueset(marrset)
    uniqueset(otherset)
    uniqueset(bioset)*/
   print("sorting\n")
    namesort(obitset)
    namesort(marrset)
    namesort(otherset)
    namesort(bioset)
    namesort(willset)
       print("printing\n")
    call intro()
    "<center>                                 Have Obituaries for :</center><br>\n"
    forindiset(obitset,person,i,j) {
	call nameout(person)
 	forlist(lookup(obittab, key(person)),newfile,n){
  	  " : <a href=" newfile ".txt> " newfile "</a> "
	}
	"<br>\n"
    }
    "<center>                       Have marriage articles for :</center><br>\n"
    forindiset(marrset,person,i,j) {
	call nameout(person)
 	forlist(lookup(marrtab, key(person)),newfile,n){
  	  " : <a href=" newfile ".txt> " newfile "</a> "
	}
	"<br>\n"
    }
    "<center>                Have Biographical or historical articles for :</center><br>\n"
    forindiset(bioset,person,i,j) {
	call nameout(person) 
 	forlist(lookup(biotab, key(person)),newfile,n){
  	  " : <a href=" newfile ".txt> " newfile "</a> "
	}
	"<br>\n"
    }
    "<center>                             Have Wills for :</center><br>\n"
    forindiset(willset,person,i,j) {
	call nameout(person)
 	forlist(lookup(willtab, key(person)),newfile,n){
  	  " : <a href=" newfile ".txt> " newfile "</a> "
	}
	"<br>\n"
    }
    "<center>                                 Have Other info for :</center><br>\n"
    forindiset(otherset,person,i,j) {
	call nameout(person)
 	forlist(lookup(othertab, key(person)),newfile,n){
  	  " : <a href=" newfile ".txt> " newfile "</a> "
	}
	"<br>\n"
    }
   call end()   
}

proc nameout(person)
{
	"<a href=" database() key(person) ".html> "
	fullname(person,1,0,999)
	"</a> "
}
proc intro()
{
    set(db_owner, getproperty("user.fullname"))
    set(owner_email, concat("mailto:",getproperty("user.email")))
 "<html>\n"
 "<title>" db_owner " Genealogy Article Index</title>\n"
 "<META NAME=\"keywords\" CONTENT=\"genealogy, obituary, index\" >\n"
 "<center> <h1>Family Article Index</h1></center>\n"
 "<center> <a href=\"" owner_email "\">" db_owner " " owner_email "</a><br></center>\n"
 "<p>\n"
 "This is an index of the various obituaries, biographies, wedding announcements, \n"
 "wills, etc. that I have, sorted into those categories.  Selecting the name\n"
 "of the person will take you to that person's page.  Following the person's\n"
 "name is a filename or list of filenames which are the articles for that person.\n"
 "Selecting the article filename will take you directly to it.\n"
 "<p> Some of the persons on this list may not have a personal page if they are \n"
 "of a generation not included here, or if they are only related to me by marriage.\n"
 "But the article should still be present.\n"
 "So if you click on a person and don't go anywhere interesting, it's OK.\n"
 "But let me know if any of the article links are invalid.\n"
 "<hr><p>"
}
proc end()
{
"<center><b>\n"
"This page hosted by <a href=\"/\"><img src=/pictures/gc_icon.gif align=middle alt=\"GeoCities\" border=0></a>\n"
"Get your own <a href=\"/\">Free Home Page</a></b></center>\n"
"<br><br>\n"
}
