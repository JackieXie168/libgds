// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 29/07/2003
// ==================================================================

#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>

#define CLI_MAX_CONTEXT 16

// ----- cli_context_item_create ------------------------------------
SCliCtxItem * cli_context_item_create(void * pItem,
				      SCliCmd * pCmd,
				      unsigned int uNumParams)
{
  SCliCtxItem * pCtxItem=
    (SCliCtxItem *) MALLOC(sizeof(SCliCtxItem));
  pCtxItem->pItem= pItem;
  pCtxItem->pCmd= pCmd;
  pCtxItem->uNumParams= uNumParams;
  return pCtxItem;
}

// ----- cli_context_item_destroy -----------------------------------
void cli_context_item_destroy(SCliCtxItem ** ppCtxItem)
{
  if (*ppCtxItem != NULL) {
    if ((*ppCtxItem)->pCmd->fCtxDestroy != NULL)
      (*ppCtxItem)->pCmd->fCtxDestroy(&(*ppCtxItem)->pItem);
    FREE(*ppCtxItem);
    *ppCtxItem= NULL;
  }
}

// ----- cli_context_create -----------------------------------------
/**
 *
 */
SCliContext * cli_context_create()
{
  SCliContext * pContext=
    (SCliContext *) MALLOC(sizeof(SCliContext));
  pContext->pStack= stack_create(CLI_MAX_CONTEXT);
  pContext->pTokens= tokens_create();
  pContext->uSavedDepth= 0;
  pContext->pContext= NULL;
  pContext->pString= NULL;
  return pContext;
}

// ----- cli_context_create -----------------------------------------
/**
 *
 */
void cli_context_destroy(SCliContext ** ppContext)
{
  int iIndex;

  if (*ppContext != NULL) {
    for (iIndex= 0; iIndex < (*ppContext)->pStack->iDepth; iIndex++) {
      SCliCtxItem * pItem=
	(SCliCtxItem *) stack_get_at((*ppContext)->pStack, iIndex);
      cli_context_item_destroy(&pItem);
    }
    stack_destroy(&(*ppContext)->pStack);
    tokens_destroy(&(*ppContext)->pTokens);
    str_destroy(&(*ppContext)->pString);
    FREE(*ppContext);
    *ppContext= NULL;
  }
}

// ----- cli_context_depth ------------------------------------------
/**
 *
 */
int cli_context_depth(SCliContext * pContext)
{
  return stack_depth(pContext->pStack);
}

// ----- cli_context_is_empty ---------------------------------------
/**
 *
 */
int cli_context_is_empty(SCliContext * pContext)
{
  return stack_is_empty(pContext->pStack);
}

// ----- cli_context_push -------------------------------------------
/**
 *
 */
int cli_context_push(SCliContext * pContext, void * pItem,
		     SCliCmd * pCmd)
{
  SCliCtxItem * pCtxItem=
    cli_context_item_create(pItem, pCmd,
			    tokens_get_num(pContext->pTokens));
  return stack_push(pContext->pStack, pCtxItem);
}

// ----- cli_context_pop --------------------------------------------
/**
 *
 */
void cli_context_pop(SCliContext * pContext)
{
  SCliCtxItem * pCtxItem= (SCliCtxItem *) stack_pop(pContext->pStack);
  cli_context_item_destroy(&pCtxItem);
}

// ----- cli_context_top ---------------------------------------------
/**
 *
 */
SCliCtxItem * cli_context_top(SCliContext * pContext)
{
  return (SCliCtxItem *) stack_top(pContext->pStack);
}

// ----- cli_context_get --------------------------------------------
/**
 *
 */
void * cli_context_get(SCliContext * pContext)
{
  return pContext->pContext;
}

// ----- cli_context_set --------------------------------------------
/**
 *
 */
void cli_context_set(SCliContext * pContext, void * pCtx)
{
  pContext->pContext= pCtx;
}

// ----- cli_context_get_at ------------------------------------------
/**
 *
 */
