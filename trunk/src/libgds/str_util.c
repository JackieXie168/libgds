// =================================================================
// @(#)str_util.c
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 24/07/2003
// $Id$
// =================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <errno.h>

#include <libgds/memory.h>
#include <libgds/str_util.h>

// -----[ strsep ]--------------------------------------------------
/**
 * Replacement for strsep() in case it is not system-provided. We
 * should check that strcspn() is available.
 *
 * Notes:
 *   strsep() is not standard (first appeared in 4.4BSD)
 *   strcspn() is ISO C90.
 */
#ifndef HAVE_STRSEP
char * strsep(char ** str, const char * delim)
{
  char *save = *str;
  if(*str == NULL)
    return NULL;
  *str = *str + strcspn(*str, delim);
  if(**str == 0)
    *str = NULL;
  else{
    **str = 0;
    (*str)++;
  }
  return save;
}
#endif /* HAVE_STRSEP */

// -----[ str_lcreate ]----------------------------------------------
/**
 * Create a string of the given size (+ NUL-termination).
 *
 * Pre: len >= 0
 *
 * Post: (result != NULL) && (result NUL-terminated)
 */
char * str_lcreate(size_t len)
{
  char * new_s= (char *) MALLOC(sizeof(char)*(len+1));
  *new_s= '\0';
  return new_s;
}

// -----[ str_create ]-----------------------------------------------
/**
 * Create a copy of the given string. If the source string is NULL,
 * the resulting string will be an empty, NUL-terminated string.
 *
 * Pre: (s == NULL) || (s NUL-terminated)
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

// -----[ str_ncreate ]----------------------------------------------
/**
 * Create a copy of the given string with at most the given number of
 * characters (resulting string buffer will be of size len+1).
 *
 * Pre: (len >= 0) && ((s == NULL) || (s NUL-terminated))
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

// -----[ str_lextend ]------------------------------------------------
/**
 *
 */
char * str_lextend(char * s, size_t new_len)
{
  return (char *) REALLOC(s, sizeof(char)*(new_len+1));
}

// -----[ str_destroy ]----------------------------------------------
/**
 * Free the given string.
 *
 * Post:
 *   *s == NULL
 */
void str_destroy(char ** s_ref)
{
  if (*s_ref != NULL) {
    FREE(*s_ref);
    *s_ref= NULL;
  }
}

// -----[ str_append ]-----------------------------------------------
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
char * str_append(char * s, const char * append)
{
  size_t len_append= 0;
  size_t len;
  char * new_s= s;

  if (append != NULL)
    len_append= strlen(append);

  if (len_append > 0) {
    if (s != NULL) {
      len= strlen(s);
      new_s= str_lextend(s, len+len_append);
      strcpy((new_s)+len, append);
    } else {
      new_s= str_create(append);
    }
  }
  return new_s;
}

// -----[ str_nappend ]----------------------------------------------
/**
 * Append a string to another, result is limited in size.
 *
 * Pre:
 *   (s == NULL) || (s is NUL-terminated)
 *   (append == NULL) || (append is NUL-terminated)
 *
 * Post:
 *   (result != NULL)
 */
char * str_nappend(char * s, const char * append, size_t len)
{
  char * new_s;

  if ((s == NULL) || (len == 0))
    return str_ncreate(append, len);

  new_s= str_lextend(s, strlen(s)+len);
  // The following line is not the most efficient since it
  // has to recompute the length of *s before appending !!!
  strncat(s, append, len);
  new_s[strlen(new_s)+len]= '\0';
  return new_s;
}

// -----[ str_prepend ]----------------------------------------------
/**
 * Prepend a string with another.
 *
 * Pre:
 *   pcString is valid and NUL-terminated
 *   pcToPrepend can be NULL
 *
 * Post: (return != NULL) && (return NUL-terminated)
 */
char * str_prepend(char * s, const char * prepend)
{
  size_t len_prepend= 0;
  size_t len;
  char * new_s= s;

  if (prepend != NULL)
    len_prepend= strlen(prepend);

  if (len_prepend > 0) {
    len= strlen(s);
    new_s= str_ncreate(prepend, len+len_prepend);
    // The following line is not the most efficient since I guess it
    // has to recompute the length of *ppcString before appending !!!
    strcat(new_s, s);
  }
  return new_s;
}

// -----[ str_translate ]--------------------------------------------
/**
 * Translate the characters in the string.
 *
 * Pre:
 *   - all input strings are valid and NUL-terminated
 *   - the length of both characters sets are equal
 */
void str_translate(char * s, const char * src_chars,
		   const char * dst_chars)
{
  char * c_pos;
  char * s_pos= s;

  while (*s_pos != 0) {
    c_pos= strchr(src_chars, *s_pos);
    if (c_pos != NULL) {
      c_pos+= dst_chars-src_chars;
      *s_pos= *c_pos;
    }
    s_pos++;
  }
}

