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
/* String routines that know about ISO Latin 1 8 bit letters.
 * To be used with LifeLines genealogy program.
 * Copyright(c) 1996 Hannu Väisänen; all rights reserved.
*/

#include "llstdlib.h"
#include "sys_inc.h"
#include <stdarg.h>
#include "arch.h"
#include "mystring.h"


static int safechar(int c);

extern int opt_finnish;	/* use standard strcmp, strncmp if this is FALSE */

/* developer mode to help find bad sign-extensions */
static int hardfail = 0;


typedef struct {
  unsigned char toup;    /* Corresponding uppercase letter. */
  unsigned char tolow;   /* Corresponding lowercase letter. */
  unsigned char iscntrl; /* Is control character? */
  unsigned char isup;    /* Is uppercase letter?  */
  unsigned char islow;   /* Is lowercase letter?  */
} my_charset_info;


static my_charset_info ISO_Latin1[] = {
  /*   0 0 */ {0, 0, 1, 0, 0},
  /*   1 1 */ {1, 1, 1, 0, 0},
  /*   2 2 */ {2, 2, 1, 0, 0},
  /*   3 3 */ {3, 3, 1, 0, 0},
  /*   4 4 */ {4, 4, 1, 0, 0},
  /*   5 5 */ {5, 5, 1, 0, 0},
  /*   6 6 */ {6, 6, 1, 0, 0},
  /*   7 7 */ {7, 7, 1, 0, 0},
  /*   8 8 */ {8, 8, 1, 0, 0},
  /*   9 9 */ {9, 9, 1, 0, 0},
  /*  10 10 */ {10, 10, 1, 0, 0},
  /*  11 11 */ {11, 11, 1, 0, 0},
  /*  12 12 */ {12, 12, 1, 0, 0},
  /*  13 13 */ {13, 13, 1, 0, 0},
  /*  14 14 */ {14, 14, 1, 0, 0},
  /*  15 15 */ {15, 15, 1, 0, 0},
  /*  16 16 */ {16, 16, 1, 0, 0},
  /*  17 17 */ {17, 17, 1, 0, 0},
  /*  18 18 */ {18, 18, 1, 0, 0},
  /*  19 19 */ {19, 19, 1, 0, 0},
  /*  20 20 */ {20, 20, 1, 0, 0},
  /*  21 21 */ {21, 21, 1, 0, 0},
  /*  22 22 */ {22, 22, 1, 0, 0},
  /*  23 23 */ {23, 23, 1, 0, 0},
  /*  24 24 */ {24, 24, 1, 0, 0},
  /*  25 25 */ {25, 25, 1, 0, 0},
  /*  26 26 */ {26, 26, 1, 0, 0},
  /*  27 27 */ {27, 27, 1, 0, 0},
  /*  28 28 */ {28, 28, 1, 0, 0},
  /*  29 29 */ {29, 29, 1, 0, 0},
  /*  30 30 */ {30, 30, 1, 0, 0},
  /*  31 31 */ {31, 31, 1, 0, 0},
  /*  32   */ {32, 32, 0, 0, 0},
  /*  33 ! */ {33, 33, 0, 0, 0},
  /*  34 " */ {34, 34, 0, 0, 0},
  /*  35 # */ {35, 35, 0, 0, 0},
  /*  36 $ */ {36, 36, 0, 0, 0},
  /*  37 % */ {37, 37, 0, 0, 0},
  /*  38 & */ {38, 38, 0, 0, 0},
  /*  39 ' */ {39, 39, 0, 0, 0},
  /*  40 ( */ {40, 40, 0, 0, 0},
  /*  41 ) */ {41, 41, 0, 0, 0},
  /*  42 * */ {42, 42, 0, 0, 0},
  /*  43 + */ {43, 43, 0, 0, 0},
  /*  44 , */ {44, 44, 0, 0, 0},
  /*  45 - */ {45, 45, 0, 0, 0},
  /*  46 . */ {46, 46, 0, 0, 0},
  /*  47 / */ {47, 47, 0, 0, 0},
  /*  48 0 */ {48, 48, 0, 0, 0},
  /*  49 1 */ {49, 49, 0, 0, 0},
  /*  50 2 */ {50, 50, 0, 0, 0},
  /*  51 3 */ {51, 51, 0, 0, 0},
  /*  52 4 */ {52, 52, 0, 0, 0},
  /*  53 5 */ {53, 53, 0, 0, 0},
  /*  54 6 */ {54, 54, 0, 0, 0},
  /*  55 7 */ {55, 55, 0, 0, 0},
  /*  56 8 */ {56, 56, 0, 0, 0},
  /*  57 9 */ {57, 57, 0, 0, 0},
  /*  58 : */ {58, 58, 0, 0, 0},
  /*  59 ; */ {59, 59, 0, 0, 0},
  /*  60 < */ {60, 60, 0, 0, 0},
  /*  61 = */ {61, 61, 0, 0, 0},
  /*  62 > */ {62, 62, 0, 0, 0},
  /*  63 ? */ {63, 63, 0, 0, 0},
  /*  64 @ */ {64, 64, 0, 0, 0},
  /*  65 A */ {65, 97, 0, 1, 0},
  /*  66 B */ {66, 98, 0, 1, 0},
  /*  67 C */ {67, 99, 0, 1, 0},
  /*  68 D */ {68, 100, 0, 1, 0},
  /*  69 E */ {69, 101, 0, 1, 0},
  /*  70 F */ {70, 102, 0, 1, 0},
  /*  71 G */ {71, 103, 0, 1, 0},
  /*  72 H */ {72, 104, 0, 1, 0},
  /*  73 I */ {73, 105, 0, 1, 0},
  /*  74 J */ {74, 106, 0, 1, 0},
  /*  75 K */ {75, 107, 0, 1, 0},
  /*  76 L */ {76, 108, 0, 1, 0},
  /*  77 M */ {77, 109, 0, 1, 0},
  /*  78 N */ {78, 110, 0, 1, 0},
  /*  79 O */ {79, 111, 0, 1, 0},
  /*  80 P */ {80, 112, 0, 1, 0},
  /*  81 Q */ {81, 113, 0, 1, 0},
  /*  82 R */ {82, 114, 0, 1, 0},
  /*  83 S */ {83, 115, 0, 1, 0},
  /*  84 T */ {84, 116, 0, 1, 0},
  /*  85 U */ {85, 117, 0, 1, 0},
  /*  86 V */ {86, 118, 0, 1, 0},
  /*  87 W */ {87, 119, 0, 1, 0},
  /*  88 X */ {88, 120, 0, 1, 0},
  /*  89 Y */ {89, 121, 0, 1, 0},
  /*  90 Z */ {90, 122, 0, 1, 0},
  /*  91 [ */ {91, 91, 0, 0, 0},
  /*  92 \ */ {92, 92, 0, 0, 0},
  /*  93 ] */ {93, 93, 0, 0, 0},
  /*  94 ^ */ {94, 94, 0, 0, 0},
  /*  95 _ */ {95, 95, 0, 0, 0},
  /*  96 ` */ {96, 96, 0, 0, 0},
  /*  97 a */ {65, 97, 0, 0, 1},
  /*  98 b */ {66, 98, 0, 0, 1},
  /*  99 c */ {67, 99, 0, 0, 1},
  /* 100 d */ {68, 100, 0, 0, 1},
  /* 101 e */ {69, 101, 0, 0, 1},
  /* 102 f */ {70, 102, 0, 0, 1},
  /* 103 g */ {71, 103, 0, 0, 1},
  /* 104 h */ {72, 104, 0, 0, 1},
  /* 105 i */ {73, 105, 0, 0, 1},
  /* 106 j */ {74, 106, 0, 0, 1},
  /* 107 k */ {75, 107, 0, 0, 1},
  /* 108 l */ {76, 108, 0, 0, 1},
  /* 109 m */ {77, 109, 0, 0, 1},
  /* 110 n */ {78, 110, 0, 0, 1},
  /* 111 o */ {79, 111, 0, 0, 1},
  /* 112 p */ {80, 112, 0, 0, 1},
  /* 113 q */ {81, 113, 0, 0, 1},
  /* 114 r */ {82, 114, 0, 0, 1},
  /* 115 s */ {83, 115, 0, 0, 1},
  /* 116 t */ {84, 116, 0, 0, 1},
  /* 117 u */ {85, 117, 0, 0, 1},
  /* 118 v */ {86, 118, 0, 0, 1},
  /* 119 w */ {87, 119, 0, 0, 1},
  /* 120 x */ {88, 120, 0, 0, 1},
  /* 121 y */ {89, 121, 0, 0, 1},
  /* 122 z */ {90, 122, 0, 0, 1},
  /* 123 { */ {123, 123, 0, 0, 0},
  /* 124 | */ {124, 124, 0, 0, 0},
  /* 125 } */ {125, 125, 0, 0, 0},
  /* 126 ~ */ {126, 126, 0, 0, 0},
  /* 127 127 */ {127, 127, 1, 0, 0},
  /* 128 128 */ {128, 128, 1, 0, 0},
  /* 129 129 */ {129, 129, 1, 0, 0},
  /* 130 130 */ {130, 130, 1, 0, 0},
  /* 131 131 */ {131, 131, 1, 0, 0},
  /* 132 132 */ {132, 132, 1, 0, 0},
  /* 133 133 */ {133, 133, 1, 0, 0},
  /* 134 134 */ {134, 134, 1, 0, 0},
  /* 135 135 */ {135, 135, 1, 0, 0},
  /* 136 136 */ {136, 136, 1, 0, 0},
  /* 137 137 */ {137, 137, 1, 0, 0},
  /* 138 138 */ {138, 138, 1, 0, 0},
  /* 139 139 */ {139, 139, 1, 0, 0},
  /* 140 140 */ {140, 140, 1, 0, 0},
  /* 141 141 */ {141, 141, 1, 0, 0},
  /* 142 142 */ {142, 142, 1, 0, 0},
  /* 143 143 */ {143, 143, 1, 0, 0},
  /* 144 144 */ {144, 144, 1, 0, 0},
  /* 145 145 */ {145, 145, 1, 0, 0},
  /* 146 146 */ {146, 146, 1, 0, 0},
  /* 147 147 */ {147, 147, 1, 0, 0},
  /* 148 148 */ {148, 148, 1, 0, 0},
  /* 149 149 */ {149, 149, 1, 0, 0},
  /* 150 150 */ {150, 150, 1, 0, 0},
  /* 151 151 */ {151, 151, 1, 0, 0},
  /* 152 152 */ {152, 152, 1, 0, 0},
  /* 153 153 */ {153, 153, 1, 0, 0},
  /* 154 154 */ {154, 154, 1, 0, 0},
  /* 155 155 */ {155, 155, 1, 0, 0},
  /* 156 156 */ {156, 156, 1, 0, 0},
  /* 157 157 */ {157, 157, 1, 0, 0},
  /* 158 158 */ {158, 158, 1, 0, 0},
  /* 159 159 */ {159, 159, 1, 0, 0},
  /* 160 160 */ {160, 160, 1, 0, 0},
  /* 161 ¡ */ {161, 161, 0, 0, 0},
  /* 162 ¢ */ {162, 162, 0, 0, 0},
  /* 163 £ */ {163, 163, 0, 0, 0},
  /* 164 ¤ */ {164, 164, 0, 0, 0},
  /* 165 ¥ */ {165, 165, 0, 0, 0},
  /* 166 ¦ */ {166, 166, 0, 0, 0},
  /* 167 § */ {167, 167, 0, 0, 0},
  /* 168 ¨ */ {168, 168, 0, 0, 0},
  /* 169 © */ {169, 169, 0, 0, 0},
  /* 170 ª */ {170, 170, 0, 0, 0},
  /* 171 « */ {171, 171, 0, 0, 0},
  /* 172 ¬ */ {172, 172, 0, 0, 0},
  /* 173 ­ */ {173, 173, 0, 0, 0},
  /* 174 ® */ {174, 174, 0, 0, 0},
  /* 175 ¯ */ {175, 175, 0, 0, 0},
  /* 176 ° */ {176, 176, 0, 0, 0},
  /* 177 ± */ {177, 177, 0, 0, 0},
  /* 178 ² */ {178, 178, 0, 0, 0},
  /* 179 ³ */ {179, 179, 0, 0, 0},
  /* 180 ´ */ {180, 180, 0, 0, 0},
  /* 181 µ */ {181, 181, 0, 0, 0},
  /* 182 ¶ */ {182, 182, 0, 0, 0},
  /* 183 · */ {183, 183, 0, 0, 0},
  /* 184 ¸ */ {184, 184, 0, 0, 0},
  /* 185 ¹ */ {185, 185, 0, 0, 0},
  /* 186 º */ {186, 186, 0, 0, 0},
  /* 187 » */ {187, 187, 0, 0, 0},
  /* 188 ¼ */ {188, 188, 0, 0, 0},
  /* 189 ½ */ {189, 189, 0, 0, 0},
  /* 190 ¾ */ {190, 190, 0, 0, 0},
  /* 191 ¿ */ {191, 191, 0, 0, 0},
  /* 192 À */ {192, 224, 0, 1, 0},
  /* 193 Á */ {193, 225, 0, 1, 0},
  /* 194 Â */ {194, 226, 0, 1, 0},
  /* 195 Ã */ {195, 227, 0, 1, 0},
  /* 196 Ä */ {196, 228, 0, 1, 0},
  /* 197 Å */ {197, 229, 0, 1, 0},
  /* 198 Æ */ {198, 230, 0, 1, 0},
  /* 199 Ç */ {199, 231, 0, 1, 0},
  /* 200 È */ {200, 232, 0, 1, 0},
  /* 201 É */ {201, 233, 0, 1, 0},
  /* 202 Ê */ {202, 234, 0, 1, 0},
  /* 203 Ë */ {203, 235, 0, 1, 0},
  /* 204 Ì */ {204, 236, 0, 1, 0},
  /* 205 Í */ {205, 237, 0, 1, 0},
  /* 206 Î */ {206, 238, 0, 1, 0},
  /* 207 Ï */ {207, 239, 0, 1, 0},
  /* 208 Ð */ {208, 240, 0, 1, 0},
  /* 209 Ñ */ {209, 241, 0, 1, 0},
  /* 210 Ò */ {210, 242, 0, 1, 0},
  /* 211 Ó */ {211, 243, 0, 1, 0},
  /* 212 Ô */ {212, 244, 0, 1, 0},
  /* 213 Õ */ {213, 245, 0, 1, 0},
  /* 214 Ö */ {214, 246, 0, 1, 0},
  /* 215 × */ {215, 215, 0, 0, 0},
  /* 216 Ø */ {216, 248, 0, 1, 0},
  /* 217 Ù */ {217, 249, 0, 1, 0},
  /* 218 Ú */ {218, 250, 0, 1, 0},
  /* 219 Û */ {219, 251, 0, 1, 0},
  /* 220 Ü */ {220, 252, 0, 1, 0},
  /* 221 Ý */ {221, 253, 0, 1, 0},
  /* 222 Þ */ {222, 254, 0, 1, 0},
  /* 223 ß */ {223, 223, 0, 0, 1},
  /* 224 à */ {192, 224, 0, 0, 1},
  /* 225 á */ {193, 225, 0, 0, 1},
  /* 226 â */ {194, 226, 0, 0, 1},
  /* 227 ã */ {195, 227, 0, 0, 1},
  /* 228 ä */ {196, 228, 0, 0, 1},
  /* 229 å */ {197, 229, 0, 0, 1},
  /* 230 æ */ {198, 230, 0, 0, 1},
  /* 231 ç */ {199, 231, 0, 0, 1},
  /* 232 è */ {200, 232, 0, 0, 1},
  /* 233 é */ {201, 233, 0, 0, 1},
  /* 234 ê */ {202, 234, 0, 0, 1},
  /* 235 ë */ {203, 235, 0, 0, 1},
  /* 236 ì */ {204, 236, 0, 0, 1},
  /* 237 í */ {205, 237, 0, 0, 1},
  /* 238 î */ {206, 238, 0, 0, 1},
  /* 239 ï */ {207, 239, 0, 0, 1},
  /* 240 ð */ {208, 240, 0, 0, 1},
  /* 241 ñ */ {209, 241, 0, 0, 1},
  /* 242 ò */ {210, 242, 0, 0, 1},
  /* 243 ó */ {211, 243, 0, 0, 1},
  /* 244 ô */ {212, 244, 0, 0, 1},
  /* 245 õ */ {213, 245, 0, 0, 1},
  /* 246 ö */ {214, 246, 0, 0, 1},
  /* 247 ÷ */ {247, 247, 0, 0, 0},
  /* 248 ø */ {216, 248, 0, 0, 1},
  /* 249 ù */ {217, 249, 0, 0, 1},
  /* 250 ú */ {218, 250, 0, 0, 1},
  /* 251 û */ {219, 251, 0, 0, 1},
  /* 252 ü */ {220, 252, 0, 0, 1},
  /* 253 ý */ {221, 253, 0, 0, 1},
  /* 254 þ */ {222, 254, 0, 0, 1},
  /* 255 ÿ */ {255, 255, 0, 0, 1},
};


