// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 25/06/2003
// @lastdate 15/01/2007
// ==================================================================

#ifndef __GDS_CLI_CTX_H__
#define __GDS_CLI_CTX_H__

#include <libgds/cli.h>

#ifdef __cplusplus
extern "C" {
#endif

  // ----- cli_context_create ---------------------------------------
  SCliContext * cli_context_create();
  // ----- cli_context_create ---------------------------------------
  void cli_context_destroy(SCliContext ** ppContext);
  // ----- cli_context_depth ----------------------------------------
  int cli_context_depth(SCliContext * pContext);
  // ----- cli_context_is_empty -------------------------------------
  int cli_context_is_empty(SCliContext * pContext);
  // ----- cli_context_push -----------------------------------------
  void cli_context_push(SCliContext * pContext);
  // ----- cli_context_pop ------------------------------------------
  void cli_context_pop(SCliContext * pContext);
  // ----- cli_context_top ------------------------------------------
  SCliCtxItem * cli_context_top(SCliContext * pContext);
  // ----- cli_context_get ------------------------------------------
  void * cli_context_get(SCliContext * pContext);
  // ----- cli_context_set ------------------------------------------
  void cli_context_set(SCliContext * pContext, void * pCtx);
  // ----- cli_context_get_at ---------------------------------------
  SCliCtxItem * cli_context_get_at(SCliContext * pContext,
				   uint32_t uIndex);
  // ----- cli_context_get_item_at ----------------------------------
  void * cli_context_get_item_at(SCliContext * pContext,
				 uint32_t uIndex);
  // ----- cli_context_get_item_from_top ----------------------------
  void * cli_context_get_item_from_top(SCliContext * pContext,
				       int uOffset);
  // ----- cli_context_get_item_at_top ------------------------------
  void * cli_context_get_item_at_top(SCliContext * pContext);
  // ----- cli_context_clear ----------------------------------------
  void cli_context_clear(SCliContext * pContext);
  // ----- cli_context_save_depth -----------------------------------
  void cli_context_save_depth(SCliContext * pContext);
  // ----- cli_context_restore_depth --------------------------------
  void cli_context_restore_depth(SCliContext * pContext);
  // ----- cli_context_to_string ------------------------------------
  char * cli_context_to_string(SCliContext * pContext,
			       char * pcPrefix);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_CTX_H__ */
