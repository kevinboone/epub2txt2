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
#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sxmlutils.h"
#include "log.h"

#ifdef DBG_MEM
static int nb_alloc = 0, nb_free = 0;
void* __malloc(size_t sz)
{
	void* p = malloc(sz);
	if (p != NULL) nb_alloc++;
	printf("0x%x: MALLOC (%d) - NA %d - NF %d = %d\n", p, sz, nb_alloc, nb_free, nb_alloc - nb_free);
	return p;
}
void* __calloc(size_t count, size_t sz)
{
	void* p = calloc(count, sz);
	if (p != NULL) nb_alloc++;
	printf("0x%x: CALLOC (%d, %d) - NA %d - NF %d = %d\n", p, count, sz, nb_alloc, nb_free, nb_alloc - nb_free);
	return p;
}
void* __realloc(void* mem, size_t sz)
{
	void* p = realloc(mem, sz);
	if (mem == NULL && p != NULL) nb_alloc++;
	printf("0x%x: REALLOC 0x%x (%d)", p, mem, sz);
	if (mem == NULL)
		printf(" - NA %d - NF %d = %d", nb_alloc, nb_free, nb_alloc - nb_free);
	printf("\n");
	return p;
}
void __free(void* mem)
{
	nb_free++;
	printf("0x%x: FREE - NA %d - NF %d = %d\n", mem, nb_alloc, nb_free, nb_alloc - nb_free);
	free(mem);
}
char* __strdup(const char* s)
{
	char* p = sx_strdup(s);
	if (p != NULL) nb_alloc++;
	printf("0x%x: STRDUP (%d) - NA %d - NF %d = %d\n", p, sx_strlen(s), nb_alloc, nb_free, nb_alloc - nb_free);
	return p;
}
#endif

/* Dictionary of special characters and their HTML equivalent */
static struct _html_special_dict {
	SXML_CHAR chr;		/* Original character */
	SXML_CHAR* html;		/* Equivalent HTML string */
	int html_len;	/* 'sx_strlen(html)' */
} HTML_SPECIAL_DICT[] = {
	{ C2SX('<'), C2SX("&lt;"), 4 },
	{ C2SX('>'), C2SX("&gt;"), 4 },
	{ C2SX('"'), C2SX("&quot;"), 6 },
	{ C2SX('&'), C2SX("&amp;"), 5 },
	{ NULC, NULL, 0 }, /* Terminator */
};

int _bgetc(DataSourceBuffer* ds)
{
	if (ds == NULL || ds->buf[ds->cur_pos] == NULC) return EOF;
	
	return (int)(ds->buf[ds->cur_pos++]);
}

int _beob(DataSourceBuffer* ds)
{

	if (ds == NULL || ds->buf[ds->cur_pos] == NULC) return true;

	return false;
}

int read_line_alloc(void* in, DataSourceType in_type, SXML_CHAR** line, int* sz_line, int i0, SXML_CHAR from, SXML_CHAR to, int keep_fromto, SXML_CHAR interest, int* interest_count)
{
	int init_sz = 0;
	SXML_CHAR c, *pt;
	int n, ret;
	int (*mgetc)(void* ds) = (in_type == DATA_SOURCE_BUFFER ? (int(*)(void*))_bgetc : (int(*)(void*))sx_fgetc);
	int (*meos)(void* ds) = (in_type == DATA_SOURCE_BUFFER ? (int(*)(void*))_beob : (int(*)(void*))feof);
	
	if (in == NULL || line == NULL) 
          {
          return 0;
          }
	
	if (to == NULC) to = C2SX('\n');
	/* Search for character 'from' */
	if (interest_count != NULL) *interest_count = 0;
	while (true) {
            
		c = (SXML_CHAR)mgetc(in);
		if (interest_count != NULL && c == interest) (*interest_count)++;
		/* Reaching EOF before 'to' char is not an error but should trigger 'line' alloc and init to '' */
		/* If 'from' is '\0', we stop here */
		if (c == from || c == CEOF || from == NULC) break;
	}
	
	if (sz_line == NULL) sz_line = &init_sz;
	
	if (*line == NULL || *sz_line == 0) {
		if (*sz_line == 0) *sz_line = MEM_INCR_RLA;
		*line = (SXML_CHAR*)__malloc(*sz_line*sizeof(SXML_CHAR));
		if (*line == NULL) return 0;
	}
	if (i0 < 0) i0 = 0;
	if (i0 > *sz_line) return 0;
	
	n = i0;
	if (c == CEOF) { /* EOF reached before 'to' char => return the empty string */
		(*line)[n] = NULC;
		return meos(in) ? n : 0; /* Error if not EOF */
	}
	if (c != from || keep_fromto)
		(*line)[n++] = c;
	(*line)[n] = NULC;
	ret = 0;
	while (true) {
		c = (SXML_CHAR)mgetc(in);
		if (interest_count != NULL && c == interest) (*interest_count)++;
		if ((char)c == (char)CEOF) { /* EOF or error */
			(*line)[n] = NULC;
			ret = meos(in) ? n : 0;
			break;
		} else {
			(*line)[n] = c;
			if (c != to || (keep_fromto && to != NULC && c == to)) n++; /* If we reached the 'to' character and we keep it, we still need to add the extra '\0' */
			if (n >= *sz_line) { /* Too many characters for our line => realloc some more */
				*sz_line += MEM_INCR_RLA;
				pt = (SXML_CHAR*)__realloc(*line, *sz_line*sizeof(SXML_CHAR));
				if (pt == NULL) {
					ret = 0;
					break;
				} else
					*line = pt;
			}
			(*line)[n] = NULC; /* If we reached the 'to' character and we want to strip it, 'n' hasn't changed and 'line[n]' (which is 'to') will be replaced by '\0' */
			if (c == to) {
				ret = n;
				break;
			}
		}

	}
	
#if 0 /* Automatic buffer resize is deactivated */
	/* Resize line to the exact size */
	pt = (SXML_CHAR*)__realloc(*line, (n+1)*sizeof(SXML_CHAR));
	if (pt != NULL)
		*line = pt;
#endif
	
	return ret;
}