/* Finnish sorting order.
 *
 * BUGS Æ æ  is sorted as 'a', it should be sorted as 'ae'.
 *      ß    is sorted as 's', it should be sorted as 'ss'.
 *      Þ þ  is sorted as 't', it should be sorted as 'th'.
 *
 * Since I don't have those letters in my data, I don't care. (-:
 */
const int my_ISO_Latin1_Finnish[] = {
      0,   1,   2,   3,   4,   5,   6,   7,
      8,   9,  10,  11,  12,  13,  14,  15,
     16,  17,  18,  19,  20,  21,  22,  23,
     24,  25,  26,  27,  28,  29,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,
     40,  41,  42,  43,  44,  45,  46,  47,
     48,  49,  50,  51,  52,  53,  54,  55,
     56,  57,  58,  59,  60,  61,  62,  63,
     64,  65,  66,  67,  68,  69,  70,  71, /* @ A B C D E F G */
     72,  73,  74,  75,  76,  77,  78,  79, /* H I J K L M N O */
     80,  81,  82,  83,  84,  85,  86,  86, /* P Q R S T U V W */
     88,  89,  90,  91,  92,  93,  94,  95, /* X Y Z [ \ ] */
     96,  65,  66,  67,  68,  69,  70,  71, /* @ a b c d e f g */
     72,  73,  74,  75,  76,  77,  78,  79, /* h i j k l m n o */
     80,  81,  82,  83,  84,  85,  86,  86, /* p q r s t u v w */
     88,  89,  90,  91,  92,  93, 126, 127, /* x y z { | } */
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
     65,  65,  65,  65,  92,  91,  65,  67, /* À Á Â Ã Ä Å Æ Ç */
     69,  69,  69,  69,  73,  73,  73,  73, /* È É Ê Ë Ë Í Î Ï */
     68,  78,  79,  79,  79,  93,  93, 215, /* Ð Ñ Ò Ó Ô Õ Ö × */
     93,  85,  85,  85,  89,  89,  84,  83, /* Ø Ù Û Û Ü Ý Þ ß */
     65,  65,  65,  65,  92,  91,  65,  67, /* à á â ã ä å æ ç */
     69,  69,  69,  69,  73,  73,  73,  73, /* è é ê ë ì í î ï */
     68,  78,  79,  79,  79,  93,  93, 215, /* ð ñ ò ó ô õ ö ÷ */
     93,  85,  85,  85,  89,  89,  84,  89, /* ø ù ú û ü ý þ ÿ */
    };


