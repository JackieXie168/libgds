// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structure and functions to manage the CLI context
 * stack. This is mainly for internal use.
 */

#ifndef __GDS_CLI_CTX_H__
#define __GDS_CLI_CTX_H__

#include <libgds/cli.h>

#ifdef __cplusplus
extern "C" {
#endif

  // ----- cli_context_create ---------------------------------------
  /**
   * \internal
   * Create a CLI context stack.
   */
  cli_ctx_t * cli_context_create(cli_cmd_t * cmd, void * user_data);

  // ----- cli_context_create ---------------------------------------
  /**
   * \internal
   * Destroy a CLI context stack.
   */
  void cli_context_destroy(cli_ctx_t ** pctx);

  // ----- cli_context_depth ----------------------------------------
  /**
   * Return the despth of the CLI context stack.
   */
  int cli_context_depth(const cli_ctx_t * ctx);

  // ----- cli_context_is_empty -------------------------------------
  /**
   * Test if the CLI context stack is empty.
   */
  int cli_context_is_empty(const cli_ctx_t * ctx);

  // ----- cli_context_push -----------------------------------------
  /**
   * \internal
   * Push data on the CLI context stack.
   */
  void cli_context_push(cli_ctx_t * ctx, cli_cmd_t * cmd,
			void * user_data, int final);

  // ----- cli_context_pop ------------------------------------------
  /**
   * \internal
   * Pop data from the CLI context stack.
   */
  void cli_context_pop(cli_ctx_t * ctx);

  // ----- cli_context_top ------------------------------------------
  /**
   * Return the top of the CLI context stack.
   */
  const _cli_ctx_item_t * cli_context_top(const cli_ctx_t * ctx);

  // -----[ cli_context_top_cmd ]------------------------------------
  /**
   * Return the top command from the CLI context stack.
   */
  const cli_cmd_t * cli_context_top_cmd(const cli_ctx_t * ctx);

  // -----[ cli_context_top_data ]-----------------------------------
  /**
   * Return the top data from the CLI context stack.
   */
  void * cli_context_top_data(const cli_ctx_t * ctx);

  // ----- cli_context_get_at ---------------------------------------
  /**
   * Return data from the CLI context stack.
   */
  const _cli_ctx_item_t * cli_context_get_at(const cli_ctx_t * ctx,
					     unsigned int index);

  // ----- cli_context_get_item_at ----------------------------------
  /**
   * Return user-data from the CLI context stack.
   */
  void * cli_context_get_item_at(const cli_ctx_t * ctx,
				 unsigned int index);

  // ----- cli_context_get_item_from_top ----------------------------
  void * cli_context_get_item_from_top(const cli_ctx_t * ctx,
				       unsigned int offset);

  // ----- cli_context_clear ----------------------------------------
  /**
   * \internal
   */
  void cli_context_clear(cli_ctx_t * ctx);

  // ----- cli_context_save_depth -----------------------------------
  /**
   * \internal
   */
  void cli_context_save_depth(cli_ctx_t * ctx);

  // ----- cli_context_restore_depth --------------------------------
  /**
   *
   */
  void cli_context_restore_depth(cli_ctx_t * ctx);

  // ----- cli_context_to_string ------------------------------------
  char * cli_context_to_string(cli_ctx_t * ctx, char * prefix);
  // -----[ cli_context_dump ]-----------------------------------------
  void cli_context_dump(gds_stream_t * stream, cli_ctx_t * ctx);

  // -----[ cli_context_set_root_user_data ]---------------------------
  /**
   * \internal
   */
  void cli_context_set_root_user_data(cli_ctx_t * ctx, void * user_data);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_CTX_H__ */
