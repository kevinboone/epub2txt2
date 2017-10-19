/*
    This file is part of sxmlc.

    sxmlc is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sxmlc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with sxmlc.  If not, see <http://www.gnu.org/licenses/>.

	Copyright 2010 - Matthieu Labas
*/
#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SXMLC_UNICODE
typedef wchar_t SXML_CHAR;
#define C2SX(c) L ## c
#define CEOF WEOF
#define sx_strcmp wcscmp
#define sx_strncmp wcsncmp
#define sx_strlen wcslen
#define sx_strdup wcsdup
#define sx_strchr wcschr
#define sx_strrchr wcsrchr
#define sx_strcpy wcscpy
#define sx_strncpy wcsncpy
#define sx_strcat wcscat
#define sx_printf wprintf
#define sx_fprintf fwprintf
#define sx_sprintf swprintf
#define sx_fgetc fgetwc
#define sx_fputc fputwc
#define sx_isspace iswspace
#if defined(WIN32) || defined(WIN64)
#define sx_fopen _wfopen
#else
#define sx_fopen fopen
#endif
#define sx_fclose fclose
#else
typedef char SXML_CHAR;
#define C2SX(c) c
#define CEOF EOF
#define sx_strcmp strcmp
#define sx_strncmp strncmp
#define sx_strlen strlen
#define sx_strdup __strdup
#define sx_strchr strchr
#define sx_strrchr strrchr
#define sx_strcpy strcpy
#define sx_strncpy strncpy
#define sx_strcat strcat
#define sx_printf printf
#define sx_fprintf fprintf
#define sx_sprintf sprintf
#define sx_fgetc fgetc
#define sx_fputc fputc
#define sx_isspace isspace
#define sx_fopen fopen
#define sx_fclose fclose
#endif

//#define DBG_MEM

#ifdef DBG_MEM
void* __malloc(size_t sz);
void* __calloc(size_t count, size_t sz);
void* __realloc(void* mem, size_t sz);
void __free(void* mem);
char* __strdup(const char* s);
#else
#define __malloc malloc
#define __calloc calloc
#define __realloc realloc
#define __free free
#undef __strdup
#define __strdup strdup
#endif

#ifndef MEM_INCR_RLA
#define MEM_INCR_RLA (256*sizeof(SXML_CHAR)) /* Initial buffer size and increment for memory reallocations */
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#define NULC ((SXML_CHAR)C2SX('\0'))

#define isquote(c) (((c) == C2SX('"')) || ((c) == C2SX('\'')))

/*
 Buffer data source used by 'read_line_alloc' when required.
 'buf' should be 0-terminated.
 */
typedef struct _DataSourceBuffer {
	const SXML_CHAR* buf;
	int cur_pos;
} DataSourceBuffer;

typedef FILE* DataSourceFile;

typedef enum _DataSourceType {
	DATA_SOURCE_FILE = 0,
	DATA_SOURCE_BUFFER,
	DATA_SOURCE_MAX
} DataSourceType;

/*
 Functions to get next byte from buffer data source and know if the end has been reached.
 Return as 'fgetc' and 'feof' would for 'FILE*'.
 */
int _bgetc(DataSourceBuffer* ds);
int _beob(DataSourceBuffer* ds);
/*
 Reads a line from data source 'in', eventually (re-)allocating a given buffer 'line'.
 Characters read will be stored in 'line' starting at 'i0' (this allows multiple calls to
 'read_line_alloc' on the same 'line' buffer without overwriting it at each call).
 'in_type' specifies the type of data source to be read: 'in' is 'FILE*' if 'in_type'
 'sz_line' is the size of the buffer 'line' if previously allocated. 'line' can point
 to NULL, in which case it will be allocated '*sz_line' bytes. After the function returns,
 '*sz_line' is the actual buffer size. This allows multiple calls to this function using the
 same buffer (without re-allocating/freeing).
 If 'sz_line' is non NULL and non 0, it means that '*line' is a VALID pointer to a location
 of '*sz_line' SXML_CHAR (not bytes! Multiply by sizeof(SXML_CHAR) to get number of bytes).
 Searches for character 'from' until character 'to'. If 'from' is 0, starts from
 current position. If 'to' is 0, it is replaced by '\n'.
 If 'keep_fromto' is 0, removes characters 'from' and 'to' from the line.
 If 'interest_count' is not NULL, will receive the count of 'interest' characters while searching
 for 'to' (e.g. use 'interest'='\n' to count lines in file).
 Returns the number of characters in the line or 0 if an error occurred.
 'read_line_alloc' uses constant 'MEM_INCR_RLA' to reallocate memory when needed. It is possible
 to override this definition to use another value.
 */
