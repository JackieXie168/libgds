// ==================================================================
// @(#)memory_debug.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 04/01/2007
// $Id$
// ==================================================================

#ifndef __GDS_MEMODY_DEBUG_H__
#define __GDS_MEMORY_DEBUG_H__

#include <stdlib.h>

#define MEM_FLAG_WARN_LEAK  0x01 /* Display a warning in case of memory
				    leak when the memory.o object is
				    destroyed (note that memory.o must
				    be the last object in the .DTOR
				    list) */
#define MEM_FLAG_TRACK_LEAK 0x02

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ memory_debug_track_alloc ]-------------------------------
  void memory_debug_track_alloc(void * new_ptr, size_t size,
				const char * filename, int line_num);
  // -----[ memory_debug_track_realloc ]-----------------------------
  void memory_debug_track_realloc(void * new_ptr, void * ptr,
				  size_t size,
				  const char * filename, int line_num);
  // -----[ memory_debug_track_free ]--------------------------------
  void memory_debug_track_free(void * ptr, const char * filename,
			       int line_num);
  // -----[ memory_debug_init ]--------------------------------------
  void memory_debug_init(int track);
  // -----[ memory_debug_destroy ]-----------------------------------
  void memory_debug_destroy();

#ifdef __cplusplus
}
#endif

#endif /* __GDS_MEMORY_DEBUG_H__ */
