/* Install modified versions of certain ANSI-incompatible system header
   files which are fixed to work correctly with ANSI C and placed in a
   directory that GNU C will search.

   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "fixlib.h"

#if defined( HAVE_MMAP_FILE )
#include <sys/mman.h>
#define  BAD_ADDR ((void*)-1)
#endif

#include <signal.h>
#if ! defined( SIGCHLD ) && defined( SIGCLD )
#  define SIGCHLD SIGCLD
#endif
#ifndef SEPARATE_FIX_PROC
#include "server.h"
#endif

/*  The contents of this string are not very important.  It is mostly
    just used as part of the "I am alive and working" test.  */

static const char program_id[] = "fixincl version 1.1";

/*  This format will be used at the start of every generated file */

static const char z_std_preamble[] =
"/*  DO NOT EDIT THIS FILE.\n\n\
    It has been auto-edited by fixincludes from:\n\n\
\t\"%s/%s\"\n\n\
    This had to be done to correct non-standard usages in the\n\
    original, manufacturer supplied header file.  */\n\n";

/*  Working environment strings.  Essentially, invocation 'options'.  */

#define _ENV_(v,m,n,t)   tCC* v = NULL;
ENV_TABLE
#undef _ENV_

int find_base_len = 0;

typedef enum {
  VERB_SILENT = 0,
  VERB_FIXES,
  VERB_APPLIES,
  VERB_PROGRESS,
  VERB_TESTS,
  VERB_EVERYTHING
} te_verbose;

te_verbose  verbose_level = VERB_PROGRESS;
int have_tty = 0;

#define VLEVEL(l)  ((unsigned int) verbose_level >= (unsigned int) l)
#define NOT_SILENT VLEVEL(VERB_FIXES)

pid_t process_chain_head = (pid_t) -1;

char*  pz_curr_file;  /*  name of the current file under test/fix  */
char*  pz_curr_data;  /*  original contents of that file  */
char*  pz_temp_file;  /*  for DOS, a place to stash the temporary
                          fixed data between system(3) calls  */
t_bool curr_data_mapped;
int    data_map_fd;
size_t data_map_size;
size_t ttl_data_size = 0;

#ifdef DO_STATS
int process_ct = 0;
int apply_ct = 0;
int fixed_ct = 0;
int altered_ct = 0;
#endif /* DO_STATS */

const char incl_quote_pat[] = "^[ \t]*#[ \t]*include[ \t]*\"[^/]";
tSCC z_fork_err[] = "Error %d (%s) starting filter process for %s\n";
regex_t incl_quote_re;

static void do_version   PARAMS((void)) ATTRIBUTE_NORETURN;
char *load_file   PARAMS((const char *));
void run_compiles PARAMS((void));
void initialize   PARAMS((int argc,char** argv));
void process      PARAMS((void));

/*  External Source Code */

#include "fixincl.x"

/* * * * * * * * * * * * * * * * * * *
 *
 *  MAIN ROUTINE
 */
extern int main PARAMS ((int, char **));
int
main (argc, argv)
     int argc;
     char **argv;
{
  char *file_name_buf;

  initialize ( argc, argv );

  have_tty = isatty (fileno (stderr));

  /* Before anything else, ensure we can allocate our file name buffer. */
  file_name_buf = load_file_data (stdin);

  /*  Because of the way server shells work, you have to keep stdin, out
      and err open so that the proper input file does not get closed
      by accident  */

  freopen ("/dev/null", "r", stdin);

  if (file_name_buf == (char *) NULL)
    {
      fputs ("No file names listed for fixing\n", stderr);
      exit (EXIT_FAILURE);
    }

  for (;;)
    {
      char* pz_end;

      /*  skip to start of name, past any "./" prefixes */

      while (ISSPACE (*file_name_buf))  file_name_buf++;
      while ((file_name_buf[0] == '.') && (file_name_buf[1] == '/'))
        file_name_buf += 2;

      /*  Check for end of list  */

      if (*file_name_buf == NUL)
        break;

      /*  Set global file name pointer and find end of name */

      pz_curr_file = file_name_buf;
      pz_end = strchr( pz_curr_file, '\n' );
      if (pz_end == (char*)NULL)
        pz_end = file_name_buf = pz_curr_file + strlen (pz_curr_file);
      else
        file_name_buf = pz_end + 1;

      while ((pz_end > pz_curr_file) && ISSPACE( pz_end[-1]))  pz_end--;

      /*  IF no name is found (blank line) or comment marker, skip line  */

      if ((pz_curr_file == pz_end) || (*pz_curr_file == '#'))
        continue;
      *pz_end = NUL;

      process ();
    } /*  for (;;) */

#ifdef DO_STATS
  if (VLEVEL( VERB_PROGRESS )) {
    tSCC zFmt[] =
      "\
Processed %5d files containing %d bytes    \n\
Applying  %5d fixes to %d files\n\
Altering  %5d of them\n";

    fprintf (stderr, zFmt, process_ct, ttl_data_size, apply_ct,
             fixed_ct, altered_ct);
  }
#endif /* DO_STATS */

# ifdef SEPARATE_FIX_PROC
  unlink( pz_temp_file );
# endif
  exit (EXIT_SUCCESS);
}


