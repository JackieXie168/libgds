// ==================================================================
// @(#)gds.h
//
// Generic Data Structures library.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 10/08/2005
// ==================================================================

#ifndef __GDS_H__
#define __GDS_H__

#include <libgds/types.h>

#define GDS_OPTION_MEMORY_DEBUG 0x01

// -----[ gds_init ]-------------------------------------------------
extern void gds_init(uint8_t uOptions);
// -----[ gds_destroy ]-------------------------------------------------
extern void gds_destroy();

#endif /* __GDS_H__ */
