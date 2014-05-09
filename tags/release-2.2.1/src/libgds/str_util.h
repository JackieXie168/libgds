// =================================================================
// @(#)str_util.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 24/07/2003
// $Id$
// =================================================================

/**
 * \file
 * Provide a set of functions to manipulate and convert
 * NUL-terminated strings.
 */

#ifndef __GDS_STR_UTIL_H__
#define __GDS_STR_UTIL_H__

#include <string.h>

typedef struct str_buf_t {
  char         * data;      // Buffer
  size_t         size;      // Current buffer size
  unsigned int   index;     // Current position in buffer
  size_t         min_size;  // Initial size (and increment)
  int            invisible_data;
} str_buf_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef strsep
  // -----[ strsep ]--------------------------------------------------
  /**
   * This is a replacement for \c strsep() in case it is not provided
   * by the system. See strsep's man page for more information.
   */
  char * strsep(char ** str, const char * delim);
#endif

  // -----[ str_lcreate ]--------------------------------------------
  /**
   * Create a string of N characters.
   *
   * Allocates memory for N characters + 1 character for the
   * terminating NUL-character.
   *
   * \param len is the number of characters (len+1 will be allocated).
   */
  char * str_lcreate(size_t len);

  // -----[ str_create ]---------------------------------------------
  /**
   * Create a copy of a string.
   *
   * \param s is the string to be copied.
   */
  char * str_create(const char * s);

  // -----[ str_ncreate ]--------------------------------------------
  /**
   * Create a copy of the N first characters of a string.
   *
   * Allocates memory for N charecters + 1 character for the
   * terminating NUL-character. If the length of the source string
   * is less than N, N+1 characters are also allocated.
   *
   * \param s   is the string to be copied.
   * \param len is the number of characters to be copied.
   */
  char * str_ncreate(const char * s, size_t len);

  // -----[ str_lextend ]--------------------------------------------
  /**
   * Change the length of a string.
   *
   * \param s       is the string to update.
   * \param new_len is the new string length.
   * \retval the new string (might be different from the initial
   *   pointer).
   */
  char * str_lextend(char * s, size_t new_len);

  // -----[ str_destroy ]--------------------------------------------
  /**
   * Destroy a string.
   *
   * \param s_ref is a pointer to the string to be destroyed.
   */
  void str_destroy(char ** s_ref);

  // -----[ str_append ]---------------------------------------------
  char * str_append(char * s, const char * append);
  // -----[ str_nappend ]--------------------------------------------
  char * str_nappend(char * s, const char * append, size_t len);
  // -----[ str_prepend ]--------------------------------------------
  char * str_prepend(char * s, const char * prepend);
  // -----[ str_translate ]------------------------------------------
  void str_translate(char * s, const char * src_chars,
		     const char * dst_chars);


  ///////////////////////////////////////////////////////////////////
  // STRING CONVERSION FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // -----[ str_as_long ]--------------------------------------------
  int str_as_long(const char * s, long int * value);
  // -----[ str_as_int ]---------------------------------------------
  int str_as_int(const char * s, int * value);
  // -----[ str_as_ulong ]-------------------------------------------
  int str_as_ulong(const char * s, unsigned long int * value);
  // -----[ str_as_uint ]--------------------------------------------
  int str_as_uint(const char * s, unsigned int * value);
  // -----[ str_as_double ]------------------------------------------
  int str_as_double(const char * s, double * value);


  ///////////////////////////////////////////////////////////////////
  // STRING BUFFER
  ///////////////////////////////////////////////////////////////////

  // -----[ str_buf_create ]-----------------------------------------
  str_buf_t * str_buf_create(size_t min_size);
  // -----[ str_buf_destroy ]----------------------------------------
  void str_buf_destroy(str_buf_t ** buf);
  // -----[ str_buf_reset ]------------------------------------------
  void str_buf_reset(str_buf_t * buf);
  // -----[ str_buf_write_char ]-------------------------------------
  void str_buf_write_char(str_buf_t * buf, char c);
  // -----[ str_buf_write_string ]-----------------------------------
  void str_buf_write_string(str_buf_t * buf, const char * str);
  // -----[ str_buf_write_invisible ]--------------------------------
  void str_buf_write_invisible(str_buf_t * buf);
  // -----[ str_buf_empty ]------------------------------------------
  int str_buf_empty(str_buf_t * buf);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_STR_UTIL_H__ */
