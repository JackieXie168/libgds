// ==================================================================
// @(#)types.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 24/11/2002
// @lastdate 14/08/2003
// ==================================================================

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef __i386__
#include <inttypes.h>
#endif

#ifdef __sparc64__
#include <sys/types.h>
#endif

#include <limits.h>

#define MAX_UINT16_T 65536U
#define MAX_UINT32_T 4294967295U

#endif
