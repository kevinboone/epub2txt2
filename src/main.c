/*============================================================================
  epub2txt v2 
  main.c
  Copyright (c)2020-2024 Kevin Boone, GPL v3.0
============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <getopt.h>
#include <signal.h>
#include "epub2txt.h" 
#include "defs.h" 
#include "log.h" 

/*============================================================================
  sig_handler 
============================================================================*/
static void sig_handler (int signo)
  {
  (void)signo;
  epub2txt_cleanup();
  exit (0);
  }

/*============================================================================
  main
============================================================================*/
int main (int argc, char **argv)
  {
  BOOL show_version = FALSE;
  BOOL show_help = FALSE;
  BOOL ascii = FALSE;
  BOOL is_a_tty = FALSE;
  BOOL noansi = FALSE;
  BOOL raw = FALSE;
  BOOL meta = FALSE;
  BOOL notext = FALSE;
  BOOL calibre = FALSE;
  char *section_separator = NULL;
  int width = 80;

  static struct option long_options[] =
    {
     {"ascii", no_argument, NULL, 'a'},
     {"calibre", no_argument, NULL, 'c'},
     {"raw", no_argument, NULL, 'r'},
     {"meta", no_argument, NULL, 'm'},
     {"version", no_argument, NULL, 'v'},
     {"noansi", no_argument, NULL, 'n'},
     {"width", required_argument, NULL, 'w'},
     {"log", required_argument, NULL, 'l'},
     {"separator", required_argument, NULL, 's'},
     {"help", no_argument, NULL, 'h'},
     {"notext", no_argument, NULL, 0},
     {0, 0, 0, 0}
    };


  log_set_level (WARNING);
  
  // If we have a TTY, try to get the console width in columns. This
  //   may not work on all systems, so we need a fall-back.
  if (isatty (STDOUT_FILENO))
    {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
      {
      width = ws.ws_col;
      }
    is_a_tty = TRUE;
    }

  // If stdout is not a tty, try stdin. Why? Because if there _is_ a
  //  terminal attached somehow, we want to use its column width,
  //  if we use the "--wrap" option
  if (!is_a_tty)
    {
    if (isatty (STDIN_FILENO))
      {
      struct winsize ws;
      if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0)
        {
        width = ws.ws_col;
        }
      }
    is_a_tty = TRUE;
    }

  int opt;
  while (1)
    {
    int option_index = 0;
    opt = getopt_long (argc, argv, "avw:l:nrmchs:",
      long_options, &option_index);

    if (opt == -1) break;

    switch (opt)
      {
      case 0:
        if (strcmp (long_options[option_index].name, "version") == 0)
          show_version = TRUE;
        else if (strcmp (long_options[option_index].name, "width") == 0)
          width = atoi (optarg);
        else if (strcmp (long_options[option_index].name, "log") == 0)
          log_set_level (atoi (optarg));
        else if (strcmp (long_options[option_index].name, "help") == 0)
          show_help = TRUE; 
        else if (strcmp (long_options[option_index].name, "raw") == 0)
          raw = TRUE; 
        else if (strcmp (long_options[option_index].name, "ascii") == 0)
          ascii = TRUE; 
        else if (strcmp (long_options[option_index].name, "calibre") == 0)
          calibre = TRUE; 
        else if (strcmp (long_options[option_index].name, "noansi") == 0)
          noansi = TRUE; 
        else if (strcmp (long_options[option_index].name, "meta") == 0)
          meta = TRUE; 
        else if (strcmp (long_options[option_index].name, "notext") == 0)
          notext = TRUE; 
        else if (strcmp 
	       (long_options[option_index].name, "separator") == 0)
          section_separator = strdup (optarg); 
        else
          exit (-1);
      case 'a':
        ascii = TRUE; break;
      case 'c':
        calibre = TRUE; break;
      case 'n':
        noansi = TRUE; break;
      case 'h':
        show_help = TRUE; break;
      case 'v':
        show_version = TRUE; break;
      case 'r':
       raw = TRUE; break;
      case 'l':
        log_set_level (atoi(optarg)); break;
      case 'm':
        meta = TRUE; break;
      case 'w':
        width = atoi (optarg); break;
      case 's':
        section_separator = strdup (optarg); break;
      }
    }

  if (show_version)
    {
    printf (APPNAME " version " VERSION "\n");
    printf ("Copyright (c)2013-2024 Kevin Boone and contributors\n");
    printf ("Distributed under the terms of the GNU Public Licence, v3.0\n");
    exit (0);
    }

  if (show_help)
    {
    printf ("Usage: %s [options] {files...}\n", argv[0]);
    printf ("  -a,--ascii          try to output ASCII only\n");
    printf ("  -c,--calibre        show Calibre metadata (with -m)\n");
    printf ("  -h,--help           show this message\n");
    printf ("  -l,--log=N          set log level, 0-4\n");
    printf ("  -m,--meta           dump document metadata\n");
    printf ("  -n,--noansi         don't output ANSI terminal codes\n");
    printf ("     --notext         don't output document body\n");
    printf ("  -r,--raw            no formatting at all\n");
    printf ("  -s,--separator=text section separator text\n");
    printf ("  -v,--version        show version\n");
    printf ("  -w,--width=N        set output width\n");
    exit (0);
    }

  if (optind == argc)
    {
    fprintf (stderr, "%s: no files selected\n", argv[0]); 
    fprintf (stderr, "'%s --help' for usage\n", argv[0]); 
    exit (-1);
    }

  Epub2TxtOptions options;
  memset (&options, 0, sizeof (options));
  options.width = width;
  options.ascii = ascii;
  options.meta = meta;
  options.notext = notext;
  options.calibre = calibre;
  options.section_separator = section_separator;

  if (is_a_tty)
    options.ansi = TRUE;
  if (noansi)
    options.ansi = FALSE; 
 
  options.raw = raw;

  signal (SIGPIPE, sig_handler);
  signal (SIGQUIT, sig_handler);
  signal (SIGINT, sig_handler);
  signal (SIGHUP, sig_handler);

  int i;
  for (i = optind; i < argc; i++)
    {
    const char *file = argv[i]; 
    char *error = NULL;
    epub2txt_do_file (file, &options, &error); 
    if (error)
      {
      fprintf (stderr, "%s: %s\n", argv[0], error);
      free (error);
      }
    }

  if (section_separator) free (section_separator);
  exit (0);
  }

