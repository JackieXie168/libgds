// =================================================================
// @(#)tokenizer.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/07/2003
// @lastdate 10/01/2005
// =================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <libgds/memory.h>
#include <string.h>
#include <libgds/str_util.h>
#include <libgds/tokenizer.h>

#define TOKENIZER_BUFFER_SIZE 1024

#define TOKENIZER_STATE_NORMAL 0
#define TOKENIZER_STATE_DELIM  1
#define TOKENIZER_STATE_QUOTED 2

// ----- tokenizer_create -------------------------------------------
/**
 *
 */
STokenizer * tokenizer_create(const char * pcDelimiters,
			      int iSingleDelimiter,
			      const char * pcOpeningQuotes,
			      const char * pcClosingQuotes)
{
  STokenizer * pTokenizer=
    (STokenizer *) MALLOC(sizeof(STokenizer));
  pTokenizer->pTokens= NULL;
  pTokenizer->pcDelimiters= str_create(pcDelimiters);
  pTokenizer->iSingleDelimiter= iSingleDelimiter;
  if ((pcOpeningQuotes != NULL) && (pcClosingQuotes != NULL)) {
    assert(strlen(pcOpeningQuotes) == strlen(pcClosingQuotes));
    pTokenizer->pcOpeningQuotes= str_create(pcOpeningQuotes);
    pTokenizer->pcClosingQuotes= str_create(pcClosingQuotes);
  } else {
    pTokenizer->pcOpeningQuotes= str_create("");
    pTokenizer->pcClosingQuotes= str_create("");
  }
  return pTokenizer;
}

// ----- tokenizer_destroy ------------------------------------------
/**
 *
 */
void tokenizer_destroy(STokenizer ** ppTokenizer)
{
  if (*ppTokenizer != NULL) {
    str_destroy(&(*ppTokenizer)->pcDelimiters);
    str_destroy(&(*ppTokenizer)->pcOpeningQuotes);
    str_destroy(&(*ppTokenizer)->pcClosingQuotes);
    tokens_destroy(&(*ppTokenizer)->pTokens);
    FREE(*ppTokenizer);
    *ppTokenizer= NULL;
  }
}

// ----- tokenizer_get_tokens ---------------------------------------
/**
 *
 */
STokens * tokenizer_get_tokens(STokenizer * pTokenizer)
{
  return pTokenizer->pTokens;
}

// ----- tokenizer_run ----------------------------------------------
/**
 *
 */
