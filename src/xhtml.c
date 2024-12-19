/*============================================================================
  epub2txt v2 
  xhtml.c
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "epub2txt.h" 
#include "log.h"
#include "string.h"
#include "wstring.h"
#include "wrap.h"
#include "xhtml.h"

/*============================================================================
  Format definition stuff 
============================================================================*/

typedef enum { FORMAT_NONE, 
               FORMAT_BOLD_ON, FORMAT_BOLD_OFF,
               FORMAT_ITALIC_ON, FORMAT_ITALIC_OFF,
               FORMAT_H1_ON, FORMAT_H1_OFF,
               FORMAT_H2_ON, FORMAT_H2_OFF,
               FORMAT_H3_ON, FORMAT_H3_OFF,
               FORMAT_H4_ON, FORMAT_H4_OFF,
               FORMAT_H5_ON, FORMAT_H5_OFF } Format;

/* bitmasks for ANSI highlighting */
enum { FMT_BOLD = 1 << 0,
       FMT_ITAL = 1 << 1 };

/*============================================================================
  xhtml_is_start_format_tag
============================================================================*/
BOOL xhtml_is_start_format_tag (const char *tag, Format *format)
  {
  if (strcasecmp (tag, "b") == 0) 
     {
     *format = FORMAT_BOLD_ON;
     return TRUE;
     }
  if (strcasecmp (tag, "i") == 0) 
     {
     *format = FORMAT_ITALIC_ON;
     return TRUE;
     }
  return FALSE;
  }


/*============================================================================
  xhtml_is_end_breaking_tag
============================================================================*/
BOOL xhtml_is_end_breaking_tag (const char *tag, Format *format)
  {
  if (strcasecmp (tag, "/h1") == 0) 
    {
    *format = FORMAT_BOLD_OFF;
    return TRUE;
    }
  if (strcasecmp (tag, "/h2") == 0) 
    {
    *format = FORMAT_BOLD_OFF;
    return TRUE;
    }
  if (strcasecmp (tag, "/h3") == 0) 
    {
    *format = FORMAT_BOLD_OFF;
    return TRUE;
    }
  if (strcasecmp (tag, "/h4") == 0) 
    {
    *format = FORMAT_BOLD_OFF;
    return TRUE;
    }
  if (strcasecmp (tag, "/h5") == 0) 
    {
    *format = FORMAT_BOLD_OFF;
    return TRUE;
    }
  if (strcasecmp (tag, "/div") == 0) 
    {
    *format = FORMAT_NONE;
    return TRUE;
    }
  if (strcasecmp (tag, "/blockquote") == 0) 
    {
    *format = FORMAT_NONE;
    return TRUE;
    }
  return FALSE;
  }


/*============================================================================
  xhtml_is_end_format_tag
============================================================================*/
BOOL xhtml_is_end_format_tag (const char *tag, Format *format)
  {
  if (strcasecmp (tag, "/b") == 0) 
    {
    *format = FORMAT_BOLD_OFF;
    return TRUE;
    }
  if (strcasecmp (tag, "/i") == 0) 
    {
    *format = FORMAT_ITALIC_OFF;
    return TRUE;
    }
  return FALSE;
  }


/*============================================================================
  xhtml_is_start_breaking_tag
============================================================================*/
BOOL xhtml_is_start_breaking_tag (const char *tag, Format *format)
  {
  if (strcasecmp (tag, "h1") == 0) 
    {
    *format = FORMAT_BOLD_ON;
    return TRUE;
    }
  if (strcasecmp (tag, "h2") == 0) 
    {
    *format = FORMAT_BOLD_ON;
    return TRUE;
    }
  if (strcasecmp (tag, "h3") == 0) 
    {
    *format = FORMAT_BOLD_ON;
    return TRUE;
    }
  if (strcasecmp (tag, "h4") == 0) 
    {
    *format = FORMAT_BOLD_ON;
    return TRUE;
    }
  if (strcasecmp (tag, "h5") == 0) 
    {
    *format = FORMAT_BOLD_ON;
    return TRUE;
    }
  if (strcasecmp (tag, "div") == 0) 
    {
    *format = FORMAT_NONE;
    return TRUE;
    }
  if (strcasecmp (tag, "blockquote") == 0) 
    {
    *format = FORMAT_NONE;
    return TRUE;
    }
  return FALSE;
  }



