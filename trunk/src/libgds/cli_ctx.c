// ==================================================================
// @(#)cli_ctx.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
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
static inline
_cli_ctx_item_t * _cli_context_item_create(cli_cmd_t * cmd,
					   void * user_data)
{
  _cli_ctx_item_t * item=
    (_cli_ctx_item_t *) MALLOC(sizeof(_cli_ctx_item_t));
  item->user_data= user_data;
  item->cmd= cmd;
  return item;
}

// -----[ _cli_context_item_destroy ]--------------------------------
static inline void _cli_context_item_destroy(_cli_ctx_item_t ** item_ref)
{
  if (*item_ref != NULL) {
    if ((*item_ref)->cmd->fCtxDestroy != NULL)
      (*item_ref)->cmd->fCtxDestroy(&(*item_ref)->user_data);
    FREE(*item_ref);
    *item_ref= NULL;
  }
}

// ----- cli_context_create -----------------------------------------
/**
 *
 */
cli_ctx_t * cli_context_create()
{
  cli_ctx_t * ctx=
    (cli_ctx_t *) MALLOC(sizeof(cli_ctx_t));
  ctx->cmd_stack= stack_create(CLI_MAX_CONTEXT);
  ctx->saved_depth= 0;
  ctx->string= NULL;
  return ctx;
}

// ----- cli_context_destroy ----------------------------------------
/**
 *
 */
void cli_context_destroy(cli_ctx_t ** ctx_ref)
{
  unsigned int index;

  if (*ctx_ref != NULL) {
    for (index= 0; index < stack_depth((*ctx_ref)->cmd_stack); index++) {
      _cli_ctx_item_t * item=
	(_cli_ctx_item_t *) stack_get_at((*ctx_ref)->cmd_stack, index);
      _cli_context_item_destroy(&item);
    }
    stack_destroy(&(*ctx_ref)->cmd_stack);
    str_destroy(&(*ctx_ref)->string);
    FREE(*ctx_ref);
    *ctx_ref= NULL;
  }
}

// ----- cli_context_depth ------------------------------------------
/**
 *
 */
int cli_context_depth(cli_ctx_t * ctx)
{
  return stack_depth(ctx->cmd_stack);
}

// ----- cli_context_is_empty ---------------------------------------
/**
 *
 */
int cli_context_is_empty(cli_ctx_t * ctx)
{
  return stack_is_empty(ctx->cmd_stack);
}

// ----- cli_context_push -------------------------------------------
/**
 * Push a command with parameters and context (user-data) on the
 * stack.
 */
void cli_context_push(cli_ctx_t * ctx)
{
  _cli_ctx_item_t * item= _cli_context_item_create(ctx->cmd,
						   ctx->user_data);
  stack_push(ctx->cmd_stack, item);
}

// ----- cli_context_pop --------------------------------------------
/**
 *
 */
void cli_context_pop(cli_ctx_t * ctx)
{
  _cli_ctx_item_t * item;
  if (!stack_is_empty(ctx->cmd_stack)) {
    item= (_cli_ctx_item_t *) stack_pop(ctx->cmd_stack);
    ctx->cmd= item->cmd;
    ctx->user_data= item->user_data;
    _cli_context_item_destroy(&item);
  }
}

// ----- cli_context_top ---------------------------------------------
/**
 *
 */
_cli_ctx_item_t * cli_context_top(cli_ctx_t * ctx)
{
  return (_cli_ctx_item_t *) stack_top(ctx->cmd_stack);
}

// ----- cli_context_get --------------------------------------------
/**
 * Get current context (user-data).
 */
void * cli_context_get(cli_ctx_t * ctx)
{
  return ctx->user_data;
}

// ----- cli_context_get_at ------------------------------------------
/**
 *
 */
_cli_ctx_item_t * cli_context_get_at(cli_ctx_t * ctx, uint32_t index)
{
  return (_cli_ctx_item_t *) stack_get_at(ctx->cmd_stack, index);
}

// ----- cli_context_get_item_at ------------------------------------
/**
 *
 */
void * cli_context_get_item_at(cli_ctx_t * ctx, uint32_t index)
{
  _cli_ctx_item_t * item=
    (_cli_ctx_item_t *) stack_get_at(ctx->cmd_stack, index);
  if (item != NULL)
    return item->user_data;
  return NULL;
}

// ----- cli_context_get_item_from_top ------------------------------
/**
 *
 */
void * cli_context_get_item_from_top(cli_ctx_t * ctx,
				     int offset)
{
  if (!cli_context_is_empty(ctx))
    return cli_context_get_item_at(ctx,
				   cli_context_depth(ctx)-offset-1);
  return NULL;
}


// ----- cli_context_get_item_at_top --------------------------------
/**
 *
 */
void * cli_context_get_item_at_top(cli_ctx_t * ctx)
{
  if (!cli_context_is_empty(ctx))
    return cli_context_get_item_at(ctx, cli_context_depth(ctx)-1);
  return NULL;
}

// ----- cli_context_clear ------------------------------------------
/**
 * Clear context-stack and params
 */
void cli_context_clear(cli_ctx_t * ctx)
{
  while (cli_context_depth(ctx) > 0)
    cli_context_pop(ctx);
  ctx->saved_depth= 0;
}

// ----- cli_context_save_depth -------------------------------------
/**
 *
 */
void cli_context_save_depth(cli_ctx_t * ctx)
{
  ctx->saved_depth= stack_depth(ctx->cmd_stack);
}

// ----- cli_context_restore_depth ----------------------------------
/**
 *
 */
void cli_context_restore_depth(cli_ctx_t * ctx)
{
  while (cli_context_depth(ctx) > ctx->saved_depth)
    cli_context_pop(ctx);
}

// ----- cli_context_to_string --------------------------------------
/**
 * This function creates a string with all the context's elements in
 * order. This string can be used as a prompt in a user-interface or
 * error messages.
 */
char * cli_context_to_string(cli_ctx_t * ctx, char * prefix)
{
  int index;//, iTokenOffset, iParamIndex;
  _cli_ctx_item_t * item;

  // Free previous context-string
  str_destroy(&ctx->string);

  // Build new string
  ctx->string= str_create(prefix);

  //iTokenOffset= 0;
  // Add context commands
  for (index= 0; index < cli_context_depth(ctx); index++) {
    item= cli_context_get_at(ctx, index);
    if ((item != NULL) && (item->cmd != NULL)) {
      str_append(&ctx->string, "-");
      str_append(&ctx->string, item->cmd->name);

      // Add token values
      /*
      for (iParamIndex= 0;
	   iParamIndex < cli_cmd_get_num_params(pCtxItem->cmd);
	   iParamIndex++) {
	str_append(&ctx->string, " ");
	str_append(&ctx->string,
		   tokens_get_string_at(ctx->pTokens, iTokenOffset));
	iTokenOffset++;
      }
      */

    }
  }
  str_append(&ctx->string, "> ");

  return ctx->string;
}

