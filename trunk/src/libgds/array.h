// ==================================================================
// @(#)array.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 10/04/2003
// @lastdate 14/04/2004
// ==================================================================

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <libgds/types.h>

#define ARRAY_OPTION_SORTED  0x01
#define ARRAY_OPTION_UNIQUE  0x02

// ----- FArrayCompare ----------------------------------------------
typedef int (*FArrayCompare)(void * pItem1, void * pItem2,
			     unsigned int uEltSize);
// ----- FArrayDestroy ----------------------------------------------
typedef void (*FArrayDestroy) (void * pItem);
// ----- FArrayForEach ----------------------------------------------
typedef void (*FArrayForEach) (void * pItem, void * pContext);
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
#define int_array_destroy(A) _array_destroy((SArray **) A)
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
#define uint16_array_destroy(A) _array_destroy((SArray **) A)
#define uint16_array_length(A) _array_length((SArray *) A)

typedef struct {
  void ** data;
} SPtrArray;
#define ptr_array_create_ref(O) \
          (SPtrArray *) _array_create(sizeof(void *), O, \
          _array_compare, NULL)
#define ptr_array_create(O, FC, FD) \
          (SPtrArray *) _array_create(sizeof(void *), O, FC, FD)
#define ptr_array_destroy(A) _array_destroy((SArray **) A)
#define ptr_array_length(A) _array_length((SArray *) A)
#define ptr_array_set_length(A, L) _array_set_length((SArray *) A, L)
#define ptr_array_sorted_find_index(A, D, I) \
          _array_sorted_find_index((SArray *) A, D, I)
#define ptr_array_add(A, D) _array_add((SArray *) A, D)
#define ptr_array_append(A, D) _array_append((SArray *) A, &D)
#define ptr_array_remove_at(A, I) _array_remove_at((SArray *) A, I)

// ----- _array_create ----------------------------------------------
extern SArray * _array_create(unsigned int uEltSize,
			      uint8_t uOptions,
			      FArrayCompare fCompare,
			      FArrayDestroy fDestroy);
// ----- _array_destroy ---------------------------------------------
extern void _array_destroy(SArray ** ppArray);
// ----- _array_length ----------------------------------------------
extern int _array_length(SArray * pArray);
// ----- _array_set_length ------------------------------------------
extern void _array_set_length(SArray * pArray,
			      unsigned int uNewLength);
// ----- _array_set_at ----------------------------------------------
extern int _array_set_at(SArray * pArray, unsigned int uIndex,
			  void * pData);
// ----- _array_get_at ----------------------------------------------
extern void _array_get_at(SArray * pArray, unsigned int uIndex,
			  void * pData);
// ----- _array_sorted_find_index -----------------------------------
extern int _array_sorted_find_index(SArray * pArray, void * pData,
				    unsigned int * puIndex);
// ----- _array_add -------------------------------------------------
extern int _array_add(SArray * pArray, void * pData);
// ----- _array_append ----------------------------------------------
extern int _array_append(SArray * pArray, void * pData);
// ----- _array_copy ------------------------------------------------
extern SArray * _array_copy(SArray * pArray);
// ----- _array_remove_at -------------------------------------------
extern void _array_remove_at(SArray * pArray, unsigned int uIndex);
// ----- _array_compare ---------------------------------------------
extern int _array_compare(void * pItem1, void * pItem2,
			  unsigned int uEltSize);
// ----- _array_sub -------------------------------------------------
extern SArray * _array_sub(SArray * pArray, unsigned int iFirst,
			   unsigned int iLast);
// ----- _array_add_all ---------------------------------------------
extern void _array_add_all(SArray * pArray, SArray * pSrcArray);
// ----- _array_trim ------------------------------------------------
extern void _array_trim(SArray * pArray, unsigned uMaxLength);
// ----- _array_sort ------------------------------------------------
extern int _array_sort(SArray * pArray, FArrayCompare fCompare);

#endif
