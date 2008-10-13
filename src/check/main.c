// ==================================================================
// @(#)main.c
//
// Generic Data Structures (libgds): validation application.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// $Id$
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
#include <string.h>

#include <libgds/array.h>
#include <libgds/assoc_array.h>
#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/dllist.h>
#include <libgds/enumerator.h>
#include <libgds/fifo.h>
#include <libgds/gds.h>
#include <libgds/hash.h>
#include <libgds/hash_utils.h>
#include <libgds/list.h>
#include <libgds/memory.h>
#include <libgds/params.h>
#include <libgds/patricia-tree.h>
#include <libgds/radix-tree.h>
#include <libgds/stream.h>
#include <libgds/stream_cmd.h>
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

// -----[ test_strutils_create ]-------------------------------------
static int test_strutils_create()
{
  char * s= str_create("Hello");
  ASSERT_RETURN(s != NULL, "str_create() should not return NULL");
  ASSERT_RETURN(strcmp(s, "Hello") == 0,
		"incorrect string content");
  str_destroy(&s);
  ASSERT_RETURN(s == NULL, "destroyed string should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_strutils_create_null ]--------------------------------
static int test_strutils_create_null()
{
  char * s= str_create(NULL);
  ASSERT_RETURN(s != NULL, "str_create() should not return NULL");
  ASSERT_RETURN(strcmp(s, "") == 0,
		"incorrect string content");
  str_destroy(&s);
  ASSERT_RETURN(s == NULL, "destroyed string should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_strutils_lcreate ]------------------------------------
static int test_strutils_lcreate()
{
  char * s= str_lcreate(10);
  ASSERT_RETURN(s != NULL, "str_lcreate() should not return NULL");
  ASSERT_RETURN(*s == '\0', "not NUL-terminated");
  str_destroy(&s);
  ASSERT_RETURN(s == NULL, "destroyed string should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_strutils_append ]-------------------------------------
static int test_strutils_append()
{
  char * s= str_create("Hello");
  s= str_append(&s, " World");
  ASSERT_RETURN(s != NULL, "str_append() should not return NULL");
  ASSERT_RETURN(strcmp(s, "Hello World") == 0,
		"incorrect string content");
  str_destroy(&s);
  return UTEST_SUCCESS;
}

// -----[ test_strutils_append_null ]--------------------------------
static int test_strutils_append_null()
{
  char * s= str_create("Hello");
  s= str_append(&s, NULL);
  ASSERT_RETURN(s != NULL, "str_append() should not return NULL");
  ASSERT_RETURN(strcmp(s, "Hello") == 0,
		"incorrect string content");
  str_destroy(&s);
  return UTEST_SUCCESS;
}

// -----[ test_strutils_append_src_null ]----------------------------
static int test_strutils_append_src_null()
{
  char * s= NULL;
  s= str_append(&s, "World");
  ASSERT_RETURN(s != NULL, "str_append() should not return NULL");
  ASSERT_RETURN(strcmp(s, "World") == 0, "incorrect string content");
  str_destroy(&s);
  return UTEST_SUCCESS;
}

// -----[ test_strutils_prepend ]------------------------------------
static int test_strutils_prepend()
{
  char * pcStr= str_create("World");
  pcStr= str_prepend(&pcStr, "Hello ");
  ASSERT_RETURN(pcStr != NULL, "str_prepend() should not return NULL");
  ASSERT_RETURN(strcmp(pcStr, "Hello World") == 0,
		"incorrect string content");
  str_destroy(&pcStr);
  return UTEST_SUCCESS;
}

// -----[ test_strutils_prepend_null ]-------------------------------
static int test_strutils_prepend_null()
{
  char * pcStr= str_create("World");
  pcStr= str_prepend(&pcStr, NULL);
  ASSERT_RETURN(pcStr != NULL, "str_prepend() should not return NULL");
  ASSERT_RETURN(strcmp(pcStr, "World") == 0,
		"incorrect string content");
  str_destroy(&pcStr);
  return UTEST_SUCCESS;
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
#define DOUBLE_MAX_DELTA 0.0001
  double dValue;

  ASSERT_RETURN(str_as_double("0.123", &dValue) == 0,
		"conversion should succeed");
  dValue-= 0.123;
  ASSERT_RETURN((dValue < DOUBLE_MAX_DELTA) &&
		(dValue > -DOUBLE_MAX_DELTA),
		"incorrect conversion result");
  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_FIFO
/////////////////////////////////////////////////////////////////////

#define FIFO_NITEMS 5
size_t FIFO_ITEMS[FIFO_NITEMS];

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
  gds_fifo_t * fifo;
  unsigned int index;

  test_fifo_init();

  fifo= fifo_create(FIFO_NITEMS, NULL);
  ASSERT_RETURN(fifo != NULL, "fifo_create() returned a NULL pointer");
  
  // Check initial depth (== 0)
  ASSERT_RETURN(fifo_depth(fifo) == 0, "incorrect depth returned");

  // Push data
  for (index= 0; index < FIFO_NITEMS; index++) {
    ASSERT_RETURN(fifo_push(fifo, (void *) FIFO_ITEMS[index]) == 0,
		  "could not push data onto FIFO");
  }

  // Check depth == FIFO_NITEMS
  ASSERT_RETURN(fifo_depth(fifo) == FIFO_NITEMS,
		"incorrect depth returned");

  // Pushing more should fail (growth option not set)
  ASSERT_RETURN(fifo_push(fifo, (void *) 255) != 0,
		"should not allow pushing more than FIFO size");

  // Pop data
  for (index= 0; index < FIFO_NITEMS; index++) {
    ASSERT_RETURN((size_t) fifo_pop(fifo)
		  == FIFO_ITEMS[index],
		  "incorrect value pop'ed");
  }

  fifo_destroy(&fifo);

  return UTEST_SUCCESS;
}

// -----[ test_fifo_grow ]-------------------------------------------
int test_fifo_grow()
{
  gds_fifo_t * fifo;
  unsigned int index;

  fifo= fifo_create(FIFO_NITEMS, NULL);
  ASSERT_RETURN(fifo != NULL, "fifo_create() returned a NULL pointer");

  fifo_set_option(fifo, FIFO_OPTION_GROW_EXPONENTIAL, 1);

  // Push data
  for (index= 0; index < FIFO_NITEMS; index++) {
    ASSERT_RETURN(fifo_push(fifo, (void *) FIFO_ITEMS[index]) == 0,
		  "could not push data onto FIFO");
  }

  // Check depth == FIFO_NITEMS
  ASSERT_RETURN(fifo_depth(fifo) == FIFO_NITEMS,
		"incorrect depth returned");

  // Pushing more should be allowed (growth option set)
  ASSERT_RETURN(fifo_push(fifo, (void *) 255) == 0,
		"should allow pushing more than FIFO size (grow)");

  // Check depth == FIFO_NITEMS+1
  ASSERT_RETURN(fifo_depth(fifo) == FIFO_NITEMS+1,
		"incorrect depth returned");

  fifo_destroy(&fifo);

  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_STACK
/////////////////////////////////////////////////////////////////////

#define STACK_NITEMS 5
size_t STACK_ITEMS[STACK_NITEMS];

// -----[ _test_stack_init ]-----------------------------------------
static int _test_stack_init()
{
  unsigned int index;

  // Initialize
  for (index= 0; index < STACK_NITEMS; index++) {
    STACK_ITEMS[index]= random() % 4096;
  }
  
  return UTEST_SUCCESS;
}

// -----[ test_stack_create ]----------------------------------------
static int test_stack_create()
{
  gds_stack_t * stack= stack_create(STACK_NITEMS);
  ASSERT_RETURN(stack != NULL, "stack_create() should not return NULL");
  ASSERT_RETURN(stack_depth(stack) == 0, "incorret stack depth");
  ASSERT_RETURN(stack_is_empty(stack), "stack should be empty");
  stack_destroy(&stack);
  ASSERT_RETURN(stack == NULL, "destroyed stack should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_stack_basic ]-----------------------------------------
static int test_stack_basic()
{
  gds_stack_t * stack;
  unsigned int index;

  _test_stack_init();

  stack= stack_create(STACK_NITEMS);
  ASSERT_RETURN(stack != NULL, "stack_create() returned NULL");

  // Push data onto stack
  for (index= 0; index < STACK_NITEMS; index++) {
    ASSERT_RETURN(stack_push(stack, (void *) STACK_ITEMS[index]) == 0,
		  "could not push onto stack (%d)", index);
  }

  // Check length (STACK_NITEMS)
  ASSERT_RETURN(stack_depth(stack) == STACK_NITEMS,
		"incorrect stack length");
  ASSERT_RETURN(stack_is_empty(stack) == 0, "stack should not be empty");

  // Pushing more shouldn't be allowed
  ASSERT_RETURN(stack_push(stack, (void *) 255) != 0,
		"shouldn't allow pushing more than stack size");

  // Pop data from stack
  for (index= 0; index < STACK_NITEMS; index++) {
    ASSERT_RETURN((size_t) stack_pop(stack)
		  == STACK_ITEMS[STACK_NITEMS-index-1],
		  "incorrect value pop'ed");
  }  

  stack_destroy(&stack);

  return UTEST_SUCCESS;
}

// -----[ test_stack_copy ]------------------------------------------
int test_stack_copy()
{
  unsigned int index;
  gds_stack_t * stack1= stack_create(STACK_NITEMS);
  gds_stack_t * stack2;
  _test_stack_init();
  for (index= 0; index < STACK_NITEMS; index++)
    stack_push(stack1, (void *) STACK_ITEMS[index]);
  stack2= stack_copy(stack1);
  ASSERT_RETURN(stack2 != NULL, "stack_copy() should not return NULL");
  ASSERT_RETURN(stack_depth(stack2) == STACK_NITEMS,
		"incorrect stack depth");
  for (index= 0; index < STACK_NITEMS; index++)
    ASSERT_RETURN(stack1->items[index] == stack2->items[index],
		  "stack content is not equal");
  stack_destroy(&stack1);
  stack_destroy(&stack2);
  return UTEST_SUCCESS;
}

// -----[ test_stack_equal ]-----------------------------------------
int test_stack_equal()
{
  unsigned int index;
  gds_stack_t * stack1= stack_create(STACK_NITEMS);
  gds_stack_t * stack2= stack_create(STACK_NITEMS);
  for (index= 0; index < STACK_NITEMS; index++)
    stack_push(stack1, (void *) STACK_ITEMS[index]);
  for (index= 0; index < STACK_NITEMS; index++)
    stack_push(stack2, (void *) STACK_ITEMS[index]);
  ASSERT_RETURN(stack_equal(stack1, stack2), "stacks should be equal");
  if (stack2->items[0] == 0)
    stack2->items[0]= (void *) 1;
  else
    stack2->items[0]= (void *) ((size_t) stack2->items[0]) - 1;
  ASSERT_RETURN(stack_equal(stack1, stack2) == 0,
		"stacks should not be equal");
  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_ENUM
/////////////////////////////////////////////////////////////////////

GDS_ENUM_TEMPLATE_TYPE(int_enum, int);
GDS_ENUM_TEMPLATE_OPS(int_enum, int);

static int _int_enum_has_next(void * ctx) {
  return (*((int *) ctx) > 0);
}

static int _int_enum_get_next(void * ctx) {
  (*((int *) ctx))--;
  return 55;
}

static void _int_enum_destroy(void * ctx) {
}

// -----[ test_enum_template ]---------------------------------------
int test_enum_template()
{
  int value= 2;
  int_enum_t * enu= int_enum_create(&value,
				    _int_enum_has_next,
				    _int_enum_get_next,
				    _int_enum_destroy);
  ASSERT_RETURN(int_enum_has_next(enu),
		"enum_has_next() should succeed");
  ASSERT_RETURN(int_enum_get_next(enu) == 55,
		"enum_get_next() returned wrong value");
  ASSERT_RETURN(int_enum_has_next(enu),
		"enum_has_next() should succeed");
  ASSERT_RETURN(int_enum_get_next(enu) == 55,
		"enum_get_next() returned wrong value");
  ASSERT_RETURN(!int_enum_has_next(enu),
		"enum_has_next() should fail");
  int_enum_destroy(&enu);
  return UTEST_SUCCESS;
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
static int _test_array_compare(const void * item1, const void * item2,
			       unsigned int elt_size)
{
  if (*((int *) item1) > *((int *) item2))
    return 1;
  else if (*((int *) item1) < *((int *) item2))
    return -1;
  return 0;
  }

#define ARRAY_NITEMS 1024
int ARRAY_ITEMS[ARRAY_NITEMS];
#define ARRAY_NITEMS2 512
  int ARRAY_ITEMS2[ARRAY_NITEMS2];

// -----[ _init_random_int_array ]-----------------------------------
static inline void _init_random_int_array(int array[],
					  unsigned int size)
{
  unsigned int index;
  int value, acc;

  // Generate random sequence of unique integers
  acc= 0;
  for (index= 0; index < size; index++) {
    value= (random() % 4096)+1;
    acc+= value;
    array[index]= acc;
  }

  // Shuffle in order to avoid ascending order
  int_array_shuffle(array, size);
}

// -----[ _random_int_array_create ]---------------------------------
static inline int_array_t * _random_int_array_create(int array[],
						     unsigned int size)
{
  int_array_t * new_array= int_array_create(0);
  unsigned int index;

  assert(new_array != NULL);
  for (index= 0; index < size; index++)
    assert(int_array_add(new_array, array[index]) == index);
  return new_array;
}

// -----[ test_before_array ]----------------------------------------
int test_before_array()
{
  _init_random_int_array(ARRAY_ITEMS, ARRAY_NITEMS);
  _init_random_int_array(ARRAY_ITEMS2, ARRAY_NITEMS2);
  return UTEST_SUCCESS;
}

// -----[ test_array_create ]----------------------------------------
int test_array_create()
{
  int_array_t * array= int_array_create(0);
  ASSERT_RETURN(array != NULL, "int_array_create() returned NULL pointer");
  ASSERT_RETURN(int_array_size(array) == 0, "array size should be 0");
  int_array_destroy(&array);
  ASSERT_RETURN(array == NULL, "destroyed array should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_array_basic ]-----------------------------------------
/**
 * Perform basic tests with arrays of integers: add, length, get.
 */
int test_array_basic()
{
  unsigned int index;
  int_array_t * array= int_array_create(0);
  // Add all items to the array
  for (index= 0; index < ARRAY_NITEMS; index++) {
    // int_array_add() must return the index of insertion
    ASSERT_RETURN(int_array_add(array, ARRAY_ITEMS[index]) == index,
		  "int_array_add() returned an incorrect insertion index")
  }
  ASSERT_RETURN(ARRAY_NITEMS == int_array_size(array),
		"int_array_size() returned an incorrect size");
  // Direct access (as a C array)
  for (index= 0; index < ARRAY_NITEMS; index++) {
    ASSERT_RETURN(array->data[index] == ARRAY_ITEMS[index],
		  "direct read did not return expected value");
  }
  int_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_array_enum ]------------------------------------------
int test_array_enum()
{
  int value;
  unsigned int index;
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  gds_enum_t * enu;
  enu= int_array_get_enum(array);
  ASSERT_RETURN(enu != NULL, "int_array_get_enum() returned NULL pointer");
  index= 0;
  while (enum_has_next(enu)) {
    value= (int) enum_get_next(enu);
    ASSERT_RETURN(value == ARRAY_ITEMS[index],
		  "enumerator returned incorrect element");
    index++;
  }
  ASSERT_RETURN(index == ARRAY_NITEMS,
		"enumerator did not traverse whole array");
  enum_destroy(&enu);
  int_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_array_copy ]------------------------------------------
int test_array_copy()
{
  unsigned int index;
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  int_array_t * array_copy;
  array_copy= int_array_copy(array);
  ASSERT_RETURN(array_copy != NULL,
		"int_array_copy() returned NUL pointer");
  ASSERT_RETURN(int_array_size(array_copy) == int_array_size(array),
		"length of copied and original arrays did not match");
  for (index= 0; index < int_array_size(array_copy); index++) {
    ASSERT_RETURN(array->data[index] == array_copy->data[index],
		  "data in copied and original arrays did not match");
  }
  int_array_destroy(&array);
  int_array_destroy(&array_copy);
  return UTEST_SUCCESS;
}

// -----[ test_array_remove ]----------------------------------------
int test_array_remove()
{
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  unsigned int index;
  // Remove (remove elements with odd indices)
  for (index= 0; index < ARRAY_NITEMS/2; index++) {
    ASSERT_RETURN(int_array_remove_at(array, index+1) == 0,
		  "could not remove item at %d", index);
  }
  ASSERT_RETURN(int_array_size(array) == ARRAY_NITEMS-(ARRAY_NITEMS/2),
		"int_array_size() returned an invalid value");
  int_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_array_insert ]----------------------------------------
/**
 * Insert back removed items
 */
int test_array_insert()
{
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  unsigned int index;
  // Remove (remove elements with odd indices)
  for (index= 0; index < ARRAY_NITEMS/2; index++) {
    ASSERT_RETURN(int_array_remove_at(array, index+1) == 0,
		  "could not remove item at %d", index);
  }  
  // Re-insert removed elements (at their previous position)
  for (index= 0; index < ARRAY_NITEMS/2; index++) {
    ASSERT_RETURN(int_array_insert_at(array,
				      index*2+1, &ARRAY_ITEMS[index*2+1])
		  == index*2+1,
		  "int_array_insert_at() returned an incorrect value");
  }
  ASSERT_RETURN(int_array_size(array) == ARRAY_NITEMS,
		"int_array_size() returned an incorrect value");
  /* Check that the array now contains all items */
  for (index= 0; index < ARRAY_NITEMS; index++) {
    ASSERT_RETURN(array->data[index] == ARRAY_ITEMS[index],
		  "direct read did not return expected value");
  }
  int_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_array_sort ]------------------------------------------
int test_array_sort()
{
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  unsigned int index;
  int ARRAY_ITEMS2[ARRAY_NITEMS];
  unsigned int index2;
  int value;
  // Sort (ascending sequence)
  ASSERT_RETURN(int_array_sort(array, _test_array_compare) == 0,
		"incorrect return code for int_array_sort()");
  ASSERT_RETURN(int_array_size(array) == ARRAY_NITEMS,
		"incorrect length returned after int_array_sort()");
  // Check ascending order
  for (index= 0; index < int_array_size(array); index++) {
    if (index > 0) {
      ASSERT_RETURN(array->data[index-1] <= array->data[index],
		    "ascending ordering not respected after _array_sort()");
    }
  }
  // Check int_array_index_of()
  for (index= 0; index < int_array_size(array); index++) {
    ASSERT_RETURN(int_array_index_of(array,
				     array->data[index],
				     &index2) == 0,
		  "incorrect return code for int_array_index_of()");
    ASSERT_RETURN(index2 == index,
		  "incorrect index returned by int_array_index_of()");
  }
  // Insertion in sorted array
  index2= 0;
  for (index= 1; index < int_array_size(array); index++) {
    if (array->data[index]-array->data[index-1] > 1) {
      value= array->data[index-1]+1;
      ARRAY_ITEMS2[index2++]= value;
    }
  }
  for (index= 0; index < index2; index++) {
    ASSERT_RETURN(int_array_add(array, ARRAY_ITEMS2[index]),
		  "could not insert in sorted array");
  }
  ASSERT_RETURN(int_array_size(array) == ARRAY_NITEMS+index2,
		"incorrect length returned (%d vs %d)",
		int_array_size(array), index2);
  // Check ascending order
  for (index= 0; index < int_array_size(array); index++) {
    if (index > 0) {
      ASSERT_RETURN(array->data[index-1] <= array->data[index],
		    "ascending ordering not respected after int_array_sort()");
    }
  }
  int_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_array_sub ]-------------------------------------------
int test_array_sub()
{
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  int_array_t * array_sub;
  unsigned int index;
  array_sub= int_array_sub(array,
			   ARRAY_NITEMS/3,
			   2*(ARRAY_NITEMS/3));
  ASSERT_RETURN(array_sub != NULL,
		"int_array_sub() should not return NULL pointer");
  ASSERT_RETURN(int_array_size(array_sub) != ARRAY_NITEMS/3,
		"Incorrect length for sub-array");
  for (index= 0; index < ARRAY_NITEMS/3; index++) {
    ASSERT_RETURN(array_sub->data[index]
		  == array->data[ARRAY_NITEMS/3+index],
		  "incorrect content in sub-array");
  }
  int_array_destroy(&array);
  int_array_destroy(&array_sub);
  return UTEST_SUCCESS;
}

// -----[ test_array_trim ]------------------------------------------
int test_array_trim()
{
  int_array_t * array= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  int_array_trim(array, ARRAY_NITEMS/2);
  ASSERT_RETURN(int_array_size(array) == ARRAY_NITEMS/2,
		"incorrect length for trimmed array");
  int_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_array_add_array ]-------------------------------------
int test_array_add_array()
{
  int_array_t * array1= _random_int_array_create(ARRAY_ITEMS, ARRAY_NITEMS);
  int_array_t * array2= _random_int_array_create(ARRAY_ITEMS2, ARRAY_NITEMS2);

  int_array_add_array(array1, array2);

  ASSERT_RETURN(int_array_size(array1) == ARRAY_NITEMS+ARRAY_NITEMS2,
		"incorrect length for new array (%d vs %d)",
		int_array_size(array1), ARRAY_NITEMS+ARRAY_NITEMS2);
  int_array_destroy(&array1);
  int_array_destroy(&array2);
  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_ASSOC
/////////////////////////////////////////////////////////////////////

// -----[ _test_assoc_for_each ]-------------------------------------
static int _test_assoc_for_each(const char * key, void * value,
				void * ctx)
{
  unsigned int * count= (unsigned int *) ctx;
  if (count !=NULL)
    (*count)++;
  return 0;
}

#define ASSOC_NITEMS 3
char * ASSOC_ITEMS[ASSOC_NITEMS][2]= {
  {"key1", "toto1"},
  {"key2", "toto2"},
  {"plop", "grominet"},
};

// -----[ test_assoc_create_destroy ]--------------------------------
int test_assoc_create_destroy()
{
  gds_assoc_array_t * array= assoc_array_create(NULL);
  ASSERT_RETURN(array != NULL, "assoc-array creation should succeed");
  assoc_array_destroy(&array);
  ASSERT_RETURN(array == NULL, "destroyed assoc-array should be NULL");
  return UTEST_SUCCESS;
}


// -----[ test_assoc_basic ]-----------------------------------------
int test_assoc_basic()
{
  gds_assoc_array_t * array= assoc_array_create(NULL);
  unsigned int index;
  // Test for 'set'
  for (index= 0; index < ASSOC_NITEMS; index++) {
    ASSERT_RETURN(assoc_array_set(array, ASSOC_ITEMS[index][0],
				  ASSOC_ITEMS[index][1]) == 0,
		  "could not set \"%s\" => \"%s\"",
		  ASSOC_ITEMS[index][0],
		  ASSOC_ITEMS[index][1]);
  }
  // Test content
  for (index= 0; index < ASSOC_NITEMS; index++) {
    ASSERT_RETURN(assoc_array_get(array, ASSOC_ITEMS[index][0])
		  == ASSOC_ITEMS[index][1],
		  "incorrect value returned for \"%s\"");
  }

  // Test for 'exists'
  for (index= 0; index < ASSOC_NITEMS; index++) {
    ASSERT_RETURN(assoc_array_exists(array, ASSOC_ITEMS[index][0]) == 1,
		  "existence test failed for \"%s\"",
		  ASSOC_ITEMS[index][0]);
  }
  ASSERT_RETURN(!assoc_array_exists(array, "plopsaland"),
		"should not report existence for missing key");
  assoc_array_set(array, "key2", "titi"); 
  ASSERT_RETURN(!strcmp(assoc_array_get(array, "key2"), "titi"),
		"incorrect value associated to \"key2\"");
  assoc_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_assoc_for_each ]--------------------------------------
static int test_assoc_for_each()
{
  gds_assoc_array_t * array= assoc_array_create(NULL);
  unsigned int index;
  unsigned int count= 0;
  for (index= 0; index < ASSOC_NITEMS; index++)
    assoc_array_set(array, ASSOC_ITEMS[index][0], ASSOC_ITEMS[index][1]);
  ASSERT_RETURN(assoc_array_for_each(array, _test_assoc_for_each, &count) == 0,
		"assoc-array for-each should succeed");
  ASSERT_RETURN(count == ASSOC_NITEMS,
		"assoc-array for-each missed items (%d vs %d)",
		count, ASSOC_NITEMS);
  assoc_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_assoc_enum_keys ]-------------------------------------
static int test_assoc_enum_keys()
{
  gds_assoc_array_t * array= assoc_array_create(NULL);
  unsigned int index, count= 0;
  gds_enum_t * enu;
  for (index= 0; index < ASSOC_NITEMS; index++)
    assoc_array_set(array, ASSOC_ITEMS[index][0], ASSOC_ITEMS[index][1]);
  enu= assoc_array_get_keys_enum(array);
  while (enum_has_next(enu)) {
    enum_get_next(enu);
    count++;
  }
  ASSERT_RETURN(count == ASSOC_NITEMS, "incorrect number of items enumerated");
  assoc_array_destroy(&array);
  return UTEST_SUCCESS;
}

// -----[ test_assoc_enum_values ]-----------------------------------
static int test_assoc_enum_values()
{
  gds_assoc_array_t * array= assoc_array_create(NULL);
  unsigned int index, count= 0;
  gds_enum_t * enu;
  for (index= 0; index < ASSOC_NITEMS; index++)
    assoc_array_set(array, ASSOC_ITEMS[index][0], ASSOC_ITEMS[index][1]);
  enu= assoc_array_get_values_enum(array);
  while (enum_has_next(enu)) {
    enum_get_next(enu);
    count++;
  }
  ASSERT_RETURN(count == ASSOC_NITEMS, "incorrect number of items enumerated");
  assoc_array_destroy(&array);
  return UTEST_SUCCESS;
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
int array_ptr_compare_function(const void * pItem1, const void * pItem2,
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
void array_ptr_destroy_function(void * item, const void * ctx)
{
  SPtrArrayItem * pRealItem= *((SPtrArrayItem **) item);

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
			      array_ptr_destroy_function,
			      NULL);
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

typedef struct my_type_t {
  unsigned int i;
} my_type_t;

static int _my_types_type_cmp(const void * item1, const void * item2,
			      unsigned int size)
{
  my_type_t * var1= *(my_type_t **) item1;
  my_type_t * var2= *(my_type_t **) item2;

  if (var1->i > var2->i)
    return 1;
  else if (var1->i < var2->i)
    return -1;
  return 0;
}

GDS_ARRAY_TEMPLATE(my_types, my_type_t *, 0, NULL, NULL, NULL)
GDS_ARRAY_TEMPLATE(my_types_sorted, my_type_t *,
		   ARRAY_OPTION_SORTED | ARRAY_OPTION_UNIQUE,
		   _my_types_type_cmp, NULL, NULL)
  
static int test_ptr_array_template()
{
  my_types_t * array= my_types_create(0);
  my_type_t var1= { 0 };
  my_type_t var2= { 1 };
  my_type_t var3= { 2 };
  ASSERT_RETURN(array != NULL, "array creation should succeed");
  ASSERT_RETURN(my_types_size(array) == 0, "array length should be 0");
  ASSERT_RETURN(my_types_add(array, &var1) >= 0,
		"array addition should succeed");
  ASSERT_RETURN(my_types_add(array, &var2) >= 0,
		"array addition should succeed");
  ASSERT_RETURN(my_types_add(array, &var3) >= 0,
		"array addition should succeed");
  ASSERT_RETURN(my_types_size(array) == 3, "array length should be 3");
  ASSERT_RETURN(array->data[0]->i == 0, "item 0 should contain 0");
  ASSERT_RETURN(array->data[1]->i == 1, "item 0 should contain 1");
  ASSERT_RETURN(array->data[2]->i == 2, "item 0 should contain 2");
  ASSERT_RETURN(my_types_remove_at(array, 1) >= 0,
		"array removal should succeed");
  ASSERT_RETURN(my_types_size(array) == 2, "array length should be 2");  
  my_types_destroy(&array);
  ASSERT_RETURN(array == NULL, "destroyed array should be NULL");
  return UTEST_SUCCESS;
}

static int test_ptr_array_template_sorted()
{
  my_types_sorted_t * array= my_types_sorted_create(0);
  my_type_t var1= { 2 };
  my_type_t var2= { 1 };
  my_type_t var3= { 0 };
  ASSERT_RETURN(array != NULL, "array creation should succeed");
  ASSERT_RETURN(my_types_sorted_size(array) == 0, "array length should be 0");
  ASSERT_RETURN(my_types_sorted_add(array, &var1) >= 0,
		"array addition should succeed");
  ASSERT_RETURN(my_types_sorted_add(array, &var2) >= 0,
		"array addition should succeed");
  ASSERT_RETURN(my_types_sorted_add(array, &var3) >= 0,
		"array addition should succeed");
  ASSERT_RETURN(my_types_sorted_size(array) == 3, "array length should be 3");
  ASSERT_RETURN(array->data[0]->i == 0, "item 0 should contain 0");
  ASSERT_RETURN(array->data[1]->i == 1, "item 0 should contain 1");
  ASSERT_RETURN(array->data[2]->i == 2, "item 0 should contain 2");
  ASSERT_RETURN(my_types_sorted_remove_at(array, 1) >= 0,
		"array removal should succeed");
  ASSERT_RETURN(my_types_sorted_size(array) == 2, "array length should be 2");  
  my_types_sorted_destroy(&array);
  ASSERT_RETURN(array == NULL, "destroyed array should be NULL");
  return UTEST_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_LIST
/////////////////////////////////////////////////////////////////////

#define LIST_NITEMS 1024
size_t LIST_ITEMS[LIST_NITEMS];

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
    ASSERT_RETURN((size_t) pData == uIndex,
		  "incorrect value for list_get_index() %d=>%d",
		  (size_t) pData, uIndex);
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
size_t DLLIST_ITEMS[DLLIST_NUM_ITEMS];

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

static unsigned int _radix_destroy_count;

// -----[ _test_radix_destroy ]--------------------------------------
static void _test_radix_destroy(void ** item)
{
  _radix_destroy_count++;
}

#define RADIX_NITEMS 1000
int RADIX_ITEMS[RADIX_NITEMS];
int RADIX_FLAGS[RADIX_NITEMS];

// -----[ test_radix_before ]----------------------------------------
static int test_radix_before()
{
  _init_random_int_array(RADIX_ITEMS, RADIX_NITEMS);
  return UTEST_SUCCESS;
}

// -----[ test_radix_basic ]-----------------------------------------
static int test_radix_basic()
{
  gds_radix_tree_t * tree= radix_tree_create(32, NULL);
  ASSERT_RETURN(tree != NULL, "new radix-tree should not be NULL");
  radix_tree_destroy(&tree);
  ASSERT_RETURN(tree == NULL, "destroyed radix-tree should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_radix_add_remove ]------------------------------------
static int test_radix_add_remove()
{
  gds_radix_tree_t * tree= radix_tree_create(32, NULL);
  void * data;
  ASSERT_RETURN(tree != NULL, "new radix-tree should not be NULL");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(1,0,0,0), 32, (void *) 1) >= 0,
		"adding an item should succeed");
  data= radix_tree_get_exact(tree, IPV4_TO_INT(1,0,0,0), 32);
  ASSERT_RETURN((data != NULL) && ((int) data == 1),
		"retrieving an item should succeed");
  ASSERT_RETURN(radix_tree_remove(tree, IPV4_TO_INT(1,0,0,0), 32, 1) >= 0,
		"removing an item should succeed");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(1,0,0,0), 32) == NULL,
		"retrieving a removed item should fail");
  radix_tree_destroy(&tree);
  ASSERT_RETURN(tree == NULL, "destroyed radix-tree should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_radix_num_nodes ]-------------------------------------
static int test_radix_num_nodes()
{
  gds_radix_tree_t * tree= radix_tree_create(32, NULL);
  unsigned int index;
  unsigned int count;
  for (index= 0; index < RADIX_NITEMS; index++)
    radix_tree_add(tree, RADIX_ITEMS[index], 32, (void *) 1);
  count= radix_tree_num_nodes(tree, 1);
  ASSERT_RETURN(count == RADIX_NITEMS,
		"incorrect number of nodes returned (%d vs %d)",
		RADIX_NITEMS, count);
  radix_tree_destroy(&tree);
  return UTEST_SUCCESS;
}

// -----[ _test_radix_for_each_cb ]----------------------------------
static int _test_radix_for_each_cb(uint32_t key, uint8_t key_len,
				void * data, void * ctx)
{
  int * count= (int *) ctx;
  int value= (int) data;
  unsigned int index;

  for (index= 0; index < RADIX_NITEMS; index++)
    if (RADIX_ITEMS[index] == value) {
      RADIX_FLAGS[index]= 1;
      break;
    }
  if (count != NULL)
    (*count)++;
  return 0;
}

// -----[ test_radix_for_each ]--------------------------------------
static int test_radix_for_each()
{
  gds_radix_tree_t * tree= radix_tree_create(32, NULL);
  unsigned int count= 0;
  unsigned int index;

  // Init radix tree with random data
  for (index= 0; index < RADIX_NITEMS; index++)
    radix_tree_add(tree, (uint32_t) RADIX_ITEMS[index], 32,
		   (void *) RADIX_ITEMS[index]);

  // Traverse the tree using a for-each callback
  memset(RADIX_FLAGS, 0, sizeof(RADIX_FLAGS));
  ASSERT_RETURN(radix_tree_for_each(tree, _test_radix_for_each_cb, &count) == 0,
		"for-each should succeed");
  ASSERT_RETURN(count == RADIX_NITEMS,
		"incorrect number of items returned by for-each (%d vs %d",
		count, RADIX_NITEMS);

  // Check that all items were enumerated
  for (index= 0; index < RADIX_NITEMS; index++)
    ASSERT_RETURN(RADIX_FLAGS[index] == 1,
		  "item @%d, value=%d was not in enumeration",
		  index, RADIX_ITEMS[index]);

  radix_tree_destroy(&tree);
  return UTEST_SUCCESS;
}

// -----[ test_radix_enum ]------------------------------------------
static int test_radix_enum()
{
  gds_radix_tree_t * tree= radix_tree_create(32, NULL);
  unsigned int count= 0;
  unsigned int index;
  int data;
  gds_enum_t * enu;

  // Init radix tree with random data
  for (index= 0; index < RADIX_NITEMS; index++)
    radix_tree_add(tree, (uint32_t) RADIX_ITEMS[index], 32,
		   (void *) RADIX_ITEMS[index]);

  // Traverse the tree using an enumeration
  enu= radix_tree_get_enum(tree);
  memset(RADIX_FLAGS, 0, sizeof(RADIX_FLAGS));
  while (enum_has_next(enu)) {
    data= (int) enum_get_next(enu);
    for (index= 0; index < RADIX_NITEMS; index++)
      if (RADIX_ITEMS[index] == data) {
	RADIX_FLAGS[index]= 1;
	break;
      }
    count++;
  }
  ASSERT_RETURN(count == RADIX_NITEMS,
		"incorrect number of items returned by enumeration (%d vs %d",
		count, RADIX_NITEMS);
  
  // Check that all items were enumerated
  for (index= 0; index < RADIX_NITEMS; index++)
    ASSERT_RETURN(RADIX_FLAGS[index] == 1,
		  "item @%d, value=%d was not in enumeration",
		  index, RADIX_ITEMS[index]);
      
  radix_tree_destroy(&tree);
  return UTEST_SUCCESS;
}

// -----[ test_radix_ipv4 ]------------------------------------------
static int test_radix_ipv4()
{
  gds_radix_tree_t * tree= radix_tree_create(32, _test_radix_destroy);

  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 128, 0, 0), 16,
			       (void *) 100) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 192, 0, 0), 15,
			       (void *) 200) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 0, 0, 0), 16,
			       (void *) 1) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 0, 0, 0), 15,
			       (void *) 2) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 1, 0, 0), 16,
			       (void *) 3) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 2, 0, 0), 16,
			       (void *) 4) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 1, 1, 1), 24,
			       (void *) 5) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 1, 1, 128), 25,
			       (void *) 6) == 0,
		"radix_tree_add() failed");
  ASSERT_RETURN(radix_tree_add(tree, IPV4_TO_INT(0, 128, 128, 128), 9,
			       (void *) 300) == 0,
		"radix_tree_add() failed");

  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 128, 0, 0), 16)
		== (void *) 100,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 192, 0, 0), 15)
		== (void *) 200,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 0, 0, 0), 16)
		== (void *) 1,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 0, 0, 0), 15)
		== (void *) 2,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 1, 0, 0), 16)
		== (void *) 3,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 2, 0, 0), 16)
		== (void *) 4,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 1, 1, 1), 24)
		== (void *) 5,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 1, 1, 128), 25)
		== (void *) 6,
		"radix_tree_get_exact() returned incorrect value");
  ASSERT_RETURN(radix_tree_get_exact(tree, IPV4_TO_INT(0, 128, 128, 128), 9)
		== (void *) 300,
		"radix_tree_get_exact() returned incorrect value");

  radix_tree_destroy(&tree);
  return UTEST_SUCCESS;
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
  gds_tokenizer_t * tokenizer;
  const gds_tokens_t * tokens;
  unsigned int index;
  int result;

  tokenizer= tokenizer_create(" \t", "\"{", "\"}");
  ASSERT_RETURN(tokenizer != NULL,
		"tokenizer_create() returned NULL pointer");

  result= tokenizer_run(tokenizer, "abc def ghi");
  ASSERT_RETURN(result == TOKENIZER_SUCCESS,
		"tokenization of [abc def ghi] failed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 3,
		"wrong number of tokens");
  for (index= 0; index < tokens_get_num(tokens); index++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(tokens, index),
			 pacTokens[index]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(tokens, index),
		  pacTokens[index]);
  }

  tokenizer_destroy(&tokenizer);

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
  gds_tokenizer_t * tokenizer= tokenizer_create(" \t", "\"{", "\"}");
  const gds_tokens_t * tokens;
  unsigned int index;
  int result;

  result= tokenizer_run(tokenizer, "abc \"def\" ghi");
  ASSERT_RETURN(result == TOKENIZER_SUCCESS,
		"tokenization of [abc \"def\" ghi] failed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 3,
		"wrong number of tokens");
  for (index= 0; index < tokens_get_num(tokens); index++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(tokens, index),
			 pacTokens[index]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(tokens, index),
		  pacTokens[index]);
  }

  tokenizer_destroy(&tokenizer);

  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_escape ]------------------------------------
