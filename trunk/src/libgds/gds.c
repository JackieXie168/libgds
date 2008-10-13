// ==================================================================
// @(#)gds.c
//
// Generic Data Structures library.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/05/2005
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libgds/gds.h>
#include <libgds/stream.h>
#include <libgds/memory.h>
#include <libgds/memory_debug.h>
#include <libgds/patricia-tree.h>

// -----[ gds_init ]-------------------------------------------------
/**
 * Initialize the GDS library. This is a replacement for all the .ctor
 * functions that were used in the previous versions of libgds. This
 * should fix a number of linking problems encountered under the
 * Solaris environment.
 */
void gds_init(uint8_t options)
{
  mem_flag_set(MEM_FLAG_TRACK_LEAK, (options & GDS_OPTION_MEMORY_DEBUG));
  _memory_init();
  _stream_init();
  _patricia_tree_init();
}

// -----[ gds_destroy ]-------------------------------------------------
/**
 *
 */
void gds_destroy()
{
  _stream_destroy();
  _memory_destroy();
}

// -----[ gds_version ]----------------------------------------------
const char * gds_version()
{
  return PACKAGE_VERSION;
}
