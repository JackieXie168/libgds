// ==================================================================
// @(#)memory_debug.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 04/01/2007
// @lastdate 20/12/2007
// ==================================================================

#ifndef __LIBGDS_MEMODY_DEBUG_H__
#define __LIBGDS_MEMORY_DEBUG_H__

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
  void memory_debug_track_alloc(void * pNewPtr, size_t size,
				char * pcFileName, int iLineNumber);
  // -----[ memory_debug_track_realloc ]-----------------------------
  void memory_debug_track_realloc(void * pNewPtr, void * pPtr,
				  size_t size, char * pcFileName,
				  int iLineNumber);
  // -----[ memory_debug_track_free ]--------------------------------
  void memory_debug_track_free(void * pPtr, char * pcFileName,
			       int iLineNumber);
  // -----[ memory_debug_init ]--------------------------------------
  void memory_debug_init(int iTrack);
  // -----[ memory_debug_destroy ]-----------------------------------
  void memory_debug_destroy();

#ifdef __cplusplus
}
#endif

#endif /* __LIBGDS_MEMORY_DEBUG_H__ */
