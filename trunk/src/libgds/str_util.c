// =================================================================
// @(#)str_util.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 24/07/2003
// @lastdate 04/12/2007
// =================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include <libgds/memory.h>
#include <libgds/str_util.h>

// -----[ strsep ]--------------------------------------------------
/**
 * Replacement for strsep() in case it is not system-provided. We
 * should check that strcspn() is available.
 */
#ifndef HAVE_STRSEP
char * strsep(char ** ppcStr, const char * pcDelim)
{
  char *pcSave = *ppcStr;
  if(*ppcStr == NULL)
    return NULL;
  *ppcStr = *ppcStr + strcspn(*ppcStr, pcDelim);
  if(**ppcStr == 0)
    *ppcStr = NULL;
  else{
    **ppcStr = 0;
    (*ppcStr)++;
  }
  return pcSave;
}
#endif

// ----- str_lcreate ------------------------------------------------
/**
 * Create a string of the given size (+ NUL-termination).
 *
 * Pre: tLen >= 0
 *
 * Post: (result != NULL) && (result NUL-terminated)
 */
char * str_lcreate(size_t tLen)
{
  char * pcNewString= (char *) MALLOC(sizeof(char)*(tLen+1));
  *pcNewString= '\0';
  return pcNewString;
}

// ----- str_create -------------------------------------------------
/**
 * Create a copy of the given string. If the source string is NULL,
 * the resulting string will be an empty, NUL-terminated string.
 *
 * Pre: (pcString == NULL) || (pcString NUL-terminated)
 * 
 * Post: (result != NULL) && (result NUL-terminated)
 */
char * str_create(const char * pcString)
{
  char * pcNewString;

  if (pcString != NULL) {
    pcNewString= str_lcreate(strlen(pcString));
    strcpy(pcNewString, pcString);
    return pcNewString;
  } else
    return str_lcreate(0);
}

// ----- str_ncreate ------------------------------------------------
/**
 * Create a copy of the given string with at most the given number of
 * characters (resulting string buffer will be of size len+1).
 *
 * Pre: (tLen >= 0) && ((pcString == NULL) || (pcString NUL-terminated))
 *
 * Post:(result != NULL) && (result NUL-terminated)
 */
char * str_ncreate(const char * pcString, size_t tLen)
{
  char * pcNewString= NULL;

  if ((pcString != NULL) && (tLen > 0)) {
    pcNewString= str_lcreate(tLen+1);
    strncpy(pcNewString, pcString, tLen);

    // strncpy() does not guarantee to NUL-terminate the string. We
    // have to this explicitly.
    pcNewString[tLen]= '\0';
  } else
    pcNewString= str_lcreate(0);

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
 * Free the given string.
 *
 * Post: *ppcString == NULL
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
 * Append a string to another.
 *
 * Pre:
 *   *ppcString is valid and NUL-terminated
 *   pcToAppend can be NULL
 *
 * Post: (result != NULL) && (result NUL-terminated)
 */
char * str_append(char ** ppcString, const char * pcToAppend)
{
  size_t tLenToAppend= 0;
  size_t tLen;

  if (pcToAppend != NULL)
    tLenToAppend= strlen(pcToAppend);

  if (tLenToAppend > 0) {
    tLen= strlen(*ppcString);
    *ppcString= str_lextend(ppcString, tLen+tLenToAppend);
    strcpy((*ppcString)+tLen, pcToAppend);
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

// ----- str_prepend ------------------------------------------------
/**
 * Prepend a string with another.
 *
 * Pre:
 *   pcString is valid and NUL-terminated
 *   pcToPrepend can be NULL
 *
 * Post: (return != NULL) && (return NUL-terminated)
 */
char * str_prepend(char ** ppcString, const char * pcToPrepend)
{
  size_t tLenToPrepend= 0;
  size_t tLen;
  char * pcNewString;

  if (pcToPrepend != NULL)
    tLenToPrepend= strlen(pcToPrepend);

  if (tLenToPrepend > 0) {
    tLen= strlen(*ppcString);
    pcNewString= str_ncreate(pcToPrepend, tLen+tLenToPrepend);
    // The following line is not the most efficient since I guess it
    // has to recompute the length of *ppcString before appending !!!
    strcat(pcNewString, *ppcString);
    *ppcString= pcNewString;
  }
  return *ppcString;
}

// ----- str_translate ----------------------------------------------
/**
 * Translate the characters in the string.
 *
 * Pre:
 *   - all input strings are valid and NUL-terminated
 *   - the length of both characters sets are equal
 */
void str_translate(char * pcString, const char * pcSrcChars,
		   const char * pcDstChars)
{
  char * pcCharPos;
  char * pcStrPos= pcString;

  while (*pcStrPos != 0) {
    pcCharPos= strchr(pcSrcChars, *pcStrPos);
    if (pcCharPos != NULL) {
      pcCharPos+= pcDstChars-pcSrcChars;
      *pcStrPos= *pcCharPos;
    }
    pcStrPos++;
  }
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
