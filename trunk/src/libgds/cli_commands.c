// ==================================================================
// @(#)cli_commands.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 02/12/2008
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgds/cli_commands.h>
#include <libgds/cli_ctx.h>
#include <libgds/gds.h>

//#define DEBUG
#include <libgds/debug.h>

// -----[ cli_cmd_enter ]--------------------------------------------
int cli_cmd_enter(cli_t * cli, cli_cmd_t * cmd, int final)
{
  int result= CLI_ERROR_GENERIC;
  void * user_data;

  __debug("cli_cmd_enter(cmd:\"%s\")\n", cmd->name);

  switch (cmd->type) {
  case CLI_CMD_TYPE_CTX:
    if (cmd->ops.ctx_create != NULL)
      result= cmd->ops.ctx_create(cli->ctx, cmd, &user_data);
    if (result != CLI_SUCCESS)
      return result;
    cli_context_push(cli->ctx, cmd, user_data, final);
    break;

  case CLI_CMD_TYPE_PFX:
    // Do nothing, this is just a prefix
    result= CLI_SUCCESS;
    break;
    
  case CLI_CMD_TYPE_GRP:
    result= CLI_SUCCESS;
    user_data= cli_context_top(cli->ctx)->user_data;
    cli_context_push(cli->ctx, cmd, user_data, final);
    break;

  default:
    gds_fatal("Impossible to enter cmd with type %d\n", cmd->type);
  }

  return result;
}

// -----[ cli_cmd_exec ]---------------------------------------------
int cli_cmd_exec(cli_t * cli, cli_cmd_t * cmd)
{
  int result= CLI_SUCCESS;

  __debug("cli_cmd_exec(cmd:\"%s\")\n", cmd->name);

  cli_set_user_error(cli, NULL);

  switch (cmd->type) {
  case CLI_CMD_TYPE_STD:
    if (cmd->ops.command != NULL)
      result= cmd->ops.command(cli->ctx, cmd);
    cli_context_restore_depth(cli->ctx);
    break;

  case CLI_CMD_TYPE_CTX:
  case CLI_CMD_TYPE_GRP:
    result= cli_cmd_enter(cli, cmd, 1);
    break;

  case CLI_CMD_TYPE_PFX:
    result= CLI_ERROR_NOT_A_CMD;
    break;
    
  default:
    gds_fatal("Impossible to execute cmd with type %d\n", cmd->type);
  }
      
  return result;
}
