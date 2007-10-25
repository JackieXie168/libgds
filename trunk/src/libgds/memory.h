// ==================================================================
// @(#)memory.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 16/01/2007
// ==================================================================

#ifndef __LIBGDS_MEMORY_H__
#define __LIBGDS_MEMORY_H__

#include <stdlib.h>

#include <libgds/types.h>

#define MALLOC(s) memalloc(s, __FILE__, __LINE__)
#define REALLOC(p, s) memrealloc(p, s, __FILE__, __LINE__)
#define FREE(p) memfree(p, __FILE__, __LINE__)

#ifdef __cplusplus
extern "C" {
#endif

  // ----- memalloc -------------------------------------------------
  void * memalloc(size_t size, char * pcFileName, int iLineNumber);
  // ----- memrealloc -----------------------------------------------
  void * memrealloc(void * pPtr, size_t size,
		    char * pcFileName, int iLineNumber);
  // ----- memfree --------------------------------------------------
  void memfree(void * pPtr, char * pcFileName, int iLineNumber);
  // ----- mem_alloc_cnt --------------------------------------------
  long int mem_alloc_cnt();
  // ----- mem_flag_set ---------------------------------------------
  void mem_flag_set(uint8_t uFlag, int iState);
  // ----- mem_flag_get ---------------------------------------------
  int mem_flag_get(uint8_t uFlag);
  
  // ----- _memory_init ---------------------------------------------
  void _memory_init();
  // ----- _memory_destroy ------------------------------------------
  void _memory_destroy();

#ifdef __cplusplus
}
#endif

#endif /* __LIBGDS_MEMORY_H__ */
