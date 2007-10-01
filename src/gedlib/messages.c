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

/* strings used to clear the screen */
STRING empstr = SS "                                                ";
STRING empstr71 = SS "                                                                       ";


ZST qSiddbse      = N_("You must identify a database.");
ZST qSidldir      = N_("What directory holds (or will hold) the LifeLines database? (? to list)");
ZST qSidldrp      = N_("enter path: ");
ZST qSnodbse      = N_("There is no LifeLines database in that directory.");
ZST qScrdbse      = N_("Do you want to create a database there?");
ZST qSbdlkar      = N_("Argument to lock (-l) must be y or n.");
ZST qSnormls      = N_("Cannot remove last person from family.");
	/* summary of options (compiled with Finnish as optional) */
ZST qSusgFinnOpt  = N_("lines [-adkrwiflntcuFxoCzI] [database]   # Use -F for Finnish database");
	/* summary of options (compiled with Finnish as mandatory) */
ZST qSusgFinnAlw  = N_("lines [-adkrwiflntcuxoCzI] [database]   # Finnish database");
	/* summary of options (normal compile -- no Finnish support) */
ZST qSusgNorm     = N_("lines [-adkrwiflntcuxoCzI] [database]");

ZST qSronlya      = N_("The database is read only; you may not add records.");
ZST qSronlye      = N_("The database is read only; you may not change records.");
ZST qSronlym      = N_("The database is read only; you may not merge records.");
ZST qSronlyr      = N_("The database is read only; you may not remove records.");
ZST qSronly       = N_("The database is read only.");

ZST qSidbrws      = N_("Name, key, refn, list, or @:");
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

ZST qSscnrfn      = N_("Enter pattern to match against refn.");

ZST qSnotonei     = N_("Please choose from among these people.");
ZST qSnotonex     = N_("Please choose from among these records.");
ZST qSiscnew      = N_("Is this the new child? ");
ZST qSissnew      = N_("Is this the new spouse? ");
ZST qSifonei      = N_("Is this the person?  Select if so.");
ZST qSifonex      = N_("Is this the record?  Select if so.");
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
ZST qSnoysib      = N_("This person has no younger sibling in the database.");
ZST qSnoosib      = N_("This person has no older sibling in the database.");
ZST qSnoprnt      = N_("This person's parents are not in the database.");
ZST qSnohusb      = N_("This family has no male spouse/parent in the database.");
ZST qSnowife      = N_("This family has no female spouse/parent in the database.");
ZST qSnocinf      = N_("There are no children in the database for this family.");
ZST qSnocofp      = N_("This person has no children in the database.");
ZST qSnochil      = N_("No such child.");
ZST qSnopers      = N_("No such person.");
ZST qSnorec       = N_("No such record.");
ZST qSnofam       = N_("No such family.");
ZST qSnosour      = N_("No sources present.");
ZST qSidsour      = N_("Please choose the source to view.");
ZST qSnoeven      = N_("No events present.");
ZST qSideven      = N_("Please choose the event to view.");
ZST qSnoothe      = N_("No others present.");
ZST qSidothe      = N_("Please choose the other to view.");
ZST qSnonote      = N_("No notes present.");
ZST qSidnote      = N_("Please choose the note to view.");
ZST qSnoptr       = N_("No references present.");
ZST qSidptr       = N_("Please choose the reference to view.");
ZST qSduprfn      = N_("Duplicated REFN - please choose.");

ZST qSnosex       = N_("New spouse/parent has unknown sex; can't add to family.");
ZST qSnotopp      = N_("The persons are not of opposite sex; can't make family.");
ZST qSntsinf      = N_("This person is not a spouse in the family; can't remove.");
ZST qSntcinf      = N_("This person is not a child in the family; can't remove.");
ZST qSunksex      = N_("This person's sex is not known; can't make family.");
ZST qShashsb      = N_("This family already has a husband/male parent.");
ZST qShaswif      = N_("This family already has a wife/female parent.");
ZST qShasbth      = N_("This family has both spouses/parents; can't add another.");
ZST qShasnei      = N_("This family has neither spouse/parent; can't remove.");
ZST qShaslnk      = N_("This family still has links; not removed.");

