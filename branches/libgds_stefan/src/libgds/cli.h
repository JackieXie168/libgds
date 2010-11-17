// ==================================================================
// @(#)cli.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structures and functions to manage an interactive
 * command-line interface (CLI). This CLI can work in conjunction
 * with the GNU readline library to provide a highly interactive
 * command-line interface with auto-completion.
 *
 * Here is a typical example of a small CLI setup. This CLI will
 * understand 3 different commands: "quit", "version" and "foo". The
 * "foo" command requires a single argument named "bar".
 *
 * \code
 * cli_t * cli= cli_create();
 * cli_cmd_t * cmd;
 * cli_cmd_t * root= cli_get_root_cmd(cli);
 * cli_add_cmd(root, cli_cmd("quit", _cli_quit));
 * cli_add_cmd(root, cli_cmd("version", _cli_version));
 * cmd= cli_add_cmd(root, cli_cmd("foo", _cli_foo));
 * cli_add_arg(cmd, cli_arg("bar", NULL));
 * cli_execute_stream(stdin);
 * \endcode
 *
 * The typical implementation of a command can be done as follows.
 * The function will be called when the command "foo" is invoked
 * with the required number of arguments. The function will retrieve
 * the argument value and print it to the standard output. The
 * function will also return the error code CLI_SUCCESS to indicate
 * that its execution was successful.
 *
 * \code
 * static int _cli_foo(cli_ctx_t * ctx, cli_cmd_t * cmd)
 * {
 *   const char * arg_bar= cli_get_arg_value(cmd, 0);
 *   fprintf(stdout, "foo executed with arg \"bar\"=\"%s\"\n", arg_bar);
 *   fflush(stdout);
 *   return CLI_SUCCESS;
 * }
 * \endcode
 *
 * It is also possible to add groups of commands in the CLI commands
 * tree. Here is a simple example that will react to the following
 * commands: "grp cmd1" and "grp cmd2". It also allows to "enter" the
 * group and further type "cmd1" or "cmd2" to invoke the registered
 * commands.
 *
 * \code
 * cli_cmd_t * grp= cli_add_cmd(root, cli_cmd_group("grp"));
 * cli_add_cmd(grp, cli_cmd_group("cmd1", _cli_cmd1));
 * cli_add_cmd(grp, cli_cmd_group("cmd2", _cli_cmd2));
 * \endcode
 *
 * The next step is to associate user-data to a group. The child
 * commands of this group can then rely on the group's user-data for
 * their execution. This is called contexts and this is created as
 * follows:
 *
 * \code
 * cli_cmd_t * ctx= cli_add_cmd(root, cli_cmd_ctx("ctx",
 *                                                _cli_ctx_create,
 *                                                _cli_ctx_destroy));
 * cli_add_arg(ctx, cli_arg("name"));
 * cli_add_cmd(ctx, cli_cmd("cmd1", _cli_cmd1));
 * cli_add_cmd(ctx, cli_cmd("cmd2", _cli_cmd2));
 * \endcode
 *
 * The context command needs two different functions to create
 * context user-data when the context is entered and to destroy
 * this user-data (if needed) when the context is exited. In the
 * following example, the "ctx" context will simply push onto the
 * context a copy of the name passed as an argument. It will then
 * free this name when the context is freed.
 * \attention
 * It would be incorrect to push the argument value onto the context
 * without performing a copy as the argument value would not be valid
 * for the execution of the subsequent commands.
 *
 * \code
 * // -----[ _cli_ctx_create ]---------------------------------------
 * static int _cli_ctx_create(cli_ctx_t * ctx, cli_cmd_t * cmd,
 *                            void ** user_data_ref)
 * {
 *   const char * arg_name= cli_get_arg_value(cmd, 0);
 *   *user_data= strdup(arg_name);
 *   return CLI_SUCCESS;
 * }
 *
 * // -----[ _cli_ctx_destroy ]--------------------------------------
 * static void _cli_ctx_destroy(void ** user_data_ref)
 * {
 *   free(*user_data_ref);
 * }
 * \endcode
 *
 * The child commands can then access the context data as shown in
 * the following example:
 *
 * \code
 * // -----[ _cli_cmd1 ]---------------------------------------------
 * static int _cli_cmd1(cli_ctx_t * ctx, cli_cmd_t * cmd)
 * {
 *   const char * name= (char *) cli_context_get_at_top(ctx);
 *   fprintf(stdout, "execution of \"cmd1\" with context \"%s\"\n", name);
 *   fflush(stdout);
 *   return CLI_SUCCESS;
 * }
 * \endcode
 */

