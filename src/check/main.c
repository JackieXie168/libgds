// ==================================================================
// @(#)main.c
//
// Generic Data Structures (libgds): validation application.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @lastdate 28/01/2005
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgds/array.h>
#include <libgds/fifo.h>
#include <libgds/list.h>
#include <libgds/memory.h>
#include <libgds/radix-tree.h>
#include <libgds/str_util.h>
#include <libgds/tokenizer.h>
#include <libgds/tokens.h>

#define MSG_CHECKING(MSG) \
  printf("\033[37;1m%s\033[0m", MSG)
#define MSG_RESULT_SUCCESS() \
  printf("\033[70G[\033[32;1mSUCCESS\033[0m]\n")
#define MSG_RESULT_FAIL() \
  printf("\033[70G[\033[31;1mFAIL\033[0m]\n")
#define ASSERT_RETURN(TEST, INFO) \
  if (!(TEST)) { \
    MSG_RESULT_FAIL(); \
    printf("  Reason: [%s]\n", INFO); \
    return -1; \
  }

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_ARRAY
/////////////////////////////////////////////////////////////////////

// ----- array_compare ----------------------------------------------
/**
 * Utility function used in test_array() to compare two array items
 * (integers).
 */
int array_compare(void * pItem1, void * pItem2,
		  unsigned int uEltSize) 
{
  if (*((int *) pItem1) > *((int *) pItem2))
    return 1;
  else if (*((int *) pItem1) < *((int *) pItem2))
    return -1;
  return 0;
}

// ----- test_array -------------------------------------------------
/**
 * Perform various tests with arrays.
 * [x] basic use with integers (add, length, get, copy, remove, sub)
 * [x] sorting
 * [ ] insertion
 */
