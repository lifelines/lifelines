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
#define ZST STRING

/*
 2002.04.24
  Perry is prefixing string constants with qS as he checks them for
  localization (and so they can all be found easily)
*/

ZST qSiddbse      = N_("You must identify a database.");
ZST qSidldir      = N_("What directory holds the LifeLines database?");
ZST qSidldrp      = N_("enter path: ");
ZST qSnodbse      = N_("There is no LifeLines database in that directory.");
ZST qScrdbse      = N_("Do you want to create a database there?");
ZST qSbdlkar      = N_("Argument to lock (-l) must be y or n.");
ZST qSnormls      = N_("Cannot remove last person from family.");

ZST qSronlya      = N_("The database is read only; you may not add records.");
ZST qSronlye      = N_("The database is read only; you may not change records.");
ZST qSronlym      = N_("The database is read only; you may not merge records.");
ZST qSronlyr      = N_("The database is read only; you may not remove records.");

ZST qSidbrws      = N_("enter name, key, refn or list:");
ZST qSidkyrfn     = N_("enter key or refn: ");
ZST qSidcrmv      = N_("Please choose the child to remove from the family.");
ZST qSidsrmv      = N_("Please choose the spouse/parent to remove from the family.");
ZST qSidcrmf      = N_("From which family is the child to be removed?");
ZST qSidsrmf      = N_("From which family is the spouse/parent to be removed?");
ZST qSidfrmv      = N_("Identify family (enter nothing to identify by individual members).");
ZST qSidfrsp      = N_("Identify a spouse of the family, if known.");
ZST qSidfrch      = N_("Identify a child of the family, if known.");
ZST qSid1csw      = N_("Identify the first child to swap.");
ZST qSid2csw      = N_("Identify the second child to swap.");
ZST qSidcrdr      = N_("Identify the child to reorder.");
ZST qSid1fsw      = N_("Identify the first family/spouse to swap.");
ZST qSid2fsw      = N_("Identify the second family/spouse to swap.");
ZST qSidsbrs      = N_("Please choose the spouse/parent to browse to.");
ZST qSid1sbr      = N_("Please choose the first spouse/parent to browse to.");
ZST qSid2sbr      = N_("Please choose the second spouse/parent to browse to.");
ZST qSidcbrs      = N_("Please choose the child to browse to.");
ZST qSid1cbr      = N_("Please choose the first child to browse to.");
ZST qSid2cbr      = N_("Please choose the second child to browse to.");
ZST qSidfbrs      = N_("Please choose the family to browse to.");
ZST qSidfamk      = N_("Enter Family Number to Browse to");
ZST qSid1fbr      = N_("Please choose the first family to browse to.");
ZST qSid2fbr      = N_("Please choose the second family to browse to.");
ZST qSidhbrs      = N_("Please choose the father/husband to browse to.");
ZST qSid1hbr      = N_("Please choose the first father/husband to browse to.");
ZST qSid2hbr      = N_("Please choose the second father/husband to browse to.");
ZST qSidwbrs      = N_("Please choose the mother/wife to browse to.");
ZST qSid1wbr      = N_("Please choose the first mother/wife to browse to.");
ZST qSid2wbr      = N_("Please choose the second mother/wife to browse to.");
ZST qSidcswp      = N_("Identify a parent in the family having children swapped.");
ZST qSidfswp      = N_("Whose families/spouses are to be swapped?");
ZST qSidprnt      = N_("Identify one of the child's parents, if known.");

ZST qSscanrs      = N_("Scan results:");
ZST qSscnnmf      = N_("Enter pattern to match against single surname or given name.");
ZST qSscnfnm      = N_("Enter pattern to match against full name.");
ZST qSscnrfn      = N_("Enter pattern to match against refn.");
ZST qSscantt      = N_("pattern: ");