/* Is Latin 1 char? */
#define islat1(c) (0 <= (c) && (c) <= 255)


int
my_isalpha (const int c1)
{
  int c = safechar(c1);
  return (islat1(c) && (ISO_Latin1[c].isup || ISO_Latin1[c].islow));
}

int
my_iscntrl (const int c1)
{
  int c = safechar(c1);
  return (islat1(c) && ISO_Latin1[c].iscntrl);
}

int
my_islower (const int c1)
{
  int c = safechar(c1);
  return (islat1(c) && ISO_Latin1[c].islow);
}

int
my_isprint (const int c1)
{
  int c = safechar(c1);
  return (islat1(c) && !ISO_Latin1[c].iscntrl);
}

int
my_isupper (const int c1)
{
  int c = safechar(c1);
  return (islat1(c) && ISO_Latin1[c].isup);
}

int
my_tolower (const int c1)
{
  int c = safechar(c1);
  if (islat1(c)) return (ISO_Latin1[c].tolow);
  return c;
}

int
my_toupper (const int c1)
{
  /* BUG: ß is not converted to SS. */
  /* Note that ÿ does not have an   */
  /* uppercase form in ISO Latin 1. */
  int c = safechar(c1);
  if (islat1(c)) return (ISO_Latin1[c].toup);
  return c;
}


