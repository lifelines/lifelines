/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-04-25 J.F.Chandler */
/*=============================================================
 * messages.c -- Holds most LifeLines messages
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 13 Aug 93
 *   2.3.6 - 29 Oct 93    3.0.0 - 05 Oct 94
 *   3.0.2 - 06 Dec 94
 *===========================================================*/

#include "llstdlib.h"

#define SS (STRING)

STRING iddbse = N_("You must identify a database.");
STRING idldir = N_("What directory holds the LifeLines database?");
STRING idldrp = N_("enter path: ");
STRING nodbse = N_("There is no LifeLines database in that directory.");
STRING crdbse = N_("Do you want to create a database there?");
STRING bdlkar = N_("Argument to lock (-l) must be y or n.");
STRING normls = N_("Cannot remove last person from family.");

STRING ronlya = N_("The database is read only; you may not add records.");
STRING ronlye = N_("The database is read only; you may not change records.");
STRING ronlym = N_("The database is read only; you may not merge records.");
STRING ronlyr = N_("The database is read only; you may not remove records.");

STRING idbrws      = N_("enter name, key, refn or list:");
STRING idkyrfn     = N_("enter key or refn: ");
STRING idcrmv      = N_("Please choose the child to remove from the family.");
STRING idsrmv      = N_("Please choose the spouse/parent to remove from the family.");
STRING idcrmf      = N_("From which family is the child to be removed?");
STRING idsrmf      = N_("From which family is the spouse/parent to be removed?");
STRING idfrmv      = N_("Identify family (enter nothing to identify by individual members).");
STRING idfrsp      = N_("Identify a spouse of the family, if known.");
STRING idfrch = SS "Identify a child of the family, if known.";
STRING id1csw = SS "Identify the first child to swap.";
STRING id2csw = SS "Identify the second child to swap.";
STRING idcrdr = SS "Identify the child to reorder.";
STRING id1fsw = SS "Identify the first family/spouse to swap.";
STRING id2fsw = SS "Identify the second family/spouse to swap.";
STRING idsbrs      = N_("Please choose the spouse/parent to browse to.");
STRING id1sbr      = N_("Please choose the first spouse/parent to browse to.");
STRING id2sbr      = N_("Please choose the second spouse/parent to browse to.");
STRING idcbrs      = N_("Please choose the child to browse to.");
STRING id1cbr      = N_("Please choose the first child to browse to.");
STRING id2cbr      = N_("Please choose the second child to browse to.");
STRING idfbrs      = N_("Please choose the family to browse to.");
STRING idfamk      = N_("Enter Family Number to Browse to");
STRING askint      = N_("enter integer:");
STRING id1fbr      = N_("Please choose the first family to browse to.");
STRING id2fbr      = N_("Please choose the second family to browse to.");
STRING idhbrs      = N_("Please choose the father/husband to browse to.");
STRING id1hbr      = N_("Please choose the first father/husband to browse to.");
STRING id2hbr      = N_("Please choose the second father/husband to browse to.");
STRING idwbrs      = N_("Please choose the mother/wife to browse to.");
STRING id1wbr      = N_("Please choose the first mother/wife to browse to.");
STRING id2wbr      = N_("Please choose the second mother/wife to browse to.");
STRING idcswp = SS "Identify a parent in the family having children swapped.";
STRING idfswp = SS "Whose families/spouses are to be swapped?";
STRING idprnt = SS "Identify one of the child's parents, if known.";

STRING scanrs = SS "Scan results:";
STRING scnnmf      = N_("Enter pattern to match against single surname or given name.");
STRING scnfnm      = N_("Enter pattern to match against full name.");
STRING scnrfn      = N_("Enter pattern to match against refn.");
STRING scantt      = N_("pattern: ");

STRING chfamily = SS "Please choose a family.";
STRING notone = SS "Please choose from among these records.";
STRING iscnew = SS "Is this the new child? ";
STRING issnew = SS "Is this the new spouse? ";
STRING ifone  = SS "Is this the person?  Select if so.";
STRING idcfam = SS "Select the child the new child precedes or select last.";
STRING idpnxt = SS "Please identify next person to browse to.";
STRING idnxt  = SS "Please identify record to browse to.";
STRING idspse = SS "Please identify one of the spouses.";
STRING idplst = SS "Please identify person or persons to browse to.";
STRING idfcop = SS "Please choose family create operation.";
STRING entnam = SS "Do you want to enter another name?";

