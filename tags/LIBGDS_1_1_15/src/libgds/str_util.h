// =================================================================
// @(#)str_util.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 24/07/2003
// @lastdate 24/07/2003
// =================================================================

#ifndef __GDS_STR_UTIL_H__
#define __GDS_STR_UTIL_H__

#include <string.h>

// ----- str_lcreate ------------------------------------------------
extern char * str_lcreate(size_t tLen);
// ----- str_create -------------------------------------------------
extern char * str_create(const char * pcString);
// ----- str_ncreate ------------------------------------------------
extern char * str_ncreate(const char * pcString, size_t tLen);
// ----- str_lextend ------------------------------------------------
extern char * str_lextend(char ** ppcString, size_t tNewLen);
// ----- str_destroy ------------------------------------------------
extern void str_destroy(char ** ppcString);
// ----- str_append -------------------------------------------------
extern char * str_append(char ** ppcString,
			 const char * pcStrToAppend);
// ----- str_nappend ------------------------------------------------
extern char * str_nappend(char ** ppcString,
			  const char * pcStrToAppend,
			  size_t tLen);

#endif