static int test_tokenizer_escape()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ","(", ")");
  ASSERT_RETURN(tokenizer_run(tokenizer, "abcd \\(12 34\\)")
		== TOKENIZER_SUCCESS,
		"should succeed");
  ASSERT_RETURN(tokens_get_num(tokenizer->tokens) == 3,
		"incorrect number of tokens");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_escape_incomplete ]-------------------------
static int test_tokenizer_escape_incomplete()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", NULL, NULL);
  ASSERT_RETURN(tokenizer_run(tokenizer, "ab\\")
		== TOKENIZER_ERROR_ESCAPE,
		"should return incomplete escape-sequence error");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_quotes_missing_close ]----------------------
static int test_tokenizer_quotes_missing_close()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", "(", ")");
  ASSERT_RETURN(tokenizer_run(tokenizer, "abcd (efg")
		== TOKENIZER_ERROR_MISSING_CLOSE,
		"should return missing-close error");
  ASSERT_RETURN(tokenizer_run(tokenizer, "abcd (efg hij")
		== TOKENIZER_ERROR_MISSING_CLOSE,
		"should return missing-close error");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_quotes_missing_open ]-------------------------------
static int test_tokenizer_quotes_missing_open()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", "(", ")");
  ASSERT_RETURN(tokenizer_run(tokenizer, "abcd) efg")
		== TOKENIZER_ERROR_MISSING_OPEN,
		"should return missing-open error");
  ASSERT_RETURN(tokenizer_run(tokenizer, "(abcd) efg) hij")
		== TOKENIZER_ERROR_MISSING_OPEN,
		"should return missing-open error");
  tokenizer_destroy(&tokenizer);
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
  gds_tokenizer_t * tokenizer= tokenizer_create(" \t", "\"{", "\"}");
  const gds_tokens_t * tokens;
  unsigned int index;
  int result;

  result= tokenizer_run(tokenizer, "123 abc\"def\"ghi 456");
  ASSERT_RETURN(result == TOKENIZER_SUCCESS,
		"tokenization of [123 abc\"def\"ghi 456] failed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 3,
		"wrong number of tokens");
  for (index= 0; index < tokens_get_num(tokens); index++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(tokens, index),
			 pacTokens[index]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(tokens, index),
		  pacTokens[index]);
  }

  tokenizer_destroy(&tokenizer);

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
  gds_tokenizer_t * tokenizer= tokenizer_create(" \t", "\"{", "\"}");
  const gds_tokens_t * tokens;
  unsigned int index;
  int result;

  result= tokenizer_run(tokenizer, "123 abc\"def\"ghi { 456 }");
  ASSERT_RETURN(result == TOKENIZER_SUCCESS,
		"tokenization of [123 abc\"def\"ghi { 456 }] failed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 3,
		"wrong number of tokens");
  for (index= 0; index < tokens_get_num(tokens); index++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(tokens, index),
			 pacTokens[index]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(tokens, index),
		  pacTokens[index]);
  }

  tokenizer_destroy(&tokenizer);

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
  gds_tokenizer_t * tokenizer= tokenizer_create(" \t", "\"{", "\"}");
  const gds_tokens_t * tokens;
  unsigned int index;
  int result;

  result= tokenizer_run(tokenizer, "  in-filter \"\"\"\"\"\"");
  ASSERT_RETURN(result == TOKENIZER_SUCCESS,
		"tokenization of [  in-filter \"\"\"\"\"\"] failed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 2,
		"wrong number of tokens");
  for (index= 0; index < tokens_get_num(tokens); index++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(tokens, index),
			 pacTokens[index]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(tokens, index),
		  pacTokens[index]);
  }

  tokenizer_destroy(&tokenizer);

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
  gds_tokenizer_t * tokenizer= tokenizer_create(" \t", "\"{", "\"}");
  const gds_tokens_t * tokens;
  unsigned int index;
  int result;

  result= tokenizer_run(tokenizer, acString);
  ASSERT_RETURN(result == TOKENIZER_SUCCESS,
		"tokenization of [%s] failed", acString);
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 7,
		"wrong number of tokens");
  for (index= 0; index < tokens_get_num(tokens); index++) {
    ASSERT_RETURN(strcmp(tokens_get_string_at(tokens, index),
			 pacTokens[index]) == 0,
		  "incorrect value in token (\"%s\" vs \"%s\")",
		  tokens_get_string_at(tokens, index),
		  pacTokens[index]);
  }

  tokenizer_destroy(&tokenizer);

  return UTEST_SUCCESS;
}