STRING ntprnt      = SS "This person is not a spouse or parent in any family.";
STRING ntchld      = N_("This person is not a child in any family.");
STRING nonamky     = N_("There is no one in the database with that name or key.");

STRING nofath      = N_("This person's father is not in the database.");
STRING nomoth      = N_("This person's mother is not in the database.");
STRING nospse      = N_("This person has no spouse in the database.");
STRING noysib = SS "This person has no younger sibling in the database.";
STRING noosib = SS "This person has no older sibling in the database.";
STRING noprnt = SS "This person's parents are not in the database.";
STRING nohusb = SS "This family has no male spouse/parent in the database.";
STRING nowife = SS "This family has no female spouse/parent in the database.";
STRING nocinf = SS "There are no children in the database for this family.";
STRING nocofp = SS "This person has no children in the database.";
STRING nochil = SS "No such child.";
STRING nopers = SS "No such person.";
STRING norec  = SS "No such record.";
STRING nofam  = SS "No such family.";
STRING nosour = SS "No sources present.";
STRING idsour = SS "Please choose the source to view.";
STRING noeven = SS "No events present.";
STRING ideven = SS "Please choose the event to view.";
STRING noothe = SS "No others present.";
STRING idothe = SS "Please choose the other to view.";
STRING nonote = SS "No notes present.";
STRING idnote = SS "Please choose the note to view.";
STRING noptr  = SS "No references present.";
STRING idptr  = SS "Please choose the reference to view.";
STRING duprfn = SS "Duplicated REFN - please choose.";

STRING nosex  = SS "New spouse/parent has unknown sex; can't add to family.";
STRING notopp = SS "The persons are not of opposite sex; can't make family.";
STRING ntsinf = SS "This person is not a spouse in the family; can't remove.";
STRING ntcinf = SS "This person is not a child in the family; can't remove.";
STRING unksex = SS "This person's sex is not known; can't make family.";
STRING hashsb = SS "This family already has a husband/male parent.";
STRING haswif = SS "This family already has a wife/female parent.";
STRING hasbth = SS "This family has both spouses/parents; can't add another.";
STRING hasnei = SS "This family has neither spouse/parent; can't remove.";
STRING haslnk = SS "This family still has links; not removed.";

STRING idfbys = SS "Choose a family by selecting a spouse/parent.";
STRING iredit = SS "Do you want to edit the person again?";
STRING fredit = SS "Do you want to edit the family again?";
STRING rredit = SS "Do you want to edit the source again?";
STRING eredit = SS "Do you want to edit the event again?";
STRING xredit = SS "Do you want to edit the record again?";
STRING idpedt = SS "Who do you want to edit?";
STRING idredt = SS "Which source record do you want to edit?";
STRING ideedt = SS "Which event record do you want to edit?";
STRING idxedt = SS "What record do you want to edit?";

STRING cfpadd = SS "Do you really want to add this person to the database?";
STRING cffadd = SS "Do you really want to add this family to the database?";
STRING cfcadd = SS "Do you really want to add this child to the family?";
STRING cfsadd = SS "Do you really want to add this spouse/parent to the family?";
STRING cfradd = N_("Do you really want to add this source to the database?");
STRING cfeadd = N_("Do you really want to add this event to the database?");
STRING cfxadd = N_("Do you really want to add this record to the database?");
STRING cfpupt = N_("Do you really want to update this person?");
STRING cffupt = N_("Do you really want to update this family?");
STRING cfrupt = N_("Do you really want to update this source?");
STRING cfeupt = N_("Do you really want to update this event?");
STRING cfxupt = N_("Do you really want to update this record?");
STRING cfpdel = N_("Are you sure you want to remove the person from the database?");
STRING cffdel = N_("Remove this family record ?");
STRING cffdeld= N_( "(%s: %d spouse(s), %d child(ren))");
STRING cfpmrg = SS "Do you really want to merge these two persons?";
STRING cffmrg = SS "Do you really want to merge these two families?";
STRING cfcrmv = SS "Do you really want to remove this child from his/her family?";
STRING cfsrmv = SS "Do you really want to remove this spouse from his/her family?";
STRING spover = SS "Too many spouses to display full list";