// -----[ str_as_long ]----------------------------------------------
/**
 * Convert a string to a long (LONG_MIN <= X <= LONG_MAX).
 */
int str_as_long(const char * s, long int * value)
{
  long int long_value;
  char * endptr;

  if (value == NULL)
    return -1;
  errno= 0;
  long_value= strtol(s, &endptr, 0);
  // If no conversion could be performed, 0 is returned and errno <- EINVAL
  if (errno == EINVAL)
    return -1;
  // If under/over-flow occurs, errno <- ERANGE
  if (errno == ERANGE)
    return -1;
  *value= long_value;
  // We do full string conversions. There is an error if the char
  // pointed by endptr is not '\0'
  return (*endptr == 0)?0:-1;
}

// -----[ str_as_int ]-----------------------------------------------
/**
 * Convert a string to an integer (INT_MIN <= X <= INT_MAX).
 */
int str_as_int(const char * s, int * value)
{
  long int l_value;

  if (value == NULL)
    return -1;
  if (str_as_long(s, &l_value) != 0)
    return -1;
  if ((l_value < INT_MIN) || (l_value > INT_MAX))
    return -1;
  *value= l_value;
  return 0;
}

// -----[ str_as_ulong ]---------------------------------------------
/**
 * Convert a string to an unsigned long (0 <= X <= ULONG_MAX).
 * 
 * The function uses 'strtoll()' in order to convert the string to a
 * 'long long int'. We cannot use 'strtol()' since it is limited to a
 * maximum value of LONG_MAX while we need ULONG_MAX. We cannot use
 * 'strtoul()' since it will convert a negative number to a positive
 * number.
 *
 * Warning: on 64-bits platforms, it is possible that 'long' and
 *          'long long' have the same size. In this case, it is also
 *          impossible to rely on 'strtoll()' to perform the
 *          conversion. Indeed, in this case LLONG_MAX < ULONG_MAX.
 */
int str_as_ulong(const char * s, unsigned long int * value)
{
#ifdef sizeof(long long) > sizeof(long)
  long long int ll_value= 0;
#else
  unsigned long ul_value= 0;
#endif
  char * endptr;

  if (value == NULL)
    return -1;
  errno= 0;

  // If the size of a 'long long' is larger than that of a 'long'
  // then we can rely on 'strtoll()'. Otherwise, we need to manually
  // check if the input string does not contain a leading minus ('-')
#ifdef sizeof(long long) > sizeof(long)
    ll_value= strtoll(s, &endptr, 0);
#else
    // spec for 'strtoul()' says that it will accepts spaces
    // followed by a '+' or '-' character. We manually skip the
    // spaces, then fail if we find a '-' character.
    while (isspace(*s)) s++;
    if (*s == '-')
      return -1;
    ul_value= strtoul(s, &endptr, 0);
#endif

  // If no conversion could be performed, 0 is returned and errno <- EINVAL
  if (errno == EINVAL)
    return -1;
  // If under/over-flow occurs, errno <- ERANGE
  if (errno == ERANGE)
    return -1;

#ifdef sizeof(long long) > sizeof(long)
  // Check for bounds (0 <= x <= ULONG_MAX)
  if ((ll_value < 0) || (ll_value > ULONG_MAX))
    return -1;
  *value= (unsigned long int) ll_value;
#else
  *value= (unsigned long int) ul_value;
#endif

  // We do full string conversions. There is an error if the char
  // pointed by endptr is not '\0'
  return (*endptr == 0)?0:-1;
}

// -----[ str_as_uint ]----------------------------------------------
/**
 * Convert a string to an unsigned int (0 < X <= UINT_MAX).
 */
int str_as_uint(const char * s, unsigned int * value)
{
  unsigned long int l_value;

  if (value == NULL)
    return -1;
  if (str_as_ulong(s, &l_value) != 0)
    return -1;
  if (l_value > UINT_MAX)
    return -1;
  *value= l_value;
  return 0;
}

// -----[ str_as_double ]--------------------------------------------
/**
 *
 */
int str_as_double(const char * s, double * value)
{
  double d_value;
  char * endptr;

  if (value == NULL)
    return -1;
  errno= 0;
  d_value= strtod(s, &endptr);
  if (errno == ERANGE)
    return -1;
  *value= d_value;
  // We do full string conversions. There is an error if the char
  // pointed by endptr is not '\0'
  return (*endptr == 0)?0:-1;
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
    buf->data= str_lextend(buf->data, buf->size);
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
    buf->data= str_lextend(buf->data, buf->size);
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
