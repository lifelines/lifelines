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

STRING iddbse = SS "You must identify a database.";
STRING idldir = SS "What directory holds the LifeLines database?";
STRING nodbse = SS "There is no LifeLines database in that directory.";
STRING crdbse = SS "Do you want to create a database there?";
STRING nocrdb = SS "Could not create the LifeLines database `%s'.";
STRING bdlkar = SS "Argument to lock (-l) must be y or n.";
STRING normls = SS "Cannot remove last person from family.";

STRING ronlya = SS "The database is read only; you may not add records.";
STRING ronlye = SS "The database is read only; you may not change records.";
STRING ronlym = SS "The database is read only; you may not merge records.";
STRING ronlyr = SS "The database is read only; you may not remove records.";

STRING idbrws = SS "enter name, key, refn or list:";
STRING idcrmv = SS "Please choose the child to remove from the family.";
STRING idsrmv = SS "Please choose the spouse/parent to remove from the family.";
STRING idcrmf = SS "From which family is the child to be removed?";
STRING idsrmf = SS "From which family is the spouse/parent to be removed?";
STRING idfrmv = SS "Identify family (enter nothing to identify by individual members).";
STRING idfrsp = SS "Identify a spouse of the family, if known.";
STRING idfrch = SS "Identify a child of the family, if known.";
STRING id1csw = SS "Identify the first child to swap.";
STRING id2csw = SS "Identify the second child to swap.";
STRING idcrdr = SS "Identify the child to reorder.";
STRING id1fsw = SS "Identify the first family/spouse to swap.";
STRING id2fsw = SS "Identify the second family/spouse to swap.";
STRING idsbrs = SS "Please choose the spouse/parent to browse to.";
STRING id1sbr = SS "Please choose the first spouse/parent to browse to.";
STRING id2sbr = SS "Please choose the second spouse/parent to browse to.";
STRING idcbrs = SS "Please choose the child to browse to.";
STRING id1cbr = SS "Please choose the first child to browse to.";
STRING id2cbr = SS "Please choose the second child to browse to.";
STRING idfbrs = SS "Please choose the family to browse to.";
STRING idfamk = SS "Enter Family Number to Browse to";
STRING id1fbr = SS "Please choose the first family to browse to.";
STRING id2fbr = SS "Please choose the second family to browse to.";
STRING idhbrs = SS "Please choose the father/husband to browse to.";
STRING id1hbr = SS "Please choose the first father/husband to browse to.";
STRING id2hbr = SS "Please choose the second father/husband to browse to.";
STRING idwbrs = SS "Please choose the mother/wife to browse to.";
STRING id1wbr = SS "Please choose the first mother/wife to browse to.";
STRING id2wbr = SS "Please choose the second mother/wife to browse to.";
STRING idcswp = SS "Identify a parent in the family having children swapped.";
STRING idfswp = SS "Whose families/spouses are to be swapped?";
STRING idprnt = SS "Identify one of the child's parents, if known.";

STRING scanrs = SS "Scan results:";
STRING scnnmf = SS "Enter pattern to match against single surname or given name.";
STRING scnfnm = SS "Enter pattern to match against full name.";
STRING scnrfn = SS "Enter pattern to match against refn.";
STRING scantt = SS "pattern: ";

STRING chfamily = SS "Please choose a family.";
STRING notone = SS "Please choose from among these persons.";
STRING iscnew = SS "Is this the new child? ";
STRING issnew = SS "Is this the new spouse? ";
STRING ifone  = SS "Is this the person?  Select if so.";
STRING idcfam = SS "Select the child the new child precedes or select last.";
STRING idpnxt = SS "Please identify next person to browse to.";
STRING idspse = SS "Please identify one of the spouses.";
STRING idplst = SS "Please identify person or persons to browse to.";
STRING idfcop = SS "Please choose family create operation.";
STRING entnam = SS "Do you want to enter another name?";

STRING ntprnt = SS "This person is not a spouse or parent in any family.";
STRING ntchld = SS "This person is not a child in any family.";

STRING nofath = SS "This person's father is not in the database.";
STRING nomoth = SS "This person's mother is not in the database.";
STRING nospse = SS "This person has no spouse in the database.";
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
STRING unknam = SS "There is no one in the database with that name or key.";

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
STRING cfradd = SS "Do you really want to add this source to the database?";
STRING cfeadd = SS "Do you really want to add this event to the database?";
STRING cfxadd = SS "Do you really want to add this record to the database?";
STRING cfpupt = SS "Do you really want to update this person?";
STRING cffupt = SS "Do you really want to update this family?";
STRING cfrupt = SS "Do you really want to update this source?";
STRING cfeupt = SS "Do you really want to update this event?";
STRING cfxupt = SS "Do you really want to update this record?";
STRING cfpdel = SS "Are you sure you want to remove the person from the database?";
STRING cffdel = SS "Remove this family record ?";
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
STRING less2f = SS "This person is a spouse/parent in less than two families.";

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

