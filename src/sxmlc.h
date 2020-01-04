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
#ifndef _CXML_H_
#define _CXML_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "sxmlutils.h"

#define SXMLC_VERSION "4.0.1"

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

/* Node types */
typedef enum _TagType {
	TAG_ERROR = -1,
	TAG_NONE = 0,
	TAG_PARTIAL,	/* Node containing a legal '>' which stopped file reading */
	TAG_FATHER,		/* <tag> - Next nodes will be children of this one. */
	TAG_SELF,		/* <tag/> - Standalone node. */
	TAG_INSTR,		/* <?prolog?> - Processing instructions, or prolog node. */
	TAG_COMMENT,	/* <!--comment--> */
	TAG_CDATA,		/* <![CDATA[ ]]> - CDATA node */
	TAG_DOCTYPE,	/* <!DOCTYPE [ ]> - DOCTYPE node */
	TAG_END,		/* </tag> - End of father node. */

	TAG_USER = 100	/* User-defined tag start */
} TagType;

/* TODO: Performance improvement with some fixed-sized strings ??? (e.g. XMLAttribute.name[64], XMLNode.tag[64]) */

typedef struct _XMLAttribute {
	SXML_CHAR* name;
	SXML_CHAR* value;
	int active;
} XMLAttribute;

/* Constant to know whether a struct has been initialized (XMLNode or XMLDoc) */
#define XML_INIT_DONE 0x19770522 /* Happy Birthday ;) */

/*
 An XML node.
 */
typedef struct _XMLNode {
	SXML_CHAR* tag;				/* Tag name */
	SXML_CHAR* text;			/* Text inside the node */
	XMLAttribute* attributes;
	int n_attributes;
	
	struct _XMLNode* father;	/* NULL if root */
	struct _XMLNode** children;
	int n_children;
	
	TagType tag_type;	/* Node type ('TAG_FATHER', 'TAG_SELF' or 'TAG_END') */
	int active;		/* 'true' to tell that node is active and should be displayed by 'XMLDoc_print' */

	void* user;	/* Pointer for user data associated to the node */

	/* Keep 'init_value' as the last member */
	int init_value;	/* Initialized to 'XML_INIT_DONE' to indicate that node has been initialized properly */
} XMLNode;

/*
 An XML document.
 */
#ifndef MAX_PATH
#define MAX_PATH 256
#endif
typedef struct _XMLDoc {
	SXML_CHAR filename[MAX_PATH];
#ifdef SXMLC_UNICODE
	BOM_TYPE bom_type;
	unsigned char bom[5];	/* First characters read that might be a BOM when unicode is used */
	int sz_bom;				/* Number of bytes in BOM */
#endif
	XMLNode** nodes;		/* Nodes of the document, including prolog, comments and root nodes */
	int n_nodes;			/* Number of nodes in 'nodes' */
	int i_root;				/* Index of first root node in 'nodes', -1 if document is empty */

	/* Keep 'init_value' as the last member */
	int init_value;	/* Initialized to 'XML_INIT_DONE' to indicate that document has been initialized properly */
} XMLDoc;

/*
 Register an XML tag, giving its 'start' and 'end' string, which should include '<' and '>'.
 The 'tag_type' is user-given and has to be less than or equal to 'TAG_USER'. It will be
 returned as the 'tag_type' member of the XMLNode struct. Note that no test is performed
 to check for an already-existing tag_type.
 Return tag index in user tags table when successful, or '-1' if the 'tag_type' is invalid or
 the new tag could not be registered (e.g. when 'start' does not start with '<' or 'end' does not end with '>').
 */
int XML_register_user_tag(int tag_type, SXML_CHAR* start, SXML_CHAR* end);

/*
 Remove a registered user tag.
 Return the new number of registered user tags or '-1' if 'i_tag' is invalid.
 */
int XML_unregister_user_tag(int i_tag);

/*
 Return the number of registered tags.
 */
int XML_get_nb_registered_user_tags(void);

/*
 Return the index of first occurrence of 'tag_type' in registered user tags, or '-1' if not found.
 */
int XML_get_registered_user_tag(TagType tag_type);