static void
do_version ()
{
  static const char zFmt[] = "echo '%s'";
  char zBuf[ 1024 ];

  /* The 'version' option is really used to test that:
     1.  The program loads correctly (no missing libraries)
     2.  that we can compile all the regular expressions.
     3.  we can correctly run our server shell process
  */
  run_compiles ();
  sprintf (zBuf, zFmt, program_id);
#ifndef SEPARATE_FIX_PROC
  puts (zBuf + 5);
  exit (strcmp (run_shell (zBuf), program_id));
#else
  exit (system (zBuf));
#endif
}

/* * * * * * * * * * * * */

void
initialize ( argc, argv )
  int argc;
  char** argv;
{
  static const char var_not_found[] =
#ifndef __STDC__
    "fixincl ERROR:  %s environment variable not defined\n"
#else
    "fixincl ERROR:  %s environment variable not defined\n"
    "each of these must be defined:\n"
# define _ENV_(vv,mm,nn,tt) "\t" nn "  - " tt "\n"
  ENV_TABLE
# undef _ENV_
#endif
    ;

  xmalloc_set_program_name (argv[0]);

  switch (argc)
    {
    case 1:
      break;

    case 2:
      if (strcmp (argv[1], "-v") == 0)
        do_version ();
      if (freopen (argv[1], "r", stdin) == (FILE*)NULL)
        {
          fprintf (stderr, "Error %d (%s) reopening %s as stdin\n",
                   errno, xstrerror (errno), argv[1] );
          exit (EXIT_FAILURE);
        }
      break;

    default:
      fputs ("fixincl ERROR:  too many command line arguments\n", stderr);
      exit (EXIT_FAILURE);
    }

#ifdef SIGCHLD
  /* We *MUST* set SIGCHLD to SIG_DFL so that the wait4() call will
     receive the signal.  A different setting is inheritable */
  signal (SIGCHLD, SIG_DFL);
#endif

#define _ENV_(v,m,n,t)   { tSCC var[] = n;  \
  v = getenv (var); if (m && (v == NULL)) { \
  fprintf (stderr, var_not_found, var);     \
  exit (EXIT_FAILURE); } }

ENV_TABLE

#undef _ENV_

  if (ISDIGIT ( *pz_verbose ))
    verbose_level = (te_verbose)atoi( pz_verbose );
  else
    switch (*pz_verbose) {
    case 's':
    case 'S':
      verbose_level = VERB_SILENT;     break;

    case 'f':
    case 'F':
      verbose_level = VERB_FIXES;      break;

    case 'a':
    case 'A':
      verbose_level = VERB_APPLIES;    break;

    case 'p':
    case 'P':
      verbose_level = VERB_PROGRESS;   break;

    case 't':
    case 'T':
      verbose_level = VERB_TESTS;      break;

    case 'e':
    case 'E':
      verbose_level = VERB_EVERYTHING; break;
    }

 while ((pz_find_base[0] == '.') && (pz_find_base[1] == '/'))
   pz_find_base += 2;
 if ((pz_find_base[0] != '.') || (pz_find_base[1] != NUL))
   find_base_len = strlen( pz_find_base );

  /*  Compile all the regular expressions now.
      That way, it is done only once for the whole run.
      */
  run_compiles ();

# ifdef SEPARATE_FIX_PROC
  /* NULL as the first argument to `tempnam' causes it to DTRT
     wrt the temporary directory where the file will be created.  */
  pz_temp_file = tempnam( NULL, "fxinc" );
# endif

  signal (SIGQUIT, SIG_IGN);
#ifdef SIGIOT
  signal (SIGIOT,  SIG_IGN);
#endif
#ifdef SIGPIPE
  signal (SIGPIPE, SIG_IGN);
#endif
  signal (SIGALRM, SIG_IGN);
  signal (SIGTERM, SIG_IGN);
}

/* * * * * * * * * * * * *

   load_file loads all the contents of a file into malloc-ed memory.
   Its argument is the name of the file to read in; the returned
   result is the NUL terminated contents of the file.  The file
   is presumed to be an ASCII text file containing no NULs.  */
char *
load_file ( fname )
    const char* fname;
{
  struct stat stbf;
  char* res;

  if (stat (fname, &stbf) != 0)
    {
      if (NOT_SILENT)
        fprintf (stderr, "error %d (%s) stat-ing %s\n",
                 errno, xstrerror (errno), fname );
      return (char *) NULL;
    }
  if (stbf.st_size == 0)
    return (char*)NULL;

  /*  Make the data map size one larger than the file size for documentation
      purposes.  Truth is that there will be a following NUL character if
      the file size is not a multiple of the page size.  If it is a multiple,
      then this adjustment sometimes fails anyway.  */
  data_map_size = stbf.st_size+1;
  data_map_fd   = open (fname, O_RDONLY);
  ttl_data_size += data_map_size-1;

  if (data_map_fd < 0)
    {
      if (NOT_SILENT)
        fprintf (stderr, "error %d (%s) opening %s for read\n",
                 errno, xstrerror (errno), fname);
      return (char*)NULL;
    }

#ifdef HAVE_MMAP_FILE
  curr_data_mapped = BOOL_TRUE;

  /*  IF the file size is a multiple of the page size,
      THEN sometimes you will seg fault trying to access a trailing byte */
  if ((stbf.st_size & (getpagesize()-1)) == 0)
    res = (char*)BAD_ADDR;
  else
    res = (char*)mmap ((void*)NULL, data_map_size, PROT_READ,
                       MAP_PRIVATE, data_map_fd, 0);
  if (res == (char*)BAD_ADDR)
#endif
    {
      FILE* fp = fdopen (data_map_fd, "r");
      curr_data_mapped = BOOL_FALSE;
      res = load_file_data (fp);
      fclose (fp);
    }

  return res;
}

