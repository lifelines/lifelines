/*
 * Author: Petter Reinholdtsen <pere@td.org.uit.no>
 * Date:   2000-09-24
 *
 * askprog.c -- Search for programs, parse info tags and present user
 * with list of programs.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "standard.h"
#include "table.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "screen.h"
#include "arch.h"

#include "llinesi.h"

#define PATHSEPARATOR ':'

/*=========================
 * The supported meta-tags.
 *=======================*/
#define PROGNAMETAG "@progname "
#define VERSIONTAG  "@version "
#define AUTHORTAG   "@author "
#define CATEGORYTAG "@category "
#define OUTPUTTAG   "@output "
/* Hm, this should be a multi-line tag.  How should I handle it? */
#define DESCTAG     "@description "

/* The program meta-info linked list entry */
struct program_info
{
  struct program_info *prev, *next;

  STRING filename;    /* Full path to script file */
  STRING progname;    /* The name of the script */
  STRING category;    /* The category of the script */
  STRING author;      /* The author of the script */
  STRING description; /* A description of purpose of the script */
  STRING output;      /* The output format produced by this script */
};

static STRING select_ext = NULL;

/*===========================================================
 * select_programs -- choose files with the correct extention
 *==========================================================*/
int
select_programs(const struct dirent *entry)
{
  int retval = 0;

  /* Compare extention with end of filename */
  if (0 == strcmp(select_ext,
                  entry->d_name + strlen(entry->d_name) - strlen(select_ext)))
    retval = 1;

  return retval;
}

/*==================================================================
 * remove_trailing_space -- terminate string at first trailing space
 *================================================================*/
void
remove_trailing_space(STRING s)
{
  int i;
  i = strlen(s)-1;
  while (0 < i)
    {
      if (!isspace(s[i]))
        break;
      s[i] = '\0';
      i--;
    }
}

/*==========================================================
 * parse_program -- Load program and locate metainformation.
 * Return struct with meta-info filled in.
 *========================================================*/
struct program_info *
parse_program(STRING directory,
              STRING filename)
{
  unsigned char filepath[MAXLINELEN];
  struct program_info *info;
  FILE *fp;
  unsigned char str[MAXLINELEN];

  sprintf(filepath, "%s/%s", directory, filename);

  if (NULL == (fp = fopen(filepath, "r")))
    return NULL;

  info = calloc(1, sizeof(*info));
  info->filename    = strdup(filepath);

  info->progname    = NULL;
  info->author      = NULL;
  info->description = NULL;
  info->output      = NULL;
  info->category    = NULL;

  while (NULL != fgets(str, sizeof(str), fp))
    {
      STRING p;

/* macro to handle single-line information */
#define GETTAG(tag, var) \
      if (NULL != (p = strstr(str, (tag)))) \
        { \
          STRING s = p + strlen((tag)); \
          while (isspace(*s)) s++; /* Skip leading space */ \
          remove_trailing_space(s); \
          (var) = strdup(s); \
         }

      GETTAG(PROGNAMETAG, info->progname);
      GETTAG(AUTHORTAG,   info->author);
      GETTAG(OUTPUTTAG,   info->output);
      GETTAG(CATEGORYTAG, info->category);

      /* XXX This should be a multi-line description */
      GETTAG(DESCTAG, info->description);

#undef GETTAG

    }

  fclose(fp);

  if (NULL == info->progname)
    info->progname = strdup(filename);

  return info;
}
/*===================================================================
 * load_programs -- Parse one directory, and add all programs to list
 *==================================================================*/
struct program_info *
load_programs(STRING directory,
              STRING ext,
              struct program_info *head)
{
  int n;
  struct dirent **programs;

  select_ext = ext;

  n = scandir(directory, &programs, select_programs, alphasort);
  if (0 > n)
    {
      perror("scandir");
      return head;
    }

  if (NULL == head)
    {
      head = calloc(1, sizeof(*head));
      head->next = NULL;
      head->prev = NULL;
    }

  while (n--)
    {
      struct program_info *info;
      info = parse_program(directory, programs[n]->d_name);
      if (NULL != info)
        {
          info->next = head->next;
          info->prev = head;
          head->next = info;
        }
      free(programs[n]);
    }
  free(programs);

