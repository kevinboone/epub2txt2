/*============================================================================
  epub2txt v2 
  list.h
  Copyright (c)2000-2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include "defs.h" 

struct _List;
typedef struct _List List;

// The comparison function should return -1, 0, +1, like strcmp 
typedef int (*ListCompareFn) (const void *i1, const void *i2);
typedef void* (*ListCopyFn) (const void *orig);
typedef void (*ListItemFreeFn) (void *);

List   *list_create (ListItemFreeFn free_fn);
void    list_destroy (List *);
void    list_append (List *self, void *item);
void    list_prepend (List *self, void *item);
void   *list_get (List *self, int index);
void    list_dump (List *self);
int     list_length (List *self);
BOOL    list_contains (List *self, const void *item, ListCompareFn fn);
BOOL    list_contains_string (List *self, const char *item);
void    list_remove (List *self, const void *item, ListCompareFn fn);
void    list_remove_string (List *self, const char *item);
List   *list_clone (List *self, ListCopyFn copyFn);
List   *list_create_strings (void);