ZST qSnotone      = N_("Please choose from among these records.");
ZST qSiscnew      = N_("Is this the new child? ");
ZST qSissnew      = N_("Is this the new spouse? ");
ZST qSifone       = N_("Is this the person?  Select if so.");
ZST qSidcfam      = N_("Select the child the new child precedes or select last.");
ZST qSidpnxt      = N_("Please identify next person to browse to.");
ZST qSidnxt       = N_("Please identify record to browse to.");
ZST qSidspse      = N_("Please identify one of the spouses.");
ZST qSidplst      = N_("Please identify person or persons to browse to.");
ZST qSidfcop      = N_("Please choose family create operation.");
ZST qSentnam      = N_("Do you want to enter another name?");

ZST qSntprnt      = N_("This person is not a spouse or parent in any family.");
ZST qSntchld      = N_("This person is not a child in any family.");
ZST qSnonamky     = N_("There is no one in the database with that name or key.");

ZST qSnofath      = N_("This person's father is not in the database.");
ZST qSnomoth      = N_("This person's mother is not in the database.");
ZST qSnospse      = N_("This person has no spouse in the database.");
ZST noysib = SS "This person has no younger sibling in the database.";
ZST noosib = SS "This person has no older sibling in the database.";
ZST noprnt = SS "This person's parents are not in the database.";
ZST nohusb = SS "This family has no male spouse/parent in the database.";
ZST nowife = SS "This family has no female spouse/parent in the database.";
ZST nocinf = SS "There are no children in the database for this family.";
ZST nocofp = SS "This person has no children in the database.";
ZST nochil = SS "No such child.";
ZST nopers = SS "No such person.";
ZST norec  = SS "No such record.";
ZST nofam  = SS "No such family.";
ZST nosour = SS "No sources present.";
ZST idsour = SS "Please choose the source to view.";
ZST noeven = SS "No events present.";
ZST ideven = SS "Please choose the event to view.";
ZST noothe = SS "No others present.";
ZST idothe = SS "Please choose the other to view.";
ZST nonote = SS "No notes present.";
ZST idnote = SS "Please choose the note to view.";
ZST noptr  = SS "No references present.";
ZST idptr  = SS "Please choose the reference to view.";
ZST duprfn = SS "Duplicated REFN - please choose.";

ZST nosex  = SS "New spouse/parent has unknown sex; can't add to family.";
ZST notopp = SS "The persons are not of opposite sex; can't make family.";
ZST ntsinf = SS "This person is not a spouse in the family; can't remove.";
ZST ntcinf = SS "This person is not a child in the family; can't remove.";
ZST unksex = SS "This person's sex is not known; can't make family.";
ZST hashsb = SS "This family already has a husband/male parent.";
ZST haswif = SS "This family already has a wife/female parent.";
ZST hasbth = SS "This family has both spouses/parents; can't add another.";
ZST hasnei = SS "This family has neither spouse/parent; can't remove.";
ZST haslnk = SS "This family still has links; not removed.";

ZST idfbys = SS "Choose a family by selecting a spouse/parent.";
ZST iredit = SS "Do you want to edit the person again?";
ZST fredit = SS "Do you want to edit the family again?";
ZST rredit = SS "Do you want to edit the source again?";
ZST eredit = SS "Do you want to edit the event again?";
ZST xredit = SS "Do you want to edit the record again?";
ZST idpedt = SS "Who do you want to edit?";
ZST idredt = SS "Which source record do you want to edit?";
ZST ideedt = SS "Which event record do you want to edit?";
ZST idxedt = SS "What record do you want to edit?";

