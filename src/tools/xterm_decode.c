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
//
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

#define MAXTEXT 60
char textbuf[MAXTEXT];
int textlen=0;
char charsetname[80] = "";
char shortcharsetname[80] = "";
char msg[150];  // buffer to assemble messages
char msg1[100]; // buffer to assemble messages

// C0 Codes
char *controlchars[32] = 
     { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
       "BS",  "TAB",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
       "DLE", "DCI", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
       "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US" };

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
void flushtext(char *label);
void addCharToText(char *label, char text);
void addStrToText(char *label, char *text);
void printtoST(char *label, FILE *fd);
void readOSC(char* label, FILE *fd);
void processCSI(char *label, FILE *fd);
void parse(int c, char* name, FILE* fd);
void decode(char *name, FILE *fd);

void usage(int e) {
    printf("usage:xterm_decode input... \n");
    printf("      xterm_decode\n");
    printf("      output to stdout, input is list of files, or stdin.\n");
    exit(e);

}

// flushtext - look at character set and decode line drawing characters
void flushtext(char *label) {
    char textname[90];
    if(textlen == 0) return;
    textbuf[textlen] = 0;

    // create string with "text name of font"
    if (strcmp(label, "text") == 0) {
        if (strlen(shortcharsetname) == 0) {
            snprintf(textname,sizeof(textname),"%s default font", label);
        } else {
            snprintf(textname,sizeof(textname),"%s %s", label,shortcharsetname);
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
            printf("%s: '%*s'\n", textname, textlen, textbuf);
        }
    } else {
        printf("%s: '%s'\n", label,textbuf);

    }
    textlen = 0;
}

void addCharToText(char *label, char text) {
    if (textlen >= MAXTEXT) flushtext(label);
    textbuf[textlen++] = text;
}

void addStrToText(HINT_PARAM_UNUSED char *label, char *text) {
    while (*text) {
        if (textlen >= MAXTEXT) flushtext("text");
        textbuf[textlen++] = *text++;
    }
}

//STring terminator is <ESC> \ or <ESC> 0x9c
void printtoST(char *label, FILE *fd) {
    char c;
    while ((c = getc(fd)) != EOF) {
        if (c != ESC) {
            addCharToText(label,c);
            continue;
        } else {
            addStrToText(label, "<ESC>");
            if ((c = getc(fd)) != EOF) { 
                if (c <= 127) {
                    addCharToText("text",c);
                } else {
                    snprintf(msg,sizeof(msg),"<0x%x>",c);
                    addStrToText("text",msg);
                }
                if (c == '\\' ) {
                    flushtext(label);
                } else if (c == 0x9c) {
                    flushtext(label);
                } else {
                    flushtext("Unrecognized string Terminator");
                }
            } else {
                flushtext("interrupted escape sequence");
            }
            break;
        }
    }
}
//readOSC - 2 nums  terminated by ST or BEL
// Read operating system command
void readOSC(char* label, FILE *fd) {
    char c;
    while ((c = getc(fd)) != EOF) {
        if (c != ESC) {
            addCharToText(label,c);
            continue;
        } else {
            if ((c = getc(fd)) != EOF) {
                if (c <= 127) {
                    addCharToText("text",c);
                } else {
                    snprintf(msg,sizeof(msg),"<0x%x>",c);
                    addStrToText("text",msg);
                }
                if (c == '\\' ) {
                    flushtext(label);
                } else if (c == 0x9c) {
                    flushtext(label);
                } else if (c == BEL) {
                    flushtext(label);
                } else {
                    flushtext("Unrecognized escape sequence");
                }
            } else {
                flushtext("interrupted escape sequence");
            }
            break;
        }
    }
}