/* --- */

SXML_CHAR* strcat_alloc(SXML_CHAR** src1, const SXML_CHAR* src2)
{
	SXML_CHAR* cat;
	int n;

	if (src1 == NULL || *src1 == src2) return NULL; /* Do not concatenate '*src1' with itself */

	/* Concatenate a NULL or empty string */
	if (src2 == NULL || *src2 == NULC) return *src1;

	n = (*src1 == NULL ? 0 : sx_strlen(*src1)) + sx_strlen(src2) + 1;
	cat = (SXML_CHAR*)__realloc(*src1, n*sizeof(SXML_CHAR));
	if (cat == NULL) return NULL;
	if (*src1 == NULL) *cat = NULC;
	*src1 = cat;
	sx_strcat(*src1, src2);

	return *src1;
}

SXML_CHAR* strip_spaces(SXML_CHAR* str, SXML_CHAR repl_sq)
{
	SXML_CHAR* p;
	int i, len;
	
	/* 'p' to the first non-space */
	for (p = str; *p && sx_isspace(*p); p++) ; /* No need to search for 'protect' as it is not a space */
	len = sx_strlen(str);
	for (i = len-1; sx_isspace(str[i]); i--) ;
	if (str[i] == C2SX('\\')) i++; /* If last non-space is the protection, keep the last space */
	str[i+1] = NULC; /* New end of string to last non-space */
	
	if (repl_sq == NULC) {
		if (p == str && i == len) return str; /* Nothing to do */
		for (i = 0; (str[i] = *p) != NULC; i++, p++) ; /* Copy 'p' to 'str' */
		return str;
	}
	
	/* Squeeze all spaces with 'repl_sq' */
	i = 0;
	while (*p != NULC) {
		if (sx_isspace(*p)) {
			str[i++] = repl_sq;
			while (sx_isspace(*++p)) ; /* Skips all next spaces */
		} else {
			if (*p == C2SX('\\')) p++;
			str[i++] = *p++;
		}
	}
	str[i] = NULC;
	
	return str;
}

SXML_CHAR* str_unescape(SXML_CHAR* str)
{
	int i, j;

	if (str == NULL) return NULL;

	for (i = j = 0; str[j]; j++) {
		if (str[j] == C2SX('\\')) j++;
		str[i++] = str[j];
	}

	return str;
}