ZST cfpadd = SS "Do you really want to add this person to the database?";
ZST cffadd = SS "Do you really want to add this family to the database?";
ZST cfcadd = SS "Do you really want to add this child to the family?";
ZST cfsadd = SS "Do you really want to add this spouse/parent to the family?";
ZST qScfradd      = N_("Do you really want to add this source to the database?");
ZST qScfeadd      = N_("Do you really want to add this event to the database?");
ZST qScfxadd      = N_("Do you really want to add this record to the database?");
ZST qScfpupt      = N_("Do you really want to update this person?");
ZST qScffupt      = N_("Do you really want to update this family?");
ZST qScfrupt      = N_("Do you really want to update this source?");
ZST qScfeupt      = N_("Do you really want to update this event?");
ZST qScfxupt      = N_("Do you really want to update this record?");
ZST qScfpdel      = N_("Are you sure you want to remove the person from the database?");
ZST qScffdel      = N_("Remove this family record ?");
ZST qScffdeld     = N_( "(%s: %d spouse(s), %d child(ren))");
ZST qScfpmrg      = N_("Do you really want to merge these two persons?");
ZST qScffmrg      = N_("Do you really want to merge these two families?");
ZST qScfcrmv      = N_("Do you really want to remove this child from his/her family?");
ZST qScfsrmv      = N_("Do you really want to remove this spouse from his/her family?");
ZST qSspover      = N_("Too many spouses to display full list");
ZST qSmgsfam      = N_("These persons are children in different families.");
ZST qSmgconf      = N_("Are you sure you want to merge them?");

ZST qSbadata      = N_("There is something wrong with the data.");
ZST qSidchld      = N_("Please identify the child.");
ZST qSidsbln      = N_("Please identify one of the child's siblings.");
ZST idsadd = SS "Identify spouse/parent to add to an existing family.";
ZST idsinf = SS "Identify spouse/parent already in family, if known.";
ZST kchild = SS "Identify child already in family.";
ZST iscinf = SS "This person is already a child in a family.";
ZST idsps1 = SS "Identify a spouse/parent for the new family.";
ZST idsps2 = SS "Identify the second spouse/parent, if known.";
ZST twohsb = SS "Both families must have husbands/fathers.";
ZST twowif = SS "Both families must have wives/mothers.";

ZST ids2fm = SS "Identify spouse/parent in second family, if known.";
ZST idc2fm = SS "Identify child in second family.";
ZST idp2br = SS "Identify second person to browse to.";

ZST crtcfm = SS "Create a family with this person as a child.";
ZST crtsfm = SS "Create a family with this person as a spouse/parent.";
ZST qSless2c      = N_("This family has less than two children; can't swap.");
ZST qSless2f      = N_("This person is a spouse/parent in less than two families.");
ZST qSparadox     = N_("Something impossible happened. Contact tech support.");

ZST okcswp = SS "The two children were swapped.";
ZST okfswp = SS "The two families were swapped.";
ZST okcrmv = SS "The child was removed from his/her family.";
ZST oksrmv = SS "The spouse was removed from his/her family.";

ZST nopmrg = SS "A person cannot be merged with him/herself.";
ZST nofmrg = SS "A family cannot be merged with itself.";
ZST noqmrg = SS "Two persons with different parents cannot be merged.";
ZST noxmrg = SS "Two parents of different sexes cannot be merged.";
ZST dhusb  = SS "The families have different fathers/husbands; cannot merge.";
ZST dwife  = SS "The families have different wives/mothers; cannot merge.";
ZST idpdel = SS "Who do you want to remove from the database?";
ZST mklast = SS "Place the child last in the family.";
ZST abverr = SS "Error in abbreviations file.";
ZST uoperr = SS "Error in user options file.";
ZST cmperr = SS "Error in character mapping file.";
ZST sepch  = SS "(Separator is %s)";
ZST aredit = SS "Do you want to re-edit it?";

ZST gdpadd = SS "%s was added to the database.";
ZST gdcadd = SS "%s was added as a child.";
ZST gdsadd = SS "%s was added as a spouse and/or parent.";
ZST gdfadd = SS "The new family was added to the database.";
ZST gdpmod = SS "%s was modified in the database.";
ZST gdfmod = SS "The family was modified in the database.";
ZST gdrmod = SS "The source was modified in the database.";
ZST gdemod = SS "The event was modified in the database.";
ZST gdxmod = SS "The record was modified in the database.";

ZST empstr = SS "                                                ";
ZST empstr71 = SS "                                                                       ";
ZST empstr120 = SS "                                                                                                                        ";
ZST qSnofopn      = N_("Could not open file %s.");

ZST mrkper = SS "Please mark a person first.";

