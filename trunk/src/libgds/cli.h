// ==================================================================
// @(#)cli.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

#ifndef __GDS_CLI_H__
#define __GDS_CLI_H__

#include <libgds/array.h>
#include <libgds/cli_params.h>
#include <libgds/stream.h>
#include <libgds/stack.h>
#include <stdio.h>
#include <libgds/tokenizer.h>

#define MAX_CLI_LINE_LENGTH 1024

#define CLI_SUCCESS                    0   // The execution was successful
#define CLI_ERROR_GENERIC             -1   // Generic error (no details)
#define CLI_ERROR_UNEXPECTED          -2   // Error detected, cause unknown
#define CLI_ERROR_UNKNOWN_COMMAND     -3   // Command does not exist in context
#define CLI_ERROR_MISSING_PARAM       -4   // Command needs more parameters
#define CLI_ERROR_NOT_A_COMMAND       -5   // Command cannot be executed
#define CLI_ERROR_COMMAND_FAILED      -6   // Command returned an error
#define CLI_ERROR_BAD_PARAM           -7   // Invalid parameter value
#define CLI_ERROR_CTX_CREATE          -8   // Context creation failed
#define CLI_ERROR_TOO_MANY_PARAMS     -9   // Too many parameters provided
#define CLI_ERROR_UNKNOWN_OPTION      -10  // Option does not exist in command
#define CLI_ERROR_BAD_OPTION          -11  // Invalid option value
#define CLI_ERROR_SYNTAX              -12  // Syntax error
#define CLI_WARNING_EMPTY_COMMAND      1
#define CLI_SUCCESS_TERMINATE          2
#define CLI_SUCCESS_HELP               3

#define CLI_MATCH_NOTHING      0
#define CLI_MATCH_COMMAND      1
#define CLI_MATCH_OPTION_NAMES 2
#define CLI_MATCH_OPTION_VALUE 3
#define CLI_MATCH_PARAM_VALUE  4

typedef SPtrArray cli_cmds_t;

struct cli_cmd_t;

typedef struct {
  void             * user_data;         // Current user data
  struct cli_cmd_t * cmd;           // Current command
  gds_stack_t      * cmd_stack;       // Stack of commands
  unsigned int       saved_depth;
  char             * string;
} cli_ctx_t;

typedef int (*FCliContextCreate)(cli_ctx_t * ctx, void ** item_ref);
typedef void (*FCliContextDestroy)(void ** item_ref);
typedef int (*FCliCommand)(cli_ctx_t * ctx, struct cli_cmd_t * cmd);
typedef void (*FCliHelp)(struct cli_cmd_t * cmd, void * ctx);
typedef int (*FCliExitOnError)(int result);

typedef struct cli_cmd_t {
  char               * name;
  cli_cmds_t         * sub_cmds;
  cli_params_t       * params;
  struct cli_cmd_t   * parent;
  gds_tokens_t       * param_values;
  cli_options_t      * options;
  FCliContextCreate    fCtxCreate;
  FCliContextDestroy   fCtxDestroy;
  FCliCommand          fCommand;
  char               * help;
} cli_cmd_t;

typedef struct {
  void      * user_data;     // User data
  cli_cmd_t * cmd;           // Saved command
} _cli_ctx_item_t;

typedef struct {
  int    error;       // Last error code
  int    line_number; // Line number of last command
  int    user_error;  // Last application error code
                      // (has a meaning only if last command failed)
  char * user_msg;    // Last application error message
                      // (has a meaning only if last command failed)
} cli_error_t;

typedef struct {
  gds_tokenizer_t * tokenizer;
  cli_cmd_t       * root_cmd;
  cli_ctx_t       * ctx;             // Current execution context (stack)
  FCliExitOnError   fExitOnError;    // Exit callback function
  FCliHelp          fHelp;           // Help callback function
  // --- Variables used for error reporting purpose ---
  int               uExecTokenIndex; // Index to current token in cli
  cli_param_t     * exec_param;      // Currently expected parameter
  cli_error_t       error;
} cli_t;

