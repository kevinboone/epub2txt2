/*==========================================================================
  epub2txt v2 
  log.c
  Copyright (c)2000-2017 Kevin Boone, GPL v3.0
*==========================================================================*/

#define _GNU_SOURCE 1
#include <syslog.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "log.h"

static int log_level = DEBUG;

/*==========================================================================
log_set_level
*==========================================================================*/
void log_set_level (const int level)
  {
  log_level = level;
  }

/*==========================================================================
log_vprintf
*==========================================================================*/
void log_vprintf (const int level, const char *fmt, va_list ap)
  {
  if (level > log_level) return;
  char *str = NULL;
  vasprintf (&str, fmt, ap);
  const char *levelstr = "ERROR";
  if (level == WARNING)
    levelstr = "WARN";
  else if (level == INFO)
    levelstr = "INFO";
  else if (level == DEBUG)
    levelstr = "DEBUG";
  else if (level == TRACE)
    levelstr = "TRACE";
  fprintf (stderr, APPNAME " %s %s\n", levelstr, str);
  free (str);
  }


/*==========================================================================
log_error
*==========================================================================*/
void log_error (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_vprintf (ERROR,  fmt, ap);
  va_end (ap);
  }


/*==========================================================================
log_warning
*==========================================================================*/
void log_warning (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_vprintf (WARNING,  fmt, ap);
  va_end (ap);
  }


/*==========================================================================
log_info
*==========================================================================*/
void log_info (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_vprintf (INFO,  fmt, ap);
  va_end (ap);
  }


/*==========================================================================
log_debug
*==========================================================================*/
void log_debug (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_vprintf (DEBUG,  fmt, ap);
  va_end (ap);
  }


/*==========================================================================
log_trace
*==========================================================================*/
void log_trace (const char *fmt,...)
  {
  va_list ap;
  va_start (ap, fmt);
  log_vprintf (TRACE,  fmt, ap);
  va_end (ap);
  }



