// ==================================================================
// @(#)main.c
//
// Generic Data Structures (libgds): validation application.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @lastdate 08/03/2004
// ==================================================================

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

//#define _CHECK_ARRAY_
//#define _CHECK_PTR_ARRAY_
//#define _CHECK_LIST_
//#define _CHECK_RADIX_TREE_
//#define _CHECK_TOKENIZER_
//#define _CHECK_FIFO_
#define _CHECK_MEMORY_

int array_compare(void * pItem1, void * pItem2,
		  unsigned int uEltSize) 
{
  printf("%d <-> %d\n", *((int *) pItem1), *((int *) pItem2));
  if (*((int *) pItem1) > *((int *) pItem2))
    return 1;
  else if (*((int *) pItem1) < *((int *) pItem2))
    return -1;
  return 0;
}

// ----- test_array -------------------------------------------------
/**
 *
 */
void test_array()
{
  SIntArray * pArray, * pArrayCopy, * pSubArray;
  int iIndex;
  int iData;

  pArray= int_array_create(0/*ARRAY_OPTION_SORTED*/);
  iData= 27;
  assert(int_array_add(pArray, &iData) == 0);
  iData= 33;
  assert(int_array_add(pArray, &iData) == 0);
  iData= 5;
  assert(int_array_add(pArray, &iData) == 0);
  iData= 3;
  assert(int_array_add(pArray, &iData) == 0);
  iData= 7;
  assert(int_array_add(pArray, &iData) == 0);
  for (iIndex= 0; iIndex < int_array_length(pArray); iIndex++) {
    printf("(%d)>-->%d\n", iIndex, pArray->data[iIndex]);
  }
  pArrayCopy= (SIntArray *) _array_copy((SArray *) pArray);
  int_array_destroy(&pArray);
  int_array_remove_at(pArrayCopy, 1);
  for (iIndex= 0; iIndex < int_array_length(pArrayCopy); iIndex++) {
    printf("(%d)>-->%d\n", iIndex, pArrayCopy->data[iIndex]);
  }
  _array_sort((SArray *) pArrayCopy, array_compare);
  for (iIndex= 0; iIndex < int_array_length(pArrayCopy); iIndex++) {
    printf("(%d)>-->%d\n", iIndex, pArrayCopy->data[iIndex]);
  }
  pSubArray= (SIntArray *) _array_sub((SArray *) pArrayCopy, 2, 3);
  for (iIndex= 0; iIndex < int_array_length(pSubArray); iIndex++) {
    printf("(%d)>-->%d\n", iIndex, pSubArray->data[iIndex]);
  }
  int_array_destroy(&pArrayCopy);
  int_array_destroy(&pSubArray);
  printf("array-test: done.\n");
}

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

  printf("[%d:%d] vs [%d:%d]\n",
	 pRealItem1->uHighID, pRealItem1->uLowID,
	 pRealItem2->uHighID, pRealItem2->uLowID);

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
void test_ptr_array()
{
  SPtrArray * pPtrArray;
  SPtrArrayItem * pItem;
  int iIndex;

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
    printf("(%d)>-->[%d:%d], %d\n", iIndex,
	   pItem->uHighID, pItem->uLowID, pItem->uData);
  }
  ptr_array_destroy(&pPtrArray);
  printf("array-ptr-test: done.\n");
}

// ----- test_list --------------------------------------------------
/**
 *
 */
void test_list()
{
  SList * pList;

  pList= list_create(NULL, NULL, 1);
  list_destroy(&pList);
  printf("list-test: done.\n");
}

// ----- radix_tree_for_each_function -------------------------------
int radix_tree_for_each_function(uint32_t uKey, uint8_t uKeyLen,
				 void * pItem, void * pContext)
{
  printf("(%d/%d)>-->%d\n", uKey, uKeyLen, (int) pItem);
  return 0;
}

// ----- test_radix_tree --------------------------------------------
/**
 *
 */
void test_radix_tree()
{
  SRadixTree * pTree;

  pTree= radix_tree_create(32, NULL);

  printf("add(1.0.0.0/16, 100)\n");
  radix_tree_add(pTree, 256*256*256, 16, (void *) 100);
  printf("add(0.0.0.0/16, 200)\n");
  radix_tree_add(pTree, 0, 16, (void *) 200);
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
  printf("best(0.3.0.0/32)>-->%d\n", (int) radix_tree_get_best(pTree, 3*256*256, 32));
  printf("add(0.0.0.0/8, 300)\n");
  radix_tree_add(pTree, 0, 8, (void *) 300);
  radix_tree_for_each(pTree, radix_tree_for_each_function, NULL);
  printf("best(0.3.0.0/32)>-->%d\n", (int) radix_tree_get_best(pTree, 3*256*256, 32));

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
  printf("radix-tree: done.\n");
}

// ----- test_tokenizer ---------------------------------------------
/**
 *
 */
