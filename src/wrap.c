/*============================================================================
  epub2txt v2 
  wrap.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

  This file contains general-purpose text string wrapping functions, that
  work on 32-bit characters, so each character is a fixed length. This is
  to avoid the problems with character length that tend to arise when 
  working with chars as UTF-8 bytes.
============================================================================*/

#include <stdio.h>

#if !defined(__MACH__)
#include <malloc.h>
#endif

#include <string.h>
#include <stdlib.h>
#include "defs.h" 
#include "wrap.h"
#include "convertutf.h"
#include "xhtml.h"

#define WT_STATE_START 0
#define WT_STATE_WORD 1
#define WT_STATE_WHITE 2

typedef struct _WrapTextContextPriv 
  {
  WrapTextOutputFn outputFn;
  int width;
  int flags;
  int state;
  int column;
  int white_count;
  unsigned int fmt;
  void *app_opts;
  void *app_data;
  BOOL blank_line;
  WT_UTF32 last;
  WT_UTF32 *token;
  } WrapTextContextPriv;


/** Convert a single UTF32 character to a UTF8 representation, where
 * the UTF8 is an array of characters terminated with a zero. The 
 * utf8 parameter must be a pointed to an array of WT_UTF8 (aka char)
 * of at least WT_UTF8_MAX_BYTES size. */
void wraptext_context_utf32_char_to_utf8 (const uint32_t c, WT_UTF8* utf8)
  {
  WT_UTF32 _in = c;
  const UTF32* in = (const UTF32 *) &_in;
  int max_out = WT_UTF8_MAX_BYTES;
  UTF8 *out = (UTF8 *)utf8;
  memset (out, 0, max_out * sizeof (UTF8));
  UTF8 *out_temp = out;

  ConvertUTF32toUTF8 (&in, in + 1,
      //&out_temp, out + max_out * 4, 0);
      &out_temp, out + max_out, 0);
  int len = out_temp - out;
  utf8[len] = 0;
  }


void _stdout_output_fn (void *app_data, WT_UTF32 c)
  {
  WT_UTF8 buff [WT_UTF8_MAX_BYTES];  
  wraptext_context_utf32_char_to_utf8 (c, buff);
  fputs (buff, stdout); 
  }


static void _wraptext_append_token (WrapTextContext *context, const WT_UTF32 c)
  {
  WT_UTF32 *token = context->priv->token;
  if (!token)
    {
    token = malloc (sizeof (WT_UTF32));
    token[0] = 0;
    }
   
  int l = wraptext_utf32_length (token);
   
  token = realloc (token, (l+2) * sizeof (WT_UTF32));

  token [l] = c;
  token [l+1] = 0;

  context->priv->token = token;
  }


// Whitespace other than newline
BOOL _wraptext_is_white (WT_UTF32 c)
  {
  if (c == 160) return TRUE; // nbsp
  if (c == 32) return TRUE;
  if (c == 9) return TRUE;
  //TODO -- other unicode whitespace chars
  return FALSE;
  }

// Whitespace other than newline
BOOL _wraptext_is_all_white (const WT_UTF32 *s)
  {
  while (*s)
    {
    if (!_wraptext_is_white (*s)) return FALSE;
    s++;
    }
  return TRUE;
  }


// TODO -- detect other newline characters 
BOOL _wraptext_is_newline (WT_UTF32 c)
  {
  if (c == 10) return TRUE;
  return FALSE;
  }


void _wraptext_emit_newline (WrapTextContext *context)
  {
  context->priv->outputFn (context->priv->app_data, (WT_UTF32)'\n'); 
  }


void _wraptext_new_line (WrapTextContext *context)
  {
  _wraptext_emit_newline (context);
  context->priv->column = 0;
  }


void _wraptext_flush_string (WrapTextContext *context, WT_UTF32 *s)
  {
  int i, l = wraptext_utf32_length (s);

  if (l + context->priv->column + 1 >= context->priv->width)
    {
    xhtml_emit_fmt_eol_pre (context);    /* upcall: turn-off all ANSI highlghting before EOL */
    _wraptext_emit_newline (context);
    xhtml_emit_fmt_eol_post (context);   /* upcall: restore ANSI highlighting after EOL */
    context->priv->column = 0;
    }
 
  for (i = 0; i < l; i++)
    {
    WT_UTF32 c = s[i];
    context->priv->outputFn (context->priv->app_data, c); 
    }

  context->priv->column += l;
  }


void _wraptext_flush_space (WrapTextContext *context, BOOL allowAtStart)
  {
  if ((context->priv->column > 0) || allowAtStart)
    {
    context->priv->outputFn (context->priv->app_data, ' '); 
    context->priv->column++;
    }
  }


void _wraptext_flush_token (WrapTextContext *context)
  {
  WT_UTF32 *token = context->priv->token;
  // Don't flush anything -- even a space -- if the token is
  //  null. This will only happen at end-of-line or end-of-file
  //  states (hopefully)
  if (token)
    {
    if (token[0])
      {
      if (!_wraptext_is_all_white (token))
        context->priv->blank_line = FALSE;
      }
    _wraptext_flush_string (context, token);
    _wraptext_flush_space (context, FALSE);
    free (context->priv->token);
    }

  context->priv->token = NULL;
  }


