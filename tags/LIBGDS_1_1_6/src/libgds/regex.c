// ==================================================================
// @(#)regex.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 29/09/2004
// @lastdate 16/11/2004
// ==================================================================

/* 
 * Simple regex wrapper based on the pcre library. It's mainly 
 * conceived for very simple matching of regexp.
 */

#include <stdio.h>
#include <string.h>

#include <libgds/memory.h>
#include <libgds/regex.h>
#include <libgds/log.h>
//#include <regex.h>

//#define LOG_WARNING   printf
//#define MALLOC	      malloc
//#define FREE	      free

// ----- regex_compile -----------------------------------------------
/**
 *
 *
 */
SRegEx * regex_init(char * pattern, const unsigned int uMaxResult)
{
  SRegEx * pRegEx = MALLOC(sizeof(SRegEx));

  pcre *	RegEx;
  const char *	sError;
  int		iErrorOffset;
  
  RegEx = pcre_compile(pattern, 0, 
			  &sError, &iErrorOffset,
			  NULL);
  if (pRegEx == NULL) {
    LOG_WARNING("regex_compile> PCRE compilation failed at offset"
		  "%d : %s\n", iErrorOffset, sError);
    FREE(pRegEx);
    pRegEx = NULL;
  } else {
    pRegEx->pRegEx = RegEx;
    pRegEx->uMaxResult = uMaxResult;
    pRegEx->iVectorResult = NULL;
    pRegEx->iNbrResult = 0;
  }
  return pRegEx;
}

// ----- regex_exec --------------------------------------------------
/**
 * returns the number of matching in the expression
 *
 */
int regex_search(SRegEx * pRegEx, char * sExp)
{
  if (pRegEx != NULL) {

    if (pRegEx->iVectorResult)
      FREE(pRegEx->iVectorResult);

    pRegEx->iVectorResult = MALLOC( (sizeof(int)*(pRegEx->uMaxResult+1)) * 3);

    pRegEx->iNbrResult = pcre_exec(pRegEx->pRegEx, NULL, sExp, strlen(sExp), 0, 0,
				  pRegEx->iVectorResult, 
				  (pRegEx->uMaxResult+1)*3);
    if (pRegEx->iNbrResult < 0) {
      switch (pRegEx->iNbrResult) {
        //No match of the pRegEx in sExp
        case PCRE_ERROR_NOMATCH:
	  pRegEx->iNbrResult = 0;
	 break;
      /* PCRE_ERROR_NULL
       * PCRE_ERROR_BADOPTION
       * PCRE_ERROR_BADMAGIC
       * PCRE_ERROR_UNKNOWN_NODE 
       * PCRE_ERROR_NOMEMORY
       * PCRE_ERROR_NOSUBSTRING
       * PCRE_ERROR_MATCHLIMIT
       * PCRE_ERROR_CALLOUT
       * PCRE_ERROR_BADUTF8
       * PCRE_ERROR_BADUTF8_OFFSET
       * PCRE_ERROR_PARTIAL
       * PCRE_ERROR_BAD_PARTIAL
       * PCRE_ERROR_INTERNAL
       * PCRE_ERROR_BADCOUNT
       */
        default: 
	 LOG_WARNING("regex_exec>Matching error %d\n", pRegEx->iNbrResult);
      }
      FREE(pRegEx->iVectorResult);
      pRegEx->iVectorResult = NULL;
    }
  }
  return pRegEx->iNbrResult;
}

// ----- regex_get_result --------------------------------------------
/**
 *
 *
 */
char * regex_get_result(SRegEx * pRegEx, char * sExp, 
			  unsigned int iNumRes)
{
  char * sMatchedString;
  int iMatchedStringLen;
  
  if (pRegEx == NULL || iNumRes > pRegEx->iNbrResult || iNumRes <= 0)
    return NULL;

  iNumRes--;

  iMatchedStringLen = pRegEx->iVectorResult[iNumRes*2+1] - pRegEx->iVectorResult[iNumRes*2];

  sMatchedString = MALLOC(iMatchedStringLen+1);
  strncpy(sMatchedString, sExp, iMatchedStringLen);
  sMatchedString[iMatchedStringLen] = '\0';

  return sMatchedString;
}

// ----- regex_finalize ----------------------------------------------
/**
 *
 *
 */
void regex_finalize(SRegEx * pRegEx)
{
  if (pRegEx != NULL) {
    if (pRegEx->pRegEx != NULL)
      FREE(pRegEx->pRegEx);
    if (pRegEx->iVectorResult != NULL)
      FREE(pRegEx->iVectorResult);
    FREE(pRegEx);
  }
}

/* just a simple example of use
int main (int args, char ** argv)
{
  char * pattern, * exp, *substring;
  unsigned int iResult; 
  SRegEx * pRegEx;

  pattern = argv[1];
  exp = argv[2];

  printf("pattern %s\n", pattern);
  printf("string : %s\n", exp);

  pRegEx = regex_init(pattern, 30);

  if (pRegEx != NULL) {
    if ( (iResult = regex_search(pRegEx, exp)) > 0) {
      printf("matched %d times\n", iResult);
      while (iResult > 0) {
	substring = regex_get_result(pRegEx, exp, iResult);
	printf ("%s\n", substring);
	FREE(substring);
	iResult--;
      }
    } else
      printf("no matching\n");
    regex_finalize(pRegEx);
  }
  return 0;
}
*/
