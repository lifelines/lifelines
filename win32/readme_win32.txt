readme_win32.txt 08 Jan 2000 by Paul B. McBride (pmcbride@tiac.net)

This file contains notes on the Open Source version of LifeLines (3.0.3)
to which I have added the changes that I made to fix bugs, add features,
and run on Windows95 and Windows NT.

- The version number reported by the LifeLines program is 3.0.5.
- The files in the win32 directory are the files that I use to make
  the Windows95 and WindowsNT version of Lifelines. I do not use
  a Makefile for this. I use a Borland C++ *.ide file to build that version.
  The sources should still compile on UNIX systems. To get the Windows
  version define "WIN32". You also get some changes and features that should be
  added to the UNIX versions as well. In particular, the use of "system()"
  is replaced by an appropriate function to perform the operation.

  To make the original Windows version I used the *.h files in the directory
  "win32/hdrs". Essentially what this does is to include all of the
  include files for lifelines in each source file. It also contains a
  "proto.h" file with function prototypes for most of the functions in
  the lifelines source.

Changes:
  - The GEDCOM spec specifies 1 to 22 character cross references, @xxxx@.
    I now allow 31 characters, and will truncate cross references
    longer than 31 characters.
  - Many others (see notes on win32 version below)
Bugs?:
  - When merging I think that is still possible to end up with a GEDCOM key in
    a record in the database which is not a key in the database. Under some
    condition (rare), not all of the the cross references are updated
    to the new key. This will cause a FATAL error and program exit when
    you browse to the person or family which contains this error.
    This problem is not new to this release. I has existed since 3.0.2.
  - The temporary file created for editing might not get deleted.
Recommended future work:
  - The recommended extension of LifeLines report programs should be ".ll".
    When searching for a report both the exact name as typed in,
    and the name with ".ll" appended should match. The ".ll" is not required
    to be typed, or to be part of the name of the file in the report
    directory.
  - In general the generation of FATAL errors (ASSERT) should be checked,
    and be less fatal if possible.
  - The functions in interp/write.c are not used and are not correct.
    I believe that they have to do with making changes to the database
    from report programs (dangerous!).

The following are older notes on the beta versions of my changes:

readme_win32: Notes on changes to LL303B2  28-Feb-99  Version: 3.0.3-Win32j

See readme.txt for important information on LL303B2.

Hi,

Here are some notes on changes to the LL303 Beta 2 distribution.

I received the LL303B2 distribution from Tom Wetmore in about August, 1996.
I found a couple problems with this version, and reported them, and then
went back to using LL302. In about November, I found that I was mainly
using Windows NT and Windows 95 systems, and it would be more convenient
for me to use those systems for LifeLines rather than the Novell/SCO UnixWare
systems which I had been using. Although I knew that there was a
DOS version of LifeLines 3.0.2, I thought that I would try to make
the LL303B2 version work on Windows 95, fixing the problems that I
had encountered when I tried this version, previously. Anyway, here is
the result.

The notes on changes are divided into the following categories:

	Changes since previous version
	Notes for Users on Windows 95/NT
	Command Line Options
	Development Environment
	Cosmetic Changes
	Curses Library for Windows 95/NT
	Changes for Windows 95/NT
	User Interface Changes
	Report Language Changes
	Translation Changes
	Finnish Sorting Order Option
	Miscellaneous Changes
	Remaining Bugs

Changes since previous version
    Version: Win32j
	In pedigree charts, if a birth or death date consisted of
	only a 3 digit year, it was not displayed. It would be displayed
	if it had a prefix.
	For example a bith date of "ABT 720" would be displayed
	as "[720-]", but a birth date of "720" would not be displayed,
	"[-]". The problem was in node.c:shorten_date() which did
	an initial check for a date string less than 4 characters.
	This was changed to check for less than 3 characters, to
	fix the problem. Note years less than 100 will not be
	displayed. You can use leading zeros to force them to be
 	displayed, e.g. 099, or 0099.

	When describing a person, include information about
	a RESIdence on the "born:" line, if no place is shown.
	The RESIdence will be prefixed with " of ".

	When describing a person, show the person's TITLe in
	square brackets, if they have one.

	When the database contains a cross-reference for an
  	person the program would core dump or exit when trying
	to display information about that person on the screen
	with a "FATAL keynode.c ..." error message. In the case
	that an individual or a family is being displayed when the
	missing cross-reference "key" is accessed, a warning will be 
	displayed at the bottom of the screen giving the "key".
	This key probably appears in either the current family, or
	the person's parents, or marriage families. The database
	must be manually modified using btedit, or a GEDCOM
   	backup file can be saved, and edited and loaded into a new
	empty database.

	When 5 generations of a pedigree fit on the screen, the
	5th generation name and key info could overlap the border
	of the window and be continued on the next line. This has
	been corrected.


