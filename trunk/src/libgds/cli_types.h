// ==================================================================
// @(#)cli_types.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 02/12/2008
// $Id$
// ==================================================================

/**
 * \file
 * Definition of data structures and error codes used by the CLI.
 */

#ifndef __GDS_CLI_TYPES_H__
#define __GDS_CLI_TYPES_H__

#include <libgds/tokenizer.h>
#include <libgds/stack.h>

// -----[ Error codes ]----------------------------------------------
/**
 * These are the possible CLI error codes.
 */
typedef enum {
  /** Execution was successful. */
  CLI_SUCCESS                = 0,
  /** Generic error (no details). */
  CLI_ERROR_GENERIC          = -1,
  /** Error detected, cause unknown. */
  CLI_ERROR_UNEXPECTED       = -2,
  /** Command does not exist in context. */
  CLI_ERROR_UNKNOWN_CMD      = -3,
  /** Command needs more parameters. */
  CLI_ERROR_MISSING_ARG      = -4,
  /** Command cannot be executed. */
  CLI_ERROR_NOT_A_CMD        = -5,
  /** Command returned an error. */
  CLI_ERROR_CMD_FAILED       = -6,
  /** Invalid parameter value. */
  CLI_ERROR_BAD_ARG_VALUE    = -7,
  /** Context creation failed. */
  CLI_ERROR_CTX_CREATE       = -8,
  /** Too many parameters provided. */
  CLI_ERROR_TOO_MANY_ARGS    = -9,
  /** Option does not exist in command. */
  CLI_ERROR_UNKNOWN_OPT      = -10,
  /** Option needs value. */
  CLI_ERROR_MISSING_OPT_VALUE= -11,
  /** Option does not need value. */
  CLI_ERROR_OPT_NO_VALUE     = -12,
  /** Syntax error. */
  CLI_ERROR_SYNTAX           = -13,
  /** Completion could not be performed. */
  CLI_ERROR_COMPL_FAILED     = -14,
  /** Input line was too long. */
  CLI_ERROR_LINE_TOO_LONG    = -15,
  /** Terminate the CLI processing. */
  CLI_SUCCESS_TERMINATE      = 1,
} cli_error_type_t;
#define CLI_ERROR_COMMAND_FAILED CLI_ERROR_CMD_FAILED

typedef ptr_array_t cli_cmds_t;
struct cli_t;
struct cli_cmd_t;

typedef struct {
  gds_stack_t  * cmd_stack;   // Stack of commands
  unsigned int   saved_depth;
  char         * string;
} cli_ctx_t;

struct cli_arg_t;

// -----[ cli_arg_check_f ]------------------------------------------
/** Argument value check function.
 * \param value is the value to be checked.
 * \retval 0 if the value is accepted,
 *   or -1 otherwise.
 */
typedef int    (*cli_arg_check_f)(const char * value);

// -----[ cli_arg_enum_f ]-------------------------------------------
/** Argument values enumeration function.
 * \param text is a value prefix to filter enumerated values.
 * \param state is an integer value (0 is for initializing the
 *   generator, >0 for subsequent calls).
 * \retval NULL value if no more matching value,
 *   or non-NULL value otherwise.
 *
 * Example:
 * \code
 * static char * _cli_arg_enum(const char * text, int state) {
 *   static const char * values= {"on", "off"};
 *   static index;
 *   if (state == 0)
 *     index= 0;
 *   while (index < 2) {
 *     if (strncmp(text, state, strlen(text)))
 *       continue;
 *     index++;
 *   return strdup(values[state]);
 * }
 * \endcode
 */
typedef char * (*cli_arg_enum_f) (const char * text, int state);

typedef ptr_array_t cli_args_t;
typedef cli_args_t cli_opts_t;
typedef enum {
  CLI_ARG_TYPE_STD, // Single argument
  CLI_ARG_TYPE_VAR, // Variable number of arguments
  CLI_ARG_TYPE_OPT, // Option
} cli_arg_type_t;

typedef struct {
  cli_arg_check_f check;
  cli_arg_enum_f  enumerate;
} cli_arg_ops_t;

/**
 * Definition of a CLI argument or option.
 */
