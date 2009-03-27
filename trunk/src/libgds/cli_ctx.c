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
					   void * user_data,
					   int final)
{
  _cli_ctx_item_t * item=
    (_cli_ctx_item_t *) MALLOC(sizeof(_cli_ctx_item_t));
  item->user_data= user_data;
  item->cmd= cmd;
  item->final= final;
  return item;
}

// -----[ _cli_context_item_destroy ]--------------------------------
static inline void _cli_context_item_destroy(_cli_ctx_item_t ** item_ref)
{
  _cli_ctx_item_t * item= *item_ref;

  if (item != NULL) {
    if (item->final && (item->cmd->ops.ctx_destroy != NULL))
      item->cmd->ops.ctx_destroy(&item->user_data);
    FREE(item);
    *item_ref= NULL;
  }
}

// ----- cli_context_create -----------------------------------------
/**
 *
 */
cli_ctx_t * cli_context_create(cli_cmd_t * cmd, void * user_data)
{
  cli_ctx_t * ctx=
    (cli_ctx_t *) MALLOC(sizeof(cli_ctx_t));
  ctx->cmd_stack= stack_create(CLI_MAX_CONTEXT);
  stack_push(ctx->cmd_stack, _cli_context_item_create(cmd, user_data, 1));
  cli_context_save_depth(ctx);
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
  cli_ctx_t * ctx= *ctx_ref;

  if (ctx != NULL) {
    for (index= 0; index < stack_depth(ctx->cmd_stack); index++) {
      _cli_ctx_item_t * item=
	(_cli_ctx_item_t *) stack_get_at(ctx->cmd_stack, index);
      _cli_context_item_destroy(&item);
    }
    stack_destroy(&ctx->cmd_stack);
    str_destroy(&ctx->string);
    FREE(ctx);
    *ctx_ref= NULL;
  }
}

// -----[ cli_context_depth ]----------------------------------------
/**
 *
 */
int cli_context_depth(const cli_ctx_t * ctx)
{
  return stack_depth(ctx->cmd_stack);
}

// -----[ cli_context_is_empty ]-------------------------------------
/**
 *
 */
int cli_context_is_empty(const cli_ctx_t * ctx)
{
  return stack_is_empty(ctx->cmd_stack);
}

// ----- cli_context_push -------------------------------------------
/**
 * Push a command with parameters and context (user-data) on the
 * stack.
 */
void cli_context_push(cli_ctx_t * ctx, cli_cmd_t * cmd,
		      void * user_data, int final)
{
  stack_push(ctx->cmd_stack,
	     _cli_context_item_create(cmd, user_data, final));
}

// ----- cli_context_pop --------------------------------------------
/**
 *
 */
void cli_context_pop(cli_ctx_t * ctx)
{
  _cli_ctx_item_t * item;

  // Always leave root element on stack
  if (stack_depth(ctx->cmd_stack) <= 1)
    return;

  // Pop until next element is final
  // - pop at least 1 item
  // - leave root element on stack
  do {
    item= (_cli_ctx_item_t *) stack_pop(ctx->cmd_stack);
    _cli_context_item_destroy(&item);
  } while ((stack_depth(ctx->cmd_stack) > 1) &&
	   !((_cli_ctx_item_t *) stack_top(ctx->cmd_stack))->final);
}

// -----[ cli_context_top ]------------------------------------------
const _cli_ctx_item_t * cli_context_top(const cli_ctx_t * ctx)
{
  return (_cli_ctx_item_t *) stack_top(ctx->cmd_stack);
}

// -----[ cli_context_top_cmd ]------------------------------------
const cli_cmd_t * cli_context_top_cmd(const cli_ctx_t * ctx)
{
  return cli_context_top(ctx)->cmd;
}

// -----[ cli_context_top_data ]-------------------------------------
void * cli_context_top_data(const cli_ctx_t * ctx)
{
  return cli_context_top(ctx)->user_data;
}

// -----[ cli_context_get_at ]---------------------------------------
const _cli_ctx_item_t * cli_context_get_at(const cli_ctx_t * ctx,
					   unsigned int index)
{
  return (_cli_ctx_item_t *) stack_get_at(ctx->cmd_stack, index);
}

// -----[ cli_context_get_item_at ]----------------------------------
void * cli_context_get_item_at(const cli_ctx_t * ctx,
			       unsigned int index)
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
void * cli_context_get_item_from_top(const cli_ctx_t * ctx,
					   unsigned int offset)
{
  if (!cli_context_is_empty(ctx))
    return cli_context_get_item_at(ctx,
				   cli_context_depth(ctx)-offset-1);
  return NULL;
}


// ----- cli_context_clear ------------------------------------------
/**
 * Clear context-stack and params
 */
void cli_context_clear(cli_ctx_t * ctx)
{
  while (cli_context_depth(ctx) > 1)
    cli_context_pop(ctx);
  cli_context_save_depth(ctx);
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
  int index;
  const _cli_ctx_item_t * item;

  // Free previous context-string
  str_destroy(&ctx->string);

  // Build new string
  ctx->string= str_create(prefix);

  // Add context commands (skip root command)
  for (index= 1; index < cli_context_depth(ctx); index++) {
    item= cli_context_get_at(ctx, index);
    if ((item != NULL) && (item->cmd != NULL)) {
      ctx->string= str_append(ctx->string, "-");
      ctx->string= str_append(ctx->string, item->cmd->name);
    }
  }
  ctx->string= str_append(ctx->string, "> ");

  return ctx->string;
}

// -----[ cli_context_dump ]-----------------------------------------
void cli_context_dump(gds_stream_t * stream, cli_ctx_t * ctx)
{
  unsigned int index;
  _cli_ctx_item_t * item;

  for (index= stack_depth(ctx->cmd_stack); index > 0; index--) {
    item= (_cli_ctx_item_t *) stack_get_at(ctx->cmd_stack, index-1);
    stream_printf(stream, "stack[%.2u]->cmd : %p (%s)\n",
		  index, item->cmd, item->cmd->name);
    stream_printf(stream, "         ->data: %p\n",
		  item->user_data);
    stream_printf(stream, "         ->final: %d\n", item->final);
  }
}

// -----[ cli_context_set_root_user_data ]---------------------------
void cli_context_set_root_user_data(cli_ctx_t * ctx, void * user_data)
{
  _cli_ctx_item_t * item= (_cli_ctx_item_t *) cli_context_get_at(ctx, 0);
  item->user_data= user_data;
}