int
my_chrcmp (const int sa1,
           const int sa2)
{
  int s1 = safechar(sa1);
  int s2 = safechar(sa2);
  if (islat1(s1) && islat1(s2)) {
    return (my_ISO_Latin1_Finnish[s1] - my_ISO_Latin1_Finnish[s2]);
  }
  else {
    return (s1 - s2);
  }
}


int
my_strcmp (const char *s1, const char *s2, const int cmp_table[])
{
  int i;
  /* to make things easier, work with pointers that give uchars */
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  if(!opt_finnish) return(strcmp(s1,s2));

  for (i=0; p1[i] && p2[i]; i++) {
#if 0
    fprintf (stdout, "%d %c %c %d\n", i, p1[i], p2[i],
             (cmp_table[p1[i]] != cmp_table[p2[i]]));
#endif
    if (cmp_table[p1[i]] != cmp_table[p2[i]]) {
      break;
    }
  }
  return (cmp_table[p1[i]] - cmp_table[p2[i]]);
}

int
my_strncmp (const char *s1,
            const char *s2,
            const int n,
            const int cmp_table[])
{
  int i;
  /* to make things easier, work with pointers that give uchars */
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  if(!opt_finnish) return(strncmp(s1,s2,n));

  for (i=0; i<n && p1[i] && p2[i]; i++) {
    if (cmp_table[p1[i]] != cmp_table[p2[i]]) {
      break;
    }
  }
  if (i == n) return 0;
  return cmp_table[p1[i]] - cmp_table[p2[i]];
}


