// =================================================================
// @(#)str_util.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 24/07/2003
// $Id$
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
char * str_lcreate(size_t len)
{
  char * new_s= (char *) MALLOC(sizeof(char)*(len+1));
  *new_s= '\0';
  return new_s;
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
char * str_create(const char * s)
{
  char * new_s;

  if (s == NULL)
    return str_lcreate(0);

  new_s= str_lcreate(strlen(s));
  strcpy(new_s, s);
  return new_s;
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
char * str_ncreate(const char * s, size_t len)
{
  char * new_s= NULL;

  if ((s == NULL) || (len == 0))
    return str_lcreate(0);
  
  new_s= str_lcreate(len);
  strncpy(new_s, s, len);
  
  // strncpy() does not guarantee to NUL-terminate the string. We
  // have to this explicitly.
  new_s[len]= '\0';
  return new_s;
}

// ----- str_lextend ------------------------------------------------
/**
 *
 */
char * str_lextend(char ** s, size_t new_len)
{
  *s= (char *) REALLOC(*s, sizeof(char)*(new_len+1));
  return *s;
}

// ----- str_destroy ------------------------------------------------
/**
 * Free the given string.
 *
 * Post:
 *   *s == NULL
 */
void str_destroy(char ** s)
{
  if (*s != NULL) {
    FREE(*s);
    *s= NULL;
  }
}

// ----- str_append -------------------------------------------------
/**
 * Append a string to another.
 *
 * Pre:
 *   (*s == NULL) || (*s is NUL-terminated)
 *   (append == NULL) || (append is NUL-terminated)
 *
 * Post:
 *   (result != NULL) && (result is NUL-terminated)
 */
char * str_append(char ** s, const char * append)
{
  size_t len_append= 0;
  size_t len;

  if (append != NULL)
    len_append= strlen(append);

  if (len_append > 0) {
    if (*s != NULL) {
      len= strlen(*s);
      *s= str_lextend(s, len+len_append);
      strcpy((*s)+len, append);
    } else {
      *s= str_create(append);
    }
  }
  return *s;
}

// ----- str_nappend ------------------------------------------------
/**
 * Append a string to another, result is limited in size.
 *
 * Pre:
 *   (*s == NULL) || (*s is NUL-terminated)
 *   (append == NULL) || (append is NUL-terminated)
 *
 * Post:
 *   (result != NULL)
 */
char * str_nappend(char ** s, const char * append,
		   size_t len)
{
  if ((*s == NULL) || (len == 0))
    return str_ncreate(append, len);

  *s= str_lextend(s, strlen(*s)+len);
  // The following line is not the most efficient since it
  // has to recompute the length of *s before appending !!!
  strncat(*s, append, len);
  (*s)[strlen(*s)+len]= '\0';
  return *s;
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


/////////////////////////////////////////////////////////////////////
//
// STRING BUFFER
//
/////////////////////////////////////////////////////////////////////

// -----[ str_buf_create ]-----------------------------------------
str_buf_t * str_buf_create(size_t min_size)
{
  str_buf_t * buf= MALLOC(sizeof(str_buf_t));
  buf->data= NULL;
  buf->size= 0;
  buf->min_size= min_size;
  buf->index= 0;
  return buf;
}

// -----[ str_buf_destroy ]----------------------------------------
void str_buf_destroy(str_buf_t ** buf)
{
  if (*buf != NULL) {
    str_destroy(&(*buf)->data);
    FREE(*buf);
    *buf= NULL;
  }
}

// -----[ str_buf_reset ]--------------------------------------------
void str_buf_reset(str_buf_t * buf)
{
  buf->index= 0;
  buf->invisible_data= 0;
}

// -----[ str_buf_write_char ]---------------------------------------
void str_buf_write_char(str_buf_t * buf, char c)
{
  // Check token buffer size
  if (buf->data == NULL) {
    buf->size= buf->min_size;
    buf->data= str_lcreate(buf->size);
  } else if (buf->index >= buf->size) {
    buf->size+= buf->min_size;
    str_lextend(&buf->data, buf->size);
  }

  // Store character
  buf->data[buf->index++]= c;
}

// -----[ str_buf_write_string ]-------------------------------------
void str_buf_write_string(str_buf_t * buf, const char * str)
{
  size_t len= strlen(str);

  // Check token buffer size
  if (buf->data == NULL) {
    buf->size= buf->min_size * ((len / buf->min_size)+1);
    buf->data= str_lcreate(buf->size);
  } else if (buf->index+len >= buf->size) {
    buf->size+= buf->min_size * ((len / buf->min_size)+1);
    str_lextend(&buf->data, buf->size);
  }

  // Store character
  strncpy(&buf->data[buf->index], str, len);
  buf->index+= len;
}

// -----[ str_buf_write_invisible ]----------------------------------
void str_buf_write_invisible(str_buf_t * buf)
{
  buf->invisible_data= 1;
}

// -----[ str_buf_empty ]--------------------------------------------
int str_buf_empty(str_buf_t * buf)
{
  return (!buf->invisible_data) && (buf->index == 0);
}