int read_line_alloc(void* in, DataSourceType in_type, SXML_CHAR** line, int* sz_line, int i0, SXML_CHAR from, SXML_CHAR to, int keep_fromto, SXML_CHAR interest, int* interest_count);

/*
 Concatenates the string pointed at by 'src1' with 'src2' into '*src1' and
 return it ('*src1').
 Return NULL when out of memory.
 */
SXML_CHAR* strcat_alloc(SXML_CHAR** src1, const SXML_CHAR* src2);

/*
 Strip spaces at the beginning and end of 'str', modifying 'str'.
 If 'repl_sq' is not '\0', squeezes spaces to an single character ('repl_sq').
 If not '\0', 'protect' is used to protect spaces from being deleted (usually a backslash).
 Returns the string or NULL if 'protect' is a space (which would not make sense).
 */
SXML_CHAR* strip_spaces(SXML_CHAR* str, SXML_CHAR repl_sq);

/*
 Remove '\' characters from 'str', modifying it.
 Return 'str'.
 */
SXML_CHAR* str_unescape(SXML_CHAR* str);

/*
 Split 'str' into a left and right part around a separator 'sep'.
 The left part is located between indexes 'l0' and 'l1' while the right part is
 between 'r0' and 'r1' and the separator position is at 'i_sep' (whenever these are
 not NULL).
 If 'ignore_spaces' is 'true', computed indexes will not take into account potential
 spaces around the separator as well as before left part and after right part.
 if 'ignore_quotes' is 'true', " or ' will not be taken into account when parsing left
 and right members.
 Whenever the right member is empty (e.g. "attrib" or "attrib="), '*r0' is initialized
 to 'str' size and '*r1' to '*r0-1' (crossed).
 If the separator was not found (i.e. left member only), '*i_sep' is '-1'.
 Return 'false' when 'str' is malformed, 'true' when splitting was successful.
 */
int split_left_right(SXML_CHAR* str, SXML_CHAR sep, int* l0, int* l1, int* i_sep, int* r0, int* r1, int ignore_spaces, int ignore_quotes);

typedef enum _BOM_TYPE {
	BOM_NONE = 0x00,
	BOM_UTF_8 = 0xefbbbf,
	BOM_UTF_16BE = 0xfeff,
	BOM_UTF_16LE = 0xfffe,
	BOM_UTF_32BE = 0x0000feff,
	BOM_UTF_32LE = 0xfffe0000
} BOM_TYPE;
/*
 Detect a potential BOM at the current file position and read it into 'bom' (if not NULL,
 'bom' should be at least 5 bytes). It also moves the 'f' beyond the BOM so it's possible to
 skip it by calling 'freadBOM(f, NULL, NULL)'. If no BOM is found, it leaves 'f' file pointer
 is reset to its original location.
 If not null, 'sz_bom' is filled with how many bytes are stored in 'bom'.
 Return the BOM type or BOM_NONE if none found (empty 'bom' in this case).
 */
BOM_TYPE freadBOM(FILE* f, unsigned char* bom, int* sz_bom);

/*
 Replace occurrences of special HTML characters escape sequences (e.g. '&amp;') found in 'html'
 by its character equivalent (e.g. '&') into 'str'.
 If 'html' and 'str' are the same pointer replacement is made in 'str' itself, overwriting it.
 If 'str' is NULL, replacement is made into 'html', overwriting it.
 Returns 'str' (or 'html' if 'str' was NULL).
 */
SXML_CHAR* html2str(SXML_CHAR* html, SXML_CHAR* str);

/*
 Replace occurrences of special characters (e.g. '&') found in 'str' into their HTML escaped
 equivalent (e.g. '&amp;') into 'html'.
 'html' is supposed allocated to the correct size (e.g. using 'malloc(strlen_html(str))') and
 different from 'str' (unlike 'html2str'), as string will expand.
 Return 'html' or NULL if 'str' or 'html' are NULL, or when 'html' is 'str'.
*/
SXML_CHAR* str2html(SXML_CHAR* str, SXML_CHAR* html);

/*
 Return the length of 'str' as if all its special character were replaced by their HTML
 equivalent.
 Return 0 if 'str' is NULL.
 */
int strlen_html(SXML_CHAR* str);

/*
 Print 'str' to 'f', transforming special characters into their HTML equivalent.
 Returns the number of output characters.
 */
int fprintHTML(FILE* f, SXML_CHAR* str);

/*
 Checks whether 'str' corresponds to 'pattern'.
 'pattern' can use wildcads such as '*' (any potentially empty string) or
 '?' (any character) and use '\' as an escape character.
 Returns 'true' when 'str' matches 'pattern', 'false' otherwise.
 */
int regstrcmp(SXML_CHAR* str, SXML_CHAR* pattern);

#ifdef __cplusplus
}
#endif

#endif