typedef enum _ParseError {
	PARSE_ERR_NONE = 0,
	PARSE_ERR_MEMORY = -1,
	PARSE_ERR_UNEXPECTED_TAG_END = -2,
	PARSE_ERR_SYNTAX = -3,
	PARSE_ERR_EOF = -4,
	PARSE_ERR_TEXT_OUTSIDE_NODE = -5, /* During DOM loading */
	PARSE_ERR_UNEXPECTED_NODE_END = -6 /* During DOM loading */
} ParseError;

/*
 Events that can happen when loading an XML document.
 These will be passed to the 'all_event' callback of the SAX parser.
 */
typedef enum _XMLEvent {
	XML_EVENT_START_DOC,
	XML_EVENT_START_NODE,
	XML_EVENT_END_NODE,
	XML_EVENT_TEXT,
	XML_EVENT_ERROR,
	XML_EVENT_END_DOC
} XMLEvent;

/*
 Structure given as an argument for SAX callbacks to retrieve information about
 parsing status
 */
typedef struct _SAX_Data {
	const SXML_CHAR* name;
	int line_num;
	void* user;
} SAX_Data;

/*
 User callbacks used for SAX parsing. Return values of these callbacks should be 0 to stop parsing.
 Members can be set to NULL to disable handling of some events.
 All parameters are pointers to structures that will no longer be available after callback returns.
 It is recommended that the callback uses the information and stores it in its own data structure.
 WARNING! SAX PARSING DOES NOT CHECK FOR XML INTEGRITY! e.g. a tag end without a matching tag start
 will not be detected by the parser and should be detected by the callbacks instead.
 */
typedef struct _SAX_Callbacks {
	/*
	 Callback called when parsing starts, before parsing the first node.
	 */
	int (*start_doc)(SAX_Data* sd);

	/*
	 Callback called when a new node starts (e.g. '<tag>' or '<tag/>').
	 If any, attributes can be read from 'node->attributes'.
	 N.B. '<tag/>' will trigger an immediate call to the 'end_node' callback
	 after the 'start_node' callback.
	 */
	int (*start_node)(const XMLNode* node, SAX_Data* sd);

	/*
	 Callback called when a node ends (e.g. '</tag>' or '<tag/>').
	 */
	int (*end_node)(const XMLNode* node, SAX_Data* sd);

	/*
	 Callback called when text has been found in the last node.
	 */
	int (*new_text)(SXML_CHAR* text, SAX_Data* sd);

	/*
	 Callback called when parsing is finished.
	 No other callbacks will be called after it.
	 */
	int (*end_doc)(SAX_Data* sd);

	/*
	 Callback called when an error occurs during parsing.
	 'error_num' is the error number and 'line_number' is the line number in the stream
	 being read (file or buffer).
	 */
	int (*on_error)(ParseError error_num, int line_number, SAX_Data* sd);

	/*
	 Callback called when text has been found in the last node.
	 'event' is the type of event for which the callback was called:
	 	 XML_EVENT_START_DOC:
	 	 	 'node' is NULL.
	 	 	 'text' is the file name if a file is being parsed, NULL if a buffer is being parsed.
	 	 	 'n' is 0.
	 	 XML_EVENT_START_NODE:
	 	 	 'node' is the node starting, with tag and all attributes initialized.
	 	 	 'text' is NULL.
	 	 	 'n' is the number of lines parsed.
	 	 XML_EVENT_END_NODE:
	 	 	 'node' is the node ending, with tag, attributes and text initialized.
	 	 	 'text' is NULL.
	 	 	 'n' is the number of lines parsed.
	 	 XML_EVENT_TEXT:
	 	 	 'node' is NULL.
	 	 	 'text' is the text to be added to last node started and not finished.
	 	 	 'n' is the number of lines parsed.
	 	 XML_EVENT_ERROR:
	 	 	 Everything is NULL.
	 	 	 'n' is one of the 'PARSE_ERR_*'.
	 	 XML_EVENT_END_DOC:
	 	 	 'node' is NULL.
	 	 	 'text' is the file name if a file is being parsed, NULL if a buffer is being parsed.
	 	 	 'n' is the number of lines parsed.
	 */
	int (*all_event)(XMLEvent event, const XMLNode* node, SXML_CHAR* text, const int n, SAX_Data* sd);
} SAX_Callbacks;

/*
 Helper function to initialize all 'sax' members to NULL.
 Return 'false' is 'sax' is NULL.
 */
int SAX_Callbacks_init(SAX_Callbacks* sax);

