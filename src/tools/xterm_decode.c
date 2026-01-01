// this is a xterm escape sequence disassembler.
//
// Written by Stephen A Dum 
//            dr.doom@frontier.com
//            15 Mar 2019
// Copyright 2019 Stephen A Dum All rights reserved
// Licensed under the Open Software License version 3.0
// Xterm data from
// http://www.xfree86.org/4.7.0/ctlseqs.html
// https://vt100.net/docs/vt510-rm/SGR.html

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define VERSION "2.0"
#define ENQ 0x05        // Return Terminal Status (Ctrl-E).
                        // Default response is an empty string,
                        // but may be overridden by a resource answerbackString.
#define BEL 0x07        // Bell (Ctrl-G)
#define BS  0x08        // Backspace (Ctrl-H)
#define TAB 0x09        // Horizontal Tab (HT) (Ctrl-I)
#define LF  0x0a        // Line Feed or New Line (NL) (Ctrl-J)
#define VT  0x0b        // Vertical Tab (Ctrl-K) same as LF
#define FF  0x0c        // Form Feed or New Page (NP) (Ctrl-L) same as LF
#define CR  0x0d        // Carriage Return (Ctrl-M)
#define SO  0x0e        // Shift Out (Ctrl-N) Switch to Alternate Character Set 
                        // invokes the G1 character set.
#define SI  0x0f        // Shift In (Ctrl-O) Switch to Standard Character Set: 
                        // invokes the G0 character set (the default).
#define ESC 0x1b        // Escape (Ctrl-K)
#define SP  0x20        // Space.

// 8 bit defines
#define IND 0x84        //ESC D	Index (linefeed)
#define NEL 0x85        //ESC E	Next Line
#define HTS 0x88        //ESC H	Tab Set
#define RI  0x8d        //ESC M	Reverse Index (reverse linefeed)
#define SS2 0x8e        //ESC N	Single Shift Select of G2 Character Set
#define SS3 0x83        //ESC O	Single Shift Select of G3 Character Set
#define DCS  0x90        // Device Control String - ESC P
#define SPA  0x96        // Start of Guarded Area - ESC V
#define EPA  0x97        // End of Guarded Area - ESC W
#define SOS  0x98        // Start of String - ESC X
#define DECID 0x9a       // Return Terminal ID - ESC Z
#define SCI   0x9a       // Return Terminal ID - ESC Z
                         // Obsolete form of CSI c
#define CSI  0x9b        // Control Sequence Introducer - ESC [
#define ST   0x9c        // String Terminator  - ESC backslash
#define OSC  0x9d        // Operating System Command - ESC ]
#define PM   0x9e        // Privacy Message - ESC ^
#define APC  0x9f        // Application Program Command - ESC _

// Buffers for Label and Text data
#define MAXLABEL 120
char labelbuf[MAXLABEL];
int labellen=0;
int labelTooLong=0;
#define MAXTEXT 90
char textbuf[MAXTEXT];
int textlen=0;

// tracking current charset
char charsetname[80] = "";
char shortcharsetname[80] = "";

char msg[250];  // buffer to assemble messages
//char msg1[150]; // buffer to assemble messages
//char msg2[150]; // buffer to assemble messages

// C0 Codes
char *controlchars[32] = 
     { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
       "BS",  "TAB",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
       "DLE", "DCI", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
       "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US" };
char *C1controlchars[32] = 
     { "PAD", "HOP",  "BPH", "NBH", "IND", "NEL", "SSA", "ESA",
       "HTS", "HTJ",  "VTS", "PLD", "PLU", "RI",  "SS2", "SS3",
       "DCS", "PU1",  "PU2", "STS", "CCH", "MW",  "SPA", "EPA",
       "SOS", "SGCI", "SCI", "CSI", "ST",  "OSC", "PM",  "APC"};

// flag those characters we want to flushtext before processing
char C1flush[32] = 
     { 0, 0, 0, 0, 1, 1, 0, 0,
       1, 0, 0, 0, 0, 1, 1, 1,
       1, 0, 0, 0, 0, 0, 1, 1,
       1, 0, 1, 1, 1, 1, 1, 1};

// Dec Special Graphics
// mostly line drawing symbols 31 symbols 0x60 thru 0x7e 
// `abcdefghijklmnopqrstuvwxyz{|}~
char *linedrawingchars[31] =
    {"diamond","ckboard","TAB-sym","FF-sym",         //~abc  0x60-0x63
     "CR-sym","LF-sym","Degree Symbol","plus/minus", //defg  0x64-0x67
     "NL-sym","VT-sym","lr corner","ur corner",       //hijk  0x68-0x6b
           //NL and VT are also board of squares and Lantern symbol 
     "ul corner","ll corner","Crossover","Scan Line 1", //lmno 0x6c-0x6f
     "scan line 3", "Horizontal line","scan line 7","scan line 9", //pqrs 70-73
     "right pointing tee","left pointing tee",   //tu 0x74-0x75
     "up pointing tee","down pointing tee",      //vw 0x76-0x77
     "Vertical line", "less/equal", "greater/equal","greek Pi", //xyz{ 0x78-0x7b
     //(you can spot pprryyzz{{||}} in a lot of AT&T terminfo strings).
     "not equal","uk pound sign","bullet",      //|}~     0x7c-0x7e
     };

/* Teletype 5410v1 symbols begin here 2B,2C,2D,2E,  30 (+,-.x0)*/
char *ldc2[6] =
    {"right pointing arrow","left pointing arrow",
     "up pointing arrow","down pointing arrow",
     "","solid square block"};

/* Compiler hints */
#if defined __GNUC__
#define HINT_PARAM_UNUSED __attribute__ ((unused))
#endif

/* Function prototypes */
void usage(int e);
void flushtext(void);
void addCharToText(unsigned char text);
void addStrToText(char *text);
void addStrToLabel(char *label);
void printtoST(int term, char *label, FILE *fd);
void processCSI(FILE *fd);
void parse(int c, char* name, FILE* fd);
void decode(FILE *fd);

void usage(int e) {
    printf("xterm_decode Version " VERSION "\n");
    printf("usage:xterm_decode input... \n");
    printf("      xterm_decode\n");
    printf("      output to stdout, input is list of files, or stdin.\n");
    exit(e);

}

// flushtext - look at character set and decode line drawing characters
void flushtext(void) {
    char textname[90];
    if(textlen == 0) return;
    textbuf[textlen] = 0;

    // create string with "text " . <name of font>
    if (labelbuf[0] == 0) {  // just text
        if (shortcharsetname[0] == 0) {
            snprintf(textname,sizeof(textname),"text default font");
        } else {
            snprintf(textname,sizeof(textname),"text %s", shortcharsetname);
        }
        if (strstr(shortcharsetname,"Dec Special")) {
            //0x60-0x7e linedrawingchars 0-1e  with at least 1 char to print
            char type[80];       // tmp buffer to assemble type info
            int typelen=0;       // length of record type we are printing
            char *textptr=textbuf; // next char to process in text buffer
            int repcnt;          // count number times a char is repeated
 
            fputs(textname,stdout);
            typelen = strlen(textname);
            char *start = textptr;    // start of this batch of char
            while (textptr-textbuf < textlen) {
                char thischar=*textptr;   // 1st char in this batch
                repcnt=1;
                while(textptr - textbuf < textlen && *++textptr == thischar) {
                    repcnt++;
                }
                // print this batch of same chars
                if (thischar >= 0x60 && thischar < 0x7f) {
                    snprintf(type,sizeof(type),"%s %d-%s",
                                 (start == textbuf ? "" : ","),
                                 repcnt,
                                 linedrawingchars[thischar-0x60]);
                } else {
                    snprintf(type,sizeof(type),"%s %d-%c",
                                 (start == textbuf ? "" : ","),
                                 repcnt,thischar);
                }
                fputs(type,stdout);
                typelen += strlen(type);
                if (typelen+(textptr-start) > (80 - 25)) {  // print a line
                    printf(": '%.*s'\n",(int)(textptr-start),start);
                    start = textptr;
                    if (textptr < textbuf+textlen) {
                        fputs(textname,stdout);
                        typelen = strlen(textname);
                    }
                }
            }
            if ((textptr-start) > 0) {  // print a line
                printf(": '%.*s'\n",(int)(textptr-start),start);
            }
        } else {
            printf("%s: '%s'\n", textname, textbuf);
        }
    } else {
        printf("%s: '%s'\n", labelbuf,textbuf);
    }
    textlen = 0;
    *textbuf = 0;
    labellen = 0;
    *labelbuf = 0;
    labelTooLong=0;
}

void addCharToText(unsigned char text) {
    if (textlen >= MAXTEXT-2) flushtext();
    if (text < 32) {
        // name of control char can take 5 chars
        if (textlen >= MAXTEXT-6) flushtext();
        textbuf[textlen++] = '<';
        char *p = controlchars[(int)(text&0xff)];
        while (*p) {
            textbuf[textlen++] = *p++;
        }
        textbuf[textlen++] = '>';
        if (text == LF) flushtext();
    } else if (text > 0x7f && text <= 0x9f) {
        textbuf[textlen++] = '<';
        char *p = C1controlchars[(int)(text&0x7f)];
        while (*p) {
            textbuf[textlen++] = *p++;
        }
        textbuf[textlen++] = '>';
    } else if (isprint(text)) {
        textbuf[textlen++] = text;
    } else {
        char buf[15];
        snprintf(buf,sizeof(buf),"<0x%x>",text);
        char *p = buf;
        while (*p) {
            textbuf[textlen++] = *p++;
        }
    }
    textbuf[textlen] = 0;     // keep buf looking like a string
}

void addStrToText(char *text) {
    while (*text) {
        if (textlen >= MAXTEXT) flushtext();
        textbuf[textlen++] = *text++;
    }
}
void addStrToLabel(char *label) {
    while(*label) {
        if(labellen >= MAXLABEL-2 && labelTooLong==0) {
            printf("Error, label too long: %s\n", labelbuf);
            labelTooLong=1;
        }
        labelbuf[labellen++] = *label++;
    }
    labelbuf[labellen] = 0;
}

//String terminator is <ESC> \ or (ST -> 0x9c or BEL -> 0x07) 
// consume characters until terminator is found
// used in APC, DCS, P, PM sequences
// (OSC uses ST or BEL), 
void printtoST(int term, char *label, FILE *fd) {
    int c;
    addStrToLabel(label);
    // copy data until we see <ESC>\ or 0x9c
    while ((c = getc(fd)) != EOF) {
        addCharToText(c);
        if (c == ESC) {
            if ((c = getc(fd)) != EOF) { 
                addCharToText(c);
                if (c == '\\' ) {
                    flushtext();  // found string terminator
                    return;
                }
            } 
        } else if (c == term) {   // ST or BEL depending on command
            flushtext();
            return;
        } else if (c == LF) {   // at end of a line
            addStrToLabel(" interrupted escape sequence");
            flushtext();
            return;
        }
    }
    addStrToLabel(" interrupted escape sequence");
    flushtext();
}


// fix this sometime...
//control characters
//The xterm program recognizes both 8-bit and 7-bit control characters. 
//It generates 7-bit controls (by default) or 8-bit if S8C1T is enabled.
//The following pairs of 7-bit and 8-bit control characters are equivalent:
//
//ESC D	Index ( IND is 0x84)
//ESC E	Next Line ( NEL is 0x85)
//ESC H	Tab Set ( HTS is 0x88)
//ESC M	Reverse Index ( RI is 0x8d)
//ESC N	Single Shift Select of G2 Character Set ( SS2 is 0x8e): affects next character only
//ESC O	Single Shift Select of G3 Character Set ( SS3 is 0x8f): affects next character only
//ESC P	Device Control String ( DCS is 0x90)
//ESC V	Start of Guarded Area ( SPA is 0x96)
//ESC W	End of Guarded Area ( EPA is 0x97)
//ESC X	Start of String ( SOS is 0x98)
//ESC Z	Return Terminal ID (DECID is 0x9a). Obsolete form of CSI c (DA).
//ESC [	Control Sequence Introducer ( CSI is 0x9b)
//ESC \	String Terminator ( ST is 0x9c)
//ESC ]	Operating System Command ( OSC is 0x9d)
//ESC ^	Privacy Message ( PM is 0x9e)
//ESC _	Application Program Command ( APC is 0x9f)
// 
// so these are pairs, either see say ESC \ or 0x9c xxxx not ESC 0x9c

