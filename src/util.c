/*============================================================================
  epub2txt v2 
  util.c
  Copyright (c)2022 Marco Bonelli, GPL v3.0
============================================================================*/

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "util.h"
#include "log.h"

/*==========================================================================
run_command
Run an helper command through fork + execvp, wait for it to finish and return
its status. Log execvp errors, and abort execution if abort_on_error is TRUE.
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
