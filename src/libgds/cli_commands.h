// ==================================================================
// @(#)cli_commands.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

/**
 * \file
 * Provide functions to manage CLI commands. Most of these functions
 * are for internal use only.
 */

#ifndef __GDS_CLI_COMMANDS_H__
#define __GDS_CLI_COMMANDS_H__

#include <libgds/array.h>
#include <libgds/cli_types.h>
#include <libgds/stream.h>

// -----[ cli_cmds_add ]---------------------------------------------
/**
 * Add a command to a list of commands.
 */
static inline
int cli_cmds_add(cli_cmds_t * cmds, cli_cmd_t * cmd)
{
  return ptr_array_add((ptr_array_t *) cmds, &cmd);
}

// -----[ cli_cmds_num ]---------------------------------------------
/**
 * Get the number of commands in a list of commands.
 */
static inline
int cli_cmds_num(cli_cmds_t * cmds)
{
  if (cmds == NULL)
    return 0;
  return ptr_array_length(cmds);
}

// -----[ cli_cmds_destroy ]-----------------------------------------
/**
 * Destroy a list of commands.
 *
 * NOTE: all the commands in the list are also freed thanks to the
 * underlying array's destroy command. It is also safe to use this
 * function on lists returned by the cli_matching_cmds function since
 * in this case, the underlying array's destroy function is NULL.
 */
static inline
void cli_cmds_destroy(cli_cmds_t ** cmds_ref)
{
  ptr_array_destroy((ptr_array_t **) cmds_ref);
}

// -----[ cli_cmds_at ]----------------------------------------------
/**
 * Get a command from a list of commands.
 *
 * \param cmds  is the list of commands.
 * \param index is the position of the requested command.
 * \retval the requested command.
 *
 * \pre \p index < size(\p cmds)
 */
static inline
struct cli_cmd_t * cli_cmds_at(cli_cmds_t * cmds, unsigned int index)
{
  return (struct cli_cmd_t *) cmds->data[index];
}

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ cli_cmds_create ]----------------------------------------
  /**
   * Create a list of commands.
   *
   * The commands in the list are sorted based on lexicographic
   * order.
   */
  cli_cmds_t * cli_cmds_create();

  // -----[ cli_cmds_create_ref ]------------------------------------
  /**
   * Create a list of references to commands.
   *
   * The difference compared to \c cli_cmds_create is that the list
   * has no item destructor. This means that when this list is
   * disposed, the reference commands will not be freed.
   */
  cli_cmds_t * cli_cmds_create_ref();

  // -----[ cli_cmd_create ]-------------------------------------------
  /**
   * \internal
   * Create a CLI command.
   *
   * \param type
   *   is the command type.
   * \param name
   *   is the command name.
   * \param ops
   *   is the set callback functions that implement the command's
   *   behaviour.
   * \param sub_cmds
   *   is the list of sub-commands.
   * \retval the newly created command.
   */
  cli_cmd_t * cli_cmd_create(cli_cmd_type_t type,
			     const char * name,
			     cli_cmd_ops_t ops,
			     cli_cmds_t * sub_cmds);

  // -----[ cli_cmd_destroy ]----------------------------------------
  /**
   * \internal
   * Destroy a CLI command.
   *
   * \param cmd_ref is a pointer to the command to be destroyed.
   */
  void cli_cmd_destroy(cli_cmd_t ** cmd_ref);

  // -----[ cli_cmd_find_submd ]-------------------------------------
  /**
   * Find a child of a command.
   *
   * \param cmd  is the parent command.
   * \param name is the name of the searched child command.
   * \retval the searched command if it exists,
   *   or NULL if it does not exist.
   */
  cli_cmd_t * cli_cmd_find_subcmd(cli_cmd_t * cmd, const char * name);

  // -----[ cli_cmd_dump ]-------------------------------------------
  /**
   * Dump the definition of a command.
   *
   * \param stream  is the output stream.
   * \param prefix  is a prefix to print before each line of output.
   * \param cmd     is the command to be dumped.
   * \param recurse tells if the sub-commands must be dumped
   *   recursively.
   */
  void cli_cmd_dump(gds_stream_t * stream, char * prefix,
		    cli_cmd_t * cmd, int recurse);

  // -----[ cli_cmd_enter ]------------------------------------------
  /**
   * \internal
   * Enter a command context/group.
   */
  int cli_cmd_enter(cli_t * cli, cli_cmd_t * cmd, int final);

  // -----[ cli_cmd_exec ]-------------------------------------------
  /**
   * \internal
   * Execute a command.
   */
  int cli_cmd_exec(cli_t * cli, cli_cmd_t * cmd);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_COMMANDS_H__ */
