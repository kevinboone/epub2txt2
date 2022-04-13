/*============================================================================
  epub2txt v2 
  wstring.c
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#define _GNU_SOURCE
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <errno.h> 
#include <string.h> 
#include "wstring.h"
#include "string.h"
#include "convertutf.h"
#include "log.h"

struct _WString
  {
  uint32_t *str;
  }; 


/*============================================================================
  wstring_convert_utf8_to_utf32
===========================================================================*/
uint32_t *wstring_convert_utf8_to_utf32 (const char *_in)
  {
  IN
  const char* in = (const char *)_in;
  int max_out = strlen (_in);
  uint32_t *out = malloc ((max_out + 1) * sizeof (uint32_t));
  memset (out, 0, (max_out + 1) * sizeof (uint32_t));
  uint32_t *out_temp = out;
  
  ConvertUTF8toUTF32 ((const UTF8 **)&in, (const UTF8 *)in+strlen((char *)in),
      (UTF32**)&out_temp, (UTF32*)out + max_out, 0);
  
  int len = out_temp - out;
  out [len] = 0;
  OUT
  return out;
  }


/*============================================================================
  wstring_create_empty
============================================================================*/
WString *wstring_create_empty (void)
  {
  WString *self = malloc (sizeof (WString));
  self->str = malloc (sizeof (uint32_t));
  self->str[0] = 0;
  return self;
  }



/*============================================================================
  wstring_create_from_utf8
============================================================================*/
WString *wstring_create_from_utf8 (const char *s)
  {
  WString *self = malloc (sizeof (WString));
  self->str = wstring_convert_utf8_to_utf32 (s);
  return self;
  }


/*============================================================================
  wstring_create_from_utf8_file
============================================================================*/
BOOL wstring_create_from_utf8_file (const char *filename, 
    WString **result, char **error)
  {
  IN
  WString *self = NULL;
  BOOL ok = FALSE; 
  int f = open (filename, O_RDONLY);
  if (f > 0)
    {
    self = malloc (sizeof (WString));
    struct stat sb;
    fstat (f, &sb);
    int64_t size = sb.st_size;
    char *buff = malloc (size + 2);
    int n = read (f, buff, size);
    close (f);
    buff[n] = 0;

    // Might need to skip a UTF-8 BOM when reading file
    if (buff[0] == (char)0xEF && buff[1] == (char)0xBB && buff[2] == (char)0xBF)
      self->str = wstring_convert_utf8_to_utf32 (buff + 3);
    else
      self->str = wstring_convert_utf8_to_utf32 (buff);

    free (buff);

    *result = self;
    ok = TRUE;
    }
  else
    {
    asprintf (error, "Can't open file '%s' for reading: %s", 
      filename, strerror (errno));
    ok = FALSE;
    }

  OUT
  return ok;
  }


/*============================================================================
  wstring_length
============================================================================*/
const int wstring_length (const WString *self)
  {
  IN
  if (!self) 
    {
    OUT
    return 0;
    }
  uint32_t *s = self->str;
  int i = 0;
  uint32_t c = 0;
  do
    {
    c = s[i];
    i++;
    } while (c != 0);
  int ret = i - 1;
  return ret;
  OUT
  }


/*============================================================================
  wstring_destroy
============================================================================*/
void wstring_destroy (WString *self)
  {
  IN
  if (self)
    {
    if (self->str) free (self->str);
    free (self);
    }
  OUT
  }


/*============================================================================
  wstring_wstr
============================================================================*/
const uint32_t *wstring_wstr (const WString *self)
  {
  return self->str;
  }


/*============================================================================
  wstring_to_utf8
============================================================================*/
char *wstring_to_utf8 (const WString *self)
  {
  const uint32_t *s = self->str;
  String *temp = string_create_empty();
  int i, l = wstring_length (self);
  for (i = 0; i < l; i++)
     string_append_c (temp, s[i]);

  char *ret = strdup (string_cstr (temp));
  string_destroy (temp);
  return ret;
  }


/*============================================================================
  wstring_append_c
============================================================================*/
void wstring_append_c (WString *self, const uint32_t c)
  {
  int l = wstring_length (self);
  self->str = realloc (self->str, (l + 2) * sizeof (uint32_t));
  self->str[l] = c;
  self->str[l+1] = 0; 
  }


/*============================================================================
  wstring_append
============================================================================*/
void wstring_append (WString *self, const WString *other)
  {
  int mylen = wstring_length (self);
  int otherlen = wstring_length (other);
  self->str = realloc (self->str, (mylen + otherlen + 1) * sizeof (uint32_t));
  int i;
  for (i = 0; i < otherlen; i++)
    self->str[mylen+i] = other->str[i];
  self->str[mylen+i] = 0; 
  }


/*============================================================================
  wstring_clear
============================================================================*/
void  wstring_clear (WString *self)
  {
  free (self->str);
  self->str = malloc (sizeof (uint32_t));
  self->str[0] = 0;
  }


/*============================================================================
  wstring_is_whitespace
============================================================================*/
BOOL wstring_is_whitespace (const WString *self)
  {
  int l = wstring_length (self);
  uint32_t *s = self->str;
  int i;
  for (i = 0; i < l; i++)
    {
    uint32_t c = s[i];
    if (c != ' ' && c != '\n' && c != '\t') return FALSE;
    }

  return TRUE;
  }




