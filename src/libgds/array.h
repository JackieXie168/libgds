// ==================================================================
// @(#)array.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/04/2003
// $Id$
// ==================================================================

#ifndef __GDS_ARRAY_H__
#define __GDS_ARRAY_H__

#include <stdlib.h>

#include <libgds/enumerator.h>
#include <libgds/types.h>

#define ARRAY_OPTION_SORTED  0x01
#define ARRAY_OPTION_UNIQUE  0x02

// ----- FArrayCompare ----------------------------------------------
/**
 * Warning: FArrayCompare is being passed pointers to the items to be
 * compared.
 */
typedef int (*FArrayCompare)(const void * item1,
			     const void * item2,
			     unsigned int elt_size);
// ----- FArrayDestroy ----------------------------------------------
typedef void (*FArrayDestroy) (void * item, const void * ctx);
// ----- FArrayForEach ----------------------------------------------
typedef int (*FArrayForEach) (const void * item, const void * ctx);
// ----- FArrayCopyItem ---------------------------------------------
typedef void * (*FArrayCopyItem) (const void * item);

typedef struct array_t {
  char * data;
} array_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- _array_create --------------------------------------------
  GDS_EXP_DECL array_t * _array_create(unsigned int elt_size,
				       unsigned int size,
				       uint8_t options,
				       FArrayCompare compare,
				       FArrayDestroy destroy,
				       const void * destroy_ctx);
  // ----- _array_set_fdestroy --------------------------------------
  GDS_EXP_DECL void _array_set_fdestroy(array_t * array,
					FArrayDestroy destroy,
					const void * destroy_ctx);
  // ----- _array_destroy -------------------------------------------
  GDS_EXP_DECL void _array_destroy(array_t ** array_ref);
  // ----- _array_length --------------------------------------------
  GDS_EXP_DECL unsigned int _array_length(array_t * array);
  // ----- _array_set_length ----------------------------------------
  GDS_EXP_DECL void _array_set_length(array_t * array,
				      unsigned int size);
  // ----- _array_set_at --------------------------------------------
  GDS_EXP_DECL int _array_set_at(array_t * array, unsigned int index,
				 void * data);
  // ----- _array_get_at --------------------------------------------
  GDS_EXP_DECL int _array_get_at(array_t * array, unsigned int index,
				 void * data);
  // ----- _array_sorted_find_index ---------------------------------
  GDS_EXP_DECL int _array_sorted_find_index(array_t * array, void * data,
					    unsigned int * index);
  // ----- _array_add -----------------------------------------------
  GDS_EXP_DECL int _array_add(array_t * array, void * data);
  // ----- _array_append --------------------------------------------
  GDS_EXP_DECL int _array_append(array_t * array, void * data);
  // ----- _array_insert_at -----------------------------------------
  GDS_EXP_DECL int _array_insert_at(array_t * array, unsigned int index,
				    void * data);
  // ----- _array_for_each ------------------------------------------
  GDS_EXP_DECL int _array_for_each(array_t * array, FArrayForEach foreach,
				   const void * ctx);
  // ----- _array_copy ----------------------------------------------
  GDS_EXP_DECL array_t * _array_copy(array_t * array);
  // ----- _array_remove_at -----------------------------------------
  GDS_EXP_DECL int _array_remove_at(array_t * array, unsigned int index);
  // ----- _array_compare -------------------------------------------
  GDS_EXP_DECL int _array_compare(const void * item1, const void * item2,
				  unsigned int elt_size);
  // ----- _array_sub -----------------------------------------------
  GDS_EXP_DECL array_t * _array_sub(array_t * array, unsigned int first,
				    unsigned int last);
  // ----- _array_add_array -----------------------------------------
  GDS_EXP_DECL void _array_add_array(array_t * array, array_t * src_array);
  // ----- _array_trim ----------------------------------------------
  GDS_EXP_DECL void _array_trim(array_t * array, unsigned max_length);
  // ----- _array_sort ----------------------------------------------
  GDS_EXP_DECL int _array_sort(array_t * array, FArrayCompare fCompare);
  // ----- _array_get_enum ------------------------------------------
  GDS_EXP_DECL gds_enum_t * _array_get_enum(array_t * array);
  
#ifdef __cplusplus
}
#endif

// ------------------------------------------------------------------
// ARRAY DECLARATION TEMPLATE:
// ------------------------------------------------------------------
// Use as follows:
//   GDS_ARRAY_DEFINE(net_ifaces, net_iface_t *, 0, NULL, NULL, NULL)
//
// This will create the following typedef and functions:
//   typedef struct net_ifaces_t {
//     net_iface_t ** data;
//   } net_ifaces_t;
//   static inline net_ifaces_t * net_ifaces_create() {
//     return (net_ifaces_t *) _array_create(sizeof(net_iface_t *),
//                                           0, NULL, NULL, NULL);
//   }
//   static inline void net_ifaces_destroy(net_ifaces_t ** array_ref) {
//     _array_destroy((array_t **) array_ref);
//   }
//   (...)
// ------------------------------------------------------------------

#define GDS_ARRAY_TEMPLATE_TYPE(N,T)					\
  typedef struct N##_t {						\
    T * data;								\
  } N##_t;

