// ==================================================================
// @(#)types.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 24/11/2002
// @lastdate 31/05/2004
// ==================================================================

#ifndef __TYPES_H__
#define __TYPES_H__

#include <libgds/config.h>

/* `inttypes.h' vs. `stdint.h' (quoting from the GNU autoconf manual)
 *
 * Paul Eggert notes that: ISO C 1999 says that `inttypes.h' includes
 * `stdint.h', so there's no need to include `stdint.h' separately in
 * a standard environment. Many implementations have `inttypes.h' but
 * not `stdint.h' (e.g., Solaris 7), but I don't know of any
 * implementation that has `stdint.h' but not `inttypes.h'. Nor do I
 * know of any free software that includes `stdint.h'; `stdint.h'
 * seems to be a creation of the committee. */
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#include <limits.h>

#define MAX_UINT16_T 65536U
#define MAX_UINT32_T 4294967295U

#endif
