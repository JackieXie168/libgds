// =================================================================
// @(#)tokens.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 20/08/2003
// @lastdate 20/08/2003
// =================================================================

#ifndef __GDS_TOKENS_H__
#define __GDS_TOKENS_H__

#include <libgds/array.h>
#include <libgds/types.h>

typedef SPtrArray STokens;

// ----- tokens_create ----------------------------------------------
extern STokens * tokens_create();
// ----- tokens_destroy ---------------------------------------------
extern void tokens_destroy(STokens ** pTokens);
// ----- tokens_add -------------------------------------------------
extern int tokens_add(STokens * pTokens, char * pcToken);
// ----- tokens_add_copy --------------------------------------------
extern int tokens_add_copy(STokens * pTokens, char * pcToken);
// ----- tokens_get_num ---------------------------------------------
extern unsigned int tokens_get_num(STokens * pTokens);
// ----- tokens_get_at ----------------------------------------------
extern char * tokens_get_string_at(STokens * pTokens,
				   unsigned int uIndex);
// ----- tokens_get_long_at -----------------------------------------
extern int tokens_get_long_at(STokens * pTokens, uint16_t uIndex,
			      long int * plValue);
// ----- tokens_get_int_at ------------------------------------------
extern int tokens_get_int_at(STokens * pTokens, uint16_t uIndex,
			     int * piValue);
// ----- tokens_get_ulong_at ----------------------------------------
extern int tokens_get_ulong_at(STokens * pTokens, uint16_t uIndex,
			       unsigned long int * plValue);
// ----- tokens_get_uint_at -----------------------------------------
extern int tokens_get_uint_at(STokens * pTokens, uint16_t uIndex,
			      unsigned int * puValue);
// ----- tokens_get_double_at ---------------------------------------
extern int tokens_get_double_at(STokens * pTokens,
				uint16_t uIndex,
				double * pdValue);

#endif