/*============================================================================
  xhtml_emit_format
============================================================================*/
void xhtml_emit_format (const Epub2TxtOptions *options, Format format)
  {
  IN
  
  if (options->ansi && !options->raw)
    {
    switch (format)
      {
      case FORMAT_BOLD_ON:
	 printf ("\x1B[1m"); break;

      case FORMAT_BOLD_OFF:
	 printf ("\x1B[0m"); break;

      case FORMAT_ITALIC_ON:
	printf ("\x1B[3m"); break;

      case FORMAT_ITALIC_OFF:
	 printf ("\x1B[0m"); break;

      case FORMAT_NONE:
	 break;

      case FORMAT_H1_ON:
      case FORMAT_H2_ON:
      case FORMAT_H3_ON:
      case FORMAT_H4_ON:
      case FORMAT_H5_ON:
	 printf ("\x1B[1m"); break;

      case FORMAT_H1_OFF:
      case FORMAT_H2_OFF:
      case FORMAT_H3_OFF:
      case FORMAT_H4_OFF:
      case FORMAT_H5_OFF:
	 printf ("\x1B[0m"); break;

      }
    }
  OUT
  }

/*============================================================================
  xhtml_emit_fmt_eol_pre
============================================================================*/
void xhtml_emit_fmt_eol_pre (WrapTextContext *context)
  {
  IN
  
  unsigned int fmt = wraptext_context_get_fmt (context);
  const Epub2TxtOptions *options = (Epub2TxtOptions *) wraptext_context_get_app_opts (context);

  if (options->ansi && !options->raw && fmt)
    {
    /* reset ANSI escape-sequence at EOL. */
    xhtml_emit_format (options, FORMAT_BOLD_OFF);
    }
  OUT
  }

/*============================================================================
  xhtml_emit_fmt_eol_post
============================================================================*/
void xhtml_emit_fmt_eol_post (WrapTextContext *context)
  {
  IN
  
  unsigned int fmt = wraptext_context_get_fmt (context);
  const Epub2TxtOptions *options = (Epub2TxtOptions *) wraptext_context_get_app_opts (context);

  if (options->ansi && !options->raw && fmt)
    {
    /* turn those set, back on at BOL. */
    if (fmt & FMT_BOLD)
      xhtml_emit_format (options, FORMAT_BOLD_ON);
    if (fmt & FMT_ITAL)
      {
      xhtml_emit_format (options, FORMAT_ITALIC_ON);
      }
    }
  OUT
  }

/*============================================================================
  xhtml_set_format
============================================================================*/
void xhtml_set_format (const Epub2TxtOptions *options, Format format, WrapTextContext *context)
  {
  IN
  
  if (options->ansi && !options->raw)
    {
    switch (format)
      {
      case FORMAT_BOLD_ON:
        wraptext_context_set_fmt (context, FMT_BOLD);
        break;

      case FORMAT_BOLD_OFF:
        wraptext_context_reset_fmt (context, FMT_BOLD);
        break;

      case FORMAT_ITALIC_ON:
        wraptext_context_set_fmt (context, FMT_ITAL);
        break;

      case FORMAT_ITALIC_OFF:
        wraptext_context_reset_fmt (context, FMT_ITAL);
        break;

      case FORMAT_NONE:
        wraptext_context_zero_fmt (context);
        break;

      case FORMAT_H1_ON:
      case FORMAT_H2_ON:
      case FORMAT_H3_ON:
      case FORMAT_H4_ON:
      case FORMAT_H5_ON:
        wraptext_context_set_fmt (context, FMT_BOLD);
        break;

      case FORMAT_H1_OFF:
      case FORMAT_H2_OFF:
      case FORMAT_H3_OFF:
      case FORMAT_H4_OFF:
      case FORMAT_H5_OFF:
        wraptext_context_reset_fmt (context, FMT_BOLD);
        break;

      }
    }
  OUT
  }




