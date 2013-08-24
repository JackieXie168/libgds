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

#include <stdarg.h>

#include <libgds/gds.h>
#include <libgds/stream.h>
#include <libgds/memory.h>
#include <libgds/memory_debug.h>
#include <libgds/trie.h>

// -----[ gds_init ]-------------------------------------------------
void gds_init(uint8_t options)
{
  mem_flag_set(MEM_FLAG_TRACK_LEAK, (options & GDS_OPTION_MEMORY_DEBUG));
  _memory_init();
  _stream_init();
  _trie_init();
}

// -----[ gds_destroy ]-------------------------------------------------
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

// -----[ gds_fatal ]------------------------------------------------
void gds_fatal(const char * msg, ...)
{
  va_list ap;

  va_start(ap, msg);
  fprintf(stderr, "GDS FATAL ERROR: ");
  vfprintf(stderr, msg, ap);
  va_end(ap);
  fflush(stderr);
  abort();
}

// -----[ gds_warn ]------------------------------------------------
void gds_warn(const char * msg, ...)
{
  va_list ap;

  va_start(ap, msg);
  fprintf(stderr, "GDS WARNING: ");
  vfprintf(stderr, msg, ap);
  fflush(stderr);
  va_end(ap);
}
