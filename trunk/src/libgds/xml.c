// ==================================================================
// @(#)xml.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 26/08/2004
// @lastdate 27/01/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBXML

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <libgds/log.h>
#include <libgds/xml.h>
#include <libgds/memory.h>
#include <libgds/stack.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>


struct xmlParsingInternals
{
  xmlDocPtr	      pDoc;
  xmlNodePtr	      pRootNode;
  xmlNodePtr          pNode;
  SStack *	      sNode;
  xmlNodeSetPtr	      pNodeSet;
  xmlNodePtr	      pRootNodeSet;
  int		      iNodeSetElt;
  xmlXPathContextPtr  xpathCtx;
  xmlXPathObjectPtr   xpathObj;
};


//******************************************************************//
//			XPath Section				    //
//******************************************************************//

// ----- xml_xpath_search_init ---------------------------------------
/**
 *
 *
 */
int xml_xpath_search_init(xmlHandle * hXML)
{
  hXML->xpathCtx = xmlXPathNewContext(hXML->pDoc);
  if (hXML->xpathCtx == NULL) {
    LOG_DEBUG("xml_init_xpath_search> unable to create new XPath context.\n");
    xmlFreeDoc(hXML->pDoc);
    return -1;
  }
  return 0;
}

// ----- xml_xpath_search_finalize -----------------------------------
/**
 *
 *
 */
void xml_xpath_search_finalize(xmlHandle * hXML)
{
  if (hXML->xpathObj != NULL)
    xmlXPathFreeObject(hXML->xpathObj);
  hXML->xpathObj = NULL;

  xmlXPathFreeContext(hXML->xpathCtx);
  hXML->xpathCtx = NULL;
}

// ----- xml_xpath_search ----------------------------------------------
/**
 *
 *
 */
int xml_xpath_search (xmlHandle * hXML, const char * xmlExpr)
{
  // means that a search has already been done 'cause the initialization
  // has set it to NULL
  if (hXML->xpathObj)
    xmlXPathFreeObject(hXML->xpathObj);

  hXML->xpathObj = xmlXPathEvalExpression(xmlExpr, hXML->xpathCtx);
  if(hXML->xpathObj == NULL) {
    LOG_DEBUG("xml_path_search> unable to evaluate xpath expression \"%s\"\n", (const xmlChar *)xmlExpr);
    xmlXPathFreeContext(hXML->xpathCtx);
    xmlFreeDoc(hXML->pDoc);
    return -1;
  }

  hXML->pNodeSet = hXML->xpathObj->nodesetval;
  if (hXML->xpathObj->nodesetval->nodeNr) 
    hXML->pRootNodeSet = hXML->pNode = hXML->xpathObj->nodesetval->nodeTab[0];
  
  return 0;
}

// ----- xml_xpath_first_nodeset ---------------------------------------------
/**
 *
 *
 */
void xml_xpath_first_nodeset(xmlHandle * hXML)
{
  hXML->pRootNodeSet = hXML->xpathObj->nodesetval->nodeTab[0];
  hXML->pNodeSet = hXML->xpathObj->nodesetval;
  hXML->iNodeSetElt = 0;
}


//******************************************************************//
// General section  ** Parsing **				    //
//******************************************************************//

// ----- xml_handle_init ---------------------------------------------
/**
 *
 *
 */
xmlHandle * xml_handle_init()
{
  xmlHandle * hXML = malloc(sizeof(xmlHandle));

  hXML->pDoc	      = NULL;
  hXML->pNode	      = NULL;
  hXML->pRootNode     = NULL;
  hXML->pNodeSet      = NULL;
  hXML->sNode	      = NULL;
  hXML->pRootNodeSet  = NULL;
  hXML->iNodeSetElt   = 0;
  hXML->xpathCtx      = NULL;
  hXML->xpathObj      = NULL;

  return hXML;
}

// ----- xml_destroy_handle ------------------------------------------
/**
 * We have to free the internals of this structure before calling it !!!
 * maybe place a bit of code freing preventively the structures 
 *
 */