//processCSI
void processCSI(HINT_PARAM_UNUSED char *label, FILE *fd) {
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
    if ((c = getc(fd)) == EOF) {
    //        some CSI strings have a char before command char (!,",$,')
        flushtext("interrupted CSI string");
        return;
    } 
    addCharToText("CSI",c);
    if (c == '?' || c == '>') {
        lead = c;
        if ((c = getc(fd)) == EOF) {
            addCharToText("CSI",c);
            flushtext("interrupted CSI string");
            return;
        } 
        addCharToText("CSI",c);
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
            addCharToText("CSI",c);
            flushtext("interrupted CSI string");
            return;
        } 
        addCharToText("CSI",c);
    }
    // we have read in numbers -- if any
    if (c == '!' || c == '"' || c =='$' || c == '\'') {
        tail = c;
        if ((c = getc(fd)) == EOF) {
            flushtext("interrupted CSI string");
            return;
        } 
    }
    // CSI Control Sequence Introducer (<ESC>[ or <ESC><0x9b>
    switch (c) {
    case '@':
        // CSI P s @    Insert P s (Blank) Character(s) (default = 1) (ICH)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI insert %d blank%s",buf[0],
                  buf[0] == 1 ? "" : "s");
        flushtext(msg);
        break;
    case 'A':
        // CSI P s A     Cursor Up P s Times (default = 1) (CUU)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor up %d Times",buf[0]);
        flushtext(msg);
        break;
    case 'B':
        // CSI P s B     Cursor Down P s Times (default = 1) (CUD)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor down %d Times",buf[0]);
        flushtext(msg);
        break;
    case 'C':
        // CSI P s C     Cursor Forward P s Times (default = 1) (CUF)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor forward %d Times",buf[0]);
        flushtext(msg);
        break;
    case 'D':
        //    CSI P s D     Cursor Backward P s Times (default = 1) (CUB)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor Backward %d Times",buf[0]);
        flushtext(msg);
        break;
    case 'E':
        // CSI P s E     Cursor Next Line P s Times (default = 1) (CNL)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor Next Line %d Times",buf[0]);
        flushtext(msg);
        break;
    case 'F':
        //    CSI P s F     Cursor Preceding Line P s Times (default = 1) (CPL)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor Preceding Line %d Times",buf[0]);
        flushtext(msg);
        break;
    case 'G':
        // CSI P s G Cursor Character Absolute [column] (default = [row,1]) (CHA)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor to Column %d",buf[0]);
        flushtext(msg);
        break;
    case 'H':
        // CSI P s ; P s H  Cursor Position [row;column] (default = [1,1]) (CUP)
        if (lead != ' ' || tail != ' ' || cnt > 1) {
            flushtext("Invalid CSI string");
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
        snprintf(msg,sizeof(msg), "CSI Position Cursor to row %d,Col %d]",buf[0],buf[1]);
        flushtext(msg);
        break;
    case 'I':
        // CSI P s I  Cursor Forward Tabulation P s tab stops (default = 1) (CHT)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor Forward  %d tab stops",buf[0]);

        flushtext(msg);
        break;
    case 'J':
        // CSI P s J    Erase in Display (ED)
        //    P s = 0 → Erase Below (default) 
        //    P s = 1 → Erase Above 
        //    P s = 2 → Erase All 
        //    P s = 3 → Erase Saved Lines (xterm)
        // CSI ? P s J  Erase in Display (DECSED)       
        //    P s = 0 → Selective Erase Below (default) 
        //    P s = 1 → Selective Erase Above 
        //    P s = 2 → Selective Erase All
        if (tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        {
        char *m = "oops";
        if (lead == '?') {
            switch(buf[0]) {
            case 0:
                m = "Selective Erase Below";
                break;
            case 1:
                m = "Selective Erase Above";
                break;
            case 2:
                m = "Selective Erase All";
                break;
            }
        } else {
            switch(buf[0]) {
            case 0:
                m = "Below";
                break;
            case 1:
                m = "Above";
                break;
            case 2:
                m = "All";
                break;
            case 3:
                m = "Saved Lines";
                break;
            }
        }
        snprintf(msg,sizeof(msg), "CSI Erase Display %s",m);
        flushtext(msg);
        }
        break;
    case 'K':
        // CSI P s K    Erase in Line (EL)      
        //    P s = 0 → Erase to Right (default) 
        //    P s = 1 → Erase to Left 
        //    P s = 2 → Erase All
        // CSI ? P s K  Erase in Line (DECSEL)  
        //    P s = 0 → Selective Erase to Right (default) 
        //    P s = 1 → Selective Erase to Left 
        //    P s = 2 → Selective Erase All
        if (tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        {
        char *m = "oops";
        if (lead == '?') {
            switch(buf[0]) {
            case 0:
                m = "Selective Erase Below";
                break;
            case 1:
                m = "Selective Erase Above";
                break;
            case 2:
                m = "Selective Erase All";
                break;
            }
        } else {
            switch(buf[0]) {
            case 0:
                m = "Below";
                break;
            case 1:
                m = "Above";
                break;
            case 2:
                m = "All";
                break;
            case 3:
                m = "Saved Lines";
                break;
            }
        }
        snprintf(msg,sizeof(msg), "CSI Erase in Line %s",m);
        }
        flushtext(msg);
        break;
    case 'L':
        // CSI P s L Insert P s Line(s) (default = 1) (IL)   
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Insert %d Line(s)",buf[0]);
        flushtext(msg);
        break;
    case 'M':
        // CSI P s M Delete P s Line(s) (default = 1) (DL)   
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Delete %d Line(s)",buf[0]);
        flushtext(msg);
        break;
    case 'P':
        // CSI P s P Delete P s Character(s) (default = 1) (DCH)     
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Delete %d character(s)",buf[0]);
        flushtext(msg);
        break;
    case 'S':
        // CSI P s S Scroll up P s lines (default = 1) (SU)  
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Scroll up %d Line(s)",buf[0]);
        flushtext(msg);
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
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        if (cnt == 0) {
            snprintf(msg,sizeof(msg), "CSI Scroll Down %d line(s)",buf[0]);
        } else {
            snprintf(msg,sizeof(msg), "CSI Initiate Mouse Tracking %d,%d,%d,%d,%d",
                       buf[0],buf[1],buf[2],buf[3],buf[4]);
        }
        flushtext(msg);
        break;
    case 'X':
        // CSI P s X  Erase P s Character(s) (default = 1) (ECH)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Erase %d Character(s)(s)",buf[0]);
        flushtext(msg);
        break;
    case 'Z':
        // CSI P s Z         Cursor Backward Tabulation P s tab stops (default = 1) (CBT)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Cursor Backward Tab  %d tab stops",buf[0]);
        flushtext(msg);
        break;
    case '`':
        // CSI P m `  Character Position Absolute [column] (default = [row,1]) (HPA)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Position column Absolute row unchanged, Col %d",buf[0]);
        flushtext(msg);
        break;
    case 'b':
        // CSI P s b  Repeat the preceding graphic character P s times (REP)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Repeat Previous Graphic char  %d times",buf[0]);
        flushtext(msg);
        break;
    case 'c':
        // CSI > P s c               Send Device Attributes (Primary DA)   
        //    P s = 0 or omitted → request attributes from terminal. 
        //    The response depends on the decTerminalID resource setting. 
        //    → CSI ? 1 ; 2 c (‘‘VT100 with Advanced Video Option’’) 
        //    → CSI ? 1 ; 0 c (‘‘VT101 with No Options’’) 
        //    → CSI ? 6 c (‘‘VT102’’) 
        //    → CSI ? 6 0 ; 1 ; 2 ; 6 ; 8 ; 9 ; 1 5 ; c (‘‘VT220’’) 
        //    The VT100-style response parameters do not mean anything by
        //    themselves. VT220 parameters do, telling the host what 
        //    features the terminal supports: 
        //    → 1 132-columns 
        //    → 2 Printer 
        //    → 6 Selective erase 
        //    → 8 User-defined keys 
        //    → 9 National replacement character sets 
        //    → 1 5 Technical characters 
        //    → 2 2 ANSI color, e.g., VT525 
        //    → 2 9 ANSI text locator (i.e., DEC Locator mode)
        // CSI > P S c  Send Device Attributes (Secondary DA)   
        //    P s = 0 or omitted → request the terminal’s identification code.
        //        The response depends on the decTerminalID resource setting.
        //        It should apply only to VT220 and up, but xterm extends this
        //        to VT100.
        //    → CSI > P p ; P v ; P c c 
        //    where P p denotes the terminal type 
        //    → 0 (‘‘VT100’’) 
        //    → 1 (‘‘VT220’’) 
        //    and P v is the firmware version (for xterm, this is the XFree86
        //        patch number, starting with 95). In a DEC terminal, 
        //        P c indicates the ROM cartridge registration number and is
        //        always zero.
        if (lead == '?' || tail != ' ' || cnt > 0) {
            // want no lead, or > but not ?
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if (lead == ' ') {
            snprintf(msg,sizeof(msg), "CSI Send Device Primary Attributes");
        } else {
            snprintf(msg,sizeof(msg), "CSI Send Device Secondary Attributes");
        }
        flushtext(msg);
        break;
    case 'd':
        // CSI P m d  Line Position Absolute [row] (default = [1,column]) (VPA)
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        snprintf(msg,sizeof(msg), "CSI Line Position Absolutge [row] to %d, Col unchanged",buf[0]);
        flushtext(msg);
        break;
    case 'f':
        // CSI P s ; P s f  Horizontal and Vertical Position [row;column] (default = [1,1]) (HVP)
        if (lead != ' ' || tail != ' ' || cnt > 1) {
            flushtext("Invalid CSI string");
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
        snprintf(msg,sizeof(msg), "CSI Horizontal and Vertical Position row=%d, col=%d",buf[0],buf[1]);
        flushtext(msg);
        break;
    case 'g':
        // CSI P s g  Tab Clear (TBC)
        //    P s = 0 → Clear Current Column (default) 
        //    P s = 3 → Clear All
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 1;
        }
        if (buf[0] == 0) {
            flushtext("CSI Clear Tab CUrrent Column");
        } else if (buf[0] == 3) {
            flushtext("CSI Clear All Tabs");
        } else {
            flushtext("Invalid CSI string");
        }
        break;
    case 'h':
        // CSI P m h         Set Mode (SM)   
        //    P s = 2 → Keyboard Action Mode (AM) 
        //    P s = 4 → Insert Mode (IRM) 
        //    P s = 1 2 → Send/receive (SRM) 
        //    P s = 2 0 → Automatic Newline (LNM)
        // CSI ? P m h  DEC Private Mode Set (DECSET)   
        //    P s = 1 → Application Cursor Keys (DECCKM) 
        //    P s = 2 → Designate USASCII for character sets G0-G3 (DECANM), and set VT100 mode. 
        //    P s = 3 → 132 Column Mode (DECCOLM) 
        //    P s = 4 → Smooth (Slow) Scroll (DECSCLM) 
        //    P s = 5 → Reverse Video (DECSCNM) 
        //    P s = 6 → Origin Mode (DECOM) 
        //    P s = 7 → Wraparound Mode (DECAWM) 
        //    P s = 8 → Auto-repeat Keys (DECARM) 
        //    P s = 9 → Send Mouse X & Y on button press. See the section Mouse Tracking. 
        //    P s = 1 0 → Show toolbar (rxvt) 
        //    P s = 1 2 → Start Blinking Cursor (att610) 
        //    P s = 1 8 → Print form feed (DECPFF) 
        //    P s = 1 9 → Set print extent to full screen (DECPEX) 
        //    P s = 2 5 → Show Cursor (DECTCEM) 
        //    P s = 3 0 → Show scrollbar (rxvt). 
        //    P s = 3 5 → Enable font-shifting functions (rxvt). 
        //    P s = 3 8 → Enter Tektronix Mode (DECTEK) 
        //    P s = 4 0 → Allow 80 → 132 Mode 
        //    P s = 4 1 → more(1) fix (see curses resource) 
        //    P s = 4 2 → Enable Nation Replacement Character sets (DECNRCM) 
        //    P s = 4 4 → Turn On Margin Bell 
        //    P s = 4 5 → Reverse-wraparound Mode 
        //    P s = 4 6 → Start Logging (normally disabled by a compile-time option) 
        //    P s = 4 7 → Use Alternate Screen Buffer (unless disabled by the titeInhibit resource) 
        //    P s = 6 6 → Application keypad (DECNKM) 
        //    P s = 6 7 → Backarrow key sends backspace (DECBKM) 
        //    P s = 1 0 0 0 → Send Mouse X & Y on button press and release. See the section Mouse Tracking. 
        //    P s = 1 0 0 1 → Use Hilite Mouse Tracking. 
        //    P s = 1 0 0 2 → Use Cell Motion Mouse Tracking. 
        //    P s = 1 0 0 3 → Use All Motion Mouse Tracking. 
        //    P s = 1 0 1 0 → Scroll to bottom on tty output (rxvt). 
        //    P s = 1 0 1 1 → Scroll to bottom on key press (rxvt). 
        //    P s = 1 0 3 5 → Enable special modifiers for Alt and NumLock keys. 
        //    P s = 1 0 3 6 → Send ESC when Meta modifies a key (enables the metaSendsEscape resource). 
        //    P s = 1 0 3 7 → Send DEL from the editing-keypad Delete key 
        //    P s = 1 0 4 7 → Use Alternate Screen Buffer (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 8 → Save cursor as in DECSC (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 9 → Save cursor as in DECSC and use Alternate Screen Buffer, clearing it first (unless disabled by the titeInhibit resource). This combines the effects of the 1 0 4 7 and 1 0 4 8 modes. Use this with terminfo-based applications rather than the 4 7 mode. 
        //    P s = 1 0 5 1 → Set Sun function-key mode. 
        //    P s = 1 0 5 2 → Set HP function-key mode. 
        //    P s = 1 0 5 3 → Set SCO function-key mode. 
        //    P s = 1 0 6 0 → Set legacy keyboard emulation (X11R6). 
        //    P s = 1 0 6 1 → Set Sun/PC keyboard emulation of VT220 keyboard. 
        //    P s = 2 0 0 4 → Set bracketed paste mode.
        if (lead == '>' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        snprintf(msg,sizeof(msg),"CSI Set Mode %d invalid",buf[0]);
        if (lead == ' ') {
            switch (buf[0]) {
            case 2:
                strncpy(msg, "CSI Set Keyboard Action Mode",sizeof(msg));
                break;
            case 4:
                strncpy(msg, "CSI Set Insert Mode",sizeof(msg));
                break;
            case 12:
                strncpy(msg, "CSI Set Mode Send/receive",sizeof(msg));
                break;
            case 20:
                strncpy(msg, "CSI Set Mode Automatic Newline",sizeof(msg));
                break;
            }
        } else {
            // CSI ? P m h  DEC Private Mode Set (DECSET)   
            switch (buf[0]) {
            case 1:
                strncpy(msg, "CSI Set Application Cursor Keys",sizeof(msg));
                break;
            case 2:
                strncpy(msg, "CSI Set Designate USASCII for character sets G0-G3",sizeof(msg));
                break;
            case 3:
                strncpy(msg, "CSI Set 132 Column Mode",sizeof(msg));
                break;
            case 4:
                strncpy(msg, "CSI Set Smooth (Slow) Scroll",sizeof(msg));
                break;
            case 5:
                strncpy(msg, "CSI Set Reverse Video",sizeof(msg));
                break;
            case 6:
                strncpy(msg, "CSI Set Origin Mode",sizeof(msg));
                break;
            case 7:
                strncpy(msg, "CSI Set Wraparound Mode",sizeof(msg));
                break;
            case 8:
                strncpy(msg, "CSI Set Auto-repeat Keys",sizeof(msg));
                break;
            case 9:
                strncpy(msg, "CSI Set Send Mouse X & Y on button press",sizeof(msg));
                break;
            case 10:
                strncpy(msg, "CSI Set Show toolbar",sizeof(msg));
                break;
            case 12:
                strncpy(msg, "CSI Set Start Blinking Cursor",sizeof(msg));
                break;
            case 18:
                strncpy(msg, "CSI Set Print form feed",sizeof(msg));
                break;
            case 19:
                strncpy(msg, "CSI Set print extent to full screen",sizeof(msg));
                break;
            case 25:
                strncpy(msg, "CSI Set Show Cursor",sizeof(msg));
                break;
            case 30:
                strncpy(msg, "CSI Set Show scrollbar",sizeof(msg));
                break;
            case 35:
                strncpy(msg, "CSI Set Enable font-shifting functions",sizeof(msg));
                break;
            case 38:
                strncpy(msg, "CSI Set Enter Tektronix Mode",sizeof(msg));
                break;
            case 40:
                strncpy(msg, "CSI Set Allow 80-132 Mode",sizeof(msg));
                break;
            case 41:
                strncpy(msg, "CSI Set more(1) fix (see curses resource",sizeof(msg));
                break;
            case 42:
                strncpy(msg, "CSI Set Enable Nation Replacement Character sets",sizeof(msg));
                break;
            case 44:
                strncpy(msg, "CSI Set Turn On Margin Bell",sizeof(msg));
                break;
            case 45:
                strncpy(msg, "CSI Set Reverse-wraparound Mode",sizeof(msg));
                break;
            case 46:
                strncpy(msg, "CSI Set Start Logging",sizeof(msg));
                break;
            case 47:
                strncpy(msg, "CSI Set Use Alternate Screen Buffer",sizeof(msg));
                break;
            case 66:
                strncpy(msg, "CSI Set Application keypad",sizeof(msg));
                break;
            case 67:
                strncpy(msg, "CSI Set Backarrow key sends backspace",sizeof(msg));
                break;
            case 1000:
                strncpy(msg, "CSI Set Send Mouse X & Y on button press and release",sizeof(msg));
                break;
            case 1001:
                strncpy(msg, "CSI Set Use Hilite Mouse Tracking",sizeof(msg));
                break;
            case 1002:
                strncpy(msg, "CSI Set Use Cell Motion Mouse Tracking",sizeof(msg));
                break;
            case 1003:
                strncpy(msg, "CSI Set Use All Motion Mouse Tracking",sizeof(msg));
                break;
            case 1010:
                strncpy(msg, "CSI Set Scroll to bottom on tty output",sizeof(msg));
                break;
            case 1011:
                strncpy(msg, "CSI Set Scroll to bottom on key press",sizeof(msg));
                break;
            case 1035:
                strncpy(msg, "CSI Set Enable special modifiers for Alt and NumLock keys",sizeof(msg));
                break;
            case 1036:
                strncpy(msg, "CSI Set Send ESC when Meta modifies a key",sizeof(msg));
                break;
            case 1037:
                strncpy(msg, "CSI Set Send DEL from the editing-keypad Delete key",sizeof(msg));
                break;
            case 1047:
                strncpy(msg, "CSI Set Use Alternate Screen Buffer",sizeof(msg));
                break;
            case 1048:
                strncpy(msg, "CSI Set Save cursor",sizeof(msg));
                break;
            case 1049:
                strncpy(msg, "CSI Set Save cursor and use Alternate Screen Buffer",sizeof(msg));
                break;
            case 1051:
                strncpy(msg, "CSI Set Sun function-key mode",sizeof(msg));
                break;
            case 1052:
                strncpy(msg, "CSI Set HP function-key mode",sizeof(msg));
                break;
            case 1053:
                strncpy(msg, "CSI Set SCO function-key mode",sizeof(msg));
                break;
            case 1060:
                strncpy(msg, "CSI Set legacy keyboard emulation",sizeof(msg));
                break;
            case 1061:
                strncpy(msg, "CSI Set Sun/PC keyboard emulation of VT220 keyboard",sizeof(msg));
                break;
            case 2004:
                strncpy(msg, "CSI Set bracketed paste mode",sizeof(msg));
                break;
            }
        }
        flushtext(msg);
        break;
    case 'i':
        // CSI P m i         Media Copy (MC) 
        //    P s = 0 → Print screen (default) 
        //    P s = 4 → Turn off printer controller mode 
        //    P s = 5 → Turn on printer controller mode
        // CSI ? P m i               Media Copy (MC, DEC-specific)   
        //    P s = 1 → Print line containing cursor 
        //    P s = 4 → Turn off autoprint mode 
        //    P s = 5 → Turn on autoprint mode 
        //    P s = 1 0 → Print composed display, ignores DECPEX 
        //    P s = 1 1 → Print all pages
        if (lead == '>' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
       
        if (lead == '?') {
            switch (buf[0]) {
            case 0:
                strncpy(msg,"CSI Media Copy, print Line containing cursor",sizeof(msg));
                break;
            case 4:
                strncpy(msg,"CSI Media Copy, Turn off autoprint mode",sizeof(msg));
                break;
            case 5:
                strncpy(msg,"CSI Media Copy, Turn on autoprint mode",sizeof(msg));
                break;
            case 10:
                strncpy(msg,"CSI Media Copy, Print composed display",sizeof(msg));
                break;
            case 11:
                strncpy(msg,"CSI Media Copy, Print all Pages",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops %d invalid",buf[0]);
            }
        } else {
            switch (buf[0]) {
            case 0:
                strncpy(msg,"CSI Media Copy, print Screen",sizeof(msg));
                break;
            case 4:
                strncpy(msg,"CSI Media Copy, Turn off printer Controller mode",sizeof(msg));
                break;
            case 5:
                strncpy(msg,"CSI Media Copy, Turn on printer Controller mode",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"CSI Media Copy, Oops %d invalid",buf[0]);
            }

        }
        flushtext(msg);
        break;
    case 'l':
        // CSI P m l         Reset Mode (RM) 
        //    P s = 2 → Keyboard Action Mode (AM) 
        //    P s = 4 → Replace Mode (IRM) 
        //    P s = 1 2 → Send/receive (SRM) 
        //    P s = 2 0 → Normal Linefeed (LNM)
        // CSI ? P m l               DEC Private Mode Reset (DECRST) 
        //    P s = 1 → Normal Cursor Keys (DECCKM) 
        //    P s = 2 → Designate VT52 mode (DECANM). 
        //    P s = 3 → 80 Column Mode (DECCOLM) 
        //    P s = 4 → Jump (Fast) Scroll (DECSCLM) 
        //    P s = 5 → Normal Video (DECSCNM) 
        //    P s = 6 → Normal Cursor Mode (DECOM) 
        //    P s = 7 → No Wraparound Mode (DECAWM) 
        //    P s = 8 → No Auto-repeat Keys (DECARM) 
        //    P s = 9 → Don’t Send Mouse X & Y on button press 
        //    P s = 1 0 → Hide toolbar (rxvt) 
        //    P s = 1 2 → Stop Blinking Cursor (att610) 
        //    P s = 1 8 → Don’t print form feed (DECPFF) 
        //    P s = 1 9 → Limit print to scrolling region (DECPEX) 
        //    P s = 2 5 → Hide Cursor (DECTCEM) 
        //    P s = 3 0 → Don’t show scrollbar (rxvt). 
        //    P s = 3 5 → Disable font-shifting functions (rxvt). 
        //    P s = 4 0 → Disallow 80 → 132 Mode 
        //    P s = 4 1 → No more(1) fix (see curses resource) 
        //    P s = 4 2 → Disable Nation Replacement Character sets (DECNRCM) 
        //    P s = 4 4 → Turn Off Margin Bell 
        //    P s = 4 5 → No Reverse-wraparound Mode 
        //    P s = 4 6 → Stop Logging (normally disabled by a compile-time option) 
        //    P s = 4 7 → Use Normal Screen Buffer 
        //    P s = 6 6 → Numeric keypad (DECNKM) 
        //    P s = 6 7 → Backarrow key sends delete (DECBKM) 
        //    P s = 1 0 0 0 → Don’t Send Mouse X & Y on button press and release. See the section Mouse Tracking. 
        //    P s = 1 0 0 1 → Don’t Use Hilite Mouse Tracking 
        //    P s = 1 0 0 2 → Don’t Use Cell Motion Mouse Tracking 
        //    P s = 1 0 0 3 → Don’t Use All Motion Mouse Tracking 
        //    P s = 1 0 1 0 → Don’t scroll to bottom on tty output (rxvt). 
        //    P s = 1 0 1 1 → Don’t scroll to bottom on key press (rxvt). 
        //    P s = 1 0 3 5 → Disable special modifiers for Alt and NumLock keys. 
        //    P s = 1 0 3 6 → Don’t send ESC when Meta modifies a key (disables the metaSendsEscape resource). 
        //    P s = 1 0 3 7 → Send VT220 Remove from the editing-keypad Delete key 
        //    P s = 1 0 4 7 → Use Normal Screen Buffer, clearing screen first if in the Alternate Screen (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 8 → Restore cursor as in DECRC (unless disabled by the titeInhibit resource) 
        //    P s = 1 0 4 9 → Use Normal Screen Buffer and restore cursor as in DECRC (unless disabled by the titeInhibit resource). This combines the effects of the 1 0 4 7 and 1 0 4 8 modes. Use this with terminfo-based applications rather than the 4 7 mode. 
        //    P s = 1 0 5 1 → Reset Sun function-key mode. 
        //    P s = 1 0 5 2 → Reset HP function-key mode. 
        //    P s = 1 0 5 3 → Reset SCO function-key mode. 
        //    P s = 1 0 6 0 → Reset legacy keyboard emulation (X11R6). 
        //    P s = 1 0 6 1 → Reset Sun/PC keyboard emulation of VT220 keyboard. 
        //    P s = 2 0 0 4 → Reset bracketed paste mode.
        if (lead == '?' || tail == ' ' || cnt == 0) {
            // CSI Pm l
            switch (buf[0]) {
            case 1:
                strncpy(msg,"CSI Dec Private Mode Reset Normal Cursor Keys",sizeof(msg));
                break;
            case 2:
                strncpy(msg,"CSI Dec Private Mode Reset Designate Vt52 Mode",sizeof(msg));
                break;
            case 3:
                strncpy(msg,"CSI Dec Private Mode Reset 30 Column Mode",sizeof(msg));
                break;
            case 4:
                strncpy(msg,"CSI Dec Private Mode Reset Jump (fast) Scroll",sizeof(msg));
                break;
            case 5:
                strncpy(msg,"CSI Dec Private Mode Reset Normal Video",sizeof(msg));
                break;
            case 6:
                strncpy(msg,"CSI Dec Private Mode Reset Normal Cursor Mode",sizeof(msg));
                break;
            case 7:
                strncpy(msg,"CSI Dec Private Mode Reset No Wraparound Mode",sizeof(msg));
                break;
            case 8:
                strncpy(msg,"CSI Dec Private Mode Reset No Auto-repeat keys",sizeof(msg));
                break;
            case 9:
                strncpy(msg,"CSI Dec Private Mode Reset Don't send Mouse X,Y on button press",sizeof(msg));
                break;
            case 10:
                strncpy(msg,"CSI Dec Private Mode Reset Hide toolbar",sizeof(msg));
                break;
            case 12:
                strncpy(msg,"CSI Dec Private Mode Reset Stop Blinking Cursor",sizeof(msg));
                break;
            case 18:
                strncpy(msg,"CSI Dec Private Mode Reset Don't print form Feed",sizeof(msg));
                break;
            case 19:
                strncpy(msg,"CSI Dec Private Mode Reset Limit print to scrolling region",sizeof(msg));
                break;
            case 25:
                strncpy(msg,"CSI Dec Private Mode Reset Hide Cursor",sizeof(msg));
                break;
            case 30:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Show Scrollbar",sizeof(msg));
                break;
            case 35:
                strncpy(msg,"CSI Dec Private Mode Reset Disable font shifting functions",sizeof(msg));
                break;
            case 40:
                strncpy(msg,"CSI Dec Private Mode Reset Disallow 80 -> 132 Mode Reset",sizeof(msg));
                break;
            case 41:
                strncpy(msg,"CSI Dec Private Mode Reset No more(1) fix",sizeof(msg));
                break;
            case 42:
                strncpy(msg,"CSI Dec Private Mode Reset Disable Nation replacement Character sets",sizeof(msg));
                break;
            case 44:
                strncpy(msg,"CSI Dec Private Mode Reset Turn Off Margin Bell",sizeof(msg));
                break;
            case 45:
                strncpy(msg,"CSI Dec Private Mode Reset No Reverse wraparound mode",sizeof(msg));
                break;
            case 46:
                strncpy(msg,"CSI Dec Private Mode Reset Stop Logging",sizeof(msg));
                break;
            case 47:
                strncpy(msg,"CSI Dec Private Mode Reset Use Normal Screen Buffer",sizeof(msg));
                break;
            case 66:
                strncpy(msg,"CSI Dec Private Mode Reset Numeric Keypad",sizeof(msg));
                break;
            case 67:
                strncpy(msg,"CSI Dec Private Mode Reset Backarrow key sends delete",sizeof(msg));
                break;
            case 1000:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Send Mouse x,y on button press and release",sizeof(msg));
                break;
            case 1001:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Use Hilite Mouse Tracking",sizeof(msg));
                break;
            case 1002:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Use Cell Motion Mouse tracking",sizeof(msg));
                break;
            case 1003:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Use All Motion Mouse Tracking",sizeof(msg));
                break;
            case 1010:
                strncpy(msg,"CSI Dec Private Mode Reset Don't scroll to bottom on tty output",sizeof(msg));
                break;
            case 1011:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Scroll to Bottom on key Press",sizeof(msg));
                break;
            case 1035:
                strncpy(msg,"CSI Dec Private Mode Reset Disable special modifiers for Alt and Numlock keys",sizeof(msg));
                break;
            case 1036:
                strncpy(msg,"CSI Dec Private Mode Reset Don't Send SEC when Meta Modifies a key",sizeof(msg));
                break;
            case 1037:
                strncpy(msg,"CSI Dec Private Mode Reset Send VT220 Remove from the editing Keypad Delete Key",sizeof(msg));
                break;
            case 1047:
                strncpy(msg,"CSI Use Normal Screen Buffer clearing first",sizeof(msg));
                break;
            case 1048:
                strncpy(msg,"CSI Restore Cursor as in DECRC",sizeof(msg));
                break;
            case 1049:
                strncpy(msg,"CSI Use Normal Screen Buffer and restore cursor",sizeof(msg));
                break;
            case 1051:
                strncpy(msg,"CSI Reset Sun Function key mode",sizeof(msg));
                break;
            case 1052:
                strncpy(msg,"CSI Reset HP function key mode",sizeof(msg));
                break;
            case 1053:
                strncpy(msg,"CSI Reset SCO Function key mode",sizeof(msg));
                break;
            case 1060:
                strncpy(msg,"CSI Reset Legacy Keyboard emulation",sizeof(msg));
                break;
            case 1061:
                strncpy(msg,"CSI Reset SunPC keyboard emulatsion of VT 220 keyboard",sizeof(msg));
                break;
            case 2004:
                strncpy(msg,"CSI Reset Bracketed paste mode",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops Dec Mode Reset %d invalid",buf[0]);
            }
        } else if (lead == ' ' || tail == ' ' || cnt == 0) {
            switch (buf[0]) {
            case 2:
                strncpy(msg,"CSI Reset Keyboard Action mode",sizeof(msg));
                break;
            case 4:
                strncpy(msg,"CSI Reset Replace Mode",sizeof(msg));
                break;
            case 12:
                strncpy(msg,"CSI Reset Send/receive",sizeof(msg));
                break;
            case 20:
                strncpy(msg,"CSI Reset Normal Linefeed",sizeof(msg));
            default:
                snprintf(msg,sizeof(msg),"Reset Mode, Oops %d invalid",buf[0]);
            }
        } else {
                snprintf(msg,sizeof(msg),"Reset Mode, Oops %d invalid",buf[0]);
        }
        flushtext(msg);
        break;
    case 'm':
        // CSI P m m         Character Attributes (SGR)      
        //    P s = 0 → Normal (default) 
        //    P s = 1 → Bold 
        //    P s = 4 → Underlined 
        //    P s = 5 → Blink (appears as Bold) 
        //    P s = 7 → Inverse 
        //    P s = 8 → Invisible, i.e., hidden (VT300) 
        //    P s = 2 2 → Normal (neither bold nor faint) 
        //    P s = 2 4 → Not underlined 
        //    P s = 2 5 → Steady (not blinking) 
        //    P s = 2 7 → Positive (not inverse) 
        //    P s = 2 8 → Visible, i.e., not hidden (VT300) 
        //    P s = 3 0 → Set foreground color to Black 
        //    P s = 3 1 → Set foreground color to Red 
        //    P s = 3 2 → Set foreground color to Green 
        //    P s = 3 3 → Set foreground color to Yellow 
        //    P s = 3 4 → Set foreground color to Blue 
        //    P s = 3 5 → Set foreground color to Magenta 
        //    P s = 3 6 → Set foreground color to Cyan 
        //    P s = 3 7 → Set foreground color to White 
        //    P s = 3 9 → Set foreground color to default (original) 
        //    P s = 4 0 → Set background color to Black 
        //    P s = 4 1 → Set background color to Red 
        //    P s = 4 2 → Set background color to Green 
        //    P s = 4 3 → Set background color to Yellow 
        //    P s = 4 4 → Set background color to Blue 
        //    P s = 4 5 → Set background color to Magenta 
        //    P s = 4 6 → Set background color to Cyan 
        //    P s = 4 7 → Set background color to White 
        //    P s = 4 9 → Set background color to default (original). 
        //          If 16-color support is compiled, the following apply.
        //          Assume that xterm’s resources are set so that the ISO
        //          color codes are the first 8 of a set of 16. Then the
        //          aixterm colors are the bright versions of the ISO colors: 
        //    P s = 9 0 → Set foreground color to Black 
        //    P s = 9 1 → Set foreground color to Red 
        //    P s = 9 2 → Set foreground color to Green 
        //    P s = 9 3 → Set foreground color to Yellow 
        //    P s = 9 4 → Set foreground color to Blue 
        //    P s = 9 5 → Set foreground color to Magenta 
        //    P s = 9 6 → Set foreground color to Cyan 
        //    P s = 9 7 → Set foreground color to White 
        //    P s = 1 0 0 → Set background color to Black 
        //    P s = 1 0 1 → Set background color to Red 
        //    P s = 1 0 2 → Set background color to Green 
        //    P s = 1 0 3 → Set background color to Yellow 
        //    P s = 1 0 4 → Set background color to Blue 
        //    P s = 1 0 5 → Set background color to Magenta 
        //    P s = 1 0 6 → Set background color to Cyan 
        //    P s = 1 0 7 → Set background color to White If xterm is 
        //                 compiled with the 16-color support disabled, 
        //                 it supports the following, from rxvt: 
        //    P s = 1 0 0 → Set foreground and background color to default
        //                  If 88- or 256-color support is compiled,
        //                  the following apply. 
        //    P s = 3 8 ; 5 ; P s → Set foreground color to the second P s 
        //    P s = 4 8 ; 5 ; P s → Set background color to the second P s
        if (lead != ' ' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        switch (buf[0]) {
        case 0:
            strncpy(msg,"CSI Character Attributes-Normal",sizeof(msg));
            break;
        case 1:
            strncpy(msg,"CSI Character Attributes-Bold",sizeof(msg));
            break;
        case 4:
            strncpy(msg,"CSI Character Attributes-Underlined",sizeof(msg));
            break;
        case 5:
            strncpy(msg,"CSI Character Attributes-Blink (appears as bold)",sizeof(msg));
            break;
        case 7:
            strncpy(msg,"CSI Character Attributes-Inverse",sizeof(msg));
            break;
        case 8:
            strncpy(msg,"CSI Character Attributes-Invisible",sizeof(msg));
            break;
        case 22:
            strncpy(msg,"CSI Character Attributes-Normal(Neither bold nor faint)",sizeof(msg));
            break;
        case 24:
            strncpy(msg,"CSI Character Attributes-Not Underlined",sizeof(msg));
            break;
        case 25:
            strncpy(msg,"CSI Character Attributes-Steady (not blinking)",sizeof(msg));
            break;
        case 27:
            strncpy(msg,"CSI Character Attributes-Positive (not inverse)",sizeof(msg));
            break;
        case 28:
            strncpy(msg,"CSI Character Attributes-Visible (not hidden)",sizeof(msg));
            break;
        case 30:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Black",sizeof(msg));
            break;
        case 31:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Red",sizeof(msg));
            break;
        case 32:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Green",sizeof(msg));
            break;
        case 33:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Yellow",sizeof(msg));
            break;
        case 34:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Blue",sizeof(msg));
            break;
        case 35:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Magenta",sizeof(msg));
            break;
        case 36:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to Cyan",sizeof(msg));
            break;
        case 37:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to White",sizeof(msg));
            break;
        case 39:
            strncpy(msg,"CSI Character Attributes-Set Forground Color to default (original)",sizeof(msg));
            break;
        case 40:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Black",sizeof(msg));
            break;
        case 41:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Red",sizeof(msg));
            break;
        case 42:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Green",sizeof(msg));
            break;
        case 43:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Yellow",sizeof(msg));
            break;
        case 44:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Blue",sizeof(msg));
            break;
        case 45:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Magenta",sizeof(msg));
            break;
        case 46:
            strncpy(msg,"CSI Character Attributes-Set Background Color to Cyan",sizeof(msg));
            break;
        case 47:
            strncpy(msg,"CSI Character Attributes-Set Background Color to White",sizeof(msg));
            break;
        case 49:
            strncpy(msg,"CSI Character Attributes-Set Background Color to default (original)",sizeof(msg));
            break;
        case 90:
            strncpy(msg,"CSI Character Attributes-Set forground color to Black",sizeof(msg));
            break;
        case 91:
            strncpy(msg,"CSI Character Attributes-Set forground color to Red",sizeof(msg));
            break;
        case 92:
            strncpy(msg,"CSI Character Attributes-Set forground color to Green",sizeof(msg));
            break;
        case 93:
            strncpy(msg,"CSI Character Attributes-Set forground color to Yellow",sizeof(msg));
            break;
        case 94:
            strncpy(msg,"CSI Character Attributes-Set forground color to Blue",sizeof(msg));
            break;
        case 95:
            strncpy(msg,"CSI Character Attributes-Set forground color to Magenta",sizeof(msg));
            break;
        case 96:
            strncpy(msg,"CSI Character Attributes-Set forground color to Cyan",sizeof(msg));
            break;
        case 97:
            strncpy(msg,"CSI Character Attributes-Set forground color to White",sizeof(msg));
            break;
        case 100:
            strncpy(msg,"CSI Character Attributes-Set background color to Black",sizeof(msg));
            break;
        case 101:
            strncpy(msg,"CSI Character Attributes-Set background color to Red",sizeof(msg));
            break;
        case 102:
            strncpy(msg,"CSI Character Attributes-Set background color to Green",sizeof(msg));
            break;
        case 103:
            strncpy(msg,"CSI Character Attributes-Set background color to Yellow",sizeof(msg));
            break;
        case 104:
            strncpy(msg,"CSI Character Attributes-Set background color to Blue",sizeof(msg));
            break;
        case 105:
            strncpy(msg,"CSI Character Attributes-Set background color to Magenta",sizeof(msg));
            break;
        case 106:
            strncpy(msg,"CSI Character Attributes-Set background color to Cyan",sizeof(msg));
            break;
        case 107:
            strncpy(msg,"CSI Character Attributes-Set background color to White",sizeof(msg));
            break;
        default:
            snprintf(msg,sizeof(msg),"CSI Character Attributes, Oops %d invalid",buf[0]);
        }
        flushtext(msg);
        break;
    case 'n':
        // CSI P s n         Device Status Report (DSR)      
        //    P s = 5 → Status Report CSI 0 n (‘‘OK’’) 
        //    P s = 6 → Report Cursor Position (CPR) [row;column] as
        // CSI ? P s n          Device Status Report (DSR, DEC-specific)
        //    P s = 6 → Report Cursor Position (CPR) [row;column] as 
        //              CSI ? r ; c R (assumes page is zero). 
        //    P s = 1 5 → Report Printer status as CSI ? 1 0 n (ready) or 
        //                 CSI ? 1 1 n (not ready) 
        //    P s = 2 5 → Report UDK status as CSI ? 2 0 n (unlocked) or
        //                CSI ? 2 1 n (locked) 
        //    P s = 2 6 → Report Keyboard status as 
        //                CSI ? 2 7 ; 1 ; 0 ; 0 n (North American) 
        //    The last two parameters apply to VT400 & up, and denote keyboard 
        //                ready and LK01 respectively. 
        //    P s = 5 3 → Report Locator status as 
        //                CSI ? 5 3 n Locator available, if compiled-in, or 
        //                CSI ? 5 0 n No Locator, if not.
        if (lead == '>' || tail != ' ' || cnt > 0) {
            flushtext("Invalid CSI string");
            return;
        }
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
       
        if (lead == '?') {
            switch (buf[0]) {
            case 6:
                strncpy(msg,"CSI Report Cursor Position",sizeof(msg));
                break;
            case 15:
                strncpy(msg,"CSI Report Printer status",sizeof (msg));
                break;
            case 25:
                strncpy(msg,"CSI Report UDK status",sizeof(msg));
                break;
            case 26:
                strncpy(msg,"CSI Report Keyboard status",sizeof(msg));
                break;
            case 53:
                strncpy(msg,"CSI Report Locator status",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"CSI Device Status Report Oops Report Status %d invalid",buf[0]);
            }
        } else {
            switch (buf[0]) {
            case 5:
                strncpy(msg,"CSI Report Device Status Ok",sizeof(msg));
                break;
            case 6:
                strncpy(msg,"CSI Report cursor Position",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"CSI Report Device Status %d invalid",buf[0]);
            }

        }
        flushtext(msg);
        break;
    case 'p':
        // CSI ! p      Soft terminal reset (DECSTR)
        // CSI P s ; P s “ p  Set conformance level (DECSCL) 
        //               Valid values for the first parameter:
        // P s = 6 1 → VT100 
        // P s = 6 2 → VT200 
        // P s = 6 3 → VT300 
        // Valid values for the second parameter: 
        // P s = 0 → 8-bit controls 
        // P s = 1 → 7-bit controls (always set for VT100) 
        // P s = 2 → 8-bit controls
        if (lead != ' ' && tail == '!' && cnt == -1) {
             flushtext("CSI Soft Terminal reset");
        } else if ( lead == ' ' && tail == '"' && cnt == 1) {
            switch (buf[0]) {
            case 0:
                strncpy(msg,"CSI Set conformance level to VT100",sizeof(msg));
                break;
            case 1:
                strncpy(msg,"CSI Set conformance level to VT200",sizeof(msg));
                break;
            case 2:
                strncpy(msg,"CSI Set conformance level to VT300",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"CSI Set Conformance level %d invalid",buf[0]);
            }
            flushtext(msg);
        } else {
            flushtext("Invalid CSI string");
            return;
        }
        break;
    case 'q': 
        // CSI P s “ q  Select character protection attribute (DECSCA). 
        //              Valid values for the parameter:
        //    P s = 0 → DECSED and DECSEL can erase (default) 
        //    P s = 1 → DECSED and DECSEL cannot erase 
        //    P s = 2 → DECSED and DECSEL can erase

        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if (lead != ' ' && tail == '"' && cnt == 0) {
            switch (buf[0]) {
            case 0:
                strncpy(msg,"CSI Select character proctection attribute can erase",sizeof(msg));
                break;
            case 1:
                strncpy(msg,"CSI Select character proctection attribute cannot erase",sizeof(msg));
                break;
            case 2:
                strncpy(msg,"CSI Select character proctection attribute can erase",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"CSI Select character protection attribute %d invalid",buf[0]);
            }
            flushtext(msg);
        } else {
            flushtext("Invalid CSI string");
            return;
        }
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
            strncpy(msg,"CSI Set Scrolling Region full size of window",sizeof(msg));
        } else if (lead != ' ' && tail == ' ' && cnt == 2) {
                snprintf(msg,sizeof(msg),"CSI Set Scrolling Region [%d,%d]",
                                          buf[0],buf[1]);
        } else if (lead != ' ' && tail == ' ' ) {
            strncpy(msg,"CSI Set Scrolling Region invalid arguments",sizeof(msg));
        } else if (lead != '?' && tail == ' ' && cnt == 1) {
            switch (buf[0]) {
            case 1:
                strncpy(msg,"CSI Dec Private Mode Restore Normal Cursor Keys",sizeof(msg));
                break;
            case 2:
                strncpy(msg,"CSI Dec Private Mode Restore Designate Vt52 Mode",sizeof(msg));
                break;
            case 3:
                strncpy(msg,"CSI Dec Private Mode Restore 30 Column Mode",sizeof(msg));
                break;
            case 4:
                strncpy(msg,"CSI Dec Private Mode Restore Jump (fast) Scroll",sizeof(msg));
                break;
            case 5:
                strncpy(msg,"CSI Dec Private Mode Restore Normal Video",sizeof(msg));
                break;
            case 6:
                strncpy(msg,"CSI Dec Private Mode Restore Normal Cursor Mode",sizeof(msg));
                break;
            case 7:
                strncpy(msg,"CSI Dec Private Mode Restore No Wraparound Mode",sizeof(msg));
                break;
            case 8:
                strncpy(msg,"CSI Dec Private Mode Restore No Auto-repeat keys",sizeof(msg));
                break;
            case 9:
                strncpy(msg,"CSI Dec Private Mode Restore Don't send Mouse X,Y on button press",sizeof(msg));
                break;
            case 10:
                strncpy(msg,"CSI Dec Private Mode Restore Hide toolbar",sizeof(msg));
                break;
            case 12:
                strncpy(msg,"CSI Dec Private Mode Restore Stop Blinking Cursor",sizeof(msg));
                break;
            case 18:
                strncpy(msg,"CSI Dec Private Mode Restore Don't print form Feed",sizeof(msg));
                break;
            case 19:
                strncpy(msg,"CSI Dec Private Mode Restore Limit print to scrolling region",sizeof(msg));
                break;
            case 25:
                strncpy(msg,"CSI Dec Private Mode Restore Hide Cursor",sizeof(msg));
                break;
            case 30:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Show Scrollbar",sizeof(msg));
                break;
            case 35:
                strncpy(msg,"CSI Dec Private Mode Restore Disable font shifting functions",sizeof(msg));
                break;
            case 40:
                strncpy(msg,"CSI Dec Private Mode Restore Disallow 80 -> 132 Mode",sizeof(msg));
                break;
            case 41:
                strncpy(msg,"CSI Dec Private Mode Restore No more(1) fix",sizeof(msg));
                break;
            case 42:
                strncpy(msg,"CSI Dec Private Mode Restore Disable Nation replacement Character sets",sizeof(msg));
                break;
            case 44:
                strncpy(msg,"CSI Dec Private Mode Restore Turn Off Margin Bell",sizeof(msg));
                break;
            case 45:
                strncpy(msg,"CSI Dec Private Mode Restore No Reverse wraparound mode",sizeof(msg));
                break;
            case 46:
                strncpy(msg,"CSI Dec Private Mode Restore Stop Logging",sizeof(msg));
                break;
            case 47:
                strncpy(msg,"CSI Dec Private Mode Restore Use Normal Screen Buffer",sizeof(msg));
                break;
            case 66:
                strncpy(msg,"CSI Dec Private Mode Restore Numeric Keypad",sizeof(msg));
                break;
            case 67:
                strncpy(msg,"CSI Dec Private Mode Restore Backarrow key sends delete",sizeof(msg));
                break;
            case 1000:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Send Mouse x,y on button press and release",sizeof(msg));
                break;
            case 1001:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Use Hilite Mouse Tracking",sizeof(msg));
                break;
            case 1002:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Use Cell Motion Mouse tracking",sizeof(msg));
                break;
            case 1003:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Use All Motion Mouse Tracking",sizeof(msg));
                break;
            case 1010:
                strncpy(msg,"CSI Dec Private Mode Restore Don't scroll to bottom on tty output",sizeof(msg));
                break;
            case 1011:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Scroll to Bottom on key Press",sizeof(msg));
                break;
            case 1035:
                strncpy(msg,"CSI Dec Private Mode Restore Disable special modifiers for Alt and Numlock keys",sizeof(msg));
                break;
            case 1036:
                strncpy(msg,"CSI Dec Private Mode Restore Don't Send SEC when Meta Modifies a key",sizeof(msg));
                break;
            case 1037:
                strncpy(msg,"CSI Dec Private Mode Restore Send VT220 Remove from the editing Keypad Delete Key",sizeof(msg));
                break;
            case 1047:
                strncpy(msg,"CSI Restore Use Normal Screen Buffer clearing first",sizeof(msg));
                break;
            case 1048:
                strncpy(msg,"CSI Restore Cursor as in DECRC",sizeof(msg));
                break;
            case 1049:
                strncpy(msg,"CSI Restore Use Normal Screen Buffer and restore cursor",sizeof(msg));
                break;
            case 1051:
                strncpy(msg,"CSI Restore Sun Function key mode",sizeof(msg));
                break;
            case 1052:
                strncpy(msg,"CSI Restore HP function key mode",sizeof(msg));
                break;
            case 1053:
                strncpy(msg,"CSI Restore SCO Function key mode",sizeof(msg));
                break;
            case 1060:
                strncpy(msg,"CSI Restore Legacy Keyboard emulation",sizeof(msg));
                break;
            case 1061:
                strncpy(msg,"CSI Restore SunPC keyboard emulatsion of VT 220 keyboard",sizeof(msg));
                break;
            case 2004:
                strncpy(msg,"CSI Restore Bracketed paste mode",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops Dec Mode Restore %d invalid",buf[0]);
            }
            flushtext(msg);
        } else if (lead != ' ' && tail == '$' && cnt > 3) {
            // CSI P t ; P l ; P b ; P r ; P s $ r
            // 5 or more args
            {
            snprintf(msg1,sizeof(msg1),
                     "CSI Change attributes in rectangle t %d,l %d,b %d,r %d",
                     buf[0],buf[1],buf[2],buf[3]);
            
            int i=4;
            while (i < cnt) {
                switch (buf[0]) {
                case 0:
                    snprintf(msg,sizeof(msg),"%s %s",msg1,"All Attributes");
                    strncpy(msg1,msg,sizeof(msg1));
                    break;
                case 1:
                    snprintf(msg,sizeof(msg),"%s %s",msg1,"Bold");
                    strncpy(msg1,msg,sizeof(msg1));
                    break;
                case 4:
                    snprintf(msg,sizeof(msg),"%s %s",msg1,"Underline");
                    strncpy(msg1,msg,sizeof(msg1));
                    break;
                case 5:
                    snprintf(msg,sizeof(msg),"%s %s",msg1,"Blinking");
                    strncpy(msg1,msg,sizeof(msg1));
                    break;
                case 7:
                    snprintf(msg,sizeof(msg),"%s %s",msg1,"Negative Image");
                    strncpy(msg1,msg,sizeof(msg1));
                    break;
                default:
                    snprintf(msg,sizeof(msg),"%s %s",msg1,"invalid");
                    strncpy(msg1,msg,sizeof(msg1));
                }
                i++;
            }
            }
            flushtext(msg1);
        } else {
            snprintf(msg,sizeof(msg),"%s %s",msg1,"invalid");
            strncpy(msg1,msg,sizeof(msg1));
            flushtext(msg1);
        }
        break;
    case 's':
        // CSI s     Save cursor (ANSI.SYS)
        // CSI ? P m s       Save DEC Private Mode Values. P s values are 
        //                   the same as for DECSET.
        if (lead != ' ' && tail == ' ' && cnt == -1) {
            // CSI s     Save cursor (ANSI.SYS)
            strncpy(msg,"CSI Save Cursor",sizeof(msg));
        } else if (lead != '?' && tail == ' ' && cnt == 0) {
             // CSI ? P m s       Save DEC Private Mode Values. P s values are 
             //                   the same as for DECSET.
            switch (buf[0]) {
            case 1:
                strncpy(msg,"CSI Dec Private Mode Save Normal Cursor Keys",sizeof(msg));
                break;
            case 2:
                strncpy(msg,"CSI Dec Private Mode Save Designate Vt52 Mode",sizeof(msg));
                break;
            case 3:
                strncpy(msg,"CSI Dec Private Mode Save 30 Column Mode",sizeof(msg));
                break;
            case 4:
                strncpy(msg,"CSI Dec Private Mode Save Jump (fast) Scroll",sizeof(msg));
                break;
            case 5:
                strncpy(msg,"CSI Dec Private Mode Save Normal Video",sizeof(msg));
                break;
            case 6:
                strncpy(msg,"CSI Dec Private Mode Save Normal Cursor Mode",sizeof(msg));
                break;
            case 7:
                strncpy(msg,"CSI Dec Private Mode Save No Wraparound Mode",sizeof(msg));
                break;
            case 8:
                strncpy(msg,"CSI Dec Private Mode Save No Auto-repeat keys",sizeof(msg));
                break;
            case 9:
                strncpy(msg,"CSI Dec Private Mode Save Don't send Mouse X,Y on button press",sizeof(msg));
                break;
            case 10:
                strncpy(msg,"CSI Dec Private Mode Save Hide toolbar",sizeof(msg));
                break;
            case 12:
                strncpy(msg,"CSI Dec Private Mode Save Stop Blinking Cursor",sizeof(msg));
                break;
            case 18:
                strncpy(msg,"CSI Dec Private Mode Save Don't print form Feed",sizeof(msg));
                break;
            case 19:
                strncpy(msg,"CSI Dec Private Mode Save Limit print to scrolling region",sizeof(msg));
                break;
            case 25:
                strncpy(msg,"CSI Dec Private Mode Save Hide Cursor",sizeof(msg));
                break;
            case 30:
                strncpy(msg,"CSI Dec Private Mode Save Don't Show Scrollbar",sizeof(msg));
                break;
            case 35:
                strncpy(msg,"CSI Dec Private Mode Save Disable font shifting functions",sizeof(msg));
                break;
            case 40:
                strncpy(msg,"CSI Dec Private Mode Save Disallow 80 -> 132 Mode",sizeof(msg));
                break;
            case 41:
                strncpy(msg,"CSI Dec Private Mode Save No more(1) fix",sizeof(msg));
                break;
            case 42:
                strncpy(msg,"CSI Dec Private Mode Save Disable Nation replacement Character sets",sizeof(msg));
                break;
            case 44:
                strncpy(msg,"CSI Dec Private Mode Save Turn Off Margin Bell",sizeof(msg));
                break;
            case 45:
                strncpy(msg,"CSI Dec Private Mode Save No Reverse wraparound mode",sizeof(msg));
                break;
            case 46:
                strncpy(msg,"CSI Dec Private Mode Save Stop Logging",sizeof(msg));
                break;
            case 47:
                strncpy(msg,"CSI Dec Private Mode Save Use Normal Screen Buffer",sizeof(msg));
                break;
            case 66:
                strncpy(msg,"CSI Dec Private Mode Save Numeric Keypad",sizeof(msg));
                break;
            case 67:
                strncpy(msg,"CSI Dec Private Mode Save Backarrow key sends delete",sizeof(msg));
                break;
            case 1000:
                strncpy(msg,"CSI Dec Private Mode Save Don't Send Mouse x,y on button press and release",sizeof(msg));
                break;
            case 1001:
                strncpy(msg,"CSI Dec Private Mode Save Don't Use Hilite Mouse Tracking",sizeof(msg));
                break;
            case 1002:
                strncpy(msg,"CSI Dec Private Mode Save Don't Use Cell Motion Mouse tracking",sizeof(msg));
                break;
            case 1003:
                strncpy(msg,"CSI Dec Private Mode Save Don't Use All Motion Mouse Tracking",sizeof(msg));
                break;
            case 1010:
                strncpy(msg,"CSI Dec Private Mode Save Don't scroll to bottom on tty output",sizeof(msg));
                break;
            case 1011:
                strncpy(msg,"CSI Dec Private Mode Save Don't Scroll to Bottom on key Press",sizeof(msg));
                break;
            case 1035:
                strncpy(msg,"CSI Dec Private Mode Save Disable special modifiers for Alt and Numlock keys",sizeof(msg));
                break;
            case 1036:
                strncpy(msg,"CSI Dec Private Mode Save Don't Send SEC when Meta Modifies a key",sizeof(msg));
                break;
            case 1037:
                strncpy(msg,"CSI Dec Private Mode Save Send VT220 Remove from the editing Keypad Delete Key",sizeof(msg));
                break;
            case 1047:
                strncpy(msg,"CSI Save Use Normal Screen Buffer clearing first",sizeof(msg));
                break;
            case 1048:
                strncpy(msg,"CSI Save Cursor as in DECRC",sizeof(msg));
                break;
            case 1049:
                strncpy(msg,"CSI Save Use Normal Screen Buffer and restore cursor",sizeof(msg));
                break;
            case 1051:
                strncpy(msg,"CSI Save Sun Function key mode",sizeof(msg));
                break;
            case 1052:
                strncpy(msg,"CSI Save HP function key mode",sizeof(msg));
                break;
            case 1053:
                strncpy(msg,"CSI Save SCO Function key mode",sizeof(msg));
                break;
            case 1060:
                strncpy(msg,"CSI Save Legacy Keyboard emulation",sizeof(msg));
                break;
            case 1061:
                strncpy(msg,"CSI Save SunPC keyboard emulatsion of VT 220 keyboard",sizeof(msg));
                break;
            case 2004:
                strncpy(msg,"CSI Save Bracketed paste mode",sizeof(msg));
                break;
            default:
                snprintf(msg,sizeof(msg),"Oops CSI Dec Mode Restore %d invalid",buf[0]);
            }
        } else if (lead != ' ' && tail == ' ' && cnt == 3) {
        } else {
            flushtext("CSI Save invalid");
        }
        break;
    case 't':   
        // CSI P s ; P s ; P s t  Window manipulation (from dtterm, as well 
        //                   as extensions). These controls may be disabled 
        //                   using the allowWindowOps resource. Valid values 
        //                   for the first (and any additional parameters) are:
        //    P s = 1 → De-iconify window. 
        //    P s = 2 → Iconify window. 
        //    P s = 3 ; x ; y → Move window to [x, y]. 
        //    P s = 4 ; height ; width → Resize the xterm window to height and width in pixels. 
        //    P s = 5 → Raise the xterm window to the front of the stacking order. 
        //    P s = 6 → Lower the xterm window to the bottom of the stacking order. 
        //    P s = 7 → Refresh the xterm window. 
        //    P s = 8 ; height ; width → Resize the text area to [height;width] in characters. 
        //    P s = 9 ; 0 → Restore maximized window. 
        //    P s = 9 ; 1 → Maximize window (i.e., resize to screen size). 
        //    P s = 1 1 → Report xterm window state. If the xterm window is open (non-iconified), it returns CSI 1 t . If the xterm window is iconified, it returns CSI 2 t . 
        //    P s = 1 3 → Report xterm window position as CSI 3 ; x; yt 
        //    P s = 1 4 → Report xterm window in pixels as CSI 4 ; height ; width t 
        //    P s = 1 8 → Report the size of the text area in characters as CSI 8 ; height ; width t 
        //    P s = 1 9 → Report the size of the screen in characters as CSI 9 ; height ; width t 
        //    P s = 2 0 → Report xterm window’s icon label as OSC L label ST 
        //    P s = 2 1 → Report xterm window’s title as OSC l title ST 
        //    P s >= 2 4 → Resize to P s lines (DECSLPP)
        // CSI P t ; P l ; P b ; P r ; P s $ t  Reverse Attributes in 
        //                                      Rectangular Area (DECRARA). 
        strncpy(msg,"CSI Window manipulation invalid command",sizeof(msg));
        if (lead == ' ' && tail == ' ') {
            // CSI P s ; P s ; P s t       
            if (cnt == 0) { // 1 arg
                switch (buf[0]) {
                case 1:
                    strncpy(msg,"CSI De-iconify window",sizeof(msg));
                    break;
                case 2:
                    strncpy(msg,"CSI Iconify Window",sizeof(msg));
                    break;
                case 5:
                    strncpy(msg,"CSI Raise xterm to front of stacking order",sizeof(msg));
                    break;
                case 6:
                    strncpy(msg,"CSI Lower Xterm to bottom of stacking order",sizeof(msg));
                    break;
                case 7:
                    strncpy(msg,"CSI Refresh xterm",sizeof(msg));
                    break;
                case 11:
                    strncpy(msg,"CSI Report xterm window state",sizeof(msg));
                    break;
                case 13:
                    strncpy(msg,"CSI Report xterm window position",sizeof(msg));
                    break;
                case 14:
                    strncpy(msg,"CSI Report window size in pixels",sizeof(msg));
                    break;
                case 18:
                    strncpy(msg,"CSI Report size of text area in characters",sizeof(msg));
                    break;
                case 19:
                    strncpy(msg,"CSI Report size of screen in characters",sizeof(msg));
                    break;
                case 20:
                    strncpy(msg,"CSI Report xterm window's icon label",sizeof(msg));
                    break;
                case 21:
                    strncpy(msg,"CSI Report xterm window's title",sizeof(msg));
                    break;
                }
                if (buf[0] > 24) {
                    snprintf(msg,sizeof(msg),"CSI Resize xterm to %d lines",buf[0]);
                }
            } else if (cnt == 1) { // 2 args
                switch (buf[0]) {
                case 9:
                    if (buf[1] == 0) {
                        strncpy(msg,"CSI Restore Maximized window",sizeof(msg));
                    } else if (buf[1] == 1) {
                        strncpy(msg,"CSI Maximize window",sizeof(msg));
                    }
                    break;
                }
            } else if (cnt == 2) { // 3 args
                switch (buf[0]) {
                default: 
                case 3:
                    snprintf(msg,sizeof(msg),"CSI Move window to [%d,%d]",buf[1],buf[2]);
                    break;
                case 4:
                    snprintf(msg,sizeof(msg),"CSI Resize xterm to %d,%d pixels",
                                             buf[1],buf[2]);
                    break;
                case 8:
                    snprintf(msg,sizeof(msg),
                            "CSI Resize text area to [%d;%d] in characters",
                            buf[1],buf[2]);
                    break;
                }
            }
        } else if (lead == ' ' && tail == '$' && cnt >= 4) {
            // CSI P t ; P l ; P b ; P r ; P s $ t  Reverse Attributes in 
            //                                      Rectangular Area (DECRARA). 
            snprintf(msg,sizeof(msg),
                     "CSI Reverse attributes in rectangle t %d,l %d,b %d,r %d",
                     buf[0],buf[1],buf[2],buf[3]);
            
            int i=4;
            while (i < cnt) {
                switch (buf[0]) {
                case 1:
                    strncat(msg," Bold",sizeof(msg)-strlen(msg)-1);
                    break;
                case 4:
                    strncat(msg," Underline",sizeof(msg)-strlen(msg)-1);
                    break;
                case 5:
                    strncat(msg," Blinking",sizeof(msg)-strlen(msg)-1);
                    break;
                case 7:
                    strncat(msg," Negative Image",sizeof(msg)-strlen(msg)-1);
                    break;
                default:
                    snprintf(msg1,sizeof(msg1)," %d invalid",buf[0]);
                    strncat(msg,msg1,sizeof(msg)-strlen(msg)-1);
                }
                i++;
            }
        }
        flushtext(msg);
        break;
    case 'u':
        // CSI u     Save cursor (ANSI.SYS)  
       flushtext("CSI Save Cursor");
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

        } else {
            strncpy(msg,"CSI Copy Rectangle Area invalid args",sizeof(msg));
        }
        flushtext(msg);
        break;
    case 'w':
        // CSI P t ; P l ; P b ; P r ’ w     
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
            strncpy(msg,"CSI Enable Filter Rectangle, Report any locator motion",
                        sizeof(msg));
        } else if (lead == ' ' && tail == ' ' && cnt < 1) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,clp,clp,clp", buf[0]);
        } else if (lead == ' ' && tail == ' ' && cnt < 2) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,clp,clp", buf[0],buf[1]);
        } else if (lead == ' ' && tail == ' ' && cnt < 3) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,%d,clp", buf[0],buf[1],buf[2]);
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,%d,%d", buf[0],buf[1],buf[2],buf[3]);
        } else if (lead == ' ' && tail == ' ' && cnt < 4) {
            snprintf(msg,sizeof(msg),"CSI Enable Filter Rectangle, tlbr= "
                    "%d,%d,%d,%d", buf[0],buf[1],buf[2],buf[3]);
        } else {
            strncpy(msg,"CSI Enable Filter Rectangle invalid args",sizeof(msg));
        }
        flushtext(msg);
        break;
    case 'x':
        // CSI P s x         Request Terminal Parameters (DECREQTPARM)       
        //    if P s is a "0" (default) or "1", and xterm is emulating VT100,
        //    the control sequence elicits a response of the same form whose
        //    parameters describe the terminal: 
        //    P s → the given P s incremented by 2. 
        //    1 → no parity 
        //    1 → eight bits 
        //    1 2 8 → transmit 38.4k baud 
        //    1 2 8 → receive 38.4k baud 
        //    1 → clock multiplier 
        //    0 → STP flags
        // CSI P s x         Select Attribute Change Extent (DECSACE).       
        //    P s = 0 → from start to end position, wrapped 
        //    P s = 1 → from start to end position, wrapped 
        //    P s = 2 → rectangle (exact).
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
            strncpy(msg,"CSI Request Terminal Parameters", sizeof(msg));
        } else if (lead == ' ' && tail == ' ' && cnt <= 0) {
            // if 1st param is really a char this fails
            snprintf(msg,sizeof(msg),"Fill Rectangular area with %c tlbr= "
                    "%d,%d,%d,%d", buf[0],buf[1],buf[2],buf[3],buf[4]);
        } else {
            strncpy(msg,"CSI Copy Rectangle Area invalid args",sizeof(msg));
        }
        flushtext(msg);
        break;
    case 'z':
        // CSI P s ; P u ’ z Enable Locator Reporting (DECELR)       
        //    Valid values for the first parameter: 
        //    P s = 0 → Locator disabled (default) 
        //    P s = 1 → Locator enabled 
        //    P s = 2 → Locator enabled for one report, then disabled 
        //    The second parameter specifies the coordinate unit for locator reports. 
        //    Valid values for the second parameter: 
        //    P u = 0 or omitted → default to character cells 
        //    P u = 1 → device physical pixels 
        //    P u = 2 → character cells
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
                strncpy(msg1,"CSI Disable Locator Reporting",sizeof(msg1));
                break;
            case 1:
                strncpy(msg1,"CSI Enable Locator Reporting",sizeof(msg1));
                break;
            case 2:
                strncpy(msg1,"CSI Enable Locator Reporting once, then disable",sizeof(msg1));
                break;
            default:
                strncpy(msg1,"Invalid CSI Enable Locator Reporting Command",sizeof(msg1));
                ok=0;
            }
            if (ok) {
            switch (buf[1]) {
            case 0:
                snprintf(msg,sizeof(msg),"%s %s",msg1,"for Character cells");
                break;
            case 1:
                snprintf(msg,sizeof(msg),"%s %s",msg1,"for physical pixels");
                break;
            case 2:
                snprintf(msg,sizeof(msg),"%s %s",msg1,"for Character cells");
                break;
            default:
                strncpy(msg,"Invalid CSI Enable Locator Reporting Command",sizeof(msg));

            }
            }
        } else if (lead == ' ' && tail == '$' && cnt == 3) {
            snprintf(msg,sizeof(msg),"CSI Erase Rectangular Area, tlbr %d,%d,%d,%d",
                     buf[0],buf[1],buf[2],buf[3]);

        } else {
            strncpy(msg,"Invalid CSI Enable Locator Reporting Command",sizeof(msg));
        }
        flushtext(msg);
        break;
    case '{':
        // CSI P m ’ {               Select Locator Events (DECSLE)   //}
        //    Valid values for the first (and any additional parameters) are: 
        //    P s = 0 → only respond to explicit host requests (DECRQLP) 
        //    (default) also cancels any filter rectangle 
        //    P s = 1 → report button down transitions 
        //    P s = 2 → do not report button down transitions 
        //    P s = 3 → report button up transitions 
        //    P s = 4 → do not report button up transitions
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
                strncpy(msg,"CSI Select Locator Events: report button down transitions",sizeof(msg));
                break;
            case 1:
                strncpy(msg,"CSI Select Locator Events: don't report button down transitions",sizeof(msg));
                break;
            case 2:
                strncpy(msg,"CSI Select Locator Events: report button up transitions",sizeof(msg));
                break;
            case 3:
                strncpy(msg,"CSI Select Locator Events: don't report button up transitions",sizeof(msg));
            default:
                strncpy(msg,"CSI Select Locator Events: Invalid argument",sizeof(msg));
            }
        } else if (lead == ' ' && tail == '$' && cnt == 3) {
            snprintf(msg,sizeof(msg),"CSI Selective Erase Rectangular Area, tlbr %d,%d,%d,%d",
                     buf[0],buf[1],buf[2],buf[3]);

        }
        flushtext(msg);
        break;
    case '|':
        // CSI P s ’ |               Request Locator Position (DECRQLP)      
        //    Valid values for the parameter are: 
        //    P s = 0 , 1 or omitted → transmit a single DECLRP locator 
        //    report If Locator Reporting has been enabled by a DECELR,
        //    xterm will respond with a DECLRP Locator Report. This report is
        //    also generated on button up and down events if they have been
        //    enabled with a DECSLE, or when the locator is detected outside
        //    of a filter rectangle, if filter rectangles have been enabled
        //    with a DECEFR. → CSI P e ; P b ; P r ; P c ; P p & w Parameters
        //    are [event;button;row;column;page]. 
        //    Valid values for the event: 
        //    P e = 0 → locator unavailable - no other parameters sent 
        //    P e = 1 → request - xterm received a DECRQLP 
        //    P e = 2 → left button down 
        //    P e = 3 → left button up 
        //    P e = 4 → middle button down 
        //    P e = 5 → middle button up 
        //    P e = 6 → right button down 
        //    P e = 7 → right button up 
        //    P e = 8 → M4 button down 
        //    P e = 9 → M4 button up 
        //    P e = 1 0 → locator outside filter rectangle 
        //    ‘‘button’’ parameter is a bitmask indicating which buttons 
        //    are pressed: 
        //    P b = 0 → no buttons down 
        //    P b & 1 → right button down 
        //    P b & 2 → middle button down 
        //    P b & 4 → left button down 
        //    P b & 8 → M4 button down 
        //    ‘‘row’’ and ‘‘column’’ parameters are the coordinates of the
        //    locator position in the xterm window, encoded as ASCII decimal. 
        //    The ‘‘page’’ parameter is not used by xterm, and will be omitted.
        if (cnt == -1) {
            cnt++;
            buf[cnt] = 0;
        }
        if (lead == ' ' && tail == '`' && (buf[cnt] == 0 || buf[cnt] == 1)) {
            strncpy(msg,"CSI Request Locator Position",sizeof(msg));
        } else {
            strncpy(msg,"CSI Request Locator Position - invalid argument",sizeof(msg));
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
        //      set (default)—SCO Console only.
        // 11   Map Hex 00-7F of the PC character set codes to the current
        //      7-bit display character set—SCO Console only.
        // 12   Map Hex 80-FF of the current character set to the current 
        //      7-bit display character set—SCO Console only.
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
            flushtext("unprocessed CSI");
    }
}

