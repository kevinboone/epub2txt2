/*============================================================================
  epub2txt v2 
  util.c
  Copyright (c)2022 Marco Bonelli, Kevin Boone, GPL v3.0
============================================================================*/

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include "util.h"
#include "log.h"

/*==========================================================================
run_command
Run an helper command through fork + execvp, wait for it to finish and return
its status. Log execvp errors, and abort execution if abort_on_error is TRUE.
(Marco Bonelli)
*==========================================================================*/
int run_command (const char *const argv[], BOOL abort_on_error)
  {
  int status;
  int pid = fork();

  if (pid == 0)
    {
    execvp(argv[0], (char **const)argv);
    log_error ("Can't execute command \"%s\": %s", argv[0], strerror (errno));

    if (abort_on_error)
      {
      kill (getppid(), SIGTERM);
      _exit (-1);
      }

    _exit (0);
    }

  waitpid (pid, &status, 0);
  return status;
  }

/*==========================================================================
  Decode %xx in URL-type strings. The caller must free the resulting
  string, which will be no longer than the input. 
  (Kevin Boone)
*==========================================================================*/
char *decode_url (const char *url)
  {
  char *ret = malloc (strlen (url) + 2);

  int len = 0;
  for (; *url; len++) 
    {
    if (*url == '%' && url[1] && url[2] && 
        isxdigit(url[1]) && isxdigit(url[2])) 
      {
      char url1 = url[1];
      char url2 = url[2];
      url1 -= url1 <= '9' ? '0' : (url1 <= 'F' ? 'A' : 'a')-10;
      url2 -= url2 <= '9' ? '0' : (url2 <= 'F' ? 'A' : 'a')-10;
      ret[len] = 16 * url1 + url2;
      url += 3;
      continue;
      }
    else if (*url == '+')
      {
      /* I have not tested this piece of the function, because I have not
         seen any instances of '+' (meaning space) in a spine href */
      url += 1;
      ret[len] = ' '; 
      }
    ret[len] = *url++;
    }
  ret[len] = '\0';

  return ret;
  }