void xml_handle_destroy(xmlHandle * hXML)
{
  FREE (hXML);
}

// ----- xml_parsing_init --------------------------------------------
/**
 * Open a xml document, initialize pNode and pRootNode to the root of 
 * the document and returns a handle
 *
 */
xmlHandle * xml_parsing_init(char *pDocName)
{
  assert(pDocName);
  xmlDocPtr pDoc;
  xmlHandle * hXML;

  xmlInitParser();

  pDoc = xmlParseFile(pDocName);
  if (pDoc == NULL) {
    LOG_DEBUG("xml_parsing_init> There is no document to parse!\n");
    return NULL;
  }

  hXML = xml_handle_init();
  hXML->pRootNode = hXML->pNode = xmlDocGetRootElement(pDoc);
  hXML->pDoc = pDoc;
  hXML->sNode = stack_create(10);

  return hXML;
}

// ----- xml_parsing_finalize ----------------------------------------
/** 
 * Destroys pDoc and cleanup the parser
 * 
 */
void xml_parsing_finalize(xmlHandle * hXML)
{
    stack_destroy(&(hXML->sNode));
    xmlFreeDoc(hXML->pDoc);
    xml_handle_destroy(hXML);
    xmlCleanupParser();
}

// ----- xml_reset_search --------------------------------------------
/**
 *
 *
 */
void xml_parsing_reset(xmlHandle * hXML)
{
  hXML->pNode = hXML->pRootNode;

  if (hXML->pNodeSet != NULL)
    xmlXPathFreeNodeSet(hXML->pNodeSet);
  
  hXML->pNodeSet = NULL;
  hXML->pRootNodeSet = NULL;

  if (hXML->xpathCtx)
    xml_xpath_search_finalize(hXML);
}

//******************************************************************//
//		  NodeSet Section				    //
//******************************************************************//

// ----- xml_nodeset_get_next_node ------------------------------------
/** 
 * Get the next node
 * 
 */
void xml_nodeset_get_next_node(xmlHandle * hXML)
{
    hXML->pNode = hXML->pNode->next;
}

// ----- xml_nodeset_get_next ----------------------------------------
/**
 *
 *
 */
int xml_nodeset_get_next (xmlHandle * hXML)
{
  if (hXML->pNodeSet->nodeNr <= hXML->iNodeSetElt+1)
    return -1;
  
  hXML->iNodeSetElt++;
  hXML->pNode = hXML->pNodeSet->nodeTab[hXML->iNodeSetElt];
  hXML->pRootNode = hXML->pNode; 
  hXML->pRootNodeSet = hXML->pNode;

  return 0;
}

// ----- xml_nodeset_push_node ---------------------------------------
/**
 *
 *
 */
void xml_nodeset_push_node(xmlHandle * hXML)
{
  if (stack_push(hXML->sNode, (void *)hXML->pNode) == 0)
    LOG_DEBUG("xml_nodeset_push_node> can't push another item.\n");
}

// ----- xml_nodeset_pop_node ----------------------------------------
/**
 *
 *
 */
void xml_nodeset_pop_node(xmlHandle * hXML)
{
  hXML->pNode = (xmlNodePtr)stack_pop(hXML->sNode);
}

// ----- xml_nodeset_search_internals --------------------------------
/**
 *
 *
 */
static char cFound;
int xml_nodeset_search_internals(xmlHandle * hXML, 
				  const char * xmlExpr, 
				  xmlNodePtr pCurNode)
{
  while (pCurNode && !cFound) {
    if (!xmlStrcmp(pCurNode->name, (const xmlChar *) xmlExpr)) {
      hXML->pNode = pCurNode;
      cFound = 1;
    }
    xml_nodeset_search_internals(hXML, xmlExpr, pCurNode->children);
    pCurNode = pCurNode->next;
  }
  return (cFound == 1) ? 0 : -1;
}




// ----- xml_nodeset_conditional_stop --------------------------------
/**
 *
 *
 */
