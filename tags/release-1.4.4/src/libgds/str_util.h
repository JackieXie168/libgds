// =================================================================
// @(#)str_util.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 24/07/2003
// @lastdate 04/12/2007
// =================================================================

#ifndef __GDS_STR_UTIL_H__
#define __GDS_STR_UTIL_H__

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ strsep ]--------------------------------------------------
#ifndef strsep
  char * strsep(char ** ppcStr, const char * pcDelim);
#endif

  // ----- str_lcreate ----------------------------------------------
  char * str_lcreate(size_t tLen);
  // ----- str_create -----------------------------------------------
  char * str_create(const char * pcString);
  // ----- str_ncreate ----------------------------------------------
  char * str_ncreate(const char * pcString, size_t tLen);
  // ----- str_lextend ----------------------------------------------
  char * str_lextend(char ** ppcString, size_t tNewLen);
  // ----- str_destroy ----------------------------------------------
  void str_destroy(char ** ppcString);
  // ----- str_append -----------------------------------------------
  char * str_append(char ** ppcString,
		    const char * pcStrToAppend);
  // ----- str_nappend ----------------------------------------------
  char * str_nappend(char ** ppcString,
		     const char * pcStrToAppend,
		     size_t tLen);
  // ----- str_prepend ----------------------------------------------
  char * str_prepend(char ** ppcString, const char * pcStrToPrepend);
  // ----- str_translate --------------------------------------------
  void str_translate(char * pcString, const char * pcSrcChars,
		     const char * pcDstChars);


  ///////////////////////////////////////////////////////////////////
  // STRING CONVERSION FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // ----- str_as_long ----------------------------------------------
  int str_as_long(const char * pcString, long int * plValue);
  // ----- str_as_int -----------------------------------------------
  int str_as_int(const char * pcString, int * piValue);
  // ----- str_as_ulong ---------------------------------------------
  int str_as_ulong(const char * pcString, unsigned long int * pulValue);
  // ----- str_as_uint ----------------------------------------------
  int str_as_uint(const char * pcString, unsigned int * puValue);
  // ----- str_as_double --------------------------------------------
  int str_as_double(const char * pcString, double * pdValue);

#ifdef __cplusplus
}
#endif

#endif