ZST lstnam = SS "The current list is now named %s.";
ZST lstnon = SS "The current list is not named.";
ZST lstwht = SS "What should the name of this list be?";
ZST lstnad = SS "No persons were added to the current list.";
ZST lstpad = SS "What persons or list do you want to add to the current list?";
ZST lstbot = SS "You are at the bottom of the list.";
ZST lsttop = SS "You are at the top of the list.";
ZST lstnew = SS "New persons were added to the current list.";

ZST qSbadttnum    = N_("System error: illegal map code");
ZST qSnosuchtt    = N_("No such translation table in this database");

/* GEDCOM file */
ZST idgedf = SS "Please enter the name of the GEDCOM file.";
ZST gdcker = SS "Checking GEDCOM file %s for errors.\n";
ZST gdnadd = SS "Because of errors the GEDCOM file was not loaded.\n";
ZST dboldk = SS "No errors; adding records with original keys...";
ZST dbnewk = SS "No errors; adding records with new keys...";
ZST dbodel = SS "Using original keys, %d deleted records will be in the database.";
ZST cfoldk = SS "Use original keys from GEDCOM file?";
ZST dbdelk = SS "Adding unused keys as deleted keys...";
ZST qSdbrdon      = N_("The database is read-only; loading has been canceled.");
ZST qSoutarc      = N_("Enter name of output archive file.");
ZST qSoutfin      = N_("Database `%s' has been saved in `%s'.");
ZST qSmouttt      = N_("Enter name of translation table file to write");
ZST mintt  = SS "Enter name of translation table file to read";

ZST extrpt      = SS "<Choose outside this list>";
ZST whatrpt     = SS "What is the name of the program?";

ZST whtout      = SS "What is the name of the output file?";
ZST opt2long    = SS "Malformed configuration file: line too long.";
ZST unsupuni    = SS "Unsupported Unicode format (only UTF-8 is supported).";

/* misc prompts */
ZST qSchoostrttl  = N_("Enter string for program");
ZST qSaskstr      = N_("enter string: ");
ZST qSaskint      = N_("enter integer:");
ZST qSasknam      = N_("enter name: ");

/* new records */
ZST qSdefsour     = N_("0 SOUR\n1 REFN\n1 TITL Title\n1 AUTH Author");
ZST qSdefeven     = N_("0 EVEN\n1 REFN\n1 DATE\n1 PLAC\n1 INDI\n  2 NAME\n  2 ROLE\n1 SOUR");
ZST qSdefothr     = N_("0 XXXX\n1 REFN");

/* node.c errors */
ZST fileof      = SS "The file is as positioned at EOF.";
ZST reremp      = SS "Line %d: This line is empty; EOF?";
ZST rerlng      = SS "Line %d: This line is too long.";
ZST rernlv      = SS "Line %d: This line has no level number.";
ZST rerinc      = SS "Line %d: This line is incomplete.";
ZST rerbln      = SS "Line %d: This line has a bad link.";
ZST rernwt      = SS "Line %d: This line needs white space before tag.";
ZST rerilv      = SS "Line %d: This line has an illegal level.";
ZST rerwlv      = SS "The record begins at wrong level.";


/* browse display stuff */
ZST dspl_indi   = SS "person";
ZST dspa_resi   = SS ", of ";
ZST dspl_fath   = SS "father";
ZST dspl_moth   = SS "mother";
ZST dspl_spouse = SS "spouse";
ZST dspl_child  = SS "child";
/* display abbreviations */
ZST dspa_mar    = SS "m. ";
ZST dspa_div    = SS "div. ";
ZST dspa_bir    = SS "b. ";
ZST dspa_chr    = SS "bap. ";
ZST dspa_dea    = SS "d. ";
ZST dspa_bur    = SS "bur. ";
ZST dspa_chbr   = SS "cb. ";
/* display longer forms */
ZST qSdspl_mar    = N_("married: ");
ZST qSdspl_bir    = N_("born: ");
ZST qSdspl_chr    = N_("bapt: ");
ZST qSdspl_dea    = N_("died: ");
ZST qSdspl_bur    = N_("buri: ");

