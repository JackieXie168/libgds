// ==================================================================
// @(#)main.c
//
// Generic Data Structures (libgds): validation application.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @lastdate 31/07/2006
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgds/array.h>
#include <libgds/cli.h>
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

#define IPV4_TO_INT(A,B,C,D) (((((uint32_t)(A))*256 +(uint32_t)(B))*256 +(uint32_t)( C))*256 +(uint32_t)(D))

static char acAddress[16];
char * INT_TO_IPV4(unsigned int uAddress)
{
  snprintf(acAddress, sizeof(acAddress), "%u.%u.%u.%u",
	   (uAddress >> 24), (uAddress >> 16) & 255,
	   (uAddress >> 8) & 255, uAddress & 255);
  return acAddress;
}

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
// GDS_CHECK_DLLIST
/////////////////////////////////////////////////////////////////////

// ----- _test_dllist_for_each --------------------------------------
static int _test_dllist_for_each(void * pUserData, void * pContext)
{
  fprintf(stdout, "for-each-dllist-item: %d\n", (int) pUserData);
  return 0;
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
  int iValue;

  pList= dllist_create(NULL);

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
  if ((dllist_get(pList, 0, (void **) &iValue) != 0) ||
      iValue != 1)
    iResult= -1;
  if ((dllist_get(pList, 1, (void **) &iValue) != 0) ||
      iValue != 110)
    iResult= -1;
  if ((dllist_get(pList, 2, (void **) &iValue) != 0) ||
      iValue != 3)
    iResult= -1;

  dllist_destroy(&pList);

  return iResult;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_RADIX_TREE
/////////////////////////////////////////////////////////////////////

// ----- radix_tree_for_each_function -------------------------------
int radix_tree_for_each_function(uint32_t uKey, uint8_t uKeyLen,
				 void * pItem, void * pContext)
{
  fprintf(stdout, "key %d.%d.%d.%d/%d --> value %d\n",
	  (uKey >> 24) & 255, (uKey >> 16) & 255,
	  (uKey >> 8) & 255, uKey & 255,
	  uKeyLen, (int) pItem);

  return 0;
}

// ----- radix_tree_dummy_destroy -----------------------------------
void radix_tree_dummy_destroy(void ** ppItem)
{
  printf("(dummy-destroy %p)\n", *ppItem);
}

// ----- test_radix_tree --------------------------------------------
/**
 *
 */
int test_radix_tree()
{
  SRadixTree * pTree;

  MSG_CHECKING("* Basic use");

  pTree= radix_tree_create(32, NULL);

  radix_tree_add(pTree, IPV4_TO_INT(1,0,0,0), 16, (void *) 100);
  radix_tree_add(pTree, IPV4_TO_INT(0,0,0,0), 16, (void *) 200);
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
  //printf("best(0.3.0.0/32)>-->%d\n", (int) radix_tree_get_best(pTree, 3*256*256, 32));
  //printf("add(0.0.0.0/8, 300)\n");
  radix_tree_add(pTree, IPV4_TO_INT(0,0,0,0), 8, (void *) 300);
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

  MSG_CHECKING("* IPv4 use");

  pTree= radix_tree_create(32, radix_tree_dummy_destroy);

  radix_tree_add(pTree, IPV4_TO_INT(12,0,0,0), 8, (void *) 1);
  radix_tree_add(pTree, IPV4_TO_INT(12,148,170,0), 24, (void *) 2);
  radix_tree_add(pTree, IPV4_TO_INT(199,165,16,0), 20, (void *) 3);
  radix_tree_add(pTree, IPV4_TO_INT(199,165,16,0), 24, (void *) 4);

  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);

  radix_tree_remove(pTree, IPV4_TO_INT(12,0,0,0), 8, 1);

  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);

  fprintf(stderr, "exact(199.165.16.0/20): %d\n",
	  (int)radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 20));
  fprintf(stderr, "exact(199.165.16.0/24): %d\n",
	  (int)radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 24));
  fprintf(stderr, "best(199.165.16.0/32): %d\n",
	  (int)radix_tree_get_best(pTree, IPV4_TO_INT(199,165,16,0), 32));

  radix_tree_remove(pTree, IPV4_TO_INT(199,165,16,0), 24, 1);

  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);

  fprintf(stderr, "exact(199.165.16.0/20): %d\n",
	  (int)radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 20));
  fprintf(stderr, "exact(199.165.16.0/24): %d\n",
	  (int)radix_tree_get_exact(pTree, IPV4_TO_INT(199,165,16,0), 24));
  fprintf(stderr, "best(199.165.16.0/32): %d\n",
	  (int)radix_tree_get_best(pTree, IPV4_TO_INT(199,165,16,0), 32));

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
  //long int lValue;
  //double dValue;
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

  //printf("alloc-count: %d\n", mem_alloc_cnt());

  MSG_CHECKING("* Allocation");

  ppcTest= (char **) MALLOC(sizeof(char *)*NALLOC);
  //fprintf(stderr, "ppcTest alloc : %p\n", ppcTest);
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    ppcTest[iIndex]= (char *) MALLOC(sizeof(char)*10);
    //fprintf(stderr, "malloc : %p\n", ppcTest[iIndex]);
  }
  MSG_RESULT_SUCCESS();

  //printf("alloc-count: %d\n", mem_alloc_cnt());

  MSG_CHECKING("* Re-allocation");
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    ppcTest[iIndex] = REALLOC(ppcTest[iIndex], sizeof(char)*20);
    //fprintf(stderr, "realloc : %p\n", ppcTest[iIndex]);
  }
  MSG_RESULT_SUCCESS();

  //printf("alloc-count: %d\n", mem_alloc_cnt());

  MSG_CHECKING("* Freeing");
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    //fprintf(stderr, "free : %p\n", ppcTest[iIndex]);
    //printf("alloc-count-before: %d\n", mem_alloc_cnt());
    FREE(ppcTest[iIndex]);
    //printf("alloc-count-after: %d\n", mem_alloc_cnt());
  }
  //fprintf(stderr, "ppcTest free : %p\n", ppcTest);
  FREE(ppcTest);
  MSG_RESULT_SUCCESS();

  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_PATRICIA_TREE
