// =================================================================
// @(#)tokenizer.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/07/2003
// @lastdate 04/03/2004
// =================================================================

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <stdio.h>
#include <libgds/tokens.h>

#define TOKENIZER_SUCCESS           0
#define TOKENIZER_ERROR_UNEXPECTED -1
#define TOKENIZER_ERROR_OPEN_BLOCK -2
#define TOKENIZER_ERROR_ESCAPE     -3

typedef struct {
  STokens * pTokens;
  char * pcDelimiters;    /* A string of field delimiters */
  int iSingleDelimiter;   /* Fields are separated by a single delimiter */
  char * pcOpeningQuotes; /* A string of opening quotes */
  char * pcClosingQuotes; /* A string of closing quotes (the ordering of
			     quotes must match the ordering of opening
			     quotes) */
} STokenizer;

// ----- tokenizer_create -------------------------------------------
extern STokenizer * tokenizer_create(const char * pcDelimiters,
				     int iSingleDelimiter,
				     const char * pcOpeningQuotes,
				     const char * pcClosingQuotes);
// ----- tokenizer_destroy ------------------------------------------
extern void tokenizer_destroy(STokenizer ** ppTokenizer);
// ----- tokenizer_get_tokens ---------------------------------------
extern STokens * tokenizer_get_tokens(STokenizer * pTokenizer);
// ----- tokenizer_run ----------------------------------------------
extern int tokenizer_run(STokenizer * pTokenizer, char * pcString);
// ----- tokenizer_perror -------------------------------------------
extern void tokenizer_perror(FILE * pStream, int iErrorCode);
// ----- tokenizer_get_num_token ------------------------------------
extern uint16_t tokenizer_get_num_tokens(STokenizer * pTokenizer);

#endif
