// ==================================================================
// @(#)enumerator.h
//
// List enumerator object.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/08/2005
// $Id$
// ==================================================================

#ifndef __GDS_ENUMERATOR_H__
#define __GDS_ENUMERATOR_H__

typedef int    (*FEnumeratorHasNext)(void * ctx);
typedef void * (*FEnumeratorGetNext)(void * ctx);
typedef void   (*FEnumeratorDestroy)(void * ctx);

typedef struct {
  FEnumeratorHasNext  has_next;
  FEnumeratorGetNext  get_next;
  FEnumeratorDestroy  destroy;
} enum_ops_t;

typedef struct {
  void       * ctx;
  enum_ops_t   ops;
} enum_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- enum_create ------------------------------------------------
  enum_t * enum_create(void * ctx,
		       FEnumeratorHasNext has_next,
		       FEnumeratorGetNext get_next,
		       FEnumeratorDestroy destroy);
  // ----- enum_destroy -----------------------------------------------
  void enum_destroy(enum_t ** enum_ref);
  
#ifdef __cplusplus
}
#endif

// ----- enum_has_next ----------------------------------------------
static inline int enum_has_next(enum_t * enu)
{
  return enu->ops.has_next(enu->ctx);
}

// ----- enum_get_next ----------------------------------------------
static inline void * enum_get_next(enum_t * enu)
{
  return enu->ops.get_next(enu->ctx);
}

#endif /* __GDS_ENUMERATOR_H__ */