int tokenizer_run(STokenizer * pTokenizer, char * pcString)
{
  int iResult= TOKENIZER_ERROR_UNEXPECTED;
  char cClosingQuote= '\0';
  int iState= TOKENIZER_STATE_NORMAL;
  char cCtrlChar, cDataChar;
  char * pcTokenBuffer= NULL;
  unsigned int uTokenBufferPos= 0;
  size_t tTokenBufferSize= TOKENIZER_BUFFER_SIZE;
  int iTokenReady= 0;
  char * pcQuote;

  // Free old list of tokens
  if (pTokenizer->pTokens != NULL)
    tokens_destroy(&pTokenizer->pTokens);
  // Allocate new one
  pTokenizer->pTokens= tokens_create();

  // Allocate token buffer
  pcTokenBuffer= str_lcreate(tTokenBufferSize);

  // Finite State Machine
  while (iResult == TOKENIZER_ERROR_UNEXPECTED) {
    // Eat one char
    cCtrlChar= *(pcString++);
    // Process escaped-characters
    if (cCtrlChar == '\\') {
      if (*pcString == '\0') {
	iResult= TOKENIZER_ERROR_ESCAPE;
	break;
      }
      switch (*pcString) {
      case 'a': cDataChar= '\a'; break;
      case 'e': cDataChar= '\033'; break;
      case 'r': cDataChar= '\r'; break;
      case 't': cDataChar= '\t'; break;
      case 'n': cDataChar= '\n'; break;
      default:
	cDataChar= *pcString;
      }
      pcString++;
    } else
      cDataChar= cCtrlChar;
    // Check buffer size
    if (uTokenBufferPos >= tTokenBufferSize) {
      tTokenBufferSize+= TOKENIZER_BUFFER_SIZE;
      str_lextend(&pcTokenBuffer, tTokenBufferSize);
    }
    // State-dependent actions
    switch (iState) {
    case TOKENIZER_STATE_NORMAL:
      if ((cCtrlChar == '\0') || (cCtrlChar == '\n')) {
	iResult= TOKENIZER_SUCCESS;
	if (iTokenReady) {
	  pcTokenBuffer[uTokenBufferPos]= '\0';
	  tokens_add_copy(pTokenizer->pTokens, pcTokenBuffer);
	  uTokenBufferPos= 0;
	  pcTokenBuffer[0]= '\0';
	  iTokenReady= 0;
	}
      } else if (strchr(pTokenizer->pcDelimiters, cCtrlChar) != NULL) {
	iState= TOKENIZER_STATE_DELIM;
	if (iTokenReady) {
	  pcTokenBuffer[uTokenBufferPos]= '\0';
	  tokens_add_copy(pTokenizer->pTokens, pcTokenBuffer);
	  uTokenBufferPos= 0;
	  pcTokenBuffer[0]= '\0';
	  iTokenReady= 0;
	}
      } else if ((pcQuote= strchr(pTokenizer->pcOpeningQuotes, cCtrlChar))
		 != NULL) {
	iState= TOKENIZER_STATE_QUOTED;
	cClosingQuote= *(pTokenizer->pcClosingQuotes+
			 (pcQuote-pTokenizer->pcOpeningQuotes));
      } else {
	// Store char in token-buffer
	pcTokenBuffer[uTokenBufferPos++]= cDataChar;
	iTokenReady= 1;
      }
      break;
    case TOKENIZER_STATE_DELIM:
      if ((cCtrlChar == '\0') || (cCtrlChar == '\n')) {
	iResult= TOKENIZER_SUCCESS;
      } else if ((pcQuote= strchr(pTokenizer->pcOpeningQuotes, cCtrlChar))
		 != NULL) {
	iState= TOKENIZER_STATE_QUOTED;
	cClosingQuote= *(pTokenizer->pcClosingQuotes+
			 (pcQuote-pTokenizer->pcOpeningQuotes));
      } else if (strchr(pTokenizer->pcDelimiters, cCtrlChar) == NULL) {
	iState= TOKENIZER_STATE_NORMAL;
	pcTokenBuffer[uTokenBufferPos++]= cDataChar;
	iTokenReady= 1;
      } else {
	// If this option is activated, this means that each delimiter
	// separates two fields. If two consecutive delimiters are
	// found, then a dummy field must be created.
	if (pTokenizer->iSingleDelimiter) {
	  tokens_add_copy(pTokenizer->pTokens, "");
	}
      }
      break;
    case TOKENIZER_STATE_QUOTED:
      iTokenReady= 1;
      if (cCtrlChar == '\0') {
	iResult= TOKENIZER_ERROR_OPEN_BLOCK;
      } else if (cCtrlChar == cClosingQuote) {
	iState= TOKENIZER_STATE_NORMAL;
      } else {
	pcTokenBuffer[uTokenBufferPos++]= cDataChar;
      }
      break;
    }
  }
  if (iResult == TOKENIZER_SUCCESS) {
    if (iState == TOKENIZER_STATE_QUOTED)
      iResult= TOKENIZER_ERROR_OPEN_BLOCK;
  }
  str_destroy(&pcTokenBuffer);
  return iResult;
}

// ----- tokenizer_perror -------------------------------------------
/**
 *
 */
void tokenizer_perror(FILE * pStream, int iErrorCode)
{
  switch (iErrorCode) {
  case TOKENIZER_SUCCESS:
    fprintf(pStream, "success\n"); break;
  case TOKENIZER_ERROR_UNEXPECTED:
    fprintf(pStream, "unexpected error\n"); break;
  case TOKENIZER_ERROR_OPEN_BLOCK:
    fprintf(pStream, "open quoted block\n"); break;
  case TOKENIZER_ERROR_ESCAPE:
    fprintf(pStream, "incomplete escaped-character\n"); break;
  default:
    fprintf(pStream, "unknown error\n"); break;
  }
}

// ----- tokenizer_get_num_token ------------------------------------
/**
 *
 */
uint16_t tokenizer_get_num_tokens(STokenizer * pTokenizer)
{
  if (pTokenizer->pTokens != NULL)
    return tokens_get_num(pTokenizer->pTokens);
  else
    return 0;
}