STRING badata = SS "There is something wrong with the data.";
STRING idchld = SS "Please identify the child.";
STRING idsbln = SS "Please identify one of the child's siblings.";
STRING idsadd = SS "Identify spouse/parent to add to an existing family.";
STRING idsinf = SS "Identify spouse/parent already in family, if known.";
STRING kchild = SS "Identify child already in family.";
STRING iscinf = SS "This person is already a child in a family.";
STRING idsps1 = SS "Identify a spouse/parent for the new family.";
STRING idsps2 = SS "Identify the second spouse/parent, if known.";
STRING twohsb = SS "Both families must have husbands/fathers.";
STRING twowif = SS "Both families must have wives/mothers.";

STRING ids2fm = SS "Identify spouse/parent in second family, if known.";
STRING idc2fm = SS "Identify child in second family.";
STRING idp2br = SS "Identify second person to browse to.";

STRING crtcfm = SS "Create a family with this person as a child.";
STRING crtsfm = SS "Create a family with this person as a spouse/parent.";
STRING less2c = SS "This family has less than two children; can't swap.";
STRING less2f      = N_("This person is a spouse/parent in less than two families.");
STRING paradox     = N_("Something impossible happened. Contact tech support.");

STRING okcswp = SS "The two children were swapped.";
STRING okfswp = SS "The two families were swapped.";
STRING okcrmv = SS "The child was removed from his/her family.";
STRING oksrmv = SS "The spouse was removed from his/her family.";

STRING nopmrg = SS "A person cannot be merged with him/herself.";
STRING nofmrg = SS "A family cannot be merged with itself.";
STRING noqmrg = SS "Two persons with different parents cannot be merged.";
STRING noxmrg = SS "Two parents of different sexes cannot be merged.";
STRING dhusb  = SS "The families have different fathers/husbands; cannot merge.";
STRING dwife  = SS "The families have different wives/mothers; cannot merge.";
STRING idpdel = SS "Who do you want to remove from the database?";
STRING mklast = SS "Place the child last in the family.";
STRING abverr = SS "Error in abbreviations file.";
STRING uoperr = SS "Error in user options file.";
STRING cmperr = SS "Error in character mapping file.";
STRING sepch  = SS "(Separator is %s)";
STRING aredit = SS "Do you want to re-edit it?";

STRING gdpadd = SS "%s was added to the database.";
STRING gdcadd = SS "%s was added as a child.";
STRING gdsadd = SS "%s was added as a spouse and/or parent.";
STRING gdfadd = SS "The new family was added to the database.";
STRING gdpmod = SS "%s was modified in the database.";
STRING gdfmod = SS "The family was modified in the database.";
STRING gdrmod = SS "The source was modified in the database.";
STRING gdemod = SS "The event was modified in the database.";
STRING gdxmod = SS "The record was modified in the database.";

STRING empstr = SS "                                                ";
STRING empstr71 = SS "                                                                       ";
STRING empstr120 = SS "                                                                                                                        ";
STRING nofopn = SS "Could not open file %s.";

STRING mrkper = SS "Please mark a person first.";

STRING lstnam = SS "The current list is now named %s.";
STRING lstnon = SS "The current list is not named.";
STRING lstwht = SS "What should the name of this list be?";
STRING lstnad = SS "No persons were added to the current list.";
STRING lstpad = SS "What persons or list do you want to add to the current list?";
STRING lstbot = SS "You are at the bottom of the list.";
STRING lsttop = SS "You are at the top of the list.";
STRING lstnew = SS "New persons were added to the current list.";

STRING badttnum    = SS "System error: illegal map code";
STRING nosuchtt    = SS "No such translation table in this database";

/* GEDCOM file */
STRING idgedf = SS "Please enter the name of the GEDCOM file.";
STRING gdcker = SS "Checking GEDCOM file %s for errors.\n";
STRING gdnadd = SS "Because of errors the GEDCOM file was not loaded.\n";
STRING dboldk = SS "No errors; adding records with original keys...";
STRING dbnewk = SS "No errors; adding records with new keys...";
STRING dbodel = SS "Using original keys, %d deleted records will be in the database.";
STRING cfoldk = SS "Use original keys from GEDCOM file?";
STRING dbdelk = SS "Adding unused keys as deleted keys...";
STRING dbrdon      = N_("The database is read-only; loading has been canceled.");
STRING outarc      = N_("Enter name of output archive file.");
STRING outfin      = N_("Database `%s' has been saved in `%s'.");
STRING mouttt      = N_("Enter name of translation table file to write");
STRING mintt  = SS "Enter name of translation table file to read";