int xml_nodeset_conditional_stop(xmlHandle * hXML, 
				  const char * xmlExprCond,
				  xmlNodePtr pNode) 
{
  return (pNode && pNode != hXML->pNode &&
      !xmlStrcmp(pNode->name, (const xmlChar *)xmlExprCond)) ? 0 : 1;
    
}

// ----- xml_nodeset_search_conditional_stop_internals ---------------
/**
 *
 *
 */
int xml_nodeset_search_conditional_stop_internals(xmlHandle * hXML,
						const char * xmlExprSearch,
						const char * xmlExprCond,
						xmlNodePtr pCurNode)
{
  int CondStop = 1;

  CondStop = xml_nodeset_conditional_stop(hXML, xmlExprCond, pCurNode);
    
  while (pCurNode && !cFound && CondStop) {
    if (!xmlStrcmp(pCurNode->name, (const xmlChar *)xmlExprSearch)) {
      hXML->pNode = pCurNode;
      cFound = 1;
    }
    xml_nodeset_search_conditional_stop_internals(hXML, xmlExprSearch, 
						    xmlExprCond, 
						    pCurNode->children);
    pCurNode = pCurNode->next;
    CondStop = xml_nodeset_conditional_stop(hXML, xmlExprCond, pCurNode);
  }

  return (cFound == 1) ? 0 : -1;
}

// ----- xml_nodeset_search ------------------------------------------
/**
 *
 *
 */
int xml_nodeset_search (xmlHandle * hXML, const char * xmlExpr)
{
  cFound = 0;
  return xml_nodeset_search_conditional_stop_internals(hXML, xmlExpr, "", hXML->pNode);
}
// ----- xml_nodeset_search_conditional_stop -------------------------
/**
 *
 *
 */
int xml_nodeset_search_conditional_stop (xmlHandle * hXML, 
					  const char * xmlExprSearch, 
					  const char * xmlExprCond)
{
  cFound = 0;
  return xml_nodeset_search_conditional_stop_internals(hXML, xmlExprSearch, 
							xmlExprCond,
							hXML->pNode);
}

// ----- xml_nodeset_search_reset ------------------------------------
/**
 *
 *
 */
int xml_nodeset_search_reset(xmlHandle * hXML)
{
  if (hXML->pRootNodeSet == NULL)
    return -1;
  
  hXML->pNode = hXML->pRootNodeSet;
  return 0;
}

//******************************************************************//
//		  Node Section					    //
//******************************************************************//

// ----- xml_node_cast_data_type -------------------------------------
/**
 *
 *
 */
void * xml_node_cast_data_type(xmlChar * cContent, uint8_t uDataType)
{ 
  int iContent;
  
  switch (uDataType) {
    case XML_STRING_DATA_TYPE:
      return (void *)cContent;
      break;
    case XML_INT_DATA_TYPE:
      iContent = atoi((const char *)cContent);
      xml_char_free(cContent);
      return (void *)iContent;
      break;
    case XML_FLOAT_DATA_TYPE:
      //return (void *)atof((const char *)cContent);
      break;
    default:
      LOG_DEBUG("xml_node_get_content> Data Type is not recognized.\n");
      break;
  }
  return NULL;
}

// ------ xml_node_get_content ---------------------------------------
/**
 *
 *
 */
void * xml_node_get_content (xmlHandle * hXML, uint8_t uDataType)
{
  xmlChar * cContent;

  cContent = xmlNodeGetContent(hXML->pNode->children);
  return (cContent) ? (void *)xml_node_cast_data_type(cContent, uDataType) : NULL;
}

// ----- xml_node_get_attribute -------------------------------------------
/**
 *
 *
 */
void * xml_node_get_attribute (xmlHandle * hXML, const char *cAttr, 
				uint8_t uDataType)
{
  char * cContent;

  cContent = xmlGetProp(hXML->pNode, (const xmlChar *)cAttr);
  return (cContent) ? xml_node_cast_data_type(cContent, uDataType) : NULL;
}

// ----- xml_char_free -----------------------------------------------
/**
 *
 *
 */
void xml_char_free(char * cContent)
{
  xmlFree(cContent);
}

#endif