/* editing errors */
ZST badind      = SS "You cannot edit the INDI line in a person record.";
ZST badfmc      = SS "You cannot edit the FAMC line in a person record.";
ZST badfms      = SS "You cannot edit the FAMS lines in a person record.";
ZST badfam      = SS "You cannot edit the FAM line in a family record.";
ZST badhsb      = SS "You cannot edit the HUSB line in a family record.";
ZST badwif      = SS "You cannot edit the WIFE line in a family record.";
ZST badchl      = SS "You cannot edit the CHIL lines in a family record.";
ZST bademp      = SS "The record is empty.";
ZST badin0      = SS "The record does not begin with an INDI line.";
ZST badfm0      = SS "The record does not begin with a FAM line.";
ZST badsr0      = SS "The record does not begin with a SOUR line.";
ZST badev0      = SS "The record does not begin with an EVEN line.";
ZST badothr0    = SS "INDI, FAM, SOUR, EVEN records may not be other records.";
ZST badmul      = SS "The record contains multiple level 0 lines.";
ZST badenm      = SS "This person record has bad GEDCOM name syntax.";
ZST badpsx      = SS "You cannot change the sex of a parent.";
ZST badirf      = SS "REFN key is already in use.";
ZST tag2long2cnc= SS "Tag is too long to connect automatically.";
ZST dbrecstats  = SS "Database records: ";

/* menus */
ZST mtitle      = SS "LifeLines %s - Genealogical DB and Programming System";
ZST cright      = SS "Copyright(c) 1991 to 1996, by T. T. Wetmore IV";
ZST qSplschs      = N_("Please choose an operation:");
ZST qSmn_unkcmd   = N_("Not valid command");

/* prompt, full list, yes list */
ZST qSaskynq      = N_("enter y (yes) or n (no): ");
ZST qSaskynyn     = N_("yYnN");
ZST qSaskyY       = N_("yY");

/* list menu */
ZST chlist      = SS "Commands:   j Move down     k Move up    i Select     q Quit";
ZST vwlist      = SS "Commands:   j Move down     k Move up    q Quit";
ZST qSerrlist     = N_("Messages:");

/* adding new xref */
ZST defttl      = SS "Please choose from the following options:";
ZST newrecis    = SS "New record is %s";
ZST autoxref    = SS "Insert xref automatically at bottom of current record.";
ZST editcur     = SS "Edit current record now to add xref manually.";
ZST gotonew     = SS "Browse new record (without adding xref).";
ZST staycur     = SS "Return to current record (without adding xref).";

/* misc */
ZST unksps      = SS "Spouse unknown";
ZST nohist      = SS "No more history";
ZST badhistcnt  = SS "Bad history count";
ZST badhistcnt2 = SS "Bad backup history count";
ZST badhistlen  = SS "Bad history length";
ZST histclr     = SS "Delete history (%d entries) ?";
ZST dataerr     = SS "Error accessing data";
ZST idhist      = SS "Choose from history";
ZST norwandro   = SS "Cannot combine immutable (-i) or read-only (-r) with read-write (-w) access.";
ZST nofandl     = SS "Cannot combine forceopen (-f) and lock (-l) flags.";
ZST idrpt       = SS "Program";
ZST iddefpath   = SS "Default path: ";
ZST qSmisskeys    = N_("WARNING: missing keys");
ZST qSbadkeyptr   = N_("WARNING: invalid pointer");
ZST qSwhtfname    = N_("enter file name");
ZST qSwhtfnameext = N_("enter file name (*%s)");
ZST qSnosuchrec   = N_("There is no record with that key or reference.");

/* translation table errors */
ZST baddec      = SS "Bad decimal number format.";
ZST badhex      = SS "Bad hexidecimal number format.";
ZST norplc      = SS "No replacement string on line.";
ZST noorig      = SS "No original string on line.";
ZST badesc      = SS "Bad escape format.";

/* many menus */
ZST qSmn_quit     = N_("q  Return to main menu");
ZST qSmn_ret      = N_("q  Return to previous menu");
ZST qSmn_exit     = N_("q  Quit program");