typedef struct cli_arg_t {
  char           * name;
  cli_arg_type_t   type;
  uint8_t          max_args;
  uint8_t          present;
  uint8_t          need_value;
  cli_arg_ops_t    ops;
  union {
    char         * value;
    gds_tokens_t * values;
  };
} cli_arg_t;

/** Implementation of a CLI context command.
 * \param ctx      is the current CLI context stack.
 * \param cmd      is the command being invoked.
 * \param item_ref is a pointer to the user-provided context data.
 * \retval an error code (either CLI_SUCCESS or CLI_ERROR_CMD_FAILED).
 */
typedef int  (*cli_ctx_create_f) (cli_ctx_t * ctx,
				  struct cli_cmd_t * cmd,
				  void ** item_ref);

/** Removal of CLI context's user-data.
 * \param item_ref is a pointer to the user-provided context data.
 */
typedef void (*cli_ctx_destroy_f)(void ** item_ref);

/** Implementation of a CLI command.
 * \param ctx is the current CLI context stack.
 * \param cmd is the command being invoked.
 * \retval an error code (either CLI_SUCCESS or CLI_ERROR_CMD_FAILED).
 */
typedef int  (*cli_command_f)    (cli_ctx_t * ctx,
				  struct cli_cmd_t * cmd);

typedef int  (*cli_on_error_f)   (struct cli_t * cli, int result);

typedef enum {
  CLI_CMD_TYPE_STD, // Simple command
  CLI_CMD_TYPE_CTX, // Command that creates context
  CLI_CMD_TYPE_GRP, // Group of commands
  CLI_CMD_TYPE_PFX, // Command prefix (not executable)
} cli_cmd_type_t;

typedef struct cli_cmd_ops_t {
  cli_ctx_create_f  ctx_create;
  cli_ctx_destroy_f ctx_destroy;
  cli_command_f     command;
} cli_cmd_ops_t;

/**
 * Definition of a CLI command.
 */
typedef struct cli_cmd_t {
  cli_cmd_type_t     type;
  char             * name;
  cli_cmds_t       * sub_cmds;
  cli_args_t       * args;
  struct cli_cmd_t * parent;
  cli_opts_t       * opts;
  cli_cmd_ops_t      ops;
  char             * help;
  int                hidden;
} cli_cmd_t;

typedef enum {
  CLI_ELEM_CMD,
  CLI_ELEM_ARG,
  CLI_ELEM_UNKNOWN
} cli_elem_type_t;

typedef struct cli_elem_t {
  cli_elem_type_t type;
  union {
    cli_cmd_t * cmd;
    cli_arg_t * arg;
  };
} cli_elem_t ;

typedef struct {
  void      * user_data;     // User data
  cli_cmd_t * cmd;           // Saved command
  int         final;
} _cli_ctx_item_t;

typedef struct {
  int          error;       // Last error code
  int          line_number; // Line number of last command
  const char * source;
  int          user_error;  // Last application error code
                            // (has a meaning only if last command failed)
  char       * user_msg;    // Last application error message
                            // (has a meaning only if last command failed)
} cli_error_t;

typedef struct {
  cli_on_error_f on_error; // called on error
} cli_ops_t;

typedef struct cli_fsm_t {
  gds_tokenizer_t    * tokenizer;
  const gds_tokens_t * tokens;
  unsigned int         token_index;
  int                  state;
  int                  consume;
  cli_cmd_t          * cmd;
  unsigned int         param_index;
  unsigned int         param_min;
  unsigned int         param_max;
  char               * token_copy;
  cli_elem_t           elem;        // Element returned for completion
} cli_fsm_t;

/**
 * Definition of a CLI.
 *
 * This data structure maintains the trees of commands, the CLI
 * state-machine, error codes and CLI context.
 */
typedef struct cli_t {
  void            * user_data;
  cli_cmd_t       * root_cmd;
  cli_cmd_t       * omni_cmd;
  cli_ctx_t       * ctx;       // Current execution context (stack)
  cli_ops_t         ops;
  cli_fsm_t       * fsm;
  cli_error_t       error;
} cli_t;

#endif /* __GDS_CLI_TYPES_H__ */
