// ==================================================================
// @(#)gds.h
//
// Generic Data Structures library.
//
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 17/05/2005
// $Id$
// ==================================================================

/**
 * \mainpage
 * This is the documentation for libGDS, a library of generic data
 * structures and functions intended for use in C programming.
 *
 * <b>Where to start ?</b>
 *
 * Before using any of the libGDS data structures, the library must be
 * initialized. Look at the functions defined in gds.h to learn how
 * initialization/finalization should be done.
 *
 * <b>Quick links to main data structures / features</b>
 *
 * \li Associative arrays : assoc_array.h
 * \li Dynamic arrays : array.h
 * \li Command-line interface : cli.h
 * \li Enumerations : enumerator.h
 * \li FIFO queues : fifi.h
 * \li Hash tables : hash.h and hash_utils.h
 * \li Lists : list.h
 * \li Parameter handling and parameter replacement : params.h
 * \li Sequences : sequence.h
 * \li Stacks : stack.h
 * \li Streams : stream.h
 * \li String tokenizer : tokenizer.h and tokens.h
 * \li String utilities : str_util.h
 * \li Unibit compressed tries : trie.h
 * \li Unit testing : utest.h
 *
 *
 * \authors
 * Bruno Quoitin (bruno.quoitin\@umons.ac.be)
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