static int machine_matches PARAMS ((tFixDesc *));
static int
machine_matches( p_fixd )
  tFixDesc *p_fixd;
        {
# ifndef SEPARATE_FIX_PROC
          tSCC case_fmt[] = "case %s in\n";     /*  9 bytes, plus string */
          tSCC esac_fmt[] =
               " )\n    echo %s ;;\n* ) echo %s ;;\nesac";/*  4 bytes */
          tSCC skip[] = "skip";                 /*  4 bytes */
          tSCC run[] = "run";                   /*  3 bytes */
          /* total bytes to add to machine sum:    49 - see fixincl.tpl */

          const char **papz_machs = p_fixd->papz_machs;
          char *pz;
          const char *pz_sep = "";
          tCC *pz_if_true;
          tCC *pz_if_false;
          char cmd_buf[ MACH_LIST_SIZE_LIMIT ]; /* size lim from fixincl.tpl */

          /* Start the case statement */

          sprintf (cmd_buf, case_fmt, pz_machine);
          pz = cmd_buf + strlen (cmd_buf);

          /*  Determine if a match means to apply the fix or not apply it */

          if (p_fixd->fd_flags & FD_MACH_IFNOT)
            {
              pz_if_true  = skip;
              pz_if_false = run;
            }
          else
            {
              pz_if_true  = run;
              pz_if_false = skip;
            }

          /*  Emit all the machine names.  If there are more than one,
              then we will insert " | \\\n" between the names  */

          for (;;)
            {
              const char* pz_mach = *(papz_machs++);

              if (pz_mach == (const char*) NULL)
                break;
              sprintf (pz, "%s%s", pz_sep, pz_mach);
              pz += strlen (pz);
              pz_sep = " | \\\n";
            }

          /* Now emit the match and not-match actions and the esac */

          sprintf (pz, esac_fmt, pz_if_true, pz_if_false);

          /*  Run the script.
              The result will start either with 's' or 'r'.  */

          {
            int skip;
            pz = run_shell (cmd_buf);
            skip = (*pz == 's');
            free ( (void*)pz );
            if (skip)
              {
                p_fixd->fd_flags |= FD_SKIP_TEST;
		return BOOL_FALSE;
	      }
	  }

  return BOOL_TRUE;
# else /* is SEPARATE_FIX_PROC */
  const char **papz_machs = p_fixd->papz_machs;
  int invert = (p_fixd->fd_flags & FD_MACH_IFNOT) != 0;
  for (;;)
    {
      const char* pz_mach = *(papz_machs++);

      if (pz_mach == (const char*) NULL)
        break;
      if (strstr (pz_mach, "dos") != NULL && !invert)
	return BOOL_TRUE;
    }

  p_fixd->fd_flags |= FD_SKIP_TEST;
  return BOOL_FALSE;
# endif
}

/* * * * * * * * * * * * *

   run_compiles   run all the regexp compiles for all the fixes once.
   */
void
run_compiles ()
{
  tFixDesc *p_fixd = fixDescList;
  int fix_ct = FIX_COUNT;
  regex_t *p_re = (regex_t *) xmalloc (REGEX_COUNT * sizeof (regex_t));

  /*  Make sure compile_re does not stumble across invalid data */

  memset ( (void*)p_re, '\0', REGEX_COUNT * sizeof (regex_t) );
  memset ( (void*)&incl_quote_re, '\0', sizeof (regex_t) );

  compile_re (incl_quote_pat, &incl_quote_re, 1,
              "quoted include", "run_compiles");

  /*  Allow machine name tests to be ignored (testing, mainly) */

  if (pz_machine && ((*pz_machine == '\0') || (*pz_machine == '*')))
    pz_machine = (char*)NULL;

  /* FOR every fixup, ...  */
  do
    {
      tTestDesc *p_test = p_fixd->p_test_desc;
      int test_ct = p_fixd->test_ct;

      /*  IF the machine type pointer is not NULL (we are not in test mode)
             AND this test is for or not done on particular machines
          THEN ...   */

      if (  (pz_machine != NULL)
         && (p_fixd->papz_machs != (const char**) NULL)
         && ! machine_matches (p_fixd) )
        continue;

      /* FOR every test for the fixup, ...  */

      while (--test_ct >= 0)
        {
          switch (p_test->type)
            {
            case TT_EGREP:
            case TT_NEGREP:
              p_test->p_test_regex = p_re++;
              compile_re (p_test->pz_test_text, p_test->p_test_regex, 0,
                          "select test", p_fixd->fix_name);
            default: break;
            }
          p_test++;
        }
    }
  while (p_fixd++, --fix_ct > 0);
}


/* * * * * * * * * * * * *

   create_file  Create the output modified file.
   Input:    the name of the file to create
   Returns:  a file pointer to the new, open file  */

#if defined(S_IRUSR) && defined(S_IWUSR) && \
    defined(S_IRGRP) && defined(S_IROTH)

#   define S_IRALL (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#else
#   define S_IRALL 0644
#endif

#if defined(S_IRWXU) && defined(S_IRGRP) && defined(S_IXGRP) && \
    defined(S_IROTH) && defined(S_IXOTH)

#   define S_DIRALL (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#else
#   define S_DIRALL 0755
#endif