static const char * _params_lookup(const char * param, void * ctx) {
  if (!strcmp(param, "VAR1")) {
    return "Hello";
  } else if (!strcmp(param, "VAR2")) {
    return "World";
  } else if (!strcmp(param, "VAR_3")) {
    return "!!!";
  }
  return NULL;
}
static param_lookup_t _param_lookup= {
  .lookup= _params_lookup,
  .ctx   = NULL
};

// -----[ test_tokenizer_params ]------------------------------------
static int test_tokenizer_params()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", "(", ")");
  tokenizer_set_lookup(tokenizer, _param_lookup);
  ASSERT_RETURN(tokenizer_run(tokenizer, "Yop: $VAR1$VAR2 $VAR_3")
		== TOKENIZER_SUCCESS,
		"tokenizing should succeed");
  ASSERT_RETURN(tokens_get_num(tokenizer_get_tokens(tokenizer)) == 3,
		"wrong number of tokens");
  ASSERT_RETURN(tokenizer_run(tokenizer, "Yop: ($VAR1$VAR2 $VAR_3)")
		== TOKENIZER_SUCCESS,
		"tokenizing should succeed");
  ASSERT_RETURN(tokens_get_num(tokenizer_get_tokens(tokenizer)) == 2,
		"wrong number of tokens");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_params_undef ]------------------------------