ZST qSidfbys      = N_("Choose a family by selecting a spouse/parent.");
ZST qSiredit      = N_("Do you want to edit the person again? (Otherwise changes will be discarded.)");
ZST qSireditopt   = N_("Do you want to edit the person again?");
ZST qSfredit      = N_("Do you want to edit the family again? (Otherwise changes will be discarded.)");
ZST qSfreditopt   = N_("Do you want to edit the family again?");
ZST qSrredit      = N_("Do you want to edit the source again? (Otherwise changes will be discarded.)");
ZST qSrreditopt   = N_("Do you want to edit the source again?");
ZST qSeredit      = N_("Do you want to edit the event again? (Otherwise changes will be discarded.)");
ZST qSereditopt   = N_("Do you want to edit the event again?");
ZST qSxredit      = N_("Do you want to edit the record again? (Otherwise changes will be discarded.)");
ZST qSxreditopt   = N_("Do you want to edit the record again?");
ZST qSidpedt      = N_("Whom do you want to edit?");
ZST qSidredt      = N_("Which source record do you want to edit?");
ZST qSideedt      = N_("Which event record do you want to edit?");
ZST qSidxedt      = N_("What record do you want to edit?");

ZST qScfpadd      = N_("Do you really want to add this person to the database?");
ZST qScffadd      = N_("Do you really want to add this family to the database?");
ZST qScfcadd      = N_("Do you really want to add this child to the family?");
ZST qScfsadd      = N_("Do you really want to add this spouse/parent to the family?");
ZST qScfradd      = N_("Do you really want to add this source to the database?");
ZST qScfeadd      = N_("Do you really want to add this event to the database?");
ZST qScfxadd      = N_("Do you really want to add this record to the database?");
ZST qScfpupt      = N_("Do you really want to update this person?");
ZST qScffupt      = N_("Do you really want to update this family?");
ZST qScfrupt      = N_("Do you really want to update this source?");
ZST qScfeupt      = N_("Do you really want to update this event?");
ZST qScfxupt      = N_("Do you really want to update this record?");
ZST qScfpdel      = N_("Are you sure you want to remove the person from the database?");
ZST qScfodel      = N_("Are you sure you want to remove this record from the database?");
ZST qScffdel      = N_("Remove this family record?");
ZST qScffdeld     = N_("(Family %s (%s, %s)");
ZST qScfpmrg      = N_("Do you really want to merge these two persons?");
ZST qScffmrg      = N_("Do you really want to merge these two families?");
ZST qScffswp      = N_("Do you really want to swap spouse orders?");
ZST qScfchswp     = N_("Do you really want to reorder children?");
ZST qScfcrmv      = N_("Do you really want to remove this child from his/her family?");
ZST qScfsrmv      = N_("Do you really want to remove this spouse from his/her family?");
ZST qSspover      = N_("Too many spouses to display full list");
ZST qSmgsfam      = N_("These persons are children in different families.");
ZST qSmgconf      = N_("Are you sure you want to merge them?");

ZST qSbadata      = N_("There is something wrong with the data.");
ZST qSidchld      = N_("Please identify the child.");
ZST qSidsbln      = N_("Please identify one of the child's siblings.");
ZST qSidsadd      = N_("Identify spouse/parent to add to an existing family.");
ZST qSidsinf      = N_("Identify spouse/parent already in family, if known.");
ZST qSkchild      = N_("Identify child already in family.");
ZST qSiscinf      = N_("This person is already a child in a family. Add anyway?");
ZST qSidsps1      = N_("Identify a spouse/parent for the new family.");
ZST qSidsps2      = N_("Identify the second spouse/parent, if known.");
ZST qStwohsb      = N_("Both families must have husbands/fathers.");
ZST qStwowif      = N_("Both families must have wives/mothers.");

ZST qSids2fm      = N_("Identify spouse/parent in second family, if known.");
ZST qSidc2fm      = N_("Identify child in second family.");
ZST qSidp2br      = N_("Identify second person to browse to.");

ZST qScrtcfm      = N_("Create a family with this person as a child.");
ZST qScrtsfm      = N_("Create a family with this person as a spouse/parent.");
ZST qSless2c      = N_("This family has less than two children; can't swap.");
ZST qSless2f      = N_("This person is a spouse/parent in less than two families.");
ZST qSparadox     = N_("Something impossible happened. Contact tech support.");

