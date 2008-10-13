// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

#ifndef __GDS_CLI_CTX_H__
#define __GDS_CLI_CTX_H__

#include <libgds/cli.h>

#ifdef __cplusplus
extern "C" {
#endif

  // ----- cli_context_create ---------------------------------------
  cli_ctx_t * cli_context_create();
  // ----- cli_context_create ---------------------------------------
  void cli_context_destroy(cli_ctx_t ** pctx);
  // ----- cli_context_depth ----------------------------------------
  int cli_context_depth(cli_ctx_t * ctx);
  // ----- cli_context_is_empty -------------------------------------
  int cli_context_is_empty(cli_ctx_t * ctx);
  // ----- cli_context_push -----------------------------------------
  void cli_context_push(cli_ctx_t * ctx);
  // ----- cli_context_pop ------------------------------------------
  void cli_context_pop(cli_ctx_t * ctx);
  // ----- cli_context_top ------------------------------------------
  _cli_ctx_item_t * cli_context_top(cli_ctx_t * ctx);
  // ----- cli_context_get ------------------------------------------
  void * cli_context_get(cli_ctx_t * ctx);
  // ----- cli_context_set ------------------------------------------
  void cli_context_set(cli_ctx_t * ctx, void * pCtx);
  // ----- cli_context_get_at ---------------------------------------
  _cli_ctx_item_t * cli_context_get_at(cli_ctx_t * ctx,
				       uint32_t index);
  // ----- cli_context_get_item_at ----------------------------------
  void * cli_context_get_item_at(cli_ctx_t * ctx,
				 uint32_t index);
  // ----- cli_context_get_item_from_top ----------------------------
  void * cli_context_get_item_from_top(cli_ctx_t * ctx,
				       int offset);
  // ----- cli_context_get_item_at_top ------------------------------
  void * cli_context_get_item_at_top(cli_ctx_t * ctx);
  // ----- cli_context_clear ----------------------------------------
  void cli_context_clear(cli_ctx_t * ctx);
  // ----- cli_context_save_depth -----------------------------------
  void cli_context_save_depth(cli_ctx_t * ctx);
  // ----- cli_context_restore_depth --------------------------------
  void cli_context_restore_depth(cli_ctx_t * ctx);
  // ----- cli_context_to_string ------------------------------------
  char * cli_context_to_string(cli_ctx_t * ctx,
			       char * prefix);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_CTX_H__ */
