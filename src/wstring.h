/*============================================================================
  epub2txt v2 
  wstring.h
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <stdint.h> 
#include "defs.h"

struct _WString;
typedef struct _WString WString;

WString        *wstring_create_empty (void);
WString        *wstring_create_from_utf8 (const char *s);
BOOL            wstring_create_from_utf8_file (const char *filename, 
                  WString **result, char **error);
void            wstring_destroy (WString *self);
const int       wstring_length (const WString *self);
const uint32_t *wstring_wstr (const WString *self);
char           *wstring_to_utf8 (const WString *self);
void            wstring_append_c (WString *self, const uint32_t c);
void            wstring_append (WString *self, const WString *other);
void            wstring_clear (WString *self);
// Note the an empty string is _not_ whitespace
BOOL            wstring_is_whitespace (const WString *self);

// Static method
uint32_t *wstring_convert_utf8_to_utf32 (const char *utf8);