static FILE *create_file PARAMS ((void));
static FILE *
create_file ()
{
  int fd;
  FILE *pf;
  char fname[MAXPATHLEN];

  sprintf (fname, "%s/%s", pz_dest_dir, pz_curr_file + find_base_len);

  fd = open (fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRALL);

  /*  We may need to create the directories needed... */
  if ((fd < 0) && (errno == ENOENT))
    {
      char *pz_dir = strchr (fname + 1, '/');
      struct stat stbf;

      while (pz_dir != (char *) NULL)
        {
          *pz_dir = NUL;
          if (stat (fname, &stbf) < 0)
            {
              mkdir (fname, S_IFDIR | S_DIRALL);
            }

          *pz_dir = '/';
          pz_dir = strchr (pz_dir + 1, '/');
        }

      /*  Now, lets try the open again... */
      fd = open (fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRALL);
    }
  if (fd < 0)
    {
      fprintf (stderr, "Error %d (%s) creating %s\n",
               errno, xstrerror (errno), fname);
      exit (EXIT_FAILURE);
    }
  if (NOT_SILENT)
    fprintf (stderr, "Fixed:  %s\n", pz_curr_file);
  pf = fdopen (fd, "w");

  /*
   *  IF pz_machine is NULL, then we are in some sort of test mode.
   *  Do not insert the current directory name.  Use a constant string.
   */
  fprintf (pf, z_std_preamble,
           (pz_machine == NULL)
           ? "fixinc/tests/inc"
           : pz_input_dir,
           pz_curr_file);

  return pf;
}


/* * * * * * * * * * * * *

  test_test   make sure a shell-style test expression passes.
  Input:  a pointer to the descriptor of the test to run and
          the name of the file that we might want to fix
  Result: APPLY_FIX or SKIP_FIX, depending on the result of the
          shell script we run.  */
#ifndef SEPARATE_FIX_PROC
static int test_test PARAMS ((tTestDesc *, char *));
static int
test_test (p_test, pz_test_file)
     tTestDesc *p_test;
     char*      pz_test_file;
{
  tSCC cmd_fmt[] =
"file=%s\n\
if ( test %s ) > /dev/null 2>&1\n\
then echo TRUE\n\
else echo FALSE\n\
fi";

  char *pz_res;
  int res;

  static char cmd_buf[4096];

  sprintf (cmd_buf, cmd_fmt, pz_test_file, p_test->pz_test_text);
  pz_res = run_shell (cmd_buf);

  switch (*pz_res) {
  case 'T':
    res = APPLY_FIX;
    break;

  case 'F':
    res = SKIP_FIX;
    break;

  default:
    fprintf (stderr, "Script yielded bogus result of `%s':\n%s\n\n",
             pz_res, cmd_buf );
  }

  free ((void *) pz_res);
  return res;
}
#else
/*
 *  IF we are in MS-DOS land, then whatever shell-type test is required
 *  will, by definition, fail
 */
#define test_test(t,tf)  SKIP_FIX
#endif

/* * * * * * * * * * * * *

  egrep_test   make sure an egrep expression is found in the file text.
  Input:  a pointer to the descriptor of the test to run and
          the pointer to the contents of the file under suspicion
  Result: APPLY_FIX if the pattern is found, SKIP_FIX otherwise

  The caller may choose to reverse meaning if the sense of the test
  is inverted.  */

static int egrep_test PARAMS ((char *, tTestDesc *));
static int
egrep_test (pz_data, p_test)
     char *pz_data;
     tTestDesc *p_test;
{
#ifdef DEBUG
  if (p_test->p_test_regex == 0)
    fprintf (stderr, "fixincl ERROR RE not compiled:  `%s'\n",
             p_test->pz_test_text);
#endif
  if (regexec (p_test->p_test_regex, pz_data, 0, 0, 0) == 0)
    return APPLY_FIX;
  return SKIP_FIX;
}


/* * * * * * * * * * * * *

  quoted_file_exists  Make sure that a file exists before we emit
  the file name.  If we emit the name, our invoking shell will try
  to copy a non-existing file into the destination directory.  */

static int quoted_file_exists PARAMS ((const char *, const char *, const char *));
static int
quoted_file_exists (pz_src_path, pz_file_path, pz_file)
     const char *pz_src_path;
     const char *pz_file_path;
     const char *pz_file;
{
  char z[ MAXPATHLEN ];
  char* pz;
  sprintf (z, "%s/%s/", pz_src_path, pz_file_path);
  pz = z + strlen ( z );

  for (;;) {
    char ch = *pz_file++;
    if (! ISGRAPH( ch ))
      return 0;
    if (ch == '"')
      break;
    *pz++ = ch;
  }
  *pz = '\0';
  {
    struct stat s;
    if (stat (z, &s) != 0)
      return 0;
    return S_ISREG( s.st_mode );
  }
}


/* * * * * * * * * * * * *
 *
   extract_quoted_files

   The syntax, `#include "file.h"' specifies that the compiler is to
   search the local directory of the current file before the include
   list.  Consequently, if we have modified a header and stored it in
   another directory, any files that are included by that modified
   file in that fashion must also be copied into this new directory.
   This routine finds those flavors of #include and for each one found
   emits a triple of:

    1.  source directory of the original file
    2.  the relative path file name of the #includ-ed file
    3.  the full destination path for this file

   Input:  the text of the file, the file name and a pointer to the
           match list where the match information was stored.
   Result: internally nothing.  The results are written to stdout
           for interpretation by the invoking shell  */


