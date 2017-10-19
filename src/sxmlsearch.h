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
#ifndef _SXMLCSEARCH_H_
#define _SXMLCSEARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sxmlutils.h"
#include "sxmlc.h"

/*
 XML search parameters. Can be initialized from an XPath string.
 A pointer to such structure is given to search functions which can modify
 its content (the 'from' structure).
 */
typedef struct _XMLSearch {
	/*
	 Search for nodes which tag match this 'tag' field.
	 If NULL or an empty string, all nodes will be matching.
	 */
	SXML_CHAR* tag;

	/*
	 Search for nodes which attributes match all the ones described.
	 If NULL, all nodes will be matching.
	 The 'attribute->name' should not be NULL. If corresponding 'attribute->value'
	 is NULL or an empty-string, search will return the first node with an attribute
	 'attribute->name', no matter what is its value.
	 If 'attribute->value' is not NULL, a matching node should have an attribute
	 'attribute->name' with the corresponding value 'attribute->value'.
	 When 'attribute->value' is not NULL, the 'attribute->active' should be 'true'
	 to specify that values should be equal, or 'false' to specify that values should
	 be different.
	 */
	XMLAttribute* attributes;
	int n_attributes;

	/*
	 Search for nodes which text match this 'text' field.
	 If NULL or an empty string, all nodes will be matching.
	 */
	SXML_CHAR* text;

	/*
	 Next search to perform on children of a node matching current struct.
	 Used to search for nodes children of specific nodes (used in XPath queries).
	 */
	struct _XMLSearch* next;
	struct _XMLSearch* prev;

	/*
	 Internal use only. Must be initialized to '-1' prior to first search.
	 */
	XMLNode* stop_at;

	/* Keep 'init_value' as the last member */
	int init_value;	/* Initialized to 'XML_INIT_DONE' to indicate that document has been initialized properly */
} XMLSearch;

typedef int (*REGEXPR_COMPARE)(SXML_CHAR* str, SXML_CHAR* pattern);

/*
 Set a new comparison function to evaluate whether a string matches a given pattern.
 The default one is the "regstrcmp" which handles limited regular expressions.
 'fct' prototype is 'int fct(SXML_CHAR* str, SXML_CHAR* pattern)' where 'str' is the string to
 evaluate the match for and 'pattern' the pattern. It should return 'true' (=1) when
 'str' matches 'pattern' and 'false' (=0) when it does not.
 Return the previous function used for matching.
 */
REGEXPR_COMPARE XMLSearch_set_regexpr_compare(REGEXPR_COMPARE fct);

/*
 Initialize 'search' struct to an empty search.
 No memory freeing is performed.
 Return 'false' when 'search' is NULL.
 */
int XMLSearch_init(XMLSearch* search);

/*
 Free all 'search' members except for the 'search->next' member that should be freed
 by its creator, unless 'free_next' is 'true'.
 It is recommended that 'free_next' is positioned to 'true' only when the creator did not
 handle the whole memory allocation chain, e.g. when using 'XMLSearch_init_from_XPath'
 that allocates all search structs.
 Return 'false' when 'search' is NULL.
 */
int XMLSearch_free(XMLSearch* search, int free_next);

/*
 Set the search based on tag.
 'tag' should be NULL or empty to search for any node (e.g. search based on attributes
 only). In this case, the previous tag is freed.
 Return 'true' upon successful completion, 'false' for memory error.
 */
int XMLSearch_search_set_tag(XMLSearch* search, const SXML_CHAR* tag);

/*
 Add an attribute search criteria.
 'attr_name' is mandatory. 'attr_value' should be NULL to test for attribute presence only
 (no test on value).  An empty string for 'attr_value' is not an equivalent to 'NULL'!
 'value_equal' should be specified to test for attribute value equality (='true') or
 difference (='false).
 Return the index of the new attribute, or '-1' for memory error.
 */
int XMLSearch_search_add_attribute(XMLSearch* search, const SXML_CHAR* attr_name, const SXML_CHAR* attr_value, int value_equal);

/*
 Search for attribute 'attr_name' in Search attribute list and return its index
 or '-1' if not found.
 */
int XMLSearch_search_get_attribute_index(const XMLSearch* search, const SXML_CHAR* attr_name);