/*============================================================================
  xhtml_transform_char
============================================================================*/
WString *xhtml_transform_char (uint32_t c, BOOL to_ascii)
  {
  WString *ret = wstring_create_empty();
  if (to_ascii && c > 127) // No ASCII chars will need transforming
    {
    if (c == 0x00B4) return wstring_create_from_utf8 ("\'");
    if (c == 0x0304) return wstring_create_from_utf8 ("-");
    if (c == 0x2010) return wstring_create_from_utf8 ("-");
    if (c == 0x2013) return wstring_create_from_utf8 ("-");
    if (c == 0x2014) return wstring_create_from_utf8 ("-");
    if (c == 0x2018) return wstring_create_from_utf8 ("'");
    if (c == 0x2019) return wstring_create_from_utf8 ("\'");
    if (c == 0x201C) return wstring_create_from_utf8 ("\"");
    if (c == 0x201D) return wstring_create_from_utf8 ("\"");
    if (c == 0xC2A0) return wstring_create_from_utf8 ("(c)"); // copyright 
    if (c == 0x00A9) return wstring_create_from_utf8 ("(c)"); // ditto
    if (c == 0xC2A9) return wstring_create_from_utf8 (" "); // nbsp
    if (c == 0x00A0) return wstring_create_from_utf8 (" "); // nbsp
    if (c == 0x2026) return wstring_create_from_utf8 (",,,"); // elipsis 
    if (c == 0x2022) return wstring_create_from_utf8 ("."); // dot
    if (c == 0x00B5) return wstring_create_from_utf8 ("u"); // mu
    if (c == 0x00C0) return wstring_create_from_utf8 ("A"); // accented A 
    if (c == 0x00C1) return wstring_create_from_utf8 ("A"); // accented A 
    if (c == 0x00C2) return wstring_create_from_utf8 ("A"); // accented A 
    if (c == 0x00C3) return wstring_create_from_utf8 ("A"); // accented A 
    if (c == 0x00C4) return wstring_create_from_utf8 ("A"); // accented A 
    if (c == 0x00C5) return wstring_create_from_utf8 ("A"); // accented A 
    if (c == 0x00C6) return wstring_create_from_utf8 ("AE"); // accented A 
    if (c == 0x00C7) return wstring_create_from_utf8 ("C"); // cedilla 
    if (c == 0x00C8) return wstring_create_from_utf8 ("E"); // accented E
    if (c == 0x00C9) return wstring_create_from_utf8 ("E"); // accented E
    if (c == 0x00CA) return wstring_create_from_utf8 ("E"); // accented E
    if (c == 0x00CB) return wstring_create_from_utf8 ("E"); // accented E
    if (c == 0x00CC) return wstring_create_from_utf8 ("I"); // accented I
    if (c == 0x00CD) return wstring_create_from_utf8 ("I"); // accented I
    if (c == 0x00CE) return wstring_create_from_utf8 ("I"); // accented I
    if (c == 0x00CF) return wstring_create_from_utf8 ("I"); // accented I
    if (c == 0x00D0) return wstring_create_from_utf8 ("D"); // accented D
    if (c == 0x00D1) return wstring_create_from_utf8 ("N"); // accented N
    if (c == 0x00D2) return wstring_create_from_utf8 ("O"); // accented O
    if (c == 0x00D3) return wstring_create_from_utf8 ("O"); // accented O
    if (c == 0x00D4) return wstring_create_from_utf8 ("O"); // accented O
    if (c == 0x00D5) return wstring_create_from_utf8 ("O"); // accented O
    if (c == 0x00D6) return wstring_create_from_utf8 ("O"); // accented O
    if (c == 0x00D7) return wstring_create_from_utf8 ("x"); // Multiply 
    if (c == 0x00D8) return wstring_create_from_utf8 ("O"); // accented O
    if (c == 0x00D9) return wstring_create_from_utf8 ("U"); // accented U
    if (c == 0x00DA) return wstring_create_from_utf8 ("U"); // accented U
    if (c == 0x00DB) return wstring_create_from_utf8 ("U"); // accented U
    if (c == 0x00DC) return wstring_create_from_utf8 ("U"); // accented U
    if (c == 0x00DD) return wstring_create_from_utf8 ("Y"); // accented Y
    if (c == 0x00DE) return wstring_create_from_utf8 ("Y"); // thorn 
    if (c == 0x00DF) return wstring_create_from_utf8 ("sz"); // esszet 
    if (c == 0x00E0) return wstring_create_from_utf8 ("a"); // accepted a 
    if (c == 0x00E1) return wstring_create_from_utf8 ("a"); // accepted a 
    if (c == 0x00E2) return wstring_create_from_utf8 ("a"); // accepted a 
    if (c == 0x00E3) return wstring_create_from_utf8 ("a"); // accepted a 
    if (c == 0x00E4) return wstring_create_from_utf8 ("a"); // accepted a 
    if (c == 0x00E5) return wstring_create_from_utf8 ("a"); // accepted a 
    if (c == 0x00E6) return wstring_create_from_utf8 ("ae"); // ae
    if (c == 0x00E7) return wstring_create_from_utf8 ("c"); // cedilla
    if (c == 0x00E8) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x00E9) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x00EA) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x00EB) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x00EC) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x00ED) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x00EE) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x00EF) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x00F0) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F1) return wstring_create_from_utf8 ("n"); //a ceepnted n 
    if (c == 0x00F2) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F3) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F4) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F5) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F6) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F7) return wstring_create_from_utf8 ("/"); // divide
    if (c == 0x00F8) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x00F9) return wstring_create_from_utf8 ("u"); //a ceepnted u 
    if (c == 0x00FA) return wstring_create_from_utf8 ("u"); //a ceepnted u 
    if (c == 0x00FB) return wstring_create_from_utf8 ("u"); //a ceepnted u 
    if (c == 0x00FC) return wstring_create_from_utf8 ("u"); //a ceepnted u 
    if (c == 0x00FD) return wstring_create_from_utf8 ("y"); //a ceepnted y 
    if (c == 0x00FE) return wstring_create_from_utf8 ("y"); //a thorn 
    if (c == 0x00FF) return wstring_create_from_utf8 ("y"); //a ceepnted y 
    if (c == 0x0100) return wstring_create_from_utf8 ("A"); //a ceepnted A 
    if (c == 0x0101) return wstring_create_from_utf8 ("a"); //a ceepnted a 
    if (c == 0x0102) return wstring_create_from_utf8 ("A"); //a ceepnted A 
    if (c == 0x0103) return wstring_create_from_utf8 ("a"); //a ceepnted a 
    if (c == 0x0104) return wstring_create_from_utf8 ("A"); //a ceepnted A 
    if (c == 0x0105) return wstring_create_from_utf8 ("a"); //a ceepnted a 
    if (c == 0x0106) return wstring_create_from_utf8 ("C"); //a ceepnted C 
    if (c == 0x0107) return wstring_create_from_utf8 ("c"); //a ceepnted c 
    if (c == 0x0108) return wstring_create_from_utf8 ("C"); //a ceepnted C 
    if (c == 0x0109) return wstring_create_from_utf8 ("c"); //a ceepnted c 
    if (c == 0x010A) return wstring_create_from_utf8 ("C"); //a ceepnted C 
    if (c == 0x010B) return wstring_create_from_utf8 ("c"); //a ceepnted c 
    if (c == 0x010C) return wstring_create_from_utf8 ("C"); //a ceepnted C 
    if (c == 0x010D) return wstring_create_from_utf8 ("c"); //a ceepnted c 
    if (c == 0x010E) return wstring_create_from_utf8 ("D"); //a ceepnted D 
    if (c == 0x010F) return wstring_create_from_utf8 ("d"); //a ceepnted d 
    if (c == 0x0110) return wstring_create_from_utf8 ("D"); //a ceepnted D 
    if (c == 0x0111) return wstring_create_from_utf8 ("d"); //a ceepnted d 
    if (c == 0x0112) return wstring_create_from_utf8 ("E"); //a ceepnted E 
    if (c == 0x0113) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x0114) return wstring_create_from_utf8 ("E"); //a ceepnted E 
    if (c == 0x0115) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x0116) return wstring_create_from_utf8 ("E"); //a ceepnted E 
    if (c == 0x0117) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x0118) return wstring_create_from_utf8 ("E"); //a ceepnted E 
    if (c == 0x0119) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x011A) return wstring_create_from_utf8 ("E"); //a ceepnted E 
    if (c == 0x011B) return wstring_create_from_utf8 ("e"); //a ceepnted e 
    if (c == 0x011C) return wstring_create_from_utf8 ("G"); //a ceepnted G 
    if (c == 0x011D) return wstring_create_from_utf8 ("g"); //a ceepnted g 
    if (c == 0x011E) return wstring_create_from_utf8 ("G"); //a ceepnted G 
    if (c == 0x011F) return wstring_create_from_utf8 ("g"); //a ceepnted g 
    if (c == 0x0120) return wstring_create_from_utf8 ("G"); //a ceepnted G 
    if (c == 0x0121) return wstring_create_from_utf8 ("g"); //a ceepnted g 
    if (c == 0x0122) return wstring_create_from_utf8 ("G"); //a ceepnted G 
    if (c == 0x0123) return wstring_create_from_utf8 ("g"); //a ceepnted g 
    if (c == 0x0124) return wstring_create_from_utf8 ("H"); //a ceepnted H 
    if (c == 0x0125) return wstring_create_from_utf8 ("h"); //a ceepnted h 
    if (c == 0x0126) return wstring_create_from_utf8 ("H"); //a ceepnted H 
    if (c == 0x0127) return wstring_create_from_utf8 ("h"); //a ceepnted h 
    if (c == 0x0128) return wstring_create_from_utf8 ("I"); //a ceepnted I 
    if (c == 0x0129) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x012A) return wstring_create_from_utf8 ("I"); //a ceepnted I 
    if (c == 0x012B) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x012C) return wstring_create_from_utf8 ("I"); //a ceepnted I 
    if (c == 0x012D) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x012E) return wstring_create_from_utf8 ("I"); //a ceepnted I 
    if (c == 0x012F) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x0130) return wstring_create_from_utf8 ("I"); //a ceepnted I 
    if (c == 0x0131) return wstring_create_from_utf8 ("i"); //a ceepnted i 
    if (c == 0x0132) return wstring_create_from_utf8 ("IJ"); 
    if (c == 0x0133) return wstring_create_from_utf8 ("ij"); 
    if (c == 0x0134) return wstring_create_from_utf8 ("J"); //a ceepnted J 
    if (c == 0x0135) return wstring_create_from_utf8 ("j"); //a ceepnted j 
    if (c == 0x0136) return wstring_create_from_utf8 ("K"); //a ceepnted K 
    if (c == 0x0138) return wstring_create_from_utf8 ("K"); //a ceepnted K 
    if (c == 0x0138) return wstring_create_from_utf8 ("k"); //a ceepnted k 
    if (c == 0x0139) return wstring_create_from_utf8 ("L"); //a ceepnted L 
    if (c == 0x013A) return wstring_create_from_utf8 ("l"); //a ceepnted l 
    if (c == 0x013B) return wstring_create_from_utf8 ("L"); //a ceepnted L 
    if (c == 0x013C) return wstring_create_from_utf8 ("l"); //a ceepnted l 
    if (c == 0x013D) return wstring_create_from_utf8 ("L"); //a ceepnted L 
    if (c == 0x013E) return wstring_create_from_utf8 ("l"); //a ceepnted l 
    if (c == 0x013F) return wstring_create_from_utf8 ("L"); //a ceepnted L 
    if (c == 0x0140) return wstring_create_from_utf8 ("l"); //a ceepnted l 
    if (c == 0x0141) return wstring_create_from_utf8 ("L"); //a ceepnted L 
    if (c == 0x0142) return wstring_create_from_utf8 ("l"); //a ceepnted l 
    if (c == 0x0143) return wstring_create_from_utf8 ("N"); //a ceepnted N 
    if (c == 0x0144) return wstring_create_from_utf8 ("n"); //a ceepnted N 
    if (c == 0x0145) return wstring_create_from_utf8 ("N"); //a ceepnted N 
    if (c == 0x0146) return wstring_create_from_utf8 ("n"); //a ceepnted N 
    if (c == 0x0147) return wstring_create_from_utf8 ("N"); //a ceepnted N 
    if (c == 0x0148) return wstring_create_from_utf8 ("n"); //a ceepnted N 
    if (c == 0x0149) return wstring_create_from_utf8 ("N"); //a ceepnted N 
    if (c == 0x014A) return wstring_create_from_utf8 ("n"); //a ceepnted N 
    if (c == 0x014B) return wstring_create_from_utf8 ("n"); //a ceepnted n 
    if (c == 0x014C) return wstring_create_from_utf8 ("O"); //a ceepnted O 
    if (c == 0x014D) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x014E) return wstring_create_from_utf8 ("O"); //a ceepnted O 
    if (c == 0x014F) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x0150) return wstring_create_from_utf8 ("O"); //a ceepnted O 
    if (c == 0x0151) return wstring_create_from_utf8 ("o"); //a ceepnted o 
    if (c == 0x0152) return wstring_create_from_utf8 ("OE"); 
    if (c == 0x0153) return wstring_create_from_utf8 ("oe"); 
    if (c == 0x0154) return wstring_create_from_utf8 ("R"); // accepted R
    if (c == 0x0155) return wstring_create_from_utf8 ("r"); // accepted r
    if (c == 0x0156) return wstring_create_from_utf8 ("R"); // accepted R
    if (c == 0x0157) return wstring_create_from_utf8 ("r"); // accepted r
    if (c == 0x0158) return wstring_create_from_utf8 ("R"); // accepted R
    if (c == 0x0159) return wstring_create_from_utf8 ("r"); // accepted r
    if (c == 0x015A) return wstring_create_from_utf8 ("S"); // accepted S
    if (c == 0x015B) return wstring_create_from_utf8 ("s"); // accepted s
    if (c == 0x015C) return wstring_create_from_utf8 ("S"); // accepted S
    if (c == 0x015D) return wstring_create_from_utf8 ("s"); // accepted s
    if (c == 0x015E) return wstring_create_from_utf8 ("S"); // accepted S
    if (c == 0x015F) return wstring_create_from_utf8 ("s"); // accepted s
    if (c == 0x0160) return wstring_create_from_utf8 ("S"); // accepted S
    if (c == 0x0161) return wstring_create_from_utf8 ("s"); // accepted s
    if (c == 0x0162) return wstring_create_from_utf8 ("T"); // accepted T
    if (c == 0x0163) return wstring_create_from_utf8 ("t"); // accepted t
    if (c == 0x0164) return wstring_create_from_utf8 ("T"); // accepted T
    if (c == 0x0165) return wstring_create_from_utf8 ("t"); // accepted t
    if (c == 0x0166) return wstring_create_from_utf8 ("T"); // accepted T
    if (c == 0x0167) return wstring_create_from_utf8 ("t"); // accepted t
    if (c == 0x0168) return wstring_create_from_utf8 ("U"); // accepted U
    if (c == 0x0169) return wstring_create_from_utf8 ("u"); // accepted u
    if (c == 0x016A) return wstring_create_from_utf8 ("U"); // accepted U
    if (c == 0x016B) return wstring_create_from_utf8 ("u"); // accepted u
    if (c == 0x016C) return wstring_create_from_utf8 ("U"); // accepted U
    if (c == 0x016D) return wstring_create_from_utf8 ("u"); // accepted u
    if (c == 0x016E) return wstring_create_from_utf8 ("U"); // accepted U
    if (c == 0x016F) return wstring_create_from_utf8 ("u"); // accepted u
    if (c == 0x0170) return wstring_create_from_utf8 ("U"); // accepted U
    if (c == 0x0171) return wstring_create_from_utf8 ("u"); // accepted u
    if (c == 0x0172) return wstring_create_from_utf8 ("U"); // accepted U
    if (c == 0x0173) return wstring_create_from_utf8 ("u"); // accepted u
    if (c == 0x0174) return wstring_create_from_utf8 ("W"); // accepted W
    if (c == 0x0175) return wstring_create_from_utf8 ("w"); // accepted w
    if (c == 0x0176) return wstring_create_from_utf8 ("Y"); // accepted Y
    if (c == 0x0177) return wstring_create_from_utf8 ("y"); // accepted y
    if (c == 0x0178) return wstring_create_from_utf8 ("Y"); // accepted Y
    if (c == 0x00) return wstring_create_from_utf8 (""); // 
    wstring_append_c (ret, c);
    }
  else
    wstring_append_c (ret, c);
  return ret; 
  }


