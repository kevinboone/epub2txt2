/*============================================================================
  epub2txt v2 
  epub2txt.h
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include "defs.h"

typedef struct _Epub2TxtOptions
  {
  int width; // Screen width
  BOOL ascii; // Reduce output to ASCII
  BOOL ansi; // Emit ANSI terminal codes
  BOOL raw; // Completely unformatted output 
  } Epub2TxtOptions;

void epub2txt_do_file (const char *file, const Epub2TxtOptions *options, 
     char **error);