static int test_tokenizer_params_undef()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", "(", ")");
  tokenizer_set_lookup(tokenizer, _param_lookup);
  ASSERT_RETURN(tokenizer_run(tokenizer, "$VAR4")
		== TOKENIZER_ERROR_PARAM_UNDEF,
		"tokenizing should fail with undef-param error");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_params_invalid ]----------------------------
static int test_tokenizer_params_invalid()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", "(", ")");
  tokenizer_set_lookup(tokenizer, _param_lookup);
  ASSERT_RETURN(tokenizer_run(tokenizer, "$1234")
		== TOKENIZER_ERROR_PARAM_INVALID,
		"tokenizing should fail with invalid-param error");
  ASSERT_RETURN(tokenizer_run(tokenizer, "$$1234")
		== TOKENIZER_ERROR_PARAM_INVALID,
		"tokenizing should fail with invalid-param error");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_opt_single_delim ]--------------------------
static int test_tokenizer_opt_single_delim()
{
  gds_tokenizer_t * tokenizer= tokenizer_create("|", NULL, NULL);
  const gds_tokens_t * tokens;
  const char * str= "1|2||3";
  ASSERT_RETURN(tokenizer_run(tokenizer, str) == TOKENIZER_SUCCESS,
		"tokenizer_run() should succeed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 3,
		"incorrect number of tokens");
  tokenizer_set_flag(tokenizer, TOKENIZER_OPT_SINGLE_DELIM);
  ASSERT_RETURN(tokenizer_run(tokenizer, str) == TOKENIZER_SUCCESS,
		"tokenizer_run() should succeed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 4,
		"incorrect number of tokens");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_opt_empty_final ]---------------------------
static int test_tokenizer_opt_empty_final()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", NULL, NULL);
  const char * str= "net node 1.0.0.0 ";
  const gds_tokens_t * tokens;
  ASSERT_RETURN(tokenizer_run(tokenizer, str) == TOKENIZER_SUCCESS,
		"tokenizer_run() should succeed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 3,
		"incorrect number of tokens");
  tokenizer_set_flag(tokenizer, TOKENIZER_OPT_EMPTY_FINAL);
  ASSERT_RETURN(tokenizer_run(tokenizer, str) == TOKENIZER_SUCCESS,
		"tokenizer_run() should succeed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(tokens_get_num(tokens) == 4,
		"incorrect number of tokens");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}

// -----[ test_tokenizer_protected_block ]---------------------------
static int test_tokenizer_protected_block()
{
  gds_tokenizer_t * tokenizer= tokenizer_create(" ", "\"'", "\"'");
  const gds_tokens_t * tokens;
  tokenizer_set_protect_quotes(tokenizer, "'");
  tokenizer_set_lookup(tokenizer, _param_lookup);
  ASSERT_RETURN(tokenizer_run(tokenizer, "\"$VAR1\"") == TOKENIZER_SUCCESS,
		"tokenizer_run() should succeed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(!strcmp(tokens_get_string_at(tokens, 0), "Hello"),
		"incorrect token value");
  ASSERT_RETURN(tokenizer_run(tokenizer, "'$VAR1'") == TOKENIZER_SUCCESS,
		"tokenizer_run() should succeed");
  tokens= tokenizer_get_tokens(tokenizer);
  ASSERT_RETURN(!strcmp(tokens_get_string_at(tokens, 0), "$VAR1"),
		"incorrect token value");
  tokenizer_destroy(&tokenizer);
  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_PARAMS
/////////////////////////////////////////////////////////////////////

// -----[ test_params_basic ]----------------------------------------
static int test_params_basic()
{
  char * replaced;
  ASSERT_RETURN(params_replace("Here is well-known: $VAR1 $VAR2$VAR_3",
			       _param_lookup, &replaced)
		== PARAMS_SUCCESS, "params_replace() should succeed");
  ASSERT_RETURN(replaced != NULL, "replaced string should not be NULL");
  ASSERT_RETURN(strcmp(replaced, "Here is well-known: Hello World!!!") == 0,
		"replaced string does not match (%s)", replaced);
  str_destroy(&replaced);
  return UTEST_SUCCESS;
}

// -----[ test_params_invalid ]--------------------------------------
static int test_params_invalid()
{
  char * replaced;
  ASSERT_RETURN(params_replace("replace $12TOTO",
			       _param_lookup, &replaced)
		== PARAMS_ERROR_INVALID,
		"params_replace() should fail with \"invalid param. name\"");
  return UTEST_SUCCESS;
}

// -----[ test_params_undef ]----------------------------------------
static int test_params_undef()
{
  char * replaced;
  ASSERT_RETURN(params_replace("replace $TOTO",
			       _param_lookup, &replaced)
		== PARAMS_ERROR_UNDEF,
		"params_replace() should fail with \"undefined parameter\"");
  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_PATRICIA_TREE
/////////////////////////////////////////////////////////////////////

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
void dump_exact(gds_trie_t * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
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
void dump_best(gds_trie_t * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  void * pData= trie_find_best(pTrie, uKey, uKeyLen);
  printf("best %s/%u => ", INT_TO_IPV4(uKey), uKeyLen);
  if (pData == NULL)
    printf("(null)\n");
  else
    printf("%u\n", (int) pData);
}
  */

gds_trie_t * trie= NULL;
#define TRIE_NITEMS 9
typedef struct _trie_item_t {
  size_t  key;
  uint8_t key_len;
  size_t  data;
  size_t  x;
} _trie_item_t;
_trie_item_t TRIE_ITEMS[TRIE_NITEMS]=  {
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
int TRIE_FLAGS[TRIE_NITEMS];
int _trie_destroy_count;

// -----[ _trie_destroy ]--------------------------------------------
static void _trie_destroy(void ** data)
{
  _trie_destroy_count++;
}

// -----[ test_trie_create_destroy ]---------------------------------
static int test_trie_create_destroy()
{
  gds_trie_t * trie= trie_create(NULL);
  ASSERT_RETURN(trie != NULL, "trie_create() should succeed");
  trie_destroy(&trie);
  ASSERT_RETURN(trie == NULL, "destroyed trie should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_trie_insert ]-----------------------------------------
static int test_trie_insert()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  trie_key_t key= IPV4_TO_INT(1,2,3, 0);
  trie_key_len_t key_len= 24;
  
  _trie_destroy_count= 0;
  ASSERT_RETURN(trie_insert(trie, key, key_len, (void *) 1234, 0)
		== TRIE_SUCCESS,
		"trie_insert() should succeed");
  ASSERT_RETURN(trie_find_exact(trie, key, key_len) == (void *) 1234,
		"trie_find_exact() returned an incorrect value");
  trie_destroy(&trie);
  ASSERT_RETURN(_trie_destroy_count == 1,
		"_trie_destroy() has not been called");
  return UTEST_SUCCESS;
}

// -----[ test_trie_insert_dup ]-------------------------------------
static int test_trie_insert_dup()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  trie_key_t key= IPV4_TO_INT(1,2,3, 0);
  trie_key_len_t key_len= 24;

  _trie_destroy_count= 0;
  ASSERT_RETURN(trie_insert(trie, key, key_len, (void *) 1234, 0)
		== TRIE_SUCCESS,
		"trie_insert() should succeed");
  ASSERT_RETURN(trie_find_exact(trie, key, key_len) == (void *) 1234,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_insert(trie, key, key_len, (void *) 5678, 0)
		== TRIE_ERROR_DUPLICATE,
		"trie_insert(replace=0) should fail (duplicate)");
  ASSERT_RETURN(_trie_destroy_count == 0,
		"_trie_destroy() has not been called");
  ASSERT_RETURN(trie_find_exact(trie, key, key_len) == (void *) 1234,
		"trie_find_exact() returned an incorrect value");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_insert_replace ]-------------------------------------
static int test_trie_insert_replace()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  trie_key_t key= IPV4_TO_INT(1,2,3, 0);
  trie_key_len_t key_len= 24;

  _trie_destroy_count= 0;
  ASSERT_RETURN(trie_insert(trie, key, key_len, (void *) 1234, 0)
		== TRIE_SUCCESS,
		"trie_insert() should succeed");
  ASSERT_RETURN(trie_find_exact(trie, key, key_len) == (void *) 1234,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_insert(trie, key, key_len, (void *) 5678,
			    TRIE_INSERT_OR_REPLACE)
		== TRIE_SUCCESS,
		"trie_insert(replace=1) should succeed");
  ASSERT_RETURN(_trie_destroy_count == 1,
		"_trie_destroy() has not been called");
  ASSERT_RETURN(trie_find_exact(trie, key, key_len) == (void *) 5678,
		"trie_find_exact() returned an incorrect value");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_exact_match ]--------------------------------
static int test_trie_exact_match()
{
  gds_trie_t * trie= trie_create(NULL);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 0, (void *) 1, 0);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 1, (void *) 2, 0);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 1, (void *) 3, 0);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 2, (void *) 4, 0);
  trie_insert(trie, IPV4_TO_INT(64,0,0,0), 2, (void *) 5, 0);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 2, (void *) 6, 0);
  trie_insert(trie, IPV4_TO_INT(192,0,0,0), 2, (void *) 7, 0);
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(0,0,0,0), 0) == (void *) 1,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(0,0,0,0), 1) == (void *) 2,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(128,0,0,0), 1) == (void *) 3,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(0,0,0,0), 2) == (void *) 4,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(64,0,0,0), 2) == (void *) 5,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(128,0,0,0), 2) == (void *) 6,
		"trie_find_exact() returned an incorrect value");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(192,0,0,0), 2) == (void *) 7,
		"trie_find_exact() returned an incorrect value");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_best_match ]--------------------------------