ZST qSokcswp      = N_("The two children were swapped.");
ZST qSokfswp      = N_("The two families were swapped.");
ZST qSokcrmv      = N_("The child was removed from his/her family.");
ZST qSoksrmv      = N_("The spouse was removed from his/her family.");

ZST qSnopmrg      = N_("A person cannot be merged with him/herself.");
ZST qSnofmrg      = N_("A family cannot be merged with itself.");
ZST qSnoqmrg      = N_("Two persons with different parents cannot be merged.");
ZST qSnoxmrg      = N_("Two parents of different sexes cannot be merged.");
ZST qSdhusb       = N_("The families have different fathers/husbands; cannot merge.");
ZST qSdwife       = N_("The families have different wives/mothers; cannot merge.");
ZST qSidpdel      = N_("Who do you want to remove from the database?");
ZST qSidodel      = N_("What record do you want to remove from the database?");
ZST qSmklast      = N_("Place the child last in the family.");
ZST qSabverr      = N_("Error in abbreviations file.");
ZST qSuoperr      = N_("Error in user options file.");
ZST qScmperr      = N_("Error in character mapping file.");
ZST qSsepch       = N_("(Separator is %s)");
ZST qSaredit      = N_("Do you want to re-edit it?");

ZST qSgdpadd      = N_("%s was added to the database.");
ZST qSgdcadd      = N_("%s was added as a child.");
ZST qSgdsadd      = N_("%s was added as a spouse and/or parent.");
ZST qSgdfadd      = N_("The new family was added to the database.");
ZST qSgdpmod      = N_("%s was modified in the database.");
ZST qSgdfmod      = N_("The family was modified in the database.");
ZST qSgdrmod      = N_("The source was modified in the database.");
ZST qSgdemod      = N_("The event was modified in the database.");
ZST qSgdxmod      = N_("The record was modified in the database.");

ZST qSnofopn      = N_("Could not open file %s.");
ZST qSfn2long     = N_("Filepath too long.");

ZST qSmrkrec      = N_("Please mark a record first.");

ZST qSlstnam      = N_("The current list is now named %s.");
ZST qSlstnon      = N_("The current list is not named.");
ZST qSlstwht      = N_("What should the name of this list be?");
ZST qSlstnad      = N_("No persons were added to the current list.");
ZST qSlstpad      = N_("What persons or list do you want to add to the current list?");
ZST qSlstbot      = N_("You are at the bottom of the list.");
ZST qSlsttop      = N_("You are at the top of the list.");
ZST qSlstnew      = N_("New persons were added to the current list.");

ZST qSbadttnum    = N_("System error: illegal map code");
ZST qSnosuchtt    = N_("No such translation table in this database");

/* GEDCOM file */
ZST qSgdnadd      = N_("Because of errors the GEDCOM file was not loaded.\n");
ZST qSdboldk      = N_("No errors; adding records with original keys...");
ZST qSdbnewk      = N_("No errors; adding records with new keys...");
ZST qScfoldk      = N_("Use original keys from GEDCOM file?");
ZST qSproceed     = N_("Proceed?");
ZST qSoutarc      = N_("Enter name of output archive file.");
ZST qSoutfin      = N_("Database `%s' has been saved in `%s'.");
ZST qSmouttt      = N_("Enter name of translation table file to write");
ZST qSmintt       = N_("Enter name of translation table file to read");
ZST qSmisixr      = N_("Line %d: The person defined here has no key: skipped.");
ZST qSmisfxr      = N_("Line %d: The family defined here has no key.");
ZST qSmulper      = N_("Lines %d and %d: Person %s is multiply defined: skipped.");
ZST qSmulfam      = N_("Lines %d and %d: Family %s is multiply defined.");
ZST qSmatper      = N_("Line %d: Person %s has an incorrect key: skipped.");
ZST qSmatfam      = N_("Line %d: Family %s has an incorrect key.");
ZST qSundper      = N_("Person %s is referred to but not defined.");
ZST qSundfam      = N_("Family %s is referred to but not defined.");
ZST qSundsrc      = N_("Source %s is referred to but not defined.");
ZST qSundevn      = N_("Event %s is referred to but not defined.");
ZST qSbadlev      = N_("Line %d: This line has a level number that is too large.");
ZST qSnoname      = N_("Line %d: Person defined here has no name.");
#if 0
STRING noxref = SS "Line %d: This record has no cross reference value.";
#endif

	/* Option at bottom of list, if none in list are desired */
