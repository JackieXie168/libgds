// ==================================================================
// @(#)gds.c
//
// Generic Data Structures library.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 17/05/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libgds/log.h>
#include <libgds/memory.h>
#include <libgds/patricia-tree.h>

// -----[ gds_init ]-------------------------------------------------
/**
 * Initialize the GDS library. This is a replacement for all the .ctor
 * functions that were used in the previous versions of libgds. This
 * should fix a number of linking problems encountered under the
 * Solaris environment.
 */
void gds_init()
{
  _memory_init();
  _log_init();
  _patricia_tree_init();
}

// -----[ gds_destroy ]-------------------------------------------------
/**
 *
 */
void gds_destroy()
{
  _log_destroy();
  _memory_destroy();
}