static int test_trie_best_match()
{
  gds_trie_t * trie= trie_create(NULL);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 0, (void *) 1, 0);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 1, (void *) 2, 0);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 1, (void *) 3, 0);
  trie_insert(trie, IPV4_TO_INT(64,0,0,0), 2, (void *) 5, 0);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 2, (void *) 6, 0);
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(0,0,0,0), 0) == (void *) 1,
		"trie_find_best() returned an incorrect value");
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(0,0,0,0), 1) == (void *) 2,
		"trie_find_best() returned an incorrect value");
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(128,0,0,0), 1) == (void *) 3,
		"trie_find_best() returned an incorrect value");
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(0,0,0,0), 2) == (void *) 2,
		"trie_find_best() returned an incorrect value");
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(64,0,0,0), 2) == (void *) 5,
		"trie_find_best() returned an incorrect value");
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(128,0,0,0), 2) == (void *) 6,
		"trie_find_best() returned an incorrect value");
  ASSERT_RETURN(trie_find_best(trie, IPV4_TO_INT(192,0,0,0), 2) == (void *) 3,
		"trie_find_best() returned an incorrect value");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_replace ]------------------------------------
static int test_trie_replace()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  uint32_t key= IPV4_TO_INT(0, 128, 128, 127);
  uint8_t key_len= 9;
  ASSERT_RETURN(trie_insert(trie, key, key_len, (void *) 5678, 0) == 0,
		"trie_insert() should succeed");
  ASSERT_RETURN(trie_replace(trie, key, key_len, (void *) 1234) == 0,
		"trie_replace() should succeed");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_replace_missing ]----------------------------
static int test_trie_replace_missing()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  uint32_t key= IPV4_TO_INT(0, 128, 128, 127);
  uint8_t key_len= 9;
  ASSERT_RETURN(trie_replace(trie, key, key_len, (void *) 1234) != 0,
		"trie_replace() should fail (unexisting value)");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_num_nodes ]----------------------------------
static int test_trie_num_nodes()
{
  gds_trie_t * trie= trie_create(NULL);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 1, (void *) 128, 0);
  ASSERT_RETURN(trie_num_nodes(trie, 0)== 1,
		"incorrect number of nodes");
  ASSERT_RETURN(trie_num_nodes(trie, 1)== 1,
		"incorrect number of nodes");
  trie_insert(trie, IPV4_TO_INT(192,0,0,0), 2, (void *) 192, 0);
  ASSERT_RETURN(trie_num_nodes(trie, 0)== 2,
		"incorrect number of nodes"); 
  ASSERT_RETURN(trie_num_nodes(trie, 1)== 2,
		"incorrect number of nodes");
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 2, (void *) 0, 0);
  ASSERT_RETURN(trie_num_nodes(trie, 0)== 4,
		"incorrect number of nodes");
  ASSERT_RETURN(trie_num_nodes(trie, 1)== 3,
		"incorrect number of nodes");
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 0, (void *) 0, 0);
  ASSERT_RETURN(trie_num_nodes(trie, 0)== 4,
		"incorrect number of nodes");
  ASSERT_RETURN(trie_num_nodes(trie, 1)== 4,
		"incorrect number of nodes");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_remove ] ------------------------------------
static int test_trie_remove()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  _trie_destroy_count= 0;
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 1, (void *) 128, 0);
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(128,0,0,0), 2)
		== TRIE_ERROR_NO_MATCH,
		"trie_remove() should fail (no-match)");
  ASSERT_RETURN(_trie_destroy_count == 0, "plouf");
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(128,0,0,0), 0)
		== TRIE_ERROR_NO_MATCH,
		"trie_remove() should fail (no-match)");
  ASSERT_RETURN(_trie_destroy_count == 0, "plouf");
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(128,0,0,0), 1)
		== TRIE_SUCCESS,
		"trie_remove() should succeed");
  ASSERT_RETURN(_trie_destroy_count == 1,
		"_trie_destroy() has not been called");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_remove2 ]----------------------------------------
static int test_trie_remove2()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  _trie_destroy_count= 0;
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 0, (void *) 0, 0);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 1, (void *) 0, 0);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 1, (void *) 1, 0);
  trie_insert(trie, IPV4_TO_INT(64,0,0,0), 2, (void *) 01, 0);
  trie_insert(trie, IPV4_TO_INT(0,0,0,0), 2, (void *) 00, 0);
  trie_insert(trie, IPV4_TO_INT(192,0,0,0), 2, (void *) 11, 0);
  trie_insert(trie, IPV4_TO_INT(128,0,0,0), 2, (void *) 10, 0);
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(128,0,0,0), 1) == TRIE_SUCCESS,
		"trie_remove() should succeed");
  ASSERT_RETURN(_trie_destroy_count == 1,
		"_trie_destroy() has not been called");
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(128,0,0,0), 2) == TRIE_SUCCESS,
		"trie_remove() should succeed");
  ASSERT_RETURN(_trie_destroy_count == 2,
		"_trie_destroy() has not been called");
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(0,0,0,0), 1) == TRIE_SUCCESS,
		"trie_remove() should succeed");
  ASSERT_RETURN(_trie_destroy_count == 3,
		"_trie_destroy() has not been called");
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(64,0,0,0), 2) == TRIE_SUCCESS,
		"trie_remove() should succeed");
  ASSERT_RETURN(_trie_destroy_count == 4,
		"_trie_destroy() has not been called");
  ASSERT_RETURN(trie_remove(trie, IPV4_TO_INT(0,0,0,0), 0) == TRIE_SUCCESS,
		"trie_remove() should succeed");
  ASSERT_RETURN(_trie_destroy_count == 5,
		"_trie_destroy() has not been called");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_masking ]------------------------------------
static int test_trie_masking()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  ASSERT_RETURN(trie_insert(trie, IPV4_TO_INT(123,234,198,76), 17,
			    (void *) 12357, 0) == 0,
		"could not insert");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(123,234,198,76), 17) ==
		(void *) 12357,
		"could not find exact-match");
  ASSERT_RETURN(trie_find_exact(trie, IPV4_TO_INT(123,234,128,0), 17) ==
		(void *) 12357,
		"could not find exact-match");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ _trie_for_each_cb ]----------------------------------------
static int _trie_for_each_cb(trie_key_t key, trie_key_len_t key_len,
			     void * data, void * ctx)
{
  unsigned int * count= (unsigned int *) ctx;
  if (count != NULL)
    (*count)++;
  return 0;
}

// -----[ test_trie_for_each ]-----------------------------------
static int test_trie_for_each()
{
  gds_trie_t * trie= trie_create(NULL);
  unsigned int index, count= 0;
  for (index= 0; index < TRIE_NITEMS; index++)
    trie_insert(trie, TRIE_ITEMS[index].key,
		TRIE_ITEMS[index].key_len,
		(void *) TRIE_ITEMS[index].data, 0);
  trie_for_each(trie, _trie_for_each_cb, &count);
  ASSERT_RETURN(count == TRIE_NITEMS,
		"for-each did not traverse whole trie (%u vs %u)",
		count, TRIE_NITEMS);
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_enum ]---------------------------------------
static int test_trie_enum()
{
  gds_enum_t * enu;
  int data;
  unsigned int index, count= 0;
  gds_trie_t * trie= trie_create(NULL);
  for (index= 0; index < TRIE_NITEMS; index++)
    trie_insert(trie, TRIE_ITEMS[index].key,
		TRIE_ITEMS[index].key_len,
		(void *) TRIE_ITEMS[index].data, 0);
  enu= trie_get_enum(trie);
  ASSERT_RETURN(enu != NULL, "trie_get_enum() returned NULL pointer "
		"(%u vs %u)", count, TRIE_NITEMS);
  memset(TRIE_FLAGS, 0, sizeof(TRIE_FLAGS));
  while (enum_has_next(enu)) {
    data= (int) enum_get_next(enu);
    for (index= 0; index < TRIE_NITEMS; index++)
      if (TRIE_ITEMS[index].data == data) {
	TRIE_FLAGS[index]= 1;
	break;
      }
    count++;
  }
  enum_destroy(&enu);
  ASSERT_RETURN(count == TRIE_NITEMS,
		"enumerator did not traverse whole trie");
  for (index= 0; index < TRIE_NITEMS; index++)
    ASSERT_RETURN(TRIE_FLAGS[index], "item %u not enumerated");
  trie_destroy(&trie);
  return UTEST_SUCCESS;
}

