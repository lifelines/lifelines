/*
 * Author: Petter Reinholdtsen <pere@td.org.uit.no>
 * Date:   2000-09-24
 *
 * askprog.c -- Search for programs, parse info tags and present user
 * with list of programs.
 *
 * NB: This file uses spaces only for indent, and uses a different curly
 * braces style than the rest of LifeLines. Please respect.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "llstdlib.h"
#include "table.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "feedback.h"
#include "arch.h"
#include "lloptions.h"

#include "llinesi.h"

typedef struct program_info_s *PROGRAM_INFO;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
FILE *ask_for_program (STRING mode,
                       STRING ttl,
                       STRING *pfname,
                       STRING path,
                       STRING ext,
                       BOOLEAN picklist);
PROGRAM_INFO find_all_programs(STRING path, STRING ext);
void free_program_list(PROGRAM_INFO head, STRING *list, int len);
PROGRAM_INFO get_choice(PROGRAM_INFO head, int choice);
PROGRAM_INFO load_programs(STRING directory,
                           STRING ext,
                           PROGRAM_INFO head);
int make_program_list(PROGRAM_INFO head, STRING leader, STRING **list);
PROGRAM_INFO parse_program(STRING directory, STRING filename);
static void progdetails(ARRAY_DETAILS arrdets, void * param);
void remove_trailing_space(STRING s);
int select_programs(const struct dirent *entry);

/* messages used */
extern STRING extrpt, whatrpt;

/*=========================
 * The supported meta-tags.
 *=======================*/

enum { P_PROGNAME=0, P_VERSION=1, P_OUTPUT=4 };

static CNSTRING f_tags[] = {
  "progname"    /* The name of the script */
  , "version"     /* The version of the script */
  , "author"      /* The author of the script */
  , "category"    /* The category of the script */
  , "output"      /* The output format produced by this script */
  , "description" /* A description of purpose of the script */
};

/* The program meta-info linked list entry */
struct program_info_s
{
  PROGRAM_INFO prev, next;

  STRING filename;    /* Full path to script file */
  STRING tags[ARRSIZE(f_tags)];

};

static STRING select_ext = NULL;

/*===========================================================
 * select_programs -- choose files with the correct extention
 *==========================================================*/
int
select_programs(const struct dirent *entry)
{
  int retval = 0;
  /* examine end of entry->d_name */
  CNSTRING entext = entry->d_name + strlen(entry->d_name) - strlen(select_ext);

  /* is it what we want ? use platform correct comparison, from path.c */
  if (path_match(select_ext, entext))
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
PROGRAM_INFO
parse_program(STRING directory,
              STRING filename)
{
  char filepath[MAXLINELEN];
  PROGRAM_INFO info;
  FILE *fp;
  char str[MAXLINELEN];
  int i;

  sprintf(filepath, "%s/%s", directory, filename);

  if (NULL == (fp = fopen(filepath, "r")))
    return NULL;

  info = calloc(1, sizeof(*info));
  info->filename = strdup(filepath);
  for (i=0; i<ARRSIZE(info->tags); ++i)
    info->tags[i] = "";

  while (NULL != fgets(str, sizeof(str), fp))
    {
      STRING p;
      chomp(str); /* trim trailing CR or LF */
      for (i=0; i<ARRSIZE(f_tags); ++i)
        {
          CNSTRING tag = f_tags[i];
          if (info->tags[i][0])
            continue; /* already have this tag */
          if (NULL != (p = strstr(str, tag)))
            {
              STRING s = p + strlen(tag);
              /* skip leading space */
              while (s[0] && isspace((uchar)s[0]))
                ++s;
              remove_trailing_space(s);
              if (s[0])
                info->tags[i] = strdup(s);
              break;
            }
        }
          if (strstr(str, "*/"))
            break;
    }

  fclose(fp);

  /* ensure we always have a program name */
  if (!info->tags[P_PROGNAME][0]) 
    info->tags[P_PROGNAME] = strdup(filename);

  return info;
}
/*===================================================================
 * load_programs -- Parse one directory, and add all programs to list
 *==================================================================*/
PROGRAM_INFO
load_programs(STRING directory,
              STRING ext,
              PROGRAM_INFO head)
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
      PROGRAM_INFO info;
      info = parse_program(directory, programs[n]->d_name);
      if (NULL != info)
        {
          info->next = head->next;
          info->prev = head;
          head->next = info;
        }
      stdfree(programs[n]);
    }
  stdfree(programs);

  return head;
}

/*=========================================================
 * find_all_programs -- Load all programs in path into list
 *=======================================================*/
PROGRAM_INFO
find_all_programs(STRING path,
                  STRING ext)
{
  PROGRAM_INFO head = NULL;
  char buf[MAXLINELEN];
  int bufpos = 0;
  STRING p = path;

  if (NULL == path || '\0' == *path)
    return NULL;

  if (NULL == ext)
    ext = ".ll";

  while ('\0' != *p)
    {
      buf[bufpos++] = *p++;
      if ('\0' == *p || LLCHRPATHSEPARATOR == *p)
        {
          buf[bufpos] = '\0';
          head = load_programs(buf, ext, head);
          bufpos = 0;
          if (LLCHRPATHSEPARATOR == *p)
            p++;
        }
    }
  return head;
}

/*=========================================================
 * free_program_list -- Release memory used in program list
 *=======================================================*/
