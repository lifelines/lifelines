/* 
   Copyright (c) 2001 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * dispcode.c -- 
 * This is a trivial program, just to show the character code 
 * numbers & representations on the current console.
 * Just run it with no args & gaze happily at the display.
 * Press return when tired of gazing.
 *
 *   Created: 2001/07/07 by Perry Rapp
 *==============================================================*/

#include <stdio.h>

#define PROG_VERSION "dispcode: 2001/07/07"

static void dispall(void);
static void dispversion(void);
static void dispcode(unsigned int charnum);

int main(int argc, char * argv[])
{
	if (argc==1)
		dispall();
	else if (argv[1][0] == '-')
		dispversion();
	else {
		int i, rtn;
		unsigned int charnum;
		for (i=1; i<argc; i++) {
			if (argv[i][0] == '0' 
				&& (argv[i][1] == 'x' || argv[i][1] == 'X')) {
				rtn = sscanf(argv[i], "%x", &charnum);
			} else {
				rtn = sscanf(argv[i], "%ud", &charnum);
			}
			if (rtn == 1)
				dispcode(charnum);
			else
				printf("Skipping unrecognized arg (%s)\n", argv[i]);
		}

			
	}
	return 0;
}

static void
dispall(void)
{
	int i, j;
	unsigned int charnum;
	char c;

	/* 24 rows of display */
	for (i=33; i<33+23; i++) {
		if (i>33) printf("\n");
		/* 10 columns across */
		for (j=0; j<11; j++) {
			charnum = i+j*23;
			if (charnum > 255)
				break;
			printf("%3d:%c", charnum, charnum);
			if (j<9)
				printf("  "); /* inter-column padding */
		}
	}
	scanf("%c", &c);
}

static void 
dispversion(void)
{
	printf(PROG_VERSION);
	printf("\n");
	printf("\n");
	printf("With no arguments, it will display the entire character\n");
	printf("table, and then pause until user presses <enter>.\n");
	printf("\n");
	printf("If given arguments, it will treat them as decimal\n");
	printf("character codes (unless they begin with 0x) and\n");
	printf("display their representation.\n");
	printf("\n");
	printf("If the first argument begins with a hyphen, this help\n");
	printf("is displayed.\n");
}

static void
dispcode(unsigned int charnum)
{
	printf("%3d (0x%02x):%c\n", charnum, charnum, charnum);
}
