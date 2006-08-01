// ==================================================================
// @(#)sequence.h
//
// @author Bruno Quoitin (bqu@infonet.fundp.ac.be)
// @date 23/11/2002
// @lastdate 04/12/2002
// ==================================================================

#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__

extern unsigned long sequence_create_count;
extern unsigned long sequence_copy_count;
extern unsigned long sequence_destroy_count;


// ----- FSeqCompare -----------------------------------------------
typedef int (*FSeqCompare)(void * pItem1, void * pItem2);

// ----- FSeqDestroy -----------------------------------------------
typedef void (*FSeqDestroy)(void ** ppItem);

// ----- FSeqForEach -----------------------------------------------
typedef void (*FSeqForEach)(void * pItem, void * pContext);

// ----- FSeqCopyItem ----------------------------------------------
typedef void * (*FSeqCopyItem)(void * pItem);

// ----- SSequence -------------------------------------------------
typedef struct {
  int iSize;
  void ** ppItems;
  FSeqCompare fCompare;
  FSeqDestroy fDestroy;
} SSequence;

// ----- sequence_create --------------------------------------------
extern SSequence * sequence_create(FSeqCompare fCompare,
				   FSeqDestroy fDestroy);
// ----- sequence_destroy -------------------------------------------
extern void sequence_destroy(SSequence ** ppSequence);
// ----- sequence_find_index ----------------------------------------
extern int sequence_find_index(SSequence * pSequence, void * pItem);
// ----- sequence_insert_index --------------------------------------
extern int sequence_insert_at(SSequence * pSequence, int iIndex,
			      void * pItem);
// ----- sequence_add -----------------------------------------------
extern int sequence_add(SSequence * pSequence, void * pItem);
// ----- sequence_remove --------------------------------------------
extern int sequence_remove(SSequence * pSequence, void * pItem);
// ----- sequence_for_each ------------------------------------------
extern void sequence_for_each(SSequence * pSequence,
			      FSeqForEach fForEach,
			      void * pContext);
// ----- sequence_copy ----------------------------------------------
extern SSequence * sequence_copy(SSequence * pSequence,
				 FSeqCopyItem fCopyItem);
// ----- sequence_remove_at -----------------------------------------
extern int sequence_remove_at(SSequence * pSequence, int iIndex);

#endif
