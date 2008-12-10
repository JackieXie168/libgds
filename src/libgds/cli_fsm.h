// ==================================================================
// @(#)cli_fsm.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 02/12/2008
// $Id$
// ==================================================================

#ifndef __GDS_CLI_FSM_H__
#define __GDS_CLI_FSM_H__

#include <libgds/cli.h>

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ cli_fsm_create ]-----------------------------------------
  cli_fsm_t * cli_fsm_create();
  // -----[ cli_fsm_destroy ]----------------------------------------
  void cli_fsm_destroy(cli_fsm_t ** fsm_ref);
  // -----[ cli_fsm_run ]--------------------------------------------
  int cli_fsm_run(cli_t * cli, cli_cmd_t * cmd, const char * line,
		  const char * compl, cli_elem_t * elem);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_FSM_H__ */