int split_left_right(SXML_CHAR* str, SXML_CHAR sep, int* l0, int* l1, int* i_sep, int* r0, int* r1, int ignore_spaces, int ignore_quotes)
{
	int n0, n1, is;
	SXML_CHAR quote = 0;

	if (str == NULL) return false;

	if (i_sep != NULL) *i_sep = -1;

	if (!ignore_spaces) ignore_quotes = false; /* No sense of ignore quotes if spaces are to be kept */

	/* Parse left part */

	if (ignore_spaces) {
		for (n0 = 0; str[n0] && sx_isspace(str[n0]); n0++) ; /* Skip head spaces, n0 points to first non-space */
		if (ignore_quotes && isquote(str[n0])) { /* If quote is found, look for next one */
			quote = str[n0++]; /* Quote can be '\'' or '"' */
			for (n1 = n0; str[n1] && str[n1] != quote; n1++) {
				if (str[n1] == C2SX('\\') && str[++n1] == NULC) break; /* Escape character (can be the last) */
			}
			for (is = n1 + 1; str[is] && sx_isspace(str[is]); is++) ; /* '--' not to take quote into account */
		} else {
			for (n1 = n0; str[n1] && str[n1] != sep && !sx_isspace(str[n1]); n1++) ; /* Search for separator or a space */
			for (is = n1; str[is] && sx_isspace(str[is]); is++) ;
		}
	} else {
		n0 = 0;
		for (n1 = 0; str[n1] && str[n1] != sep; n1++) ; /* Search for separator only */
		if (str[n1] != sep) return false; /* Separator not found: malformed string */
		is = n1;
	}

	/* Here 'n0' is the start of left member, 'n1' is the character after the end of left member */

	if (l0 != NULL) *l0 = n0;
	if (l1 != NULL) *l1 = n1 - 1;
	if (i_sep != NULL) *i_sep = is;
	if (str[is] == NULC || str[is+1] == NULC) { /* No separator => empty right member */
		if (r0 != NULL) *r0 = is;
		if (r1 != NULL) *r1 = is-1;
		if (i_sep != NULL) *i_sep = (str[is] == NULC ? -1 : is);
		return true;
	}

	/* Parse right part */

	n0 = is + 1;
	if (ignore_spaces) {
		for (; str[n0] && sx_isspace(str[n0]); n0++) ;
		if (ignore_quotes && isquote(str[n0])) quote = str[n0];
	}

	for (n1 = ++n0; str[n1]; n1++) {
		if (ignore_quotes && str[n1] == quote) break; /* Quote was reached */
		if (str[n1] == C2SX('\\') && str[++n1] == NULC) break; /* Escape character (can be the last) */
	}
	if (ignore_quotes && str[n1--] != quote) return false; /* Quote is not the same than earlier, '--' is not to take it into account */
	if (!ignore_spaces)
		while (str[++n1]) ; /* Jump down the end of the string */

	if (r0 != NULL) *r0 = n0;
	if (r1 != NULL) *r1 = n1;

	return true;
}

BOM_TYPE freadBOM(FILE* f, unsigned char* bom, int* sz_bom)
{
	unsigned char c1, c2;
	long pos;

	if (f == NULL) return BOM_NONE;

	/* Save position and try to read and skip BOM if found. If not, go back to save position. */
	pos = ftell(f);
	fread(&c1, sizeof(char), 1, f);
	fread(&c2, sizeof(char), 1, f);
	if (bom != NULL) {
		bom[0] = c1;
		bom[1] = c2;
		bom[2] = '\0';
		if (sz_bom != NULL) *sz_bom = 2;
	}
	switch ((unsigned short)(c1 << 8) | c2) {
		case (unsigned short)0xfeff:
			return BOM_UTF_16BE;

		case (unsigned short)0xfffe:
			pos = ftell(f); /* Save current position to get it back if BOM is not UTF-32LE */
			fread(&c1, sizeof(char), 1, f);
			fread(&c2, sizeof(char), 1, f);
			if (c1 == 0x00 && c2 == 0x00) {
				if (bom != NULL) bom[2] = bom[3] = bom[4] = '\0';
				if (sz_bom != NULL) *sz_bom = 4;
				return BOM_UTF_32LE;
			}
			fseek(f, pos, SEEK_SET); /* fseek(f, -2, SEEK_CUR) is not garanteed under Windows (and actually fail in Unicode...) */
			return BOM_UTF_16LE;

		case (unsigned short)0x0000:
			fread(&c1, sizeof(char), 1, f);
			fread(&c2, sizeof(char), 1, f);
			if (c1 == 0xfe && c2 == 0xff) {
				bom[2] = c1;
				bom[3] = c2;
				bom[4] = '\0';
				if (sz_bom != NULL) *sz_bom = 4;
				return BOM_UTF_32BE;
			}
			fseek(f, pos, SEEK_SET);
			return BOM_NONE;

		case (unsigned short)0xefbb: /* UTF-8? */
			fread(&c1, sizeof(char), 1, f);
			if (c1 != 0xbf) { /* Not UTF-8 */
				fseek(f, pos, SEEK_SET);
				if (bom != NULL) bom[0] = '\0';
				if (sz_bom != NULL) *sz_bom = 0;
				return BOM_NONE;
			}
			if (bom != NULL) {
				bom[2] = c1;
				bom[3] = '\0';
			}
			if (sz_bom != NULL) *sz_bom = 3;
			return BOM_UTF_8;

		default: /* No BOM, go back */
			fseek(f, pos, SEEK_SET);
			if (bom != NULL) bom[0] = '\0';
			if (sz_bom != NULL) *sz_bom = 0;
			return BOM_NONE;
	}
}