static void extract_quoted_files PARAMS ((char *, const char *, regmatch_t *));
static void
extract_quoted_files (pz_data, pz_fixed_file, p_re_match)
     char *pz_data;
     const char *pz_fixed_file;
     regmatch_t *p_re_match;
{
  char *pz_dir_end = strrchr (pz_fixed_file, '/');
  char *pz_incl_quot = pz_data;

  if (VLEVEL( VERB_APPLIES ))
    fprintf (stderr, "Quoted includes in %s\n", pz_fixed_file);

  /*  Set "pz_fixed_file" to point to the containing subdirectory of the source
      If there is none, then it is in our current directory, ".".   */

  if (pz_dir_end == (char *) NULL)
    pz_fixed_file = ".";
  else
    *pz_dir_end = '\0';

  for (;;)
    {
      pz_incl_quot += p_re_match->rm_so;

      /*  Skip forward to the included file name */
      while (*pz_incl_quot != '"')
        pz_incl_quot++;

      if (quoted_file_exists (pz_src_dir, pz_fixed_file, pz_incl_quot))
        {
          /* Print the source directory and the subdirectory
             of the file in question.  */
          printf ("%s  %s/", pz_src_dir, pz_fixed_file);
          pz_dir_end = pz_incl_quot;

          /* Append to the directory the relative path of the desired file */
          while (*pz_incl_quot != '"')
            putc (*pz_incl_quot++, stdout);

          /* Now print the destination directory appended with the
             relative path of the desired file */
          printf ("  %s/%s/", pz_dest_dir, pz_fixed_file);
          while (*pz_dir_end != '"')
            putc (*pz_dir_end++, stdout);

          /* End of entry */
          putc ('\n', stdout);
        }

      /* Find the next entry */
      if (regexec (&incl_quote_re, pz_incl_quot, 1, p_re_match, 0) != 0)
        break;
    }
}


/* * * * * * * * * * * * *

    Somebody wrote a *_fix subroutine that we must call.
    */
#ifndef SEPARATE_FIX_PROC
static int internal_fix PARAMS ((int, tFixDesc *));
static int
internal_fix (read_fd, p_fixd)
  int read_fd;
  tFixDesc* p_fixd;
{
  int fd[2];

  if (pipe( fd ) != 0)
    {
      fprintf (stderr, "Error %d on pipe(2) call\n", errno );
      exit (EXIT_FAILURE);
    }

  for (;;)
    {
      pid_t childid = fork();

      switch (childid)
        {
        case -1:
          break;

        case 0:
          close (fd[0]);
          goto do_child_task;

        default:
          /*
           *  Parent process
           */
          close (read_fd);
          close (fd[1]);
          return fd[0];
        }

      /*
       *  Parent in error
       */
      fprintf (stderr, z_fork_err, errno, xstrerror (errno),
               p_fixd->fix_name);
      {
        static int failCt = 0;
        if ((errno != EAGAIN) || (++failCt > 10))
          exit (EXIT_FAILURE);
        sleep (1);
      }
    } do_child_task:;

  /*
   *  Close our current stdin and stdout
   */
  close (STDIN_FILENO);
  close (STDOUT_FILENO);
  UNLOAD_DATA();

  /*
   *  Make the fd passed in the stdin, and the write end of
   *  the new pipe become the stdout.
   */
  fcntl (fd[1], F_DUPFD, STDOUT_FILENO);
  fcntl (read_fd, F_DUPFD, STDIN_FILENO);

  apply_fix (p_fixd, pz_curr_file);
  exit (0);
}
#endif /* !SEPARATE_FIX_PROC */


