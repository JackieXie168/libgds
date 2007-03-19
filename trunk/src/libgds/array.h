// ==================================================================
// @(#)array.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/04/2003
// @lastdate 19/03/2007
// ==================================================================

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <libgds/enumerator.h>
#include <libgds/types.h>

#define ARRAY_OPTION_SORTED  0x01
#define ARRAY_OPTION_UNIQUE  0x02

// ----- FArrayCompare ----------------------------------------------
/**
 * Warning: FArrayCompare is being passed pointers to the items to be
 * compared.
 */
typedef int (*FArrayCompare)(void * pItem1, void * pItem2,
			     unsigned int uEltSize);
// ----- FArrayDestroy ----------------------------------------------
typedef void (*FArrayDestroy) (void * pItem);
// ----- FArrayForEach ----------------------------------------------
typedef int (*FArrayForEach) (void * pItem, void * pContext);
// ----- FArrayCopyItem ---------------------------------------------
typedef void * (*FArrayCopyItem) (void * pItem);

typedef struct {
  char * data;
} SArray;

typedef struct {
  int * data;
} SIntArray;

#define int_array_create(O) \
          (SIntArray *) _array_create(sizeof(int), O, \
          _array_compare, NULL)
#define int_array_length(A) _array_length((SArray *) A)
#define int_array_set_length(A, L) _array_set_length((SArray *) A, L)
#define int_array_sorted_find_index(A, D, I) \
          _array_sorted_find_index((SArray *) A, D, I)
#define int_array_add(A, D) _array_add((SArray *) A, D)
#define int_array_append(A, D) _array_append((SArray *) A, D)
#define int_array_remove_at(A, I) _array_remove_at((SArray *) A, I)

typedef struct {
  uint32_t * data;
} SUInt32Array;
#define uint32_array_create(O) \
          (SUInt32Array *) _array_create(sizeof(uint32_t), O, \
          _array_compare, NULL)

typedef struct {
  uint16_t * data;
} SUInt16Array;
#define uint16_array_create(O) \
          (SUInt16Array *) _array_create(sizeof(uint16_t), O, \
          _array_compare, NULL)
#define uint16_array_length(A) _array_length((SArray *) A)

typedef struct {
  double * data;
} SDoubleArray;
#define double_array_create(O) \
          (SDoubleArray *) _array_create(sizeof(double), O, \
          _array_compare, NULL)
#define double_array_length(A) _array_length((SArray *) A)
#define double_array_set_length(A, L) _array_set_length((SArray *) A, L)
#define double_array_get_at(A, I, E) _array_get_at((SArray *) A, I, E)
#define double_array_remove_at(A, I) _array_remove_at((SArray *)A, I)
#define double_array_add(A, E) _array_add((SArray *)A, E)
#define double_array_sorted_find_index(A, E, I) _array_sorted_find_index(A, E, I)

typedef struct {
  void ** data;
} SPtrArray;
#define ptr_array_create_ref(O) \
          (SPtrArray *) _array_create(sizeof(void *), O, \
          _array_compare, NULL)
#define ptr_array_create(O, FC, FD) \
          (SPtrArray *) _array_create(sizeof(void *), O, FC, FD)
#define ptr_array_length(A) _array_length((SArray *) A)
#define ptr_array_set_length(A, L) _array_set_length((SArray *) A, L)
#define ptr_array_sorted_find_index(A, D, I) \
          _array_sorted_find_index((SArray *) A, D, I)
#define ptr_array_add(A, D) _array_add((SArray *) A, D)
#define ptr_array_append(A, D) _array_append((SArray *) A, &D)
#define ptr_array_remove_at(A, I) _array_remove_at((SArray *) A, I)
#define ptr_array_get_at(A, I, E) _array_get_at((SArray *) A, I, E)
#define ptr_array_set_fdestroy(A, F) _array_set_fdestroy((SArray *)A, F)

#ifdef __cplusplus
extern "C" {
#endif

  // ----- _array_create --------------------------------------------
  SArray * _array_create(unsigned int uEltSize,
				uint8_t uOptions,
				FArrayCompare fCompare,
				FArrayDestroy fDestroy);
  // ----- _array_set_fdestroy --------------------------------------
  void _array_set_fdestroy(SArray * pArray, FArrayDestroy fDestroy);
  // ----- _array_destroy -------------------------------------------
  void _array_destroy(SArray ** ppArray);
  // ----- _array_length --------------------------------------------
  int _array_length(SArray * pArray);
  // ----- _array_set_length ----------------------------------------
  void _array_set_length(SArray * pArray,
			 unsigned int uNewLength);
  // ----- _array_set_at --------------------------------------------
  int _array_set_at(SArray * pArray, unsigned int uIndex,
		    void * pData);
  // ----- _array_get_at --------------------------------------------
  int _array_get_at(SArray * pArray, unsigned int uIndex,
		    void * pData);
  // ----- _array_sorted_find_index ---------------------------------
  int _array_sorted_find_index(SArray * pArray, void * pData,
			       unsigned int * puIndex);
  // ----- _array_add -----------------------------------------------
  int _array_add(SArray * pArray, void * pData);
  // ----- _array_append --------------------------------------------
  int _array_append(SArray * pArray, void * pData);
  // ----- _array_insert_at -----------------------------------------
  int _array_insert_at(SArray * pArray, unsigned int uIndex, void * pData);
  // ----- _array_for_each ------------------------------------------
  int _array_for_each(SArray * pArray, FArrayForEach fForEach,
		      void * pContext);
  // ----- _array_copy ----------------------------------------------
  SArray * _array_copy(SArray * pArray);
  // ----- _array_remove_at -----------------------------------------
  int _array_remove_at(SArray * pArray, unsigned int uIndex);
  // ----- _array_compare -------------------------------------------
  int _array_compare(void * pItem1, void * pItem2,
		     unsigned int uEltSize);
  // ----- _array_sub -----------------------------------------------
  SArray * _array_sub(SArray * pArray, unsigned int iFirst,
		      unsigned int iLast);
  // ----- _array_add_array -----------------------------------------
  void _array_add_array(SArray * pArray, SArray * pSrcArray);
  // ----- _array_trim ----------------------------------------------
  void _array_trim(SArray * pArray, unsigned uMaxLength);
  // ----- _array_sort ----------------------------------------------
  int _array_sort(SArray * pArray, FArrayCompare fCompare);
  // ----- _array_get_enum ------------------------------------------
  SEnumerator * _array_get_enum(SArray * pArray);
  
#ifdef __cplusplus
}
#endif

#define ARRAY_DESTROY_TEMPLATE(P, T)		\
inline void P##_array_destroy(T ** ppArray);

ARRAY_DESTROY_TEMPLATE(double, SDoubleArray)
ARRAY_DESTROY_TEMPLATE(int, SIntArray)
ARRAY_DESTROY_TEMPLATE(ptr, SPtrArray)
ARRAY_DESTROY_TEMPLATE(uint16, SUInt16Array)
ARRAY_DESTROY_TEMPLATE(uint32, SUInt32Array)

#undef ARRAY_DESTROY_TEMPLATE

#endif