/* --- */

SXML_CHAR* html2str(SXML_CHAR* html, SXML_CHAR* str)
{
	SXML_CHAR *ps, *pd;
	int i;

	if (html == NULL) return NULL;

	if (str == NULL) str = html;
	
	/* Look for '&' and matches it to any of the recognized HTML pattern. */
	/* If found, replaces the '&' by the corresponding char. */
	/* 'p2' is the char to analyze, 'p1' is where to insert it */
	for (pd = str, ps = html; *ps; ps++, pd++) {
		if (*ps != C2SX('&')) {
			if (pd != ps) *pd = *ps;
			continue;
		}
		
		for (i = 0; HTML_SPECIAL_DICT[i].chr; i++) {
			if (sx_strncmp(ps, HTML_SPECIAL_DICT[i].html, HTML_SPECIAL_DICT[i].html_len)) continue;
			
			*pd = HTML_SPECIAL_DICT[i].chr;
			ps += HTML_SPECIAL_DICT[i].html_len-1;
			break;
		}
		/* If no string was found, simply copy the character */
		if (HTML_SPECIAL_DICT[i].chr == NULC && pd != ps) *pd = *ps;
	}
	*pd = NULC;
	
	return str;
}

/* TODO: Allocate 'str'? */
SXML_CHAR* str2html(SXML_CHAR* str, SXML_CHAR* html)
{
	SXML_CHAR *ps, *pd;
	int i;

	if (str == NULL || html == NULL) return NULL;

	if (html == str) return NULL; /* Not handled yet */

	for (ps = str, pd = html; *ps; ps++, pd++) {
		for (i = 0; HTML_SPECIAL_DICT[i].chr; i++) {
			if (*ps == HTML_SPECIAL_DICT[i].chr) {
				sx_strcpy(pd, HTML_SPECIAL_DICT[i].html);
				pd += HTML_SPECIAL_DICT[i].html_len - 1;
				break;
			}
		}
		if (HTML_SPECIAL_DICT[i].chr == NULC && pd != ps) *pd = *ps;
	}
	*pd = NULC;

	return str;
}

int strlen_html(SXML_CHAR* str)
{
	int i, j, n;
	
	if (str == NULL) return 0;

	n = 0;
	for (i = 0; str[i]; i++) {
		for (j = 0; HTML_SPECIAL_DICT[j].chr; j++) {
			if (str[i] == HTML_SPECIAL_DICT[j].chr) {
				n += HTML_SPECIAL_DICT[j].html_len;
				break;
			}
		}
		if (HTML_SPECIAL_DICT[j].chr == NULC) n++;
	}

	return n;
}

int fprintHTML(FILE* f, SXML_CHAR* str)
{
	SXML_CHAR* p;
	int i, n;
	
	for (p = str, n = 0; *p != NULC; p++) {
		for (i = 0; HTML_SPECIAL_DICT[i].chr; i++) {
			if (*p != HTML_SPECIAL_DICT[i].chr) continue;
			sx_fprintf(f, HTML_SPECIAL_DICT[i].html);
			n += HTML_SPECIAL_DICT[i].html_len;
			break;
		}
		if (HTML_SPECIAL_DICT[i].chr == NULC) {
			(void)sx_fputc(*p, f);
			n++;
		}
	}
	
	return n;
}

int regstrcmp(SXML_CHAR* str, SXML_CHAR* pattern)
{
	SXML_CHAR *p, *s;

	if (str == NULL && pattern == NULL) return true;

	if (str == NULL || pattern == NULL) return false;

	p = pattern;
	s = str;
	while (true) {
		switch (*p) {
			/* Any character matches, go to next one */
			case C2SX('?'):
				p++;
				s++;
				break;

			/* Go to next character in pattern and wait until it is found in 'str' */
			case C2SX('*'):
				for (; *p != NULC; p++) { /* Squeeze '**?*??**' to '*' */
					if (*p != C2SX('*') && *p != C2SX('?')) break;
				}
				for (; *s != NULC; s++) {
					if (*s == *p) break;
				}
				break;

			/* NULL character on pattern has to be matched by 'str' */
			case 0:
				return *s ? false : true;

			default:
				if (*p == C2SX('\\')) p++; /* Escape character */
				if (*p++ != *s++) return false; /* Characters do not match */
				break;
		}
	}
}