/* &&begin main menu (70 chars after spaces) */
ZST qSmn_mmbrws   = N_("b  Browse the persons in the database");
ZST qSmn_mmsear   = N_("s  Search database");
ZST qSmn_mmadd    = N_("a  Add information to the database");
ZST qSmn_mmdel    = N_("d  Delete information from the database");
ZST qSmn_mmrpt    = N_("r  Generate report by entering report name");
ZST qSmn_mmprpt   = N_("p  Pick a report from list and run");
ZST qSmn_mmcset   = N_("c  Character set options");
ZST qSmn_mmtt     = N_("t  Modify character translation tables");
ZST qSmn_mmut     = N_("u  Miscellaneous utilities");
ZST qSmn_mmex     = N_("x  Handle source, event and other records");

/* &&end main menu, begin utility menu */
ZST qSmn_uttl     = N_("What utility do you want to perform?");
ZST qSmn_utsave   = N_("s  Save the database in a GEDCOM file");
ZST qSmn_utread   = N_("r  Read in data from a GEDCOM file");
ZST qSmn_utkey    = N_("k  Find a person's key value");
ZST qSmn_utkpers  = N_("i  Identify a person from key value");
ZST qSmn_utdbstat = N_("d  Show database statistics");
ZST qSmn_utmemsta = N_("m  Show memory statistics");
ZST qSmn_utplaces = N_("e  Edit the place abbreviation file");
ZST qSmn_utusropt = N_("o  Edit the user options file");

/* &&end utility menu, begin extra menu */
ZST qSmn_xttl     = N_("What activity do you want to perform?");
ZST qSmn_xxbsour  = N_("s  Browse source records");
ZST qSmn_xxbeven  = N_("e  Browse event records");
ZST qSmn_xxbothr  = N_("x  Browse other records");
ZST qSmn_xxasour  = N_("1  Add a source record to the database");
ZST qSmn_xxesour  = N_("2  Edit source record from the database");
ZST qSmn_xxaeven  = N_("3  Add an event record to the database");
ZST qSmn_xxeeven  = N_("4  Edit event record from the database");
ZST qSmn_xxaothr  = N_("5  Add an other record to the database");
ZST qSmn_xxeothr  = N_("6  Edit other record from the database");

/* end extra menu, begin character set menu */
ZST mn_csttl    = SS "Character set menu";
ZST mn_cstt     = SS "t  Edit translation tables";
ZST mn_csrpt    = SS "r  Report character set setup";
ZST mn_csintcs  = SS "Internal code set: ";
ZST mn_csdsploc = SS "Display locale: ";
 /* The following 5 choices look best if padded to same length (70 chars after spaces) */
ZST mn_cstsort  = SS "s  Edit custom sort table        ";
ZST mn_cspref   = SS "p  Edit custom sort prefix table ";
ZST mn_cschar   = SS "c  Edit custom codepage mapping  ";
ZST mn_cslcas   = SS "l  Edit custom lower case mapping";
ZST mn_csucas   = SS "u  Edit custom upper case mapping";
/* menu for report character set issues */
ZST mn_csrpttl  = SS "Report character set menu";
ZST mn_csrptcs  = SS "Report code set: ";

/* &&begin translation table menu */
ZST qSmn_tt_ttl   = N_("Translation Tables");
ZST qSmn_tt_edit  = N_("e  edit individual tables (in db)");
ZST qSmn_tt_load  = N_("l  load a table from a file (into db)");
ZST qSmn_tt_save  = N_("s  save a table to a file (from db)");
ZST qSmn_tt_exp   = N_("x  export all tables (from db to files)");
ZST qSmn_tt_imp   = N_("i  import all tables (from files into db)");
ZST qSmn_tt_dir   = N_("export/import directory:");