void
free_program_list(PROGRAM_INFO head,
                  STRING *list,
                  int len)
{
  PROGRAM_INFO info;
  while (NULL != head)
    {
      INT i;
      info = head;
      head = head->next;

      stdfree(info->filename);
      for (i=0; i<ARRSIZE(info->tags); ++i)
        {
          if (info->tags[i] && info->tags[i][0])
            stdfree(info->tags[i]);
        }
      stdfree(info);
    }
  if (list)
    {
      while (len--)
        {
          stdfree(list[len]);
        }
      stdfree(list);
    }
}

/*=====================================================================
 * make_program_list -- convert program list to menu (array of strings)
 * head is the linked list of programs
 * leader is an optional special first line (NULL if not desired)
 * list is the output - newly created list
 *===================================================================*/
int
make_program_list(PROGRAM_INFO head,
                  STRING leader,
                  STRING **list)
{
  int i;
  int len = 0;
  PROGRAM_INFO cur;
  STRING *newlist;

  if (!list || *list || !head)
    return 0; /* error by caller */

  /* How many do we have.. */
  cur = head->next;
  len = leader ? 1 : 0;
  while (NULL != cur)
    {
      len++;
      cur = cur->next;
    }

  if (!len)
    return 0;

  newlist = stdalloc(sizeof(char*)*len);
  assert(NULL != newlist);
  cur = head->next;
  i = 0;
  if (leader)
    {
      newlist[i++] = strdup(leader);
    }
  while (NULL != cur)
    {
      char buf[MAXLINELEN];
      STRING program = cur->tags[P_PROGNAME];
      STRING version = cur->tags[P_VERSION];
      STRING output = cur->tags[P_OUTPUT];
      if (!output)
        output = "?";
      if (!version[0])
        version = "V?.?";
      snprintf(buf, sizeof(buf), "%s (%s) [%s]"
        , program, version, output);
      newlist[i] = strdup(buf);
      i++;
      cur = cur->next;
    }
  *list = newlist;
  return len;
}
/*==================================================
 * get_choice -- Return file pointer for menu choice
 * fill in pfname unless internal error
 * return non-null FILE if file could be opened
 * so if file could not be opened, pfname still can tell us
 *  what file the user tried & failed to open
 *================================================*/
PROGRAM_INFO
get_choice (PROGRAM_INFO head, int choice)
{
  PROGRAM_INFO cur = head->next;
  while (NULL != cur && choice)
    {
      cur = cur->next;
      choice--;
    }
  if (0 != choice)
    {
      return NULL;
    }
  return cur;
}

/*===================================================
 * ask_for_program -- Ask for and open program script
 *=================================================*/
FILE *
ask_for_program (STRING mode,
                 STRING ttl,
                 STRING *pfname,
                 STRING path,
                 STRING ext,
                 BOOLEAN picklist)
{
  FILE *fp = NULL;
  int len, choice;
  PROGRAM_INFO head;
  STRING *list = NULL;
  STRING programsdir = getoptstr("LLPROGRAMS", ".");
  static char fname[MAXLINELEN]; /* for returning path */

  if (!picklist)
    {
      goto AskForString;
    }

  head = find_all_programs(path, ext);
  if (!head)
    {
      goto AskForString;
    }
  if (!head->next)
    {
      free_program_list(head, NULL, 0);
      goto AskForString;
    }
  len = make_program_list(head, extrpt, &list);
  if (!len)
    {
      goto AskForString;
    }
  else
    {
      PROGRAM_INFO cur=0;
      choice = choose_from_array_x(ttl, len, list, progdetails, head);
      if (choice == -1)
        {
          free_program_list(head, list, len);
          message("Cancelled.");
          return NULL;
        }
      if (choice == 0)
        {
          /* 0th choice is to go to AskForString prompt instead */
          free_program_list(head, list, len);
          goto AskForString;
        }
      choice--;
      /* Try to open user's choice */
      cur = get_choice(head, choice);
      if (cur)
        {
          fp = fopen(cur->filename, mode);
          /* give path to caller, whether or not we succeeded */
          strncpy(fname, cur->filename,sizeof(fname)-1);
          fname[sizeof(fname)-1] = '\0';
          *pfname = fname;
        }
      free_program_list(head, list, len);
    }
  return fp;

AskForString:
  fp = ask_for_input_file(LLREADTEXT, whatrpt, pfname, programsdir, ".ll");
  return fp;
}
/*================================================
 * progdetails -- print details of a program
 * Callback from choose_from_array_x
 *  arrdets:  [IN]  package of list & current status
 *  param:    [IN]  the param we passed when we called choose_from_array_x
 * Created: 2001/12/15 (Perry Rapp)
 *==============================================*/
static void
progdetails (ARRAY_DETAILS arrdets, void * param)
{
  PROGRAM_INFO head = (PROGRAM_INFO)param;
  PROGRAM_INFO cur=0;
  INT count = arrdets->count;
  INT len = arrdets->maxlen;
  INT index = arrdets->cur - 1; /* 0 slot taken by choose string */
  INT row=0;
  INT scroll = arrdets->scroll;
  INT i;

  if (index<0) return;

  cur = get_choice(head, index);

  /* print tags & values */
  for (i=scroll; i<ARRSIZE(cur->tags); ++i)
    {
      STRING ptr = arrdets->lines[row];
      INT mylen = len;
      llstrcatn(&ptr, f_tags[i], &mylen);
      llstrcatn(&ptr, ": ", &mylen);
      llstrcatn(&ptr, cur->tags[i], &mylen);
      ++row;
      if (row==count)
        return;
    }
  /* TODO: read file header */
}
