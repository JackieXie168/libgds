// =================================================================
// @(#)str_util.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 24/07/2003
// @lastdate 15/01/2007
// =================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

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
  char * pcNewString= str_lcreate(tLen+1);
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

// ----- str_as_long ------------------------------------------------
/**
 * Convert a string to a long (LONG_MIN <= X <= LONG_MAX).
 */
int str_as_long(const char * pcString, long int * plValue)
{
  long int lValue;
  char * pcEndPtr;

  if (plValue == NULL)
    return -1;
  lValue= strtol(pcString, &pcEndPtr, 0);
  if (errno == ERANGE)
    return -1;
  *plValue= lValue;
  return (*pcEndPtr == 0)?0:-1;
}

// ----- str_as_int -------------------------------------------------
/**
 * Convert a string to an integer (INT_MIN <= X <= INT_MAX).
 */
int str_as_int(const char * pcString, int * piValue)
{
  long int lValue;

  if (piValue == NULL)
    return -1;
  if (str_as_long(pcString, &lValue) != 0)
    return -1;
  if ((lValue < INT_MIN) || (lValue > INT_MAX))
    return -1;
  *piValue= lValue;
  return 0;
}

// ----- str_as_ulong -----------------------------------------------
/**
 * Convert a string to an unsigned long (0 <= X <= ULONG_MAX).
 * 
 * The function uses 'strtoll()' in order to convert the string to a
 * 'long long int'. We cannot use 'strtol()' since it is limited to a
 * maximum value of LONG_MAX while we need ULONG_MAX. We cannot use
 * 'strtoul()' since it will convert a negative number to a positive
 * number.
 */
int str_as_ulong(const char * pcString, unsigned long int * pulValue)
{
  long long int llValue;
  char * pcEndPtr;

  if (pulValue == NULL)
    return -1;
  llValue= strtoll(pcString, &pcEndPtr, 0);
  /* Check for errors and bounds (0 <= x <= ULONG_MAX) */
  if ((errno == ERANGE) || (llValue < 0) || (llValue > ULONG_MAX))
    return -1;
  *pulValue= (unsigned long int) llValue;
  return (*pcEndPtr == 0)?0:-1;
}

// ----- str_as_uint ------------------------------------------------
/**
 * Convert a string to an unsigned int (0 < X <= UINT_MAX).
 */
int str_as_uint(const char * pcString, unsigned int * puValue)
{
  unsigned long int ulValue;

  if (puValue == NULL)
    return -1;
  if (str_as_ulong(pcString, &ulValue) != 0)
    return -1;
  if (ulValue > UINT_MAX)
    return -1;
  *puValue= ulValue;
  return 0;
}

// ----- str_as_double ----------------------------------------------
/**
 *
 */
int str_as_double(const char * pcString, double * pdValue)
{
  double dValue;
  char * pcEndPtr;

  if (pdValue == NULL)
    return -1;
  dValue= strtod(pcString, &pcEndPtr);
  if (errno == ERANGE)
    return -1;
  *pdValue= dValue;
  return (*pcEndPtr == 0)?0:-1;
}