/* menus for translation tables */
ZST qSmn_edttttl  = N_("Which character mapping do you want to edit?");
ZST qSmn_svttttl  = N_("Which character mapping do you want to save?");
/* strings for choosing translation table (these looks best if choices padded to same length)*/
ZST qSmn_tt_edin  = N_("e  Editor to Internal mapping ");
ZST qSmn_tt_ined  = N_("m  Internal to Editor mapping ");
ZST qSmn_tt_gdin  = N_("i  GEDCOM to Internal mapping ");
ZST qSmn_tt_ingd  = N_("x  Internal to GEDCOM mapping ");
ZST qSmn_tt_dsin  = N_("g  Display to Internal mapping");
ZST qSmn_tt_inds  = N_("d  Internal to Display mapping");
ZST qSmn_tt_inrp  = N_("r  Internal to Report mapping ");
/* not yet implemented choices */
ZST qSmn_notimpl  = N_("Not implemented yet");

/* &&begin add menu */
ZST qSmn_add_ttl  = N_("What do you want to add?");
ZST qSmn_add_indi = N_("p  Person - add new person to the database");
ZST qSmn_add_fam  = N_("f  Family - create family record from one or two spouses");
ZST qSmn_add_chil = N_("c  Child - add a child to an existing family");
ZST qSmn_add_spou = N_("s  Spouse - add a spouse to an existing family");

/* &&end add menu, begin delete menu */
ZST qSmn_del_ttl  = N_("What do you want to remove?");
ZST qSmn_del_chil = N_("c  Child - remove a child from his/her family");
ZST qSmn_del_spou = N_("s  Spouse - remove a spouse from a family");
ZST qSmn_del_indi = N_("i  Individual - remove a person completely");
ZST qSmn_del_fam  = N_("f  Family - remove a family completely");

/* &&end delete menu, begin scan menu */
ZST qSmn_sca_ttl  = N_("What scan type?");
ZST qSmn_sca_nmfu = N_("f  Full name scan");
ZST qSmn_sca_nmfr = N_("n  Name fragment (whitespace-delimited) scan");
ZST qSmn_sca_refn = N_("r  Refn scan");
ZST qSsts_sca_ful = N_("Performing full name scan");
ZST qSsts_sca_fra = N_("Performing name fragment scan");
ZST qSsts_sca_ref = N_("Performing refn scan");
ZST qSsts_sca_non = N_("No records found in scan");

/* &&complex date strings (A=abbrev, B=full)*/
ZST qSdatea_abtA  = N_("abt %1");
ZST qSdatea_abtB  = N_("about %1");
ZST qSdatea_estA  = N_("est %1");
ZST qSdatea_estB  = N_("estimated %1");
ZST qSdatea_calA  = N_("cal %1");
ZST qSdatea_calB  = N_("calculated %1");
ZST qSdatep_fromA = N_("fr %1");
ZST qSdatep_fromB = N_("from %1");
ZST qSdatep_toA   = N_("to %1");
ZST qSdatep_toB   = N_("to %1");
ZST qSdatep_frtoA = N_("fr %1 to %2");
ZST qSdatep_frtoB = N_("from %1 to %2");
ZST qSdater_befA  = N_("bef %1");
ZST qSdater_befB  = N_("before %1");
ZST qSdater_aftA  = N_("aft %1");
ZST qSdater_aftB  = N_("after %1");
ZST qSdater_betA  = N_("bet %1 and %2");
ZST qSdater_betB  = N_("between %1 and %2");
	/* &&origin/era trailers */
ZST qSdatetrl_bcA = N_("B.C.");
ZST qSdatetrl_bcB = N_("BC");
ZST qSdatetrl_bcC = N_("B.C.E.");
ZST qSdatetrl_bcD = N_("BCE");
ZST qSdatetrl_adA = N_("A.D.");
ZST qSdatetrl_adB = N_("AD");
ZST qSdatetrl_adC = N_("C.E.");
ZST qSdatetrl_adD = N_("CE");
	/* &&calendar pics */
ZST qScaljul      = N_("%1J");
ZST qScalheb      = N_("%1 HEB");
ZST qScalfr       = N_("%1 FR");
ZST qScalrom      = N_("%1 AUC");
	/* &&Gregorian/Julian months */