int test_array()
{
#define NUM_ITEMS 5
  int aiArray[NUM_ITEMS]= {27, 33, 5, 3, 7};
  SIntArray * pArray, * pArrayCopy, * pSubArray;
  int iIndex;
  int iData;
  int iTestResult= 0;

  MSG_CHECKING("* Basic use");

  /* BASIC USE WITH INTEGERS */
  pArray= int_array_create(0);
  
  /* Add */
  for (iIndex= 0; iIndex < NUM_ITEMS; iIndex++) {
    iData= aiArray[iIndex];
    /* int_array_add() must return the index of insertion */
    ASSERT_RETURN(int_array_add(pArray, &iData) == iIndex,
		  "int_array_add() does not return the index of insertion")
  }

  /* Length */
  ASSERT_RETURN(NUM_ITEMS == int_array_length(pArray),
		"int_array_length() returns an incorrect length");

  /* Direct get (as a C array) */
  for (iIndex= 0; iIndex < NUM_ITEMS; iIndex++) {
    ASSERT_RETURN(pArray->data[iIndex] == aiArray[iIndex],
		  "direct read does not return expected value");
  }

  /* Get */
  for (iIndex= 0; iIndex < NUM_ITEMS; iIndex++) {
    _array_get_at((SArray *) pArray, iIndex, &iData);
    ASSERT_RETURN(iData == aiArray[iIndex],
		  "_array_get_at() does not return expected value");
  }

  /* Copy */
  pArrayCopy= (SIntArray *) _array_copy((SArray *) pArray);
  ASSERT_RETURN(int_array_length(pArrayCopy) == int_array_length(pArray),
		"length of copied array does not match length of original array");
  for (iIndex= 0; iIndex < int_array_length(pArrayCopy); iIndex++) {
    ASSERT_RETURN(pArray->data[iIndex] == pArrayCopy->data[iIndex],
		  "data in copied array does not match data in original array");
  }

  /* Remove (remove even elements) */
  for (iIndex= 0; iIndex < (NUM_ITEMS-1)/2; iIndex++) {
    int_array_remove_at(pArrayCopy, iIndex+1);
  }
  ASSERT_RETURN(int_array_length(pArrayCopy) == (NUM_ITEMS+1)/2,
		"unexpected number of elements after int_array_remove_at()");

  MSG_RESULT_SUCCESS();
  MSG_CHECKING("* Sorting");

  /* Sort (ascending sequence) */
  _array_sort((SArray *) pArray, array_compare);
  for (iIndex= 0; iIndex < int_array_length(pArray); iIndex++) {
    if (iIndex > 0) {
      ASSERT_RETURN(pArray->data[iIndex-1] <= pArray->data[iIndex],
		    "ascending ordering not respected after _array_sort()");
    }
  }

  /* Sub (extract a sub-array) */
  pSubArray= (SIntArray *) _array_sub((SArray *) pArray, 2, 3);

  /* Destroy */
  int_array_destroy(&pArray);
  int_array_destroy(&pArrayCopy);
  int_array_destroy(&pSubArray);

  MSG_RESULT_SUCCESS();

  return iTestResult;
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

  MSG_CHECKING("* Basic use");

  pPtrArray= ptr_array_create(ARRAY_OPTION_SORTED,
			      array_ptr_compare_function,
			      array_ptr_destroy_function);
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

  MSG_RESULT_SUCCESS();

  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_LIST
/////////////////////////////////////////////////////////////////////

// ----- test_list --------------------------------------------------
/**
 *
 */
int test_list()
{
  SList * pList;

  pList= list_create(NULL, NULL, 1);
  list_destroy(&pList);

  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_RADIX_TREE
/////////////////////////////////////////////////////////////////////

// ----- radix_tree_for_each_function -------------------------------
int radix_tree_for_each_function(uint32_t uKey, uint8_t uKeyLen,
				 void * pItem, void * pContext)
{
  return 0;
}

// ----- test_radix_tree --------------------------------------------
/**
 *
 */
int test_radix_tree()
{
  SRadixTree * pTree;

  pTree= radix_tree_create(32, NULL);

  MSG_CHECKING("* Basic use");

  //printf("add(1.0.0.0/16, 100)\n");
  radix_tree_add(pTree, 256*256*256, 16, (void *) 100);
  //printf("add(0.0.0.0/16, 200)\n");
  radix_tree_add(pTree, 0, 16, (void *) 200);
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
  //printf("best(0.3.0.0/32)>-->%d\n", (int) radix_tree_get_best(pTree, 3*256*256, 32));
  //printf("add(0.0.0.0/8, 300)\n");
  radix_tree_add(pTree, 0, 8, (void *) 300);
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
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

  MSG_RESULT_SUCCESS();

  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_TOKENIZER
/////////////////////////////////////////////////////////////////////

// ----- test_tokenizer ---------------------------------------------
/**
 * Perform various tests with tokenizer
 * [x] Test 1 ["abc def ghi"]
 * [x] Test 2 ["abc \"def\" ghi"]
 * [x] Test 3 ["123 abc\"def\"ghi 456"]
 * [x] Test 4 ["123 abc\"def\"ghi { 456 }"]
 * [x] Test 5 ["  in-filter \"\"\"\"\"\""]
 * [x] Test 6 ["abcdef 12345678901 ghijkl 123:45\t-123.456 \"Hello World !\" ceci\" n{'}est pas \"une{ pipe}"]
 */
int test_tokenizer()
{
  STokenizer * pTokenizer;
  long int lValue;
  double dValue;
  STokens * pTokens;
  int iIndex;
  int iResult;
  int iTestResult= 0;

  pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");

  MSG_CHECKING("* Test 1");
  iResult= tokenizer_run(pTokenizer, "abc def ghi");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [abc def ghi] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    //printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  }
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Test 2");
  iResult= tokenizer_run(pTokenizer, "abc \"def\" ghi");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [abc \"def\" ghi] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    //printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  }
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Test 3");
  iResult= tokenizer_run(pTokenizer, "123 abc\"def\"ghi 456");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [123 abc\"def\"ghi 456] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    //printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  }
  MSG_RESULT_SUCCESS();


  MSG_CHECKING("* Test 4");
  iResult= tokenizer_run(pTokenizer, "123 abc\"def\"ghi { 456 }");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [123 abc\"def\"ghi { 456 }] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 3,
		"wrong number of tokens");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    //printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  }
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Test 5");
  iResult= tokenizer_run(pTokenizer, "  in-filter \"\"\"\"\"\"");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [  in-filter \"\"\"\"\"\"] failed")
    pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 2,
		"wrong number of tokens");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    //printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  }
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Test 6");
  iResult= tokenizer_run(pTokenizer, "abcdef 12345678901 ghijkl 123:45\t-123.456 \"Hello World !\" ceci\" n{'}est pas \"une{ pipe}");
  ASSERT_RETURN(iResult == TOKENIZER_SUCCESS,
		"tokenization of [abcdef 12345678901 ghijkl 123:45\t-123.456 \"Hello World !\" ceci\" n{'}est pas \"une{ pipe}] failed");
  pTokens= tokenizer_get_tokens(pTokenizer);
  ASSERT_RETURN(tokens_get_num(pTokens) == 7,
		"wrong number of tokens");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    //printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  }
  MSG_RESULT_SUCCESS();

  //printf("\n");
  //printf("# tokens: %hu\n", tokens_get_num(pTokens));
  //printf("item 0: \"%s\"\n", tokens_get_string_at(pTokens, 0));
  //assert(!tokens_get_long_at(pTokens, 1, &lValue));
  //printf("item 1: %ld\n", lValue);
  //printf("item 2: \"%s\"\n", tokens_get_string_at(pTokens, 2));
  //printf("item 3: \"%s\"\n", tokens_get_string_at(pTokens, 3));
  //assert(!tokens_get_double_at(pTokens, 4, &dValue));
  //printf("item 4: %f\n", dValue);
  //printf("item 5: \"%s\"\n", tokens_get_string_at(pTokens, 5));
  //printf("item 6: \"%s\"\n", tokens_get_string_at(pTokens, 6));

  tokenizer_destroy(&pTokenizer);

  return iTestResult;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_FIFO
/////////////////////////////////////////////////////////////////////

// ----- test_fifo --------------------------------------------------
/**
 *
 */
int test_fifo()
{
  SFIFO * pFIFO;

  MSG_CHECKING("* Basic use");

  pFIFO= fifo_create(5, NULL);

  fifo_destroy(&pFIFO);

  MSG_RESULT_SUCCESS();

  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_MEMORY
/////////////////////////////////////////////////////////////////////

// ----- test_memory ------------------------------------------------
int test_memory()
{
#define NALLOC 5
  char ** ppcTest;
  int iIndex;

  printf("alloc-count: %d\n", mem_alloc_cnt());

  MSG_CHECKING("* Allocation");

  ppcTest= (char **) MALLOC(sizeof(char *)*NALLOC);
  //fprintf(stderr, "ppcTest alloc : %p\n", ppcTest);
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    ppcTest[iIndex]= (char *) MALLOC(sizeof(char)*10);
    //fprintf(stderr, "malloc : %p\n", ppcTest[iIndex]);
  }
  MSG_RESULT_SUCCESS();

  printf("alloc-count: %d\n", mem_alloc_cnt());

  MSG_CHECKING("* Re-allocation");
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    ppcTest[iIndex] = REALLOC(ppcTest[iIndex], sizeof(char)*20);
    //fprintf(stderr, "realloc : %p\n", ppcTest[iIndex]);
  }
  MSG_RESULT_SUCCESS();

  printf("alloc-count: %d\n", mem_alloc_cnt());

  MSG_CHECKING("* Freeing");
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    //fprintf(stderr, "free : %p\n", ppcTest[iIndex]);
    printf("alloc-count-before: %d\n", mem_alloc_cnt());
    FREE(ppcTest[iIndex]);
    printf("alloc-count-after: %d\n", mem_alloc_cnt());
  }
  //fprintf(stderr, "ppcTest free : %p\n", ppcTest);
  FREE(ppcTest);
  MSG_RESULT_SUCCESS();

  return 0;
}

/////////////////////////////////////////////////////////////////////
// MAIN PART
/////////////////////////////////////////////////////////////////////

// ----- main -------------------------------------------------------
/**
 *
 */
int main(int argc, char * argv[])
{
#ifdef _GDS_CHECK_ARRAY_
  printf("CHECK [array]\n");
  return test_array();
#endif
#ifdef _GDS_CHECK_PTR_ARRAY_
  printf("CHECK [ptr_array]\n");
  return test_ptr_array();
#endif
#ifdef _GDS_CHECK_LIST_
  printf("CHECK [list]\n");
  return test_list();
#endif
#ifdef _GDS_CHECK_RADIX_TREE_
  printf("CHECK [radix_tree]\n");
  return test_radix_tree();
#endif
#ifdef _GDS_CHECK_TOKENIZER_
  printf("CHECK [tokenizer]\n");
  return test_tokenizer();
#endif
#ifdef _GDS_CHECK_FIFO_
  printf("CHECK [fifo]\n");
  return test_fifo();
#endif
#ifdef _GDS_CHECK_MEMORY_
  printf("CHECK [memory]\n");
  return test_memory();
#endif

  fprintf(stderr, "Error: no test requested.\n");
  return -1;
}
