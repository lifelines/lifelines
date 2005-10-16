/* 
   Copyright (c) 2001 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: 
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * prettytt.c -- 
 * This is a trivial program, which takes a binary translation table
 * (such as one from the konwert program)
 * and makes a pretty ascii version, safe for distribution via email
 * or display on the web, and still valid.
 * See dispversion at bottom for more explanation.
 *
 *   Created: 2001/12/29 by Perry Rapp
 *==============================================================*/

#include <stdio.h>
#include <ctype.h>

#define PROG_VERSION "prettytt: 2001/12/29"

static void prettytt(const char * f1, const char * f2);
static void dispversion(void);

typedef unsigned char uchar;

static int skipws=0;

int main (int argc, char * argv[])
{
	const char *f1=0, *f2=0;
	int count=0, i;
	for (i=1; i<argc; ++i) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'w')
				skipws=1;
			else {
				dispversion();
				return 0;
			}
		} else {
			if (!count)
				f1 = argv[i];
			else if (count==1)
				f2 = argv[i];
			else {
				dispversion();
				return 0;
			}
			++count;
		}
	}
	prettytt(f1, f2);
	return 0;
}

static int
iscr (char c)
{
	return c=='\n' || c=='\r';
}

static void
prettytt (const char * f1, const char * f2)
{
	FILE *fp1=stdin, *fp2=stdout;
	int c;
	if (f1) {
		fp1 = fopen(f1, LLREADTEXT);
		if (!fp1) {
			fprintf(stderr, "Could not open input file: %s\n"
				, f1);
			goto end;
		}
	}
	if (f2) {
		fp2 = fopen(f2, LLWRITETEXT);
		if (!fp2) {
			fprintf(stderr, "Could not open output file: %s\n"
				, f2);
			goto end;
		}
	}

	if (f1) {
		fprintf(fp2, "##!name: %s\n", f1);
	}
	fprintf(fp2, "##!sep=\n");

beginline:
	/* skip over line returns */
	while (1) {
		c = fgetc(fp1);
		if (c == EOF) goto end;
		if (!iscr((uchar)c)) break;
	}
	/* skip over leading whitespace if option set */
	if (skipws && isspace((uchar)c)) {
		while (1) {
			c = fgetc(fp1);
			if (c == EOF) goto end;
			if (!isspace((uchar)c)) break;
		}
	}
	while (1) {
		if (c == '\t') {
			fprintf(fp2, "=");
		} else if (iscr((uchar)c)) {
			fprintf(fp2, "\n");
			goto beginline;
		} else {
			if ((uchar)c > 127)
				fprintf(fp2, "#%d", c);
			else
				fprintf(fp2, "%c", c);
		}
		c = fgetc(fp1);
		if (c == EOF) {
			fprintf(fp2, "\n");
			goto end;
		}
	}
	goto beginline;

end:
	if (fp1 != stdin) fclose(fp1);
	if (fp2 != stdout) fclose(fp2);
}


static void 
dispversion (void)
{
	printf(PROG_VERSION);
	printf("\n");
	printf("\n");
	printf("This program converts a LifeLines binary translation table\n");
	printf("to an ascii version (safe for transmittal in email or\n");
	printf("display on the web).\n");
	printf("With no arguments, it will read from stdin and write to\n");
	printf("stdout. With one argument, it will treat it as the input\n");
	printf("filename. With two arguments, it will treat the first as\n");
	printf("the input filename, and the second as the output filename.\n");
	printf("If an argument is -w, it will ignore leading whitespace.\n");
	printf("This is oriented towards use with files from konwert:\n");
	printf("http://kki.net.pl/qrczak/programy/dos/konwert/index.html");
}