#ifdef __cplusplus
extern "C" {
#endif
  
  ///////////////////////////////////////////////////////////////////
  // COMMANDS MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // ----- cli_cmds_create ------------------------------------------
  cli_cmds_t * cli_cmds_create();
  // ----- cli_cmds_destroy -----------------------------------------
  void cli_cmds_destroy(cli_cmds_t ** cmds_ref);
  // ----- cli_matching_cmds ----------------------------------------
  cli_cmds_t * cli_matching_cmds(cli_cmds_t * cmds, const char * text);
  // ----- cli_cmds_add ---------------------------------------------
  int cli_cmds_add(cli_cmds_t * cmds, cli_cmd_t * cmd);
  // ----- cli_cmd_create -------------------------------------------
  cli_cmd_t * cli_cmd_create(char * name, FCliCommand fCommand,
			     cli_cmds_t * sub_cmds,
			     cli_params_t * params);
  // ----- cli_cmd_create -------------------------------------------
  cli_cmd_t * cli_cmd_create_ctx(char * name,
				 FCliContextCreate fCtxCreate,
				 FCliContextDestroy fCtxDestroy,
				 cli_cmds_t * sub_cmds,
				 cli_params_t * params);
  // ----- cli_cmd_destroy ------------------------------------------
  void cli_cmd_destroy(cli_cmd_t ** cmd_ref);
  // ----- cli_cmd_dump ---------------------------------------------
  void cli_cmd_dump(gds_stream_t * stream, char * prefix,
		    cli_cmd_t * cmd);
  // ----- cli_cmd_add_subcmd ---------------------------------------
  int cli_cmd_add_subcmd(cli_cmd_t * cmd, cli_cmd_t * sub_cmd);
  // ----- cli_cmd_find_submd ---------------------------------------
  cli_cmd_t * cli_cmd_find_subcmd(cli_cmd_t * cmd, char * name);
  // ----- cli_cmd_get_num_subcmds ----------------------------------
  int cli_cmd_get_num_subcmds(cli_cmd_t * cmd);
  // ----- cli_cmd_get_subcmd_at ------------------------------------
  cli_cmd_t * cli_cmd_get_subcmd_at(cli_cmd_t * cmd, unsigned int index);
  // ----- cli_cmd_add_param ----------------------------------------
  int cli_cmd_add_param(cli_cmd_t * cmd, char * name,
			FCliCheckParam fCheckParam);
  // ----- cli_cmd_get_num_params -----------------------------------
  int cli_cmd_get_num_params(cli_cmd_t * cmd);
  // ----- cli_cmd_get_param_at -------------------------------------
  cli_param_t * cli_cmd_get_param_at(cli_cmd_t * cmd,
				     unsigned int index);
  // ----- cli_cmd_add_option ---------------------------------------
  int cli_cmd_add_option(cli_cmd_t * cmd,
			 char * name,
			 FCliCheckParam fCheckParam);
  // ----- _cli_cmd_match -------------------------------------------
  int cli_cmd_match(cli_t * cli, cli_cmd_t * cmd, char * start_cmd,
		    char * end_cmd, void ** ctx_ref);


  ///////////////////////////////////////////////////////////////////
  // CLI MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // ----- cli_create -----------------------------------------------
  cli_t * cli_create();
  // ----- cli_destroy ----------------------------------------------
  void cli_destroy(cli_t ** cli_ref);
  // ----- cli_set_exit_callback ------------------------------------
  void cli_set_exit_callback(cli_t * cli, FCliExitOnError fExitOnError);
  // -----[ cli_set_param_lookup ]-----------------------------------
  void cli_set_param_lookup(cli_t * cli, param_lookup_t lookup);
  // ----- cli_execute_ctx ------------------------------------------
  int cli_execute_ctx(cli_t * cli, const char * cmd,
		      void * ctx);
  // ----- cli_execute ----------------------------------------------
  int cli_execute(cli_t * cli, const char * cmd);
  // ----- cli_register_cmd -----------------------------------------
  int cli_register_cmd(cli_t * cli, cli_cmd_t * cmd);
  // ----- cli_perror -----------------------------------------------
  void cli_perror(gds_stream_t * stream, int error);
  // ----- cli_strerror -----------------------------------------------
  const char * cli_strerror(int error);
  // ----- cli_execute_file -----------------------------------------
  int cli_execute_file(cli_t * cli, FILE * stream);
  // ----- cli_execute_line -----------------------------------------
  int cli_execute_line(cli_t * cli, const char * line);
  // ----- cli_get_cmd_context --------------------------------------
  cli_cmd_t * cli_get_cmd_context(cli_t * cli);
  // ----- cli_get_error_details ------------------------------------
  int cli_get_error_details(cli_t * cli, cli_error_t * error);
  // ----- cli_set_user_error -----------------------------------------
  void cli_set_user_error(cli_t * cli, const char * format, ...);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_H__ */