#ifndef __GDS_CLI_H__
#define __GDS_CLI_H__

#include <stdio.h>

#include <libgds/array.h>
#include <libgds/cli_types.h>
#include <libgds/stream.h>

#define CLI_MAX_LINE_LENGTH 1024

#ifdef __cplusplus
extern "C" {
#endif
  
  ///////////////////////////////////////////////////////////////////
  // COMMANDS MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////
  
  // -----[ cli_matching_subcmds ]-----------------------------------
  cli_cmds_t * cli_matching_subcmds(cli_t * cli, cli_cmd_t * cmd,
				    const char * text, int en_omni);
  // -----[ cli_matching_opts ]--------------------------------------
  cli_args_t * cli_matching_opts(cli_cmd_t * cmd, const char * text);


  ///////////////////////////////////////////////////////////////////
  // CLI TREE MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // -----[ cli_cmd ]------------------------------------------------
  /**
   * Create a command.
   *
   * The created command cannot have children.
   *
   * \param name    is the name of the command.
   * \param command is the callback function that implements the
   *   command.
   * \retval the command.
   */
  cli_cmd_t * cli_cmd(const char * name, cli_command_f command);

  // -----[ cli_cmd_ctx ]------------------------------------------
  /**
   * Create a context command.
   *
   * \param name        is the name of the command.
   * \param ctx_create  is the context creation callback.
   * \param ctx_destroy is the context destruction callback.
   * \retval the command.
   */
  cli_cmd_t * cli_cmd_ctx(const char * name,
			  cli_ctx_create_f ctx_create,
			  cli_ctx_destroy_f ctx_destroy);

  // -----[ cli_cmd_group ]------------------------------------------
  /**
   * Create a group of commands.
   *
   * \param name is the name of the group.
   * \retval the group.
   */
  cli_cmd_t * cli_cmd_group(const char * name);

  // -----[ cli_cmd_prefix ]-----------------------------------------
  /**
   * Create a prefix of commands.
   *
   * \param name is the name of the prefix.
   * \retval the prefix.
   */
  cli_cmd_t * cli_cmd_prefix(const char * name);

  // -----[ cli_add_cmd ]--------------------------------------------
  /**
   * Add a child to a command.
   *
   * \param cmd     is the parent command.
   * \param sub_cmd is the child command.
   * \retval the child command.
   */
  cli_cmd_t * cli_add_cmd(cli_cmd_t * cmd, cli_cmd_t * sub_cmd);

  // -----[ cli_add_arg ]--------------------------------------------
  /**
   * Add an argument to a command.
   *
   * \param cmd is the command.
   * \param arg is the argument.
   * \retval the argument.
   */
  cli_arg_t * cli_add_arg(cli_cmd_t * cmd, cli_arg_t * arg);

  // -----[ cli_add_opt ]--------------------------------------------
  /**
   * Add an option to a command.
   *
   * \param cmd is the command.
   * \param opt is the option.
   * \retval the option.
   */
  cli_arg_t * cli_add_opt(cli_cmd_t * cmd, cli_arg_t * opt);


  ///////////////////////////////////////////////////////////////////
  // CLI MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // -----[ cli_create ]---------------------------------------------
  /**
   * Create a CLI.
   */
  cli_t * cli_create();

  // -----[ cli_destroy ]--------------------------------------------
  /**
   * Destroy a CLI.
   */
  void cli_destroy(cli_t ** cli_ref);

  // -----[ cli_get_root_cmd ]---------------------------------------
  /**
   * Return the CLI's root command.
   *
   * The root command cannot be executed. It is the root of the CLI
   * commands tree.
   */
  cli_cmd_t * cli_get_root_cmd(cli_t * cli);

  // -----[ cli_get_omni_cmd ]---------------------------------------
  /**
   * Return the CLI's omni command.
   *
   * The omni command cannot be executed. It is the root of the CLI
   * omnipresent commands tree.
   */
  cli_cmd_t * cli_get_omni_cmd(cli_t * cli);

  // -----[ cli_set_on_error ]---------------------------------------
  void cli_set_on_error(cli_t * cli, cli_on_error_f on_error);
  // -----[ cli_set_param_lookup ]-----------------------------------
  void cli_set_param_lookup(cli_t * cli, param_lookup_t lookup);

  // -----[ cli_set_user_data ]----------------------------------------
  /**
   * Set the base CLI context.
   *
   * The provided context can be used by the implementation of
   * commands.
   */
  void cli_set_user_data(cli_t * cli, void * user_data);


  ///////////////////////////////////////////////////////////////////
  // CLI ERROR MANAGEMENT
  ///////////////////////////////////////////////////////////////////

  // -----[ cli_perror ]---------------------------------------------
  /**
   * Print an error message.
   *
   * \param stream is the output stream.
   * \param error  is the error code.
   */
  void cli_perror(gds_stream_t * stream, cli_error_type_t error);

  // -----[ cli_strerror ]-------------------------------------------
  /**
   * Provide an error message from an error code.
   *
   * \param error is the error code.
   * \retval corresponding error message.
   */
  const char * cli_strerror(cli_error_type_t error);

  // -----[ cli_get_error_details ]----------------------------------
  int cli_get_error_details(cli_t * cli, cli_error_t * error);

  // -----[ cli_set_user_error ]-------------------------------------
  /**
   * Set a user error message.
   *
   * This function can be used in user defined commands to provide
   * a more comprehensive error message.
   *
   * \attention
   * This function should only be used in the commands callback
   * functions.
   *
   * \param cli    is the CLI.
   * \param format is the message and variable argument list
   *   specifier (with a printf like format).
   */
  void cli_set_user_error(cli_t * cli, const char * format, ...);

  // -----[ cli_dump_error ]-----------------------------------------
  void cli_dump_error(gds_stream_t * stream, cli_t * cli);


  ///////////////////////////////////////////////////////////////////
  // CLI EXECUTION / COMPLETION
  ///////////////////////////////////////////////////////////////////

  // -----[ cli_execute ]--------------------------------------------
  /**
   * Execute a single line.
   *
   * \param cli  is the CLI.
   * \param str is the line to interpret.
   * \retval the execution status code.
   */
  int cli_execute(cli_t * cli, const char * str);

  // -----[ cli_complete ]-------------------------------------------
  int cli_complete(cli_t * cli, const char * str, const char * compl,
		   cli_elem_t * ctx);

  // -----[ cli_execute_stream ]-------------------------------------
  /**
   * Execute commands from a stream.
   *
   * \param cli    is the CLI.
   * \param stream is the input stream.
   * \retval the execution status code.
   */
  int cli_execute_stream(cli_t * cli, FILE * stream);

  // -----[ cli_execute_line ]---------------------------------------
  int cli_execute_line(cli_t * cli, const char * line);
  // -----[ cli_get_cmd_context ]------------------------------------
  void cli_get_cmd_context(cli_t * cli, cli_cmd_t ** cmd,
			   void ** user_data);


  ///////////////////////////////////////////////////////////////////
  // CLI ARGUMENTS / OPTIONS RUN-TIME ACCESS
  ///////////////////////////////////////////////////////////////////

  // -----[ cli_get_arg_value ]--------------------------------------
  /**
   * Get the value of a command's argument.
   *
   * \param cmd   is the command.
   * \param index is the argument's index.
   * \retval the argument's value.
   */
  const char * cli_get_arg_value(const cli_cmd_t * cmd,
				 unsigned int index);

  // -----[ cli_get_arg_num_values ]-----------------------------------
  /**
   * Get the number of values of command's argument.
   *
   * \param cmd   is the command.
   * \param index is the argument's index.
   * \retval the number of values. This should be 1 for standard
   *   arguments and possibly more than 1 for varargs.
   */
  unsigned int cli_get_arg_num_values(const cli_cmd_t * cmd,
				      unsigned int index);
  
  // -----[ cli_get_opt_value ]--------------------------------------
  /**
   * Get the value of a command's option.
   *
   * \param cmd  is the command.
   * \param name is the option's name.
   * \retval the option's value.
   */
  const char * cli_get_opt_value(const cli_cmd_t * cmd,
				 const char * name);

  // -----[ cli_has_opt_value ]--------------------------------------
  /**
   * Test if a command's option is present.
   *
   * \param cmd is the command.
   * \param name is the option's name.
   * \retval 0 if the option is not present,
   *   or != 0 if the option is present.
   */
  int cli_has_opt_value(const cli_cmd_t * cmd, const char * name);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_H__ */