ZST qSmon_gj1A    = N_("jan");
ZST qSmon_gj1B    = N_("january");
ZST qSmon_gj2A    = N_("feb");
ZST qSmon_gj2B    = N_("february");
ZST qSmon_gj3A    = N_("mar");
ZST qSmon_gj3B    = N_("march");
ZST qSmon_gj4A    = N_("apr");
ZST qSmon_gj4B    = N_("april");
/* Put short form for may (don't use **) */
ZST qSmon_gj5A    = N_("**may");
ZST qSmon_gj5B    = N_("may");
ZST qSmon_gj6A    = N_("jun");
ZST qSmon_gj6B    = N_("june");
ZST qSmon_gj7A    = N_("jul");
ZST qSmon_gj7B    = N_("july");
ZST qSmon_gj8A    = N_("aug");
ZST qSmon_gj8B    = N_("august");
ZST qSmon_gj9A    = N_("sep");
ZST qSmon_gj9B    = N_("september");
ZST qSmon_gj10A   = N_("oct");
ZST qSmon_gj10B   = N_("october");
ZST qSmon_gj11A   = N_("nov");
ZST qSmon_gj11B   = N_("november");
ZST qSmon_gj12A   = N_("dec");
ZST qSmon_gj12B   = N_("december");
	/* &&Hebrew months */
ZST qSmon_heb1A   = N_("tsh");
ZST qSmon_heb1B   = N_("tishri");
ZST qSmon_heb2A   = N_("csh");
ZST qSmon_heb2B   = N_("cheshvan");
ZST qSmon_heb3A   = N_("ksl");
ZST qSmon_heb3B   = N_("kislev");
ZST qSmon_heb4A   = N_("tvt");
ZST qSmon_heb4B   = N_("tevet");
ZST qSmon_heb5A   = N_("shv");
ZST qSmon_heb5B   = N_("shevat");
ZST qSmon_heb6A   = N_("adr");
ZST qSmon_heb6B   = N_("adar");
ZST qSmon_heb7A   = N_("ads");
ZST qSmon_heb7B   = N_("adar sheni");
ZST qSmon_heb8A   = N_("nsn");
ZST qSmon_heb8B   = N_("nisan");
ZST qSmon_heb9A   = N_("iyr");
ZST qSmon_heb9B   = N_("iyar");
ZST qSmon_heb10A  = N_("svn");
ZST qSmon_heb10B  = N_("sivan");
ZST qSmon_heb11A  = N_("tmz");
ZST qSmon_heb11B  = N_("tamuz");
ZST qSmon_heb12A  = N_("aav");
ZST qSmon_heb12B  = N_("av");
ZST qSmon_heb13A  = N_("ell");
ZST qSmon_heb13B  = N_("elul");
	/* &&French Republic months */
ZST qSmon_fr1A    = N_("vend");
ZST qSmon_fr1B    = N_("vendemiaire");
ZST qSmon_fr2A    = N_("brum");
ZST qSmon_fr2B    = N_("brumaire");
ZST qSmon_fr3A    = N_("frim");
ZST qSmon_fr3B    = N_("frimaire");
ZST qSmon_fr4A    = N_("nivo");
ZST qSmon_fr4B    = N_("nivose");
ZST qSmon_fr5A    = N_("pluv");
ZST qSmon_fr5B    = N_("pluviose");
ZST qSmon_fr6A    = N_("vent");
ZST qSmon_fr6B    = N_("ventose");
ZST qSmon_fr7A    = N_("germ");
ZST qSmon_fr7B    = N_("germinal");
ZST qSmon_fr8A    = N_("flor");
ZST qSmon_fr8B    = N_("floreal");
ZST qSmon_fr9A    = N_("prai");
ZST qSmon_fr9B    = N_("prairial");
ZST qSmon_fr10A   = N_("mess");
ZST qSmon_fr10B   = N_("messidor");
ZST qSmon_fr11A   = N_("ther");
ZST qSmon_fr11B   = N_("thermidor");
ZST qSmon_fr12A   = N_("fruc");
ZST qSmon_fr12B   = N_("fructidor");
ZST qSmon_fr13A   = N_("comp");
ZST qSmon_fr13B   = N_("jour_complementairs");
