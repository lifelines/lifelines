#ifndef mychar_h_included
#define mychar_h_included


typedef struct my_charset_info_tag {
  unsigned char toup;    /* Corresponding uppercase letter. */
  unsigned char tolow;   /* Corresponding lowercase letter. */
  unsigned char iscntrl; /* Is control character? */
  unsigned char isup;    /* Is uppercase letter?  */
  unsigned char islow;   /* Is lowercase letter?  */
} my_charset_info;

extern my_charset_info ISO_Latin1[];

int mych_isalpha(const int c);
int mych_iscntrl(const int c);
int mych_islower(const int c);
int mych_isprint(const int c);
int mych_isupper(const int c);
void mych_set_table(my_charset_info * charset_table);
int mych_tolower(const int c);
int mych_toupper(const int c);

#endif /* mychar_h_included */
