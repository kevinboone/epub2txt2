/*============================================================================
  epub2txt v2 
  epub2txt.c
  Copyright (c)2017 Kevin Boone, GPL v3.0
============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __APPLE__
#include <malloc.h>
#endif
#include "epub2txt.h" 
#include "log.h"
#include "list.h"
#include "string.h"
#include "sxmlc.h"
#include "xhtml.h"


/*============================================================================
  epub2txt_get_items
  Parse the OPF file to get the spine items 
============================================================================*/
List *epub2txt_get_items (const char *opf, char **error)
  {
  IN
  List *ret = NULL;
  String *buff = NULL;
  if (string_create_from_utf8_file (opf, &buff, error))
    {
    const char *buff_cstr = string_cstr (buff);
    log_debug ("Read OPF, size %d", string_length (buff));
    BOOL got_manifest = FALSE;
    XMLNode *manifest = NULL;
    XMLDoc doc;
    XMLDoc_init (&doc);
    if (XMLDoc_parse_buffer_DOM (buff_cstr, APPNAME, &doc))
      {
      XMLNode *root = XMLDoc_root (&doc);

      int i, l = root->n_children;
      for (i = 0; i < l; i++)
	{
	XMLNode *r1 = root->children[i];
	// Add workaround for bug #4 
	if (strcmp (r1->tag, "manifest") == 0 || strstr (r1->tag, ":manifest"))
	  {
	  manifest = r1;
	  got_manifest = TRUE;
	  }
	}

      if (!got_manifest)
	{
	asprintf (error, "File %s has no manifest", opf);
	OUT
	return NULL; 
	}
   
      ret = list_create_strings();

      for (i = 0; i < l; i++)
	{
	XMLNode *r1 = root->children[i];
	// Add workaround for bug #4
	if (strcmp (r1->tag, "spine") == 0 || strstr (r1->tag, ":spine"))
	  {
	  int j, l2 = r1->n_children;
	  for (j = 0; j < l2; j++)
	    {
	    XMLNode *r2 = r1->children[j]; // itemref
	    int k, nattrs = r2->n_attributes;
	    for (k = 0; k < nattrs; k++)
	      {
	      char *name = r2->attributes[k].name;
	      if (strcmp (name, "idref") == 0)
		{
		char *value= r2->attributes[k].value;
		int m;
		for (m = 0; m < manifest->n_children; m++)
		  {
		  XMLNode *r3 = manifest->children[m]; // item
		  int n, nattrs = r3->n_attributes;
		  for (n = 0; n < nattrs; n++)
		    {
		    char *name2 = r3->attributes[n].name;
		    char *val2 = r3->attributes[n].value;
		    if (strcmp (name2, "id") == 0 && 
			strcmp (val2, value) == 0)
		      {
		      int p;
		      for (p = 0; p < nattrs; p++)
			{
			char *name2 = r3->attributes[p].name;
			char *val2 = r3->attributes[p].value;
			if (strcmp (name2, "href") == 0)
			  {
			  list_append (ret, strdup (val2));
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
          }
        }
      XMLDoc_free (&doc);
      }
    else
      {
      asprintf (error, "Can't parse OPF XML");
      }
    string_destroy (buff);
    }

  OUT
  return ret;
  }


/*============================================================================
  epub2txt_get_root_file 
  Parse the OPF file to get the root document
============================================================================*/
String *epub2txt_get_root_file (const char *opf, char **error)
  {
  IN
  String *ret = NULL;
  String *buff = NULL;
  if (string_create_from_utf8_file (opf, &buff, error))
    {
    const char *buff_cstr = string_cstr (buff);
    log_debug ("Read OPF, size %d", string_length (buff));
    XMLDoc doc;
    XMLDoc_init (&doc);
    if (XMLDoc_parse_buffer_DOM (buff_cstr, APPNAME, &doc))
      {
      XMLNode *root = XMLDoc_root (&doc);
      int i, l = root->n_children;
      for (i = 0; i < l; i++)
	{
	XMLNode *r1 = root->children[i];
	if (strcmp (r1->tag, "rootfiles") == 0)
	  {
	  XMLNode *rootfiles = r1;
	  int i, l = rootfiles->n_children;
	  for (i = 0; i < l; i++)
	    {
	    XMLNode *r1 = rootfiles->children[i];
	    if (strcmp (r1->tag, "rootfile") == 0)
	      {
	      int k, nattrs = r1->n_attributes;
	      for (k = 0; k < nattrs; k++)
		{
		char *name = r1->attributes[k].name;
		char *value = r1->attributes[k].value;
		if (strcmp (name, "full-path") == 0)
		  {
		  ret = string_create (value);
		  }
		}
	      }
	    }
	  }
	}

      if (ret == NULL)
        asprintf (error, "container.xml does not specify a root file");
      XMLDoc_free (&doc);
      }
    else
      {
      asprintf (error, "Can't parse OPF XML");
      }
    string_destroy (buff);
    }

  OUT
  return ret;
  }



/*============================================================================
  epub2txt 
============================================================================*/
void epub2txt_do_file (const char *file, const Epub2TxtOptions *options, 
     char **error)
  {
  IN

  log_debug ("epub2txt_do_file: %s", file);
  if (access (file, R_OK) == 0)
    {
    log_debug ("File access OK");

    //output_para = 0;
    char tempdir[256];
    char tempbase[256];
    char cmd[512];

    if (getenv ("TMP"))
      strcpy (tempbase, getenv("TMP"));
    else
      strcpy (tempbase, "/tmp");

    log_debug ("tempbase is: %s", tempbase);

    sprintf (tempdir, "%s/epub2txt%d", tempbase, getpid()); 

    log_debug ("tempdir is: %s", tempdir);

    // We should really check this but, honestly, anybody who creates
    //  a non-writable tempdir has problems far worse than being unable
    //  to run this utility
    mkdir (tempdir, 0777); 

    sprintf (cmd, "unzip -o -qq \"%s\" -d \"%s\"", file, tempdir);
    log_debug ("Running unzip command; %s", cmd);
    system (cmd);
    log_debug ("Unzip finished");
    // On some systems, unzip results in a file with no read permissions
    //   for the user -- reason unknown 
    sprintf (cmd, "chmod -R 744 \"%s\"", tempdir);
    log_debug ("Fix permissions: %s", cmd);
    system (cmd);
    log_debug ("Permissions fixed");


    char opf[512];
    sprintf (opf, "%s/META-INF/container.xml", tempdir);
    log_debug ("OPF path is: %s", opf);
    String *rootfile = epub2txt_get_root_file (opf, error);
    if (*error == NULL)
      {
      log_debug ("OPF rootfile is: %s", string_cstr(rootfile));

      sprintf (opf, "%s/%s", tempdir, string_cstr (rootfile));
      char *content_dir = strdup (opf);
      char *p = strrchr (content_dir, '/');
      *p = 0; 
      log_debug ("Content directory is: %s", content_dir);
      List *list = epub2txt_get_items (opf, error);
      if (*error == NULL)
        {
        log_debug ("EPUB spine has %d items", list_length (list));
        int i, l = list_length (list);
        for (i = 0; i < l; i++)
          {
          const char *item = (const char *)list_get (list, i);
          sprintf (opf, "%s/%s", content_dir, item);
          xhtml_to_stdout (opf, options, error);
          }
        list_destroy (list);
        }
      free (content_dir);
      }

    if (rootfile) string_destroy (rootfile);
    sprintf (cmd, "rm -rf \"%s\"", tempdir);
    log_debug ("Deleting temporary directory");
    system (cmd); 
    }
  else
    {
    asprintf (error, "File not found: %s", file);
    }

  OUT
  }