ZST qSextchoos     = N_("<Choose outside this list>");
	/* What report program to run ? */
ZST qSwhatrpt     = N_("What is the name of the program?");
ZST qSwhatgedc    = N_("Please enter the name of the GEDCOM file.");

ZST qSwhtout      = N_("What is the name of the output file?");
ZST qSopt2long    = N_("Malformed configuration file: line too long.");
ZST qSunsupunix   = N_("Unsupported file encoding (no multibyte encodings except UTF-8).");
ZST qSunsupuniv    = N_("Unsupported file encoding: %s.");

/* misc prompts */
ZST qSchoostrttl  = N_("Enter string for program");
ZST qSaskstr      = N_("enter string: ");
ZST qSaskint      = N_("enter integer:");
ZST qSasknam      = N_("enter name: ");
ZST qShitkey      = N_("Strike any key to continue.");

/* new records prototypes */
	/* (all-caps words are in GEDCOM language -- don't change) */
ZST qSdefsour     = N_("0 SOUR\n1 REFN\n1 TITL Title\n1 AUTH Author");
ZST qSdefeven     = N_("0 EVEN\n1 REFN\n1 DATE\n1 PLAC\n1 INDI\n  2 NAME\n  2 ROLE\n1 SOUR");
ZST qSdefothr     = N_("0 XXXX\n1 REFN");
/* end new record prototypes */

/* node.c errors */
ZST qSfileof      = N_("The file is as positioned at EOF.");
ZST qSreremp      = N_("Line %d: This line is empty; EOF?");
ZST qSrerlng      = N_("Line %d: This line is too long.");
ZST qSrernlv      = N_("Line %d: This line has no level number.");
ZST qSrerinc      = N_("Line %d: This line is incomplete.");
ZST qSrerbln      = N_("Line %d: This line has a bad link.");
ZST qSrernwt      = N_("Line %d: This line needs white space before tag.");
ZST qSrerilv      = N_("Line %d: This line has an illegal level.");
ZST qSrerwlv      = N_("The record begins at wrong level.");

/* &&begin signals */
ZST qScoredump    = N_("\nAborting now. Core dump? [y/n]");
ZST qSprogsig     = N_("Looks like a program was running.\nCheck file %1 around line %2.\n");
ZST qSsignal      = N_("signal %1: %2");
	/* system signal#0 name */
ZST qSsig00       = N_("SIGNAL 0");
ZST qSsig01       = N_("HANGUP");
ZST qSsig02       = N_("INTERRUPT");
ZST qSsig03       = N_("QUIT");
ZST qSsig04       = N_("ILLEGAL INSTRUCTION");
ZST qSsig05       = N_("TRACE TRAP");
ZST qSsig06       = N_("ABORT");
ZST qSsig07       = N_("EMT INST");
ZST qSsig08       = N_("FLOATING POINT EXCEPTION");
ZST qSsig09       = N_("KILL");
ZST qSsig10       = N_("BUS ERROR");
ZST qSsig11       = N_("SEGMENTATION ERROR");
ZST qSsig12       = N_("SYSTEM CALL ERROR");
ZST qSsig13       = N_("PIPE WRITE");
ZST qSsig14       = N_("ALARM CLOCK");
	/* system signal#15 name -- presumably user used UNIX kill command to stop lifelines */
ZST qSsig15       = N_("TERMINATE FROM KILL");
ZST qSsig16       = N_("USER SIGNAL 1");
ZST qSsig17       = N_("USER SIGNAL 2");
ZST qSsig18       = N_("DEATH OF CHILD");
ZST qSsig19       = N_("POWER-FAIL RESTART");
ZST qSsig20       = N_("WINDOW CHANGE");
ZST qSsigunk      = N_("Unknown signal");
/* &&end signals */

/* browse display stuff */
ZST qSdspl_indi   = N_("person");
ZST qSdspa_resi   = N_(", of ");
ZST qSdspl_fath   = N_("father");
ZST qSdspl_moth   = N_("mother");
ZST qSdspl_spouse = N_("spouse");
ZST qSdspl_child  = N_("child");
/* &&begin display abbreviations */
	/* m.: married */