void test_tokenizer()
{
  STokenizer * pTokenizer= tokenizer_create(" \t", 0, "\"{", "\"}");
  long int lValue;
  double dValue;
  STokens * pTokens;
  int iIndex;
  int iResult;

  char * pcTest= str_create("abc");
  printf("[%s]\n", pcTest);
  str_append(&pcTest, "def");
  printf("[%s]\n", pcTest);
  str_nappend(&pcTest, "ghijkl", 3);
  printf("[%s]\n", pcTest);
  str_destroy(&pcTest);

  tokenizer_perror(stderr, tokenizer_run(pTokenizer, "abc def ghi"));
  pTokens= tokenizer_get_tokens(pTokenizer);
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++)
    printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  printf("\n");

  tokenizer_perror(stderr, tokenizer_run(pTokenizer, "abc \"def\" ghi"));
  pTokens= tokenizer_get_tokens(pTokenizer);
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++)
    printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  printf("\n");

  tokenizer_perror(stderr, tokenizer_run(pTokenizer, "123 abc\"def\"ghi 456"));
  pTokens= tokenizer_get_tokens(pTokenizer);
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++)
    printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  printf("\n");

  tokenizer_perror(stderr, tokenizer_run(pTokenizer, "123 abc\"def\"ghi {456}"));
  pTokens= tokenizer_get_tokens(pTokenizer);
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++)
    printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  printf("\n");

  tokenizer_perror(stderr, tokenizer_run(pTokenizer, "  in-filter \"\"\"\"\"\""));
  pTokens= tokenizer_get_tokens(pTokenizer);
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++)
    printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  printf("\n");

  iResult= tokenizer_run(pTokenizer, "abcdef 12345678901 ghijkl 123:45\t-123.456 \"Hello World !\" ceci\" n{'}est pas \"une{ pipe}");
  if (iResult)
    tokenizer_perror(stderr, iResult);
  pTokens= tokenizer_get_tokens(pTokenizer);
  for (iIndex= 0; iIndex < tokens_get_num(pTokens); iIndex++)
    printf("(%s)", tokens_get_string_at(pTokens, iIndex));
  printf("\n");
  printf("# tokens: %hu\n", tokens_get_num(pTokens));
  printf("item 0: \"%s\"\n", tokens_get_string_at(pTokens, 0));
  assert(!tokens_get_long_at(pTokens, 1, &lValue));
  printf("item 1: %ld\n", lValue);
  printf("item 2: \"%s\"\n", tokens_get_string_at(pTokens, 2));
  printf("item 3: \"%s\"\n", tokens_get_string_at(pTokens, 3));
  assert(!tokens_get_double_at(pTokens, 4, &dValue));
  printf("item 4: %f\n", dValue);
  printf("item 5: \"%s\"\n", tokens_get_string_at(pTokens, 5));
  printf("item 6: \"%s\"\n", tokens_get_string_at(pTokens, 6));


  tokenizer_destroy(&pTokenizer);

  printf("tokenizer: done.\n");
}

// ----- test_fifo --------------------------------------------------
/**
 *
 */
void test_fifo()
{
  SFIFO * pFIFO= fifo_create(5, NULL);

  fifo_destroy(&pFIFO);
}

// ----- test_memory ------------------------------------------------
void test_memory()
{
#define NALLOC 5
  char ** ppcTest;
  int iIndex;

  ppcTest= (char **) MALLOC(sizeof(char *)*NALLOC);
  fprintf(stderr, "ppcTest alloc : %p\n", ppcTest);
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    ppcTest[iIndex]= (char *) MALLOC(sizeof(char)*10);
    fprintf(stderr, "malloc : %p\n", ppcTest[iIndex]);
  }
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    ppcTest[iIndex] = REALLOC(ppcTest[iIndex], sizeof(char)*20);
    fprintf(stderr, "realloc : %p\n", ppcTest[iIndex]);
  }
  for (iIndex= 0; iIndex < NALLOC; iIndex++) {
    fprintf(stderr, "free : %p\n", ppcTest[iIndex]);
    FREE(ppcTest[iIndex]);
  }
  fprintf(stderr, "ppcTest free : %p\n", ppcTest);
  FREE(ppcTest);
  printf("memory-test: done.\n");
}

// ----- main -------------------------------------------------------
/**
 *
 */
int main(int argc, char * argv[])
{
#ifdef _CHECK_ARRAY_
  test_array();
#endif
#ifdef _CHECK_PTR_ARRAY_
  test_ptr_array();
#endif
#ifdef _CHECK_LIST_
  test_list();
#endif
#ifdef _CHECK_RADIX_TREE_
  test_radix_tree();
#endif
#ifdef _CHECK_TOKENIZER_
  test_tokenizer();
#endif
#ifdef _CHECK_FIFO_
  test_fifo();
#endif
#ifdef _CHECK_MEMORY_
  test_memory();
#endif

  return 0;
}