/////////////////////////////////////////////////////////////////////

// -----[ destroy ]--------------------------------------------------
void destroy(void ** ppData)
{
  //fprintf(stdout, "destroy %u\n", (int) pData);
}

// -----[ dump ]-----------------------------------------------------
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

// -----[ dump_exact ]-----------------------------------------------
void dump_exact(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  void * pData= trie_find_exact(pTrie, uKey, uKeyLen);
  printf("exact %s/%u => ", INT_TO_IPV4(uKey), uKeyLen);
  if (pData == NULL)
    printf("(null)\n");
  else
    printf("%u\n", (int) pData);
}

// -----[ dump_best ]------------------------------------------------
void dump_best(STrie * pTrie, trie_key_t uKey, trie_key_len_t uKeyLen)
{
  void * pData= trie_find_best(pTrie, uKey, uKeyLen);
  printf("best %s/%u => ", INT_TO_IPV4(uKey), uKeyLen);
  if (pData == NULL)
    printf("(null)\n");
  else
    printf("%u\n", (int) pData);
}

// ----- test_patricia_tree -----------------------------------------
int test_patricia_tree()
{
  STrie * pTrie= trie_create(destroy);

  MSG_CHECKING("* Insertion");
  trie_insert(pTrie, IPV4_TO_INT(0, 128, 0, 0), 16, (void *) 100);
  trie_insert(pTrie, IPV4_TO_INT(0, 192, 0, 0), 15, (void *) 200);
  trie_insert(pTrie, IPV4_TO_INT(0, 0, 0, 0), 16, (void *) 1);
  trie_insert(pTrie, IPV4_TO_INT(0, 0, 0, 0), 15, (void *) 2);
  trie_insert(pTrie, IPV4_TO_INT(0, 0, 0, 0), 16, (void *) -1);
  trie_insert(pTrie, IPV4_TO_INT(0, 1, 0, 0), 16, (void *) 3);
  trie_insert(pTrie, IPV4_TO_INT(0, 2, 0, 0), 16, (void *) 4);
  trie_insert(pTrie, IPV4_TO_INT(0, 1, 1, 1), 24, (void *) 5);
  trie_insert(pTrie, IPV4_TO_INT(0, 1, 1, 128), 25, (void *) 6);
  trie_insert(pTrie, IPV4_TO_INT(0, 128, 128, 128), 9, (void *) 300);
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Exact matching");
  dump_exact(pTrie, IPV4_TO_INT(0, 0, 0, 0), 16);
  dump_exact(pTrie, IPV4_TO_INT(0, 0, 0, 0), 15);
  dump_exact(pTrie, IPV4_TO_INT(0, 1, 0, 0), 16);
  dump_exact(pTrie, IPV4_TO_INT(0, 2, 0, 0), 16);
  dump_exact(pTrie, IPV4_TO_INT(0, 1, 1, 0), 24);
  dump_exact(pTrie, IPV4_TO_INT(0, 1, 1, 128), 25);
  dump_exact(pTrie, IPV4_TO_INT(0, 128, 0, 0), 9);
  dump_exact(pTrie, IPV4_TO_INT(0, 192, 0, 0), 15);
  dump_exact(pTrie, IPV4_TO_INT(0, 192, 128, 128), 9);
  dump_exact(pTrie, IPV4_TO_INT(0, 192, 0, 0), 10);
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Best matching");
  dump_best(pTrie, IPV4_TO_INT(0, 192, 0, 0), 32);
  dump_best(pTrie, IPV4_TO_INT(0, 192, 128, 128), 32);
  dump_best(pTrie, IPV4_TO_INT(0, 128, 128, 128), 32);
  dump_best(pTrie, IPV4_TO_INT(0, 1, 0, 0), 16);
  dump_best(pTrie, IPV4_TO_INT(0, 2, 0, 0), 16);
  dump_best(pTrie, IPV4_TO_INT(0, 0, 0, 0), 23);
  dump_best(pTrie, IPV4_TO_INT(0, 0, 0, 0), 15);
  MSG_RESULT_SUCCESS();

  MSG_CHECKING("* Removal");
  fprintf(stdout, "DUMP: {\n");
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
  fprintf(stdout, "}\n");
  MSG_RESULT_SUCCESS();

  trie_destroy(&pTrie);
  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_CLI
/////////////////////////////////////////////////////////////////////

// -----[ _cli_cmd1 ]------------------------------------------------
int _cli_cmd1(SCliContext * pContext, STokens * pTokens)
{
  int iIndex;

  printf("cli_cmd1:");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    printf(" %s", tokens_get_string_at(pTokens, iIndex));
  }
  printf("\n");
  return CLI_SUCCESS;
}

