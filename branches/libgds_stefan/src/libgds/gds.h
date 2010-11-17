// ==================================================================
// @(#)gds.h
//
// Generic Data Structures library.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/05/2005
// $Id$
// ==================================================================

/**
 * \mainpage
 * This is the documentation for libGDS, a library of generic data
 * structures and functions intended for use in C programming.
 *
 * \authors
 * Bruno Quoitin (bruno.quoitin\@uclouvain.be)
 */

/**
 * \file
 * This file contains the main GDS initialization and finalization
 * functions. Before using the library, the gds_init function must
 * be used. Symmetrically, the gds_destroy function must be used after
 * the library has been used.
 */

#ifndef __GDS_H__
#define __GDS_H__

#include <libgds/types.h>

#define GDS_OPTION_MEMORY_DEBUG 0x01

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ gds_init ]-----------------------------------------------
  /**
   * Initialize the GDS library.
   *
   * This function must be called exactly once before any of the
   * library function is called.
   *
   * \internal NOTE:
   *   This is a replacement for all the .ctor functions that were
   *   used in the previous versions of libgds. This should fix a
   *   number of linking problems encountered under the Solaris
   *   environment.
   */
  void gds_init(uint8_t options);

  // -----[ gds_destroy ]--------------------------------------------
  /**
   * Finalize the GDS library.
   *
   * This function must be called exactly once when none of the
   * library function is needed anymore (end of program).
   */
  void gds_destroy();

  // -----[ gds_version ]--------------------------------------------
  /**
   * Return a string with the library version.
   */
  const char * gds_version();

  // -----[ gds_fatal ]----------------------------------------------
  /**
   * Abort with an error message.
   */
  void gds_fatal(const char * msg, ...);

  // -----[ gds_warn ]-----------------------------------------------
  /**
   * Display a warning message.
   */
  void gds_warn(const char * msg, ...);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_H__ */