Version: Win32i
      Support screen sizes form 24 to 40 lines. When the screen size
      is 40 lines you also get one more generation in the pedigree screen.
      I have used this with 40 lines on both Windows95 and WindowsNT.
      My minimal curses routines limit the size to a maximum of 40 lines.
      If the LifeLines source is compiled with a normal curses library
      it will not have this limitation, as long as that curses library
      supports the larger variable screen size.
      Note that the screen size must be set before the program starts.
      It is not changed if the program is already running.

      When showing a list of people to select from, an indicator is
      appended to show if the person could have parents "P" (has a FAMC record)
      or could have a spouse "S" (has a FAMS record). This will show up
      as the normal information with one of the following appended:
      	[PS]	has both FAMC and FAMS records
      	[S]	has a FAMS record (is a spouse in a family)
      	[P]	has a FAMC record (is a child in a family)

      Decreased the maximum length of the info displayed in a list from
      70 to 68 characters so it would not overlap the window border.

    Version: Win32h

      The ord() report builtin function had a couple of problems:

	1) If the argument value is greater than 13, the wrong value is printed.
	2) If the argument value is 13, an entry outside an array of string
	   pointers will be used, with unpredictable results.

	The patch to interp/builtin.c __ord() function is as follows:

	<       if (i > 13)
	<               sprintf(scratch, "%dth", val);
	---
	>       if (i > 12)
	>               sprintf(scratch, "%dth", i);

    Version: Win32g
    	valuesort() builtin readded as described by John F. Chandler (untested).

	Prior to this version, if you chose to preserve xref keys when
	you loaded the database from a GEDCOM file, and there were deleted
	records, the forindi, forfam, forsour, forothr, and foreven
	functions would not return each of the records of the specified
	type. They would stop at the first deleted record of that type
	in the database. This has been fixed.

    Version: Win32f
    	"1 SEX" with no value field caused crash. Fixed.

Notes for Users on Windows 95/NT
	The directory c:\tmp is used for temporary files.
	The environment variables can be set in the autoexec.bat
	file as in the following examples:

		set EDITOR=vim.exe
		set LLPROGRAMS=.;c:\lines\llib;c:\lines\reports

	The ";" character is used to separate directories to be searched
	(as with PATH) rather than ":" as for the UNIX Bourne Shell.

	You may use the Windows 95 NOTEPAD or EDIT as the editor.
	I use VIM which is very similar to VI. The default editor
	is VI.

