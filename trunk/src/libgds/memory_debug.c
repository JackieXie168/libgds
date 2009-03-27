// ==================================================================
// @(#)memory_debug.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/05/2005
// $Id$
// ==================================================================
// Implementation of a kind of garbage collector to detect the origin
// of some memory bugs. It can be used to debug two types of memory
// bugs:
//   1) memory leaks: at the end of the process, it displays all the
//      memory allocated by this library and not freed by
//      it. Furthermore, the file name and the line where the
//      allocation has been made is displayed too.
//   2) untracked allocations (freeing a memory space allocated by an
//      other way than the library): when this kind of memory space is
//      freed, the gc will notice that this part of memory hasn't been
//      allocated by this library. Furthermore, the file name and the
//      line where the free has been made is displayed too.
//
// As it is memory and CPU hungry, this part must be explicitly
// compiled into libgds by configuring the option
// '--enable-memory-debug'
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>

#include <libgds/memory_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgds/list.h>

#define MAX_ALLOCATION_TRACK 1000

typedef struct {
  void         * addr;
  unsigned int   size;
  char         * filename;
  int            line_num;
} _mem_alloc_t;

static gds_list_t * _alloc_list= NULL;
static struct {
  long int allocated;
  long int freed;
  long int largest_alloc;
} _stats= { 0, 0, 0 };

// -----[ _update_stats ]--------------------------------------------
static inline void _update_stats(unsigned int alloc_inc,
				 unsigned int free_inc)
{
  _stats.allocated+= alloc_inc;
  _stats.freed+= free_inc;
  if (_stats.largest_alloc < alloc_inc)
    _stats.largest_alloc= alloc_inc;
}

// -----[ _mem_dbg_alloc_init ]--------------------------------------
static inline
_mem_alloc_t * _mem_dbg_alloc_init(void * addr, unsigned int size, 
				   const char * filename, int line_num)
{
  _mem_alloc_t * alloc= malloc(sizeof(_mem_alloc_t));
  alloc->addr= addr;
  alloc->size= size;
  alloc->filename= strdup(filename);
  alloc->line_num= line_num;
  return alloc;
}

// -----[ _mem_dbg_alloc_cmp ]---------------------------------------
static int _mem_dbg_alloc_cmp(const void * item1, const void * item2)
{
  _mem_alloc_t * alloc1= (_mem_alloc_t *) item1;
  _mem_alloc_t * alloc2= (_mem_alloc_t *) item2;

  if (alloc1->addr < alloc2->addr)
    return -1;
  if (alloc1->addr > alloc2->addr)
    return 1;
  return 0;
}

// -----[ _mem_dbg_alloc_destroy ]-----------------------------------
static void _mem_dbg_alloc_destroy(void ** item_ref)
{
  _mem_alloc_t ** alloc_ref= (_mem_alloc_t **) item_ref;
  _mem_alloc_t * alloc= *alloc_ref;
  
  if (alloc == NULL)
    return;
  if (alloc->filename != NULL)
    free(alloc->filename);
  free(alloc);
  *alloc_ref= NULL;
}

// -----[ _mem_dbg_alloc_for_each ]----------------------------------
static int _mem_dbg_alloc_for_each(void * item, void * ctx)
{
  _mem_alloc_t * alloc= (_mem_alloc_t *) item;
  fprintf(stderr, "[%p] : %u bytes memory leak in %s (line %d)\n", 
	  alloc->addr, alloc->size, alloc->filename, alloc->line_num);
  fflush(stderr);
  return 0;
}

// -----[ memory_debug_track_alloc ]---------------------------------
void memory_debug_track_alloc(void * new_ptr,
			      size_t size,
			      const char * filename,
			      int line_num)
{
  _mem_alloc_t * alloc = NULL;

  if (_alloc_list == NULL)
    return;

  alloc= _mem_dbg_alloc_init(new_ptr, size, filename, line_num);
  if (list_add(_alloc_list, alloc) < 0) {
    fprintf(stderr, "allocation already made\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
  }
  _update_stats(size, 0);
}

// -----[ memory_debug_track_realloc ]-------------------------------
void memory_debug_track_realloc(void * new_ptr,
				void * ptr,
				size_t size,
				const char * filename,
				int line_num)
{
  _mem_alloc_t alloc= { .addr= ptr };
  _mem_alloc_t * alloc_searched= NULL;
  unsigned int index = 0;

  if (_alloc_list == NULL)
    return;
  
  if (list_index_of(_alloc_list, &alloc, &index) < 0) {
    fprintf(stderr, "[%p] : memory not allocated by MALLOC : %s (line %d)\n",
	    ptr, filename, line_num);
    exit(EXIT_FAILURE);
  }
  
  alloc_searched= list_get_at(_alloc_list, index);
  _update_stats(size, alloc_searched->size);
  assert(list_remove_at(_alloc_list, index) >= 0);
  assert(list_add(_alloc_list,
		  _mem_dbg_alloc_init(new_ptr, size,
				      filename, line_num)) >= 0);
}

// -----[ memory_debug_track_free ]----------------------------------
void memory_debug_track_free(void * ptr,
			     const char * filename,
			     int line_num)
{
  _mem_alloc_t alloc= { .addr= ptr };
  _mem_alloc_t * alloc_searched= NULL;
  unsigned int index= 0;

  if (_alloc_list == NULL)
    return;

  if (list_index_of(_alloc_list, &alloc, &index) < 0) {
    fprintf(stderr, "[%p] : memory not allocated by MALLOC : %s (line %d)\n",
	    ptr, filename, line_num);
    fflush(stderr);
  }
  alloc_searched= list_get_at(_alloc_list, index);
  _update_stats(0, alloc_searched->size);
  assert(list_remove_at(_alloc_list, index) >= 0);
}

// -----[ memory_debug_init ]----------------------------------------
void memory_debug_init(int track)
{
  if (track)
    _alloc_list= list_create(_mem_dbg_alloc_cmp,
			     _mem_dbg_alloc_destroy,
			     MAX_ALLOCATION_TRACK);
}

// -----[ memory_debug_destroy ]-------------------------------------
void memory_debug_destroy()
{
  if (_alloc_list == NULL)
    return;
  
  list_for_each(_alloc_list, _mem_dbg_alloc_for_each, NULL);
  list_destroy(&_alloc_list);
  fprintf(stderr, "total allocated bytes : %li\n", _stats.allocated);
  fprintf(stderr, "total freed bytes     : %li\n", _stats.freed);
  fprintf(stderr, "largest allocation    : %li\n", _stats.largest_alloc);
}