SCliCtxItem * cli_context_get_at(SCliContext * pContext, uint32_t uIndex)
{
  return (SCliCtxItem *) stack_get_at(pContext->pStack, uIndex);
}

// ----- cli_context_get_item_at ------------------------------------
/**
 *
 */
void * cli_context_get_item_at(SCliContext * pContext, uint32_t uIndex)
{
  SCliCtxItem * pCtxItem=
    (SCliCtxItem *) stack_get_at(pContext->pStack, uIndex);
  if (pCtxItem != NULL)
    return pCtxItem->pItem;
  return NULL;
}

// ----- cli_context_get_item_from_top ------------------------------
/**
 *
 */
void * cli_context_get_item_from_top(SCliContext * pContext,
				     int uOffset)
{
  if (!cli_context_is_empty(pContext))
    return cli_context_get_item_at(pContext,
				   cli_context_depth(pContext)-uOffset-1);
  return NULL;
}


// ----- cli_context_get_item_at_top --------------------------------
/**
 *
 */
void * cli_context_get_item_at_top(SCliContext * pContext)
{
  if (!cli_context_is_empty(pContext))
    return cli_context_get_item_at(pContext, cli_context_depth(pContext)-1);
  return NULL;
}

// ----- cli_context_add_token_copy ---------------------------------
/**
 *
 */
int cli_context_add_token_copy(SCliContext * pContext, char * pcToken)
{
  return tokens_add_copy(pContext->pTokens, pcToken);
}

// ----- cli_context_clear ------------------------------------------
/**
 * Clear context-stack and params
 */
void cli_context_clear(SCliContext * pContext)
{
  while (cli_context_depth(pContext) > 0)
    cli_context_pop(pContext);
  tokens_destroy(&pContext->pTokens);
  pContext->pTokens= tokens_create();
  pContext->uSavedDepth= 0;
}

// ----- cli_context_backtrack --------------------------------------
/**
 * Backtrack in context-stack and clear useless parameters.
 */
void cli_context_backtrack(SCliContext * pContext)
{
  cli_context_pop(pContext);
  if (!cli_context_is_empty(pContext)) {
  }
}

// ----- cli_context_save_depth -------------------------------------
/**
 *
 */
void cli_context_save_depth(SCliContext * pContext)
{
  pContext->uSavedDepth= stack_depth(pContext->pStack);
}

// ----- cli_context_restore_depth ----------------------------------
/**
 *
 */
void cli_context_restore_depth(SCliContext * pContext)
{
  while (cli_context_depth(pContext) > pContext->uSavedDepth)
    cli_context_pop(pContext);
}

// ----- cli_context_to_string --------------------------------------
/**
 * This function creates a string with all the context's elements in
 * order. This string can be used as a prompt in a user-interface or
 * error messages.
 */
char * cli_context_to_string(SCliContext * pContext, char * pcPrefix)
{
  int iIndex;//, iTokenOffset, iParamIndex;
  SCliCtxItem * pCtxItem;

  // Free previous context-string
  str_destroy(&pContext->pString);

  // Build new string
  pContext->pString= str_create(pcPrefix);

  //iTokenOffset= 0;
  // Add context commands
  for (iIndex= 0; iIndex < cli_context_depth(pContext); iIndex++) {
    pCtxItem= cli_context_get_at(pContext, iIndex);
    if ((pCtxItem != NULL) && (pCtxItem->pCmd != NULL)) {
      str_append(&pContext->pString, "-");
      str_append(&pContext->pString, pCtxItem->pCmd->pcName);

      // Add token values
      /*
      for (iParamIndex= 0;
	   iParamIndex < cli_cmd_get_num_params(pCtxItem->pCmd);
	   iParamIndex++) {
	str_append(&pContext->pString, " ");
	str_append(&pContext->pString,
		   tokens_get_string_at(pContext->pTokens, iTokenOffset));
	iTokenOffset++;
      }
      */

    }
  }
  str_append(&pContext->pString, "> ");

  return pContext->pString;
}
