// =================================================================
// @(#)tokenizer.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 10/07/2003
// @lastdate 20/08/2003
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
  char * pcDelimiters;
  char * pcOpeningQuotes;
  char * pcClosingQuotes;
} STokenizer;

// ----- tokenizer_create -------------------------------------------
extern STokenizer * tokenizer_create(const char * pcDelimiters,
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