#ifdef SEPARATE_FIX_PROC
static void
fix_with_system (p_fixd, pz_fix_file, pz_file_source, pz_temp_file)
  tFixDesc* p_fixd;
  tCC* pz_fix_file;
  tCC* pz_file_source;
  tCC* pz_temp_file;
{
  char*  pz_cmd;
  char*  pz_scan;
  size_t argsize;

  if (p_fixd->fd_flags & FD_SUBROUTINE)
    {
      tSCC z_applyfix_prog[] = "/fixinc/applyfix";

      argsize = 32
              + strlen( pz_orig_dir )
              + sizeof( z_applyfix_prog )
              + strlen( pz_fix_file )
              + strlen( pz_file_source )
              + strlen( pz_temp_file );

      pz_cmd = (char*)xmalloc( argsize );

      strcpy( pz_cmd, pz_orig_dir );
      pz_scan = pz_cmd + strlen( pz_orig_dir );
      strcpy( pz_scan, z_applyfix_prog );
      pz_scan += sizeof( z_applyfix_prog ) - 1;
      *(pz_scan++) = ' ';

      /*
       *  Now add the fix number and file names that may be needed
       */
      sprintf (pz_scan, "%ld \'%s\' \'%s\' \'%s\'", p_fixd - fixDescList,
	       pz_fix_file, pz_file_source, pz_temp_file);
    }
  else /* NOT an "internal" fix: */
    {
      size_t parg_size;
#ifdef __MSDOS__
      /* Don't use the "src > dstX; rm -f dst; mv -f dstX dst" trick:
         dst is a temporary file anyway, so we know there's no other
         file by that name; and DOS's system(3) doesn't mind to
         clobber existing file in redirection.  Besides, with DOS 8+3
         limited file namespace, we can easily lose if dst already has
         an extension that is 3 or more characters long.

         I do not think the 8+3 issue is relevant because all the files
         we operate on are named "*.h", making 8+2 adequate.  Anyway,
         the following bizarre use of 'cat' only works on DOS boxes.
         It causes the file to be dropped into a temporary file for
         'cat' to read (pipes do not work on DOS).  */
      tSCC   z_cmd_fmt[] = " \'%s\' | cat > \'%s\'";
#else
      /* Don't use positional formatting arguments because some lame-o
         implementations cannot cope  :-(.  */
      tSCC   z_cmd_fmt[] = " %s > %sX ; rm -f %s; mv -f %sX %s";
#endif
      tCC**  ppArgs = p_fixd->patch_args;

      argsize = sizeof( z_cmd_fmt ) + strlen( pz_temp_file )
              + strlen( pz_file_source );
      parg_size = argsize;
      

      /*
       *  Compute the size of the command line.  Add lotsa extra space
       *  because some of the args to sed use lotsa single quotes.
       *  (This requires three extra bytes per quote.  Here we allow
       *  for up to 8 single quotes for each argument, including the
       *  command name "sed" itself.  Nobody will *ever* need more. :)
       */
      for (;;)
        {
          tCC* p_arg = *(ppArgs++);
          if (p_arg == NULL)
            break;
          argsize += 24 + strlen( p_arg );
        }

      /* Estimated buffer size we will need.  */
      pz_scan = pz_cmd = (char*)xmalloc( argsize );
      /* How much of it do we allot to the program name and its
         arguments.  */
      parg_size = argsize - parg_size;

      ppArgs = p_fixd->patch_args;

      /*
       *  Copy the program name, unquoted
       */
      {
        tCC*   pArg = *(ppArgs++);
        for (;;)
          {
            char ch = *(pArg++);
            if (ch == NUL)
              break;
            *(pz_scan++) = ch;
          }
      }

      /*
       *  Copy the program arguments, quoted
       */
      for (;;)
        {
          tCC*   pArg = *(ppArgs++);
	  char*  pz_scan_save;
          if (pArg == NULL)
            break;
          *(pz_scan++) = ' ';
          pz_scan = make_raw_shell_str( pz_scan_save = pz_scan, pArg,
					parg_size - (pz_scan - pz_cmd) );
	  /*
	   *  Make sure we don't overflow the buffer due to sloppy
	   *  size estimation.
	   */
	  while (pz_scan == (char*)NULL)
	    {
	      size_t already_filled = pz_scan_save - pz_cmd;
	      pz_cmd = (char*)xrealloc( pz_cmd, argsize += 100 );
	      pz_scan_save = pz_scan = pz_cmd + already_filled;
	      parg_size += 100;
	      pz_scan = make_raw_shell_str( pz_scan, pArg,
					    parg_size - (pz_scan - pz_cmd) );
	    }
        }

      /*
       *  add the file machinations.
       */
#ifdef __MSDOS__
      sprintf (pz_scan, z_cmd_fmt, pz_file_source, pz_temp_file );
#else
      sprintf (pz_scan, z_cmd_fmt, pz_file_source, pz_temp_file,
               pz_temp_file, pz_temp_file, pz_temp_file);
#endif
    }
  system( pz_cmd );
  free( (void*)pz_cmd );
}

/* * * * * * * * * * * * *

    This loop should only cycle for 1/2 of one loop.
    "chain_open" starts a process that uses "read_fd" as
    its stdin and returns the new fd this process will use
    for stdout.  */

#else /* is *NOT* SEPARATE_FIX_PROC */
static int start_fixer PARAMS ((int, tFixDesc *, char *));
static int
start_fixer (read_fd, p_fixd, pz_fix_file)
  int read_fd;
  tFixDesc* p_fixd;
  char* pz_fix_file;
{
  tCC* pz_cmd_save;
  char* pz_cmd;

  if ((p_fixd->fd_flags & FD_SUBROUTINE) != 0)
    return internal_fix (read_fd, p_fixd);

  if ((p_fixd->fd_flags & FD_SHELL_SCRIPT) == 0)
    pz_cmd = (char*)NULL;
  else
    {
      tSCC z_cmd_fmt[] = "file='%s'\n%s";
      pz_cmd = (char*) xmalloc (strlen (p_fixd->patch_args[2])
				+ sizeof( z_cmd_fmt )
				+ strlen( pz_fix_file ));
      sprintf (pz_cmd, z_cmd_fmt, pz_fix_file, p_fixd->patch_args[2]);
      pz_cmd_save = p_fixd->patch_args[2];
      p_fixd->patch_args[2] = pz_cmd;
    }

  /*  Start a fix process, handing off the  previous read fd for its
      stdin and getting a new fd that reads from the fix process' stdout.
      We normally will not loop, but we will up to 10 times if we keep
      getting "EAGAIN" errors.

      */
  for (;;)
    {
      static int failCt = 0;
      int fd;

      fd = chain_open (read_fd,
                       (tCC **) p_fixd->patch_args,
                       (process_chain_head == -1)
                       ? &process_chain_head : (pid_t *) NULL);

      if (fd != -1)
        {
          read_fd = fd;
          break;
        }

      fprintf (stderr, z_fork_err, errno, xstrerror (errno),
               p_fixd->fix_name);

      if ((errno != EAGAIN) || (++failCt > 10))
        exit (EXIT_FAILURE);
      sleep (1);
    }

  /*  IF we allocated a shell script command,
      THEN free it and restore the command format to the fix description */
  if (pz_cmd != (char*)NULL)
    {
      free ((void*)pz_cmd);
      p_fixd->patch_args[2] = pz_cmd_save;
    }

  return read_fd;
}
#endif