/* Change this to 1 if you want to test mystring and */
/* compile the progam, e.g. 'cc -I../hdrs mystring.c -o m'.  */
#if 0
#include "sys_inc.h"

int
cmp (const void *s,
     const void *t)
{
  const unsigned char **s1 = (const unsigned char **)s;
  const unsigned char **s2 = (const unsigned char **)t;
/*  fprintf (stdout, "cmp: '%s' '%s'\n", *s1, *s2); */
  return my_strcmp (*s1, *s2, my_ISO_Latin1_Finnish);
}

enum {NCHARS = 256};
int opt_finnish = 1;

int main()
{
#define NN 10
  unsigned char *test[] = {
    "Väisänen",
    "Wäisänen",
    "Wegelius",
    "Varis   ",
    "Waris   ",
    "Voionmaa",
    "Rissanen",
    "Häkkilä ",
    "Hakkila ",
    "Aaltonen"
  };

  const int *t = my_ISO_Latin1_Finnish;
  int i;
  int mini = NCHARS;
  int maxi = 0;
  for (i=0; i<NCHARS; i++) {   /* Min and max code for letters. */
    if (my_isalpha(i)) {
      if (mini > i) mini = i;
      if (maxi < i) maxi = i;
    }
  }
  fprintf (stdout, "Finnish sorting order:\n");
  for (i=mini; i<=maxi; i++) {
    int j, flag = 0;
    for (j=0; j<NCHARS; j++) {
      if (my_isalpha(j) && t[j] == i) {
        fprintf (stdout, "%c ", j);  
        flag = 1;
      }
    }
    if (flag) fprintf (stdout, "\n");
  }
/***
  fprintf (stdout, "size %d %d\n",
           sizeof(test)/sizeof(test[0]), sizeof(test[0]));
***/
  llqsort (test, sizeof(test)/sizeof(test[0]), sizeof(test[0]), cmp);

  fprintf (stdout, "Sort test.\n");
  for (i=0; i<NN; i++) {
    fprintf (stdout, "%d %s\n", i, test[i]);
  }

  fprintf (stdout, "Uppercase: ");
  for (i=0; i<NCHARS; i++) {
    if (my_isupper(i)) fprintf (stdout, "%c", i);
  }
  fprintf (stdout, "\nLowercase: ");
  for (i=0; i<NCHARS; i++) {
    if (my_islower(i) && (char)i != 'ß' && (char)i != 'ÿ') {
      fprintf (stdout, "%c", i);
    }
  }
  fprintf (stdout,
    "\nNote that ß and ÿ does not have an uppercase form in ISO Latin 1.\n");
  return 0;
}
#endif
/*==================================
 * appendstr -- Append to string, subject to length limit
 * Advance target pointer and decrease length.
 * Safe to call after length goes to zero (nothing happens).
 *  pdest:  [I/O] output buffer
 *  len:    [I/O] bytes remaining in output
 *  src:    [IN]  source to append
 * Created: 2000/11/29, Perry Rapp
 * NB: Need one byte for terminating zero, so len==1 is same as len==0.
 *================================*/
