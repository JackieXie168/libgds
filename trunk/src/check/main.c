// ==================================================================
// @(#)main.c
//
// Generic Data Structures (libgds): validation application.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @lastdate 05/10/2007
// ==================================================================
// Notes on unit testing:
//  - keep tests short and focused on a single aspect
//
// Todo:
//  - investigate code coverage of unit testing using gcov ?
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgds/array.h>
#include <libgds/assoc_array.h>
#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/dllist.h>
#include <libgds/enumerator.h>
#include <libgds/fifo.h>
#include <libgds/gds.h>
#include <libgds/hash.h>
#include <libgds/list.h>
#include <libgds/memory.h>
#include <libgds/patricia-tree.h>
#include <libgds/radix-tree.h>
#include <libgds/str_util.h>
#include <libgds/tokenizer.h>
#include <libgds/tokens.h>
#include <libgds/utest.h>

#define IPV4_TO_INT(A,B,C,D) (((((uint32_t)(A))*256 +(uint32_t)(B))*256 +(uint32_t)( C))*256 +(uint32_t)(D))

static char acAddress[16];
char * INT_TO_IPV4(unsigned int uAddress)
{
  snprintf(acAddress, sizeof(acAddress), "%u.%u.%u.%u",
	   (uAddress >> 24), (uAddress >> 16) & 255,
	   (uAddress >> 8) & 255, uAddress & 255);
  return acAddress;
}

// ----- int_array_shuffle ------------------------------------------
/**
 * Random permutation of an array (Durstenfeld, 1964, CACM)
 */