// -----[ _cli_cmd2 ]------------------------------------------------
int _cli_cmd2(SCliContext * pContext, STokens * pTokens)
{
  int iIndex;
  printf("cli_cmd2:");
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++) {
    printf(" %s", tokens_get_string_at(pTokens, iIndex));
  }
  printf("\n");
  return CLI_SUCCESS;
}

// -----[ test_cli ]-------------------------------------------------
int test_cli()
{
  SCli * pCli;
  SCliCmds * pCmds;
  SCliParams * pParams;
  int iResult;

  pCli= cli_create();
  pCmds= cli_cmds_create();
  pParams= cli_params_create();
  cli_params_add(pParams, "<param1>", NULL);
  cli_params_add(pParams, "<param2>", NULL);
  cli_params_add_vararg(pParams, "<param3>", 5, NULL);
  // Adding an additional parameter should abort() the program
  // cli_params_add(pParams, "<param4>", NULL);
  cli_cmds_add(pCmds, cli_cmd_create("cmd1", _cli_cmd1, NULL, pParams));
  pParams= cli_params_create();
  cli_params_add(pParams, "<param1>", NULL);
  cli_cmds_add(pCmds, cli_cmd_create("cmd2", _cli_cmd2, NULL, pParams));
  cli_register_cmd(pCli, cli_cmd_create("test", NULL, pCmds, NULL));

  // Dump registered commands
  cli_cmd_dump(pLogOut, "", pCli->pBaseCommand);

  // Call cmd1 with 3 varargs
  if (cli_execute(pCli, "test cmd1 arg1 arg2 vararg1 vararg2 vararg3") != CLI_SUCCESS) {
    printf("error: could not execute command\n");
    return -1;
  }

  // Call cmd1 with no varargs (this is allowed)
  if (cli_execute(pCli, "test cmd1 arg1 arg2") != CLI_SUCCESS) {
    printf("error: could not execute command\n");
    return -1;
  }
  
  // Call cmd1 with too many varargs
  iResult= cli_execute(pCli, "test cmd1 arg1 arg2 va1 va2 va3 va4 va5 va6");
  if (iResult != CLI_ERROR_TOO_MANY_PARAMETERS) {
    fprintf(stderr, "error: wrong error reported: ");
    cli_perror(pLogErr, iResult);
    return -1;
  } else {
    fprintf(stderr, "reported error: ");
    cli_perror(pLogErr, iResult);
  }
  
  // Standard call for cmd2
  if (cli_execute(pCli, "test cmd2 arg1") != CLI_SUCCESS) {
    printf("error: could not execute command\n");
    return -1;
  }

  cli_destroy(&pCli);
  return 0;
}