//processCSI
void processCSI(FILE *fd) {
    int buf[10];    //collection of parameter values
    char c;
    char lead = ' ';
    char tail = ' ';
    int cnt = -1;  // next parameter to be filled
                   // values 0..cnt are initialized to 0 when cnt is incremented
    int innumber;  // are we processing a number
    int ok;

    // format of CSI is  '<esc>[' (?|>) values (>|) (letter|{|\|)    }
    //                    values is a ; separated lists of numbers
    // read in semicolon separated parameters
    addStrToLabel("CSI");
    if ((c = getc(fd)) == EOF) {
    //        some CSI strings have a char before command char (!,",$,')
        addStrToLabel(" interrupted CSI string");
        flushtext();
        return;
    } 
    addCharToText(c);
    if (c == '?' || c == '>') {
        lead = c;
        if ((c = getc(fd)) == EOF) {
            addCharToText(c);
            addStrToLabel(" interrupted CSI string");
            flushtext();
            return;
        } 
        addCharToText(c);
    }
    innumber = 0;
    while (1) {
        if (isdigit(c)) {
            if (innumber == 0) {
                innumber = 1;   
                buf[++cnt] = 0;   // advance to next number in buffer 
                                // and initialize it
            }
            buf[cnt] = buf[cnt]*10 + c - '0';
        } else if (c ==  ';') {
            buf[++cnt] = 0;    
        } else {
            break;
        }
        if ((c = getc(fd)) == EOF) {
            addCharToText(c);
            addStrToLabel(" interrupted CSI string");
            flushtext();
            return;
        } 
        addCharToText(c);
    }
    // we have read in numbers -- if any
    if (c == '!' || c == '"' || c =='$' || c == '\'') {
        tail = c;
        if ((c = getc(fd)) == EOF) {
            addStrToLabel(" interrupted CSI string");
            flushtext();
            return;
        } 
    }
    // CSI Control Sequence Introducer (<ESC>[ or <ESC><0x9b>
    switch (c) {
    case '@':
        // CSI P s @    Insert P s (Blank) Character(s) (default = 1) (ICH)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " insert %d blank%s",buf[0],
                  buf[0] == 1 ? "" : "s");
        addStrToLabel(msg);
        flushtext();
        break;
    case 'A':
        // CSI P s A     Cursor Up P s Times (default = 1) (CUU)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor up %d Times",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'B':
        // CSI P s B     Cursor Down P s Times (default = 1) (CUD)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor down %d Times",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'C':
        // CSI P s C     Cursor Forward P s Times (default = 1) (CUF)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " CSI Cursor forward %d Times",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'D':
        //    CSI P s D     Cursor Backward P s Times (default = 1) (CUB)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor Backward %d Times",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'E':
        // CSI P s E     Cursor Next Line P s Times (default = 1) (CNL)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor Next Line %d Times",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'F':
        //    CSI P s F     Cursor Preceding Line P s Times (default = 1) (CPL)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor Preceding Line %d Times",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'G':
        // CSI P s G Cursor Character Absolute [column] (default = [row,1]) (CHA)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor to Column %d",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'H':
        // CSI P s ; P s H  Cursor Position [row;column] (default = [1,1]) (CUP)
        if (lead != ' ' || tail != ' ' || cnt > 1) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        if (cnt == 0) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Position Cursor to row %d,Col %d]",buf[0],buf[1]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'I':
        // CSI P s I  Cursor Forward Tabulation P s tab stops (default = 1) (CHT)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Cursor Forward  %d tab stops",buf[0]);

        addStrToLabel(msg);
        flushtext();
        break;
    case 'J':
        // CSI P s J    Erase in Display (ED)
        //    P s = 0 ? Erase Below (default) 
        //    P s = 1 ? Erase Above 
        //    P s = 2 ? Erase All 
        //    P s = 3 ? Erase Saved Lines (xterm)
        // CSI ? P s J  Erase in Display (DECSED)       
        //    P s = 0 ? Selective Erase Below (default) 
        //    P s = 1 ? Selective Erase Above 
        //    P s = 2 ? Selective Erase All
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if ((lead == ' ' || lead == '?') && tail == ' ' && cnt == 0) {
            if (lead == ' ') {
                addStrToLabel(" Erase Display ");
            } else {
                addStrToLabel(" Selective Erase Display ");
            }
            switch(buf[0]) {
            case 0:
                addStrToLabel("Below");
                break;
            case 1:
                addStrToLabel("Above");
                break;
            case 2:
                addStrToLabel("All");
                break;
            default:
                addStrToLabel(" Invalid count argument");
                break;
            }
        } else {
            addStrToLabel(" Invalid argument");
        }
        flushtext();
        break;
    case 'K':
        // CSI P s K    Erase in Line (EL)      
        //    P s = 0   Erase to Right (default) 
        //    P s = 1   Erase to Left 
        //    P s = 2   Erase All
        // CSI ? P s K  Erase in Line (DECSEL)  
        //    P s = 0   Selective Erase to Right (default) 
        //    P s = 1   Selective Erase to Left 
        //    P s = 2   Selective Erase All
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if ((lead == ' ' || lead == '?') && tail == ' ' && cnt == 0) {
            if (lead == ' ') {
                addStrToLabel(" Erase line ");
            } else {
                addStrToLabel(" Selective Erase line ");
            }
            switch(buf[0]) {
            case 0:
                addStrToLabel("to right");
                break;
            case 1:
                addStrToLabel("to left");
                break;
            case 2:
                addStrToLabel("All");
                break;
            default:
                addStrToLabel("Invalid count argument");
                break;
            }
        } else {
            addStrToLabel("Invalid argument");
        }
        flushtext();
        break;
    case 'L':
        // CSI P s L Insert P s Line(s) (default = 1) (IL)   
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), " Insert %d Line(s)",buf[0]);
        addStrToLabel(msg);
        flushtext();
        break;
    case 'M':
        // CSI P s M Delete P s Line(s) (default = 1) (DL)   
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Delete %d Line(s)",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'P':
        // CSI P s P Delete P s Character(s) (default = 1) (DCH)     
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
            snprintf(msg,sizeof(msg), " Delete %d character(s)",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'S':
        // CSI P s S Scroll up P s lines (default = 1) (SU)  
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Scroll up %d Line(s)",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'T':
        // CSI P s T Scroll down P s lines (default = 1) (SD)        
        // CSI P s ; P s ; P s ; P s ; P s T
        //    Initiate highlight mouse tracking. Parameters are 
        //    [func;startx;starty;firstrow;lastrow].
        //    See the section Mouse Tracking.
        //    looks like we can have 0, 1, or 5 params only
//xxxxx this isn't right cnt > 4 or from 1 to 4 ??? redo 
        if (lead != ' ' || tail != ' ' || cnt > 4 || (cnt > 1 && cnt < 4)) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            if (cnt == 0) {
                snprintf(msg,sizeof(msg), " Scroll Down %d line(s)",buf[0]);
            } else {
                snprintf(msg,sizeof(msg), " Initiate Mouse Tracking %d,%d,%d,%d,%d",
                           buf[0],buf[1],buf[2],buf[3],buf[4]);
            }
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'X':
        // CSI P s X  Erase P s Character(s) (default = 1) (ECH)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Erase %d Character(s)(s)",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'Z':
        // CSI P s Z         Cursor Backward Tabulation P s tab stops (default = 1) (CBT)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Cursor Backward Tab  %d tab stops",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case '`':
        // CSI P m `  Character Position Absolute [column] (default = [row,1]) (HPA)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Position column Absolute row unchanged, Col %d",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'b':
        // CSI P s b  Repeat the preceding graphic character P s times (REP)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Repeat Previous Graphic char  %d times",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'c':
        // CSI > P s c               Send Device Attributes (Primary DA)   
        //    P s = 0 or omitted ? request attributes from terminal. 
        //    The response depends on the decTerminalID resource setting. 
        //    ? CSI ? 1 ; 2 c (??VT100 with Advanced Video Option??) 
        //    ? CSI ? 1 ; 0 c (??VT101 with No Options??) 
        //    ? CSI ? 6 c (??VT102??) 
        //    ? CSI ? 6 0 ; 1 ; 2 ; 6 ; 8 ; 9 ; 1 5 ; c (??VT220??) 
        //    The VT100-style response parameters do not mean anything by
        //    themselves. VT220 parameters do, telling the host what 
        //    features the terminal supports: 
        //    ? 1 132-columns 
        //    ? 2 Printer 
        //    ? 6 Selective erase 
        //    ? 8 User-defined keys 
        //    ? 9 National replacement character sets 
        //    ? 1 5 Technical characters 
        //    ? 2 2 ANSI color, e.g., VT525 
        //    ? 2 9 ANSI text locator (i.e., DEC Locator mode)
        // CSI > P S c  Send Device Attributes (Secondary DA)   
        //    P s = 0 or omitted ? request the terminal?s identification code.
        //        The response depends on the decTerminalID resource setting.
        //        It should apply only to VT220 and up, but xterm extends this
        //        to VT100.
        //    ? CSI > P p ; P v ; P c c 
        //    where P p denotes the terminal type 
        //    ? 0 (??VT100??) 
        //    ? 1 (??VT220??) 
        //    and P v is the firmware version (for xterm, this is the XFree86
        //        patch number, starting with 95). In a DEC terminal, 
        //        P c indicates the ROM cartridge registration number and is
        //        always zero.
        if ((lead == ' ' || lead == '>') && tail == ' ' && cnt <= 0) {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 0;
            }
            if (lead == ' ') {
                addStrToLabel(" Send Device Primary Attributes");
            } else {
                addStrToLabel(" Send Device Secondary Attributes");
            }
        } else {
            addStrToLabel(" Invalid CSI string");
        }
        flushtext();
        break;
    case 'd':
        // CSI P m d  Line Position Absolute [row] (default = [1,column]) (VPA)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Line Position Absolutge [row] to %d, Col unchanged",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'f':
        // CSI P s ; P s f  Horizontal and Vertical Position [row;column] (default = [1,1]) (HVP)
        if (lead != ' ' || tail != ' ' || cnt > 1) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            if (cnt == 0) {
                cnt++;
                buf[cnt] = 1;
            }
            snprintf(msg,sizeof(msg), " Horizontal and Vertical Position row=%d, col=%d",buf[0],buf[1]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'g':
        // CSI P s g  Tab Clear (TBC)
        //    P s = 0 ? Clear Current Column (default) 
        //    P s = 3 ? Clear All
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
        } else {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 1;
            }
            if (buf[0] == 1) {
                addStrToLabel(" Clear Tab Current Column");
            } else if (buf[0] == 3) {
                addStrToLabel(" Clear All Tabs");
            } else {
                addStrToLabel(" Invalid CSI string");
            }
        }
        flushtext();
        break;
    case 'h':
        // CSI P m h         Set Mode (SM)   
        //    P s = 2 ? Keyboard Action Mode (AM) 
        //    P s = 4 ? Insert Mode (IRM) 
        //    P s = 1 2 ? Send/receive (SRM) 
        //    P s = 2 0 ? Automatic Newline (LNM)
        // CSI ? P m h  DEC Private Mode Set (DECSET)   
        //    P s = 1 ? Application Cursor Keys (DECCKM) 
        //    P s = 2 ? Designate USASCII for character sets G0-G3 (DECANM), and set VT100 mode. 
        //    P s = 3 ? 132 Column Mode (DECCOLM) 
        //    P s = 4 ? Smooth (Slow) Scroll (DECSCLM) 
        //    P s = 5 ? Reverse Video (DECSCNM) 
        //    P s = 6 ? Origin Mode (DECOM) 
        //    P s = 7 ? Wraparound Mode (DECAWM) 
        //    P s = 8 ? Auto-repeat Keys (DECARM) 
        //    P s = 9 ? Send Mouse X & Y on button press. See the section Mouse Tracking. 
        //    P s = 1 0 ? Show toolbar (rxvt) 
        //    P s = 1 2 ? Start Blinking Cursor (att610) 
        //    P s = 1 8 ? Print form feed (DECPFF) 
        //    P s = 1 9 ? Set print extent to full screen (DECPEX) 
        //    P s = 2 5 ? Show Cursor (DECTCEM) 
        //    P s = 3 0 ? Show scrollbar (rxvt). 
        //    P s = 3 5 ? Enable font-shifting functions (rxvt). 
        //    P s = 3 8 ? Enter Tektronix Mode (DECTEK) 
        //    P s = 4 0 ? Allow 80 ? 132 Mode 
        //    P s = 4 1 ? more(1) fix (see curses resource) 
        //    P s = 4 2 ? Enable Nation Replacement Character sets (DECNRCM) 
        //    P s = 4 4 ? Turn On Margin Bell 
        //    P s = 4 5 ? Reverse-wraparound Mode 
        //    P s = 4 6 ? Start Logging (normally disabled by a compile-time option) 
        //    P s = 4 7 ? Use Alternate Screen Buffer (unless disabled by the titeInhibit resource) 
        //    P s = 6 6 ? Application keypad (DECNKM) 
        //    P s = 6 7 ? Backarrow key sends backspace (DECBKM) 
        //    P s = 1 0 0 0 ? Send Mouse X & Y on button press and release. See the section Mouse Tracking. 
        //    P s = 1 0 0 1 ? Use Hilite Mouse Tracking. 
        //    P s = 1 0 0 2 ? Use Cell Motion Mouse Tracking. 
        //    P s = 1 0 0 3 ? Use All Motion Mouse Tracking. 
        //    P s = 1 0 1 0 ? Scroll to bottom on tty output (rxvt). 
        //    P s = 1 0 1 1 ? Scroll to bottom on key press (rxvt). 
        //    P s = 1 0 3 5 ? Enable special modifiers for Alt and NumLock keys. 
        //    P s = 1 0 3 6 ? Send ESC when Meta modifies a key (enables the metaSendsEscape resource). 
        //    P s = 1 0 3 7 ? Send DEL from the editing-keypad Delete key 
        //    P s = 1 0 4 7 ? Use Alternate Screen Buffer (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 8 ? Save cursor as in DECSC (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 9 ? Save cursor as in DECSC and use Alternate Screen Buffer, clearing it first (unless disabled by the titeInhibit resource). This combines the effects of the 1 0 4 7 and 1 0 4 8 modes. Use this with terminfo-based applications rather than the 4 7 mode. 
        //    P s = 1 0 5 1 ? Set Sun function-key mode. 
        //    P s = 1 0 5 2 ? Set HP function-key mode. 
        //    P s = 1 0 5 3 ? Set SCO function-key mode. 
        //    P s = 1 0 6 0 ? Set legacy keyboard emulation (X11R6). 
        //    P s = 1 0 6 1 ? Set Sun/PC keyboard emulation of VT220 keyboard. 
        //    P s = 2 0 0 4 ? Set bracketed paste mode.
        if (lead == ' ' && tail == ' ' && cnt <= 0) {
            switch (buf[0]) {
            case 2:
                addStrToLabel(" Set Keyboard Action Mode");
                break;
            case 4:
                addStrToLabel(" Set Insert Mode");
                break;
            case 12:
                addStrToLabel(" Set Mode Send/receive");
                break;
            case 20:
                addStrToLabel(" Set Mode Automatic Newline");
                break;
            default:
                snprintf(msg,sizeof(msg)," Set Mode %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else if (lead == '?' && tail == ' ' && cnt <= 0) {
            // CSI ? P m h  DEC Private Mode Set (DECSET)   
            switch (buf[0]) {
            case 1:
                addStrToLabel(" Set Application Cursor Keys");
                break;
            case 2:
                addStrToLabel(" Set Designate USASCII for character sets G0-G3");
                break;
            case 3:
                addStrToLabel(" Set 132 Column Mode");
                break;
            case 4:
                addStrToLabel(" Set Smooth (Slow) Scroll");
                break;
            case 5:
                addStrToLabel(" Set Reverse Video");
                break;
            case 6:
                addStrToLabel(" Set Origin Mode");
                break;
            case 7:
                addStrToLabel(" Set Wraparound Mode");
                break;
            case 8:
                addStrToLabel(" Set Auto-repeat Keys");
                break;
            case 9:
                addStrToLabel(" Set Send Mouse X & Y on button press");
                break;
            case 10:
                addStrToLabel(" Set Show toolbar");
                break;
            case 12:
                addStrToLabel(" Set Start Blinking Cursor");
                break;
            case 18:
                addStrToLabel(" Set Print form feed");
                break;
            case 19:
                addStrToLabel(" Set print extent to full screen");
                break;
            case 25:
                addStrToLabel(" Set Show Cursor");
                break;
            case 30:
                addStrToLabel(" Set Show scrollbar");
                break;
            case 35:
                addStrToLabel(" Set Enable font-shifting functions");
                break;
            case 38:
                addStrToLabel(" Set Enter Tektronix Mode");
                break;
            case 40:
                addStrToLabel(" Set Allow 80-132 Mode");
                break;
            case 41:
                addStrToLabel(" Set more(1) fix (see curses resource");
                break;
            case 42:
                addStrToLabel(" Set Enable Nation Replacement Character sets");
                break;
            case 44:
                addStrToLabel(" Set Turn On Margin Bell");
                break;
            case 45:
                addStrToLabel(" Set Reverse-wraparound Mode");
                break;
            case 46:
                addStrToLabel(" Set Start Logging");
                break;
            case 47:
                addStrToLabel(" Set Use Alternate Screen Buffer");
                break;
            case 66:
                addStrToLabel(" Set Application keypad");
                break;
            case 67:
                addStrToLabel(" Set Backarrow key sends backspace");
                break;
            case 1000:
                addStrToLabel(" Set Send Mouse X & Y on button press and release");
                break;
            case 1001:
                addStrToLabel(" Set Use Hilite Mouse Tracking");
                break;
            case 1002:
                addStrToLabel(" Set Use Cell Motion Mouse Tracking");
                break;
            case 1003:
                addStrToLabel(" Set Use All Motion Mouse Tracking");
                break;
            case 1010:
                addStrToLabel(" Set Scroll to bottom on tty output");
                break;
            case 1011:
                addStrToLabel(" Set Scroll to bottom on key press");
                break;
            case 1035:
                addStrToLabel(" Set Enable special modifiers for Alt and NumLock keys");
                break;
            case 1036:
                addStrToLabel(" Set Send ESC when Meta modifies a key");
                break;
            case 1037:
                addStrToLabel(" Set Send DEL from the editing-keypad Delete key");
                break;
            case 1047:
                addStrToLabel(" Set Use Alternate Screen Buffer");
                break;
            case 1048:
                addStrToLabel(" Set Save cursor");
                break;
            case 1049:
                addStrToLabel(" Set Save cursor and use Alternate Screen Buffer");
                break;
            case 1051:
                addStrToLabel(" Set Sun function-key mode");
                break;
            case 1052:
                addStrToLabel(" Set HP function-key mode");
                break;
            case 1053:
                addStrToLabel(" Set SCO function-key mode");
                break;
            case 1060:
                addStrToLabel(" Set legacy keyboard emulation");
                break;
            case 1061:
                addStrToLabel(" Set Sun/PC keyboard emulation of VT220 keyboard");
                break;
            case 2004:
                addStrToLabel(" Set bracketed paste mode");
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops Dec Mode Set %d invalid",buf[0]);
                addStrToLabel(msg);

            }
        } else {
            addStrToLabel(" Invalid CSI string");
        }
        flushtext();
        break;
    case 'i':
        // CSI P m i         Media Copy (MC) 
        //    P s = 0 ? Print screen (default) 
        //    P s = 4 ? Turn off printer controller mode 
        //    P s = 5 ? Turn on printer controller mode
        // CSI ? P m i               Media Copy (MC, DEC-specific)   
        //    P s = 1 ? Print line containing cursor 
        //    P s = 4 ? Turn off autoprint mode 
        //    P s = 5 ? Turn on autoprint mode 
        //    P s = 1 0 ? Print composed display, ignores DECPEX 
        //    P s = 1 1 ? Print all pages
        if (lead == ' ' && tail == ' ' && cnt <= 0) {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 0;
            }
            switch (buf[0]) {
            case 0:
                addStrToLabel(" Media Copy, print Screen");
                break;
            case 4:
                addStrToLabel(" Media Copy, Turn off printer Controller mode");
                break;
            case 5:
                addStrToLabel(" Media Copy, Turn on printer Controller mode");
                break;
            default:
                snprintf(msg,sizeof(msg)," Media Copy, Oops %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else if (lead == '?' && tail == ' ' && cnt <= 0) {
            switch (buf[0]) {
            case 0:
                addStrToLabel(" Media Copy, print Line containing cursor");
                break;
            case 4:
                addStrToLabel(" Media Copy, Turn off autoprint mode");
                break;
            case 5:
                addStrToLabel(" Media Copy, Turn on autoprint mode");
                break;
            case 10:
                addStrToLabel(" Media Copy, Print composed display");
                break;
            case 11:
                addStrToLabel(" Media Copy, Print all Pages");
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops %d invalid",buf[0]);
                addStrToLabel(msg);

            }
        } else {
            addStrToLabel("Invalid CSI string");
        }
        flushtext();
        break;
    case 'l':
        // CSI P m l         Reset Mode (RM) 
        //    P s = 2 ? Keyboard Action Mode (AM) 
        //    P s = 4 ? Replace Mode (IRM) 
        //    P s = 1 2 ? Send/receive (SRM) 
        //    P s = 2 0 ? Normal Linefeed (LNM)
        // CSI ? P m l               DEC Private Mode Reset (DECRST) 
        //    P s = 1 ? Normal Cursor Keys (DECCKM) 
        //    P s = 2 ? Designate VT52 mode (DECANM). 
        //    P s = 3 ? 80 Column Mode (DECCOLM) 
        //    P s = 4 ? Jump (Fast) Scroll (DECSCLM) 
        //    P s = 5 ? Normal Video (DECSCNM) 
        //    P s = 6 ? Normal Cursor Mode (DECOM) 
        //    P s = 7 ? No Wraparound Mode (DECAWM) 
        //    P s = 8 ? No Auto-repeat Keys (DECARM) 
        //    P s = 9 ? Don?t Send Mouse X & Y on button press 
        //    P s = 1 0 ? Hide toolbar (rxvt) 
        //    P s = 1 2 ? Stop Blinking Cursor (att610) 
        //    P s = 1 8 ? Don?t print form feed (DECPFF) 
        //    P s = 1 9 ? Limit print to scrolling region (DECPEX) 
        //    P s = 2 5 ? Hide Cursor (DECTCEM) 
        //    P s = 3 0 ? Don?t show scrollbar (rxvt). 
        //    P s = 3 5 ? Disable font-shifting functions (rxvt). 
        //    P s = 4 0 ? Disallow 80 ? 132 Mode 
        //    P s = 4 1 ? No more(1) fix (see curses resource) 
        //    P s = 4 2 ? Disable Nation Replacement Character sets (DECNRCM) 
        //    P s = 4 4 ? Turn Off Margin Bell 
        //    P s = 4 5 ? No Reverse-wraparound Mode 
        //    P s = 4 6 ? Stop Logging (normally disabled by a compile-time option) 
        //    P s = 4 7 ? Use Normal Screen Buffer 
        //    P s = 6 6 ? Numeric keypad (DECNKM) 
        //    P s = 6 7 ? Backarrow key sends delete (DECBKM) 
        //    P s = 1 0 0 0 ? Don?t Send Mouse X & Y on button press and release. See the section Mouse Tracking. 
        //    P s = 1 0 0 1 ? Don?t Use Hilite Mouse Tracking 
        //    P s = 1 0 0 2 ? Don?t Use Cell Motion Mouse Tracking 
        //    P s = 1 0 0 3 ? Don?t Use All Motion Mouse Tracking 
        //    P s = 1 0 1 0 ? Don?t scroll to bottom on tty output (rxvt). 
        //    P s = 1 0 1 1 ? Don?t scroll to bottom on key press (rxvt). 
        //    P s = 1 0 3 5 ? Disable special modifiers for Alt and NumLock keys. 
        //    P s = 1 0 3 6 ? Don?t send ESC when Meta modifies a key (disables the metaSendsEscape resource). 
        //    P s = 1 0 3 7 ? Send VT220 Remove from the editing-keypad Delete key 
        //    P s = 1 0 4 7 ? Use Normal Screen Buffer, clearing screen first if in the Alternate Screen (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 8 ? Restore cursor as in DECRC (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 9 ? Use Normal Screen Buffer and restore cursor as in DECRC (unless disabled by the titeInhibit resource). This combines the effects of the 1 0 4 7 and 1 0 4 8 modes. Use this with terminfo-based applications rather than the 4 7 mode. 
        //    P s = 1 0 5 1 ? Reset Sun function-key mode. 
        //    P s = 1 0 5 2 ? Reset HP function-key mode. 
        //    P s = 1 0 5 3 ? Reset SCO function-key mode. 
        //    P s = 1 0 6 0 ? Reset legacy keyboard emulation (X11R6). 
        //    P s = 1 0 6 1 ? Reset Sun/PC keyboard emulation of VT220 keyboard. 
        //    P s = 2 0 0 4 ? Reset bracketed paste mode.
        if (lead == '?' || tail == ' ' || cnt == 0) {
            // CSI Pm l
            switch (buf[0]) {
            case 1:
                addStrToLabel(" Dec Private Mode Reset Normal Cursor Keys");
                break;
            case 2:
                addStrToLabel(" Dec Private Mode Reset Designate Vt52 Mode");
                break;
            case 3:
                addStrToLabel(" Dec Private Mode Reset 30 Column Mode");
                break;
            case 4:
                addStrToLabel(" Dec Private Mode Reset Jump (fast) Scroll");
                break;
            case 5:
                addStrToLabel(" Dec Private Mode Reset Normal Video");
                break;
            case 6:
                addStrToLabel(" Dec Private Mode Reset Normal Cursor Mode");
                break;
            case 7:
                addStrToLabel(" Dec Private Mode Reset No Wraparound Mode");
                break;
            case 8:
                addStrToLabel(" Dec Private Mode Reset No Auto-repeat keys");
                break;
            case 9:
                addStrToLabel(" Dec Private Mode Reset Don't send Mouse X,Y on button press");
                break;
            case 10:
                addStrToLabel(" Dec Private Mode Reset Hide toolbar");
                break;
            case 12:
                addStrToLabel(" Dec Private Mode Reset Stop Blinking Cursor");
                break;
            case 18:
                addStrToLabel(" Dec Private Mode Reset Don't print form Feed");
                break;
            case 19:
                addStrToLabel(" Dec Private Mode Reset Limit print to scrolling region");
                break;
            case 25:
                addStrToLabel(" Dec Private Mode Reset Hide Cursor");
                break;
            case 30:
                addStrToLabel(" Dec Private Mode Reset Don't Show Scrollbar");
                break;
            case 35:
                addStrToLabel(" Dec Private Mode Reset Disable font shifting functions");
                break;
            case 40:
                addStrToLabel(" Dec Private Mode Reset Disallow 80 -> 132 Mode Reset");
                break;
            case 41:
                addStrToLabel(" Dec Private Mode Reset No more(1) fix");
                break;
            case 42:
                addStrToLabel(" Dec Private Mode Reset Disable Nation replacement Character sets");
                break;
            case 44:
                addStrToLabel(" Dec Private Mode Reset Turn Off Margin Bell");
                break;
            case 45:
                addStrToLabel(" Dec Private Mode Reset No Reverse wraparound mode");
                break;
            case 46:
                addStrToLabel(" Dec Private Mode Reset Stop Logging");
                break;
            case 47:
                addStrToLabel(" Dec Private Mode Reset Use Normal Screen Buffer");
                break;
            case 66:
                addStrToLabel(" Dec Private Mode Reset Numeric Keypad");
                break;
            case 67:
                addStrToLabel(" Dec Private Mode Reset Backarrow key sends delete");
                break;
            case 1000:
                addStrToLabel(" Dec Private Mode Reset Don't Send Mouse x,y on button press and release");
                break;
            case 1001:
                addStrToLabel(" Dec Private Mode Reset Don't Use Hilite Mouse Tracking");
                break;
            case 1002:
                addStrToLabel(" Dec Private Mode Reset Don't Use Cell Motion Mouse tracking");
                break;
            case 1003:
                addStrToLabel(" Dec Private Mode Reset Don't Use All Motion Mouse Tracking");
                break;
            case 1010:
                addStrToLabel(" Dec Private Mode Reset Don't scroll to bottom on tty output");
                break;
            case 1011:
                addStrToLabel(" Dec Private Mode Reset Don't Scroll to Bottom on key Press");
                break;
            case 1035:
                addStrToLabel(" Dec Private Mode Reset Disable special modifiers for Alt and Numlock keys");
                break;
            case 1036:
                addStrToLabel(" Dec Private Mode Reset Don't Send SEC when Meta Modifies a key");
                break;
            case 1037:
                addStrToLabel(" Dec Private Mode Reset Send VT220 Remove from the editing Keypad Delete Key");
                break;
            case 1047:
                addStrToLabel(" Use Normal Screen Buffer clearing first");
                break;
            case 1048:
                addStrToLabel(" Restore Cursor as in DECRC");
                break;
            case 1049:
                addStrToLabel(" Use Normal Screen Buffer and restore cursor");
                break;
            case 1051:
                addStrToLabel(" Reset Sun Function key mode");
                break;
            case 1052:
                addStrToLabel(" Reset HP function key mode");
                break;
            case 1053:
                addStrToLabel(" Reset SCO Function key mode");
                break;
            case 1060:
                addStrToLabel(" Reset Legacy Keyboard emulation");
                break;
            case 1061:
                addStrToLabel(" Reset SunPC keyboard emulatsion of VT 220 keyboard");
                break;
            case 2004:
                addStrToLabel(" Reset Bracketed paste mode");
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops Dec Mode Reset %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else if (lead == ' ' || tail == ' ' || cnt == 0) {
            switch (buf[0]) {
            case 2:
                addStrToLabel(" Reset Keyboard Action mode");
                break;
            case 4:
                addStrToLabel(" Reset Replace Mode");
                break;
            case 12:
                addStrToLabel(" Reset Send/receive");
                break;
            case 20:
                addStrToLabel(" Reset Normal Linefeed");
                break;
            default:
                snprintf(msg,sizeof(msg),"Reset Mode, Oops %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else {
            snprintf(msg,sizeof(msg),"Reset Mode, Oops %d invalid",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'm':
        // CSI P m m         Character Attributes (SGR)      
        //    P s = 0 ? Normal (default) 
        //    P s = 1 ? Bold 
        //    P s = 4 ? Underlined 
        //    P s = 5 ? Blink (appears as Bold) 
        //    P s = 7 ? Inverse 
        //    P s = 8 ? Invisible, i.e., hidden (VT300) 
        //    P s = 2 2 ? Normal (neither bold nor faint) 
        //    P s = 2 4 ? Not underlined 
        //    P s = 2 5 ? Steady (not blinking) 
        //    P s = 2 7 ? Positive (not inverse) 
        //    P s = 2 8 ? Visible, i.e., not hidden (VT300) 
        //    P s = 3 0 ? Set foreground color to Black 
        //    P s = 3 1 ? Set foreground color to Red 
        //    P s = 3 2 ? Set foreground color to Green 
        //    P s = 3 3 ? Set foreground color to Yellow 
        //    P s = 3 4 ? Set foreground color to Blue 
        //    P s = 3 5 ? Set foreground color to Magenta 
        //    P s = 3 6 ? Set foreground color to Cyan 
        //    P s = 3 7 ? Set foreground color to White 
        //    P s = 3 9 ? Set foreground color to default (original) 
        //    P s = 4 0 ? Set background color to Black 
        //    P s = 4 1 ? Set background color to Red 
        //    P s = 4 2 ? Set background color to Green 
        //    P s = 4 3 ? Set background color to Yellow 
        //    P s = 4 4 ? Set background color to Blue 
        //    P s = 4 5 ? Set background color to Magenta 
        //    P s = 4 6 ? Set background color to Cyan 
        //    P s = 4 7 ? Set background color to White 
        //    P s = 4 9 ? Set background color to default (original). 
        //          If 16-color support is compiled, the following apply.
        //          Assume that xterm?s resources are set so that the ISO
        //          color codes are the first 8 of a set of 16. Then the
        //          aixterm colors are the bright versions of the ISO colors: 
        //    P s = 9 0 ? Set foreground color to Black 
        //    P s = 9 1 ? Set foreground color to Red 
        //    P s = 9 2 ? Set foreground color to Green 
        //    P s = 9 3 ? Set foreground color to Yellow 
        //    P s = 9 4 ? Set foreground color to Blue 
        //    P s = 9 5 ? Set foreground color to Magenta 
        //    P s = 9 6 ? Set foreground color to Cyan 
        //    P s = 9 7 ? Set foreground color to White 
        //    P s = 1 0 0 ? Set background color to Black 
        //    P s = 1 0 1 ? Set background color to Red 
        //    P s = 1 0 2 ? Set background color to Green 
        //    P s = 1 0 3 ? Set background color to Yellow 
        //    P s = 1 0 4 ? Set background color to Blue 
        //    P s = 1 0 5 ? Set background color to Magenta 
        //    P s = 1 0 6 ? Set background color to Cyan 
        //    P s = 1 0 7 ? Set background color to White If xterm is 
        //                 compiled with the 16-color support disabled, 
        //                 it supports the following, from rxvt: 
        //    P s = 1 0 0 ? Set foreground and background color to default
        //                  If 88- or 256-color support is compiled,
        //                  the following apply. 
        //    P s = 3 8 ; 5 ; P s ? Set foreground color to the second P s 
        //    P s = 4 8 ; 5 ; P s ? Set background color to the second P s
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            addStrToLabel(" Invalid CSI string");
            flushtext();
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        addStrToLabel(" Character Attributes-");
        switch (buf[0]) {
        case 0:
            addStrToLabel("Normal");
            break;
        case 1:
            addStrToLabel("Bold");
            break;
        case 4:
            addStrToLabel("Underlined");
            break;
        case 5:
            addStrToLabel("Blink (appears as bold)");
            break;
        case 7:
            addStrToLabel("Inverse");
            break;
        case 8:
            addStrToLabel("Invisible");
            break;
        case 22:
            addStrToLabel("Normal(Neither bold nor faint)");
            break;
        case 24:
            addStrToLabel("Not Underlined");
            break;
        case 25:
            addStrToLabel("Steady (not blinking)");
            break;
        case 27:
            addStrToLabel("Positive (not inverse)");
            break;
        case 28:
            addStrToLabel("Visible (not hidden)");
            break;
        case 30:
            addStrToLabel("Set Forground Color to Black");
            break;
        case 31:
            addStrToLabel("Set Forground Color to Red");
            break;
        case 32:
            addStrToLabel("Set Forground Color to Green");
            break;
        case 33:
            addStrToLabel("Set Forground Color to Yellow");
            break;
        case 34:
            addStrToLabel("Set Forground Color to Blue");
            break;
        case 35:
            addStrToLabel("Set Forground Color to Magenta");
            break;
        case 36:
            addStrToLabel("Set Forground Color to Cyan");
            break;
        case 37:
            addStrToLabel("Set Forground Color to White");
            break;
        case 39:
            addStrToLabel("Set Forground Color to default (original)");
            break;
        case 40:
            addStrToLabel("Set Background Color to Black");
            break;
        case 41:
            addStrToLabel("Set Background Color to Red");
            break;
        case 42:
            addStrToLabel("Set Background Color to Green");
            break;
        case 43:
            addStrToLabel("Set Background Color to Yellow");
            break;
        case 44:
            addStrToLabel("Set Background Color to Blue");
            break;
        case 45:
            addStrToLabel("Set Background Color to Magenta");
            break;
        case 46:
            addStrToLabel("Set Background Color to Cyan");
            break;
        case 47:
            addStrToLabel("Set Background Color to White");
            break;
        case 49:
            addStrToLabel("Set Background Color to default (original)");
            break;
        case 90:
            addStrToLabel("Set forground color to Black");
            break;
        case 91:
            addStrToLabel("Set forground color to Red");
            break;
        case 92:
            addStrToLabel("Set forground color to Green");
            break;
        case 93:
            addStrToLabel("Set forground color to Yellow");
            break;
        case 94:
            addStrToLabel("Set forground color to Blue");
            break;
        case 95:
            addStrToLabel("Set forground color to Magenta");
            break;
        case 96:
            addStrToLabel("Set forground color to Cyan");
            break;
        case 97:
            addStrToLabel("Set forground color to White");
            break;
        case 100:
            addStrToLabel("Set background color to Black");
            break;
        case 101:
            addStrToLabel("Set background color to Red");
            break;
        case 102:
            addStrToLabel("Set background color to Green");
            break;
        case 103:
            addStrToLabel("Set background color to Yellow");
            break;
        case 104:
            addStrToLabel("Set background color to Blue");
            break;
        case 105:
            addStrToLabel("Set background color to Magenta");
            break;
        case 106:
            addStrToLabel("Set background color to Cyan");
            break;
        case 107:
            addStrToLabel("Set background color to White");
            break;
        default:
            snprintf(msg,sizeof(msg)," Oops %d invalid",buf[0]);
            addStrToLabel(msg);
        }
        flushtext();
        break;
    case 'n':
        // CSI P s n         Device Status Report (DSR)      
        //    P s = 5 ? Status Report CSI 0 n (??OK??) 
        //    P s = 6 ? Report Cursor Position (CPR) [row;column] as
        // CSI ? P s n          Device Status Report (DSR, DEC-specific)
        //    P s = 6 ? Report Cursor Position (CPR) [row;column] as 
        //              CSI ? r ; c R (assumes page is zero). 
        //    P s = 1 5 ? Report Printer status as CSI ? 1 0 n (ready) or 
        //                 CSI ? 1 1 n (not ready) 
        //    P s = 2 5 ? Report UDK status as CSI ? 2 0 n (unlocked) or
        //                CSI ? 2 1 n (locked) 
        //    P s = 2 6 ? Report Keyboard status as 
        //                CSI ? 2 7 ; 1 ; 0 ; 0 n (North American) 
        //    The last two parameters apply to VT400 & up, and denote keyboard 
        //                ready and LK01 respectively. 
        //    P s = 5 3 ? Report Locator status as 
        //                CSI ? 5 3 n Locator available, if compiled-in, or 
        //                CSI ? 5 0 n No Locator, if not.
        if (lead == ' ' && tail == ' ' && cnt == 0) {
            switch (buf[0]) {
            case 5:
                addStrToLabel(" Report Device Status Ok");
                break;
            case 6:
                addStrToLabel(" Report cursor Position");
                break;
            default:
                snprintf(msg,sizeof(msg)," Report Device Status %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else  if (lead == '?' && tail == ' ' && cnt == 0) {
            switch (buf[0]) {
            case 6:
                addStrToLabel(" Report Cursor Position");
                break;
            case 15:
                addStrToLabel(" Report Printer status");
                break;
            case 25:
                addStrToLabel(" Report UDK status");
                break;
            case 26:
                addStrToLabel(" Report Keyboard status");
                break;
            case 53:
                addStrToLabel(" Report Locator status");
                break;
            default:
                snprintf(msg,sizeof(msg)," Device Status Report Oops Report Status %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else {
            addStrToLabel(" Invalid CSI string");
        }
        flushtext();
        break;
    case 'p':
        // CSI ! p      Soft terminal reset (DECSTR)
        // CSI P s ; P s ? p  Set conformance level (DECSCL) 
        //               Valid values for the first parameter:
        // P s = 6 1 ? VT100 
        // P s = 6 2 ? VT200 
        // P s = 6 3 ? VT300 
        // Valid values for the second parameter: 
        // P s = 0 ? 8-bit controls 
        // P s = 1 ? 7-bit controls (always set for VT100) 
        // P s = 2 ? 8-bit controls
        if (lead == ' ' && tail == '!' && cnt == -1) {
             addStrToLabel(" Soft Terminal reset");
        } else if ( lead == ' ' && tail == '"' && cnt == 1) {
            switch (buf[0]) {
            case 0:
                addStrToLabel(" Set conformance level to VT100");
                break;
            case 1:
                addStrToLabel(" Set conformance level to VT200");
                break;
            case 2:
                addStrToLabel(" Set conformance level to VT300");
                break;
            default:
                snprintf(msg,sizeof(msg)," Set Conformance level %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else {
            addStrToLabel(" Invalid CSI string");
        }
        flushtext();
        break;
    case 'q': 
        // CSI P s ? q  Select character protection attribute (DECSCA). 
        //              Valid values for the parameter:
        //    P s = 0 ? DECSED and DECSEL can erase (default) 
        //    P s = 1 ? DECSED and DECSEL cannot erase 
        //    P s = 2 ? DECSED and DECSEL can erase

        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if (lead == ' ' && tail == '"' && cnt == 0) {
            addStrToLabel("Select character protection attribute ");
            switch (buf[0]) {
            case 0:
                addStrToLabel("can erase");
                break;
            case 1:
                addStrToLabel("cannot erase");
                break;
            case 2:
                addStrToLabel("can erase");
                break;
            default:
                snprintf(msg,sizeof(msg)," %d invalid",buf[0]);
            }
            addStrToLabel(msg);
        } else {
            addStrToLabel(" Invalid CSI string");
        }
        flushtext();
        break;
    case 'r':
        // CSI P s ; P s r  Set Scrolling Region [top;bottom] 
        //                      (default = full size of window) (DECSTBM)
        // CSI ? P m r  Restore DEC Private Mode Values. The value of P s 
        //              previously saved is restored. P s values are the same as for DECSET.
        // CSI P t ; P l ; P b ; P r ; P s $ r
        // Change Attributes in Rectangular Area (DECCARA).
        // P t ; P l ; P b ; P r denotes the rectangle. 
        // P s denotes the SGR attributes to change: 0, 1, 4, 5, 7
       
        if (lead != ' ' && tail == ' ' && cnt == -1) {
            // CSI r     Save cursor (ANSI.SYS)
            addStrToLabel(" Set Scrolling Region full size of window");
        } else if (lead != ' ' && tail == ' ' && cnt == 2) {
            snprintf(msg,sizeof(msg)," Set Scrolling Region [%d,%d]",
                                      buf[0],buf[1]);
            addStrToLabel(msg);
        } else if (lead != ' ' && tail == ' ' ) {
            addStrToLabel(" Set Scrolling Region invalid arguments");
        } else if (lead != '?' && tail == ' ' && cnt == 1) {
            switch (buf[0]) {
            case 1:
                addStrToLabel(" Dec Private Mode Restore Normal Cursor Keys");
                break;
            case 2:
                addStrToLabel(" Dec Private Mode Restore Designate Vt52 Mode");
                break;
            case 3:
                addStrToLabel(" Dec Private Mode Restore 30 Column Mode");
                break;
            case 4:
                addStrToLabel(" Dec Private Mode Restore Jump (fast) Scroll");
                break;
            case 5:
                addStrToLabel(" Dec Private Mode Restore Normal Video");
                break;
            case 6:
                addStrToLabel(" Dec Private Mode Restore Normal Cursor Mode");
                break;
            case 7:
                addStrToLabel(" Dec Private Mode Restore No Wraparound Mode");
                break;
            case 8:
                addStrToLabel(" Dec Private Mode Restore No Auto-repeat keys");
                break;
            case 9:
                addStrToLabel(" Dec Private Mode Restore Don't send Mouse X,Y on button press");
                break;
            case 10:
                addStrToLabel(" Dec Private Mode Restore Hide toolbar");
                break;
            case 12:
                addStrToLabel(" Dec Private Mode Restore Stop Blinking Cursor");
                break;
            case 18:
                addStrToLabel(" Dec Private Mode Restore Don't print form Feed");
                break;
            case 19:
                addStrToLabel(" Dec Private Mode Restore Limit print to scrolling region");
                break;
            case 25:
                addStrToLabel(" Dec Private Mode Restore Hide Cursor");
                break;
            case 30:
                addStrToLabel(" Dec Private Mode Restore Don't Show Scrollbar");
                break;
            case 35:
                addStrToLabel(" Dec Private Mode Restore Disable font shifting functions");
                break;
            case 40:
                addStrToLabel(" Dec Private Mode Restore Disallow 80 -> 132 Mode");
                break;
            case 41:
                addStrToLabel(" Dec Private Mode Restore No more(1) fix");
                break;
            case 42:
                addStrToLabel(" Dec Private Mode Restore Disable Nation replacement Character sets");
                break;
            case 44:
                addStrToLabel(" Dec Private Mode Restore Turn Off Margin Bell");
                break;
            case 45:
                addStrToLabel(" Dec Private Mode Restore No Reverse wraparound mode");
                break;
            case 46:
                addStrToLabel(" Dec Private Mode Restore Stop Logging");
                break;
            case 47:
                addStrToLabel(" Dec Private Mode Restore Use Normal Screen Buffer");
                break;
            case 66:
                addStrToLabel(" Dec Private Mode Restore Numeric Keypad");
                break;
            case 67:
                addStrToLabel(" Dec Private Mode Restore Backarrow key sends delete");
                break;
            case 1000:
                addStrToLabel(" Dec Private Mode Restore Don't Send Mouse x,y on button press and release");
                break;
            case 1001:
                addStrToLabel(" Dec Private Mode Restore Don't Use Hilite Mouse Tracking");
                break;
            case 1002:
                addStrToLabel(" Dec Private Mode Restore Don't Use Cell Motion Mouse tracking");
                break;
            case 1003:
                addStrToLabel(" Dec Private Mode Restore Don't Use All Motion Mouse Tracking");
                break;
            case 1010:
                addStrToLabel(" Dec Private Mode Restore Don't scroll to bottom on tty output");
                break;
            case 1011:
                addStrToLabel(" Dec Private Mode Restore Don't Scroll to Bottom on key Press");
                break;
            case 1035:
                addStrToLabel(" Dec Private Mode Restore Disable special modifiers for Alt and Numlock keys");
                break;
            case 1036:
                addStrToLabel(" Dec Private Mode Restore Don't Send SEC when Meta Modifies a key");
                break;
            case 1037:
                addStrToLabel(" Dec Private Mode Restore Send VT220 Remove from the editing Keypad Delete Key");
                break;
            case 1047:
                addStrToLabel(" Restore Use Normal Screen Buffer clearing first");
                break;
            case 1048:
                addStrToLabel(" Restore Cursor as in DECRC");
                break;
            case 1049:
                addStrToLabel(" Restore Use Normal Screen Buffer and restore cursor");
                break;
            case 1051:
                addStrToLabel(" Restore Sun Function key mode");
                break;
            case 1052:
                addStrToLabel(" Restore HP function key mode");
                break;
            case 1053:
                addStrToLabel(" Restore SCO Function key mode");
                break;
            case 1060:
                addStrToLabel(" Restore Legacy Keyboard emulation");
                break;
            case 1061:
                addStrToLabel(" Restore SunPC keyboard emulatsion of VT 220 keyboard");
                break;
            case 2004:
                addStrToLabel(" Restore Bracketed paste mode");
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops Dec Mode Restore %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        } else if (lead != ' ' && tail == '$' && cnt > 3) {
            // CSI P t ; P l ; P b ; P r ; P s $ r
            // 5 or more args
            snprintf(msg,sizeof(msg),
                     " Change attributes in rectangle t %d,l %d,b %d,r %d",
                     buf[0],buf[1],buf[2],buf[3]);
            addStrToLabel(msg);
            
            int i=4;
            while (i < cnt) {
                switch (buf[0]) {
                case 0:
                    addStrToLabel(" All Attributes");
                    break;
                case 1:
                    addStrToLabel(" Bold");
                    break;
                case 4:
                    addStrToLabel(" Underline");
                    break;
                case 5:
                    addStrToLabel(" Blinking");
                    break;
                case 7:
                    addStrToLabel(" Negative Image");
                    break;
                default:
                    addStrToLabel(" invalid");
                }
                i++;
            }
            
        } else {
            addStrToLabel(" invalid");
        }
        flushtext();
        break;
    case 's':
        // CSI s     Save cursor (ANSI.SYS)
        // CSI ? P m s       Save DEC Private Mode Values. P s values are 
        //                   the same as for DECSET.
        if (lead != ' ' && tail == ' ' && cnt == -1) {
            // CSI s     Save cursor (ANSI.SYS)
            addStrToLabel(" Save Cursor");
        } else if (lead != '?' && tail == ' ' && cnt == 0) {
             // CSI ? P m s       Save DEC Private Mode Values. P s values are 
             //                   the same as for DECSET.
            switch (buf[0]) {
            case 1:
                addStrToLabel(" Dec Private Mode Save Normal Cursor Keys");
                break;
            case 2:
                addStrToLabel(" Dec Private Mode Save Designate Vt52 Mode");
                break;
            case 3:
                addStrToLabel(" Dec Private Mode Save 30 Column Mode");
                break;
            case 4:
                addStrToLabel(" Dec Private Mode Save Jump (fast) Scroll");
                break;
            case 5:
                addStrToLabel(" Dec Private Mode Save Normal Video");
                break;
            case 6:
                addStrToLabel(" Dec Private Mode Save Normal Cursor Mode");
                break;
            case 7:
                addStrToLabel(" Dec Private Mode Save No Wraparound Mode");
                break;
            case 8:
                addStrToLabel(" Dec Private Mode Save No Auto-repeat keys");
                break;
            case 9:
                addStrToLabel(" Dec Private Mode Save Don't send Mouse X,Y on button press");
                break;
            case 10:
                addStrToLabel(" Dec Private Mode Save Hide toolbar");
                break;
            case 12:
                addStrToLabel(" Dec Private Mode Save Stop Blinking Cursor");
                break;
            case 18:
                addStrToLabel(" Dec Private Mode Save Don't print form Feed");
                break;
            case 19:
                addStrToLabel(" Dec Private Mode Save Limit print to scrolling region");
                break;
            case 25:
                addStrToLabel(" Dec Private Mode Save Hide Cursor");
                break;
            case 30:
                addStrToLabel(" Dec Private Mode Save Don't Show Scrollbar");
                break;
            case 35:
                addStrToLabel(" Dec Private Mode Save Disable font shifting functions");
                break;
            case 40:
                addStrToLabel(" Dec Private Mode Save Disallow 80 -> 132 Mode");
                break;
            case 41:
                addStrToLabel(" Dec Private Mode Save No more(1) fix");
                break;
            case 42:
                addStrToLabel(" Dec Private Mode Save Disable Nation replacement Character sets");
                break;
            case 44:
                addStrToLabel(" Dec Private Mode Save Turn Off Margin Bell");
                break;
            case 45:
                addStrToLabel(" Dec Private Mode Save No Reverse wraparound mode");
                break;
            case 46:
                addStrToLabel(" Dec Private Mode Save Stop Logging");
                break;
            case 47:
                addStrToLabel(" Dec Private Mode Save Use Normal Screen Buffer");
                break;
            case 66:
                addStrToLabel(" Dec Private Mode Save Numeric Keypad");
                break;
            case 67:
                addStrToLabel(" Dec Private Mode Save Backarrow key sends delete");
                break;
            case 1000:
                addStrToLabel(" Dec Private Mode Save Don't Send Mouse x,y on button press and release");
                break;
            case 1001:
                addStrToLabel(" Dec Private Mode Save Don't Use Hilite Mouse Tracking");
                break;
            case 1002:
                addStrToLabel(" Dec Private Mode Save Don't Use Cell Motion Mouse tracking");
                break;
            case 1003:
                addStrToLabel(" Dec Private Mode Save Don't Use All Motion Mouse Tracking");
                break;
            case 1010:
                addStrToLabel(" Dec Private Mode Save Don't scroll to bottom on tty output");
                break;
            case 1011:
                addStrToLabel(" Dec Private Mode Save Don't Scroll to Bottom on key Press");
                break;
            case 1035:
                addStrToLabel(" Dec Private Mode Save Disable special modifiers for Alt and Numlock keys");
                break;
            case 1036:
                addStrToLabel(" Dec Private Mode Save Don't Send SEC when Meta Modifies a key");
                break;
            case 1037:
                addStrToLabel(" Dec Private Mode Save Send VT220 Remove from the editing Keypad Delete Key");
                break;
            case 1047:
                addStrToLabel(" Save Use Normal Screen Buffer clearing first");
                break;
            case 1048:
                addStrToLabel(" Save Cursor as in DECRC");
                break;
            case 1049:
                addStrToLabel(" Save Use Normal Screen Buffer and restore cursor");
                break;
            case 1051:
                addStrToLabel(" Save Sun Function key mode");
                break;
            case 1052:
                addStrToLabel(" Save HP function key mode");
                break;
            case 1053:
                addStrToLabel(" Save SCO Function key mode");
                break;
            case 1060:
                addStrToLabel(" Save Legacy Keyboard emulation");
                break;
            case 1061:
                addStrToLabel(" Save SunPC keyboard emulatsion of VT 220 keyboard");
                break;
            case 2004:
                addStrToLabel(" Save Bracketed paste mode");
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops CSI Dec Mode Restore %d invalid",buf[0]);
                addStrToLabel(msg);
            }
        //} else if (lead != ' ' && tail == ' ' && cnt == 3) {
        //    // ? save cursor ??
        } else {
            addStrToLabel(" Save invalid");
        }
        flushtext();
        break;
    case 't':   
        // CSI P s ; P s ; P s t  Window manipulation (from dtterm, as well 
        //                   as extensions). These controls may be disabled 
        //                   using the allowWindowOps resource. Valid values 
        //                   for the first (and any additional parameters) are:
        //    P s = 1 ? De-iconify window. 
        //    P s = 2 ? Iconify window. 
        //    P s = 5 ? Raise the xterm window to the front of the stacking order. 
        //    P s = 6 ? Lower the xterm window to the bottom of the stacking order. 
        //    P s = 7 ? Refresh the xterm window. 
        //    P s = 1 1 ? Report xterm window state. If the xterm window is open (non-iconified), it returns CSI 1 t . If the xterm window is iconified, it returns CSI 2 t . 
        //    P s = 1 3 ? Report xterm window position as CSI 3 ; x; yt 
        //    P s = 1 4 ? Report xterm window in pixels as CSI 4 ; height ; width t 
        //    P s = 1 8 ? Report the size of the text area in characters as CSI 8 ; height ; width t 
        //    P s = 1 9 ? Report the size of the screen in characters as CSI 9 ; height ; width t 
        //    P s = 2 0 ? Report xterm window?s icon label as OSC L label ST 
        //    P s = 2 1 ? Report xterm window?s title as OSC l title ST 
        //    P s >= 2 4 ? Resize to P s lines (DECSLPP)
        //    2 args
        //    P s = 9 ; 0 ? Restore maximized window. 
        //    P s = 9 ; 1 ? Maximize window (i.e., resize to screen size). 
        // 3 args
        //    P s = 3 ; x ; y ? Move window to [x, y]. 
        //    P s = 4 ; height ; width ? Resize the xterm window to height and width in pixels. 
        //    P s = 8 ; height ; width ? Resize the text area to [height;width] in characters. 
        // CSI P t ; P l ; P b ; P r ; P s $ t  Reverse Attributes in 
        //                                      Rectangular Area (DECRARA). 
        addStrToLabel(" Window manipulation");
        if (lead == ' ' && tail == ' ') {
            // CSI P s ; P s ; P s t       
            if (cnt == 0) { // 1 arg
                if (buf[0] >= 24) {
                    snprintf(msg,sizeof(msg)," CSI Resize xterm to %d lines",buf[0]);
                    addStrToLabel(msg);
                } else {
                    switch (buf[0]) {
                    case 1:
                        addStrToLabel(" De-iconify window");
                        break;
                    case 2:
                        addStrToLabel(" Iconify Window");
                        break;
                    case 5:
                        addStrToLabel(" Raise xterm to front of stacking order");
                        break;
                    case 6:
                        addStrToLabel(" Lower Xterm to bottom of stacking order");
                        break;
                    case 7:
                        addStrToLabel(" Refresh xterm");
                        break;
                    case 11:
                        addStrToLabel(" Report xterm window state");
                        break;
                    case 13:
                        addStrToLabel(" Report xterm window position");
                        break;
                    case 14:
                        addStrToLabel(" Report window size in pixels");
                        break;
                    case 18:
                        addStrToLabel(" Report size of text area in characters");
                        break;
                    case 19:
                        addStrToLabel(" Report size of screen in characters");
                        break;
                    case 20:
                        addStrToLabel(" Report xterm window's icon label");
                        break;
                    case 21:
                        addStrToLabel(" Report xterm window's title");
                        break;
                    default:
                        addStrToLabel(" invalid command");
                    }
                }
            } else if (cnt == 1) { // 2 args
                if(buf[0] == 9) {
                    if (buf[1] == 0) {
                        addStrToLabel("CSI Restore Maximized window");
                    } else if (buf[1] == 1) {
                        addStrToLabel("CSI Maximize window");
                    } else {
                        addStrToLabel(" invalid command");
                    }
                } else {
                    addStrToLabel(" invalid command");
                }
            } else if (cnt == 2) { // 3 args
                snprintf(msg,sizeof(msg),"[%d,%d]",buf[1],buf[2]);
                switch (buf[0]) {
                case 3:
                    addStrToLabel(" CSI Move window to ");
                    addStrToLabel(msg);
                    break;
                case 4:
                    addStrToLabel("CSI Resize xterm to");
                    addStrToLabel(msg);
                    addStrToLabel(" pixels");
                    break;
                case 8:
                    addStrToLabel("CSI Resize text area to ");
                    addStrToLabel(msg);
                    addStrToLabel(" in characters");
                    break;
                default: 
                    addStrToLabel(" invalid command");
                }
            }
        } else if (lead == ' ' && tail == '$' && cnt >= 4) {
            // CSI P t ; P l ; P b ; P r ; P s $ t  Reverse Attributes in 
            //                                      Rectangular Area (DECRARA). 
            snprintf(msg,sizeof(msg),
                     "CSI Reverse attributes in rectangle t %d,l %d,b %d,r %d",
                     buf[0],buf[1],buf[2],buf[3]);
            addStrToLabel(msg);
            
            int i=4;
            while (i < cnt) {
                switch (buf[0]) {
                case 1:
                    addStrToLabel(" Bold");
                    break;
                case 4:
                    addStrToLabel(" Underline");
                    break;
                case 5:
                    addStrToLabel(" Blinking");
                    break;
                case 7:
                    addStrToLabel(" Negative Image");
                    break;
                default:
                    snprintf(msg,sizeof(msg)," %d invalid",buf[0]);
                    addStrToLabel(msg);
                }
                i++;
            }
        }
        flushtext();
        break;
    case 'u':
        // CSI u     Save cursor (ANSI.SYS)  
       addStrToLabel("CSI Save Cursor");
       flushtext();
       break;
    case 'v':
        // CSI P t ; P l ; P b ; P r ; P p ; P t ; P l ; P p $ v
        //    Copy Rectangular Area (DECCRA)    
        //    P t ; P l ; P b ; P r denotes the rectangle. 
        //    P p denotes the source page. 
        //    P t ; P l denotes the target location. 
        //    P p denotes the target page.
        if (lead == ' ' && tail == ' ' && cnt == 7) {
            snprintf(msg,sizeof(msg),"CSI Copy Rectangular Area tlbr %d,%d,%d,%d to %d,%d,%d,%d",
               buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

            addStrToLabel(msg);
        } else {
            addStrToLabel("CSI Copy Rectangle Area invalid args");
        }
        flushtext();
        break;
    case 'w':
        // CSI P t ; P l ; P b ; P r ? w     
        //    Enable Filter Rectangle (DECEFR) 
        //    Parameters are [top;left;bottom;right]. 
        //    Defines the coordinates of a filter rectangle and activates it. 
        //    Anytime the locator is detected outside of the filter rectangle,
        //    an outside rectangle event is generated and the rectangle is 
        //    disabled. Filter rectangles are always treated as "one-shot"
        //    events. Any parameters that are omitted default to the current 
        //    locator position. If all parameters are omitted, any locator
        //    motion will be reported. DECELR always cancels any prevous
        //    rectangle definition.
        if (lead == ' ' && tail == ' ' && cnt < 0) {
            // no parameter Enable filter Rectangle - Report any locator motion
            addStrToLabel("CSI Enable Filter Rectangle, Report any locator motion");
        } else if (lead == ' ' && tail == ' ' && cnt < 1) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,clp,clp,clp", buf[0]);
            addStrToLabel(msg);
        } else if (lead == ' ' && tail == ' ' && cnt < 2) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,clp,clp", buf[0],buf[1]);
            addStrToLabel(msg);
        } else if (lead == ' ' && tail == ' ' && cnt < 3) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,%d,clp", buf[0],buf[1],buf[2]);
            addStrToLabel(msg);
        } else if (lead == ' ' && tail == ' ' && cnt < 4) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,%d,%d", buf[0],buf[1],buf[2],buf[3]);
            addStrToLabel(msg);
        } else {
            addStrToLabel("CSI Enable Filter Rectangle invalid args");
        }
        flushtext();
        break;
    case 'x':
        // CSI P s x         Request Terminal Parameters (DECREQTPARM)       
        //    if P s is a "0" (default) or "1", and xterm is emulating VT100,
        //    the control sequence elicits a response of the same form whose
        //    parameters describe the terminal: 
        //    P s ? the given P s incremented by 2. 
        //    1 ? no parity 
        //    1 ? eight bits 
        //    1 2 8 ? transmit 38.4k baud 
        //    1 2 8 ? receive 38.4k baud 
        //    1 ? clock multiplier 
        //    0 ? STP flags
        // CSI P s x         Select Attribute Change Extent (DECSACE).       
        //    P s = 0 ? from start to end position, wrapped 
        //    P s = 1 ? from start to end position, wrapped 
        //    P s = 2 ? rectangle (exact).
        // CSI P c ; P t ; P l ; P b ; P r $ x       
        //    Fill Rectangular Area (DECFRA). 
        //    P c is the character to use. 
        //    P t ; P l ; P b ; P r denotes the rectangle.
        if (lead == ' ' && tail == ' ' && cnt <= 0) {
            // these 1st 2 commands seem contradictory just report name
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 0;
            }
            addStrToLabel("CSI Request Terminal Parameters");
        } else if (lead == ' ' && tail == ' ' && cnt <= 0) {
            // if 1st param is really a char this fails
            snprintf(msg,sizeof(msg),"Fill Rectangular area with %c tlbr= "
                    "%d,%d,%d,%d", buf[0],buf[1],buf[2],buf[3],buf[4]);
            addStrToLabel(msg);
        } else {
            addStrToLabel("CSI Copy Rectangle Area invalid args");
        }
        flushtext();
        break;
    case 'z':
        // CSI P s ; P u ? z Enable Locator Reporting (DECELR)       
        //    Valid values for the first parameter: 
        //    P s = 0 ? Locator disabled (default) 
        //    P s = 1 ? Locator enabled 
        //    P s = 2 ? Locator enabled for one report, then disabled 
        //    The second parameter specifies the coordinate unit for locator reports. 
        //    Valid values for the second parameter: 
        //    P u = 0 or omitted ? default to character cells 
        //    P u = 1 ? device physical pixels 
        //    P u = 2 ? character cells
        // CSI P t ; P l ; P b ; P r $ z     
        //    Erase Rectangular Area (DECERA). 
        //    P t ; P l ; P b ; P r denotes the rectangle.
        if (lead == ' ' && tail == '`' && cnt <= 0) {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 0;
            }
            if (cnt == 0) {
                cnt++;
                buf[cnt] = 0;
            }
            ok = 1;
            switch (buf[0]) {
            case 0:
                addStrToLabel("CSI Disable Locator Reporting");
                break;
            case 1:
                addStrToLabel("CSI Enable Locator Reporting");
                break;
            case 2:
                addStrToLabel("CSI Enable Locator Reporting once, then disable");
                break;
            default:
                addStrToLabel(" Invalid CSI Enable Locator Reporting Command");
                ok=0;
            }
            if (ok) {
                switch (buf[1]) {
                case 0:
                    addStrToLabel(" for Character cells");
                    break;
                case 1:
                    addStrToLabel(" for physical pixels");
                    break;
                case 2:
                    addStrToLabel(" for Character cells");
                    break;
                default:
                    addStrToLabel(" Invalid CSI Enable Locator Reporting Command");

                }
            }
        } else if (lead == ' ' && tail == '$' && cnt == 3) {
            snprintf(msg,sizeof(msg),"CSI Erase Rectangular Area, tlbr %d,%d,%d,%d",
                     buf[0],buf[1],buf[2],buf[3]);
            addStrToLabel(msg);

        } else {
            addStrToLabel(" Invalid CSI Enable Locator Reporting Command");
        }
        flushtext();
        break;
    case '{':
        // CSI P m ? {               Select Locator Events (DECSLE)   //}
        //    Valid values for the first (and any additional parameters) are: 
        //    P s = 0 ? only respond to explicit host requests (DECRQLP) 
        //    (default) also cancels any filter rectangle 
        //    P s = 1 ? report button down transitions 
        //    P s = 2 ? do not report button down transitions 
        //    P s = 3 ? report button up transitions 
        //    P s = 4 ? do not report button up transitions
        // CSI P t ; P l ; P b ; P r $ {     //}     
        //    Selective Erase Rectangular Area (DECSERA). 
        //    P t ; P l ; P b ; P r denotes the rectangle.
        ok = 1;
        if (lead == ' ' && tail == '`' && cnt <= 0) {
            if (cnt == -1) {
                cnt++;
                buf[cnt] = 0;
            }
            switch (buf[0]) {
            case 0:
                addStrToLabel("CSI Select Locator Events: report button down transitions");
                break;
            case 1:
                addStrToLabel("CSI Select Locator Events: don't report button down transitions");
                break;
            case 2:
                addStrToLabel("CSI Select Locator Events: report button up transitions");
                break;
            case 3:
                addStrToLabel("CSI Select Locator Events: don't report button up transitions");
                break;
            default:
                addStrToLabel("CSI Select Locator Events: Invalid argument");
            }
        } else if (lead == ' ' && tail == '$' && cnt == 3) {
            snprintf(msg,sizeof(msg),"CSI Selective Erase Rectangular Area, tlbr %d,%d,%d,%d",
                     buf[0],buf[1],buf[2],buf[3]);
            addStrToLabel(msg);

        }
        flushtext();
        break;
    case '|':
        // CSI P s ? |               Request Locator Position (DECRQLP)      
        //    Valid values for the parameter are: 
        //    P s = 0 , 1 or omitted ? transmit a single DECLRP locator 
        //    report If Locator Reporting has been enabled by a DECELR,
        //    xterm will respond with a DECLRP Locator Report. This report is
        //    also generated on button up and down events if they have been
        //    enabled with a DECSLE, or when the locator is detected outside
        //    of a filter rectangle, if filter rectangles have been enabled
        //    with a DECEFR. ? CSI P e ; P b ; P r ; P c ; P p & w Parameters
        //    are [event;button;row;column;page]. 
        //    Valid values for the event: 
        //    P e = 0 ? locator unavailable - no other parameters sent 
        //    P e = 1 ? request - xterm received a DECRQLP 
        //    P e = 2 ? left button down 
        //    P e = 3 ? left button up 
        //    P e = 4 ? middle button down 
        //    P e = 5 ? middle button up 
        //    P e = 6 ? right button down 
        //    P e = 7 ? right button up 
        //    P e = 8 ? M4 button down 
        //    P e = 9 ? M4 button up 
        //    P e = 1 0 ? locator outside filter rectangle 
        //    ??button?? parameter is a bitmask indicating which buttons 
        //    are pressed: 
        //    P b = 0 ? no buttons down 
        //    P b & 1 ? right button down 
        //    P b & 2 ? middle button down 
        //    P b & 4 ? left button down 
        //    P b & 8 ? M4 button down 
        //    ??row?? and ??column?? parameters are the coordinates of the
        //    locator position in the xterm window, encoded as ASCII decimal. 
        //    The ??page?? parameter is not used by xterm, and will be omitted.
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if (lead == ' ' && tail == '`' && (buf[cnt] == 0 || buf[cnt] == 1)) {
            addStrToLabel("CSI Request Locator Position");
        } else {
            addStrToLabel("CSI Request Locator Position - invalid argument");
        }
        break;


        //    SGI Attributes
        //    0 All attributes off
        // 1    Bold
        // 4    Underline
        // 5    Blinking
        // 7    Negative image
        // 8    Invisible image
        // 10   The ASCII character set is the current 7-bit display character
        //      set (default)?SCO Console only.
        // 11   Map Hex 00-7F of the PC character set codes to the current
        //      7-bit display character set?SCO Console only.
        // 12   Map Hex 80-FF of the current character set to the current 
        //      7-bit display character set?SCO Console only.
        // 22   Bold off
        // 24   Underline off
        // 25   Blinking off
        // 27   Negative image off
        // 28   Invisible image off
        // examples
        // CSI 7 m              Display negative image of text
        // CSI 0 ; 1 ; 5 ; 4 m  Reset all, then display bold, blinking &
        //                      underline
    default:
            addStrToLabel("unprocessed CSI");
    }
    flushtext();
}

void decode (FILE *fd) {
    int c;
    int c1;
    while((c = getc(fd)) != EOF) {
        if (c < 32 && c != ESC) {
            if (c == TAB ) {
                addCharToText(c);
                continue;
            } else if (c == LF) {
                addCharToText(c);
                flushtext();
                continue;
            }
            flushtext();
            addCharToText(c);
            switch(c) {
            case ENQ:
                addStrToLabel("C0 Control Character (Ctrl-E) return Terminal Status");
                flushtext();
                break;
            case BEL:
                addStrToLabel("C0 Control Character (Ctrl-G) Bell");
                flushtext();
                break;
            case BS:
                addStrToLabel("C0 Control Character (Ctrl-H) Backspace");
                flushtext();
                break;
            case VT:
                addStrToLabel("C0 Control Character (Ctrl-K) Vertical Tab");
                flushtext();
                break;
            case FF:
                addStrToLabel("C0 Control Character (Ctrl-L) Form Feed or New Page)");
                flushtext();
                break;
            case CR:
                addStrToLabel("C0 Control Character (Ctrl-M) Carriage Return");
                flushtext();
                break;
            case SO:
                addStrToLabel("C0 Control Character (Ctrl-N) Switch to Alternate Character Set (G1)");
                flushtext();
                break;
            case SI:
                addStrToLabel("C0 Control Character (Ctrl-O) Switch to Standard Character Set (G0)");
                flushtext();
                break;
            default:
                snprintf(msg,sizeof(msg),"C0 Control Character (Ctrl-%c)",
                                 c+0x40);
                addStrToLabel(msg);
                flushtext();
            }
        } else if (c > 0x7F) { // 8 bit chars 
            if ( c <= 0x9f && C1flush[c-128] == 1) {
                flushtext();
            }
            addCharToText(c);
            switch(c) {
            case IND:   // Index - 0x84 or ESC D
                addStrToLabel("C1 Index");
                break;
            case NEL:   // Next Line - 0x85 or ESC E
                addStrToLabel("C1 Next Line");
                break;
            case HTS:   // Tab Set - 0x88 or ESC H
                addStrToLabel("C1 Tab Set");
                break;
            case RI:    // Reverse Index - 0x8d or ESC M
                addStrToLabel("C1 Reverse Index");
                break;
            case SS2:   // Single Shift Select of G2 Character Set - 0x8e or ESC N 
                        //    affects next character only
                addStrToLabel("C1 Single Shift Select G2 Character Set (next char only)");
                break;
            case SS3:   // Single Shift Select of G3 Character Set - 0x8f or ESC O
                        //    affects next character only
                addStrToLabel("C1 Single Shift Select G3 Character set(Next char only)");
                break;
            case DCS:   // Device Control String - 0x90 or ESC P
                printtoST(ST, "C1 Device Control String(DCS)",fd);
                break;
            case SPA:   // Start of Guarded Area - 0x96 or ESC V
                addStrToLabel("C1 Start of Guarded Area (SPA)");
                break;
            case EPA:   // End of Guarded Area - 0x97 or ESC W
                addStrToLabel("C1 End of Guarded Area (EPA)");
                flushtext();
                break;
            case SOS:   // Start of String - 0x98 or ESC X
                printtoST(ST,"C1 Start of String (SOS)",fd);
                break;
            case DECID: // Return Terminal ID - 0x9a (AKA SCI) or ESC Z
                addStrToLabel("C1 Return Terminal ID");
                break;
            case CSI:   // Control Sequence Introducer - 0x9b or ESC [
                addStrToLabel("C1 ");
                processCSI(fd);
                break;
            case OSC:    // Operating System Command - 0x9d or ESC ]
                printtoST(BEL,"C1 Operating Systgem Command(OSC)",fd);
                break;
            case PM:     // Privacy Message - 0x9e or ESC ^
                printtoST(ST, "C1 Privacy Message(PM)",fd);
                break;
            // default:
            //     nothing else to do, already added char
            }
            if ( c <= 0x9f && C1flush[c-128] == 1) {
                flushtext();
            }
        } else if (c != ESC) {
            addCharToText(c);
        } else {   // control sequence starting with escape
            flushtext(); // flush anything before the <esc>
            addCharToText(c);
            if ((c = getc(fd)) == EOF) {
                addStrToLabel(" interrupted control string");
                flushtext();
                return;
            } 
            addCharToText(c);
            switch (c) {
            case ' ':
                // ESC SP F 7-bit controls (S7C1T).
                // ESC SP G 8-bit controls (S8C1T).
                // ESC SP L Set ANSI conformance level 1 (dpANS X3.134.1).
                // ESC SP M Set ANSI conformance level 2 (dpANS X3.134.1).
                // ESC SP N Set ANSI conformance level 3 (dpANS X3.134.1).
                if ((c = getc(fd)) == EOF) {
                    addStrToLabel("interrupted control string");
                    flushtext();
                    return;
                } 
                addCharToText(c);
                switch (c) {
                case 'F':
                    addStrToLabel("7-bit controls");
                    break;
                case 'G':
                    addStrToLabel("8-bit controls");
                    break;
                case 'L':
                    addStrToLabel("Set ANSI conformance level 1");
                    break;
                case 'M':
                    addStrToLabel("Set ANSI conformance level 2");
                    break;
                case 'N':
                    addStrToLabel("Set ANSI conformance level 3");
                    break;
                default:
                    addStrToLabel("Invalid '<ESC> ' sequence");
                }
                break;
            case '#':
                // ESC # 3  DEC double-height line, top half (DECDHL)
                // ESC # 4  DEC double-height line, bottom half (DECDHL)
                // ESC # 5  DEC single-width line (DECSWL)
                // ESC # 6  DEC double-width line (DECDWL)
                // ESC # 8  DEC Screen Alignment Test (DECALN)
                if ((c = getc(fd)) == EOF) {
                    addStrToLabel("interrupted control string");
                    flushtext();
                    return;
                } 
                addCharToText(c);
                switch (c) {
                case '3':
                    addStrToLabel("DEC Double-height line, top half");
                    break;
                case '4':
                    addStrToLabel("DEC Double-height line, bottom half");
                    break;
                case '5':
                    addStrToLabel("DEC single-width line");
                    break;
                case '6':
                    addStrToLabel("DEC double-width line");
                    break;
                case '8':
                    addStrToLabel("DEC Screen Alignment Test");
                    break;
                default:
                    addStrToLabel("Invalid '<ESC>#' sequence");
                }
                break;
            case '%':
                // ESC % @  Select default character set, ISO 8859-1 (ISO 2022)
                // ESC % G  Select UTF-8 character set (ISO 2022)
                if ((c = getc(fd)) == EOF) {
                    addStrToLabel("interrupted control string");
                    return;
                } 
                addCharToText(c);
                switch (c) {
                case '@':
                    addStrToLabel("Select default character set");
                    break;
                case 'G':
                    addStrToLabel("Select UTF-8 character set");
                    break;
                default:
                    addStrToLabel("Invalid '<ESC>%' sequence");
                }
                break;
            case '(':
            case ')':
            case '*':
            case '+':
                if ((c1 = getc(fd)) == EOF) {
                    addStrToLabel(" interrupted control string");
                    flushtext();
                    return;
                } 
                addCharToText(c1);
                switch (c1) {
                case '0':
                    strcpy(charsetname, "Dec Special and Line Drawing Set");
                    strcpy(shortcharsetname, "Dec Special");
                    break;
                case 'A':
                    strcpy(charsetname, "United Kingdom (UK)");
                    strcpy(shortcharsetname, "UK");
                    break;
                case 'B':
                    strcpy(charsetname, "United States (USASCII)");
                    strcpy(shortcharsetname, "USASCII");
                    break;
                case '4':
                    strcpy(charsetname, "Dutch");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'C':
                case '5':
                    strcpy(charsetname, "Finnish");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'R':
                    strcpy(charsetname, "French");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'Q':
                    strcpy(charsetname, "French Canadian");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'K':
                    strcpy(charsetname, "German");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'Y':
                    strcpy(charsetname, "Italian");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'E':
                case '6':
                    strcpy(charsetname, "Norwegian/Danish");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'Z':
                    strcpy(charsetname, "Spanish");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case 'H':
                case '7':
                    strcpy(charsetname, "Swedish");
                    strcpy(shortcharsetname, charsetname);
                    break;
                case '=':
                    strcpy(charsetname, "Swiss");
                    strcpy(shortcharsetname, charsetname);
                    break;
                default:
                    strcpy(charsetname, "Invalid Charset Name");
                    strcpy(shortcharsetname, "Invalid");
                }
                switch(c) {
                case '(':
                    snprintf(msg,sizeof(msg),"Designate G0 Character %s",
                                charsetname);
                    break;
                case ')':
                    snprintf(msg,sizeof(msg),"Designate G1 Character %s",
                           charsetname);
                    break;
                case '*':
                    snprintf(msg,sizeof(msg),"Designate G2 Character %s",
                           charsetname);
                    break;
                case '+':
                    snprintf(msg,sizeof(msg),"Designate G3 Character %s",
                           charsetname);
                    break;
                }
                addStrToLabel(msg);
                break;
            case '7':
                addStrToLabel("Save Curser");
                break;
            case '8':
                addStrToLabel("Restor Curser");
                break;
            case '=':
                addStrToLabel("Application Keypad");
                break;
            case '>':
                addStrToLabel("Normal Keypad");
                break;
            case 'F':
                addStrToLabel("Cursor to lower left corner");
                break;
            case 'c':
                addStrToLabel("Full Reset");
                break;
            case 'l':
                addStrToLabel("Lock Memory above cursor");
                break;
            case 'm':
                addStrToLabel("Unlock Memory");
                break;
            case 'n':
                addStrToLabel("Invoke G2 Character Set as GL");
                break;
            case 'o':
                addStrToLabel("Invoke G3 Character Set as GL");
                break;
            case '|':
                addStrToLabel("Invoke G3 Character Set as GR");
                break;
            case '}':
                addStrToLabel("Invoke G2 Character Set as GR");
                break;
            case '~':
                addStrToLabel("Invoke G1 Character Set as GR");
                break;

            // following are C1 (8 bit control chars)
            // xterm recognizes these as 7 bit or 8 bit controls

            case 'D':
                addStrToLabel("Index");
                break;
            case 'E':
                addStrToLabel("Next Line");
                break;
            case 'H':
                addStrToLabel("Tab Set");
                break;
            case 'M':
                addStrToLabel("Reverse Index");
                break;
            case 'N':
                addStrToLabel("Single Shift Select G2 Character Set (next char only)");
                break;
            case 'O':
                addStrToLabel("Single Shift Select G3 Character set(Next char only)");
                break;
            case 'V':
                addStrToLabel("Start of Guarded Area (SPA)");
                break;
            case 'W':
                addStrToLabel("End of Guarded Area (EPA)");
                break;
            case 'X':
                printtoST(ST,"Start of String (SOS)",fd);

                break;
            //ESC Z Return Terminal ID (DECID is 0x9a). 
            //Obsolete form of CSI c (DA).
            case 'Z':
                addStrToLabel("Return Terminal ID");
                break;
            case 'P':
                printtoST(ST, "Device Control String(DCS)",fd);
                break;
    //xxx ESC [ Control Sequence Introducer ( CSI is 0x9b)
            case '[':
                processCSI(fd);
                break;
    //xxx ESC ] Operating System Command ( OSC is 0x9d) 
            case ']':
                printtoST(BEL,"Operating Systgem Command(OSC)",fd);
                break;
            case '^':
                printtoST(ST,"Privacy Message (PM)",fd);
                break;
    //xxx ESC _ Application Program Command ( APC is 0x9f)
    // xterm has no apc functions ignore data to ST (ESC \)
            case '_':
                printtoST(ST, "Application Program Command(APC)",fd);
                break;

            case 0x9f:
                addStrToLabel("Application Program COmmand");
                break;
            default:
                addStrToLabel("Unrecognized  CSI (Control Sequence Introducer");
            }
            flushtext();
        }
    }
}

int main( int argc, char *argv[]) {
    char *p;
    FILE *fd;

    while(argc > 1 && *argv[1] == '-') {
        for(p = &argv[1][1]; *p != '\0'; p++) {
            switch(*p) {
                case 'h':
                    usage(0);
                    break;
                default:
                    fprintf(stderr, "xterm decode: unknown option -%c\n", *p);
                    usage(1);
            }
        }
        argc--;
        argv++;
    }

    if(argc == 1) {
            decode(stdin);
    } else {
        while(--argc > 0) {
            if((fd = fopen(*++argv, "r")) == NULL) {
                fprintf(stderr, "xterm_decode: can't open %s\n", *argv);
                continue;
            }
            decode(fd);
            fclose(fd);
        }
    }
}
