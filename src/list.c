/*============================================================================
  epub2txt v2 
  list.c
  Copyright (c)2000-2017 Kevin Boone, GPL v3.0
============================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#if defined(__MACH__)
#include <stdlib.h>
#else 
#include <malloc.h>
#endif

#include <pthread.h>
#include "list.h" 

typedef struct _ListItem
  {
  struct _ListItem *next;
  void *data;
  } ListItem;

struct _List
  {
  pthread_mutex_t mutex;
  ListItemFreeFn free_fn; 
  ListItem *head;
  };

/*==========================================================================
list_create
*==========================================================================*/
List *list_create (ListItemFreeFn free_fn)
  {
  List *list = malloc (sizeof (List));
  memset (list, 0, sizeof (List));
  list->free_fn = free_fn;
  pthread_mutex_init (&list->mutex, NULL);
  return list;
  }

/*==========================================================================
list_create_strings 
*==========================================================================*/
List *list_create_strings (void)
  {
  return list_create (free);
  }

/*==========================================================================
list_destroy
*==========================================================================*/
void list_destroy (List *self)
  {
  if (!self) return;

  pthread_mutex_lock (&self->mutex);
  ListItem *l = self->head;
  while (l)
    {
    if (self->free_fn)
      self->free_fn (l->data);
    ListItem *temp = l;
    l = l->next;
    free (temp);
    }

  pthread_mutex_unlock (&self->mutex);
  pthread_mutex_destroy (&self->mutex);
  free (self);
  }


/*==========================================================================
list_prepend
Note that the caller must not modify or free the item added to the list. It
will remain on the list until free'd by the list itself, by calling
the supplied free function
*==========================================================================*/
void list_prepend (List *self, void *item)
  {
  pthread_mutex_lock (&self->mutex);
  ListItem *i = malloc (sizeof (ListItem));
  i->data = item;
  i->next = NULL;

  if (self->head)
    {
    i->next = self->head;
    self->head = i;
    }
  else
    {
    self->head = i;
    }
  pthread_mutex_unlock (&self->mutex);
  }


/*==========================================================================
list_append
Note that the caller must not modify or free the item added to the list. It
will remain on the list until free'd by the list itself, by calling
the supplied free function
*==========================================================================*/
void list_append (List *self, void *item)
  {
  pthread_mutex_lock (&self->mutex);
  ListItem *i = malloc (sizeof (ListItem));
  i->data = item;
  i->next = NULL;

  if (self->head)
    {
    ListItem *l = self->head;
    while (l->next)
      l = l->next;
    l->next = i;
    }
  else
    {
    self->head = i;
    }
  pthread_mutex_unlock (&self->mutex);
  }


/*==========================================================================
list_length
*==========================================================================*/
int list_length (List *self)
  {
  if (!self) return 0;

  pthread_mutex_lock (&self->mutex);
  ListItem *l = self->head;
  int i = 0;
  while (l != NULL)
    {
    l = l->next;
    i++;
    }

  pthread_mutex_unlock (&self->mutex);
  return i;
  }

/*==========================================================================
list_get
*==========================================================================*/
void *list_get (List *self, int index)
  {
  if (!self) return NULL;

  pthread_mutex_lock (&self->mutex);
  ListItem *l = self->head;
  int i = 0;
  while (l != NULL && i != index)
    {
    l = l->next;
    i++;
    }
  pthread_mutex_unlock (&self->mutex);

  return l->data;
  }


/*==========================================================================
list_dump
*==========================================================================*/
void list_dump (List *self)
  {
  int i, l = list_length (self);
  for (i = 0; i < l; i++)
    {
    const char *s = list_get (self, i);
    printf ("%s\n", s);
    }
  }


/*==========================================================================
list_contains
*==========================================================================*/
BOOL list_contains (List *self, const void *item, ListCompareFn fn)
  {
  if (!self) return FALSE;
  pthread_mutex_lock (&self->mutex);
  ListItem *l = self->head;
  BOOL found = FALSE;
  while (l != NULL && !found)
    {
    if (fn (l->data, item) == 0) found = TRUE; 
    l = l->next;
    }
  pthread_mutex_unlock (&self->mutex);
  return found; 
  }


/*==========================================================================
list_contains_string
*==========================================================================*/
BOOL list_contains_string (List *self, const char *item)
  {
  return list_contains (self, item, (ListCompareFn)strcmp);
  }


/*==========================================================================
list_remove
IMPORTANT -- The "item" argument cannot be a direct reference to an
item already in the list. If that items is removed from the list its
memory will be freed. The "item" argument will this be an invalid
memory reference, and the program will crash. It is necessary
to copy the item first.
*==========================================================================*/
void list_remove (List *self, const void *item, ListCompareFn fn)
  {
  if (!self) return;
  pthread_mutex_lock (&self->mutex);
  ListItem *l = self->head;
  ListItem *last_good = NULL;
  while (l != NULL)
    {
    if (fn (l->data, item) == 0)
      {
      if (l == self->head)
        {
        self->head = l->next; // l-> next might be null
        }
      else
        {
        if (last_good) last_good->next = l->next;
        }
      self->free_fn (l->data);  
      ListItem *temp = l->next;
      free (l);
      l = temp;
      } 
    else
      {
      last_good = l;
      l = l->next;
      }
    }
  pthread_mutex_unlock (&self->mutex);
  }

/*==========================================================================
list_remove_string
*==========================================================================*/
void list_remove_string (List *self, const char *item)
  {
  list_remove (self, item, (ListCompareFn)strcmp);
  }


/*==========================================================================
list_clone
*==========================================================================*/
List *list_clone (List *self, ListCopyFn copyFn)
  {
  ListItemFreeFn free_fn = self->free_fn; 
  List *new = list_create (free_fn);

  pthread_mutex_lock (&self->mutex);
  ListItem *l = self->head;
  while (l != NULL)
    {
    void *data = copyFn (l->data);
    list_append (new, data);
    l = l->next;
    }
  pthread_mutex_unlock (&self->mutex);

  return new;
  }