STRING extrpt      = SS "<Choose outside this list>";
STRING whatrpt     = SS "What is the name of the program?";

STRING whtout      = SS "What is the name of the output file?";
STRING opt2long    = SS "Malformed configuration file: line too long.";
STRING unsupuni    = SS "Unsupported Unicode format (only UTF-8 is supported).";

/* new records */
STRING defsour     = N_("0 SOUR\n1 REFN\n1 TITL Title\n1 AUTH Author");
STRING defeven     = N_("0 EVEN\n1 REFN\n1 DATE\n1 PLAC\n1 INDI\n  2 NAME\n  2 ROLE\n1 SOUR");
STRING defothr     = N_("0 XXXX\n1 REFN");

/* node.c errors */
STRING fileof      = SS "The file is as positioned at EOF.";
STRING reremp      = SS "Line %d: This line is empty; EOF?";
STRING rerlng      = SS "Line %d: This line is too long.";
STRING rernlv      = SS "Line %d: This line has no level number.";
STRING rerinc      = SS "Line %d: This line is incomplete.";
STRING rerbln      = SS "Line %d: This line has a bad link.";
STRING rernwt      = SS "Line %d: This line needs white space before tag.";
STRING rerilv      = SS "Line %d: This line has an illegal level.";
STRING rerwlv      = SS "The record begins at wrong level.";


/* browse display stuff */
STRING dspl_indi   = SS "person";
STRING dspa_resi   = SS ", of ";
STRING dspl_fath   = SS "father";
STRING dspl_moth   = SS "mother";
STRING dspl_spouse = SS "spouse";
STRING dspl_child  = SS "child";
/* display abbreviations */
STRING dspa_mar    = SS "m. ";
STRING dspa_div    = SS "div. ";
STRING dspa_bir    = SS "b. ";
STRING dspa_chr    = SS "bap. ";
STRING dspa_dea    = SS "d. ";
STRING dspa_bur    = SS "bur. ";
STRING dspa_chbr   = SS "cb. ";
/* display longer forms */
STRING dspl_mar    = N_("married: ");
STRING dspl_bir    = N_("born: ");
STRING dspl_chr    = N_("bapt: ");
STRING dspl_dea    = N_("died: ");
STRING dspl_bur    = N_("buri: ");

/* editing errors */
STRING badind      = SS "You cannot edit the INDI line in a person record.";
STRING badfmc      = SS "You cannot edit the FAMC line in a person record.";
STRING badfms      = SS "You cannot edit the FAMS lines in a person record.";
STRING badfam      = SS "You cannot edit the FAM line in a family record.";
STRING badhsb      = SS "You cannot edit the HUSB line in a family record.";
STRING badwif      = SS "You cannot edit the WIFE line in a family record.";
STRING badchl      = SS "You cannot edit the CHIL lines in a family record.";
STRING bademp      = SS "The record is empty.";
STRING badin0      = SS "The record does not begin with an INDI line.";
STRING badfm0      = SS "The record does not begin with a FAM line.";
STRING badsr0      = SS "The record does not begin with a SOUR line.";
STRING badev0      = SS "The record does not begin with an EVEN line.";
STRING badothr0    = SS "INDI, FAM, SOUR, EVEN records may not be other records.";
STRING badmul      = SS "The record contains multiple level 0 lines.";
STRING badenm      = SS "This person record has bad GEDCOM name syntax.";
STRING badpsx      = SS "You cannot change the sex of a parent.";
STRING badirf      = SS "REFN key is already in use.";
STRING tag2long2cnc= SS "Tag is too long to connect automatically.";
STRING dbrecstats  = SS "Database records: ";

/* menus */
STRING mtitle      = SS "LifeLines %s - Genealogical DB and Programming System";
STRING cright      = SS "Copyright(c) 1991 to 1996, by T. T. Wetmore IV";
STRING plschs      = N_("Please choose an operation:");
STRING mn_unkcmd   = N_("Not valid command");

/* prompt, full list, yes list */
STRING askynq      = N_("enter y (yes) or n (no): ");
STRING askynyn     = N_("yYnN");
STRING askyY       = N_("yY");

/* list menu */
STRING chlist      = SS "Commands:   j Move down     k Move up    i Select     q Quit";
STRING vwlist      = SS "Commands:   j Move down     k Move up    q Quit";
STRING errlist     = SS "Messages:";

