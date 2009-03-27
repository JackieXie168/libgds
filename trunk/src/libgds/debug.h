// ==================================================================
// @(#)debug.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 02/12/2008
// $Id$
// ==================================================================

#ifndef __GDS_DEBUG_H__
#define __GDS_DEBUG_H__

#include <stdarg.h>
#include <stdio.h>

// -----[ debug ]----------------------------------------------------
#ifdef DEBUG
static int __debug_enabled= 1;
#endif /* DEBUG */

static inline void __debug(const char * fmt, ...)
{
#ifdef DEBUG
  va_list ap;

  if (!__debug_enabled)
    return;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
#endif /* DEBUG */
}

#endif /* __GDS_DEBUG_H__ */