/*============================================================================
  xhtml_translate_entity
============================================================================*/
WString *xhtml_translate_entity (const WString *entity)
  {
  /* Program flow in this function is very ugly, and prone to memory
     leaks when modified. The whole thing needs to be rewritten */
  char out[20];
  IN
  char *in = wstring_to_utf8 (entity);
  if (strcasecmp (in, "amp") == 0) 
    strcpy (out, "&");
  else if (strcasecmp (in, "nbsp") == 0) 
    strcpy (out, " ");
  else if (strcasecmp (in, "lt") == 0) 
    strcpy (out, "<");
  else if (strcasecmp (in, "gt") == 0) 
    strcpy (out, ">");
  else if (strcasecmp (in, "cent") == 0) 
    strcpy (out, "¢");
  else if (strcasecmp (in, "pound") == 0) 
    strcpy (out, "£");
  else if (strcasecmp (in, "yen") == 0) 
    strcpy (out, "£");
  else if (strcasecmp (in, "euro") == 0) 
    strcpy (out, "€");
  else if (strcasecmp (in, "sect") == 0) 
    strcpy (out, "§");
  else if (strcasecmp (in, "copy") == 0) 
    strcpy (out, "©");
  else if (strcasecmp (in, "reg") == 0) 
    strcpy (out, "®");
  else if (strcasecmp (in, "trade") == 0) 
    strcpy (out, "™");
  else if (strcasecmp (in, "quot") == 0) 
    strcpy (out, "\"");
  else if (in[0] == '#')
    {
    char *s = strdup (in);
    s[0] = '0';
    int v = 0;
    if (sscanf (s, "%d", &v) == 1)
      {
      WString *ret = wstring_create_empty();
      wstring_append_c (ret, (uint32_t)v);
      OUT
      free (s);
      free (in);
      return ret; 
      } 
    free (s);
    }
  else 
    {
    strncpy (out, in, sizeof (out) - 1);
    out[sizeof (out) - 1] = 0;
    }
  free (in);
  OUT
  return wstring_create_from_utf8 (out);
  }


