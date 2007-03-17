// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 16/01/2007
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>

#define CLI_MAX_CONTEXT 16

// -----[ _cli_context_item_create ]---------------------------------
static SCliCtxItem * _cli_context_item_create(SCliCmd * pCmd,
					      void * pUserData)
{
  SCliCtxItem * pCtxItem=
    (SCliCtxItem *) MALLOC(sizeof(SCliCtxItem));
  pCtxItem->pUserData= pUserData;
  pCtxItem->pCmd= pCmd;
  return pCtxItem;
}

// -----[ _cli_context_item_destroy ]--------------------------------
static void _cli_context_item_destroy(SCliCtxItem ** ppCtxItem)
{
  if (*ppCtxItem != NULL) {
    if ((*ppCtxItem)->pCmd->fCtxDestroy != NULL)
      (*ppCtxItem)->pCmd->fCtxDestroy(&(*ppCtxItem)->pUserData);
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
  pContext->pCmdStack= stack_create(CLI_MAX_CONTEXT);
  pContext->uSavedDepth= 0;
  pContext->pString= NULL;
  return pContext;
}

// ----- cli_context_destroy ----------------------------------------
/**
 *
 */
void cli_context_destroy(SCliContext ** ppContext)
{
  unsigned int uIndex;

  if (*ppContext != NULL) {
    for (uIndex= 0; uIndex < stack_depth((*ppContext)->pCmdStack); uIndex++) {
      SCliCtxItem * pItem=
	(SCliCtxItem *) stack_get_at((*ppContext)->pCmdStack, uIndex);
      _cli_context_item_destroy(&pItem);
    }
    stack_destroy(&(*ppContext)->pCmdStack);
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
  return stack_depth(pContext->pCmdStack);
}

// ----- cli_context_is_empty ---------------------------------------
/**
 *
 */
int cli_context_is_empty(SCliContext * pContext)
{
  return stack_is_empty(pContext->pCmdStack);
}

// ----- cli_context_push -------------------------------------------
/**
 * Push a command with parameters and context (user-data) on the
 * stack.
 */
void cli_context_push(SCliContext * pContext)
{
  SCliCtxItem * pItem= _cli_context_item_create(pContext->pCmd,
						pContext->pUserData);
  stack_push(pContext->pCmdStack, pItem);
}

// ----- cli_context_pop --------------------------------------------
/**
 *
 */
void cli_context_pop(SCliContext * pContext)
{
  SCliCtxItem * pItem;
  if (!stack_is_empty(pContext->pCmdStack)) {
    pItem= (SCliCtxItem *) stack_pop(pContext->pCmdStack);
    pContext->pCmd= pItem->pCmd;
    pContext->pUserData= pItem->pUserData;
    _cli_context_item_destroy(&pItem);
  }
}

// ----- cli_context_top ---------------------------------------------
/**
 *
 */
SCliCtxItem * cli_context_top(SCliContext * pContext)
{
  return (SCliCtxItem *) stack_top(pContext->pCmdStack);
}

// ----- cli_context_get --------------------------------------------
/**
 * Get current context (user-data).
 */
void * cli_context_get(SCliContext * pContext)
{
  return pContext->pUserData;
}

// ----- cli_context_get_at ------------------------------------------
/**
 *
 */
SCliCtxItem * cli_context_get_at(SCliContext * pContext, uint32_t uIndex)
{
  return (SCliCtxItem *) stack_get_at(pContext->pCmdStack, uIndex);
}

// ----- cli_context_get_item_at ------------------------------------
/**
 *
 */
void * cli_context_get_item_at(SCliContext * pContext, uint32_t uIndex)
{
  SCliCtxItem * pCtxItem=
    (SCliCtxItem *) stack_get_at(pContext->pCmdStack, uIndex);
  if (pCtxItem != NULL)
    return pCtxItem->pUserData;
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

// ----- cli_context_clear ------------------------------------------
/**
 * Clear context-stack and params
 */
void cli_context_clear(SCliContext * pContext)
{
  while (cli_context_depth(pContext) > 0)
    cli_context_pop(pContext);
  pContext->uSavedDepth= 0;
}

// ----- cli_context_save_depth -------------------------------------
/**
 *
 */
void cli_context_save_depth(SCliContext * pContext)
{
  pContext->uSavedDepth= stack_depth(pContext->pCmdStack);
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