// -----[ test_trie_complex ]--------------------------------------
static int test_trie_complex()
{
  gds_trie_t * trie= trie_create(_trie_destroy);
  unsigned int index;
  void * data;
  trie_key_t key, new_key;
  trie_key_len_t key_len, new_key_len;
  // Insertion
  for (index= 0; index < TRIE_NITEMS; index++) {
    ASSERT_RETURN(trie_insert(trie, TRIE_ITEMS[index].key,
			      TRIE_ITEMS[index].key_len,
			      (void *) TRIE_ITEMS[index].data, 0) == 0,
		  "could not insert %d/%d/%d",
		  TRIE_ITEMS[index].key,
		  TRIE_ITEMS[index].key_len,
		  TRIE_ITEMS[index].data);
  }
  // Exact match
  for (index= 0; index < TRIE_NITEMS; index++) {
    data= trie_find_exact(trie, TRIE_ITEMS[index].key,
			  TRIE_ITEMS[index].key_len);
    ASSERT_RETURN(data == (void *) TRIE_ITEMS[index].data,
		  "exact match failed for %d/%d: %p vs %p",
		  TRIE_ITEMS[index].key,
		  TRIE_ITEMS[index].key_len,
		  (void *) TRIE_ITEMS[index].data,
		  data);
  }
  // Best match
  //  For each prefix that is shorter than /32 in the initial data, look
  //  for a more specific prefix (length+1). Check that if the more
  //  specific prefix that is generated exists, the best match should
  //  return this prefix. Otherwise, the initial (less specific prefix)
  // should be found.
  //
  // Example:
  //   trie contains (0.192.0.0/16, 0.192.0.0/17)
  //   original=0.192.0.0/16
  //     - generated=0.192.0.0/17 (exists)
  //         => exact-match=0.192.0.0/17
  //     - generated=0.192.128.0/17 (does not exist)
  //         => best-match=0.192.0.0/16
  for (index= 0; index < TRIE_NITEMS; index++) {
    key= TRIE_ITEMS[index].key;
    key_len= TRIE_ITEMS[index].key_len;
    if (key_len < 32) {
      new_key_len= key_len+1;
      new_key= key|(1 << (31-key_len));
      if (trie_find_exact(trie, new_key, new_key_len) == NULL) {
	ASSERT_RETURN((size_t) trie_find_best(trie, new_key, new_key_len)
		      == TRIE_ITEMS[index].data,
		      "best-match failed for %d/%d (more specific than %d/%d)",
		      new_key, new_key_len, key, key_len);

      }
      new_key= key & ~(1 << (31-key_len));
      if (trie_find_exact(trie, new_key, new_key_len) == NULL) {
	ASSERT_RETURN((size_t) trie_find_best(trie, new_key, new_key_len)
		      == TRIE_ITEMS[index].data,
		      "best-match failed for %d/%d (more specific than %d/%d)",
		      new_key, new_key_len, key, key_len);

      }
    }
  }
  trie_destroy(&trie);
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

static cli_t * cli= NULL;
static int cli_cmd_executed;
static char * cli_param_value;

// -----[ _cli_cmd_success ]-----------------------------------------
static int _cli_cmd_success(cli_ctx_t * ctx, cli_cmd_t * cmd)
{
  if (!strcmp(cmd->name, "cmd3")) {
    if (cli_param_value != NULL)
      free(cli_param_value);
    cli_param_value= strdup(tokens_get_string_at(cmd->param_values, 0));
  } else if (!strcmp(cmd->name, "cmd4")) {
    assert(cli_context_get_item_at_top(ctx) == (void *) 1234);
  }
  return CLI_SUCCESS;
}

// -----[ _cli_cmd_failure ]-----------------------------------------
static int _cli_cmd_failure(cli_ctx_t * ctx, cli_cmd_t * cmd)
{
  return CLI_ERROR_COMMAND_FAILED;
}

// -----[ _cli_cmd4_ctx_create ]-------------------------------------
static int _cli_cmd4_ctx_create(cli_ctx_t * ctx, void ** item)
{
  // Should push something on the context stack here that cmd5
  // should try to find (and report an error if it is not found.
  *item= (void *) 1234;
  return CLI_SUCCESS;
}

// -----[ _cli_cmd4_ctx_destroy ]------------------------------------
static void _cli_cmd4_ctx_destroy(void ** item)
{
  assert(*item == (void *) 1234);
}

// -----[ test_before_cli ]------------------------------------------
/**
 * Initialize the CLI tests
 *
 * test
 *   cmd1: <param1> <param2> <...>[0-5]
 *   cmd2: <param1>
 *   cmd2_fail: <param1> => execution fails
 *   cmd3: <param1> [--plop]
 *   cmd4: 
 *   cmd5:
 */
static int test_before_cli()
{
  cli_cmd_t * cmd;
  cli_cmds_t * cmds, * sub_cmds;
  cli_params_t * params;

  cli= cli_create();
  ASSERT_RETURN(cli != NULL, "cli_create() returned NULL pointer");

  cmds= cli_cmds_create();
  ASSERT_RETURN(cmds != NULL, "cli_cmds_create() returned NULL pointer");

  // cmd1: <param1> <param2> <...>[0-5]
  params= cli_params_create();
  cli_params_add(params, "<param1>", NULL);
  cli_params_add(params, "<param2>", NULL);
  cli_params_add_vararg(params, "<param3>", 5, NULL);
  // Note: adding an additional parameter should abort() the program
  // cli_params_add(pParams, "<param4>", NULL);
  cli_cmds_add(cmds, cli_cmd_create("cmd1", _cli_cmd_success,
				    NULL, params));

  // cmd2: <param1>
  params= cli_params_create();
  cli_params_add(params, "<param1>", NULL);
  cli_cmds_add(cmds, cli_cmd_create("cmd2", _cli_cmd_success,
				    NULL, params));

  // cmd2_fail: <param1> => execution failed
  params= cli_params_create();
  cli_params_add(params, "<param1>", NULL);
  cli_cmds_add(cmds, cli_cmd_create("cmd2_fail", _cli_cmd_failure,
				    NULL, params));

  // cmd3: <param1> [--plop]
  params= cli_params_create();
  cli_params_add(params, "<param1>", NULL);
  cmd= cli_cmd_create("cmd3", _cli_cmd_success,
		      NULL, params);
  cli_cmd_add_option(cmd, "plop", NULL);
  cli_cmds_add(cmds, cmd);

  // cmd5:
  sub_cmds= cli_cmds_create();
  cli_cmds_add(sub_cmds, cli_cmd_create("cmd5", _cli_cmd_success,
					NULL, NULL));

  // cmd4: create context
  cli_cmds_add(cmds, cli_cmd_create_ctx("cmd4",
					_cli_cmd4_ctx_create,
					_cli_cmd4_ctx_destroy,
					sub_cmds, NULL));

  // test (root of command tree)
  cli_register_cmd(cli, cli_cmd_create("test", NULL, cmds, NULL));

  // Identifier of last executed command
  cli_cmd_executed= 0;
  cli_param_value= NULL;

  return UTEST_SUCCESS;
}

// -----[ test_after_cli ]-------------------------------------------
static int test_after_cli()
{
  cli_destroy(&cli);
  cli= NULL;
  if (cli_param_value != NULL)
    free(cli_param_value);

  return UTEST_SUCCESS;
}

// -----[ test_cli_basic ]-------------------------------------------
/**
 * Test basic use.
 */
static int test_cli_basic()
{
  // Standard call for cmd2
  ASSERT_RETURN(cli_execute(cli, "test cmd2 arg1")
		== CLI_SUCCESS, "could not execute command");

  // Standard call for cmd2_fail => should fail
  ASSERT_RETURN(cli_execute(cli, "test cmd2_fail arg1")
		== CLI_ERROR_COMMAND_FAILED,
		"returned wrong error code");
  return UTEST_SUCCESS;
}

// -----[ test_cli_varargs ]-----------------------------------------
/**
 * Test use with varargs
 */
static int test_cli_varargs()
{
  // Call cmd1 with 3 varargs
  ASSERT_RETURN(cli_execute(cli, "test cmd1 A1 A2 VA1 VA2 VA3")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd1 with no varargs (this is allowed: [0-N])
  ASSERT_RETURN(cli_execute(cli, "test cmd1 A1 A2")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd1 with too many varargs
  ASSERT_RETURN(cli_execute(cli, "test cmd1 A1 A2 VA1 VA2 VA3 VA4 VA5 VA6")
		== CLI_ERROR_TOO_MANY_PARAMS,
		"wrong error reported");

  return UTEST_SUCCESS;
}

// -----[ test_cli_options ]-----------------------------------------
/**
 * Test with options
 */
static int test_cli_options()
{
  // Call cmd1 with unknown option
  ASSERT_RETURN(cli_execute(cli, "test cmd1 --plop A1 A2 VA1")
		== CLI_ERROR_UNKNOWN_OPTION, "wrong error reported");
  // Call cmd3 with option
  ASSERT_RETURN(cli_execute(cli, "test cmd3 --plop A1")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd3 with option
  ASSERT_RETURN(cli_execute(cli, "test cmd3 --plop=\"AC & DC\" A1")
		== CLI_SUCCESS, "could not execute command");

  return UTEST_SUCCESS;
}

// -----[ test_cli_context ]-----------------------------------------
/**
 * Test with context
 */
static int test_cli_context()
{
  // Exit (without context)
  ASSERT_RETURN(cli_execute(cli, "exit")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd4/cmd5
  ASSERT_RETURN(cli_execute(cli, "test cmd4 cmd5")
		== CLI_SUCCESS, "could not execute command");
  // Enter context cmd4
  ASSERT_RETURN(cli_execute(cli, "test cmd4")
		== CLI_SUCCESS, "could not execute command");
  // Call cmd5 (from context cmd4)
  ASSERT_RETURN(cli_execute(cli, "cmd5")
		== CLI_SUCCESS, "could not execute command");
  // Exit context
  ASSERT_RETURN(cli_execute(cli, "exit")
		== CLI_SUCCESS, "could not execute command");
  return UTEST_SUCCESS;
}

// -----[ test_cli_params ]------------------------------------------
static int test_cli_params()
{
  const char * line= "test cmd3 \"this is a ($VAR1, $VAR2)\"";
  cli_set_param_lookup(cli, _param_lookup);
  cli_execute_line(cli, line);
  ASSERT_RETURN((cli_param_value != NULL) &&
		!strcmp(cli_param_value, "this is a (Hello, World)"),
		"parameter incorrectly replaced (%s)", cli_param_value);
  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_HASH
/////////////////////////////////////////////////////////////////////

/*
int test_hash_set_insertion_search_deletion()
{
  gds_hash_set_t* pHash;
  uint32_t uNbr;
  SHashItem * pItem;


  return UTEST_SUCCESS;
}
*/

 /*
int test_hash_set_reference()
{
  gds_hash_set_t* pHash;
  uint32_t uNbr, uCpt;
  SHashItem * pItem;

  // static test
  pHash= hash_set_create(6, 0, _hash_cmp, _hash_set_destroy, _hash_fct);
  for (uCpt = 0; uCpt < 10; uCpt++) {
    for (uNbr = 0; uNbr < 20; uNbr++) {
      pItem = MALLOC(sizeof(SHashItem));
      pItem->uNbr = uNbr;
      ASSERT_RETURN(hash_set_add(pHash, pItem) != NULL, "%d can't be inserted (static)", uNbr);
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


  hash_set_destroy(&pHash);

  return UTEST_SUCCESS;
}
 */

// -----[ test_hash_set_old ]--------------------------------------------
int test_hash_set_old()
{
  /*
  hash_t* pHash;
  gds_enum_t * pEnum;
  uint32_t uNbr;
  
  // static mode
  pHash= hash_set_create(6, 0, _hash_cmp, _hash_set_destroy, _hash_fct);
  ASSERT_RETURN(pHash != NULL, "hash_init() returned NULL pointer");

  for (uNbr = 0; uNbr < 20; uNbr++) {
    hash_set_add(pHash, (void *) uNbr);
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
  hash_set_add(pHash, (void *)4);

  printf("*** removal of 4 ***\n");
  hash_del(pHash, (void *)4);
  hash_for_each(pHash, _hash_for_each, NULL);

  printf("*** add 22 ***\n");
  hash_set_add(pHash, (void *)22);
  hash_for_each(pHash, _hash_for_each, NULL);

  hash_set_destroy(&pHash);

  // dynamic mode
  pHash= hash_set_create(3, 0.75, _hash_cmp, _hash_set_destroy, _hash_fct);
  for (uNbr = 0; uNbr < 20; uNbr++) {
    hash_add(pHash, (void *) uNbr);
  }
  hash_for_each(pHash, _hash_for_each, NULL);

  hash_set_destroy(&pHash);
  */
  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_HASH_SET
/////////////////////////////////////////////////////////////////////

unsigned int _hash_set_destroy_count;

// -----[ _hash_cmp ]------------------------------------------------
static int _hash_cmp(const void * item1, const void * item2, unsigned int size)
{
  unsigned int i1= (unsigned int) item1;
  unsigned int i2= (unsigned int) item2;

  // -> item(K) == item(K+1000*N)
  i1= i1 % 1000;
  i2= i2 % 1000;

  if (i1 < i2)
    return 1;
  if (i1 > i2)
    return -1;
  return 0;
}

// -----[ _hash_set_destroy ]--------------------------------------------
static void _hash_set_destroy(void * item)
{
  _hash_set_destroy_count++;
}

// -----[ _hash_compute ]--------------------------------------------
static uint32_t _hash_compute(const void * item, unsigned int size)
{
  return ((unsigned int) item) % size;
}

// -----[ _hash_for_each ]-------------------------------------------
static int _hash_for_each(void * item, void * ctx)
{
  unsigned int * count= (unsigned int *) ctx;
  if (count != NULL)
    (*count)++;
  return 0;
}

// -----[ test_hash_set_create_destroy ]---------------------------------
static int test_hash_set_create_destroy()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  ASSERT_RETURN(hash != NULL, "hash_set_create() should succeed");
  hash_set_destroy(&hash);
  ASSERT_RETURN(hash == NULL, "destroyed hash should be NULL");
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_add ]--------------------------------------------
static int test_hash_set_add()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  ASSERT_RETURN(hash_set_add(hash, (void *) 100) == (void *) 100,
		"hash_set_add() should return same pointer");
  ASSERT_RETURN(hash_set_get_refcnt(hash, (void *) 100) == 1,
		"incorrect reference count");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_add_ref ]----------------------------------------
static int test_hash_set_add_ref()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  _hash_set_destroy_count= 0;
  ASSERT_RETURN(hash_set_add(hash, (void *) 100) == (void *) 100,
		"hash_set_add() should return same pointer");
  ASSERT_RETURN(hash_set_get_refcnt(hash, (void *) 100) == 1,
		"incorrect reference count");
  ASSERT_RETURN(hash_set_add(hash, (void *) 1100) == (void *) 100,
		"hash_set_add() should return other pointer (ref)");
  ASSERT_RETURN(hash_set_get_refcnt(hash, (void *) 100) == 2,
		"incorrect reference count");
  ASSERT_RETURN(_hash_set_destroy_count == 0,
		"_hash_set_destroy() should not have been called");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_search ]-----------------------------------------
static int test_hash_set_search()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  hash_set_add(hash, (void *) 1);
  hash_set_add(hash, (void *) 2);
  ASSERT_RETURN(hash_set_search(hash, (void *) 1) == (void *) 1,
		"hash_set_search() returned an incorrect value");
  ASSERT_RETURN(hash_set_search(hash, (void *) 1) == (void *) 1,
		"hash_set_search() returned an incorrect value");
  hash_set_destroy(&hash);  
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_search_missing ]---------------------------------
static int test_hash_set_search_missing()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  ASSERT_RETURN(hash_set_search(hash, (void *) 1) == NULL,
		"hash_set_search() should return NULL");
  hash_set_destroy(&hash);  
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_remove ]-----------------------------------------
static int test_hash_set_remove()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  _hash_set_destroy_count= 0;
  hash_set_add(hash, (void *) 100);
  ASSERT_RETURN(hash_set_get_refcnt(hash, (void *) 100) == 1,
		"incorrect reference count");
  ASSERT_RETURN(hash_set_remove(hash, (void *) 100) == HASH_SUCCESS,
		"hash_set_remove() should succeed");
  ASSERT_RETURN(_hash_set_destroy_count == 1,
		"_hash_set_destroy() was not called");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_remove_unref ]-----------------------------------
static int test_hash_set_remove_unref()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  _hash_set_destroy_count= 0;
  hash_set_add(hash, (void *) 100);
  hash_set_add(hash, (void *) 100);
  ASSERT_RETURN(hash_set_get_refcnt(hash, (void *) 100) == 2,
		"incorrect reference count");
  ASSERT_RETURN(hash_set_remove(hash, (void *) 100) == HASH_SUCCESS_UNREF,
		"hash_set_remove() should succeed");
  ASSERT_RETURN(_hash_set_destroy_count == 0,
		"_hash_set_destroy() should not have been called");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_remove_missing ]---------------------------------