/*============================================================================
  xhtml_flush_line
============================================================================*/
void xhtml_flush_line (const WString *para, const Epub2TxtOptions *options,
     WrapTextContext *context) 
  {
  IN

  if (options->raw)
    {
    char *s = wstring_to_utf8 (para);
    fputs (s, stdout); 
    free (s);
    }
  else
    {
    wraptext_wrap_utf32 (context, wstring_wstr (para));
    wraptext_eof (context);
    }

  OUT
  }


/*============================================================================
  xhtml_flush_para
============================================================================*/
void xhtml_flush_para (const WString *para, const Epub2TxtOptions *options,
     WrapTextContext *context) 
  {
  IN

  xhtml_flush_line (para, options, context);

  OUT
  }


/*============================================================================
  xhtml_line_break
============================================================================*/
void xhtml_line_break (WrapTextContext *context) 
  {
  IN
  //static uint32_t s[2] = { '\n', 0 };
  static uint32_t s[2] = { WT_HARD_LINE_BREAK, 0 };
  wraptext_wrap_utf32 (context, s);
  wraptext_eof (context);
  OUT
  }


/*============================================================================
  xhtml_para_break
============================================================================*/
void xhtml_para_break (WrapTextContext *context, 
      const Epub2TxtOptions *options) 
  {
  IN
  static uint32_t s[3] = { '\n', '\n', 0 };
  if (options->raw)
    {
    printf ("\n\n");
    }
  else
    { 
    wraptext_wrap_utf32 (context, s);
    }
  OUT
  }