/*
 Removes the search attribute given by its index 'i_attr'.
 Return the number of attributes left.
 */
int XMLSearch_search_remove_attribute(XMLSearch* search, int i_attr);

/*
 Set the search based on text content.
 'text' should be NULL or empty to search for any node (e.g. search based on attributes
 only). In this case, the previous text is freed.
 Return 'true' upon successful completion, 'false' for memory error.
 */
int XMLSearch_search_set_text(XMLSearch* search, const SXML_CHAR* text);

/*
 Set an additional search on children nodes of a previously matching node.
 Search struct are chained to finally return the node matching the last search struct,
 which father node matches the previous search struct, and so on.
 This allows describing more complex search queries like XPath
 "//FatherTag[@attrib=val]/ChildTag/".
 In this case, a first search struct would have 'search->tag = "FatherTag"' and
 'search->attributes[0] = { "attrib", "val" }' and a second search struct with
 'search->tag = "ChildTag"'.
 If 'children_search' is NULL, next search is removed. Freeing previous search is to be
 performed by its owner.
 In any case, if 'search' next search is not NULL, it is freed.
 Return 'true' when association has been made, 'false' when an error occurred.
 */
int XMLSearch_search_set_children_search(XMLSearch* search, XMLSearch* children_search);

/*
 Compute an XPath-equivalent string of the 'search' criteria.
 'xpath' is a pointer to a string that will be allocated by the function and should
 be freed after use.
 'quote' is the quote character to be used (e.g. '"' or '\''). If '\0', XML_DEFAULT_QUOTE will be used.
 A NULL 'search' will return an empty string.
 Return 'false' for a memory problem, 'true' otherwise.
 */
SXML_CHAR* XMLSearch_get_XPath_string(const XMLSearch* search, SXML_CHAR** xpath, SXML_CHAR quote);

/*
 Initialize a 'search' struct from an XPath-like query. "XPath-like" means that
 it does not fully comply to XPath standard.
 'xpath' should be like "tag[.=text, @attrib="value", @attrib!='value']/tag...".
 Warning: the XPath query on node text like 'father[child="text"]' should be
 re-written 'father/child[.="text"]' instead (which should be XPath-compliant as well).
 Return 'true' when 'search' was correctly initialized, 'false' in case of memory
 problem or malformed 'xpath'.
 */
int XMLSearch_init_from_XPath(SXML_CHAR* xpath, XMLSearch* search);

/*
 Check whether a 'node' matches 'search' criteria.
 'node->tag_type' should be 'TAG_FATHER' or 'TAG_SELF' only.
 If 'search->prev' is not nULL (i.e. has a father search), 'node->father' is also
 tested, recursively (i.e. grand-father and so on).
 Return 'false' when 'node' does not match or for invalid arguments, 'true'
 if 'node' is a match.
 */
int XMLSearch_node_matches(const XMLNode* node, const XMLSearch* search);

/*
 Search next matching node, according to search parameters given by 'search'.
 Search starts from node 'from' by scanning all its children, and going up to siblings,
 uncles and so on.
 Searching for the next matching node is performed by running the search again on the last
 matching node. So 'search' has to be initialized by 'XMLSearch_init' prior to the first call,
 to memorize the initial 'from' node and know where to stop search.
 'from' ITSELF IS NOT CHECKED! Direct call to 'XMLSearch_node_matches(from, search);' should
 be made if necessary.
 If the document has several root nodes, a complete search in the document should be performed
 by manually calling 'XMLSearch_next' on each root node in a for loop.
 Note that 'search' should be the initial search struct (i.e. 'search->prev' should be NULL). This
 cannot be checked/corrected by the function itself as it is partly recursive.
 Return the next matching node according to 'search' criteria, or NULL when no more nodes match
 or when an error occurred.
 */
XMLNode* XMLSearch_next(const XMLNode* from, XMLSearch* search);

/*
 Get 'node' XPath-like equivalent: 'tag[.="text", @attribute="value", ...]', potentially
 including father nodes XPathes.
 The computed XPath is stored in a dynamically-allocated string.
 Return the XPath, or NULL if 'node' is invalid or on memory error.
 */
SXML_CHAR* XMLNode_get_XPath(XMLNode* node, SXML_CHAR** xpath, int incl_parents);

#ifdef __cplusplus
}
#endif

#endif