ZST qSdspa_mar    = N_("m. ");
	/* div.: divorced */
ZST qSdspa_div    = N_("div. ");
	/* b.: born */
ZST qSdspa_bir    = N_("b. ");
	/* bap.: baptized */
ZST qSdspa_chr    = N_("bap. ");
	/* d.: died */
ZST qSdspa_dea    = N_("d. ");
	/* bur.: buried */
ZST qSdspa_bur    = N_("bur. ");
	/* cb.: child born */
ZST qSdspa_chbr   = N_("cb. ");
/* &&end display abbreviations, begin long forms */
ZST qSdspl_mar    = N_("married: ");
ZST qSdspl_bir    = N_("born: ");
ZST qSdspl_chr    = N_("bapt: ");
ZST qSdspl_dea    = N_("died: ");
ZST qSdspl_bur    = N_("buri: ");
/* &&end display long forms */

/* editing errors */
ZST qSbadind      = N_("You cannot edit the INDI line in a person record.");
ZST qSbadfmc      = N_("You cannot edit the FAMC line in a person record.");
ZST qSbadfms      = N_("You cannot edit the FAMS lines in a person record.");
ZST qSbadfam      = N_("You cannot edit the FAM line in a family record.");
ZST qSbadhsb      = N_("You cannot edit the HUSB line in a family record.");
ZST qSbadwif      = N_("You cannot edit the WIFE line in a family record.");
ZST qSbadchl      = N_("You cannot edit the CHIL lines in a family record.");
ZST qSbademp      = N_("The record is empty.");
ZST qSbadin0      = N_("The record does not begin with an INDI line.");
ZST qSbadfm0      = N_("The record does not begin with a FAM line.");
ZST qSbadsr0      = N_("The record does not begin with a SOUR line.");
ZST qSbadev0      = N_("The record does not begin with an EVEN line.");
ZST qSbadothr0    = N_("INDI, FAM, SOUR, EVEN records may not be other records.");
ZST qSbadmul      = N_("The record contains multiple level 0 lines.");
ZST qSbadenm      = N_("This person record has bad GEDCOM name syntax.");
ZST qSbadparsex   = N_("You cannot change the sex of a parent.");
ZST qSbadirefn    = N_("REFN key is already in use.");

ZST qStag2lng2cnc = N_("Tag is too long to connect automatically.");
	/* I,F,S,E,X are conventional letters, so leave them as is) */
ZST qSdbrecords   = N_("Database records");
ZST qSdbrecstats  = N_("%dI, %dF, %dS, %dE, %dX");

/* menus */
ZST qSmtitle      = N_("LifeLines %s - Genealogical DB and Programming System");
ZST qScright      = N_("Copyright(c) 1991 to 1996, by T. T. Wetmore IV");
ZST qSdbname      = N_("Current Database - %s");
	/* immutable is read-only with no reader/writer conflict protection */
ZST qSdbimmut     = N_(" (immutable)");
	/* read-only has protection against reader/writer conflict */
ZST qSdbrdonly    = N_(" (read only)");
ZST qSplschs      = N_("Please choose an operation:");
ZST qSmn_unkcmd   = N_("Not valid command");

/* prompt, full list, yes list */
ZST qSaskynq      = N_("enter y (yes) or n (no): ");
ZST qSaskynyn     = N_("yYnN"); /* valid chars for yes/no answer */
ZST qSaskyY       = N_("yY"); /* chars meaning yes answer */

/* browse menu titles */
ZST qSttlindibrw  = N_("LifeLines -- Person Browse Screen (* toggles menu)");
ZST qSttlfambrw   = N_("LifeLines -- Family Browse Screen (* toggles menu)");
ZST qSttl2perbrw  = N_("LifeLines -- Two Person Browse Screen (* toggles menu)");
ZST qSttl2fambrw  = N_("LifeLines -- Two Family Browse Screen (* toggles menu)");
ZST qSttlauxbrw   = N_("LifeLines -- Auxiliary Browse Screen (* toggles menu)");
ZST qSttllstbrw   = N_("LifeLines -- List Browse Screen (* toggles menu)");

/* list menu */