/*============================================================================
  xhtml_all_white
  Note that, for the purpses of application logic, an empty string
  is considered to be whitespace
============================================================================*/
BOOL xhtml_all_white (WString *s) 
  {
  if (wstring_length (s) == 0) return TRUE;
  return wstring_is_whitespace (s);
  }


/*============================================================================
  xhtml_utf8_to_stdout
============================================================================*/
void xhtml_utf8_to_stdout (const char *s, const Epub2TxtOptions *options, 
       char **error)
  {
  IN
  char *ss;
  // This is all a bit ugly. The entity translation is in 
  //  xhtml_to_stdout, which expects something that looks like a viable
  //  XHTML file. There's no guarantee that the input to this function
  //  will actually be a full XHTML file, so we must wrap it in a body
  //  to fool xhtml_to_stdout. Ugh.
  asprintf (&ss, "<body>%s</body>", s);
  WString *sw = wstring_create_from_utf8 (ss); 
  xhtml_to_stdout (sw, options, error);
  wstring_destroy (sw);
  free (ss);
  OUT
  }

/*============================================================================
  xhtml_file_to_stdout
============================================================================*/
void xhtml_file_to_stdout (const char *filename, const Epub2TxtOptions *options, 
             char **error)
  {
  IN
  log_debug ("Process XHTML file %s", filename);

  WString *s;
  wstring_create_from_utf8_file (filename, &s, error); 
  if (*error == NULL)
     {
     xhtml_to_stdout (s, options, error);
     wstring_destroy (s);
     }

  OUT
  }

