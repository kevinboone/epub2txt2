/*============================================================================
  epub2txt v2 
  string.c
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"

struct _String;
typedef struct _String String;

String      *string_create_empty (void);
String      *string_create (const char *s);
String      *string_clone (const String *self);
int          string_find (const String *self, const char *search);
void         string_destroy (String *self);
const char  *string_cstr (const String *self);
const char  *string_cstr_safe (const String *self);
void         string_append_printf (String *self, const char *fmt,...);
void         string_append (String *self, const char *s);
void         string_append_c (String *self, const uint32_t c);
void         string_prepend (String *self, const char *s);
int          string_length (const String *self);
String      *string_substitute_all (const String *self, 
                const char *search, const char *replace);
void        string_delete (String *self, const int pos, 
                const int len);
void        string_insert (String *self, const int pos, 
                const char *replace);
BOOL        string_create_from_utf8_file (const char *filename, 
                String **result, char **error);
String     *string_encode_url (const char *s);
void        string_append_byte (String *self, const BYTE byte);

