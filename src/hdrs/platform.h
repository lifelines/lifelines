/*  platform.h */
/* declarations for platform.c */

#ifndef platform_h_included
#define platform_h_included

#if defined(_WIN32) || defined(__CYGWIN__)
int w_get_codepage(void); /* Windows codepage */
int w_get_oemin_codepage(void); /* Console codepage */
int w_get_oemout_codepage(void); /* Console output codepage */
#endif

#endif /* platform_h_included */
