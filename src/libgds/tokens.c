// =================================================================
// @(#)tokens.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 20/08/2003
// @lastdate 01/03/2004
// =================================================================

#include <stdlib.h>
#include <str_util.h>
#include <tokens.h>

// ----- tokens_item_destroy ----------------------------------------
void tokens_item_destroy(void * pItem)
{
  str_destroy((char **) pItem);
}

// ----- tokens_create ----------------------------------------------
/**
 *
 */
STokens * tokens_create()
{
  return (STokens *) ptr_array_create(0, NULL, tokens_item_destroy);
}

// ----- tokens_destroy ---------------------------------------------
/**
 *
 */
void tokens_destroy(STokens ** ppTokens)
{
  ptr_array_destroy((SPtrArray **) ppTokens);
}

// ----- tokens_add -------------------------------------------------
/**
 *
 */
int tokens_add(STokens * pTokens, char * pcToken)
{
  return ptr_array_add((SPtrArray *) pTokens, &pcToken);
}

// ----- tokens_add_copy --------------------------------------------
/**
 *
 */
int tokens_add_copy(STokens * pTokens, char * pcToken)
{
  char * pcTokenCopy= str_create(pcToken);
  return ptr_array_add((SPtrArray *) pTokens, &pcTokenCopy);
}

// ----- tokens_get_num ---------------------------------------------
/**
 *
 */
unsigned int tokens_get_num(STokens * pTokens)
{
  return ptr_array_length((SPtrArray *) pTokens);
}

// ----- tokens_get_string_at ---------------------------------------
/**
 *
 */
char * tokens_get_string_at(STokens * pTokens, unsigned int uIndex)
{
  if (uIndex < tokens_get_num(pTokens))
    return (char *) pTokens->data[uIndex];
  return NULL;
}

// ----- tokens_get_long_at -----------------------------------------
/**
 *
 */
int tokens_get_long_at(STokens * pTokens, uint16_t uIndex,
		       long int * plValue)
{
  long int lValue;
  char * pcValue;
  char * pcEndPtr;

  if (plValue == NULL)
    return -1;
  if (uIndex < tokens_get_num(pTokens)) {
    pcValue= pTokens->data[uIndex];
    lValue= strtol(pcValue, &pcEndPtr, 0);
    *plValue= lValue;
    return (*pcEndPtr == 0)?0:-1;
  }
  return -1;
}

// ----- tokens_get_int_at ------------------------------------------
/**
 *
 */
int tokens_get_int_at(STokens * pTokens, uint16_t uIndex,
		      int * piValue)
{
  long int lValue;

  if (piValue == NULL)
    return -1;
  if (tokens_get_long_at(pTokens, uIndex, &lValue) != 0)
    return -1;
  if ((lValue < INT_MIN) || (lValue > INT_MAX))
    return -1;
  *piValue= lValue;
  return 0;
}

// ----- tokens_get_ulong_at ----------------------------------------
/**
 *
 */
int tokens_get_ulong_at(STokens * pTokens, uint16_t uIndex,
			unsigned long int * pulValue)
{
  unsigned long int ulValue;
  char * pcValue;
  char * pcEndPtr;

  if (pulValue == NULL)
    return -1;
  if (uIndex < tokens_get_num(pTokens)) {
    pcValue= pTokens->data[uIndex];
    ulValue= strtoul(pcValue, &pcEndPtr, 0);
    *pulValue= ulValue;
    return (*pcEndPtr == 0)?0:-1;
  }
  return -1;
}

// ----- tokens_get_uint_at -----------------------------------------
/**
 *
 */
int tokens_get_uint_at(STokens * pTokens, uint16_t uIndex,
		       unsigned int * puValue)
{
  unsigned long int ulValue;

  if (puValue == NULL)
    return -1;
  if (tokens_get_ulong_at(pTokens, uIndex, &ulValue) != 0)
    return -1;
  if (ulValue > UINT_MAX)
    return -1;
    *puValue= ulValue;
  return 0;
}

// ----- tokens_get_double_at ---------------------------------------
/**
 *
 */
int tokens_get_double_at(STokens * pTokens, uint16_t uIndex,
			 double * pdValue)
{
  double dValue;
  char * pcValue;
  char * pcEndPtr;

  if (pdValue == NULL)
    return -1;
  if (uIndex < tokens_get_num(pTokens)) {
    pcValue= pTokens->data[uIndex];
    dValue= strtod(pcValue, &pcEndPtr);
    *pdValue= dValue;
    return (*pcEndPtr == 0)?0:-1;
  }
  return -1;
}
