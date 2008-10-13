// ==================================================================
// @(#)gds.h
//
// Generic Data Structures library.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/05/2005
// $Id$
// ==================================================================

#ifndef __GDS_H__
#define __GDS_H__

#include <libgds/types.h>

#define GDS_OPTION_MEMORY_DEBUG 0x01

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ gds_init ]-------------------------------------------------
  void gds_init(uint8_t options);
  // -----[ gds_destroy ]-------------------------------------------------
  void gds_destroy();
  // -----[ gds_version ]----------------------------------------------
  const char * gds_version();

#ifdef __cplusplus
}
#endif

#endif /* __GDS_H__ */
