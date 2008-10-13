// ==================================================================
// @(#)stream_cmd.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/09/2008
// $Id$
// ==================================================================

#ifndef __GDS_STREAM_CMD_H__
#define __GDS_STREAM_CMD_H__

#include <libgds/stream.h>

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ stream_create_cmd ]--------------------------------------
  gds_stream_t * stream_create_cmd(const char * cmd);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_STREAM_CMD_H__ */