ZST qSchlistx     = N_("Commands:   Select by number, u Page Up, d Page Down, i Select, q Quit");
ZST qSvwlistx     = N_("Commands:   u Page Up, d Page Down, q Quit");
ZST qSerrlist     = N_("Messages:");

/* adding new xref */
ZST qSdefttl      = N_("Please choose from the following options:");
ZST qSnewrecis    = N_("New record is %s");
ZST qSautoxref    = N_("Insert xref automatically at bottom of current record.");
ZST qSeditcur     = N_("Edit current record now to add xref manually.");
ZST qSgotonew     = N_("Browse new record (without adding xref).");
ZST qSstaycur     = N_("Return to current record (without adding xref).");

/* misc */
ZST qSunksps      = N_("Spouse unknown");
ZST qSnohist      = N_("No more history");
ZST qSbadhistcnt  = N_("Bad history count");
ZST qSbadhistcnt2 = N_("Bad backup history count");
ZST qSbadhistlen  = N_("Bad history length");
ZST qShistclr     = N_("Delete history (%d entries)?");
ZST qSdataerr     = N_("Error accessing data");
ZST qSidhist      = N_("Choose from history");
ZST qSnorwandro   = N_("Cannot combine immutable (-i) or read-only (-r) with read-write (-w) access.");
ZST qSnofandl     = N_("Cannot combine forceopen (-f) and lock (-l) flags.");
ZST qSiddefpath   = N_("Default path: ");
ZST qSmisskeys    = N_("WARNING: missing keys");
ZST qSbadkeyptr   = N_("This does not point to another record in the database!");
ZST qSwhtfname    = N_("enter file name");
ZST qSwhtfnameext = N_("enter file name (*%s)");
ZST qSnosuchrec   = N_("There is no record with that key or reference.");
ZST qSbaddb       = N_("Database was corrupt.");

/* translation table errors */
ZST qSbaddec      = N_("Bad decimal number format.");
ZST qSbadhex      = N_("Bad hexidecimal number format.");
ZST qSnorplc      = N_("No replacement string on line.");
ZST qSnoorig      = N_("No original string on line.");
ZST qSbadesc      = N_("Bad escape format.");
ZST qSmaperr      = N_("%s: line %d (entry %d): %s");

/* many menus */
ZST qSmn_quit     = N_("q  Return to main menu");
ZST qSmn_ret      = N_("q  Return to previous menu");
ZST qSmn_exit     = N_("q  Quit program");
ZST qSmn_changedb = N_("Q  Quit current database");

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
ZST qSmn_utgdchoo = N_("R  Pick a GEDCOM file and read in");
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

/* &&end utility menu, &&begin translation table menu */
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
ZST qSmn_del_any  = N_("o  Other - remove other record completely");

/* &&end delete menu, begin search menu */
ZST qSmn_sea_ttl  = N_("How would you like to find a record?");

/* &&end search menu, begin scan status strings */
ZST qSsts_sca_ful = N_("Performing full name scan");
ZST qSsts_sca_fra = N_("Performing name fragment scan");
ZST qSsts_sca_ref = N_("Performing refn scan");
ZST qSsts_sca_src = N_("Performing source scan");
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
	/* B.C. = Before Christ (calendar) */
ZST qSdatetrl_bcA = N_("B.C.");
	/* BC = Before Christ (calendar) */
ZST qSdatetrl_bcB = N_("BC");
	/* B.C.E. = Before Common Era (calendar) */
ZST qSdatetrl_bcC = N_("B.C.E.");
	/* BCE. = Before Common Era (calendar) */
ZST qSdatetrl_bcD = N_("BCE");
	/* A.D. = Anno Domini (calendar) */
ZST qSdatetrl_adA = N_("A.D.");
	/* AD = Anno Domini (calendar) */
ZST qSdatetrl_adB = N_("AD");
	/* C.E. = Common Era (calendar) */
ZST qSdatetrl_adC = N_("C.E.");
	/* CE = Common Era (calendar) */
ZST qSdatetrl_adD = N_("CE");
	/* &&calendar pics */
	/* Julian calendar year */
ZST qScaljul      = N_("%1J");
	/* Hebrew calendar year */
ZST qScalheb      = N_("%1 HEB");
	/* French Republic calendar year */
ZST qScalfr       = N_("%1 FR");
	/* Roman calendar year -- Anno Urbe Condite ? */
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
