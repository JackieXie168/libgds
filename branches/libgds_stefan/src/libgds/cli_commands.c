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
#include <libgds/cli_params.h>
#include <libgds/gds.h>
#include <libgds/memory.h>

//#define DEBUG
#include <libgds/debug.h>

// -----[ _cli_cmds_item_cmp ]---------------------------------------
/**
 * Compare two commands based on their names. The lexicographic
 * ordering is used for the comparison. This is used for sorting
 * commands in the CLI command tree.
 */
static int _cli_cmds_item_cmp(const void * item1,
			      const void * item2,
			      unsigned int elt_size)
{
  cli_cmd_t * cmd1= *((cli_cmd_t **) item1);
  cli_cmd_t * cmd2= *((cli_cmd_t **) item2);

  return strcmp(cmd1->name, cmd2->name);
}

// -----[ _cli_cmds_item_destroy ]-----------------------------------
/**
 * Free a command in a list of commands.
 */
static void _cli_cmds_item_destroy(void * item, const void * ctx)
{
  cli_cmd_destroy((cli_cmd_t **) item);
}

// -----[ cli_cmds_create ]------------------------------------------
/**
 * Create a list of command. The commands in the list are sorted based
 * on the alphanumeric ordering (thanks to the _cli_cmds_item_compare
 * function).
 */
cli_cmds_t * cli_cmds_create()
{
  return (cli_cmds_t *) ptr_array_create(ARRAY_OPTION_SORTED,
					 _cli_cmds_item_cmp,
					 _cli_cmds_item_destroy,
					 NULL);
}

// -----[ cli_cmds_create_ref ]--------------------------------------
cli_cmds_t * cli_cmds_create_ref()
{
  return (cli_cmds_t *) ptr_array_create(ARRAY_OPTION_SORTED,
					 _cli_cmds_item_cmp,
					 NULL, NULL);
}

// -----[ _cli_cmds_find ]-------------------------------------------
static inline
cli_cmd_t * _cli_cmds_find(cli_cmds_t * cmds, const char * name)
{
  cli_cmd_t dummy= { .name= (char *) name };
  cli_cmd_t * ptr= &dummy;
  unsigned int index;

  if (!ptr_array_sorted_find_index(cmds, &ptr, &index))
    return cmds->data[index];
  return NULL;
}

// -----[ cli_cmd_create ]-------------------------------------------
cli_cmd_t * cli_cmd_create(cli_cmd_type_t type,
			   const char * name,
			   cli_cmd_ops_t ops,
			   cli_cmds_t * sub_cmds)
{
  cli_cmd_t * cmd= (cli_cmd_t *) MALLOC(sizeof(cli_cmd_t));
  cmd->type= type;
  cmd->name= str_create(name);
  cmd->parent= NULL;
  cmd->ops= ops;
  cmd->opts= NULL;
  cmd->args= NULL;
  cmd->sub_cmds= sub_cmds;
  cmd->help= NULL;
  cmd->hidden= 0;
  return cmd;
}

// -----[ cli_cmd_destroy ]-----------------------------------------
/**
 * Destroy a CLI command.
 */
void cli_cmd_destroy(cli_cmd_t ** cmd_ref)
{
  cli_cmd_t * cmd= *cmd_ref;

  if (cmd != NULL) {
    str_destroy(&cmd->name);
    cli_cmds_destroy(&cmd->sub_cmds);
    cli_args_destroy(&cmd->args);
    cli_opts_destroy(&cmd->opts);
    FREE(cmd);
    *cmd_ref= NULL;
  }
}

// -----[ cli_cmd_find_submd ]---------------------------------------
cli_cmd_t * cli_cmd_find_subcmd(cli_cmd_t * cmd, const char * name)
{
  if (cmd->sub_cmds == NULL)
    return NULL;
  return _cli_cmds_find(cmd->sub_cmds, name);
}

// -----[ cli_cmd_dump ]---------------------------------------------
void cli_cmd_dump(gds_stream_t * stream, char * prefix, cli_cmd_t * cmd,
		  int recurse)
{
  unsigned int index;
  cli_cmd_t * sub_cmd;
  char * sub_prefix;

  stream_printf(stream, "%s%s", prefix, cmd->name);
  if (cmd->opts != NULL)
    cli_args_dump(stream, cmd->opts);
  if (cmd->args != NULL)
    cli_args_dump(stream, cmd->args);
  stream_printf(stream, "\n");

  if ((cmd->sub_cmds != NULL) && recurse) {
    sub_prefix= str_append(str_create(prefix), "  ");
    for (index= 0; index < cli_cmds_num(cmd->sub_cmds); index++) {
      sub_cmd= cli_cmds_at(cmd->sub_cmds, index);
      cli_cmd_dump(stream, sub_prefix, sub_cmd, 1);
    }
    str_destroy(&sub_prefix);
  }
}

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
