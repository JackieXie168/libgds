// =================================================================
// @(#)str_util.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 24/07/2003
// $Id$
// =================================================================

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

  // -----[ strsep ]--------------------------------------------------
#ifndef strsep
  char * strsep(char ** ppcStr, const char * pcDelim);
#endif

  // ----- str_lcreate ----------------------------------------------
  char * str_lcreate(size_t len);
  // ----- str_create -----------------------------------------------
  char * str_create(const char * s);
  // ----- str_ncreate ----------------------------------------------
  char * str_ncreate(const char * s, size_t len);
  // ----- str_lextend ----------------------------------------------
  char * str_lextend(char ** s, size_t new_len);
  // ----- str_destroy ----------------------------------------------
  void str_destroy(char ** s);
  // ----- str_append -----------------------------------------------
  char * str_append(char ** s,
		    const char * append);
  // ----- str_nappend ----------------------------------------------
  char * str_nappend(char ** s, const char * append, size_t len);
  // ----- str_prepend ----------------------------------------------
  char * str_prepend(char ** s, const char * prepend);
  // ----- str_translate --------------------------------------------
  void str_translate(char * s, const char * src_chars,
		     const char * dst_chars);


  ///////////////////////////////////////////////////////////////////
  // STRING CONVERSION FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // ----- str_as_long ----------------------------------------------
  int str_as_long(const char * s, long int * value);
  // ----- str_as_int -----------------------------------------------
  int str_as_int(const char * s, int * value);
  // ----- str_as_ulong ---------------------------------------------
  int str_as_ulong(const char * s, unsigned long int * value);
  // ----- str_as_uint ----------------------------------------------
  int str_as_uint(const char * s, unsigned int * value);
  // ----- str_as_double --------------------------------------------
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