void _wraptext_wrap_next (WrapTextContext *context, const WT_UTF32 c)
  {
  WT_UTF32 last = context->priv->last;

  int state = context->priv->state;

  // This logic counts spaces at the ends of lines, so MD-style
  //   double-space linebreaks can be respected.
  // NB -- not used in epub2txt
  if (_wraptext_is_newline (c))
    {
    }
  else
    {
    if (_wraptext_is_white (c))
      context->priv->white_count++;
    else
      context->priv->white_count = 0;
    }
  
  // STATE_START

  if (state == WT_STATE_START && _wraptext_is_newline (c))
     {
     //printf ("!");
     // Double blank line -- respect this as a para separator
     if (context->priv->blank_line)
       {
       }
     else
       {
       _wraptext_new_line (context); 
       _wraptext_new_line (context); 
       context->priv->blank_line = TRUE;
       }
     state = WT_STATE_WHITE;
     }
  else if (state == WT_STATE_START && _wraptext_is_white (c))
     {
     // Space at the beginning of the line
     // Do nothing yet TODO
     }
  else if (state == WT_STATE_START)
     {
     _wraptext_append_token (context, c);
     state = WT_STATE_WORD;
     }

  // STATE_WORD

  else if (state == WT_STATE_WORD && c == WT_HARD_LINE_BREAK)
     {
     _wraptext_flush_token (context);
     _wraptext_new_line (context);
     state = WT_STATE_START;
     }
  else if (state == WT_STATE_WORD && _wraptext_is_newline (c))
     {
     _wraptext_flush_token (context);
     state = WT_STATE_START;
     }
  else if (state == WT_STATE_WORD && _wraptext_is_white (c))
     {
     _wraptext_flush_token (context);
     state = WT_STATE_WHITE;
     }
  else if (state == WT_STATE_WORD)
     {
     _wraptext_append_token (context, c);
     state = WT_STATE_WORD;
     }
  
  // STATE_WHITE

  else if (state == WT_STATE_WHITE && _wraptext_is_newline (c))
     {
     _wraptext_flush_token (context);
     state = WT_STATE_START;
     }
  else if (state == WT_STATE_WHITE && _wraptext_is_white (c))
     {
     state = WT_STATE_WHITE;
     }
  else if (state == WT_STATE_WHITE)
     {
     _wraptext_append_token (context, c);
     state = WT_STATE_WORD;
     }
  
  // We should ever get here
  else
     {
     fprintf (stderr, "Internal error: char %d in state %d\n", c, state);
     exit (-1);
     }

  context->priv->last = last;
  context->priv->state = state;
  }


void wraptext_eof (WrapTextContext *context)
  {
  // Handle any input that has not been handled already
  _wraptext_flush_token (context);
  }


void wraptext_wrap_utf32 (WrapTextContext *context, const WT_UTF32 *utf32)
  {
  int i, len = wraptext_utf32_length (utf32);
  for (i = 0; i < len; i++)
    {
    WT_UTF32 c = utf32[i];
    _wraptext_wrap_next (context, c);
    }
  }


void wraptext_easy_stdout_utf32 (const int width, const WT_UTF32 *utf32,
     int flags)
  {
  WrapTextContext *context = wraptext_context_new();
  wraptext_context_set_output_fn (context, _stdout_output_fn);
  wraptext_context_set_flags (context, flags);
  wraptext_context_set_width (context, width);
  wraptext_wrap_utf32 (context, utf32);
  wraptext_eof (context);
  wraptext_context_free (context);
  }


WrapTextContext *wraptext_context_new (void)
  {
  WrapTextContext *self = malloc (sizeof (WrapTextContext));
  memset (self, 0, sizeof (WrapTextContext));
  WrapTextContextPriv *priv = malloc (sizeof (WrapTextContextPriv));
  memset (priv, 0, sizeof (WrapTextContextPriv));
  self->priv = priv;
  self->priv->width = 80;
  self->priv->blank_line = TRUE; // Assume that we are starting on a new line
  self->priv->outputFn = _stdout_output_fn;
  wraptext_context_reset (self);
  return self;
  }


void wraptext_context_reset (WrapTextContext *self)
  {
  self->priv->state = WT_STATE_START;
  self->priv->column = 0;
  self->priv->last = 0;
  self->priv->white_count = 0;
  self->priv->fmt = 0;
  self->priv->blank_line = TRUE;
  if (self->priv->token) free (self->priv->token);
  self->priv->token = NULL;
  }


void wraptext_context_set_output_fn (WrapTextContext *self, 
    WrapTextOutputFn fn)
  {
  self->priv->outputFn = fn;
  }


void wraptext_context_set_width (WrapTextContext *self, int width)
  {
  self->priv->width = width;
  }

void wraptext_context_set_flags (WrapTextContext *self, int flags)
  {
  self->priv->flags = flags;
  }

void wraptext_context_zero_fmt (WrapTextContext *self)
  {
  self->priv->fmt = 0;
  }

unsigned int wraptext_context_get_fmt (WrapTextContext *self)
  {
  return self->priv->fmt;
  }

void wraptext_context_set_fmt (WrapTextContext *self, unsigned int fmt)
  {
  self->priv->fmt |= fmt;
  }

void wraptext_context_reset_fmt (WrapTextContext *self, unsigned int fmt)
  {
  self->priv->fmt &= ~fmt;
  }

void wraptext_context_set_app_opts (WrapTextContext *self, void *app_opts)
  {
  self->priv->app_opts = app_opts;
  }

void *wraptext_context_get_app_opts (WrapTextContext *self)
  {
  return self->priv->app_opts;
  }

void wraptext_context_set_app_data (WrapTextContext *self, void *app_data)
  {
  self->priv->app_data = app_data;
  }

void wraptext_context_free (WrapTextContext *self)
  {
  if (!self) return;
  if (self->priv)
    {
    free (self->priv);
    self->priv = NULL;
    }
  free (self);
  }


const int wraptext_utf32_length (const WT_UTF32 *s)
  {
  if (!s) return 0;
  int i = 0;
  WT_UTF32 c = 0;
  do
    {
    c = s[i];
    i++;
    } while (c != 0);
  return i - 1;
  }