/* * * * * * * * * * * * *

   Process the potential fixes for a particular include file.
   Input:  the original text of the file and the file's name
   Result: none.  A new file may or may not be created.  */

static t_bool fix_applies PARAMS ((tFixDesc *));
static t_bool
fix_applies (p_fixd)
  tFixDesc *p_fixd;
{
  const char *pz_fname = pz_curr_file;
  const char *pz_scan = p_fixd->file_list;
  int test_ct;
  tTestDesc *p_test;

# ifdef SEPARATE_FIX_PROC
  /*
   *  There is only one fix that uses a shell script as of this writing.
   *  I hope to nuke it anyway, it does not apply to DOS and it would
   *  be painful to implement.  Therefore, no "shell" fixes for DOS.
   */
  if (p_fixd->fd_flags & (FD_SHELL_SCRIPT | FD_SKIP_TEST))
    return BOOL_FALSE;
# else
  if (p_fixd->fd_flags & FD_SKIP_TEST)
    return BOOL_FALSE;
# endif

  /*  IF there is a file name restriction,
      THEN ensure the current file name matches one in the pattern  */

  if (pz_scan != (char *) NULL)
    {
      size_t name_len;

      while ((pz_fname[0] == '.') && (pz_fname[1] == '/'))
        pz_fname += 2;
      name_len = strlen (pz_fname);

      for (;;)
        {
          pz_scan = strstr (pz_scan + 1, pz_fname);
          /*  IF we can't match the string at all,
              THEN bail  */
          if (pz_scan == (char *) NULL)
            return BOOL_FALSE;

          /*  IF the match is surrounded by the '|' markers,
              THEN we found a full match -- time to run the tests  */

          if ((pz_scan[-1] == '|') && (pz_scan[name_len] == '|'))
            break;
        }
    }

  /*  FOR each test, see if it fails.
      IF it does fail, then we go on to the next test */

  for (p_test = p_fixd->p_test_desc, test_ct = p_fixd->test_ct;
       test_ct-- > 0;
       p_test++)
    {
      switch (p_test->type)
        {
        case TT_TEST:
          if (test_test (p_test, pz_curr_file) != APPLY_FIX) {
#ifdef DEBUG
            if (VLEVEL( VERB_EVERYTHING ))
              fprintf (stderr, z_failed, "TEST", p_fixd->fix_name,
                       pz_fname, p_fixd->test_ct - test_ct);
#endif
            return BOOL_FALSE;
          }
          break;

        case TT_EGREP:
          if (egrep_test (pz_curr_data, p_test) != APPLY_FIX) {
#ifdef DEBUG
            if (VLEVEL( VERB_EVERYTHING ))
              fprintf (stderr, z_failed, "EGREP", p_fixd->fix_name,
                       pz_fname, p_fixd->test_ct - test_ct);
#endif
            return BOOL_FALSE;
          }
          break;

        case TT_NEGREP:
          if (egrep_test (pz_curr_data, p_test) == APPLY_FIX) {
#ifdef DEBUG
            if (VLEVEL( VERB_EVERYTHING ))
              fprintf (stderr, z_failed, "NEGREP", p_fixd->fix_name,
                       pz_fname, p_fixd->test_ct - test_ct);
#endif
            /*  Negated sense  */
            return BOOL_FALSE;
          }
          break;

        case TT_FUNCTION:
          if (run_test (p_test->pz_test_text, pz_curr_file, pz_curr_data)
              != APPLY_FIX) {
#ifdef DEBUG
            if (VLEVEL( VERB_EVERYTHING ))
              fprintf (stderr, z_failed, "FTEST", p_fixd->fix_name,
                       pz_fname, p_fixd->test_ct - test_ct);
#endif
            return BOOL_FALSE;
          }
          break;
        }
    }

  return BOOL_TRUE;
}


/* * * * * * * * * * * * *

   Write out a replacement file  */

static void write_replacement PARAMS ((tFixDesc *));
static void
write_replacement (p_fixd)
  tFixDesc *p_fixd;
{
   const char* pz_text = p_fixd->patch_args[0];

   if ((pz_text == (char*)NULL) || (*pz_text == NUL))
     return;

   {
     FILE* out_fp = create_file ();
     fputs (pz_text, out_fp);
     fclose (out_fp);
   }
}


/* * * * * * * * * * * * *

    We have work to do.  Read back in the output
    of the filtering chain.  Compare each byte as we read it with
    the contents of the original file.  As soon as we find any
    difference, we will create the output file, write out all
    the matched text and then copy any remaining data from the
    output of the filter chain.
    */
