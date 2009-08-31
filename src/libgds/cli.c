// ==================================================================
// @(#)cli.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <libgds/cli.h>
#include <libgds/cli_commands.h>
#include <libgds/cli_ctx.h>
#include <libgds/cli_fsm.h>
#include <libgds/cli_params.h>
#include <libgds/stream.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>

//#define DEBUG
#include <libgds/debug.h>


/////////////////////////////////////////////////////////////////////
//
// DEFAULT COMMANDS
//
/////////////////////////////////////////////////////////////////////

// -----[ _cli_cmd_exit ]--------------------------------------------
static int _cli_cmd_exit(cli_ctx_t * ctx, struct cli_cmd_t * cmd)
{
  cli_context_pop(ctx);
  return CLI_SUCCESS;
}

// -----[ _cli_cmd_debug ]-------------------------------------------
static int _cli_cmd_debug(cli_ctx_t * ctx, struct cli_cmd_t * cmd)
{
  const char * token= cli_get_arg_value(cmd, 0);
  if (token == NULL)
    return CLI_ERROR_CMD_FAILED;
  if (!strcmp(token, "ctx"))
    cli_context_dump(gdserr, ctx);
  else
    return CLI_ERROR_CMD_FAILED;
  return CLI_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
//
// SET OF COMMANDS (PRIVATE FUNCTIONS)
//
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
//
// COMMANDS MANAGMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// -----[ cli_cmd ]--------------------------------------------------
cli_cmd_t * cli_cmd(const char * name,
		    cli_command_f command)
{
  cli_cmd_ops_t ops= {
    .command= command,
  };
  return cli_cmd_create(CLI_CMD_TYPE_STD, name, ops, NULL);
}

// -----[ cli_cmd_ctx ]----------------------------------------------
cli_cmd_t * cli_cmd_ctx(const char * name,
			cli_ctx_create_f ctx_create,
			cli_ctx_destroy_f ctx_destroy)
{
  cli_cmd_ops_t ops= {
    .ctx_create = ctx_create,
    .ctx_destroy= ctx_destroy,
  };
  return cli_cmd_create(CLI_CMD_TYPE_CTX, name, ops, cli_cmds_create());
}

// -----[ cli_cmd_group ]--------------------------------------------
cli_cmd_t * cli_cmd_group(const char * name)
{
  cli_cmd_ops_t ops= { };
  return cli_cmd_create(CLI_CMD_TYPE_GRP, name, ops, cli_cmds_create());
}

// -----[ cli_cmd_prefix ]-------------------------------------------
cli_cmd_t * cli_cmd_prefix(const char * name)
{
  cli_cmd_ops_t ops= { };
  return cli_cmd_create(CLI_CMD_TYPE_PFX, name, ops, cli_cmds_create());
}


// -----[ _cli_matching_cmds ]---------------------------------------
static inline
void _cli_matching_cmds(cli_cmds_t * cmds, const char * text,
			cli_cmds_t ** m_cmds)
{
  unsigned int index;
  size_t text_len= strlen(text);
  cli_cmd_t * cmd;

  if (cmds == NULL)
    return;

  // Build list of matching commands
  for (index= 0; index < cli_cmds_num(cmds); index++) {
    cmd= cli_cmds_at(cmds, index);
    if (cmd->hidden)
      continue;
    if (strncmp(text, cmd->name, text_len))
      continue;
    if (*m_cmds == NULL)
      *m_cmds= cli_cmds_create_ref();
    cli_cmds_add(*m_cmds, cmd);
  }
}

// -----[ cli_matching_subcmds ]-------------------------------------
/**
 * Create a sublist of the commands included in the list (cmds). Only
 * the commands that match the given text (text) are included.
 *
 * IMPORTANT NOTE: the returned list is not a classical command list,
 * since the commands that it includes are only references to the
 * commands from the original list. However, the returned list of
 * commands can be freed using the cli_cmds_destroy function, since
 * the returned array's destroy function is NULL.
 */
cli_cmds_t * cli_matching_subcmds(cli_t * cli, cli_cmd_t * cmd,
				  const char * text, int en_omni)
{
  cli_cmds_t * cmds= NULL;
  if (cmd == NULL)
    return NULL;
  _cli_matching_cmds(cmd->sub_cmds, text, &cmds);
  if (en_omni)
    _cli_matching_cmds(cli->omni_cmd->sub_cmds, text, &cmds);
  return cmds;
}

// -----[ cli_matching_opts ]--------------------------------------
cli_args_t * cli_matching_opts(cli_cmd_t * cmd, const char * text)
{
  unsigned int index;
  size_t text_len= strlen(text);
  cli_arg_t * opt;
  cli_args_t * args= NULL;

  if ((cmd == NULL) || (cmd->opts == NULL))
    return NULL;

  // Skip option name's prefix ("--") 
  if ((text_len < 2) || strncmp("--", text, 2))
    return NULL;
  text+= 2;
  text_len-= 2;
  
  // Build list of matching options
  for (index= 0; index < cli_args_num(cmd->opts); index++) {
    opt= cli_args_at(cmd->opts, index);
    if (strncmp(text, opt->name, text_len))
      continue;
    if (args == NULL)
      args= cli_args_create_ref();
    cli_args_add(args, opt);
  }
  return args;
}


/////////////////////////////////////////////////////////////////////
//
// CLI TREE MANAGMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// -----[ cli_add_cmd ]----------------------------------------------
cli_cmd_t * cli_add_cmd(cli_cmd_t * cmd, cli_cmd_t * sub_cmd)
{
  assert(cmd->sub_cmds != NULL);
  sub_cmd->parent= cmd;
  cli_cmds_add(cmd->sub_cmds, sub_cmd);
  return sub_cmd;
}

// -----[ cli_add_arg ]----------------------------------------------
cli_arg_t * cli_add_arg(cli_cmd_t * cmd, cli_arg_t * arg)
{
  if (cmd->args == NULL)
    cmd->args= cli_args_create();
  return cli_args_add(cmd->args, arg);
}

// -----[ cli_add_opt ]----------------------------------------------
cli_arg_t * cli_add_opt(cli_cmd_t * cmd, cli_arg_t * opt)
{
  if (cmd->opts == NULL)
    cmd->opts= cli_opts_create();
  return cli_opts_add(cmd->opts, opt);
}

/////////////////////////////////////////////////////////////////////
//
// COMMAND-LINE MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// -----[ _cli_register_default_cmds ]-------------------------------
static inline void _cli_register_default_cmds(cli_t * cli)
{
  cli_cmd_t * cmd;
  cli_add_cmd(cli->omni_cmd, cli_cmd("exit", _cli_cmd_exit));
  cmd= cli_add_cmd(cli->omni_cmd, cli_cmd("debug", _cli_cmd_debug));
  cmd->hidden= 1;
  cli_add_arg(cmd, cli_arg("param", NULL));
}

// -----[ cli_create ]-----------------------------------------------
cli_t * cli_create()
{
  cli_t * cli= (cli_t *) MALLOC(sizeof(cli_t));

  cli->user_data= NULL;
  cli->root_cmd= cli_cmd_group("root");
  cli->omni_cmd= cli_cmd_group("omni");
  cli->ctx= cli_context_create(cli->root_cmd, cli->user_data);
  cli->fsm= cli_fsm_create();

  cli->error.user_msg= NULL;
  cli->error.line_number= -1;
  cli->error.source= NULL;

  cli->ops.on_error= NULL;

  _cli_register_default_cmds(cli);
  return cli;
}

// -----[ cli_destroy ]----------------------------------------------
void cli_destroy(cli_t ** cli_ref)
{
  cli_t * cli= *cli_ref;

  if (cli != NULL) {
    cli_context_destroy(&cli->ctx);
    cli_cmd_destroy(&cli->root_cmd);
    cli_cmd_destroy(&cli->omni_cmd);
    if (cli->error.user_msg != NULL)
      free(cli->error.user_msg);
    cli_fsm_destroy(&cli->fsm);
    FREE(cli);
    *cli_ref= NULL;
  }
}

// -----[ cli_get_root_cmd ]-----------------------------------------
cli_cmd_t * cli_get_root_cmd(cli_t * cli)
{
  return cli->root_cmd;
}

// -----[ cli_get_omni_cmd ]-----------------------------------------
cli_cmd_t * cli_get_omni_cmd(cli_t * cli)
{
  return cli->omni_cmd;
}

// -----[ cli_set_on_error ]-----------------------------------------
/**
 * Set the exit callback function. The default behaviour of the CLI
 * is to exit as soon as an error occurs. If this callback is
 * defined, it will be called in order to check it the CLI must
 * exit or not.
 */
void cli_set_on_error(cli_t * cli, cli_on_error_f on_error)
{
  cli->ops.on_error= on_error;
}

// -----[ cli_set_param_lookup ]-------------------------------------
void cli_set_param_lookup(cli_t * cli, param_lookup_t lookup)
{
  tokenizer_set_lookup(cli->fsm->tokenizer, lookup);
}

// -----[ cli_set_user_data ]----------------------------------------
void cli_set_user_data(cli_t * cli, void * user_data)
{
  cli_context_set_root_user_data(cli->ctx, user_data);
}

// -----[ cli_execute ]----------------------------------------------
/**
 * Execute the given command (cmd) with the given CLI (cli) and
 * the given context (user_data).
 */
int cli_execute(cli_t * cli, const char * str)
{
  cli_cmd_t * cmd;
  cli_elem_t elem;
  int error= CLI_SUCCESS;

  __debug("cli_execute(line:\"%s\")\n", str);

  // Get current context
  cli_get_cmd_context(cli, &cmd, NULL);
  cli_context_save_depth(cli->ctx);

  // Parse command-line
  error= cli_fsm_run(cli, cmd, str, NULL, &elem);
  if (error)
    goto cli_execute_error;

  __debug(" +-- cmd:\"%s\"\n", cmd->name);

  // Execute command
  if (elem.cmd == cmd) {
    cli_context_clear(cli->ctx);
  } else {
    error= cli_cmd_exec(cli, elem.cmd);
    if (error)
      goto cli_execute_error;
  }

  return error;

 cli_execute_error:
  cli->error.error= error;
  cli_context_restore_depth(cli->ctx);
  return error;
}

// -----[ cli_complete ]---------------------------------------------
int cli_complete(cli_t * cli, const char * str, const char * compl,
		 cli_elem_t * ctx)
{
  int error;
  cli_context_save_depth(cli->ctx);
  error= cli_fsm_run(cli, NULL, str, compl, ctx);
  cli_context_restore_depth(cli->ctx);
  return error;
}

// -----[ cli_execute_line ]-----------------------------------------
/**
 *
 */
int cli_execute_line(cli_t * cli, const char * line)
{
  int result= CLI_SUCCESS;

  // Skip leading blank spaces
  while ((*line == ' ') || (*line == '\t'))
    line++;

  // Skip commented lines
  if (*line == '#')
    return result;

  // Parse and execute command
  return cli_execute(cli, (char *) line);
}

// -----[ cli_execute_stream ]---------------------------------------
/**
 * Execute all the lines from the given input stream.
 *
 * NOTE: The execution of the script will stop at the first error.
 */
int cli_execute_stream(cli_t * cli, FILE * stream)
{
  char line[CLI_MAX_LINE_LENGTH];
  int result;
  char ch;

  cli->error.line_number= 1;
  
  // Execute all lines in the input file...
  while (fgets(line, sizeof(line), stream) != NULL) {

    if ((strchr(line, '\n') == NULL) && !feof(stream)) {
      // Newline not found, flush stream to end of line (see STR35-C)
      result= CLI_ERROR_LINE_TOO_LONG;
      while (((ch= fgetc(stream)) != '\n') &&
	     !feof(stream) && !ferror(stream));
    } else {
      result= cli_execute_line(cli, line);
    }

    if (result < 0) {
      // In case of error, we call the 'on_error' function (if defined).
      // The decision to stop processing depends on the error code
      // returned by this function. If the 'on_error' function
      // is not defined, we just return the error code.
      if (cli->ops.on_error != NULL)
	result= cli->ops.on_error(cli, result);
      if (result)
	return result;
    }
    cli->error.line_number++;
  }

  // Clear the context
  cli_context_clear(cli->ctx);

  return CLI_SUCCESS;
}

// -----[ cli_get_cmd_context ]--------------------------------------
/**
 * Returns the current command context. If there is currently no
 * context, the root command is returned.
 *
 * Note: the function can also optionaly return the current
 *       user-data (if and only if the passed 'user_data' pointer
 *       is not NULL).
 */
void cli_get_cmd_context(cli_t * cli, cli_cmd_t ** cmd,
			 void ** user_data)
{
  const _cli_ctx_item_t * item= cli_context_top(cli->ctx);
  assert(item != NULL);
  *cmd= item->cmd;
  if (user_data != NULL)
    *user_data= item->user_data;
}


/////////////////////////////////////////////////////////////////////
//
// CLI ERROR MANAGEMENT
//
/////////////////////////////////////////////////////////////////////

// -----[ cli_perror ]-----------------------------------------------
void cli_perror(gds_stream_t * stream, cli_error_type_t error)
{
  const char * error_str= cli_strerror(error);
  if (error_str != NULL)
    stream_printf(stream, error_str);
  else
    stream_printf(stream, "unknown error (%i)", error);
}

// -----[ cli_strerror ]---------------------------------------------
const char * cli_strerror(cli_error_type_t error)
{
  switch (error) {
  case CLI_SUCCESS:
    return "success";
  case CLI_ERROR_GENERIC:
    return "error";
  case CLI_ERROR_UNEXPECTED:
    return "unexpected error";
  case CLI_ERROR_UNKNOWN_CMD:
    return "unknown command";
  case CLI_ERROR_MISSING_ARG:
    return "missing argument";
  case CLI_ERROR_TOO_MANY_ARGS:
    return "too many arguments (vararg)";
  case CLI_ERROR_NOT_A_CMD:
    return "not a command";
  case CLI_ERROR_CMD_FAILED:
    return "command failed";
  case CLI_ERROR_BAD_ARG_VALUE:
    return "bad argument value";
  case CLI_ERROR_CTX_CREATE:
    return "unable to create context";
  case CLI_ERROR_UNKNOWN_OPT:
    return "unknown option";
  case CLI_ERROR_MISSING_OPT_VALUE:
    return "missing option value";
  case CLI_ERROR_OPT_NO_VALUE:
    return "option does not take a value";
  case CLI_ERROR_SYNTAX:
    return "syntax error";
  case CLI_ERROR_COMPL_FAILED:
    return "completion could not be performed";
  case CLI_ERROR_LINE_TOO_LONG:
    return "input line too long";
  case CLI_SUCCESS_TERMINATE:
    gds_fatal("CLI_SUCCESS_TERMINATE is not an error");
  }
  return NULL;
}

static inline void _highlight(gds_stream_t * stream)
{
  stream_printf(stream, "\033[1m");
}

static inline void _normal(gds_stream_t * stream)
{
  stream_printf(stream, "\033[0m");
}

static inline void _red(gds_stream_t * stream)
{
  stream_printf(stream, "\033[0;31;1m");
}

// -----[ _cli_fsm_dump_matched_tokens ]-----------------------------
static inline
void _cli_fsm_dump_matched_tokens(gds_stream_t * stream, cli_fsm_t * fsm,
				  int hl_index)
{
  unsigned int index;
  const char * token;

  for (index= 0; index < tokens_get_num(fsm->tokens); index++) {
    token= tokens_get_string_at(fsm->tokens, index);
    if (index == hl_index) {
      _highlight(stream);
      stream_printf(stream, "%s", token);
      _normal(stream);
    } else
      stream_printf(stream, "%s", token);
    stream_printf(stream, " ");
  }

  if (hl_index > tokens_get_num(fsm->tokens)) {
    _highlight(stream);
    stream_printf(stream, "??");
    _normal(stream);
  }
}

// -----[ cli_perror_details ]---------------------------------------
/**
 * Show detailled error messages depending on the error code.
 */
void cli_perror_details(gds_stream_t * stream, cli_t * cli)
{
  unsigned int index;
  cli_cmd_t * cmd;
  cli_arg_t * arg;

  if (cli->error.error >= CLI_SUCCESS)
    return;

  if (cli->error.line_number >= 0)
    stream_printf(stream, " +-- line  : %d\n", cli->error.line_number);
  if (cli->error.source != NULL)
    stream_printf(stream, " +-- source: \"%s\"\n", cli->error.source);

  switch (cli->error.error) {

  case CLI_ERROR_UNKNOWN_CMD:
    stream_printf(stream, " +-- input : ");
    _cli_fsm_dump_matched_tokens(stream, cli->fsm, cli->fsm->token_index-1);
    stream_printf(stream, "\n");
    if (cli->fsm->cmd->sub_cmds != NULL) {
      stream_printf(stream, " +-- expect: ");
      for (index= 0; index < cli_cmds_num(cli->fsm->cmd->sub_cmds); index++) {
	cmd= cli_cmds_at(cli->fsm->cmd->sub_cmds, index);
	stream_printf(stream, "%s ", cmd->name);
      }
      stream_printf(stream, "\n");
    }
    break;

  case CLI_ERROR_NOT_A_CMD:
    break;

  case CLI_ERROR_MISSING_ARG:
    stream_printf(stream, " +-- input : ");
    _cli_fsm_dump_matched_tokens(stream, cli->fsm, cli->fsm->token_index);
    stream_printf(stream, "\n");
    stream_printf(stream, " +-- expect: ");
    cli_arg_dump(stream, cli_args_at(cli->fsm->cmd->args, cli->fsm->param_index));
    stream_printf(stream, "\n");
    break;

  case CLI_ERROR_BAD_ARG_VALUE:
    break;

  case CLI_ERROR_UNKNOWN_OPT:
    stream_printf(stream, " +-- input : ");
    _cli_fsm_dump_matched_tokens(stream, cli->fsm, cli->fsm->token_index-1);
    stream_printf(stream, "\n");
    if (cli->fsm->cmd->opts != NULL) {
      stream_printf(stream, " +-- expect: ");
      for (index= 0; index < cli_args_num(cli->fsm->cmd->opts); index++) {
	arg= cli_args_at(cli->fsm->cmd->opts, index);
	cli_arg_dump(stream, arg);
	stream_printf(stream, " ");
      }
      stream_printf(stream, "\n");
    }
    break;

  case CLI_ERROR_MISSING_OPT_VALUE:
    stream_printf(stream, " +-- input : ");
    _cli_fsm_dump_matched_tokens(stream, cli->fsm, cli->fsm->token_index-1);
    stream_printf(stream, "\n");
    break;

  case CLI_ERROR_OPT_NO_VALUE:
    stream_printf(stream, " +-- input : ");
    _cli_fsm_dump_matched_tokens(stream, cli->fsm, cli->fsm->token_index-1);
    stream_printf(stream, "\n");
    break;

  case CLI_ERROR_CMD_FAILED:
  case CLI_ERROR_CTX_CREATE:
  case CLI_ERROR_SYNTAX:
    if (cli->error.user_msg != NULL) {
      stream_printf(stream, " +-- reason: ");
      stream_printf(stream, "%s", cli->error.user_msg);
      stream_printf(stream, "\n");
    }
    break;

  default:
    gds_warn("no details available for this error code (%d)\n",
	     cli->error.error);
  }
}

// -----[ cli_dump_error ]-------------------------------------------
void cli_dump_error(gds_stream_t * stream, cli_t * cli)
{
  _red(stream);
  stream_printf(stream, "Error: ");
  cli_perror(stream, cli->error.error);
  _normal(stream);
  stream_printf(stream, "\n"),
  cli_perror_details(stream, cli);
}

// ----- cli_get_error_details --------------------------------------
int cli_get_error_details(cli_t * cli, cli_error_t * error)
{
  memcpy(error, &cli->error, sizeof(cli_error_t));
  return cli->error.error;
}

// ----- cli_set_user_error -----------------------------------------
void cli_set_user_error(cli_t * cli, const char * format, ...)
{
#define CLI_USER_ERROR_BUFFER_SIZE 1000
  char buf[CLI_USER_ERROR_BUFFER_SIZE];
  va_list ap;

  if (cli->error.user_msg != NULL) {
    str_destroy(&cli->error.user_msg);
    cli->error.user_msg= NULL;
  }

  if (format != NULL) {
    va_start(ap, format);
    vsnprintf(buf, CLI_USER_ERROR_BUFFER_SIZE, format, ap);
    
    cli->error.user_msg= str_create(buf);
  }
}

// -----[ cli_get_arg_value ]--------------------------------------
const char * cli_get_arg_value(const cli_cmd_t * cmd, unsigned int index)
{
  unsigned int num_args;
  assert(cmd->args != NULL);
  num_args= cli_args_num(cmd->args);
  if (index < num_args)
    return cli_arg_get_value(cli_args_at(cmd->args, index), 0);
  else
    return cli_arg_get_value(cli_args_at(cmd->args, num_args-1),
			     index-num_args);
}

// -----[ cli_get_arg_num_values ]-----------------------------------
unsigned int cli_get_arg_num_values(const cli_cmd_t * cmd,
				    unsigned int index)
{
  cli_arg_t * arg;
  assert(cmd->args != NULL);
  arg= cli_args_at(cmd->args, index);
  if (arg == NULL)
    gds_fatal("trying to get non-existing argument (index:%u) in cmd \"%s\"\n",
	      index, cmd->name);
  return cli_arg_get_num_values(arg);
}


// -----[ cli_get_opt_value ]--------------------------------------
const char * cli_get_opt_value(const cli_cmd_t * cmd, const char * name)
{
  cli_arg_t * opt;
  assert(cmd->opts != NULL);
  opt= cli_opts_find(cmd->opts, name);
  if (opt == NULL)
    gds_fatal("trying to get non-existing option \"%s\" in cmd \"%s\"\n",
	     name, cmd->name);
  return opt->value;
}

// -----[ cli_has_opt_value ]--------------------------------------
int cli_has_opt_value(const cli_cmd_t * cmd, const char * name)
{
  cli_arg_t * opt;
  assert(cmd->opts != NULL);
  opt= cli_opts_find(cmd->opts, name);
  if (opt == NULL)
    gds_fatal("trying to get non-existing option \"%s\" in cmd \"%\"\n",
	      name, cmd->name);
  return opt->present;
}