/* adding new xref */
STRING defttl      = SS "Please choose from the following options:";
STRING newrecis    = SS "New record is %s";
STRING autoxref    = SS "Insert xref automatically at bottom of current record.";
STRING editcur     = SS "Edit current record now to add xref manually.";
STRING gotonew     = SS "Browse new record (without adding xref).";
STRING staycur     = SS "Return to current record (without adding xref).";

/* misc */
STRING unksps      = SS "Spouse unknown";
STRING nohist      = SS "No more history";
STRING badhistcnt  = SS "Bad history count";
STRING badhistcnt2 = SS "Bad backup history count";
STRING badhistlen  = SS "Bad history length";
STRING histclr     = SS "Delete history (%d entries) ?";
STRING dataerr     = SS "Error accessing data";
STRING idhist      = SS "Choose from history";
STRING norwandro   = SS "Cannot combine immutable (-i) or read-only (-r) with read-write (-w) access.";
STRING nofandl     = SS "Cannot combine forceopen (-f) and lock (-l) flags.";
STRING idrpt       = SS "Program";
STRING iddefpath   = SS "Default path: ";
STRING misskeys    = SS "WARNING: missing keys";
STRING whtfname    = SS "enter file name";
STRING whtfnameext = SS "enter file name (*%s)";

/* translation table errors */
STRING baddec      = SS "Bad decimal number format.";
STRING badhex      = SS "Bad hexidecimal number format.";
STRING norplc      = SS "No replacement string on line.";
STRING noorig      = SS "No original string on line.";
STRING badesc      = SS "Bad escape format.";

/* many menus */
STRING mn_quit     = N_("q  Return to main menu");
STRING mn_ret      = N_("q  Return to previous menu");
STRING mn_exit     = N_("q  Quit program");

/* &&begin main menu (70 chars after spaces) */
STRING mn_mmbrws   = N_("b  Browse the persons in the database");
STRING mn_mmsear   = N_("s  Search database");
STRING mn_mmadd    = N_("a  Add information to the database");
STRING mn_mmdel    = N_("d  Delete information from the database");
STRING mn_mmrpt    = N_("r  Generate report by entering report name");
STRING mn_mmprpt   = N_("p  Pick a report from list and run");
STRING mn_mmcset   = N_("c  Character set options");
STRING mn_mmtt     = N_("t  Modify character translation tables");
STRING mn_mmut     = N_("u  Miscellaneous utilities");
STRING mn_mmex     = N_("x  Handle source, event and other records");

/* &&end main menu, begin utility menu */
STRING mn_uttl     = N_("What utility do you want to perform?");
STRING mn_utsave   = N_("s  Save the database in a GEDCOM file");
STRING mn_utread   = N_("r  Read in data from a GEDCOM file");
STRING mn_utkey    = N_("k  Find a person's key value");
STRING mn_utkpers  = N_("i  Identify a person from key value");
STRING mn_utdbstat = N_("d  Show database statistics");
STRING mn_utmemsta = N_("m  Show memory statistics");
STRING mn_utplaces = N_("e  Edit the place abbreviation file");
STRING mn_utusropt = N_("o  Edit the user options file");

/* &&end utility menu, begin extra menu */
STRING mn_xttl     = N_("What activity do you want to perform?");
STRING mn_xxbsour  = N_("s  Browse source records");
STRING mn_xxbeven  = N_("e  Browse event records");
STRING mn_xxbothr  = N_("x  Browse other records");
STRING mn_xxasour  = N_("1  Add a source record to the database");
STRING mn_xxesour  = N_("2  Edit source record from the database");
STRING mn_xxaeven  = N_("3  Add an event record to the database");
STRING mn_xxeeven  = N_("4  Edit event record from the database");
STRING mn_xxaothr  = N_("5  Add an other record to the database");
STRING mn_xxeothr  = N_("6  Edit other record from the database");

/* end extra menu, begin character set menu */
STRING mn_csttl    = SS "Character set menu";
STRING mn_cstt     = SS "t  Edit translation tables";
STRING mn_csrpt    = SS "r  Report character set setup";
STRING mn_csintcs  = SS "Internal code set: ";
STRING mn_csdsploc = SS "Display locale: ";
 /* The following 5 choices look best if padded to same length (70 chars after spaces) */
