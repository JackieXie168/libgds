// ==================================================================
// @(#)memory.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be), 
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 17/05/2005
// $Id$
// ==================================================================

#include <config.h>
#include <libgds/memory.h>

#include <libgds/memory_debug.h>

/* 'alloc_count' tracks the balance of memalloc/memfree calls. The
 * allocation count is initialized to -1 and re-initialized to 0 by
 * the global ctor function _memory_init. If memalloc detects that the
 * allocation counter is -1, this means that the ctor function has not
 * yet been called (problem during linking process ?) */
static long int _alloc_count= -1;
static uint8_t  _flags      = 0;

// -----[ _mem_alloc_count_inc ]-------------------------------------
void _mem_alloc_count_inc(const char * filename, int line_num)
{
  if (_alloc_count < 0)
    gds_fatal("memalloc: dtor function _memory_init has not yet been called\n"
	      "Check your linking process !!!\n");
}

// -----[ _mem_alloc_count_dec ]-------------------------------------
void _mem_alloc_count_dec(const char * filename, int line_num)
{
  if (_alloc_count <= 0)
    gds_warn("memfree: alloc-count == %ld : %s (line %d)\n",
	    _alloc_count, filename, line_num);
}

// -----[ _mem_alloc_count_get ]-------------------------------------
long int _mem_alloc_count_get()
{
  return _alloc_count;
}

// -----[ mem_flag_set ]---------------------------------------------
void mem_flag_set(uint8_t flag, int state)
{
  if (state)
    _flags|= state;
  else
    _flags&= ~state;
}

// -----[ mem_flag_get ]---------------------------------------------
int mem_flag_get(uint8_t flag)
{
  return (_flags & flag);
}


/////////////////////////////////////////////////////////////////////
// INITIALIZATION AND FINALIZATION FUNCTIONS
/////////////////////////////////////////////////////////////////////

// -----[ _memory_init ]---------------------------------------------
void _memory_init()
{
#ifdef GDS_MEMORY_DEBUG
  _alloc_count= 0;
  memory_debug_init(mem_flag_get(MEM_FLAG_TRACK_LEAK));
#endif /* GDS_MEMORY_DEBUG */
}

// -----[ _memory_destroy ]------------------------------------------
void _memory_destroy()
{
#ifdef GDS_MEMORY_DEBUG
  if (mem_flag_get(MEM_FLAG_WARN_LEAK) && (_alloc_count > 0))
    gds_warn("memory leak (%lu) !\n", _alloc_count);

  memory_debug_destroy();
#endif
}