  return head;
}

/*=========================================================
 * find_all_programs -- Load all programs in path into list
 *=======================================================*/
struct program_info *
find_all_programs(STRING path,
                  STRING ext)
{
  struct program_info *head = NULL;
  unsigned char buf[MAXLINELEN];
  int bufpos = 0;
  STRING p = path;

  if (NULL == path || '\0' == *path)
    return NULL;

  if (NULL == ext)
    ext = ".ll";

  while ('\0' != *p)
    {
      buf[bufpos++] = *p++;
      if ('\0' == *p || PATHSEPARATOR == *p)
        {
          buf[bufpos] = '\0';
          head = load_programs(buf, ext, head);
          bufpos = 0;
          if (PATHSEPARATOR == *p)
            p++;
        }
    }
  return head;
}

/*=========================================================
 * free_program_list -- Release memory used in program list
 *=======================================================*/
void
free_program_list(struct program_info *head,
                  STRING *list,
                  int len)
{
  struct program_info *info;
  while (NULL != head)
    {
      info = head;
      head = head->next;

      if (info->filename)
        free(info->filename);
      if (info->author)
        free(info->author);
      if (info->progname)
        free(info->progname);
      if (info->description)
        free(info->description);
      free(info);
    }

  while (len--)
    free(list[len]);
  free(list);
}

/*=====================================================================
 * make_program_list -- convert program list to menu (array of strings)
 *===================================================================*/
int
make_program_list(struct program_info *head,
                  STRING **list)
{
  int i;
  int len = 0;
  struct program_info *cur;
  STRING *newlist;

  if (NULL != *list || NULL == head || NULL == head->next)
    return 0;

  /* How many do we have.. */
  cur = head->next;
  while (NULL != cur)
    {
      len++;
      cur = cur->next;
    }

  if (0 >= len)
    return 0;

  newlist = malloc(sizeof(char*)*len);
  assert(NULL != newlist);
  cur = head->next;
  i = 0;
  while (NULL != cur)
    {
      unsigned char buf[MAXLINELEN];
#if defined( WIN32) && defined(_MSC_VER)
      _snprintf(buf, sizeof(buf), "%s (%s)",
              NULL != cur->progname    ? cur->progname       : cur->filename,
              NULL != cur->output      ? cur->output      : (STRING)"[?]");
#else
      snprintf(buf, sizeof(buf), "%s (%s)",
              NULL != cur->progname    ? cur->progname       : cur->filename,
              NULL != cur->output      ? cur->output      : (STRING)"[?]");
#endif
      newlist[i] = strdup(buf);
      i++;
      cur = cur->next;
    }
  *list = newlist;
  return len;
}

/*==================================================
 * get_choice -- Return file pointer for menu choice
 *================================================*/
FILE *
get_choice(struct program_info *head,
           int choice,
           STRING *pfname)
{
  struct program_info *cur;
  FILE *fp;
  cur = head->next;
  while (NULL != cur && choice)
    {
      cur = cur->next;
      choice--;
    }
  if (0 != choice)
    return NULL;

  fp = fopen(cur->filename, "r");

  /*
   * The static string is ugly, but apparently this is how it is done
   * in ask_for_string()
   */
  if (NULL != fp && cur->filename && pfname)
    {
      static unsigned char fname[MAXLINELEN];
      strncpy(fname, cur->filename,sizeof(fname)-1);
      fname[sizeof(fname)-1] = '\0';
      *pfname = fname;
    }

  return fp;
}

/*===================================================
 * ask_for_program -- Ask for and open program script
 *=================================================*/
FILE *
ask_for_program (STRING mode,
                 STRING ttl,
                 STRING *pfname,
                 STRING path,
                 STRING ext)
{
  FILE *fp = NULL;
  int len, choice;
  struct program_info *head = find_all_programs(path, ext);
  STRING *list = NULL;
  len = make_program_list(head, &list);
  if (0 == len)
    message("No scripts found in LLPROGRAMS path");
  else
    {
      choice = choose_from_list(ttl, len, list);
      if (choice < 0)
        {
          message("Nothing choosen?");
          return NULL;
        }
      fp = get_choice(head, choice, pfname);
      free_program_list(head, list, len);
    }
  return fp;
}
