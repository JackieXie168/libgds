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

#include <libgds/types.h>

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
} gds_enum_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- enum_create ------------------------------------------------
  GDS_EXP_DECL gds_enum_t * enum_create(void * ctx,
					FEnumeratorHasNext has_next,
					FEnumeratorGetNext get_next,
					FEnumeratorDestroy destroy);
  // ----- enum_destroy -----------------------------------------------
  GDS_EXP_DECL void enum_destroy(gds_enum_t ** enum_ref);
  
#ifdef __cplusplus
}
#endif

// ----- enum_has_next ----------------------------------------------
static inline int enum_has_next(gds_enum_t * enu)
{
  return enu->ops.has_next(enu->ctx);
}

// ----- enum_get_next ----------------------------------------------
static inline void * enum_get_next(gds_enum_t * enu)
{
  return enu->ops.get_next(enu->ctx);
}

#define GDS_ENUM_TEMPLATE_TYPE(N,T)					\
  typedef gds_enum_t N##_t;						\
  typedef T (*N##_get_next_func)(void * ctx);

#define GDS_ENUM_TEMPLATE_OPS(N,T)					\
  static inline N##_t * N##_create(void * ctx,				\
				   FEnumeratorHasNext has_next,		\
				   N##_get_next_func get_next,		\
				   FEnumeratorDestroy destroy) {	\
    return enum_create(ctx, has_next,					\
		       (FEnumeratorGetNext) get_next,			\
		       destroy);					\
  }									\
  static inline void N##_destroy(N##_t ** enu) {			\
    enum_destroy(enu);							\
  }									\
  static inline int N##_has_next(N##_t * enu) {				\
    return enum_has_next(enu);						\
  }									\
  static inline T N##_get_next(N##_t * enu) {				\
    return ((N##_get_next_func) enu->ops.get_next)(enu->ctx);		\
  }

#endif /* __GDS_ENUMERATOR_H__ */