static int test_hash_set_remove_missing()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  ASSERT_RETURN(hash_set_remove(hash, (void *) 100) == HASH_ERROR_NO_MATCH,
		"hash_set_remove() should fail (no-match)");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_for_each ]---------------------------------------
static int test_hash_set_for_each()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  unsigned int count= 0;
  hash_set_add(hash, (void *) 1);
  hash_set_add(hash, (void *) 2);
  hash_set_add(hash, (void *) 3);
  hash_set_add(hash, (void *) 4);
  ASSERT_RETURN(hash_set_for_each(hash, _hash_for_each, &count) == 0,
		"hash_set_for_each() should succeed");
  ASSERT_RETURN(count == 4, "incorrect number of items enumerated");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_enum ]-------------------------------------------
static int test_hash_set_enum()
{
  gds_hash_set_t * hash= hash_set_create(100, 0.0, _hash_cmp,
				     _hash_set_destroy, _hash_compute);
  gds_enum_t * enu;
  unsigned int value;
  unsigned int count= 0;
  hash_set_add(hash, (void *) 1);
  hash_set_add(hash, (void *) 2);
  hash_set_add(hash, (void *) 3);
  hash_set_add(hash, (void *) 4);
  enu= hash_set_get_enum(hash);
  while (enum_has_next(enu)) {
    value= **(unsigned int**) enum_get_next(enu);
    count++;
  }
  ASSERT_RETURN(count == 4, "incorrect number of items enumerated");
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}