/*
 Set of SAX callbacks used by 'XMLDoc_parse_file_DOM'.
 These are made available to be able to load an XML document using DOM implementation
 with user-defined code at some point (e.g. counting nodes, running search, ...).
 In this case, the 'XMLDoc_parse_file_SAX' has to be called instead of the 'XMLDoc_parse_file_DOM',
 providing either these callbacks directly, or a functions calling these callbacks.
 To do that, you should initialize the 'doc' member of the 'DOM_through_SAX' struct and call the
 'XMLDoc_parse_file_SAX' giving this struct as a the 'user' data pointer.
 */

typedef struct _DOM_through_SAX {
	XMLDoc* doc;		/* Document to fill up */
	XMLNode* current;	/* For internal use (current father node) */
	ParseError error;	/* For internal use (parse status) */
	int line_error;		/* For internal use (line number when error occurred) */
} DOM_through_SAX;

int DOMXMLDoc_doc_start(SAX_Data* dom);
int DOMXMLDoc_node_start(const XMLNode* node, SAX_Data* dom);
int DOMXMLDoc_node_text(SXML_CHAR* text, SAX_Data* dom);
int DOMXMLDoc_node_end(const XMLNode* node, SAX_Data* dom);
int DOMXMLDoc_parse_error(ParseError error_num, int line_number, SAX_Data* sd);
int DOMXMLDoc_doc_end(SAX_Data* dom);

/*
 Initialize 'sax' with the "official" DOM callbacks.
 */
int SAX_Callbacks_init_DOM(SAX_Callbacks* sax);

/* --- XMLNode methods --- */

/*
 Fills 'xmlattr' with 'xmlattr->name' to 'attrName' and 'xmlattr->value' to 'attr Value'.
 'str' is supposed to be like 'attrName[ ]=[ ]["]attr Value["]'.
 Return 0 if not enough memory or bad parameters (NULL 'str' or 'xmlattr').
        2 if last quote is missing in the attribute value.
		1 if 'xmlattr' was filled correctly.
 */
int XML_parse_attribute(const SXML_CHAR* str, XMLAttribute* xmlattr);

/*
 Reads a string that is supposed to be an xml tag like '<tag (attribName="attribValue")* [/]>' or '</tag>'.
 Fills the 'xmlnode' structure with the tag name and its attributes.
 Returns 0 if an error occurred (malformed 'str' or memory). 'TAG_*' when string is recognized.
 */
TagType XML_parse_1string(SXML_CHAR* str, XMLNode* xmlnode);

/*
 Allocate and initialize XML nodes.
 'n' is the number of contiguous elements to allocate (to create and array).
 Return 'NULL' if not enough memory, or the pointer to the elements otherwise.
 */
XMLNode* XMLNode_allocN(int n);

/*
 Shortcut to allocate one node only.
 */
#define XMLNode_alloc() XMLNode_allocN(1)

/*
 Initialize an already-allocated XMLNode.
 */
int XMLNode_init(XMLNode* node);

/*
 Free a node and all its children.
 */
int XMLNode_free(XMLNode* node);

/*
 Free XMLNode 'dst' and copy 'src' to 'dst', along with its children if specified.
 If 'src' is NULL, 'dst' is freed and initialized.
 */
int XMLNode_copy(XMLNode* dst, const XMLNode* src, int copy_children);

/*
 Allocate a node and copy 'node' into it.
 If 'copy_children' is 'true', all children of 'node' will be copied to the new node.
 Return 'NULL' if not enough memory, or a pointer to the new node otherwise.
 */
XMLNode* XMLNode_dup(const XMLNode* node, int copy_children);

/*
 Set the active/inactive state of 'node'.
 Set 'active' to 'true' to activate 'node' and all its children, and enable its use
 in other functions (e.g. 'XMLDoc_print', 'XMLNode_search_child').
 */
int XMLNode_set_active(XMLNode* node, int active);

/*
 Set 'node' tag.
 Return 'false' for memory error, 'true' otherwise.
 */
int XMLNode_set_tag(XMLNode* node, const SXML_CHAR* tag);

/*
 Set the node type among one of the valid ones (TAG_FATHER, TAG_SELF, TAG_INSTR,
 TAG_COMMENT, TAG_CDATA, TAG_DOCTYPE) or any user-registered tag.
 Return 'false' when the node or the 'tag_type' is invalid.
 */
int XMLNode_set_type(XMLNode* node, const TagType tag_type);