void
appendstr (char ** pdest, int * len, const char * src)
{
	int amount;
	*pdest[0]=0; /* so client doesn't have to initialize */
	if (*len<=1) { *len=0; return; }

	llstrncpy(*pdest, src, *len);
	amount = strlen(*pdest);
	*pdest += amount;
	*len -= amount;
	if (*len<=1) *len=0;
}
/*==================================
 * appendstrf -- sprintf style append to string,
 * subject to length limit
 * Advance target pointer and decrease length.
 * Safe to call after length goes to zero (nothing happens).
 *  pdest:  [I/O] output buffer
 *  len:    [I/O] bytes remaining in output
 *  fmt:    [IN]  sprintf style format string
 * Created: 2002/01/05, Perry Rapp
 * NB: Need one byte for terminating zero, so len==1 is same as len==0.
 *================================*/
void
appendstrf (char ** pdest, int * len, const char * fmt, ...)
{
	va_list args;
	int amount;
	*pdest[0]=0; /* so client doesn't have to initialize */
	if (*len<1) { *len=0; return; }

	va_start(args, fmt);
	vsnprintf(*pdest, *len-1, fmt, args);
	(*pdest)[*len-1]=0;
	va_end(args);

	amount = strlen(*pdest);
	*pdest += amount;
	*len -= amount;
	if (*len<1) *len=0;
}
/*==================================
 * llstrncpy -- strncpy that always zero-terminates
 * Created: 2001/03/17, Perry Rapp
 *================================*/
char *
llstrncpy (char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n-1);
	dest[n-1] = 0;
	return dest;
}
/*==========================================================
 * stdstring_hardfail -- Set programmer debugging mode
 *  to coredump if any wrong characters passed to character
 *  classification routines
 * Created: 2002/01/24 (Perry Rapp)
 *========================================================*/
void
stdstring_hardfail (void)
{
	hardfail = 1;
}
/*====================================================
 * safechar -- fix any bad sign-extended characters
 * Created: 2002/01/24 (Perry Rapp)
 *==================================================*/
static int
safechar (int c)
{
	if (hardfail) {
		ASSERT(c>0);
	}
	return (int)(unsigned char)(c);
}
