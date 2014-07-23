// ==================================================================
// @(#)cli_fsm.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 02/12/2008
// $Id$
// ==================================================================

/**
 * \file
 * Provide data structures and functions to manage the CLI Finite
 * State Machine. This is for internal purpose only.
 */

#ifndef __GDS_CLI_FSM_H__
#define __GDS_CLI_FSM_H__

#include <libgds/cli.h>

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ cli_fsm_create ]-----------------------------------------
  /**
   * Create a CLI FSM instance.
   */
  cli_fsm_t * cli_fsm_create();

  // -----[ cli_fsm_destroy ]----------------------------------------
  /**
   * Free a CLI FSM instance.
   *
   * \param fsm_ref is a pointer to the FSM to be destroyed.
   */
  void cli_fsm_destroy(cli_fsm_t ** fsm_ref);

  // -----[ cli_fsm_run ]--------------------------------------------
  /**
   * Process a line of text using a CLI FSM.
   *
   * \param cli   is the CLI to be used.
   * \param cmd   is the base command.
   * \param line  is the line to process.
   * \param compl is an optional completion text.
   * \param elem  is the resulting CLI element (command/argument/option).
   * \retval an error code.
   */
  int cli_fsm_run(cli_t * cli, cli_cmd_t * cmd, const char * line,
		  const char * compl, cli_elem_t * elem);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_FSM_H__ */