/////////////////////////////////////////////////////////////////////
// GDS_CHECK_HASH
/////////////////////////////////////////////////////////////////////

// -----[ _hash_cmp ]------------------------------------------------
int _hash_cmp(void * pElt1, void * pElt2, uint32_t uEltSize)
{
  if ((unsigned int) pElt1 > (unsigned int) pElt2) {
    return 1;
  } else if ((unsigned int) pElt1 < (unsigned int) pElt2) {
    return -1;
  } else
    return 0;
}

// -----[ _hash_destroy ]-------------------------------------------
void _hash_destroy(void * pElt)
{
}

// -----[ _hash_fct ]------------------------------------------------
uint32_t _hash_fct(void * pElt, uint32_t uHashSize)
{
  return ((unsigned int) pElt) % uHashSize;
}

// -----[ _hash_for_each ]-------------------------------------------
int _hash_for_each(void * pElt, void * pContext)
{
  fprintf(stderr, "for-each-item: %d\n", (unsigned int) pElt);
  return 0;
}

// -----[ test_hash ]------------------------------------------------
int test_hash()
{
  SHash * pHash;
  SEnumerator * pEnum;
  uint32_t uNbr;

  printf("Test in static mode\n");
  pHash = hash_init(15, 0, _hash_cmp, _hash_destroy, _hash_fct);
  for (uNbr = 0; uNbr < 100; uNbr++) {
    hash_add(pHash, (void *) uNbr);
  }
  hash_for_each(pHash, _hash_for_each, NULL);
  pEnum= hash_get_enum(pHash);
  while (enum_has_next(pEnum)) {
    printf("hash-item: %d\n", (unsigned int) enum_get_next(pEnum));
  }

  printf("Test in dynamic mode\n");
  pHash= hash_init(3, 0.75, _hash_cmp, _hash_destroy, _hash_fct);
  for (uNbr = 0; uNbr < 100; uNbr++) {
    hash_add(pHash, (void *) uNbr);
  }
  hash_for_each(pHash, _hash_for_each, NULL);
  pEnum= hash_get_enum(pHash);
  while (enum_has_next(pEnum)) {
    printf("hash-item: %d\n", (unsigned int) enum_get_next(pEnum));
  }
  hash_destroy(&pHash);
  return 0;
}

/////////////////////////////////////////////////////////////////////
// MAIN PART
/////////////////////////////////////////////////////////////////////

// -----[ definition of tests ]--------------------------------------
typedef int (*FTest)();
typedef struct {
  FTest fTest;
  char * pcName;
} STest;
#define NUM_TESTS 11
STest TEST_LIST[]= {
  {test_memory, "Memory management"},
  {test_array, "Basic arrays"},
  {test_ptr_array, "Arrays of pointers"},
  {test_fifo, "FIFO"},
  {test_list, "Lists"},
  {test_dllist, "Doubly-linked lists"},
  {test_hash, "Hashs"},
  {test_radix_tree, "Radix trees"},
  {test_patricia_tree, "Patricia trees"},
  {test_tokenizer, "Tokenizers"},
  {test_cli, "Command-line interface"}
};

// ----- main -------------------------------------------------------
/**
 *
 */
int main(int argc, char * argv[])
{
  int iIndex;
  int iResult;

  gds_init(0);
  for (iIndex= 0; iIndex < NUM_TESTS; iIndex++) {
    printf("Testing [\033[1m%s\033[0m]\n", TEST_LIST[iIndex].pcName);
    iResult= TEST_LIST[iIndex].fTest();
    if (iResult != 0) {
      printf("=> Result:\033[70G[\033[33;1mFAIL\033[0m]\n");
      break;
    } else {
      printf("=> Result:\033[70G[\033[32;1mSUCCESS\033[0m]\n");
    }
  }
  gds_destroy();

  return iResult;
}