Command Line options:

    -c[{ifesox}direct,indirect]...
    	This option allows you to set the size of the caches for
	Individuals, Families, Events, Sources, and Others. Usually
	changing the size of the Individual and Family caches
	would be all that you might want to change. For example
	the following shows the default settings for the
	Individual and Family caches:

		-cI200,3000F200,2000

	For my own database (8700I, 4500F) I have been using:

		-cI9000,9000F5000,5000

    -F  Finnish sorting order (only if compiled with FINNISH and
        FINNISHOPTION defined. If only FINNISH is defined, the
	program is always in this mode and there is no -F option.

Development Environment

	I am using Borland C++ 5.00 with its Integrated Developement
	Environment (IDE). To make use of the function prototype
	checking I generated function prototypes for most of the
	subroutines in LifeLines, and then added some dummy include
	files which would always include all of the other include
	files, and the function prototypes.

	Changes specific to Window95/NT are enabled by defining WIN32.

	To maintain binary compatibility with databases created with
	Linux, DOS, and Unixware versions of LifeLines it is necessary
	to compile with double word alignment (-a4), rather than
	the default of byte alignment (-a1).

Cosmetic Changes
	Some changes were made to prevent the Borland compiler from
	producing warning messages. I chose this approach rather
	than just disabling the warning, which could have hidden problems.

Curses Library for Windows 95/NT
	LifeLines uses a very small subset of the full capabilities
	of a curses library, so I wrote my own. Although this can
	be replaced with a more standard curses library, it may
	be of benefit in making a first pass at a LifeLines with a GUI.
	PDcurses seems like a viable alternative. I have tried it out
	on Windows 95 and it works ok, but I haven't tried using it
	with LifeLines.

Changes for Windows 95/NT
    Changes which are specific to Windows 95/NT are turned on by
    defining WIN32.

    Borland C++ 5.02 defines wprintf() in stdio.h This conflicts with
    LifeLines wprintf() which is used to write to a curses window.
    The include files for Win32 redefine wprintf to be llwprintf.

    Files can be read and written in TEXT mode where carriage return
    line feed is converted to and from newline (line feed) as used in
    UNIX, or they can be read and written in BINARY mode where no
    conversion takes place. The database is read/write BINARY, while
    extenal files are read/write TEXT.

    In several places, the system() function was used to run 
    the mkdir command to make a new directory, or the rm command
    to remove files. Rather than this, when WIN32 is defined, the mkdir() 
    or unlink() library routines are used instead.

    To create the temporary file used while editing, the mktemp
    routine is used as follows:

	editfile = strsave(mktemp("\\tmp\\lltmpXXXXXX"));

    The six X's are replaced with 2 characters, a dot, and then
    a three character extension.

    "clock" was changed to "cclock" because of a name conflict.

    "reg" as a local variable has been change to "regorder"

    stdlib/path.c
	remove #include <unistd.h>
    stdlib/signals.c
	SIGHUP, SIGBUS, SIGPIPE, SIGQUIT are not defined
    hdrs/interp.h
	Change iparent() to iiparent() because of conflict with btree.h
    interp/{alloc.c,interp.c}
	Change iparent() to iiparent() because of conflict with btree.h

User Interface Changes
    The screens for individual and family browse now briefly list the
    Advanced Edit (i.e. Advanced View) options, and the new
    tandem browse commands to aid in merging:

    	A	Advanced View (enter editor with cross referneces expanded)
	C	Tandem browse to two children
	F	Tandem browse to two fathers
	G	Tandem browse to two families
	S	Tandem browse to two spouses
	M	Tandem browse to two mothers
	U	Tandem browse to two parents families

    The Advanced View command enters the editor with cross references
    expanded. I find this most useful for looking a source details.
    NOTE: You will not be able to make any changes when in this mode.

    The tandem browse commands and the impovements to merging should
    make merging databases much easier.

    When loading a GEDCOM file into an empty database, you will be
    given the option of using the original keys if they are in the
    standard LifeLines format. This also allows you to reload your
    own saved database, maintaining the original keys.

Report Language Changes

    New builtins:

      BOOLEAN inlist(LIST, ANY)
         Returns TRUE if the ANY (usually a STRING) is in the LIST.

      free(ANY)
         Frees up extra space associated with a variable (STRING, LIST,
	 SET, TABLE).  Avoid if possible, and use with caution.

Translation Changes
    The Internal to GEDCOM translation is now used when saving the
    data base into a GEDCOM file. Note that the translation tables
    themselves are not saved, so always keep copies of your translation
    tables in external files.

    In translation tables blank lines, and lines beginning with
    ## are ignored. Also, anything starting with the second tab on 
    the line is ignored.

    Miscellaneous bugs were corrected.
	
Change to surname() builtin function
    The surname() builtin function no longer uses the getsurname()
    function which is used internally to generate soundex codes
    and record keys. The getsurname() function returns "____" if
    the first character of the surname portion of a GEDCOM NAME
    is not a letter. This is fine for soundex and record keys, but
    is not appropriate for reports. The surname() report builtin function
    will return the surname extracted from between the "/" delimeters
    with leading white space removed. surname() only returns "_____"
    if no surname was specified or there were no non blank characters
    between the "/" delimeters. (W32E)

Finnish Sorting Order
    I have included the changes which were made to support Finnish
    sorting order. Use the compiler option -DFINNISH to include
    these changes. I also added a runtime option -DFINNISHOPTION.
    However, to use the runtime option, you must be aware that your
    database may be corrupted unless you always use the same mode
    when making changes to your database. This is because of changes
    to the Soundex codes for names.

Miscellaneous Changes
     When reading in a GEDCOM file, if a line contains a "1 SEX"
     record with no value, a SEGMENTATION ERROR occurred. This
     has been corrected. It is interpreted to idicate that the
     sex of the individual is not known. (W32F)

     When you get the inevitable FATAL keytonod.c error, the last
     10 keys referenced are displayed. The missing key may be a link
     in one of those records. Also, a report mode has been added
     so that it is possible to have a request for a key fail, without
     having a FATAL error.

     When reading in a GEDCOM files <XXXX> will be converted to
     @YYYY@ where XXXX is a REFN in record YYYY.

     Many changes were made to decrease the amount of memory used.

Remaining Bugs

    Translation
    	The translation table for internal to report is not used
	in all places, so some data included in reports will not be translated.
	This applies to value(), title(), etc.
    Memory Leak
    	A lot more memory seems to used than is necessary. All of the
	data which is allocated for local symbols in procedures is not
	freed when the procedure returns. My own suspicion is that there
	is a memory leak in the individual cache.
    interp/write.c
    	This file has not been updated to use the new PVALUES for
	storing variables. I believe these functions are for modifying
	the database from report programs.

    Extra value stored with individual sets
        When adding an individual to a set you can add a value of
	any type at the same time. When an ancestor set is generated,
	the value is set to the number of generations from the seed
	person. These values can be used in reports. However, internally
	some functions used in browsing set this value to the family
	number. This value cannot be used in a report because it is
	not stored as a PVALUE, which specifies the type and value
	of a variable.
