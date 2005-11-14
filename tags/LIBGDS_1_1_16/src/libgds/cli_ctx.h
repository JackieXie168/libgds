// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 25/06/2003
// @lastdate 25/07/2003
// ==================================================================

#ifndef __GDS_CLI_CTX_H__
#define __GDS_CLI_CTX_H__

#include <libgds/cli.h>

// ----- cli_context_create -----------------------------------------
extern SCliContext * cli_context_create();
// ----- cli_context_create -----------------------------------------
extern void cli_context_destroy(SCliContext ** ppContext);
// ----- cli_context_depth ------------------------------------------
extern int cli_context_depth(SCliContext * pContext);
// ----- cli_context_is_empty ---------------------------------------
extern int cli_context_is_empty(SCliContext * pContext);
// ----- cli_context_push -------------------------------------------
extern int cli_context_push(SCliContext * pContext, void * pItem,
			    SCliCmd * pCmd);
// ----- cli_context_pop --------------------------------------------
extern void cli_context_pop(SCliContext * pContext);
// ----- cli_context_top ---------------------------------------------
extern SCliCtxItem * cli_context_top(SCliContext * pContext);
// ----- cli_context_get --------------------------------------------
extern void * cli_context_get(SCliContext * pContext);
// ----- cli_context_set --------------------------------------------
extern void cli_context_set(SCliContext * pContext, void * pCtx);
// ----- cli_context_get_at -----------------------------------------
extern SCliCtxItem * cli_context_get_at(SCliContext * pContext,
					uint32_t uIndex);
// ----- cli_context_get_item_at ------------------------------------
extern void * cli_context_get_item_at(SCliContext * pContext,
				      uint32_t uIndex);
// ----- cli_context_get_item_from_top ------------------------------
extern void * cli_context_get_item_from_top(SCliContext * pContext,
					    int uOffset);
// ----- cli_context_get_item_at_top --------------------------------
extern void * cli_context_get_item_at_top(SCliContext * pContext);
// ----- cli_context_add_token_copy ---------------------------------
extern int cli_context_add_token_copy(SCliContext * pContext,
				      char * pcToken);
// ----- cli_context_clear ------------------------------------------
extern void cli_context_clear(SCliContext * pContext);
// ----- cli_context_backtrack --------------------------------------
extern void cli_context_backtrack(SCliContext * pContext);
// ----- cli_context_save_depth -------------------------------------
extern void cli_context_save_depth(SCliContext * pContext);
// ----- cli_context_restore_depth ----------------------------------
extern void cli_context_restore_depth(SCliContext * pContext);
// ----- cli_context_to_string --------------------------------------
extern char * cli_context_to_string(SCliContext * pContext,
				    char * pcPrefix);

#endif
