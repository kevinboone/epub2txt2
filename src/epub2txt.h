/*============================================================================
  epub2txt v2 
  epub2txt.h
  Copyright (c)2017-2024 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include "defs.h"

typedef struct _Epub2TxtOptions
  {
  int width; // Screen width
  BOOL ascii; // Reduce output to ASCII
  BOOL ansi; // Emit ANSI terminal codes
  BOOL raw; // Completely unformatted output 
  BOOL meta; // Show metadata
  BOOL notext; // Don't dump text 
  BOOL calibre; // Show Calibre metadata 
  char *section_separator; // Section separator; may be NULL
  } Epub2TxtOptions;

void epub2txt_do_file (const char *file, const Epub2TxtOptions *options, 
     char **error);

void epub2txt_cleanup (void);

