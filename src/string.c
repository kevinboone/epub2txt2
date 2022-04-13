/*============================================================================
  epub2txt v2 
  string.c
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if !defined(__MACH__)
#include <malloc.h>
#endif

#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 

struct _String
  {
  char *str;
  }; 


/*==========================================================================
string_create_empty 
*==========================================================================*/
String *string_create_empty (void)
  {
  return string_create ("");
  }


/*==========================================================================
string_create
*==========================================================================*/
String *string_create (const char *s)
  {
  String *self = malloc (sizeof (String));
  self->str = strdup (s);
  return self;
  }


/*==========================================================================
string_destroy
*==========================================================================*/
void string_destroy (String *self)
  {
  if (self)
    {
    if (self->str) free (self->str);
    free (self);
    }
  }


/*==========================================================================
string_cstr
*==========================================================================*/
const char *string_cstr (const String *self)
  {
  return self->str;
  }


/*==========================================================================
string_cstr_safe
*==========================================================================*/
const char *string_cstr_safe (const String *self)
  {
  if (self)
    {
    if (self->str) 
      return self->str;
    else
      return "";
    }
  else
    return "";
  }


/*==========================================================================
string_append
*==========================================================================*/
void string_append (String *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = strlen (self->str) + strlen (s) + 2;
  self->str = realloc (self->str, newlen);
  strcat (self->str, s);
  }


/*==========================================================================
string_prepend
*==========================================================================*/
void string_prepend (String *self, const char *s) 
  {
  if (!s) return;
  if (self->str == NULL) self->str = strdup ("");
  int newlen = strlen (self->str) + strlen (s) + 2;
  char *temp = strdup (self->str); 
  free (self->str);
  self->str = malloc (newlen);
  strcpy (self->str, s);
  strcat (self->str, temp);
  free (temp);
  }


/*==========================================================================
string_append_printf
*==========================================================================*/
void string_append_printf (String *self, const char *fmt,...) 
  {
  if (self->str == NULL) self->str = strdup ("");
  va_list ap;
  va_start (ap, fmt);
  char *s;
  vasprintf (&s, fmt, ap);
  string_append (self, s);
  free (s);
  va_end (ap);
  }


/*==========================================================================
string_length
*==========================================================================*/
int string_length (const String *self)
  {
  if (self == NULL) return 0;
  if (self->str == NULL) return 0;
  return strlen (self->str);
  }


/*==========================================================================
string_clone
*==========================================================================*/
String *string_clone (const String *self)
  {
  if (!self) return NULL;
  if (!self->str) return string_create_empty();
  return string_create (string_cstr (self));
  }


/*==========================================================================
string_find
*==========================================================================*/
int string_find (const String *self, const char *search)
  {
  if (!self) return -1;
  if (!self->str) return -1;
  const char *p = strstr (self->str, search);
  if (p)
    return p - self->str;
  else
    return -1;
  }


/*==========================================================================
string_delete
*==========================================================================*/
void string_delete (String *self, const int pos, const int len)
  {
  char *str = self->str;
  if (pos + len > strlen (str))
    string_delete (self, pos, strlen(str) - len);
  else
    {
    char *buff = malloc (strlen (str) - len + 2);
    strncpy (buff, str, pos); 
    strcpy (buff + pos, str + pos + len);
    free (self->str);
    self->str = buff;
    }
  }


/*==========================================================================
string_insert
*==========================================================================*/
void string_insert (String *self, const int pos, 
    const char *replace)
  {
  char *buff = malloc (strlen (self->str) + strlen (replace) + 2);
  char *str = self->str;
  strncpy (buff, str, pos);
  buff[pos] = 0;
  strcat (buff, replace);
  strcat (buff, str + pos); 
  free (self->str);
  self->str = buff;
  }



/*==========================================================================
string_substitute_all
*==========================================================================*/
String *string_substitute_all (const String *self, 
    const char *search, const char *replace)
  {
  String *working = string_clone (self);
  BOOL cont = TRUE;
  while (cont)
    {
    int i = string_find (working, search);
    if (i >= 0)
      {
      string_delete (working, i, strlen (search));
      string_insert (working, i, replace);
      }
    else
      cont = FALSE;
    }
  return working;
  }


/*==========================================================================
  string_create_from_utf8_file 
*==========================================================================*/
BOOL string_create_from_utf8_file (const char *filename, 
    String **result, char **error)
  {
  String *self = NULL;
  BOOL ok = FALSE; 
  int f = open (filename, O_RDONLY);
  if (f > 0)
    {
    self = malloc (sizeof (String));
    struct stat sb;
    fstat (f, &sb);
    int64_t size = sb.st_size;
    char *buff = malloc (size + 2);
    self->str = buff; 

    // Read the first three characters, to check for a UTF-8 byte-order-mark
    read (f, buff, 3);
    if (buff[0] == (char)0xEF && buff[1] == (char)0xBB && buff[2] == (char)0xBF)
      {
      read (f, buff, size - 3);
      self->str[size - 3] = 0;
      }
    else
      {
      read (f, buff + 3, size - 3);
      self->str[size] = 0;
      }

    *result = self;
    ok = TRUE;
    }
  else
    {
    asprintf (error, "Can't open file '%s' for reading: %s", 
      filename, strerror (errno));
    ok = FALSE;
    }

  return ok;
  }


/*==========================================================================
  string_encode_url
*==========================================================================*/
static char to_hex(char code)
  {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
  }


String *string_encode_url (const char *str)
  {
  if (!str) return string_create_empty();;
  const char *pstr = str; 
  char *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr)
    {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_'
      || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4),
         *pbuf++ = to_hex(*pstr & 15);
    pstr++;
    }
  *pbuf = '\0';
  String *result = string_create (buf);
  free (buf);
  return (result);
  }


/*==========================================================================
  string_append_byte
*==========================================================================*/
void string_append_byte (String *self, const BYTE byte)
  {
  char buff[2];
  buff[0] = byte;
  buff[1] = 0;
  string_append (self, buff);
  }


/*==========================================================================
  string_append_c
*==========================================================================*/
void string_append_c (String *self, const uint32_t ch)
  {
  if (ch < 0x80) 
    {
    string_append_byte (self, (BYTE)ch);
    }
  else if (ch < 0x0800) 
    {
    string_append_byte (self, (BYTE)((ch >> 6) | 0xC0));
    string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  else if (ch < 0x10000) 
    {
    string_append_byte (self, (BYTE)((ch >> 12) | 0xE0));
    string_append_byte (self, (BYTE)((ch >> 6 & 0x3F) | 0x80));
    string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  else 
    {
    string_append_byte (self, (BYTE)((ch >> 18) | 0xF0));
    string_append_byte (self, (BYTE)(((ch >> 12) & 0x3F) | 0x80));
    string_append_byte (self, (BYTE)(((ch >> 6) & 0x3F) | 0x80));
    string_append_byte (self, (BYTE)((ch & 0x3F) | 0x80));
    }
  }