/*============================================================================
  xhtml_to_stdout
============================================================================*/
void xhtml_to_stdout (const WString *s, const Epub2TxtOptions *options, 
             char **error)
  {
  IN
  log_debug ("Process XHTML string");

  typedef enum {MODE_ANY=0, MODE_INTAG = 1, MODE_ENTITY = 2} Mode;

  if (TRUE)
     {
     int width;
     if (options->width <= 0)
       width = INT_MAX;
     else
       width = options->width - 1;

     WrapTextContext *context = wraptext_context_new();
     wraptext_context_set_width (context, width);
     wraptext_context_set_app_opts (context, (void *)options);

     Mode mode = MODE_ANY;
     BOOL inbody = FALSE;
     BOOL can_newline = FALSE;
     WString *tag = wstring_create_empty();
     WString *entity = wstring_create_empty();
     WString *para = wstring_create_empty();
     WString *ruby = wstring_create_empty();
     BOOL inruby = FALSE;
     int i, l = wstring_length (s);
     uint32_t last_c = 0;
     int taglen = 0;

     const uint32_t *text = wstring_wstr (s);
     for (i = 0; i < l; i++)
       {
       uint32_t c = text[i];  
       if (c == 13) // DOS EOL
         continue;

        if (c == 9) // Tab
            c = ' ';

	//printf ("c=%c %04x\n", (char)c, c);
	if (mode == MODE_ANY && c == '<')
	  {
          taglen = 0;
	  mode = MODE_INTAG;
	  }
	else if (mode == MODE_ANY && c == '\n')
	  {
	  if (inbody)
	    {
	    if (last_c != ' ')
	      {
	      wstring_append_c (para, ' ');
	      }
	    }
	  }
	else if (mode == MODE_ANY && c == '&')
	  {
	  mode = MODE_ENTITY;
	  }
	else if (mode == MODE_ANY)
	  {
	  if (inbody)
	    {
	    if (c == ' ' && last_c == ' ')
	      {
	      }
	    else
	      {
	      WString *s = xhtml_transform_char (c, options->ascii);
	      wstring_append (inruby ? ruby : para, s);
	      wstring_destroy (s);
	      }
	    }
	  }
	else if (mode == MODE_ENTITY && c == ';')
	  {
	  if (inbody)
	    {
	    WString *trans = xhtml_translate_entity (entity);
	    wstring_append (inruby ? ruby : para, trans);
	    wstring_destroy (trans);
	    }
	  wstring_clear (entity);
	  mode = MODE_ANY;
	  }
	else if (mode == MODE_ENTITY)
	  {
	  wstring_append_c (entity, c);
	  }
	else if (mode == MODE_INTAG && c == '>')
	  {
          taglen = 0;
          Format format = FORMAT_NONE;
	  char *ss_tag = wstring_to_utf8 (tag);
	  char *p = strchr (ss_tag, ' ');
	  if (p) *p = 0;
	  if (strcasecmp (ss_tag, "body") == 0) 
	    {
	    inbody = TRUE;
	    }
	  else if (strcasecmp (ss_tag, "/body") == 0) 
	    {
	    if (xhtml_all_white (para))
	      can_newline = FALSE; 
	    else
	      can_newline = TRUE; 
	    xhtml_flush_para (para, options, context); 
	    wstring_clear (para);
	    if (can_newline)
	      {
	      xhtml_para_break (context, options);
	      can_newline = FALSE;
	      }
	    inbody = FALSE;
	    }
	  else if ((strcasecmp (ss_tag, "p/") == 0) 
	      || (strcasecmp (ss_tag, "/p") == 0))
	    {
	    if (inbody)
	      {
	      if (xhtml_all_white (para))
		can_newline = FALSE; 
	      else
		{
		can_newline = TRUE; 
		}
	      xhtml_flush_para (para, options, context);
	      wstring_clear (para);
	      if (can_newline)
		{
		xhtml_para_break (context, options);
		can_newline = FALSE;
		}
	      }
	    }
	  else if ((strcasecmp (ss_tag, "br/") == 0) 
	      || (strcasecmp (ss_tag, "br") == 0)
	      || (strcasecmp (ss_tag, "br /") == 0))
	    {
	    if (inbody)
	      {
	      if (xhtml_all_white (para))
		can_newline = FALSE; 
	      else
		can_newline = TRUE; 
	      xhtml_flush_para (para, options, context);
	      wstring_clear (para);
	      if (can_newline)
		{
		xhtml_line_break (context);
		can_newline = FALSE;
		}
	      }
	    }
	  else if (xhtml_is_start_format_tag (ss_tag, &format))
	    {
	    if (inbody)
	      {
	      xhtml_flush_line (para, options, context); 
	      wstring_clear (para);
              xhtml_emit_format (options, format);
              xhtml_set_format (options, format, context);
	      }
	    }
	  else if (xhtml_is_end_format_tag (ss_tag, &format))
	    {
	    if (inbody)
	      {
	      xhtml_flush_line (para, options, context); 
              xhtml_emit_format (options, format);
	      xhtml_set_format (options, format, context);
	      wstring_clear (para);
	      }
            }
	  else if (xhtml_is_end_breaking_tag (ss_tag, &format))
	    {
            xhtml_flush_line (para, options, context);
            xhtml_emit_format (options, format);
            xhtml_set_format (options, format, context);
	    wstring_clear (para);
	    xhtml_para_break (context, options);
            }

	  else if (xhtml_is_start_breaking_tag (ss_tag, &format))
	    {
            xhtml_flush_line (para, options, context);
	    wstring_clear (para);
            xhtml_emit_format (options, format);
            xhtml_set_format (options, format, context);
            }

    else if (strcasecmp(ss_tag, "ruby") == 0)
      {
      wstring_clear (ruby);
      }
    else if (strcasecmp(ss_tag, "/ruby") == 0)
      {
        // Append concatenated ruby annotations
      wstring_append_c (para, '(');
      wstring_append (para, ruby);
      wstring_append_c (para, ')');
      wstring_clear (ruby);
      }
    else if (strcasecmp(ss_tag, "rt") == 0)
      {
          // Start accumulating ruby annotations
        inruby = TRUE;
      }
    else if (strcasecmp(ss_tag, "/rt") == 0)
      {
        inruby = FALSE;
      }

	  free (ss_tag);
	  wstring_clear (tag);
	  mode = MODE_ANY;
	  }
	else if (mode == MODE_INTAG)
	 {
         taglen++;
         // Bug #5 -- Added support to abort tag reading if tag > 1000 
         //   characters. This is an arbitrary number, but it's larger than
         //   any tag that we can handle. 
         if (taglen > 1000)
           {
           while (i < l)
             {
             uint32_t c = text[i];  
             if (c == (uint32_t)'>')
               {
               wstring_clear (tag);
               }
             i++;
             }
           }
	 wstring_append_c (tag, c);
	 }
	else
	  log_error ("Unexpected character %d in mode %d", c, mode);
	last_c = c;
        }
     if (wstring_length (para) > 0)
      xhtml_flush_para (para, options, context); 

     wstring_destroy (tag);
     wstring_destroy (entity);
     wstring_destroy (para);
     wstring_destroy (ruby);

     wraptext_eof (context);
     wraptext_context_free (context);
     }
  OUT
  }







