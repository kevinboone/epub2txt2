/*============================================================================
  epub2txt v2 
  wraptext.h
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#ifndef __WRAPTEXT_H
#define __WRAPTEXT_H

#include <stdint.h>

// The largest number of bytes required to store a unicode character as
// UTF8, including a terminating 0
#define WT_UTF8_MAX_BYTES 8 

// Hard line break should be an unusued code point
#define WT_HARD_LINE_BREAK 9999

typedef uint32_t WT_UTF32;
typedef char WT_UTF8;

typedef void (*WrapTextOutputFn) (void *app_data, WT_UTF32 c);

struct _WrapTextContextPriv;

typedef struct _WrapTextContext
  {
  struct _WrapTextContextPriv *priv;
  } WrapTextContext;

#ifdef __CPLUSPLUS
extern "C" {
#endif

void wraptext_wrap_utf32 (WrapTextContext *context, const WT_UTF32 *utf32);

WrapTextContext *wraptext_context_new (void);

void wraptext_context_free (WrapTextContext *self);

void wraptext_context_set_output_fn (WrapTextContext *self, 
  WrapTextOutputFn fn);

unsigned int wraptext_context_get_fmt (WrapTextContext *self);
void wraptext_context_zero_fmt (WrapTextContext *self);
void wraptext_context_set_fmt (WrapTextContext *self, unsigned int fmt);
void wraptext_context_reset_fmt (WrapTextContext *self, unsigned int fmt);
void wraptext_context_set_app_opts (WrapTextContext *self, void *app_opts);
void *wraptext_context_get_app_opts (WrapTextContext *self);

void wraptext_context_set_flags (WrapTextContext *self, int flags);

void wraptext_context_set_width (WrapTextContext *self, int width);

void wraptext_context_set_app_data (WrapTextContext *self, void *app_data);

void wraptext_context_reset (WrapTextContext *self);

void wraptext_eof (WrapTextContext *context);

WT_UTF32 *wraptext_convert_utf8_to_utf32 (const WT_UTF8 *utf8);

const int wraptext_utf32_length (const WT_UTF32 *s);

#ifdef __CPLUSPLUS
}
#endif

#endif