/*
 Add an attribute to 'node' or update an existing one.
 The attribute has a 'name' and a 'value'.
 Return the new number of attributes, or -1 for memory problem.
 */
int XMLNode_set_attribute(XMLNode* node, const SXML_CHAR* attr_name, const SXML_CHAR* attr_value);

/*
 Retrieve an attribute value, based on its name, allocating 'attr_value'.
 If the attribute name does not exist, set 'attr_value' to the given default value.
 Return 'false' when the node is invalid, 'attr_name' is NULL or empty, or 'attr_value' is NULL.
 */
int XMLNode_get_attribute_with_default(XMLNode* node, const SXML_CHAR* attr_name, const SXML_CHAR** attr_value, const SXML_CHAR* default_attr_value);

/*
 Helper macro that retrieve an attribute value, or an empty string if the attribute does
 not exist.
 */
#define XMLNode_get_attribute(node, attr_name, attr_value) XMLNode_get_attribute_with_default(node, attr_name, attr_value, C2SX(""))

/*
 Search for the active attribute 'attr_name' in 'node', starting from index 'isearch'
 and returns its index, or -1 if not found or error.
 */
int XMLNode_search_attribute(const XMLNode* node, const SXML_CHAR* attr_name, int isearch);

/*
 Remove attribute index 'i_attr'.
 Return the new number of attributes or -1 on invalid arguments.
 */
int XMLNode_remove_attribute(XMLNode* node, int i_attr);

/*
 Remove all attributes from 'node'.
 */
int XMLNode_remove_all_attributes(XMLNode* node);

/*
 Set node text.
 Return 'true' when successful, 'false' on error.
 */
int XMLNode_set_text(XMLNode* node, const SXML_CHAR* text);

/*
 Helper macro to remove text from 'node'.
 */
#define XMLNode_remove_text(node) XMLNode_set_text(node, NULL);

/*
 Add a child to a node.
 Return 'false' for memory problem, 'true' otherwise.
 */
int XMLNode_add_child(XMLNode* node, XMLNode* child);

/*
 Return the number of active children nodes of 'node', or '-1' if 'node' is invalid.
 */
int XMLNode_get_children_count(const XMLNode* node);

/*
 Return a reference to the 'i_child'th active node.
 */
XMLNode* XMLNode_get_child(const XMLNode* node, int i_child);

/*
 Remove the 'i_child'th active child of 'node'.
 If 'free_child' is 'true', free the child node itself. This parameter is usually 'true'
 but should be 'false' when child nodes are pointers to local or global variables instead of
 user-allocated memory.
 Return the new number of children or -1 on invalid arguments.
 */
int XMLNode_remove_child(XMLNode* node, int i_child, int free_child);

/*
 Remove all children from 'node'.
 */
int XMLNode_remove_children(XMLNode* node);

/*
 Return 'true' if 'node1' is the same as 'node2' (i.e. same tag, same active attributes).
 */
int XMLNode_equal(const XMLNode* node1, const XMLNode* node2);

/*
 Return the next sibling of node 'node', or NULL if 'node' is invalid or the last child
 or if its father could not be determined (i.e. 'node' is a root node).
 */
XMLNode* XMLNode_next_sibling(const XMLNode* node);

/*
 Return the next node in XML order i.e. first child or next sibling, or NULL
 if 'node' is invalid or the end of its root node is reached.
 */
XMLNode* XMLNode_next(const XMLNode* node);


/* --- XMLDoc methods --- */


/*
 Initializes an already-allocated XML document.
 */
int XMLDoc_init(XMLDoc* doc);

/*
 Free an XML document.
 Return 'false' if 'doc' was not initialized.
 */
int XMLDoc_free(XMLDoc* doc);

/*
 Set the new 'doc' root node among all existing nodes in 'doc'.
 Return 'false' if bad arguments, 'true' otherwise.
 */
int XMLDoc_set_root(XMLDoc* doc, int i_root);

/*
 Add a node to the document, specifying the type.
 If its type is TAG_FATHER, it also sets the document root node if previously undefined.
 Return the node index, or -1 if bad arguments or memory error.
 */
int XMLDoc_add_node(XMLDoc* doc, XMLNode* node);

/*
 Remove a node from 'doc' root nodes, base on its index.
 If 'free_node' is 'true', free the node itself. This parameter is usually 'true'
 but should be 'false' when the node is a pointer to local or global variable instead of
 user-allocated memory.
 Return 'true' if node was removed or 'false' if 'doc' or 'i_node' is invalid.
 */