STRING mn_cstsort  = SS "s  Edit custom sort table        ";
STRING mn_cspref   = SS "p  Edit custom sort prefix table ";
STRING mn_cschar   = SS "c  Edit custom codepage mapping  ";
STRING mn_cslcas   = SS "l  Edit custom lower case mapping";
STRING mn_csucas   = SS "u  Edit custom upper case mapping";
/* menu for report character set issues */
STRING mn_csrpttl  = SS "Report character set menu";
STRING mn_csrptcs  = SS "Report code set: ";

/* &&begin translation table menu */
STRING mn_tt_ttl   = N_("Translation Tables");
STRING mn_tt_edit  = N_("e  edit individual tables (in db)");
STRING mn_tt_load  = N_("l  load a table from a file (into db)");
STRING mn_tt_save  = N_("s  save a table to a file (from db)");
STRING mn_tt_exp   = N_("x  export all tables (from db to files)");
STRING mn_tt_imp   = N_("i  import all tables (from files into db)");
STRING mn_tt_dir   = N_("export/import directory:");

/* menus for translation tables */
STRING mn_edttttl  = N_("Which character mapping do you want to edit?");
STRING mn_svttttl  = N_("Which character mapping do you want to save?");
/* strings for choosing translation table (these looks best if choices padded to same length)*/
STRING mn_tt_edin  = N_("e  Editor to Internal mapping ");
STRING mn_tt_ined  = N_("m  Internal to Editor mapping ");
STRING mn_tt_gdin  = N_("i  GEDCOM to Internal mapping ");
STRING mn_tt_ingd  = N_("x  Internal to GEDCOM mapping ");
STRING mn_tt_dsin  = N_("g  Display to Internal mapping");
STRING mn_tt_inds  = N_("d  Internal to Display mapping");
STRING mn_tt_inrp  = N_("r  Internal to Report mapping ");
/* not yet implemented choices */
STRING mn_notimpl  = SS "Not implemented yet";

/* &&begin add menu */
STRING mn_add_ttl  = N_("What do you want to add?");
STRING mn_add_indi = N_("p  Person - add new person to the database");
STRING mn_add_fam  = N_("f  Family - create family record from one or two spouses");
STRING mn_add_chil = N_("c  Child - add a child to an existing family");
STRING mn_add_spou = N_("s  Spouse - add a spouse to an existing family");

/* &&end add menu, begin delete menu */
STRING mn_del_ttl  = N_("What do you want to remove?");
STRING mn_del_chil = N_("c  Child - remove a child from his/her family");
STRING mn_del_spou = N_("s  Spouse - remove a spouse from a family");
STRING mn_del_indi = N_("i  Individual - remove a person completely");
STRING mn_del_fam  = N_("f  Family - remove a family completely");

/* &&end delete menu, begin scan menu */
STRING mn_sca_ttl  = N_("What scan type?");
STRING mn_sca_nmfu = N_("f  Full name scan");
STRING mn_sca_nmfr = N_("n  Name fragment (whitespace-delimited) scan");
STRING mn_sca_refn = N_("r  Refn scan");
STRING sts_sca_ful = N_("Performing full name scan");
STRING sts_sca_fra = N_("Performing name fragment scan");
STRING sts_sca_ref = N_("Performing refn scan");
STRING sts_sca_non = N_("No records found in scan");

/* &&complex date strings (A=abbrev, B=full)*/
STRING datea_abtA  = N_("abt %1");
STRING datea_abtB  = N_("about %1");
STRING datea_estA  = N_("est %1");
STRING datea_estB  = N_("estimated %1");
STRING datea_calA  = N_("cal %1");
STRING datea_calB  = N_("calculated %1");
STRING datep_fromA = N_("fr %1");
STRING datep_fromB = N_("from %1");
STRING datep_toA   = N_("to %1");
STRING datep_toB   = N_("to %1");
STRING datep_frtoA = N_("fr %1 to %2");
STRING datep_frtoB = N_("from %1 to %2");
STRING dater_befA  = N_("bef %1");
STRING dater_befB  = N_("before %1");
STRING dater_aftA  = N_("aft %1");
STRING dater_aftB  = N_("after %1");
STRING dater_betA  = N_("bet %1 and %2");
STRING dater_betB  = N_("between %1 and %2");
	/* &&origin/era trailers */