void parse(int c, HINT_PARAM_UNUSED char *name, HINT_PARAM_UNUSED FILE *fd) {
    int c1;
    if (c < 32 && c != ESC) {
        flushtext("text");
        switch(c) {
        case ENQ:
            addStrToText("text","<ENQ>");
            flushtext("C0 Control Character (Ctrl-E) return Terminal Status");
            break;
        case BEL:
            addStrToText("text","<BEL>");
            flushtext("C0 Control Character (Ctrl-G) Bell");
            break;
        case BS:
            addStrToText("text","<BS>");
            flushtext("C0 Control Character (Ctrl-H) Backspace");
            break;
        case TAB:
            addStrToText("text","<TAB>");
            flushtext("C0 Control Character (Ctrl-I) Horizontal Tab");
            break;
        case LF:
            addStrToText("text","<NL>");
            flushtext("C0 Control Character Ctrl-J or New Line");
            break;
        case VT:
            addStrToText("text","<VT>");
            flushtext("C0 Control Character (Ctrl-K) Vertical Tab");
            break;
        case FF:
            addStrToText("text","<FF>");
            flushtext("C0 Control Character (Ctrl-L) Form Feed or New Page)");
            break;
        case CR:
            addStrToText("text","<CR>");
            flushtext("C0 Control Character (Ctrl-M) Carriage Return");
            break;
        case SO:
            addStrToText("text","<SO>");
            flushtext("C0 Control Character (Ctrl-N) Switch to Alternate Character Set (G1)");
            break;
        case SI:
            addStrToText("text","<SI>");
            flushtext("C0 Control Character (Ctrl-O) Switch to Standard Character Set (G0)");
            break;
        default:
                snprintf(msg,sizeof(msg),"<0x%2x>",c);
                addStrToText("text",msg);
                snprintf(msg,sizeof(msg),"C0 Control Character (Ctrl-%c)",
                                 c+0x40);
                flushtext(msg);
        }
    } else if (c != ESC) {
        addCharToText("text",c);
    } else {   // control sequence starting with escape
        flushtext("text");       // flush anything before the <esc>
        addStrToText("text","<ESC>");
        if ((c = getc(fd)) == EOF) {
            flushtext("interrupted control string");
            return;
        } 
        if (c <= 127) {
            addCharToText("text",c);
        } else {
            snprintf(msg,sizeof(msg),"<0x%x>",c);
            addStrToText("text",msg);
        }
        switch (c) {
        case ' ':
            // ESC SP F 7-bit controls (S7C1T).
            // ESC SP G 8-bit controls (S8C1T).
            // ESC SP L Set ANSI conformance level 1 (dpANS X3.134.1).
            // ESC SP M Set ANSI conformance level 2 (dpANS X3.134.1).
            // ESC SP N Set ANSI conformance level 3 (dpANS X3.134.1).
            if ((c = getc(fd)) == EOF) {
                flushtext("interrupted control string");
                return;
            } 
            addCharToText("text",c);
            switch (c) {
            case 'F':
                flushtext("7-bit controls");
                break;
            case 'G':
                flushtext("8-bit controls");
                break;
            case 'L':
                flushtext("Set ANSI conformance level 1");
                break;
            case 'M':
                flushtext("Set ANSI conformance level 2");
                break;
            case 'N':
                flushtext("Set ANSI conformance level 3");
                break;
            default:
                flushtext("Invalid '<ESC> ' sequence");
            }
            break;
        case '#':
            // ESC # 3  DEC double-height line, top half (DECDHL)
            // ESC # 4  DEC double-height line, bottom half (DECDHL)
            // ESC # 5  DEC single-width line (DECSWL)
            // ESC # 6  DEC double-width line (DECDWL)
            // ESC # 8  DEC Screen Alignment Test (DECALN)
            if ((c = getc(fd)) == EOF) {
                flushtext("interrupted control string");
                return;
            } 
            addCharToText("text",c);
            switch (c) {
            case '3':
                flushtext("DEC Double-height line, top half");
                break;
            case '4':
                flushtext("DEC Double-height line, bottom half");
                break;
            case '5':
                flushtext("DEC single-width line");
                break;
            case '6':
                flushtext("DEC double-width line");
                break;
            case '8':
                flushtext("DEC Screen Alignment Test");
                break;
            default:
                flushtext("Invalid '<ESC>#' sequence");
            }
            break;
        case '%':
            // ESC % @  Select default character set, ISO 8859-1 (ISO 2022)
            // ESC % G  Select UTF-8 character set (ISO 2022)
            if ((c = getc(fd)) == EOF) {
                flushtext("interrupted control string");
                return;
            } 
            addCharToText("text",c);
            switch (c) {
            case '@':
                flushtext("Select default character set");
                break;
            case 'G':
                flushtext("Select UTF-8 character set");
                break;
            default:
                flushtext("Invalid '<ESC>%' sequence");
            }
            break;
        case '(':
        case ')':
        case '*':
        case '+':
            if ((c1 = getc(fd)) == EOF) {
                flushtext("interrupted control string");
                return;
            } 
            addCharToText("text",c1);
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
            flushtext(msg);
            break;
        case '7':
            flushtext("Save Curser");
            break;
        case '8':
            flushtext("Restor Curser");
            break;
        case '=':
            flushtext("Application Keypad");
            break;
        case '>':
            flushtext("Normal Keypad");
            break;
        case 'F':
            flushtext("Cursor to lower left corner");
            break;
        case 'c':
            flushtext("Full Reset");
            break;
        case 'l':
            flushtext("Lock Memory above cursor");
            break;
        case 'm':
            flushtext("Unlock Memory");
            break;
        case 'n':
            flushtext("Invoke G2 Character Set as GL");
            break;
        case 'o':
            flushtext("Invoke G3 Character Set as GL");
            break;
        case '|':
            flushtext("Invoke G3 Character Set as GR");
            break;
        case '}':
            flushtext("Invoke G2 Character Set as GR");
            break;
        case '~':
            flushtext("Invoke G1 Character Set as GR");
            break;

        // following are C1 (8 bit control chars)
        // xterm recognizes these as 7 bit or 8 bit controls

        case 'D':
            flushtext("Index");
            break;
        case 0x84:
            flushtext("C1 Index");
            break;
        case 'E':
            flushtext("Next Line");
            break;
        case 0x85:
            flushtext("C1 Next Line");
            break;
        case 'H':
            flushtext("Tab Set");
            break;
        case 0x88:
            flushtext("Tab Set");
            break;
        case 'M':
            flushtext("Reverse Index");
            break;
        case 0x8d:
            flushtext("Reverse Index");
            break;
        case 'N':
            flushtext("Single Shift Select G2 Character Set (next char only)");
            break;
        case 0x8e:
            flushtext("Single Shift Select G2 Character Set (next char only)");
            break;
        case 'O':
            flushtext("Single Shift Select G3 Character set(Next char only)");
            break;
        case 0x8f:
            flushtext("Single Shift Select G3 Character set(Next char only)");
            break;
        case 'V':
            flushtext("Start of Guarded Area (SPA)");
            break;
        case 0x96:
            flushtext("Start of Guarded Area (SPA)");
            break;
        case 'W':
            flushtext("End of Guarded Area (EPA)");
            break;
        case 0x97:
            flushtext("End of Guarded Area (EPA)");
            break;
        case 'X':
            flushtext("Start of String (SOS)");
            break;
        case 0x98:
            flushtext("Start of String (SOS)");
            break;
//        ST handled by printtoST()
//        case '\':
//            flushtext("String Terminator (ST)");
//            break;
//        case 0x9c:
//            flushtext("String Terminator (ST)");
//            break;
        //ESC Z Return Terminal ID (DECID is 0x9a). Obsolete form of CSI c (DA).
        case 'Z':
            flushtext("Return Terminal ID");
            break;
        case 0x9a:
            flushtext("Return Terminal ID");
            break;
//xxxESC P Device Control String ( DCS is 0x90)
//xxx should decode these
        case 'P':
            flushtext("Device Control String(DCS)");
            printtoST("DCS",fd);
            break;
        case 0x90:
            flushtext("Device Control String(DCS)");
            printtoST("DCS",fd);
            break;
//xxx ESC [ Control Sequence Introducer ( CSI is 0x9b)
        case '[':
            //xxxxxx but we already put the esc[ in the buffer
            processCSI("CSI",fd);
            break;
//xxx ESC ] Operating System Command ( OSC is 0x9d) ??? then what
        case ']':
            flushtext("Operating System Command");
            readOSC("OSC",fd);
            break;
        case 0x9d:
            flushtext("Operating System Command");
            readOSC("OSC",fd);
            break;
        case '^':
            flushtext("Privacy Message(PM)");
            printtoST("PM",fd);
            break;
        case 0x9e:
            flushtext("Privacy Message(PM)");
            printtoST("PM",fd);
            break;
//xxx ESC _ Application Program Command ( APC is 0x9f) ???
// xterm has no apc functions ignore data to ST (ESC \)
        case '_':
            flushtext("Application Program Command(APC)");
            printtoST("APC command",fd);
            break;

        case 0x9f:
            flushtext("Application Program COmmand");
            break;
        default:
            flushtext("Unrecognized  CSI (Control Sequence Introducer");
        }
    }
}

void decode(char *name, FILE *fd) {
    int c;

    while((c = getc(fd)) != EOF) {
        parse(c, name, fd);
    }
}

int main( int argc, char *argv[]) {
    char *p;
    FILE *fd;

    while(argc > 1 && *argv[1] == '-') {
        for(p = &argv[1][1]; *p != '\0'; p++) {
            switch(*p) {
                case 'h':  /* force starting off in 4014 mode */
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
            decode("standard input", stdin);
    } else {
        while(--argc > 0) {
            if((fd = fopen(*++argv, "r")) == NULL) {
                fprintf(stderr, "xterm_decode: can't open %s\n", *argv);
                continue;
            }
            decode(*argv, fd);
            fclose(fd);
        }
    }
}
