// =================================================================
// @(#)str_util.c
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 24/07/2003
// @lastdate 24/07/2003
// =================================================================

#include <libgds/memory.h>
#include <libgds/str_util.h>

// ----- str_lcreate ------------------------------------------------
/**
 *
 */
char * str_lcreate(size_t tLen)
{
  return (char *) MALLOC(sizeof(char)*(tLen+1));
}

// ----- str_create -------------------------------------------------
/**
 *
 */
char * str_create(const char * pcString)
{
  char * pcNewString= str_lcreate(strlen(pcString));
  strcpy(pcNewString, pcString);
  return pcNewString;
}

// ----- str_ncreate ------------------------------------------------
/**
 *
 */
char * str_ncreate(const char * pcString, size_t tLen)
{
  char * pcNewString= str_lcreate(tLen);
  strncpy(pcNewString, pcString, tLen);
  pcNewString[tLen]= '\0';
  return pcNewString;
}

// ----- str_lextend ------------------------------------------------
/**
 *
 */
char * str_lextend(char ** ppcString, size_t tNewLen)
{
  *ppcString= (char *) REALLOC(*ppcString, sizeof(char)*(tNewLen+1));
  return *ppcString;
}

// ----- str_destroy ------------------------------------------------
/**
 *
 */
void str_destroy(char ** ppcString)
{
  if (*ppcString != NULL) {
    FREE(*ppcString);
    *ppcString= NULL;
  }
}

// ----- str_append -------------------------------------------------
/**
 * PRE: ppcString initialized (allocated) && (pStrToAppend != NULL)
 * POST: ppcString updated with new string. New string also returned.
 */
char * str_append(char ** ppcString, const char * pcStrToAppend)
{
  size_t tLenToAppend= strlen(pcStrToAppend);

  if (tLenToAppend > 0) {
    *ppcString= str_lextend(ppcString, strlen(*ppcString)+tLenToAppend);
    // The following line is not the most efficient since I guess it
    // has to recompute the length of *ppcString before appending !!!
    strcat(*ppcString, pcStrToAppend);
  }
  return *ppcString;
}

// ----- str_nappend ------------------------------------------------
/**
 *
 */
char * str_nappend(char ** ppcString, const char * pcStrToAppend,
		   size_t tLen)
{
  if (tLen > 0) {
    *ppcString= str_lextend(ppcString, strlen(*ppcString)+tLen);
    // The following line is not the most efficient since I guess it
    // has to recompute the length of *ppcString before appending !!!
    strncat(*ppcString, pcStrToAppend, tLen);
    (*ppcString)[strlen(*ppcString)+tLen]= '\0';
  }
  return *ppcString;
}
