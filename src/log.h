/*==========================================================================
  epub2txt v2 
  log.h
  Copyright (c)2020 Kevin Boone, GPL v3.0
*==========================================================================*/

#pragma once

#define ERROR 0
#define WARNING 1
#define INFO 2
#define DEBUG 3
#define TRACE 4

#define IN log_trace ("Entering %s", __PRETTY_FUNCTION__);
#define OUT log_trace ("Leaving %s", __PRETTY_FUNCTION__);

void log_error (const char *fmt,...);
void log_warning (const char *fmt,...);
void log_info (const char *fmt,...);
void log_debug (const char *fmt,...);
void log_trace (const char *fmt,...);
void log_set_level (const int level);

