/*============================================================================
  epub2txt v2 
  util.h
  Copyright (c)2022 Marco Bonelli, Kevin Boone GPL v3.0
============================================================================*/

#pragma once

#include "defs.h"

int run_command (const char *const argv[], BOOL abort_on_error);

/** Decode %xx in URL-type strings. The caller must free the resulting
    string, which will be no longer than the input. */
char *decode_url (const char *url);

/** Determine whether path is a subpath of root, assuming both paths are in
    canonical form. */
BOOL is_subpath (const char *root, const char *path);