/* GEDCOM file */
STRING idgedf = SS "Please enter the name of the GEDCOM file.";
STRING gdcker = SS "Checking GEDCOM file %s for errors.\n";
STRING gdnadd = SS "Because of errors the GEDCOM file was not loaded.\n";
STRING dboldk = SS "No errors; adding records with original keys...";
STRING dbnewk = SS "No errors; adding records with new keys...";
STRING dbodel = SS "Using original keys, %d deleted records will be in the database.";
STRING cfoldk = SS "Use original keys from GEDCOM file?";
STRING dbdelk = SS "Adding unused keys as deleted keys...";
STRING dbrdon = SS "The database is read-only; loading has been canceled.";
STRING outarc = SS "Enter name of output archive file.";
STRING outfin = SS "Database `%s' has been saved in `%s'.";

STRING extrpt      = SS "<Choose outside this list>";
STRING whatrpt     = SS "What is the name of the program?";

STRING norpt       = SS "No report was generated.";
STRING whtout      = SS "What is the name of the output file?";
STRING opt2long    = SS "Malformed configuration file: line too long.";

/* new records */
STRING defsour     = SS "0 SOUR\n1 REFN\n1 TITL Title\n1 AUTH Author";
STRING defeven     = SS "0 EVEN\n1 REFN\n1 DATE\n1 PLAC\n1 INDI\n  2 NAME\n  2 ROLE\n1 SOUR";
STRING defothr     = SS "0 XXXX\n1 REFN";

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
STRING nonnum      = SS "At least one argument to %s is not numeric";

/* display abbreviations */
STRING dspa_mar    = SS "m. ";
STRING dspa_bir    = SS "b. ";
STRING dspa_chr    = SS "bap. ";
STRING dspa_dea    = SS "d. ";
STRING dspa_bur    = SS "bur. ";
STRING dspa_chbr   = SS "cb. ";
/* display longer forms */
STRING dspl_mar    = SS "married: ";
STRING dspl_bir    = SS "born: ";
STRING dspl_chr    = SS "bapt: ";
STRING dspl_dea    = SS "died: ";
STRING dspl_bur    = SS "buri: ";

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
STRING badnnm      = SS "This person record does not have a name line.";
STRING badenm      = SS "This person record has bad GEDCOM name syntax.";
STRING badpsx      = SS "You cannot change the sex of a parent.";
STRING badirf      = SS "REFN key is already in use.";
STRING tag2long2cnc= SS "Tag is too long to connect automatically.";

/* menus */
STRING mtitle      = SS "LifeLines %s - Genealogical DB and Programming System";
STRING cright      = SS "Copyright(c) 1991 to 1996, by T. T. Wetmore IV";
STRING plschs      = SS "Please choose an operation:";
STRING mn_ambig    = SS "Conflicting command string: %s";
STRING mn_titindi  = SS "LifeLines -- Person Browse Screen (* toggles menu)";
STRING mn_titfam   = SS "LifeLines -- Family Browse Screen (* toggles menu)";
STRING mn_titaux   = SS "LifeLines -- Auxiliary Browse Screen (* toggles menu)";
STRING mn_unkcmd   = SS "Not valid command";
STRING mn_titmain  = SS "LifeLines -- Main Menu";
STRING mn_tit2indi = SS "LifeLines -- Two Person Browse Screen (* toggles menu)";
STRING mn_tit2fam  = SS "LifeLines -- Two Family Browse Screen (* toggles menu)";

/* prompt, full list, yes list */
STRING askynq      = SS "enter y (yes) or n (no): ";
STRING askynyn     = SS "yYnN";
STRING askyny      = SS "yY";

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
STRING dataerr     = SS "Error accessing data";
STRING idhist      = SS "Choose from history";
STRING norwandro   = SS "Cannot combine immutable (-i) or read-only (-r) with read-write (-w) access.";
STRING nofandl     = SS "Cannot combine forceopen (-f) and lock (-l) flags.";

/* translation table errors */
STRING baddec      = SS "Bad decimal number format.";
STRING badhex      = SS "Bad hexidecimal number format.";
STRING norplc      = SS "No replacement string on line.";
STRING badesc      = SS "Bad escape format.";

/* many menus */
STRING mn_quit     = SS  "q  Return to main menu";
STRING mn_ret      = SS  "q  Return to previous menu";

