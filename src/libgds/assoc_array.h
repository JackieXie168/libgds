// ==================================================================
// @(#)assoc_array.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 05/01/2007
// $Id$
// ==================================================================

#ifndef __GDS_ASSOC_ARRAY_H__
#define __GDS_ASSOC_ARRAY_H__

#include <libgds/array.h>
#include <libgds/enumerator.h>

typedef SPtrArray SAssocArray;
typedef int (*FAssocArrayForEach)(const char * pcKey, void * pData,
				  void * pContext);

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ assoc_array_create ]-------------------------------------
  SAssocArray * assoc_array_create();
  // -----[ assoc_array_destroy ]------------------------------------
  void assoc_array_destroy(SAssocArray ** pArray);
  // -----[ assoc_array_length ]-------------------------------------
  unsigned int assoc_array_length(SAssocArray * pArray);
  // -----[ assoc_array_exists ]-------------------------------------
  int assoc_array_exists(SAssocArray * pArray, const char * pcKey);
  // -----[ assoc_array_get ]----------------------------------------
  void * assoc_array_get(SAssocArray * pArray, const char * pcKey);
  // -----[ assoc_array_set ]----------------------------------------
  int assoc_array_set(SAssocArray * pArray, const char * pcKey,
		      void * pData);
  // -----[ assoc_array_for_each ]-----------------------------------
  int assoc_array_for_each(SAssocArray * pArray,
			   FAssocArrayForEach fForEach,
			   void * pContext);
  // -----[ assoc_array_get_enum ]-----------------------------------
  enum_t * assoc_array_get_enum(SAssocArray * pArray);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_ASSOC_ARRAY_H__ */