static void test_for_changes PARAMS ((int));
static void
test_for_changes (read_fd)
  int read_fd;
{
  FILE *in_fp = fdopen (read_fd, "r");
  FILE *out_fp = (FILE *) NULL;
  unsigned char *pz_cmp = (unsigned char*)pz_curr_data;

#ifdef DO_STATS
  fixed_ct++;
#endif
  for (;;)
    {
      int ch;

      ch = getc (in_fp);
      if (ch == EOF)
        break;
      ch &= 0xFF; /* all bytes are 8 bits */

      /*  IF we are emitting the output
          THEN emit this character, too.
      */
      if (out_fp != (FILE *) NULL)
        putc (ch, out_fp);

      /*  ELSE if this character does not match the original,
          THEN now is the time to start the output.
      */
      else if (ch != *pz_cmp)
        {
          out_fp = create_file ();

#ifdef DO_STATS
          altered_ct++;
#endif
          /*  IF there are matched data, write the matched part now. */
          if ((char*)pz_cmp != pz_curr_data)
            fwrite (pz_curr_data, (size_t)((char*)pz_cmp - pz_curr_data),
					1, out_fp);

          /*  Emit the current unmatching character */
          putc (ch, out_fp);
        }
      else
        /*  ELSE the character matches.  Advance the compare ptr */
        pz_cmp++;
    }

  /*  IF we created the output file, ... */
  if (out_fp != (FILE *) NULL)
    {
      regmatch_t match;

      /* Close the file and see if we have to worry about
         `#include "file.h"' constructs.  */
      fclose (out_fp);
      if (regexec (&incl_quote_re, pz_curr_data, 1, &match, 0) == 0)
        extract_quoted_files (pz_curr_data, pz_curr_file, &match);
    }

  fclose (in_fp);
  close (read_fd);  /* probably redundant, but I'm paranoid */
}


/* * * * * * * * * * * * *

   Process the potential fixes for a particular include file.
   Input:  the original text of the file and the file's name
   Result: none.  A new file may or may not be created.  */

void
process ()
{
  tFixDesc *p_fixd = fixDescList;
  int todo_ct = FIX_COUNT;
  int read_fd = -1;
# ifndef SEPARATE_FIX_PROC
  int num_children = 0;
# else /* is SEPARATE_FIX_PROC */
  char* pz_file_source = pz_curr_file;
# endif

  if (access (pz_curr_file, R_OK) != 0)
    {
      int erno = errno;
      fprintf (stderr, "Cannot access %s from %s\n\terror %d (%s)\n",
               pz_curr_file, getcwd ((char *) NULL, MAXPATHLEN),
               erno, xstrerror (erno));
      return;
    }

  pz_curr_data = load_file (pz_curr_file);
  if (pz_curr_data == (char *) NULL)
    return;

#ifdef DO_STATS
  process_ct++;
#endif
  if (VLEVEL( VERB_PROGRESS ) && have_tty)
    fprintf (stderr, "%6d %-50s   \r", data_map_size, pz_curr_file );

# ifndef SEPARATE_FIX_PROC
  process_chain_head = NOPROCESS;

  /* For every fix in our fix list, ...  */
  for (; todo_ct > 0; p_fixd++, todo_ct--)
    {
      if (! fix_applies (p_fixd))
        continue;

      if (VLEVEL( VERB_APPLIES ))
        fprintf (stderr, "Applying %-24s to %s\n",
                 p_fixd->fix_name, pz_curr_file);

      if (p_fixd->fd_flags & FD_REPLACEMENT)
        {
          write_replacement (p_fixd);
          UNLOAD_DATA();
          return;
        }

      /*  IF we do not have a read pointer,
          THEN this is the first fix for the current file.
          Open the source file.  That will be used as stdin for
          the first fix.  Any subsequent fixes will use the
          stdout descriptor of the previous fix for its stdin.  */

      if (read_fd == -1)
        {
          read_fd = open (pz_curr_file, O_RDONLY);
          if (read_fd < 0)
            {
              fprintf (stderr, "Error %d (%s) opening %s\n", errno,
                       xstrerror (errno), pz_curr_file);
              exit (EXIT_FAILURE);
            }

          /*  Ensure we do not get duplicate output */

          fflush (stdout);
        }

      read_fd = start_fixer (read_fd, p_fixd, pz_curr_file);
      num_children++;
    }

  /*  IF we have a read-back file descriptor,
      THEN check for changes and write output if changed.   */

  if (read_fd >= 0)
    {
      test_for_changes (read_fd);
#ifdef DO_STATS
      apply_ct += num_children;
#endif
      /* Wait for child processes created by chain_open()
         to avoid leaving zombies.  */
      do  {
        wait ((int *) NULL);
      } while (--num_children > 0);
    }

# else /* is SEPARATE_FIX_PROC */

  for (; todo_ct > 0; p_fixd++, todo_ct--)
    {
      if (! fix_applies (p_fixd))
        continue;

      if (VLEVEL( VERB_APPLIES ))
        fprintf (stderr, "Applying %-24s to %s\n",
                 p_fixd->fix_name, pz_curr_file);

      if (p_fixd->fd_flags & FD_REPLACEMENT)
        {
          write_replacement (p_fixd);
          UNLOAD_DATA();
          return;
        }
      fix_with_system (p_fixd, pz_curr_file, pz_file_source, pz_temp_file);
      pz_file_source = pz_temp_file;
    }

  read_fd = open (pz_temp_file, O_RDONLY);
  if (read_fd < 0)
    {
      if (errno != ENOENT)
        fprintf (stderr, "error %d (%s) opening output (%s) for read\n",
                 errno, xstrerror (errno), pz_temp_file);
    }
  else
    {
      test_for_changes (read_fd);
      /* Unlinking a file while it is still open is a Bad Idea on
         DOS/Windows.  */
      close (read_fd);
      unlink (pz_temp_file);
    }

# endif
  UNLOAD_DATA();
}