// -----[ test_hash_set_strings ]----------------------------------------
static int test_hash_set_strings()
{
  unsigned int index;
  unsigned int len;
  char allowed_chars[]= "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
  gds_hash_set_t * hash= hash_set_create(6, 0,
					 hash_utils_compare_string,
					 _hash_set_destroy,
					 hash_utils_key_compute_string);
  char * tmp;
  int result;

#define HASH_NITEMS      1024
#define HASH_STR_MAX_LEN 1024
  char * HASH_ITEMS_STR[HASH_NITEMS];
  char * HASH_ITEMS_STR_PTR[HASH_NITEMS];

  // Generate random strings
  for (index= 0; index < HASH_NITEMS; index++) {
    // Note
    // - empty strings allowed
    // - collisions (non unique strings) are allowed
    len= random() % HASH_STR_MAX_LEN;
    HASH_ITEMS_STR[index]= malloc(sizeof(char)*(len+1));
    HASH_ITEMS_STR[index][len]= '\0';
    while (len > 0) {
      HASH_ITEMS_STR[index][len-1]=
	allowed_chars[random() % strlen(allowed_chars)];
      len--;
    }
    HASH_ITEMS_STR_PTR[index]= HASH_ITEMS_STR[index];
  }

  for (index= 0; index < HASH_NITEMS; index++) {
    tmp= (char *) hash_set_add(hash, HASH_ITEMS_STR[index]);
    ASSERT_RETURN((tmp == HASH_ITEMS_STR[index]) ||
		  !strcmp(tmp, HASH_ITEMS_STR[index]),
		  "hash_set_add() failed (tmp=%p [%s] / data=%p [%s])",
		  tmp, tmp, HASH_ITEMS_STR[index], HASH_ITEMS_STR[index]);
    if (tmp != HASH_ITEMS_STR[index])
      HASH_ITEMS_STR_PTR[index]= tmp;
  }

  for (index= 0; index < HASH_NITEMS; index++) {
    ASSERT_RETURN(hash_set_search(hash, HASH_ITEMS_STR[index])
		  == HASH_ITEMS_STR_PTR[index],
		  "hash_set_search() returned an incorrect value");
  }

  for (index= 0; index < HASH_NITEMS; index++) {
    result= hash_set_remove(hash, HASH_ITEMS_STR[index]);
    ASSERT_RETURN((result == HASH_SUCCESS) ||
		  (result == HASH_SUCCESS_UNREF),
		  "hash_set_remove() failed");
  }

  for (index= 0; index < HASH_NITEMS; index++) {
    free(HASH_ITEMS_STR[index]);
  }
  hash_set_destroy(&hash);
  return UTEST_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// GDS_CHECK_BIT_VECTOR
/////////////////////////////////////////////////////////////////////

#include <libgds/bit_vector.h>
int test_bit_vector_creation_destruction()
{
  gds_bit_vector_t * bv;

  bv= bit_vector_create(1);
  ASSERT_RETURN(bv != NULL, "creation failed");
  bit_vector_destroy(&bv);

  return UTEST_SUCCESS;
}

int test_bit_vector_manipulations()
{
  gds_bit_vector_t * bv;

  bv= bit_vector_create(32);
  ASSERT_RETURN(!bit_vector_get(bv, 0), "bit 0 is inaccurate");
  ASSERT_RETURN(!bit_vector_set(bv, 0), "bit 0 not set");
  ASSERT_RETURN(bit_vector_get(bv, 0), "bit 0 is inaccurate");
  ASSERT_RETURN(!bit_vector_set(bv, 1), "bit 1 not set");
  ASSERT_RETURN(!bit_vector_set(bv, 31), "bit 32 not set");
  ASSERT_RETURN(bit_vector_set(bv, 32) == -1, "bit couldn't be set");
  ASSERT_RETURN(bit_vector_get(bv, 1), "bit 1 is inaccurate");

  ASSERT_RETURN(!bit_vector_get(bv, 2), "bit 2 is inaccurate");
  ASSERT_RETURN(!bit_vector_get(bv, 30), "bit 30 is inaccurate");
  ASSERT_RETURN(bit_vector_get(bv, 32) == -1, "bit 32 should be inaccessible");

  ASSERT_RETURN(!bit_vector_clear(bv, 0), "can't unset bit 0");
  ASSERT_RETURN(!bit_vector_get(bv, 0), "bit 0 should be unset");
  ASSERT_RETURN(bit_vector_clear(bv, 32) == -1, "bit should be inaccessible");
  ASSERT_RETURN(!bit_vector_clear(bv, 31), "can't unset bit 31");
  ASSERT_RETURN(!bit_vector_get(bv, 31), "bit 31 should be unset");

  bit_vector_destroy(&bv);
  return UTEST_SUCCESS;
}

void _test_bit_vector_set(gds_bit_vector_t * bv)
{
  bit_vector_set(bv, 0);
  bit_vector_set(bv, 1);
  bit_vector_set(bv, 3);
  bit_vector_set(bv, 10);
  bit_vector_set(bv, 15);
  bit_vector_set(bv, 23);
  bit_vector_set(bv, 32);
  bit_vector_set(bv, 33);
  bit_vector_set(bv, 34);
  bit_vector_set(bv, 64);
}

void _test_bit_vector_set_xor(gds_bit_vector_t * bv)
{
  bit_vector_set(bv, 0);
  bit_vector_set(bv, 3);
  bit_vector_set(bv, 10);
  bit_vector_set(bv, 15);
  bit_vector_set(bv, 23);
  bit_vector_set(bv, 27);
  bit_vector_set(bv, 32);
  bit_vector_set(bv, 34);
}

static const char * BIT_VECTOR_0=
  "00000000000000000000000000000000000000000000000000000000000000000";
static const char *BIT_VECTOR_INIT=
  "11010000001000010000000100000000111000000000000000000000000000001";
static const char * BIT_VECTOR_XOR=
  "10010000001000010000000100010000101000000000000000000000000000000";
static const char * BIT_VECTOR_XOR_RESULT=
  "01000000000000000000000000010000010000000000000000000000000000001";

int test_bit_vector_binary_operations()
{
  gds_bit_vector_t * bv1;
  gds_bit_vector_t * bv2;
  char * str;

  /* Legal Operations done on bit vectors of same length */
  /* AND */
  bv1= bit_vector_create(65);
  bv2= bit_vector_create(65);
  _test_bit_vector_set(bv1);
  ASSERT_RETURN(bit_vector_and(bv1, bv2) == 0, "'and' operation failed");
  str= bit_vector_to_string(bv1);
  ASSERT_RETURN(strcmp(str, BIT_VECTOR_0) == 0,
		"'and' operation not conform");
  FREE(str);
  bit_vector_destroy(&bv1);
  bit_vector_destroy(&bv2);

  /* OR */
  bv1= bit_vector_create(65);
  bv2= bit_vector_create(65);
  _test_bit_vector_set(bv1);
  ASSERT_RETURN(bit_vector_or(bv1, bv2) == 0, "'or' operation failed");
  str= bit_vector_to_string(bv1);
  ASSERT_RETURN(strcmp(str, BIT_VECTOR_INIT) == 0,
		"'or' operation not conform");
  FREE(str);
  bit_vector_destroy(&bv1);
  bit_vector_destroy(&bv2);

  /* XOR */
  bv1= bit_vector_create(65);
  bv2= bit_vector_create(65);
  _test_bit_vector_set(bv1);
  _test_bit_vector_set_xor(bv2);
  ASSERT_RETURN(bit_vector_xor(bv1, bv2) == 0, "'xor' operation failed");
  str= bit_vector_to_string(bv1);
  ASSERT_RETURN(strcmp(bit_vector_to_string(bv2), BIT_VECTOR_XOR) == 0,
		"'xor' string is inexact");
  ASSERT_RETURN(strcmp(str, BIT_VECTOR_XOR_RESULT) == 0,
		"'xor' operation not conform");
  FREE(str);
  bit_vector_destroy(&bv1);
  bit_vector_destroy(&bv2);

  /* Operation done on bit vectors of different lengths */
  bv1= bit_vector_create(65);
  bv2= bit_vector_create(3);
  ASSERT_RETURN(bit_vector_and(bv1, bv2) == -1,
		"'and' operation should have failed (len1 > len2)");
  ASSERT_RETURN(bit_vector_and(bv2, bv1) == -1,
		"'and' operation should have failed (len1 > len2)");
  ASSERT_RETURN(bit_vector_and(NULL, NULL) == -1,
		"'and' operation should have failed (NULL, NULL)");
  ASSERT_RETURN(bit_vector_and(bv1, NULL) == -1,
		"'and' operation should have failed (pBitVector, NULL)");
  ASSERT_RETURN(bit_vector_and(NULL, bv1) == -1,
		"'and' operation should have failed (NULL, pBitVector)");
  bit_vector_destroy(&bv1);
  bit_vector_destroy(&bv2);

  return UTEST_SUCCESS;
}

int test_bit_vector_representation()
{
  gds_bit_vector_t * bv;
  char * str;

  /* String representation */
  bv= bit_vector_create(65);
  _test_bit_vector_set(bv);
  str= bit_vector_to_string(bv);
  ASSERT_RETURN(strcmp(str, BIT_VECTOR_INIT) == 0, "to_string failed");
  FREE(str);

  bv= bit_vector_from_string(BIT_VECTOR_INIT);
  str= bit_vector_to_string(bv);
  ASSERT_RETURN(strcmp(str, BIT_VECTOR_INIT) == 0, "from_string failed");
  FREE(str);

  bit_vector_destroy(&bv);
  return UTEST_SUCCESS;
}

int test_bit_vector_equality()
{
  gds_bit_vector_t * bv1;
  gds_bit_vector_t * bv2;

  bv1= bit_vector_create(65);
  bv2= bit_vector_create(65);

  // Same lengths
  _test_bit_vector_set(bv1);
  // same value
  _test_bit_vector_set(bv2);
  ASSERT_RETURN(bit_vector_cmp(bv1, bv2) == 0,
		"bit vectors should be equals");
  ASSERT_RETURN(bit_vector_equals(bv1, bv2) == 1,
		"bit vector should be equals");
  // greater value
  _test_bit_vector_set_xor(bv2);
  ASSERT_RETURN(bit_vector_cmp(bv2, bv1) == 1,
		"equality result should be greater");
  ASSERT_RETURN(bit_vector_equals(bv1, bv2) == 0,
		"bit vector should be equals");
  // smaller value
  ASSERT_RETURN(bit_vector_cmp(bv1, bv2) == -1,
		"equality result should be smaller");

  // Different lengths
  bit_vector_destroy(&bv2);
  bv2= bit_vector_create(3);
  

  bit_vector_destroy(&bv1);
  bit_vector_destroy(&bv2);

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

/*int _bloom_print_for_each(void * pItem, void * pCtx)
{
  uint32_t uItem = *(uint32_t *) pItem;

  printf("%02x|", uItem);
  return 0;
}*/

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
  uint32_array_t * uArray;
  uint8_t uCpt;
  uint32_t uByte;

  pBloomHash = bloom_hash_create(MAX_UINT32_T, 20);
  uArray = bloom_hash_get(pBloomHash, (uint8_t*)msg[0], strlen(msg[0]));
  for (uCpt = 0; uCpt < 20; uCpt++) {
    uByte= uArray->data[uCpt];
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

unit_test_t STRUTILS_TESTS[]= {
  {test_strutils_create, "create"},
  {test_strutils_create_null, "create (null)"},
  {test_strutils_lcreate, "lcreate"},
  {test_strutils_append, "append"},
  {test_strutils_append_null, "append (append=null)"},
  {test_strutils_append_src_null, "append (src=null)"},
  {test_strutils_prepend, "prepend"},
  {test_strutils_prepend_null, "prepend (null)"},
  {test_strutils_convert_int, "conversion int"},
  {test_strutils_convert_uint, "conversion unsigned int"},
  {test_strutils_convert_long, "conversion long"},
  {test_strutils_convert_ulong, "conversion unsigned long"},
  {test_strutils_convert_double, "conversion double"},
};
#define STRUTILS_NTESTS ARRAY_SIZE(STRUTILS_TESTS)

unit_test_t FIFO_TESTS[]= {
  {test_fifo_basic, "basic use"},
  {test_fifo_grow, "growable"},
};
#define FIFO_NTESTS ARRAY_SIZE(FIFO_TESTS)

unit_test_t STACK_TESTS[]= {
  {test_stack_create, "create"},
  {test_stack_basic, "basic use"},
  {test_stack_copy, "copy"},
  {test_stack_equal, "comparison"},
};
#define STACK_NTESTS ARRAY_SIZE(STACK_TESTS)

unit_test_t ENUM_TESTS[]= {
  {test_enum_template, "template"},
};
#define ENUM_NTESTS ARRAY_SIZE(ENUM_TESTS)

unit_test_t ARRAY_TESTS[]= {
  {test_array_create, "create/destroy"},
  {test_array_basic, "basic use"},
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

unit_test_t PTRARRAY_TESTS[]= {
  {test_ptr_array, "basic use"},
  {test_ptr_array_template, "template"},
  {test_ptr_array_template_sorted, "template (sorted)"},
};
#define PTRARRAY_NTESTS ARRAY_SIZE(PTRARRAY_TESTS)

unit_test_t TOKENIZER_TESTS[]= {
  {test_tokenizer_basic, "basic use"},
  {test_tokenizer_quotes, "quotes"},
  {test_tokenizer_escape, "escaped characters"},
  {test_tokenizer_escape_incomplete, "escaped characters (incomplete)"},
  {test_tokenizer_quotes_missing_close, "quotes (close missing)"},
  {test_tokenizer_quotes_missing_open, "quotes (open missing)"},
  {test_tokenizer_quotes2, "quotes (2)"},
  {test_tokenizer_multiquotes, "multiquotes"},
  {test_tokenizer_braces, "braces"},
  {test_tokenizer_complex, "complex"},
  {test_tokenizer_params, "params"},
  {test_tokenizer_params_invalid, "params (invalid)"},
  {test_tokenizer_params_undef, "params (undef)"},
  {test_tokenizer_opt_single_delim, "option single-delim"},
  {test_tokenizer_opt_empty_final, "option empty-final"},
  {test_tokenizer_protected_block, "protected block"},
};
#define TOKENIZER_NTESTS ARRAY_SIZE(TOKENIZER_TESTS)

unit_test_t PARAMS_TESTS[]= {
  {test_params_basic, "params"},
  {test_params_invalid, "params (invalid)"},
  {test_params_undef, "params (undef)"},
};
#define PARAMS_NTESTS ARRAY_SIZE(PARAMS_TESTS)

unit_test_t CLI_TESTS[]= {
  {test_cli_basic, "basic use"},
  {test_cli_options, "options"},
  {test_cli_varargs, "varargs"},
  {test_cli_context, "context"},
  {test_cli_params, "params"},
};
#define CLI_NTESTS ARRAY_SIZE(CLI_TESTS)

unit_test_t TRIE_TESTS[]= {
  {test_trie_create_destroy, "creation/destruction"},
  {test_trie_insert, "insert"},
  {test_trie_insert_dup, "insert (duplicate)"},
  {test_trie_insert_replace, "insert (replace)"},
  {test_trie_exact_match, "exact-match"},
  {test_trie_best_match, "best-match"},
  {test_trie_replace, "replace"},
  {test_trie_replace_missing, "replace (missing)"},
  {test_trie_num_nodes, "num-nodes"},
  {test_trie_remove, "remove"},
  {test_trie_remove2, "remove (2)"},
  {test_trie_masking, "masking"},
  {test_trie_for_each, "for-each"},
  {test_trie_enum, "enum"},
  {test_trie_complex, "complex"},
};
#define TRIE_NTESTS ARRAY_SIZE(TRIE_TESTS)

unit_test_t ASSOC_TESTS[]= {
  {test_assoc_create_destroy, "creation/destruction"},
  {test_assoc_basic, "basic use"},
  {test_assoc_for_each, "for-each"},
  {test_assoc_enum_keys, "enum (keys)"},
  {test_assoc_enum_values, "enum (values)"},
};
#define ASSOC_NTESTS ARRAY_SIZE(ASSOC_TESTS)

unit_test_t DLLIST_TESTS[]= {
  {test_dllist_basic, "basic use"},
};
#define DLLIST_NTESTS ARRAY_SIZE(DLLIST_TESTS)

unit_test_t HASH_SET_TESTS[]= {
  {test_hash_set_create_destroy, "creation/destruction"},
  {test_hash_set_add, "add"},
  {test_hash_set_add_ref, "add (ref)"},
  {test_hash_set_search, "search"},
  {test_hash_set_search_missing, "search (missing)"},
  {test_hash_set_remove, "remove"},
  {test_hash_set_remove_unref, "remove (unref)"},
  {test_hash_set_remove_missing, "remove (missing)"},
  {test_hash_set_for_each, "for-each"},
  {test_hash_set_enum, "enum"},
  {test_hash_set_strings, "strings"},
};
#define HASH_SET_NTESTS ARRAY_SIZE(HASH_SET_TESTS)

unit_test_t LIST_TESTS[]= {
  {test_list_basic, "basic use"},
};
#define LIST_NTESTS ARRAY_SIZE(LIST_TESTS)

unit_test_t RADIX_TESTS[]= {
  {test_radix_basic, "creation/destruction"},
  {test_radix_add_remove, "add/remove"},
  {test_radix_num_nodes, "num-nodes"},
  {test_radix_for_each, "for-each"},
  {test_radix_enum, "enum"},
  {test_radix_ipv4, "IPv4"},
};
#define RADIX_NTESTS ARRAY_SIZE(RADIX_TESTS)

unit_test_t BIT_VECTOR_TESTS[] = {
  { test_bit_vector_creation_destruction, "creation/destruction" },
  { test_bit_vector_representation,	  "to_string/from_string" },
  { test_bit_vector_manipulations,	  "set/unset/get" },
  { test_bit_vector_binary_operations,	  "and/or/xor" },
  { test_bit_vector_equality,		  "equals" }
};
#define BIT_VECTOR_NTESTS ARRAY_SIZE(BIT_VECTOR_TESTS)

unit_test_t BLOOM_HASH_TESTS[] = {
  { test_bloom_hash_creation_destruction, "creation/destruction" },
  { test_bloom_hash_insertion,		  "insertion" }
};
#define BLOOM_HASH_NTESTS ARRAY_SIZE(BLOOM_HASH_TESTS)

unit_test_t BLOOM_FILTER_TESTS[] = {
  { test_bloom_filter_creation_destruction, "creation/destruction" },
  { test_bloom_filter_insertion,	    "insertion" },
  { test_bloom_filter_membership,	    "membership" },
  { test_bloom_filter_binary_operations,    "and/or/xor" }
};
#define BLOOM_FILTER_NTESTS ARRAY_SIZE(BLOOM_FILTER_TESTS)

unit_test_suite_t SUITES[]= {
  {"String-Utilities", STRUTILS_NTESTS, STRUTILS_TESTS},
  {"FIFO", FIFO_NTESTS, FIFO_TESTS},
  {"Stack", STACK_NTESTS, STACK_TESTS},
  {"Enumerator", ENUM_NTESTS, ENUM_TESTS},
  {"Array", ARRAY_NTESTS, ARRAY_TESTS,
   test_before_array, NULL},
  {"Pointer-Array", PTRARRAY_NTESTS, PTRARRAY_TESTS},
  {"Associative-Array", ASSOC_NTESTS, ASSOC_TESTS},
  {"List", LIST_NTESTS, LIST_TESTS},
  {"Doubly-Linked-List", DLLIST_NTESTS, DLLIST_TESTS},
  {"Hash-Set", HASH_SET_NTESTS, HASH_SET_TESTS},
  {"Radix-Tree", RADIX_NTESTS, RADIX_TESTS,
   test_radix_before, NULL},
  {"Trie", TRIE_NTESTS, TRIE_TESTS},
  {"Tokenizer", TOKENIZER_NTESTS, TOKENIZER_TESTS},
  {"Params", PARAMS_NTESTS, PARAMS_TESTS},
  {"CLI", CLI_NTESTS, CLI_TESTS, test_before_cli, test_after_cli},
  {"Bit Vector", BIT_VECTOR_NTESTS, BIT_VECTOR_TESTS},
  {"Bloom Hash", BLOOM_HASH_NTESTS, BLOOM_HASH_TESTS},
  {"Bloom Filter", BLOOM_FILTER_NTESTS, BLOOM_FILTER_TESTS}
};
#define NUM_SUITES ARRAY_SIZE(SUITES)

// ----- main -------------------------------------------------------
int main(int argc, char * argv[])
{
  int result= 0;

  srandom(2007);
  gds_init(0);
  //gds_init(GDS_OPTION_MEMORY_DEBUG);

  utest_init(0);
  //utest_set_fork();
  utest_set_user(getenv("USER"));
  utest_set_project(PACKAGE_NAME, PACKAGE_VERSION);
  utest_set_xml_logging("libgds-check.xml");
  result= utest_run_suites(SUITES, NUM_SUITES);

  utest_done();
  
  gds_destroy();

  return (result==0?EXIT_SUCCESS:EXIT_FAILURE);
}
