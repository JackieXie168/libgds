// ==================================================================
// @(#)xml.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 26/08/2004
// @lastdate 26/08/2004
// ==================================================================

#ifndef __GDS_XML_H__
#define __GDS_XML_H__

#define XML_STRING_DATA_TYPE  0x1
#define XML_INT_DATA_TYPE     0x2
#define XML_FLOAT_DATA_TYPE   0x3

#include <libgds/types.h>

typedef struct xmlParsingInternals xmlHandle;


// ----- xml_parsing_init --------------------------------------------
xmlHandle * xml_parsing_init(char *pDocName);
// ----- xml_parsing_finalize ----------------------------------------
void xml_parsing_finalize(xmlHandle * hXML);
// ----- xml_parsing_reset -------------------------------------------
void xml_parsing_reset(xmlHandle * hXML);

// ----- xml_nodeset_get_next_node -----------------------------------
void xml_nodeset_get_next_node(xmlHandle * hXML);
// ----- xml_nodeset_search ------------------------------------------
int xml_nodeset_search(xmlHandle * hXML,
				    const char * xmlExpr);
// ----- xml_nodeset_search_conditional_stop_internals ---------------
int xml_nodeset_search_conditional_stop (xmlHandle * hXML, 
					  const char * xmlExprSearch, 
					  const char * xmlExprCond);
// ----- xml_nodeset_get_next ----------------------------------------
int xml_nodeset_get_next (xmlHandle * hXML);
// ----- xml_nodeset_search_reset ------------------------------------
int xml_nodeset_search_reset(xmlHandle * hXML);
// ----- xml_nodeset_pop_node ----------------------------------------
void xml_nodeset_pop_node(xmlHandle * hXML);
// ----- xml_nodeset_push_node ---------------------------------------
void xml_nodeset_push_node(xmlHandle * hXML);

// ----- xml_node_get_content ----------------------------------------
void * xml_node_get_content(xmlHandle * hXML, uint8_t uDataType);
// ----- xml_node_get_attribute --------------------------------------
void * xml_node_get_attribute(xmlHandle * hXML, const char * cAttr, 
				    uint8_t uDataType);
// ----- xml_char_free -----------------------------------------------
void xml_char_free(char * cContent);
// ----- xml_xpath_search_init ---------------------------------------
int xml_xpath_search_init(xmlHandle * hXML);
// ----- xml_xpath_search_finalize -----------------------------------
void xml_xpath_search_finalize(xmlHandle * hXML);
// ----- xml_xpath_search --------------------------------------------
int xml_xpath_search(xmlHandle * hXML, const char * xmlExpr);
// ----- xml_xpath_first_nodeset -------------------------------------
void xml_xpath_first_nodeset(xmlHandle * hXML);

#endif
