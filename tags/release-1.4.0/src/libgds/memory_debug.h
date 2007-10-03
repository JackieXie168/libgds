// ==================================================================
// @(#)memory_debug.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 04/01/2007
// @lastdate 04/01/2007
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

// -----[ memory_debug_track_alloc ]---------------------------------
extern inline void memory_debug_track_alloc(void * pNewPtr,
					    size_t size,
					    char * pcFileName,
					    int iLineNumber);
// -----[ memory_debug_track_realloc ]-------------------------------
extern inline void memory_debug_track_realloc(void * pNewPtr,
					      void * pPtr,
					      size_t size,
					      char * pcFileName,
					      int iLineNumber);
// -----[ memory_debug_track_free ]----------------------------------
extern inline void memory_debug_track_free(void * pPtr,
					   char * pcFileName,
					   int iLineNumber);
// -----[ memory_debug_init ]----------------------------------------
extern void memory_debug_init(int iTrack);
// -----[ memory_debug_destroy ]-------------------------------------
extern void memory_debug_destroy();

#endif /* __LIBGDS_MEMORY_DEBUG_H__ */