int XMLDoc_remove_node(XMLDoc* doc, int i_node, int free_node);

/*
 Shortcut macro to retrieve root node from a document.
 Equivalent to
 doc->nodes[doc->i_root]
 */
#define XMLDoc_root(doc) ((doc)->nodes[(doc)->i_root])

/*
 Shortcut macro to add a node to 'doc' root node.
 Equivalent to
 XMLDoc_add_child_root(XMLDoc* doc, XMLNode* child);
 */
#define XMLDoc_add_child_root(doc, child) XMLNode_add_child((doc)->nodes[(doc)->i_root], (child))

/*
 Default quote to use to print attribute value.
 User can redefine it with its own character by adding a #define XML_DEFAULT_QUOTE before including
 this file.
 */
#ifndef XML_DEFAULT_QUOTE
#define XML_DEFAULT_QUOTE C2SX('"')
#endif

/*
 Print the node and its children to a file (that can be stdout).
 - 'tag_sep' is the string to use to separate nodes from each other (usually "\n").
 - 'child_sep' is the additional string to put for each child level (usually "\t").
 - 'keep_text_spaces' indicates that text should not be printed if it is composed of
   spaces, tabs or new lines only (e.g. when XML document spans on several lines due to
   pretty-printing).
 - 'sz_line' is the maximum number of characters that can be put on a single line. The
   node remainder will be output to extra lines.
 - 'nb_char_tab' is how many characters should be counted for a tab when counting characters
   in the line. It usually is 8 or 4, but at least 1.
 - 'depth' is an internal parameter that is used to determine recursively how deep we are in
   the tree. It should be initialized to 0 at first call.
 Return 'false' on invalid arguments (NULL 'node' or 'f'), 'true' otherwise.
 */
int XMLNode_print(const XMLNode* node, FILE* f, const SXML_CHAR* tag_sep, const SXML_CHAR* child_sep, int keep_text_spaces, int sz_line, int nb_char_tab);

/*
 Print the node "header": <tagname attribname="attibval" ...[/]>, spanning it on several lines if needed.
 Return 'false' on invalid arguments (NULL 'node' or 'f'), 'true' otherwise.
 */
int XMLNode_print_header(const XMLNode* node, FILE* f, int sz_line, int nb_char_tab);

/*
 Prints the XML document using 'XMLNode_print' on all document root nodes.
 */
int XMLDoc_print(const XMLDoc* doc, FILE* f, const SXML_CHAR* tag_sep, const SXML_CHAR* child_sep, int keep_text_spaces, int sz_line, int nb_char_tab);

/*
 Create a new XML document from a given 'filename' and load it to 'doc'.
 Return 'false' in case of error (memory or unavailable filename, malformed document), 'true' otherwise.
 */
int XMLDoc_parse_file_DOM(const SXML_CHAR* filename, XMLDoc* doc);

/*
 Create a new XML document from a memory buffer 'buffer' that can be given a name 'name', and load
 it into 'doc'.
 Return 'false' in case of error (memory or unavailable filename, malformed document), 'true' otherwise.
 */
int XMLDoc_parse_buffer_DOM(const SXML_CHAR* buffer, const SXML_CHAR* name, XMLDoc* doc);

/*
 Parse an XML document from a given 'filename', calling SAX callbacks given in the 'sax' structure.
 'user' is a user-given pointer that will be given back to all callbacks.
 Return 'false' in case of error (memory or unavailable filename, malformed document), 'true' otherwise.
 */
int XMLDoc_parse_file_SAX(const SXML_CHAR* filename, const SAX_Callbacks* sax, void* user);

/*
 Parse an XML document from a memory buffer 'buffer' that can be given a name 'name',
 calling SAX callbacks given in the 'sax' structure.
 'user' is a user-given pointer that will be given back to all callbacks.
 Return 'false' in case of error (memory or unavailable filename, malformed document), 'true' otherwise.
 */
int XMLDoc_parse_buffer_SAX(const SXML_CHAR* buffer, const SXML_CHAR* name, const SAX_Callbacks* sax, void* user);

/*
 Parse an XML file using the DOM implementation.
 */
#define XMLDoc_parse_file XMLDOC_parse_file_DOM

#ifdef __cplusplus
}
#endif

#endif
