// ==================================================================
// @(#)regex.h
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 29/09/2004
// @lastdate 16/11/2004
// ==================================================================

#ifndef __GDS_REGEX_H__
#define __GDS_REGEX_H__

#ifdef PCRE_PATH_BASE
#include <pcre.h>
#else
#include <pcre/pcre.h>
#endif

typedef struct {
  pcre * pRegEx;
  unsigned int uMaxResult;
  int * iVectorResult;
  int iNbrResult;
} SRegEx;


// ----- regex_int ---------------------------------------------------
SRegEx * regex_init(char * pattern, const unsigned int uMaxResult);
// ----- regex_search ------------------------------------------------
int regex_search(SRegEx * pRegEx, char * sExp);
// ----- regex_get_result --------------------------------------------
char * regex_get_result(SRegEx * pRegEx, char * sExp, 
			  unsigned int iNumRes);
// ----- regex_finalize ----------------------------------------------
void regex_finalize(SRegEx * pRegEx);

#endif 