#define GDS_ARRAY_TEMPLATE_OPS(N,T,OPT,FC,FD,FDC)			\
  static inline N##_t * N##_create(unsigned int size) {			\
    return (N##_t *) _array_create(sizeof(T),size,OPT,FC,FD,FDC);	\
  }									\
  static inline N##_t * N##_create2(unsigned int size,			\
				    uint8_t options) {			\
    return (N##_t *) _array_create(sizeof(T),size,options,FC,FD,NULL);	\
  }									\
  static inline void N##_destroy(N##_t ** ref) {			\
    _array_destroy((array_t **) ref);					\
  }									\
  static inline unsigned int N##_size(N##_t * array) {			\
    return _array_length((array_t *) array);				\
  }									\
  static inline void N##_set_size(N##_t * array, unsigned int size) {	\
    _array_set_length((array_t *) array, size);				\
  }									\
  static inline int N##_add(N##_t * array, T data) {			\
    return _array_add((array_t *) array, &data);			\
  }									\
  static inline int N##_append(N##_t * array, T data) {			\
    return _array_append((array_t *) array, &data);			\
  }									\
  static inline int N##_remove_at(N##_t * array, unsigned int index) {	\
    return _array_remove_at((array_t *) array, index);			\
  }									\
  static inline int N##_index_of(N##_t * array,				\
				 T data,				\
				 unsigned int * index) {		\
    return _array_sorted_find_index((array_t *) array, &data, index);	\
  }									\
  static inline int N##_for_each(N##_t * array,				\
				 FArrayForEach fForEach,		\
				 void * pContext) {			\
    return _array_for_each((array_t *) array, fForEach, pContext);	\
  }									\
  static inline gds_enum_t * N##_get_enum(N##_t * array) {		\
    return _array_get_enum((array_t *) array);				\
  }									\
  static inline N##_t * N##_copy(N##_t * array) {			\
    return (N##_t *) _array_copy((array_t *) array);			\
  }									\
  static inline int N##_insert_at(N##_t * array,			\
				  unsigned int index,			\
				  T * data) {				\
    return _array_insert_at((array_t *) array, index, data);		\
  }									\
  static inline int N##_sort(N##_t * array,				\
			     FArrayCompare fCompare) {			\
    return _array_sort((array_t *) array, fCompare);			\
  }									\
  static inline N##_t * N##_sub(N##_t * array,				\
				unsigned int first,			\
				unsigned int last) {			\
    return (N##_t *) _array_sub((array_t *) array, first, last);	\
  }									\
  static inline void N##_trim(N##_t * array, unsigned int size) {	\
    _array_trim((array_t *) array, size);				\
  }									\
  static inline void N##_add_array(N##_t * array, N##_t * add_array) {	\
    _array_add_array((array_t *) array, (array_t *) add_array);		\
  }

#define GDS_ARRAY_TEMPLATE(N,T,OPT,FC,FD,FDC)				\
  GDS_ARRAY_TEMPLATE_TYPE(N,T);						\
  GDS_ARRAY_TEMPLATE_OPS(N,T,OPT,FC,FD,FDC);

typedef struct ptr_array_t {
  void ** data;
} ptr_array_t;
typedef ptr_array_t SPtrArray;

#define ptr_array_create_ref(O)				\
  (ptr_array_t *) _array_create(sizeof(void *), 0, O,	\
				_array_compare, NULL, NULL)
#define ptr_array_create(O, FC, FD, FDC)				\
  (ptr_array_t *) _array_create(sizeof(void *), 0, O, FC, FD, FDC)
#define ptr_array_length(A) _array_length((array_t *) A)
#define ptr_array_set_length(A, L) _array_set_length((array_t *) A, L)
#define ptr_array_sorted_find_index(A, D, I)		\
  _array_sorted_find_index((array_t *) A, D, I)
#define ptr_array_add(A, D) _array_add((array_t *) A, D)
#define ptr_array_append(A, D) _array_append((array_t *) A, &D)
#define ptr_array_remove_at(A, I) _array_remove_at((array_t *) A, I)
#define ptr_array_get_at(A, I, E) _array_get_at((array_t *) A, I, E)
#define ptr_array_set_fdestroy(A, F, FDC)	\
  _array_set_fdestroy((array_t *)A, F, FDC)

#define ARRAY_DESTROY_TEMPLATE(P, T)			\
  inline static void P##_array_destroy(T ** array) {	\
    _array_destroy((array_t **) array); }

ARRAY_DESTROY_TEMPLATE(ptr, ptr_array_t)
  
#undef ARRAY_DESTROY_TEMPLATE
  
GDS_ARRAY_TEMPLATE(int_array, int, 0, _array_compare, NULL, NULL)
GDS_ARRAY_TEMPLATE(uint32_array, uint32_t, 0, _array_compare, NULL, NULL)
GDS_ARRAY_TEMPLATE(uint16_array, uint16_t, 0, _array_compare, NULL, NULL)
GDS_ARRAY_TEMPLATE(double_array, double, 0, _array_compare, NULL, NULL)
  
#endif /* __GDS_ARRAY_H__ */