/* main menu */
STRING mn_mmrpt    = SS "r  Generate report by entering report name";
STRING mn_mmprpt   = SS "p  Pick a report from list and run";
STRING mn_mmcset   = SS "c  Character set options";
/* utility menu */
STRING mn_uttl     = SS "What utility do you want to perform?";
STRING mn_utsave   = SS "s  Save the database in a GEDCOM file";
STRING mn_utread   = SS "r  Read in data from a GEDCOM file";
STRING mn_utkey    = SS "k  Find a person's key value";
STRING mn_utkpers  = SS "i  Identify a person from key value";
STRING mn_utdbstat = SS "d  Show database statistics";
STRING mn_utmemsta = SS "m  Show memory statistics";
STRING mn_utplaces = SS "e  Edit the place abbreviation file";
STRING mn_utusropt = SS "o  Edit the user options file";
/* extra menu */
STRING mn_xttl     = SS "What activity do you want to perform?";
STRING mn_xxbsour  = SS "s  Browse source records";
STRING mn_xxbeven  = SS "e  Browse event records";
STRING mn_xxbothr  = SS "x  Browse other records";
STRING mn_xxasour  = SS "1  Add a source record to the database";
STRING mn_xxesour  = SS "2  Edit source record from the database";
STRING mn_xxaeven  = SS "3  Add an event record to the database";
STRING mn_xxeeven  = SS "4  Edit event record from the database";
STRING mn_xxaothr  = SS "5  Add an other record to the database";
STRING mn_xxeothr  = SS "6  Edit other record from the database";
/* character set menu  (these looks best if choices padded to same length)*/
STRING mn_csttl    = SS "Character set menu";
STRING mn_cstt     = SS "t  Edit translation tables";
STRING mn_csrpt    = SS "r  Report character set setup";
STRING mn_csintcs  = SS "Internal code set: ";
STRING mn_csdsploc = SS "Display locale: ";
STRING mn_csndloc  = SS "L  Select display locale";
STRING mn_cstsort  = SS "s  Edit custom sort table        ";
STRING mn_cspref   = SS "p  Edit custom sort prefix table ";
STRING mn_cschar   = SS "c  Edit custom codepage mapping  ";
STRING mn_cslcas   = SS "l  Edit custom lower case mapping";
STRING mn_csucas   = SS "u  Edit custom upper case mapping";
STRING idsortttl   = SS "Enter valid locale name (C for non-lingual sort)";
STRING idloc       = SS "Locale: ";
/* menu for report character set issues */
STRING mn_csrpttl  = SS "Report character set menu";
STRING mn_csrptcs  = SS "Report code set: ";
STRING mn_csrptloc = SS "Report locale: ";
STRING mn_csnrloc  = SS "L  Select report locale";
/* menu for translation tables */
STRING mn_tt_ttl   = SS "Translation Tables";
STRING mn_tt_edit  = SS "e  edit individual tables (in db)";
STRING mn_tt_load  = SS "l  load a table from a file (into db)";
STRING mn_tt_save  = SS "s  save a table to a file (from db)";
STRING mn_tt_exp   = SS "x  export all tables (from db to files)";
STRING mn_tt_imp   = SS "i  import all tables (from files into db)";
STRING mn_tt_dir   = SS "export/import directory:";
/* menu for edit translation table */
STRING mn_edttttl  = SS "Which character mapping do you want to edit?";
/* strings for choosing translation table (these looks best if choices padded to same length)*/
STRING mn_tt_edin   = SS "e  Editor to Internal mapping ";
STRING mn_tt_ined   = SS "m  Internal to Editor mapping ";
STRING mn_tt_gdin   = SS "i  GEDCOM to Internal mapping ";
STRING mn_tt_ingd   = SS "x  Internal to GEDCOM mapping ";
STRING mn_tt_dsin   = SS "g  Display to Internal mapping";
STRING mn_tt_inds   = SS "d  Internal to Display mapping";
STRING mn_tt_inrp   = SS "r  Internal to Report mapping ";
/* not yet implemented choices */
STRING mn_notimpl  = SS "Not implemented yet";
/* add menu */
STRING mn_add_ttl  = SS "What do you want to add?";
STRING mn_add_indi = SS "p  Person - add new person to the database";
STRING mn_add_fam  = SS "f  Family - create family record from one or two spouses";
STRING mn_add_chil = SS "c  Child - add a child to an existing family";
STRING mn_add_spou = SS "s  Spouse - add a spouse to an existing family";
/* delete menu */
STRING mn_del_ttl  = SS "What do you want to remove?";
STRING mn_del_chil = SS "c  Child - remove a child from his/her family";
STRING mn_del_spou = SS "s  Spouse - remove a spouse from a family";
STRING mn_del_indi = SS "i  Individual - remove a person completely";
STRING mn_del_fam  = SS "f  Family - remove a family completely";
/* scan menu */
STRING mn_sca_ttl  = SS "What scan type?";
STRING mn_sca_nmfu = SS "f  Full name scan";
STRING mn_sca_nmfr = SS "n  Name fragment (whitespace-delimited) scan";
STRING mn_sca_refn = SS "r  Refn scan";