void int_array_shuffle(int aiArray[], unsigned int uSize)
{
  unsigned int uIndex;
  int iTemp;
  int uRandomIndex;

  for (uIndex= 0; uIndex < uSize; uIndex++) {
    uRandomIndex= random() % uSize;
    iTemp= aiArray[uIndex];
    aiArray[uIndex]= aiArray[uRandomIndex];
    aiArray[uRandomIndex]= iTemp;
  }
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_STRUTILS
/////////////////////////////////////////////////////////////////////

// -----[ test_strutils_basic ]--------------------------------------
int test_strutils_basic()
{
  return UTEST_SKIPPED;
}

#define INT32_MIN_STR "-2147483648"
#define INT32_MAX_STR "2147483647"
#define UINT32_MAX_STR "4294967295"

#define INT64_MIN_STR "-9223372036854775808"
#define INT64_MAX_STR "9223372036854775807"
#define UINT64_MAX_STR "18446744073709551615"

// -----[ test_strutils_convert_int ]--------------------------------
int test_strutils_convert_int()
{
  int iValue;

  switch (sizeof(iValue)) {
  case 4:
    ASSERT_RETURN((str_as_int(INT32_MIN_STR, &iValue) == 0) &&
		  (iValue == INT_MIN),
		  "could not convert string to int");
    ASSERT_RETURN((str_as_int(INT32_MAX_STR, &iValue) == 0) &&
		  (iValue == INT_MAX),
		  "could not convert string to int");
    break;

  case 8:
    ASSERT_RETURN((str_as_int(INT64_MIN_STR, &iValue) == 0)
		  && (iValue = (int) INT64_MIN),
		  "could not convert string to int");
    ASSERT_RETURN((str_as_int(INT64_MAX_STR, &iValue) == 0)
		  && (iValue == (int) INT64_MAX),
		  "could not convert string to int");
    break;

  default:
    ASSERT_RETURN(0, "int size not supported (%d)", sizeof(iValue));
    
  }
  return UTEST_SUCCESS;
}

// -----[ test_strutils_convert_uint ]-------------------------------
int test_strutils_convert_uint()
{
  unsigned int uValue;

  switch (sizeof(uValue)) {
  case 4:
    ASSERT_RETURN((str_as_uint(UINT32_MAX_STR, &uValue) == 0) &&
		  (uValue == UINT_MAX),
		  "could not convert string to uint");
    break;

  case 8:
    ASSERT_RETURN((str_as_uint(UINT64_MAX_STR, &uValue) == 0)
		  && (uValue == UINT_MAX),
		  "could not convert string to uint");
    break;

  default:
    ASSERT_RETURN(0, "uint size not supported (%d)", sizeof(uValue));
    
  }
  return UTEST_SUCCESS;
}

// -----[ test_strutils_convert_long ]-------------------------------
int test_strutils_convert_long()
{
  long int lValue;

  switch (sizeof(lValue)) {
  case 4:
    ASSERT_RETURN((str_as_long(INT32_MIN_STR, &lValue) == 0)
		  && (lValue = INT32_MIN),
		  "could not convert string to long");
    ASSERT_RETURN((str_as_long(INT32_MAX_STR, &lValue) == 0)
		  && (lValue == INT32_MAX),
		  "could not convert string to long");
    break;

  case 8:
    ASSERT_RETURN((str_as_long(INT64_MIN_STR, &lValue) == 0)
		  && (lValue = (long) INT64_MIN),
		  "could not convert string to long");
    ASSERT_RETURN((str_as_long(INT64_MAX_STR, &lValue) == 0)
		  && (lValue == (long) INT64_MAX),
		  "could not convert string to long");
    break;

  default:
    ASSERT_RETURN(0, "long size not supported (%d)", sizeof(lValue));

  }

  return UTEST_SUCCESS;
}

// -----[ test_strutils_convert_ulong ]------------------------------
int test_strutils_convert_ulong()
{
  unsigned long int ulValue;

  switch (sizeof(ulValue)) {
  case 4:
    ASSERT_RETURN((str_as_ulong(UINT32_MAX_STR, &ulValue) == 0)
		  && (ulValue == UINT32_MAX),
		  "could not convert string to ulong");
    break;

  case 8:
    ASSERT_RETURN((str_as_ulong(UINT64_MAX_STR, &ulValue) == 0)
		  && (ulValue == (unsigned long) UINT64_MAX),
		  "could not convert string to ulong");
    break;

  default:
    ASSERT_RETURN(0, "ulong size not supported (%d)", sizeof(ulValue));

  }

  return UTEST_SUCCESS;
}

// -----[ test_strutils_convert_double ]-----------------------------
int test_strutils_convert_double()
{
  /*
  // ----- str_as_double --------------------------------------------
  int str_as_double(const char * pcString, double * pdValue);
  */

  return UTEST_SKIPPED;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_FIFO
/////////////////////////////////////////////////////////////////////

#define FIFO_NITEMS 5
int FIFO_ITEMS[FIFO_NITEMS];

// -----[ test_fifo_init ]-------------------------------------------
int test_fifo_init()
{
  unsigned int uIndex;

  // Initialize
  for (uIndex= 0; uIndex < FIFO_NITEMS; uIndex++) {
    FIFO_ITEMS[uIndex]= random() % 4096;
  }
  
  return UTEST_SUCCESS;
}

// -----[ test_fifo_basic ]------------------------------------------
/**
 *
 */
int test_fifo_basic()
{
  SFIFO * pFIFO;
  unsigned int uIndex;

  test_fifo_init();

  pFIFO= fifo_create(FIFO_NITEMS, NULL);
  ASSERT_RETURN(pFIFO != NULL, "fifo_create() returned a NULL pointer");
  
  // Check initial depth (== 0)
  ASSERT_RETURN(fifo_depth(pFIFO) == 0, "incorrect depth returned");

  // Push data
  for (uIndex= 0; uIndex < FIFO_NITEMS; uIndex++) {
    ASSERT_RETURN(fifo_push(pFIFO, (void *) FIFO_ITEMS[uIndex]) == 0,
		  "could not push data onto FIFO");
  }

  // Check depth == FIFO_NITEMS
  ASSERT_RETURN(fifo_depth(pFIFO) == FIFO_NITEMS,
		"incorrect depth returned");

  // Pushing more should fail (growth option not set)
  ASSERT_RETURN(fifo_push(pFIFO, (void *) 255) != 0,
		"should not allow pushing more than FIFO size");

  // Pop data
  for (uIndex= 0; uIndex < FIFO_NITEMS; uIndex++) {
    ASSERT_RETURN((int) fifo_pop(pFIFO)
		  == FIFO_ITEMS[uIndex],
		  "incorrect value pop'ed");
  }

  fifo_destroy(&pFIFO);

  return UTEST_SUCCESS;
}

// -----[ test_fifo_grow ]-------------------------------------------
int test_fifo_grow()
{
  SFIFO * pFIFO;
  unsigned int uIndex;

  pFIFO= fifo_create(FIFO_NITEMS, NULL);
  ASSERT_RETURN(pFIFO != NULL, "fifo_create() returned a NULL pointer");

  fifo_set_option(pFIFO, FIFO_OPTION_GROW_EXPONENTIAL, 1);

  // Push data
  for (uIndex= 0; uIndex < FIFO_NITEMS; uIndex++) {
    ASSERT_RETURN(fifo_push(pFIFO, (void *) FIFO_ITEMS[uIndex]) == 0,
		  "could not push data onto FIFO");
  }

  // Check depth == FIFO_NITEMS
  ASSERT_RETURN(fifo_depth(pFIFO) == FIFO_NITEMS,
		"incorrect depth returned");

  // Pushing more should be allowed (growth option set)
  ASSERT_RETURN(fifo_push(pFIFO, (void *) 255) == 0,
		"should allow pushing more than FIFO size (grow)");

  // Check depth == FIFO_NITEMS+1
  ASSERT_RETURN(fifo_depth(pFIFO) == FIFO_NITEMS+1,
		"incorrect depth returned");

  fifo_destroy(&pFIFO);

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_STACK
/////////////////////////////////////////////////////////////////////

#define STACK_NITEMS 5
int STACK_ITEMS[STACK_NITEMS];

// -----[ test_stack_init ]------------------------------------------
int test_stack_init()
{
  unsigned int uIndex;

  // Initialize
  for (uIndex= 0; uIndex < STACK_NITEMS; uIndex++) {
    STACK_ITEMS[uIndex]= random() % 4096;
  }
  
  return UTEST_SUCCESS;
}

// -----[ test_stack_basic ]-----------------------------------------
int test_stack_basic()
{
  SStack * pStack;
  unsigned int uIndex;

  test_stack_init();

  pStack= stack_create(STACK_NITEMS);
  ASSERT_RETURN(pStack != NULL, "stack_create() returned NULL");

  // Check length (0)
  ASSERT_RETURN(stack_depth(pStack) == 0, "incorrect stack length");
  ASSERT_RETURN(stack_is_empty(pStack) != 0, "stack should be empty");

  // Push data onto stack
  for (uIndex= 0; uIndex < STACK_NITEMS; uIndex++) {
    ASSERT_RETURN(stack_push(pStack, (void *) STACK_ITEMS[uIndex]) == 0,
		  "could not push onto stack (%d)", uIndex);
  }

  // Check length (STACK_NITEMS)
  ASSERT_RETURN(stack_depth(pStack) == STACK_NITEMS,
		"incorrect stack length");
  ASSERT_RETURN(stack_is_empty(pStack) == 0, "stack should not be empty");

  // Pushing more shouldn't be allowed
  ASSERT_RETURN(stack_push(pStack, (void *) 255) != 0,
		"shouldn't allow pushing more than stack size");

  // Pop data from stack
  for (uIndex= 0; uIndex < STACK_NITEMS; uIndex++) {
    ASSERT_RETURN((int) stack_pop(pStack)
		  == STACK_ITEMS[STACK_NITEMS-uIndex-1],
		  "incorrect value pop'ed");
  }  

  stack_destroy(&pStack);

  return UTEST_SUCCESS;
}

// -----[ test_stack_copy ]------------------------------------------
int test_stack_copy()
{
  return UTEST_SKIPPED;
}

// -----[ test_stack_equal ]-----------------------------------------
int test_stack_equal()
{
  return UTEST_SKIPPED;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_ARRAY
/////////////////////////////////////////////////////////////////////

// -----[ _test_array_compare ]--------------------------------------
/**
 * Utility function used in test_array() to compare two array items
 * (integers). Note: the function is being passed pointers to the
 * items to be compared.
 */
static int _test_array_compare(void * pItem1, void * pItem2,
			       unsigned int uEltSize)
{
  if (*((int *) pItem1) > *((int *) pItem2))
    return 1;
  else if (*((int *) pItem1) < *((int *) pItem2))
    return -1;
  return 0;
  }

#define ARRAY_NITEMS 1024
int ARRAY_ITEMS[ARRAY_NITEMS];
SIntArray * pArray= NULL;
SIntArray * pArrayCopy= NULL;
SIntArray * pArraySub= NULL;

// ----- test_array_init --------------------------------------------
int test_array_init()
{
  unsigned int uIndex;
  int iValue;

  // Generate random sequence of unique integers
  for (uIndex= 0; uIndex < ARRAY_NITEMS; uIndex++) {
    iValue= (random() % 4096)+1;
    if (uIndex > 0) {
      ARRAY_ITEMS[uIndex]= ARRAY_ITEMS[uIndex-1]+iValue;
    } else {
      ARRAY_ITEMS[uIndex]= iValue;
    }
  }

  // Shuffle in order to avoid ascending order
  int_array_shuffle(ARRAY_ITEMS, ARRAY_NITEMS);

  return UTEST_SUCCESS;
}

// -----[ test_array_done ]------------------------------------------
/**
 *
 */
int test_array_done()
{
  int_array_destroy(&pArrayCopy);
  int_array_destroy(&pArray);
  int_array_destroy(&pArraySub);

  return UTEST_SUCCESS;
}

// ----- test_array_basic -------------------------------------------
/**
 * Perform basic tests with arrays of integers: add, length, get.
 */
int test_array_basic()
{
  unsigned int uIndex;

  test_array_init();

  pArray= int_array_create(0);
  ASSERT_RETURN(pArray != NULL, "int_array_create() returned NULL pointer");
  
  /* Add all items to the array */
  for (uIndex= 0; uIndex < ARRAY_NITEMS; uIndex++) {
    /* int_array_add() must return the index of insertion */
    ASSERT_RETURN(int_array_add(pArray, &ARRAY_ITEMS[uIndex]) == uIndex,
		  "int_array_add() returned an incorrect insertion index")
  }

  /* Length */
  ASSERT_RETURN(ARRAY_NITEMS == int_array_length(pArray),
		"int_array_length() returned an incorrect length");

  /* Direct get (as a C array) */
  for (uIndex= 0; uIndex < ARRAY_NITEMS; uIndex++) {
    ASSERT_RETURN(pArray->data[uIndex] == ARRAY_ITEMS[uIndex],
		  "direct read did not return expected value");
  }

  return UTEST_SUCCESS;
}

// -----[ test_array_access ]----------------------------------------
int test_array_access()
{
  unsigned int uIndex;
  int iData;

  /* Get */
  for (uIndex= 0; uIndex < ARRAY_NITEMS; uIndex++) {
    _array_get_at((SArray *) pArray, uIndex, &iData);
    ASSERT_RETURN(iData == ARRAY_ITEMS[uIndex],
		  "_array_get_at() did not return expected value");
  }
  return UTEST_SUCCESS;
}

// -----[ test_array_enum ]------------------------------------------
int test_array_enum()
{
  SEnumerator * pEnum= _array_get_enum((SArray *) pArray);
  int * piData;
  unsigned int uIndex;

  ASSERT_RETURN(pEnum != NULL, "_array_get_enum() returned NULL pointer");
  uIndex= 0;
  while (enum_has_next(pEnum)) {
    piData= enum_get_next(pEnum);
    ASSERT_RETURN(piData != NULL, "enum_get_next() returned NULL pointer");
    ASSERT_RETURN(*piData == ARRAY_ITEMS[uIndex],
      "enumerator returned incorrect element");
    uIndex++;
  }
  ASSERT_RETURN(uIndex == ARRAY_NITEMS,
		"enumerator did not traverse whole array");

  enum_destroy(&pEnum);

  return UTEST_SUCCESS;
}

// -----[ test_array_copy ]------------------------------------------
int test_array_copy()
{
  unsigned int uIndex;

  pArrayCopy= (SIntArray *) _array_copy((SArray *) pArray);
  ASSERT_RETURN(pArrayCopy != NULL,
		"_array_copy() returned NUL pointer");
  ASSERT_RETURN(int_array_length(pArrayCopy) == int_array_length(pArray),
		"length of copied and original arrays did not match");
  for (uIndex= 0; uIndex < int_array_length(pArrayCopy); uIndex++) {
    ASSERT_RETURN(pArray->data[uIndex] == pArrayCopy->data[uIndex],
		  "data in copied and original arrays did not match");
  }

  return UTEST_SUCCESS;
}

// -----[ test_array_remove ]----------------------------------------
int test_array_remove()
{
  unsigned int uIndex;
  
  // Remove (remove elements with odd indices)
  for (uIndex= 0; uIndex < ARRAY_NITEMS/2; uIndex++) {
    ASSERT_RETURN(int_array_remove_at(pArrayCopy, uIndex+1) == 0,
		  "could not remove item at %d", uIndex);
  }
  ASSERT_RETURN(int_array_length(pArrayCopy) == ARRAY_NITEMS-(ARRAY_NITEMS/2),
		"int_array_length() returned an invalid value");

  return UTEST_SUCCESS;
}

// -----[ test_array_insert ]----------------------------------------
/**
 * Insert back removed items
 */
int test_array_insert()
{
  unsigned int uIndex;
  
  // Re-insert removed elements (at their previous position)
  for (uIndex= 0; uIndex < ARRAY_NITEMS/2; uIndex++) {
    ASSERT_RETURN(_array_insert_at((SArray*) pArrayCopy,
				   uIndex*2+1, &ARRAY_ITEMS[uIndex*2+1])
		  == uIndex*2+1,
		  "_array_insert_at() returned an incorrect value");
  }
  ASSERT_RETURN(int_array_length(pArrayCopy) == ARRAY_NITEMS,
		"int_array_length() returned an incorrect value");

  /* Check that the array now contains all items */
  for (uIndex= 0; uIndex < ARRAY_NITEMS; uIndex++) {
    ASSERT_RETURN(pArrayCopy->data[uIndex] == ARRAY_ITEMS[uIndex],
		  "direct read did not return expected value");
  }

  return UTEST_SUCCESS;
}

// -----[ test_array_sort ]------------------------------------------
int test_array_sort()
{
  unsigned int uIndex;
  int ARRAY_ITEMS2[ARRAY_NITEMS];
  unsigned int uIndex2;
  int iValue;

  // Sort (ascending sequence)
  ASSERT_RETURN(_array_sort((SArray *) pArray, _test_array_compare) == 0,
		"incorrect return code for _array_sort()");

  ASSERT_RETURN(int_array_length(pArray) == ARRAY_NITEMS,
		"incorrect length returned after _array_sort()");

  // Check ascending order
  for (uIndex= 0; uIndex < int_array_length(pArray); uIndex++) {
    if (uIndex > 0) {
      ASSERT_RETURN(pArray->data[uIndex-1] <= pArray->data[uIndex],
		    "ascending ordering not respected after _array_sort()");
    }
  }

  // Check _array_sorted_find_index()
  for (uIndex= 0; uIndex < int_array_length(pArray); uIndex++) {
    ASSERT_RETURN(_array_sorted_find_index((SArray *) pArray,
					   (void *) &pArray->data[uIndex],
					   &uIndex2) == 0,
		  "incorrect return code for _array_sorted_find_index()");
    ASSERT_RETURN(uIndex2 == uIndex,
		  "incorrect index returned by _array_sorted_find_index()");
  }

  // Insertion in sorted array
  uIndex2= 0;
  for (uIndex= 1; uIndex < int_array_length(pArray); uIndex++) {
    if (pArray->data[uIndex]-pArray->data[uIndex-1] > 1) {
      iValue= pArray->data[uIndex-1]+1;
      ARRAY_ITEMS2[uIndex2++]= iValue;
    }
  }
  for (uIndex= 0; uIndex < uIndex2; uIndex++) {
    ASSERT_RETURN(int_array_add(pArray, &ARRAY_ITEMS2[uIndex]),
		  "could not insert in sorted array");
  }

  ASSERT_RETURN(int_array_length(pArray) == ARRAY_NITEMS+uIndex2,
		"incorrect length returned (%d vs %d)",
		int_array_length(pArray), uIndex2);

  // Check ascending order
  for (uIndex= 0; uIndex < int_array_length(pArray); uIndex++) {
    if (uIndex > 0) {
      ASSERT_RETURN(pArray->data[uIndex-1] <= pArray->data[uIndex],
		    "ascending ordering not respected after _array_sort()");
    }
  }

  return UTEST_SUCCESS;
}

// -----[ test_array_sub ]-------------------------------------------
int test_array_sub()
{
  unsigned int uIndex;

  pArraySub= (SIntArray *) _array_sub((SArray *) pArray,
				      ARRAY_NITEMS/3,
				      2*(ARRAY_NITEMS/3));
  ASSERT_RETURN(pArraySub != NULL,
		"_array_sub() returned NULL pointer");
  ASSERT_RETURN(int_array_length(pArraySub) != ARRAY_NITEMS/3,
		"Incorrect length for sub-array");
  for (uIndex= 0; uIndex < ARRAY_NITEMS/3; uIndex++) {
    ASSERT_RETURN(pArraySub->data[uIndex]
		  == pArray->data[ARRAY_NITEMS/3+uIndex],
		  "incorrect content in sub-array");
  }

  return UTEST_SUCCESS;
}

// -----[ test_array_trim ]------------------------------------------
int test_array_trim()
{
  _array_trim((SArray *) pArray, ARRAY_NITEMS);
  ASSERT_RETURN(int_array_length(pArray) == ARRAY_NITEMS,
		"incorrect length for trimmed array");
  return UTEST_SUCCESS;
}

// -----[ test_array_add_array ]-------------------------------------
int test_array_add_array()
{
#define ARRAY_NITEMS2 512
  int ARRAY_ITEMS2[ARRAY_NITEMS2];
  SIntArray * pArrayNew;
  unsigned int uIndex;

  pArrayNew= int_array_create(0);
  ASSERT_RETURN(pArrayNew != NULL, "int_array_create() returned NULL pointer");

  for (uIndex= 0; uIndex < ARRAY_NITEMS2; uIndex++) {
    ASSERT_RETURN(int_array_add(pArrayNew, &ARRAY_ITEMS2[uIndex]) == uIndex,
		  "incorrect error code returned by int_array_add()");
  }

  _array_add_array((SArray *) pArray, (SArray *) pArrayNew);

  ASSERT_RETURN(int_array_length(pArray) == ARRAY_NITEMS+ARRAY_NITEMS2,
		"incorrect length for new array (%d vs %d)",
		int_array_length(pArray), ARRAY_NITEMS+ARRAY_NITEMS2);
  int_array_destroy(&pArrayNew);

  test_array_done();

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_ASSOC
/////////////////////////////////////////////////////////////////////
// -----[ _test_assoc_for_each ]-------------------------------------
static int _test_assoc_for_each(const char * pcKey, void * pcValue,
				void * pContext)
{
  return 0;
}

// -----[ test_assoc_basic ]-----------------------------------------
int test_assoc_basic()
{
#define ASSOC_NITEMS 3
  char * ASSOC_ITEMS[ASSOC_NITEMS][2]= {
    {"key1", "toto1"},
    {"key2", "toto2"},
    {"plop", "grominet"},
  };
  SAssocArray * pArray;
  unsigned int uIndex;

  // Test for 'set'
  pArray= assoc_array_create();
  ASSERT_RETURN(pArray != NULL, "assoc_array_create() returned NULL pointer");

  for (uIndex= 0; uIndex < ASSOC_NITEMS; uIndex++) {
    ASSERT_RETURN(assoc_array_set(pArray, ASSOC_ITEMS[uIndex][0],
				  ASSOC_ITEMS[uIndex][1]) == 0,
		  "could not set \"%s\" => \"%s\"",
		  ASSOC_ITEMS[uIndex][0],
		  ASSOC_ITEMS[uIndex][1]);
  }

  // Test content
  for (uIndex= 0; uIndex < ASSOC_NITEMS; uIndex++) {
    ASSERT_RETURN(assoc_array_get(pArray, ASSOC_ITEMS[uIndex][0])
		  == ASSOC_ITEMS[uIndex][1],
		  "incorrect value returned for \"%s\"");
  }

  // Test for 'exists'
  for (uIndex= 0; uIndex < ASSOC_NITEMS; uIndex++) {
    ASSERT_RETURN(assoc_array_exists(pArray, ASSOC_ITEMS[uIndex][0]) == 1,
		  "existence test failed for \"%s\"",
		  ASSOC_ITEMS[uIndex][0]);
  }
  
  ASSERT_RETURN(!assoc_array_exists(pArray, "plopsaland"),
		"should not report existence for missing key");

  assoc_array_set(pArray, "key2", "titi"); 
  ASSERT_RETURN(!strcmp(assoc_array_get(pArray, "key2"), "titi"),
		"incorrect value associated to \"key2\"");

  assoc_array_for_each(pArray, _test_assoc_for_each, NULL);

  assoc_array_destroy(&pArray);

  return UTEST_SUCCESS;
}

// -----[ test_assoc_for_each ]--------------------------------------
int test_assoc_for_each()
{
  return UTEST_SKIPPED;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_PTR_ARRAY
/////////////////////////////////////////////////////////////////////

typedef struct {
  uint16_t uHighID, uLowID;
  uint8_t uData;
} SPtrArrayItem;

// ----- array_ptr_item_create --------------------------------------
SPtrArrayItem * array_ptr_item_create(uint16_t uHighID,
				      uint16_t uLowID,
				      uint8_t uData)
{
  SPtrArrayItem * pItem=
    (SPtrArrayItem *) MALLOC(sizeof(SPtrArrayItem));

  pItem->uHighID= uHighID;
  pItem->uLowID= uLowID;
  pItem->uData= uData;
  return pItem;
}

// ----- array_ptr_compare_function ---------------------------------
int array_ptr_compare_function(void * pItem1, void * pItem2,
			       unsigned int uEltSize)
{
  SPtrArrayItem * pRealItem1= *((SPtrArrayItem **) pItem1);
  SPtrArrayItem * pRealItem2= *((SPtrArrayItem **) pItem2);

  if (pRealItem1->uHighID < pRealItem2->uHighID)
    return -1;
  else if (pRealItem1->uHighID > pRealItem2->uHighID)
    return 1;
  else if (pRealItem1->uLowID < pRealItem2->uLowID)
    return -1;
  else if (pRealItem1->uLowID > pRealItem2->uLowID)
    return 1;
  else
    return 0;
}

// ----- array_ptr_destroy_function ---------------------------------
void array_ptr_destroy_function(void * pItem)
{
  SPtrArrayItem * pRealItem= *((SPtrArrayItem **) pItem);

  FREE(pRealItem);
}

// ----- test_ptr_array ---------------------------------------------
/**
 *
 */
int test_ptr_array()
{
  SPtrArray * pPtrArray;
  SPtrArrayItem * pItem;
  int iIndex;

  pPtrArray= ptr_array_create(ARRAY_OPTION_SORTED,
			      array_ptr_compare_function,
			      array_ptr_destroy_function);
  ASSERT_RETURN(pPtrArray != NULL, "ptr_array_create() returned NULL pointer");

  pItem= array_ptr_item_create(5, 2, 1);
  ptr_array_add(pPtrArray, &pItem);
  pItem= array_ptr_item_create(5, 1, 2);
  ptr_array_add(pPtrArray, &pItem);
  pItem= array_ptr_item_create(4, 2, 3);
  ptr_array_add(pPtrArray, &pItem);
  pItem= array_ptr_item_create(6, 2, 4);
  ptr_array_add(pPtrArray, &pItem);
  for (iIndex= 0; iIndex < ptr_array_length(pPtrArray);
       iIndex++) {
    pItem= (SPtrArrayItem *) pPtrArray->data[iIndex];
  }

  ptr_array_destroy(&pPtrArray);

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_LIST
/////////////////////////////////////////////////////////////////////

#define LIST_NITEMS 1024
int LIST_ITEMS[LIST_NITEMS];

// -----[ test_list_init ]-------------------------------------------
int test_list_init()
{
  unsigned int uIndex;

  for (uIndex= 0; uIndex < LIST_NITEMS; uIndex++) {
    if ((uIndex & 1) != 0) {
      LIST_ITEMS[uIndex]= LIST_NITEMS-(uIndex/2)-1;
    } else {
      LIST_ITEMS[uIndex]= uIndex/2;
    }
  }

  return UTEST_SUCCESS;
}

// -----[ test_list_basic ] -----------------------------------------
/**
 *
 */
int test_list_basic()
{
  SList * pList;
  unsigned int uIndex;
  void * pData;

  test_list_init();

  pList= list_create(NULL, NULL, 1);
  ASSERT_RETURN(pList != NULL, "list_create() returned NULL pointer");

  // Add values
  for (uIndex= 0; uIndex < LIST_NITEMS; uIndex++) {
    ASSERT_RETURN(list_add(pList, (void *) LIST_ITEMS[uIndex]) == 0,
		  "incorrect return value in list_add()");
  }

  // Get content
  for (uIndex= 0; uIndex < LIST_NITEMS; uIndex++) {
    pData= list_get_index(pList, uIndex);
    ASSERT_RETURN(pData == (void *) uIndex,
		  "incorrect value for list_get_index() %d=>%d",
		  (int) pData, uIndex);
  }

  list_destroy(&pList);

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_DLLIST
/////////////////////////////////////////////////////////////////////

// -----[ _test_dllist_for_each ]------------------------------------
static int _test_dllist_for_each(void * pUserData, void * pContext)
{
  return 0;
}

#define DLLIST_NUM_ITEMS 1024
int DLLIST_ITEMS[DLLIST_NUM_ITEMS];

// -----[ test_dllist_init ]---------------------------------------
int test_dllist_init()
{
  unsigned int uIndex;

  for (uIndex= 0; uIndex < DLLIST_NUM_ITEMS; uIndex++) {
    DLLIST_ITEMS[uIndex]= random() % 4096;
  }

  return UTEST_SUCCESS;
}

// -----[ test_dllist_basic ]----------------------------------------
int test_dllist_basic()
{
  SDLList * pList;
  unsigned int uIndex;
  void * pData;

  test_dllist_init();

  pList= dllist_create(NULL);
  ASSERT_RETURN(pList != NULL, "dllist_create() returned NULL pointer");

  // Append
  for (uIndex= 0; uIndex < DLLIST_NUM_ITEMS; uIndex++) {
    ASSERT_RETURN(dllist_append(pList, (void *) DLLIST_ITEMS[uIndex]) == 0,
		  "could not append item");
  }

  // Check content
  for (uIndex= 0; uIndex < DLLIST_NUM_ITEMS; uIndex++) {
    ASSERT_RETURN(dllist_get(pList, uIndex, &pData) == 0,
		  "could not get value at %d", uIndex);
    ASSERT_RETURN(pData == (void *) DLLIST_ITEMS[uIndex],
		  "incorrect value returned at %d", uIndex);
  }
  
  return UTEST_SUCCESS;
}

// ----- test_dllist ------------------------------------------------
/**
 * Purpose of the test:
 * - test compilation (function calls syntax)
 * - test basic list construction/modification (insertion  & removal)
 * - test for_each function
 * - test destruction
 */
int test_dllist()
{
  int iResult= 0;
  SDLList * pList;
  int iValue, * piValue= &iValue;

  // initial list => (1, 2, 3)
  dllist_append(pList, (void *) 1);
  dllist_append(pList, (void *) 2);
  dllist_append(pList, (void *) 3);

  // insert @0 => (55, 1, 2, 3)
  dllist_insert(pList, 0, (void *) 55);

  // insert @2 => (55, 1, 110, 2, 3)
  dllist_insert(pList, 2, (void *) 110);

  // insert @5 (end-of-list) => (55, 1, 110, 2, 3, 666)
  dllist_insert(pList, 5, (void *) 666);

  // remove @0 (start-of-list) => (1, 110, 2, 3, 666)
  dllist_remove(pList, 0);

  // remove @2 (in-list) => (1, 110, 3, 666)
  dllist_remove(pList, 2);

  // remove @3 (end-of-list) => (1, 110, 3)
  dllist_remove(pList, 3);

  // list should contain: (1, 110, 3)
  dllist_for_each(pList, NULL, _test_dllist_for_each);

  // Check list size and content...
  if (dllist_size(pList) != 3)
    iResult= -1;
  if ((dllist_get(pList, 0, (void **) piValue) != 0) ||
      iValue != 1)
    iResult= -1;
  if ((dllist_get(pList, 1, (void **) piValue) != 0) ||
      iValue != 110)
    iResult= -1;
  if ((dllist_get(pList, 2, (void **) piValue) != 0) ||
      iValue != 3)
    iResult= -1;

  dllist_destroy(&pList);

  return iResult;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_RADIX_TREE
/////////////////////////////////////////////////////////////////////

// -----[ _test_radix_for_each ]-------------------------------------
static int _test_radix_for_each(uint32_t uKey, uint8_t uKeyLen,
				void * pItem, void * pContext)
{
  /*
  fprintf(stdout, "key %d.%d.%d.%d/%d --> value %d\n",
	  (uKey >> 24) & 255, (uKey >> 16) & 255,
	  (uKey >> 8) & 255, uKey & 255,
	  uKeyLen, (int) pItem);*/
  return 0;
}

// -----[ _test_radix_destroy ]--------------------------------------
static void _test_radix_destroy(void ** ppItem)
{
}

SRadixTree * pRadix;

// -----[ test_radix_init ]------------------------------------------
int test_radix_init()
{
  pRadix= radix_tree_create(32, NULL);
  ASSERT_RETURN(pRadix != NULL, "radix_tree_create() returned NULL pointer");

  return UTEST_SUCCESS;
}

// -----[ test_radix_done ]------------------------------------------
int test_radix_done()
{
  radix_tree_destroy(&pRadix);

  return UTEST_SUCCESS;
}

// -----[ test_radix_basic ]-----------------------------------------
int test_radix_basic()
{
  return UTEST_SKIPPED;
}

// -----[ test_radix_old ]-------------------------------------------
int test_radix_old()
{
  SRadixTree * pTree;

  pTree= radix_tree_create(32, NULL);
  ASSERT_RETURN(pTree != NULL, "radix_tree_create() returned NULL pointer");

  radix_tree_add(pTree, IPV4_TO_INT(1,0,0,0), 16, (void *) 100);
  radix_tree_add(pTree, IPV4_TO_INT(0,0,0,0), 16, (void *) 200);
  radix_tree_for_each(pTree, _test_radix_for_each, NULL);
  //printf("best(0.3.0.0/32)>-->%d\n", (int) radix_tree_get_best(pTree, 3*256*256, 32));
  //printf("add(0.0.0.0/8, 300)\n");
  radix_tree_add(pTree, IPV4_TO_INT(0,0,0,0), 8, (void *) 300);
  radix_tree_for_each(pTree, _test_radix_for_each, NULL);
  //printf("best(0.3.0.0/32)>-->%d\n", (int) radix_tree_get_best(pTree, 3*256*256, 32));

  /*
  pTree= radix_tree_create(4, NULL);
  printf("add(0/0, 100)\n");
  radix_tree_add(pTree, 0, 0, (void *) 100);
  printf("add(8/1, 200)\n");
  radix_tree_add(pTree, 8, 1, (void *) 200);
  printf("add(0/1, 300)\n");
  radix_tree_add(pTree, 0, 1, (void *) 300);
  printf("add(7/4, 777)\n");
  radix_tree_add(pTree, 7, 4, (void *) 777);
  printf("exact(7/4)>-->%d\n", (int) radix_tree_get_exact(pTree, 7, 4));
  printf("exact(6/4)>-->%d\n", (int) radix_tree_get_exact(pTree, 6, 4));
  printf("exact(4/2)>-->%d\n", (int) radix_tree_get_exact(pTree, 4, 2));
  printf("best(7/4)>-->%d\n", (int) radix_tree_get_best(pTree, 7, 4));
  printf("best(6/4)>-->%d\n", (int) radix_tree_get_best(pTree, 6, 4));
  printf("best(4/2)>-->%d\n", (int) radix_tree_get_best(pTree, 4, 2));
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
  printf("remove(0/1)\n");
  radix_tree_remove(pTree, 0, 1);
  printf("add(8/1, 899)\n");
  radix_tree_add(pTree, 8, 1, (void *) 899);
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
  */
  radix_tree_destroy(&pTree);

  return UTEST_SKIPPED;
}

// -----[ test_radix_old_ipv4 ]--------------------------------------
int test_radix_old_ipv4()
{
  SRadixTree * pTree;

  pTree= radix_tree_create(32, _test_radix_destroy);
  ASSERT_RETURN(pTree != NULL, "radix_tree_create() returned NULL pointer");

  radix_tree_add(pTree, IPV4_TO_INT(12,0,0,0), 8, (void *) 1);
  radix_tree_add(pTree, IPV4_TO_INT(12,148,170,0), 24, (void *) 2);
  radix_tree_add(pTree, IPV4_TO_INT(199,165,16,0), 20, (void *) 3);
  radix_tree_add(pTree, IPV4_TO_INT(199,165,16,0), 24, (void *) 4);

  radix_tree_for_each(pTree, _test_radix_for_each, NULL);

  radix_tree_remove(pTree, IPV4_TO_INT(12,0,0,0), 8, 1);

  radix_tree_for_each(pTree, _test_radix_for_each, NULL);

  /*fprintf(stderr, "exact(199.165.16.0/20): %d\n",
	  (int) radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 20));
  fprintf(stderr, "exact(199.165.16.0/24): %d\n",
	  (int) radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 24));
  fprintf(stderr, "best(199.165.16.0/32): %d\n",
  (int) radix_tree_get_best(pTree, IPV4_TO_INT(199,165,16,0), 32));*/

  radix_tree_remove(pTree, IPV4_TO_INT(199,165,16,0), 24, 1);

  radix_tree_for_each(pTree, _test_radix_for_each, NULL);

  /*fprintf(stderr, "exact(199.165.16.0/20): %d\n",
	  (int) radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 20));
  fprintf(stderr, "exact(199.165.16.0/24): %d\n",
	  (int) radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 24));
  fprintf(stderr, "best(199.165.16.0/32): %d\n",
  (int) radix_tree_get_best(pTree, IPV4_TO_INT(199,165,16,0), 32));*/

  radix_tree_destroy(&pTree);

  return 0;
}

// -----[ test_radix_for_each ]--------------------------------------
int test_radix_for_each()
{
  return UTEST_SKIPPED;
}

// -----[ test_radix_enum ]------------------------------------------
int test_radix_enum()
{
  return UTEST_SKIPPED;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_TOKENIZER
/////////////////////////////////////////////////////////////////////

// -----[ test_tokenizer_basic ]-------------------------------------
/**
 * Perform various tests with tokenizer
 * Test 1 ["abc def ghi"]
 */
int test_tokenizer_basic()
{
  char * pacTokens[3]= {"abc", "def", "ghi"};
  STokenizer * pTokenizer;
  STokens * pTokens;
  unsigned int uIndex;
  int iResult;

  pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  ASSERT_RETURN(pTokenizer != NULL,
		"tokenizer_create() returned NULL pointer");

  iResult= tokenizer_run(pTokenizer, "abc def ghi");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [abc def ghi] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (uIndex= 0; uIndex < tokens_get_num(pTokens); uIndex++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(pTokens, uIndex),
			 pacTokens[uIndex]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(pTokens, uIndex),
		  pacTokens[uIndex]);
  }

  tokenizer_destroy(&pTokenizer);

  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_quotes ]------------------------------------
/**
 * Perform various tests with tokenizer
 * Test 2 ["abc \"def\" ghi"]
 */
int test_tokenizer_quotes()
{
  char * pacTokens[3]= {"abc", "def", "ghi"};
  STokenizer * pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  STokens * pTokens;
  unsigned int uIndex;
  int iResult;

  iResult= tokenizer_run(pTokenizer, "abc \"def\" ghi");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [abc \"def\" ghi] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (uIndex= 0; uIndex < tokens_get_num(pTokens); uIndex++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(pTokens, uIndex),
			 pacTokens[uIndex]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(pTokens, uIndex),
		  pacTokens[uIndex]);
  }

  tokenizer_destroy(&pTokenizer);

  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_quotes2 ]-----------------------------------
/**
 * Perform various tests with tokenizer
 * Test 3 ["123 abc\"def\"ghi 456"]
 */
int test_tokenizer_quotes2()
{
  char * pacTokens[3]= {"123", "abcdefghi", "456"};
  STokenizer * pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  STokens * pTokens;
  unsigned int uIndex;
  int iResult;

  iResult= tokenizer_run(pTokenizer, "123 abc\"def\"ghi 456");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [123 abc\"def\"ghi 456] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (uIndex= 0; uIndex < tokens_get_num(pTokens); uIndex++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(pTokens, uIndex),
			 pacTokens[uIndex]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(pTokens, uIndex),
		  pacTokens[uIndex]);
  }

  tokenizer_destroy(&pTokenizer);

  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_braces ]------------------------------------
/**
 * Perform various tests with tokenizer
 * Test 4 ["123 abc\"def\"ghi { 456 }"]
 */
int test_tokenizer_braces()
{
  char * pacTokens[3]= {"123", "abcdefghi", " 456 "};
  STokenizer * pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  STokens * pTokens;
  unsigned int uIndex;
  int iResult;

  iResult= tokenizer_run(pTokenizer, "123 abc\"def\"ghi { 456 }");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [123 abc\"def\"ghi { 456 }] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (uIndex= 0; uIndex < tokens_get_num(pTokens); uIndex++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(pTokens, uIndex),
			 pacTokens[uIndex]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(pTokens, uIndex),
		  pacTokens[uIndex]);
  }

  tokenizer_destroy(&pTokenizer);

  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_multiquotes ]-------------------------------
/**
 * Perform various tests with tokenizer
 * Test 5 ["  in-filter \"\"\"\"\"\""]
 */
int test_tokenizer_multiquotes()
{
  char * pacTokens[3]= {"in-filter", ""};
  STokenizer * pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  STokens * pTokens;
  unsigned int uIndex;
  int iResult;

  iResult= tokenizer_run(pTokenizer, "  in-filter \"\"\"\"\"\"");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [  in-filter \"\"\"\"\"\"] failed")
    pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 2,
		"wrong number of tokens");
  for (uIndex= 0; uIndex < tokens_get_num(pTokens); uIndex++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(pTokens, uIndex),
			 pacTokens[uIndex]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(pTokens, uIndex),
		  pacTokens[uIndex]);
  }

  tokenizer_destroy(&pTokenizer);

  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_complex ]-----------------------------------
/**
 * Perform various tests with tokenizer
 * [x] Test 6 ["abcdef 12345678901 ghijkl 123:45\t-123.456 \"Hello World !\" ceci\" n{'}est pas \"une{ pipe}"]
 */
int test_tokenizer_complex()
{
  char * acString= "abcdef 12345678901 ghijkl 123:45\t-123.456 \"Hello World !\" ceci\" n{'}est pas \"une{ pipe}";
  char * pacTokens[7]= {"abcdef",
			"12345678901",
			"ghijkl",
			"123:45",
			"-123.456",
			"Hello World !",
			"ceci n{'}est pas une pipe"};
  STokenizer * pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  STokens * pTokens;
  unsigned int uIndex;
  int iResult;

  iResult= tokenizer_run(pTokenizer, acString);
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [%s] failed", acString);
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 7,
		"wrong number of tokens");
  for (uIndex= 0; uIndex < tokens_get_num(pTokens); uIndex++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(pTokens, uIndex),
			 pacTokens[uIndex]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(pTokens, uIndex),
		  pacTokens[uIndex]);
  }

  tokenizer_destroy(&pTokenizer);

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_PATRICIA_TREE
/////////////////////////////////////////////////////////////////////

int TRIE_DESTROY_COUNT= 0;

// -----[ _trie_destroy ]--------------------------------------------
static void _trie_destroy(void ** ppData)
{
  TRIE_DESTROY_COUNT++;
}

// -----[ dump ]-----------------------------------------------------
/*
int dump(trie_key_t uKey, trie_key_len_t uKeyLen,
	  void * pData, void * pContext)
{
  printf("  %s/%u => ", INT_TO_IPV4(uKey), uKeyLen);
  if (pData != NULL) {
    printf("%u\n", (int) pData);
  } else
    printf("(null) %u, %u\n", uKey, uKeyLen);
  return 0;
}
*/

// -----[ dump_exact ]-----------------------------------------------
 /*
void dump_exact(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  void * pData= trie_find_exact(pTrie, uKey, uKeyLen);
  printf("exact %s/%u => ", INT_TO_IPV4(uKey), uKeyLen);
  if (pData == NULL)
    printf("(null)\n");
  else
    printf("%u\n", (int) pData);
}
 */

// -----[ dump_best ]------------------------------------------------
  /*
void dump_best(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  void * pData= trie_find_best(pTrie, uKey, uKeyLen);
  printf("best %s/%u => ", INT_TO_IPV4(uKey), uKeyLen);
  if (pData == NULL)
    printf("(null)\n");
  else
    printf("%u\n", (int) pData);
}
  */

STrie * pTrie= NULL;
#define PATRICIA_NITEMS 9
long PATRICIA_ITEMS[PATRICIA_NITEMS][4]=  {
  {IPV4_TO_INT(0, 128, 0, 0), 16, 100, 0},
  {IPV4_TO_INT(0, 192, 0, 0), 15, 200, 0},
  {IPV4_TO_INT(0, 0, 0, 0), 16, 1, 0},
  {IPV4_TO_INT(0, 0, 0, 0), 15, 2, 0},
  {IPV4_TO_INT(0, 1, 0, 0), 16, 3, 0},
  {IPV4_TO_INT(0, 2, 0, 0), 16, 4, 0},
  {IPV4_TO_INT(0, 1, 1, 1), 24, 5, 0},
  {IPV4_TO_INT(0, 1, 1, 128), 25, 6, 0},
  {IPV4_TO_INT(0, 128, 128, 128), 9, 300, 0},
  };

// -----[ test_patricia_init ]---------------------------------------
int test_patricia_init()
{
  pTrie= trie_create(_trie_destroy);
  ASSERT_RETURN(pTrie != NULL, "patricia-tree not initialized");

  return UTEST_SUCCESS;
}

// -----[ test_patricia_done ]---------------------------------------
int test_patricia_done()
{
  ASSERT_RETURN(pTrie != NULL, "patricia-tree not initialized");
  trie_destroy(&pTrie);

  return UTEST_SUCCESS;
}

// -----[ test_patricia_insertion ]----------------------------------
int test_patricia_insertion()
{
  unsigned int uIndex;

  test_patricia_init();
  
  for (uIndex= 0; uIndex < PATRICIA_NITEMS; uIndex++) {
    ASSERT_RETURN(trie_insert(pTrie, PATRICIA_ITEMS[uIndex][0],
			      PATRICIA_ITEMS[uIndex][1],
			      (void *) PATRICIA_ITEMS[uIndex][2]) == 0,
		  "could not insert %d/%d/%d",
		  PATRICIA_ITEMS[uIndex][0],
		  PATRICIA_ITEMS[uIndex][1],
		  PATRICIA_ITEMS[uIndex][2]);
  }

  return UTEST_SUCCESS;
}

// -----[ test_patricia_masking ]------------------------------------
int test_patricia_masking()
{
  STrie * pTrie= trie_create(_trie_destroy);

  ASSERT_RETURN(trie_insert(pTrie, IPV4_TO_INT(123,234,198,76), 17,
			    (void *) 12357) == 0,
		"could not insert");
  ASSERT_RETURN(trie_find_exact(pTrie, IPV4_TO_INT(123,234,198,76), 17) ==
		(void *) 12357,
		"could not find exact-match");
  ASSERT_RETURN(trie_find_exact(pTrie, IPV4_TO_INT(123,234,128,0), 17) ==
		(void *) 12357,
		"could not find exact-match");
  trie_destroy(&pTrie);
  
  return UTEST_SUCCESS;
}

// -----[ test_patricia_exact ]--------------------------------------
int test_patricia_exact()
{
  unsigned int uIndex;
  void * pData;

  for (uIndex= 0; uIndex < PATRICIA_NITEMS; uIndex++) {
    pData= trie_find_exact(pTrie, PATRICIA_ITEMS[uIndex][0],
			   PATRICIA_ITEMS[uIndex][1]);
    ASSERT_RETURN((int) pData == PATRICIA_ITEMS[uIndex][2],
		  "exact match failed for %d/%d/%d => %d",
		  PATRICIA_ITEMS[uIndex][0],
		  PATRICIA_ITEMS[uIndex][1],
		  PATRICIA_ITEMS[uIndex][2],
		  (int) pData);
  }

  return UTEST_SUCCESS;
}

// -----[ test_patricia_best ]---------------------------------------
/**
 * For each prefix that is shorter than /32 in the initial data, look
 * for a more specific prefix (length+1). Check that if the more
 * specific prefix that is generated exists, the best match should
 * return this prefix. Otherwise, the initial (less specific prefix)
 * should be found.
 *
 * Example:
 *   trie contains (0.192.0.0/16, 0.192.0.0/17)
 *   original=0.192.0.0/16
 *     - generated=0.192.0.0/17 (exists)
 *         => exact-match=0.192.0.0/17
 *     - generated=0.192.128.0/17 (does not exist)
 *         => best-match=0.192.0.0/16
 */
int test_patricia_best()
{
  unsigned int uIndex;
  trie_key_t uKey, uNewKey;
  trie_key_len_t uKeyLen, uNewKeyLen;

  for (uIndex= 0; uIndex < PATRICIA_NITEMS; uIndex++) {
    uKey= PATRICIA_ITEMS[uIndex][0];
    uKeyLen= PATRICIA_ITEMS[uIndex][1];
    if (uKeyLen < 32) {
      uNewKeyLen= uKeyLen+1;
      uNewKey= uKey|(1 << (31-uKeyLen));
      if (trie_find_exact(pTrie, uNewKey, uNewKeyLen) == NULL) {
	ASSERT_RETURN((int) trie_find_best(pTrie, uNewKey, uNewKeyLen)
		      == PATRICIA_ITEMS[uIndex][2],
		      "best-match failed for %d/%d (more specific than %d/%d)",
		      uNewKey, uNewKeyLen, uKey, uKeyLen);

      }
      uNewKey= uKey & ~(1 << (31-uKeyLen));
      if (trie_find_exact(pTrie, uNewKey, uNewKeyLen) == NULL) {
	ASSERT_RETURN((int) trie_find_best(pTrie, uNewKey, uNewKeyLen)
		      == PATRICIA_ITEMS[uIndex][2],
		      "best-match failed for %d/%d (more specific than %d/%d)",
		      uNewKey, uNewKeyLen, uKey, uKeyLen);

      }
    }
  }

  return UTEST_SUCCESS;
}

// -----[ test_patricia_update ]-------------------------------------
int test_patricia_update()
{
  uint32_t uKey= IPV4_TO_INT(0, 0, 0, 0);
  uint8_t uKeyLen= 16;
  void * pData= (void *) -1;
  void *pTemp;

  // Check that inserting an already existing value is refused
  ASSERT_RETURN(trie_insert(pTrie, uKey, uKeyLen, pData) == 0,
		"did not refuse to update existing value %d/%d/%d",
		uKey, uKeyLen, (int) pData);
  // Use replace instead
  /*ASSERT_RETURN(trie_replace(pTrie, uKey, uKeyLen, pData) == 0,
		"could not update %d/%d/%d",
		uKey, uKeyLen, (int) pData);*/
  pTemp= trie_find_exact(pTrie, uKey, uKeyLen);
  ASSERT_RETURN(pData == pTemp,
		"exact-match failed %d/%d/%d => %d",
		uKey, uKeyLen, (int) pData, (int) pTemp);

  return UTEST_SUCCESS;
}

// -----[ test_patricia_replace ]------------------------------------
int test_patricia_replace()
{
  uint32_t uKey= IPV4_TO_INT(0, 128, 128, 127);
  uint8_t uKeyLen= 9;
  void * pData= (void *) 301;
  void *pTemp;

  // Check that replacing an unexisting value is refused
  ASSERT_RETURN(trie_replace(pTrie, uKey, uKeyLen, pData) != 0,
		"did not refuse to replace an un-existing value %d/%d/%d",
		uKey, uKeyLen, (int) pData);
  pTemp= trie_find_exact(pTrie, uKey, uKeyLen);
  ASSERT_RETURN(pData == pTemp,
		"exact-match failed %d/%d/%d => %d",
		uKey, uKeyLen, (int) pData, (int) pTemp);

  return UTEST_SUCCESS;
}

// -----[ test_patricia_remove ] ------------------------------------
int test_patricia_remove()
{
  /*  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 0, 0, 0), 16));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 0, 0, 0), 15));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 1, 0, 0), 16));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 2, 0, 0), 16));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 1, 1, 0), 24));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 1, 1, 128), 25));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 128, 0, 0), 9));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 192, 0, 0), 15));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");
  assert(!trie_remove(pTrie, IPV4_TO_INT(0, 128, 0, 0), 16));
  fprintf(stdout, "DUMP: {\n");
  trie_for_each(pTrie, dump, NULL);
  fprintf(stdout, "}\n");*/

  return UTEST_SUCCESS;
}

// -----[ test_patricia_enum ]---------------------------------------
int test_patricia_enum()
{
  SEnumerator * pEnum;
  void * pData;

  pEnum= trie_get_enum(pTrie);
  ASSERT_RETURN(pEnum != NULL, "trie_get_enum() returned NULL pointer");
  while (enum_has_next(pEnum)) {
    pData= enum_get_next(pEnum);
    ASSERT_RETURN(pData != NULL, "enum_get_next() returned NULL pointer");
  }
  enum_destroy(&pEnum);

  test_patricia_done();

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_CLI
/////////////////////////////////////////////////////////////////////

// -----[ _cli_dump_context ]----------------------------------------
/*static void _cli_dump_context(SCliContext * pContext)
{
  SCliCmd * pCmd= pContext->pCmd;
  SCliOption * pOption;
  SCliParam * pParam;
  SCliCtxItem * pItem;
  unsigned int uIndex;
  unsigned int uParamIndex;

  printf("*** CLI CONTEXT DUMP ***\n");

  // Current command
  if (pCmd == NULL) {
    printf("current command: null\n");
  } else {
    printf("current command: [%s]\n", pCmd->pcName);

    // Options
    if (pCmd->pOptions != NULL) {
      printf("  options:");
      for (uIndex= 0; uIndex < cli_options_num(pCmd->pOptions); uIndex++) {
	pOption= (SCliOption *) pCmd->pOptions->data[uIndex];
	if (pOption->pcValue != NULL)
	  printf(" %s=[%s]", pOption->pcName, pOption->pcValue);
	else
	  printf(" %s=null", pOption->pcName);
      }
      printf("\n");
    }

    // Parameters
    if (pCmd->pParams != NULL) {
      printf("  params:");
      uParamIndex= 0;
      for (uIndex= 0; uIndex < cli_params_num(pCmd->pParams); uIndex++) {
	pParam= (SCliParam *) pCmd->pParams->data[uIndex];
	if (uParamIndex < tokens_get_num(pCmd->pParamValues))
	  printf(" %s=[%s]", pParam->pcName,
		 tokens_get_string_at(pCmd->pParamValues, uParamIndex));
	else
	  printf(" %s=null", pParam->pcName);
	uParamIndex++;
      }
      printf("\n");
    }
  }

  printf("current user-data: %p\n", pContext->pUserData);

  // Context stack
  if (pContext->pCmdStack != NULL) {
    for (uIndex= stack_depth(pContext->pCmdStack); uIndex > 0 ; uIndex--) {
      pItem= (SCliCtxItem *) stack_get_at(pContext->pCmdStack, uIndex-1);
      printf("stacked-command[%d]: %s\n", uIndex-1, pItem->pCmd->pcName);
      printf("stacked-user-data[%d]: %p\n", uIndex-1, pItem->pUserData);
    }
  }

  printf("************************\n");
}*/

// -----[ _cli_cmd_success ]-----------------------------------------
static int _cli_cmd_success(SCliContext * pContext, SCliCmd * pCmd)
{
  return CLI_SUCCESS;
}

// -----[ _cli_cmd_failure ]-----------------------------------------
static int _cli_cmd_failure(SCliContext * pContext, SCliCmd * pCmd)
{
  return CLI_ERROR_COMMAND_FAILED;
}

// -----[ _cli_cmd4_ctx_create ]-------------------------------------
static int _cli_cmd4_ctx_create(SCliContext * pContext, void ** ppItem)
{
  // Should push something on the context stack here that cmd5
  // should try to find (and report an error if it is not found.
  return CLI_SUCCESS;
}

// -----[ _cli_cmd4_ctx_destroy ]------------------------------------
static void _cli_cmd4_ctx_destroy(void ** ppItem)
{
}

SCli * pCli= NULL;
SCliCmd * pCmd;
SCliCmds * pCmds, * pSubCmds;
SCliParams * pParams;
int iCLICmdExecuted;

// -----[ test_cli_init ]--------------------------------------------
/**
 * Initialize the CLI tests
 *
 * test
 *   cmd1: <param1> <param2> <...>[0-5]
 *   cmd2: <param1>
 *   cmd2_fail: <param1> => execution fails
 *   cmd3: <param1> [--plop]
 *   cmd4: 
 *     cmd5:
 */
int test_cli_init()
{
  pCli= cli_create();
  ASSERT_RETURN(pCli != NULL, "cli_create() returned NULL pointer");

  pCmds= cli_cmds_create();
  ASSERT_RETURN(pCmds != NULL, "cli_cmds_create() returned NULL pointer");

  // cmd1: <param1> <param2> <...>[0-5]
  pParams= cli_params_create();
  cli_params_add(pParams, "<param1>", NULL);
  cli_params_add(pParams, "<param2>", NULL);
  cli_params_add_vararg(pParams, "<param3>", 5, NULL);
  // Note: adding an additional parameter should abort() the program
  // cli_params_add(pParams, "<param4>", NULL);
  cli_cmds_add(pCmds, cli_cmd_create("cmd1", _cli_cmd_success,
				     NULL, pParams));

  // cmd2: <param1>
  pParams= cli_params_create();
  cli_params_add(pParams, "<param1>", NULL);
  cli_cmds_add(pCmds, cli_cmd_create("cmd2", _cli_cmd_success,
				     NULL, pParams));

  // cmd2_fail: <param1> => execution failed
  pParams= cli_params_create();
  cli_params_add(pParams, "<param1>", NULL);
  cli_cmds_add(pCmds, cli_cmd_create("cmd2_fail", _cli_cmd_failure,
				     NULL, pParams));

  // cmd3: <param1> [--plop]
  pParams= cli_params_create();
  cli_params_add(pParams, "<param1>", NULL);
  pCmd= cli_cmd_create("cmd3", _cli_cmd_success,
		       NULL, pParams);
  cli_cmd_add_option(pCmd, "plop", NULL);
  cli_cmds_add(pCmds, pCmd);

  // cmd5:
  pSubCmds= cli_cmds_create();
  cli_cmds_add(pSubCmds, cli_cmd_create("cmd5", _cli_cmd_success,
					NULL, NULL));

  // cmd4: create context
  cli_cmds_add(pCmds, cli_cmd_create_ctx("cmd4",
					 _cli_cmd4_ctx_create,
					 _cli_cmd4_ctx_destroy,
					 pSubCmds, NULL));

  // test (root of command tree)
  cli_register_cmd(pCli, cli_cmd_create("test", NULL, pCmds, NULL));

  // Identifier of last executed command
  iCLICmdExecuted= 0;

  return UTEST_SUCCESS;
}

// -----[ test_cli_done ]--------------------------------------------
int test_cli_done()
{
  cli_destroy(&pCli);
  pCli= NULL;

  return UTEST_SUCCESS;
}

// -----[ test_cli_basic ]-------------------------------------------
/**
 * Test basic use.
 */
int test_cli_basic()
{
  test_cli_init();

  // Standard call for cmd2
  ASSERT_RETURN(cli_execute(pCli, "test cmd2 arg1")
		== CLI_SUCCESS, "could not execute command");

  // Standard call for cmd2_fail => should fail
  ASSERT_RETURN(cli_execute(pCli, "test cmd2_fail arg1")
		== CLI_ERROR_COMMAND_FAILED,
		"returned wrong error code");
  return UTEST_SUCCESS;
}

// -----[ test_cli_varargs ]-----------------------------------------
/**
 * Test use with varargs
 */
int test_cli_varargs()
{
  // Call cmd1 with 3 varargs
  ASSERT_RETURN(cli_execute(pCli, "test cmd1 A1 A2 VA1 VA2 VA3")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd1 with no varargs (this is allowed: [0-N])
  ASSERT_RETURN(cli_execute(pCli, "test cmd1 A1 A2")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd1 with too many varargs
  ASSERT_RETURN(cli_execute(pCli, "test cmd1 A1 A2 VA1 VA2 VA3 VA4 VA5 VA6")
		== CLI_ERROR_TOO_MANY_PARAMS,
		"wrong error reported");

  return UTEST_SUCCESS;
}

// -----[ test_cli_options ]-----------------------------------------
/**
 * Test with options
 */
int test_cli_options()
{
  // Call cmd1 with unknown option
  ASSERT_RETURN(cli_execute(pCli, "test cmd1 --plop A1 A2 VA1")
		== CLI_ERROR_UNKNOWN_OPTION, "wrong error reported");
  // Call cmd3 with option
  ASSERT_RETURN(cli_execute(pCli, "test cmd3 --plop A1")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd3 with option
  ASSERT_RETURN(cli_execute(pCli, "test cmd3 --plop=\"AC & DC\" A1")
		== CLI_SUCCESS, "could not execute command");

  return UTEST_SUCCESS;
}

// -----[ test_cli_context ]-----------------------------------------
/**
 * Test with context
 */
int test_cli_context()
{
  // Exit (without context)
  ASSERT_RETURN(cli_execute(pCli, "exit")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd4/cmd5
  ASSERT_RETURN(cli_execute(pCli, "test cmd4 cmd5")
		== CLI_SUCCESS, "could not execute command");
  // Enter context cmd4
  ASSERT_RETURN(cli_execute(pCli, "test cmd4")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd5 (from context cmd4)
  ASSERT_RETURN(cli_execute(pCli, "cmd5")
		== CLI_SUCCESS, "could not execute command");
  // Exit context
  ASSERT_RETURN(cli_execute(pCli, "exit")
		== CLI_SUCCESS, "could not execute command");

  test_cli_done();

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_HASH
/////////////////////////////////////////////////////////////////////

typedef struct _HashItem {
  uint32_t uNbr;
}SHashItem;

// -----[ _hash_cmp ]------------------------------------------------
int _hash_cmp(void * pElt1, void * pElt2, unsigned int uEltSize)
{
  SHashItem * pItem1 = (SHashItem *)pElt1;
  SHashItem * pItem2 = (SHashItem *)pElt2;

  if (pItem1->uNbr > pItem2->uNbr) {
    return 1;
  } else if (pItem1->uNbr < pItem2->uNbr) {
    return -1;
  }
  return 0;
}

// -----[ _hash_destroy ]-------------------------------------------
void _hash_destroy(void * pElt)
{
  SHashItem * pItem = (SHashItem *)pElt;
  
  if (pItem) {
    FREE(pItem);
  }
}


// -----[ _hash_fct ]------------------------------------------------
uint32_t _hash_fct(const void * pElt, const uint32_t uHashSize)
{
  SHashItem * pItem = (SHashItem *)pElt;

  return (pItem->uNbr) % uHashSize;
}

// -----[ _hash_for_each ]-------------------------------------------
int _hash_for_each(void * pElt, void * pContext)
{
  fprintf(stderr, "for-each-item: %d\n", (unsigned int) pElt);
  return 0;
}

//
int test_hash_creation_destruction()
{
  SHash * pHash;

  pHash= hash_init(6, 0, _hash_cmp, _hash_destroy, _hash_fct);
  ASSERT_RETURN(pHash, "hash can't be created");
  hash_destroy(&pHash);
  ASSERT_RETURN(!pHash, "hash isn't well destroyed");
  pHash = hash_init(10, 0.75, _hash_cmp, _hash_destroy, _hash_fct);
  ASSERT_RETURN(pHash, "hash can't be created");
  hash_destroy(&pHash);
  ASSERT_RETURN(!pHash, "hash isn't well destroyed");

  return UTEST_SUCCESS;
}

int test_hash_insertion_search_deletion()
{
  SHash * pHash;
  uint32_t uNbr;
  SHashItem * pItem;

  /* static test */
  pHash= hash_init(6, 0, _hash_cmp, _hash_destroy, _hash_fct);
  for (uNbr = 0; uNbr < 20; uNbr++) {
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_add(pHash, pItem) != NULL , "%d can't be inserted (static)", uNbr);
  }
  for (uNbr = 0; uNbr < 20; uNbr++) {
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_search(pHash, pItem), "%d can't be found (static)", uNbr);
    FREE(pItem);
  }
  for (uNbr = 0; uNbr < 20; uNbr++) {
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_del(pHash, pItem) == 2, "%d hasn't been deleted (static)", uNbr);
    FREE(pItem);
  }
  pItem = MALLOC(sizeof(SHashItem));
  pItem->uNbr = 0;
  ASSERT_RETURN(hash_del(pHash, pItem) == 0, "0 should not be present (static)");
  FREE(pItem);
  hash_destroy(&pHash);

  /* dynamic test */
  pHash = hash_init(6, 0.75, _hash_cmp, _hash_destroy, _hash_fct);
 /* for (uNbr = 0; uNbr < 20; uNbr++) {
    printf("insert %d\n", uNbr);
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_add(pHash, pItem) != -1, "%d can't be inserted (dynamic)", uNbr);
  }
  for (uNbr = 0; uNbr < 20; uNbr++) {
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_search(pHash, pItem), "%d can't be found (static)", uNbr);
    FREE(pItem);
  }
  for (uNbr = 0; uNbr < 20; uNbr++) {
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_del(pHash, pItem) == 2, "%d hasn't been deleted (dynamic)", uNbr);
    FREE(pItem);
  }
  pItem = MALLOC(sizeof(SHashItem));
  pItem->uNbr = 0;
  ASSERT_RETURN(hash_del(pHash, pItem) == 0, "0 shoud not be present (dynamic)");
  FREE(pItem);*/
  hash_destroy(&pHash);

  return UTEST_SUCCESS;
}

int test_hash_reference()
{
  SHash * pHash;
  uint32_t uNbr, uCpt;
  SHashItem * pItem;

  /* static test */
  pHash= hash_init(6, 0, _hash_cmp, _hash_destroy, _hash_fct);
  for (uCpt = 0; uCpt < 10; uCpt++) {
    for (uNbr = 0; uNbr < 20; uNbr++) {
      pItem = MALLOC(sizeof(SHashItem));
      pItem->uNbr = uNbr;
      ASSERT_RETURN(hash_add(pHash, pItem) != NULL, "%d can't be inserted (static)", uNbr);
    }
  }
  for (uCpt = 0; uCpt < 9; uCpt++) {
    for (uNbr = 0; uNbr < 20; uNbr++) {
      pItem = MALLOC(sizeof(SHashItem));
      pItem->uNbr = uNbr;
      ASSERT_RETURN(hash_del(pHash, pItem) == 1, "should be unref (static)");
      FREE(pItem);
    }
  }
  for (uNbr = 0; uNbr < 20; uNbr++) {
    pItem = MALLOC(sizeof(SHashItem));
    pItem->uNbr = uNbr;
    ASSERT_RETURN(hash_del(pHash, pItem) == 2, "should be deleted (static)");
    FREE(pItem);
  }


  hash_destroy(&pHash);

  return UTEST_SUCCESS;
}

#define HASH_NITEMS 1024
int HASH_ITEMS_INT[HASH_NITEMS];
char * HASH_ITEMS_STR[HASH_NITEMS];

// -----[ test_hash_init ]-------------------------------------------
int test_hash_init()
{
  unsigned int uIndex, uIndex2;
  unsigned int uLength;
  char acAllowedChars[]= "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

  for (uIndex= 0; uIndex < HASH_NITEMS; uIndex++) {
    // Random number
    HASH_ITEMS_INT[uIndex]= random() % 4096;

    // Random string
    uLength= (random() % 1024)+1;
    HASH_ITEMS_STR[uIndex]= malloc(sizeof(char)*uLength);
    for (uIndex2= 0; uIndex2 < uLength-1; uIndex2++) {
      HASH_ITEMS_STR[uIndex][uIndex2]=
	acAllowedChars[random() % strlen(acAllowedChars)];
    }
    HASH_ITEMS_STR[uIndex][uLength-1]= '\0';
  }

  return UTEST_SUCCESS;
}

// -----[ test_hash_done ]-------------------------------------------
int test_hash_done()
{
  unsigned int uIndex;

  for (uIndex= 0; uIndex < HASH_NITEMS; uIndex++) {
    free(HASH_ITEMS_STR[uIndex]);
  }
  return UTEST_SUCCESS;
}

// -----[ test_hash_old ]--------------------------------------------
int test_hash_old()
{
  /*
  SHash * pHash;
  SEnumerator * pEnum;
  uint32_t uNbr;
  
  // static mode
  pHash= hash_init(6, 0, _hash_cmp, _hash_destroy, _hash_fct);
  ASSERT_RETURN(pHash != NULL, "hash_init() returned NULL pointer");

  for (uNbr = 0; uNbr < 20; uNbr++) {
    hash_add(pHash, (void *) uNbr);
  }
  hash_for_each(pHash, _hash_for_each, NULL);
  pEnum= hash_get_enum(pHash);
  while (enum_has_next(pEnum)) {
    printf("hash-item: %d\n", (unsigned int) enum_get_next(pEnum));
  }
  enum_destroy(&pEnum);
  hash_del(pHash, (void *)1);
  hash_del(pHash, (void *)2);
  hash_del(pHash, (void *)3);
  hash_del(pHash, (void *)5);
  hash_del(pHash, (void *)6);
  hash_del(pHash, (void *)7);
  hash_del(pHash, (void *)8);
  hash_del(pHash, (void *)9);
  hash_del(pHash, (void *)10);
  hash_del(pHash, (void *)11);
  hash_del(pHash, (void *)12);
  hash_del(pHash, (void *)13);
  hash_del(pHash, (void *)14);
  hash_del(pHash, (void *)15);
  hash_del(pHash, (void *)16);
  hash_del(pHash, (void *)17);
  hash_del(pHash, (void *)18);
  printf("*** removal of 19 ***\n");
  hash_del(pHash, (void *)19);
  hash_for_each(pHash, _hash_for_each, NULL);

  printf("*** add a second elt 4 ***\n");
  hash_add(pHash, (void *)4);

  printf("*** removal of 4 ***\n");
  hash_del(pHash, (void *)4);
  hash_for_each(pHash, _hash_for_each, NULL);

  printf("*** add 22 ***\n");
  hash_add(pHash, (void *)22);
  hash_for_each(pHash, _hash_for_each, NULL);

  hash_destroy(&pHash);

  // dynamic mode
  pHash= hash_init(3, 0.75, _hash_cmp, _hash_destroy, _hash_fct);
  for (uNbr = 0; uNbr < 20; uNbr++) {
    hash_add(pHash, (void *) uNbr);
  }
  hash_for_each(pHash, _hash_for_each, NULL);

  hash_destroy(&pHash);
  */
  return UTEST_SUCCESS;
}

// -----[ test_hash_for_each ]---------------------------------------
int test_hash_for_each()
{
  return UTEST_SKIPPED;
}

// -----[ test_hash_enum ]-------------------------------------------
int test_hash_enum()
{
  /*
  SEnumerator * pEnum;

  pEnum= hash_get_enum(pHash);
  while (enum_has_next(pEnum)) {
    printf("hash-item: %d\n", (unsigned int) enum_get_next(pEnum));
  }
  enum_destroy(&pEnum);
  */
  return UTEST_SKIPPED;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_BIT_VECTOR
/////////////////////////////////////////////////////////////////////
#include <libgds/bit_vector.h>
int test_bit_vector_creation_destruction()
{
  SBitVector *pBitVector;

  pBitVector = bit_vector_create(1);
  ASSERT_RETURN(pBitVector != NULL, "creation failed");
  bit_vector_destroy(&pBitVector);

  return UTEST_SUCCESS;
}

int test_bit_vector_manipulations()
{
  SBitVector * pBitVector;

  pBitVector = bit_vector_create(32);
  ASSERT_RETURN(!bit_vector_get(pBitVector, 0), "bit 0 is inaccurate");
  ASSERT_RETURN(!bit_vector_set(pBitVector, 0), "bit 0 not set");
  ASSERT_RETURN(bit_vector_get(pBitVector, 0), "bit 0 is inaccurate");
  ASSERT_RETURN(!bit_vector_set(pBitVector, 1), "bit 1 not set");
  ASSERT_RETURN(!bit_vector_set(pBitVector, 31), "bit 32 not set");
  ASSERT_RETURN(bit_vector_set(pBitVector, 32) == -1, "bit couldn't be set");
  ASSERT_RETURN(bit_vector_get(pBitVector, 1), "bit 1 is inaccurate");

  ASSERT_RETURN(!bit_vector_get(pBitVector, 2), "bit 2 is inaccurate");
  ASSERT_RETURN(!bit_vector_get(pBitVector, 30), "bit 30 is inaccurate");
  ASSERT_RETURN(bit_vector_get(pBitVector, 32) == -1, "bit 32 should be inaccessible");

  ASSERT_RETURN(!bit_vector_unset(pBitVector, 0), "can't unset bit 0");
  ASSERT_RETURN(!bit_vector_get(pBitVector, 0), "bit 0 should be unset");
  ASSERT_RETURN(bit_vector_unset(pBitVector, 32) == -1, "bit should be inaccessible");
  ASSERT_RETURN(!bit_vector_unset(pBitVector, 31), "can't unset bit 31");
  ASSERT_RETURN(!bit_vector_get(pBitVector, 31), "bit 31 should be unset");

  bit_vector_destroy(&pBitVector);
  return UTEST_SUCCESS;
}

void _test_bit_vector_set(SBitVector * pBitVector)
{
  bit_vector_set(pBitVector, 0);
  bit_vector_set(pBitVector, 1);
  bit_vector_set(pBitVector, 3);
  bit_vector_set(pBitVector, 10);
  bit_vector_set(pBitVector, 15);
  bit_vector_set(pBitVector, 23);
  bit_vector_set(pBitVector, 32);
  bit_vector_set(pBitVector, 33);
  bit_vector_set(pBitVector, 34);
  bit_vector_set(pBitVector, 64);
}

void _test_bit_vector_set_xor(SBitVector * pBitVector)
{
  bit_vector_set(pBitVector, 0);
  bit_vector_set(pBitVector, 3);
  bit_vector_set(pBitVector, 10);
  bit_vector_set(pBitVector, 15);
  bit_vector_set(pBitVector, 23);
  bit_vector_set(pBitVector, 27);
  bit_vector_set(pBitVector, 32);
  bit_vector_set(pBitVector, 34);
}

char sBitVector0[] =	      "00000000000000000000000000000000000000000000000000000000000000000";
char sBitVectorInit[] =	      "11010000001000010000000100000000111000000000000000000000000000001";
char sBitVectorXor[] =	      "10010000001000010000000100010000101000000000000000000000000000000";
char sBitVectorXorResult[] =  "01000000000000000000000000010000010000000000000000000000000000001";

int test_bit_vector_binary_operations()
{
  SBitVector * pBitVector1;
  SBitVector * pBitVector2;
  char * sBitVector1;

  /* Legal Operations done on bit vectors of same length */
  /* AND */
  pBitVector1 = bit_vector_create(65);
  pBitVector2 = bit_vector_create(65);
  _test_bit_vector_set(pBitVector1);
  ASSERT_RETURN(bit_vector_and(pBitVector1, pBitVector2) == 0, "'and' operation failed");
  sBitVector1 = bit_vector_to_string(pBitVector1);
  ASSERT_RETURN(strcmp(sBitVector1, sBitVector0) == 0, "'and' operation not conform");
  FREE(sBitVector1);
  bit_vector_destroy(&pBitVector1);
  bit_vector_destroy(&pBitVector2);

  /* OR */
  pBitVector1 = bit_vector_create(65);
  pBitVector2 = bit_vector_create(65);
  _test_bit_vector_set(pBitVector1);
  ASSERT_RETURN(bit_vector_or(pBitVector1, pBitVector2) == 0, "'or' operation failed");
  sBitVector1 = bit_vector_to_string(pBitVector1);
  ASSERT_RETURN(strcmp(sBitVector1, sBitVectorInit) == 0, "'or' operation not conform");
  FREE(sBitVector1);
  bit_vector_destroy(&pBitVector1);
  bit_vector_destroy(&pBitVector2);

  /* XOR */
  pBitVector1 = bit_vector_create(65);
  pBitVector2 = bit_vector_create(65);
  _test_bit_vector_set(pBitVector1);
  _test_bit_vector_set_xor(pBitVector2);
  ASSERT_RETURN(bit_vector_xor(pBitVector1, pBitVector2) == 0, "'xor' operation failed");
  sBitVector1 = bit_vector_to_string(pBitVector1);
  ASSERT_RETURN(strcmp(bit_vector_to_string(pBitVector2), sBitVectorXor) == 0, "'xor' string is inaccurate");
  ASSERT_RETURN(strcmp(sBitVector1, sBitVectorXorResult) == 0, "'xor' operation not conform");
  FREE(sBitVector1);
  bit_vector_destroy(&pBitVector1);
  bit_vector_destroy(&pBitVector2);

  /* Operation done on bit vectors of different lengths */
  pBitVector1 = bit_vector_create(65);
  pBitVector2 = bit_vector_create(3);
  ASSERT_RETURN(bit_vector_and(pBitVector1, pBitVector2) == -1, "'and' operation should have failed (len1 > len2)");
  ASSERT_RETURN(bit_vector_and(pBitVector2, pBitVector1) == -1, "'and' operation should have failed (len1 > len2)");
  ASSERT_RETURN(bit_vector_and(NULL, NULL) == -1, "'and' operation should have failed (NULL, NULL)");
  ASSERT_RETURN(bit_vector_and(pBitVector1, NULL) == -1, "'and' operation should have failed (pBitVector, NULL)");
  ASSERT_RETURN(bit_vector_and(NULL, pBitVector1) == -1, "'and' operation should have failed (NULL, pBitVector)");
  bit_vector_destroy(&pBitVector1);
  bit_vector_destroy(&pBitVector2);

  return UTEST_SUCCESS;
}

int test_bit_vector_representation()
{
  SBitVector * pBitVector1;
  char * sBitVector;

  /* String representation */
  pBitVector1 = bit_vector_create(65);
  _test_bit_vector_set(pBitVector1);
  sBitVector = bit_vector_to_string(pBitVector1);
  ASSERT_RETURN(strcmp(sBitVector, sBitVectorInit) == 0, "to_string failed");
  FREE(sBitVector);

  pBitVector1 = bit_vector_create_from_string(sBitVectorInit);
  sBitVector = bit_vector_to_string(pBitVector1);
  ASSERT_RETURN(strcmp(sBitVector, sBitVectorInit) == 0, "from_string failed");
  FREE(sBitVector);

  bit_vector_destroy(&pBitVector1);
  return UTEST_SUCCESS;
}

int test_bit_vector_equality()
{
  SBitVector * pBitVector1;
  SBitVector * pBitVector2;

  pBitVector1 = bit_vector_create(65);
  pBitVector2 = bit_vector_create(65);

  /* Same lengths */
  _test_bit_vector_set(pBitVector1);
    /* same value */
    _test_bit_vector_set(pBitVector2);
    ASSERT_RETURN(bit_vector_comp(pBitVector1, pBitVector2) == 0, "bit vectors should be equals");
    ASSERT_RETURN(bit_vector_equals(pBitVector1, pBitVector2) == 1, "bit vector should be equals");
    /* greater value */
    _test_bit_vector_set_xor(pBitVector2);
    ASSERT_RETURN(bit_vector_comp(pBitVector2, pBitVector1) == 1, "equality result should be greater");
    ASSERT_RETURN(bit_vector_equals(pBitVector1, pBitVector2) == 0, "bit vector should be equals");
    /* smaller value */
    ASSERT_RETURN(bit_vector_comp(pBitVector1, pBitVector2) == -1, "equality result should be smaller");

  /* Different lengths */
  bit_vector_destroy(&pBitVector2);
  pBitVector2 = bit_vector_create(3);
  

  bit_vector_destroy(&pBitVector1);
  bit_vector_destroy(&pBitVector2);

  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_BLOOM_FILTER
/////////////////////////////////////////////////////////////////////
#include <libgds/sha1.h>
#include <libgds/bloom_hash.h>
#include <libgds/bloom_filter.h>

static char *msg[] = {
    "abc",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    NULL
};

static char *val[] = {
    "a9993e364706816aba3e25717850c26c9cd0d89d",
    "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
    "34aa973cd4c4daa4f61eeb2bdbad27316534016f"
};

static uint8_t uResByte[] = {
  0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78,
  0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d
};

/**
 * those are the standard FIPS-180-1 (sha1) test vectors
 */
int sha1_check()
{
    int i, j;
    char output[41];
    SSHA1Context ctx;
    unsigned char buf[1000];
    unsigned char sha1sum[20];

  for( i = 0; i < 3; i++ ) {
    sha1_starts( &ctx );
    if( i < 2 ) {
        sha1_update( &ctx, (uint8_t *) msg[i],
                     strlen( msg[i] ) );
    } else {
        memset( buf, 'a', 1000 );
        for( j = 0; j < 1000; j++ ) {
            sha1_update( &ctx, (uint8_t *) buf, 1000 );
        }
    }
    sha1_finish( &ctx, sha1sum );

    for( j = 0; j < 20; j++ ) {
        snprintf( output + j * 2, 41-j*2, "%02x", sha1sum[j] );
    }
    if( memcmp( output, val[i], 40 ) ) {
        return -1;
    }
  }
  return 0;
}

int _bloom_print_for_each(void * pItem, void * pCtx)
{
  uint32_t uItem = *(uint32_t *) pItem;

  printf("%02x|", uItem);
  return 0;
}

int test_bloom_hash_creation_destruction()
{
  SBloomFilterHash * pBloomHash;

  ASSERT_RETURN(bloom_hash_create(10000000, 21) == NULL, "sha1 signature is 20 byte long. ");
  pBloomHash = bloom_hash_create(10, 20);
  bloom_hash_destroy(&pBloomHash);
  ASSERT_RETURN(pBloomHash==NULL, "Bloom Hash not well destroyed");
  return UTEST_SUCCESS;
}

/**
 *
 */
int test_bloom_hash_insertion()
{
  SBloomFilterHash * pBloomHash;
  SUInt32Array * uArray;
  uint8_t uCpt;
  uint32_t uByte;

  pBloomHash = bloom_hash_create(MAX_UINT32_T, 20);
  uArray = bloom_hash_get(pBloomHash, (uint8_t*)msg[0], strlen(msg[0]));
  for (uCpt = 0; uCpt < 20; uCpt++) {
    _array_get_at((SArray*)uArray, uCpt, &uByte);
    if (uByte != uResByte[uCpt] ) {
      bloom_hash_destroy(&pBloomHash);
      return UTEST_FAILURE;
    }
  }
  bloom_hash_destroy(&pBloomHash);
  return UTEST_SUCCESS;
}

int test_bloom_filter_creation_destruction()
{
  SBloomFilter * pBloomFilter;

  ASSERT_RETURN(bloom_filter_create(1000, 21) == NULL, "bloom filter can't have more than 20 hashes with sha1");
  pBloomFilter = bloom_filter_create(1000, 20);
  bloom_filter_destroy(&pBloomFilter);
  ASSERT_RETURN(pBloomFilter==NULL, "pBloomFilter not well destroyed");
  return UTEST_SUCCESS;
}

int test_bloom_filter_insertion()
{
  SBloomFilter * pBloomFilter;

  pBloomFilter = bloom_filter_create(30, 7);

/*  bloom_filter_add_array(pBloomFilter, (uint8_t**)msg);
  bloom_filter_add(pBloomFilter, (uint8_t*)msg[0], strlen(msg[0]));
  bloom_filter_add(pBloomFilter, (uint8_t*)msg[1], strlen(msg[1]));
  bloom_filter_add(pBloomFilter, (uint8_t*)msg[0], strlen(msg[0]));
  bloom_filter_add(pBloomFilter, (uint8_t*)msg[1], strlen(msg[1]));*/

  bloom_filter_destroy(&pBloomFilter);
  return UTEST_SUCCESS;

}

int test_bloom_filter_membership()
{
  SBloomFilter * pBloomFilter;

  pBloomFilter = bloom_filter_create(30, 9);

/*  bloom_filter_add_array(pBloomFilter, (uint8_t**)msg);
  ASSERT_RETURN(bloom_filter_is_member(pBloomFilter, (uint8_t*)msg[0], strlen(msg[0])), "it should be part of the bloom filter.");
  ASSERT_RETURN(bloom_filter_is_member(pBloomFilter, (uint8_t*)msg[1], strlen(msg[1])), "%s should be part of the bloom filter.", msg[1]);
  ASSERT_RETURN(!bloom_filter_is_member(pBloomFilter, (uint8_t*)"abcd", 4), "abcd should not be part of the bloom filter.");
  ASSERT_RETURN(!bloom_filter_is_member(pBloomFilter, (uint8_t*)"iabcE", 5), "iabcE should not be part of the bloom filter.");*/

  bloom_filter_destroy(&pBloomFilter);

  return UTEST_SUCCESS;
}

int test_bloom_filter_binary_operations()
{
  return UTEST_SKIPPED;
}
/////////////////////////////////////////////////////////////////////
// MAIN PART
/////////////////////////////////////////////////////////////////////

// -----[ definition of suite of tests ]-----------------------------
#define ARRAY_SIZE(A) sizeof(A)/sizeof(A[0])

SUnitTest STRUTILS_TESTS[]= {
  {test_strutils_basic, "basic use"},
  {test_strutils_convert_int, "conversion int"},
  {test_strutils_convert_uint, "conversion unsigned int"},
  {test_strutils_convert_long, "conversion long"},
  {test_strutils_convert_ulong, "conversion unsigned long"},
  {test_strutils_convert_double, "conversion double"},
};
#define STRUTILS_NTESTS ARRAY_SIZE(STRUTILS_TESTS)

SUnitTest FIFO_TESTS[]= {
  {test_fifo_basic, "basic use"},
  {test_fifo_grow, "growable"},
};
#define FIFO_NTESTS ARRAY_SIZE(FIFO_TESTS)

SUnitTest STACK_TESTS[]= {
  {test_stack_basic, "basic use"},
  {test_stack_copy, "copy"},
  {test_stack_equal, "comparison"},
};
#define STACK_NTESTS ARRAY_SIZE(STACK_TESTS)

SUnitTest ARRAY_TESTS[]= {
  {test_array_basic, "basic use"},
  {test_array_access, "access"},
  {test_array_enum, "enum"},
  {test_array_copy, "copy"},
  {test_array_remove, "remove items"},
  {test_array_insert, "insert items"},
  {test_array_sort, "sort"},
  {test_array_sub, "extract sub-array"},
  {test_array_trim, "trim"},
  {test_array_add_array, "add array"},
};
#define ARRAY_NTESTS ARRAY_SIZE(ARRAY_TESTS)

SUnitTest PTRARRAY_TESTS[]= {
  {test_ptr_array, "basic use"},
};
#define PTRARRAY_NTESTS ARRAY_SIZE(PTRARRAY_TESTS)

SUnitTest TOKENIZER_TESTS[]= {
  {test_tokenizer_basic, "basic use"},
  {test_tokenizer_quotes, "quotes"},
  {test_tokenizer_quotes2, "quotes (2)"},
  {test_tokenizer_multiquotes, "multiquotes"},
  {test_tokenizer_braces, "braces"},
  {test_tokenizer_complex, "complex"},
};
#define TOKENIZER_NTESTS ARRAY_SIZE(TOKENIZER_TESTS)

SUnitTest CLI_TESTS[]= {
  {test_cli_basic, "basic use"},
  {test_cli_options, "options"},
  {test_cli_varargs, "varargs"},
  {test_cli_context, "context"},
};
#define CLI_NTESTS ARRAY_SIZE(CLI_TESTS)

SUnitTest PATRICIA_TESTS[]= {
  {test_patricia_insertion, "insertion"},
  {test_patricia_masking, "masking"},
  {test_patricia_exact, "exact-match"},
  {test_patricia_best, "best-match"},
  {test_patricia_update, "update"},
  {test_patricia_update, "replace"},
  {test_patricia_remove, "remove"},
  {test_patricia_enum, "enum"},
};
#define PATRICIA_NTESTS ARRAY_SIZE(PATRICIA_TESTS)

SUnitTest ASSOC_TESTS[]= {
  {test_assoc_basic, "basic use"},
  {test_assoc_for_each, "for-each"},
};
#define ASSOC_NTESTS ARRAY_SIZE(ASSOC_TESTS)

SUnitTest DLLIST_TESTS[]= {
  {test_dllist_basic, "basic use"},  
};
#define DLLIST_NTESTS ARRAY_SIZE(DLLIST_TESTS)

SUnitTest HASH_TESTS[]= {
  {test_hash_creation_destruction, "creation/destruction"},
  {test_hash_insertion_search_deletion, "insertion/deletion" },
  {test_hash_reference, "references/unreferences" },
  {test_hash_old, "old test"},
  {test_hash_for_each, "for-each"},
  {test_hash_enum, "enum"},
};
#define HASH_NTESTS ARRAY_SIZE(HASH_TESTS)

SUnitTest LIST_TESTS[]= {
  {test_list_basic, "basic use"},
};
#define LIST_NTESTS ARRAY_SIZE(LIST_TESTS)

SUnitTest RADIX_TESTS[]= {
  {test_radix_basic, "basic use"},
  {test_radix_old, "old test"},
  {test_radix_old_ipv4, "old IPv4 test"},
  {test_radix_for_each, "for-each"},
  {test_radix_enum, "enum"},
};
#define RADIX_NTESTS ARRAY_SIZE(RADIX_TESTS)

SUnitTest BIT_VECTOR_TESTS[] = {
  { test_bit_vector_creation_destruction, "creation/destruction" },
  { test_bit_vector_representation,	  "to_string/from_string" },
  { test_bit_vector_manipulations,	  "set/unset/get" },
  { test_bit_vector_binary_operations,	  "and/or/xor" },
  { test_bit_vector_equality,		  "equals" }
};
#define BIT_VECTOR_NTESTS ARRAY_SIZE(BIT_VECTOR_TESTS)

SUnitTest BLOOM_HASH_TESTS[] = {
  { test_bloom_hash_creation_destruction, "creation/destruction" },
  { test_bloom_hash_insertion,		  "insertion" }
};
#define BLOOM_HASH_NTESTS ARRAY_SIZE(BLOOM_HASH_TESTS)

SUnitTest BLOOM_FILTER_TESTS[] = {
  { test_bloom_filter_creation_destruction, "creation/destruction" },
  { test_bloom_filter_insertion,	    "insertion" },
  { test_bloom_filter_membership,	    "membership" },
  { test_bloom_filter_binary_operations,    "and/or/xor" }
};
#define BLOOM_FILTER_NTESTS ARRAY_SIZE(BLOOM_FILTER_TESTS)

SUnitTestSuite SUITES[]= {
  {"String-Utilities", STRUTILS_NTESTS, STRUTILS_TESTS},
  {"FIFO", FIFO_NTESTS, FIFO_TESTS},
  {"Stack", STACK_NTESTS, STACK_TESTS},
  {"Array", ARRAY_NTESTS, ARRAY_TESTS},
  {"Pointer-Array", PTRARRAY_NTESTS, PTRARRAY_TESTS},
  {"Associative-Array", ASSOC_NTESTS, ASSOC_TESTS},
  {"List", LIST_NTESTS, LIST_TESTS},
  {"Doubly-Linked-List", DLLIST_NTESTS, DLLIST_TESTS},
  {"Hash-Table", HASH_NTESTS, HASH_TESTS},
  {"Radix-Tree", RADIX_NTESTS, RADIX_TESTS},
  {"Patricia-Tree", PATRICIA_NTESTS, PATRICIA_TESTS},
  {"Tokenizer",TOKENIZER_NTESTS, TOKENIZER_TESTS},
  {"CLI", CLI_NTESTS, CLI_TESTS},
  {"Bit Vector", BIT_VECTOR_NTESTS, BIT_VECTOR_TESTS},
  {"Bloom Hash", BLOOM_HASH_NTESTS, BLOOM_HASH_TESTS},
  {"Bloom Filter", BLOOM_FILTER_NTESTS, BLOOM_FILTER_TESTS}
};
#define NUM_SUITES ARRAY_SIZE(SUITES)

// ----- main -------------------------------------------------------
int main(int argc, char * argv[])
{
  int iResult= 0;

  srandom(2007);
  gds_init(0);
  //gds_init(GDS_OPTION_MEMORY_DEBUG);

  utest_init(0);
  utest_set_user(getenv("USER"));
  utest_set_project(PACKAGE_NAME, PACKAGE_VERSION);
  utest_set_xml_logging("libgds-check.xml");
  iResult= utest_run_suites(SUITES, NUM_SUITES);

  utest_done();

  gds_destroy();

  return (iResult==0?EXIT_SUCCESS:EXIT_FAILURE);
}