STRING datetrl_bcA = SS "B.C.";
STRING datetrl_bcB = SS "BC";
STRING datetrl_bcC = SS "B.C.E.";
STRING datetrl_bcD = SS "BCE";
STRING datetrl_adA = SS "A.D.";
STRING datetrl_adB = SS "AD";
STRING datetrl_adC = SS "C.E.";
STRING datetrl_adD = SS "CE";
	/* &&calendar pics */
STRING caljul      = SS "%1J";
STRING calheb      = SS "%1 HEB";
STRING calfr       = SS "%1 FR";
STRING calrom      = SS "%1 AUC";
	/* &&Gregorian/Julian months */
STRING mon_gj1A    = N_("jan");
STRING mon_gj1B    = N_("january");
STRING mon_gj2A    = N_("feb");
STRING mon_gj2B    = N_("february");
STRING mon_gj3A    = N_("mar");
STRING mon_gj3B    = N_("march");
STRING mon_gj4A    = N_("apr");
STRING mon_gj4B    = N_("april");
/* Put short form for may (don't use **) */
STRING mon_gj5A    = N_("**may");
STRING mon_gj5B    = N_("may");
STRING mon_gj6A    = N_("jun");
STRING mon_gj6B    = N_("june");
STRING mon_gj7A    = N_("jul");
STRING mon_gj7B    = N_("july");
STRING mon_gj8A    = N_("aug");
STRING mon_gj8B    = N_("august");
STRING mon_gj9A    = N_("sep");
STRING mon_gj9B    = N_("september");
STRING mon_gj10A   = N_("oct");
STRING mon_gj10B   = N_("october");
STRING mon_gj11A   = N_("nov");
STRING mon_gj11B   = N_("november");
STRING mon_gj12A   = N_("dec");
STRING mon_gj12B   = N_("december");
	/* &&Hebrew months */
STRING mon_heb1A   = N_("tsh");
STRING mon_heb1B   = N_("tishri");
STRING mon_heb2A   = N_("csh");
STRING mon_heb2B   = N_("cheshvan");
STRING mon_heb3A   = N_("ksl");
STRING mon_heb3B   = N_("kislev");
STRING mon_heb4A   = N_("tvt");
STRING mon_heb4B   = N_("tevet");
STRING mon_heb5A   = N_("shv");
STRING mon_heb5B   = N_("shevat");
STRING mon_heb6A   = N_("adr");
STRING mon_heb6B   = N_("adar");
STRING mon_heb7A   = N_("ads");
STRING mon_heb7B   = N_("adar sheni");
STRING mon_heb8A   = N_("nsn");
STRING mon_heb8B   = N_("nisan");
STRING mon_heb9A   = N_("iyr");
STRING mon_heb9B   = N_("iyar");
STRING mon_heb10A  = N_("svn");
STRING mon_heb10B  = N_("sivan");
STRING mon_heb11A  = N_("tmz");
STRING mon_heb11B  = N_("tamuz");
STRING mon_heb12A  = N_("aav");
STRING mon_heb12B  = N_("av");
STRING mon_heb13A  = N_("ell");
STRING mon_heb13B  = N_("elul");
	/* &&French Republic months */
STRING mon_fr1A    = N_("vend");
STRING mon_fr1B    = N_("vendemiaire");
STRING mon_fr2A    = N_("brum");
STRING mon_fr2B    = N_("brumaire");
STRING mon_fr3A    = N_("frim");
STRING mon_fr3B    = N_("frimaire");
STRING mon_fr4A    = N_("nivo");
STRING mon_fr4B    = N_("nivose");
STRING mon_fr5A    = N_("pluv");
STRING mon_fr5B    = N_("pluviose");
STRING mon_fr6A    = N_("vent");
STRING mon_fr6B    = N_("ventose");
STRING mon_fr7A    = N_("germ");
STRING mon_fr7B    = N_("germinal");
STRING mon_fr8A    = N_("flor");
STRING mon_fr8B    = N_("floreal");
STRING mon_fr9A    = N_("prai");
STRING mon_fr9B    = N_("prairial");
STRING mon_fr10A   = N_("mess");
STRING mon_fr10B   = N_("messidor");
STRING mon_fr11A   = N_("ther");
STRING mon_fr11B   = N_("thermidor");
STRING mon_fr12A   = N_("fruc");
STRING mon_fr12B   = N_("fructidor");
STRING mon_fr13A   = N_("comp");
STRING mon_fr13B   = N_("jour_complementairs");
