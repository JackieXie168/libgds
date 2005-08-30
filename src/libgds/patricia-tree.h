// ==================================================================
// @(#)patricia-tree.h
//
// Patricia tree implementation.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 17/05/2005
// @lastdate 10/08/2005
// ==================================================================

#ifndef __PATRICIA_TREE_H__
#define __PATRICIA_TREE_H__

#include <libgds/array.h>

typedef uint32_t trie_key_t;
typedef uint8_t trie_key_len_t;

#define TRIE_KEY_SIZE (sizeof(trie_key_t)*8)

typedef int (*FTrieForEach)(trie_key_t uKey, trie_key_len_t uKeyLen,
			    void * pData, void * pContext);
typedef void (*FTrieDestroy)(void ** ppData);

// -----[ STrieItem ]------------------------------------------------
typedef struct TTrieItem STrieItem;
struct TTrieItem {
  STrieItem * pLeft; // 0
  STrieItem * pRight; // 1
  trie_key_t uKey;
  trie_key_len_t uKeyLen;
  void * pData;
};

// -----[ STrie ]----------------------------------------------------
typedef struct {
  STrieItem * pRoot;
  FTrieDestroy fDestroy;
} STrie;

#ifdef __cplusplus
extern "C" {
#endif
  
  // -----[ trie_create ]--------------------------------------------
  extern STrie * trie_create(FTrieDestroy fDestroy);
  // -----[ trie_insert ]--------------------------------------------
  extern int trie_insert(STrie * pTrie, trie_key_t uKey,
			 trie_key_len_t uKeyLen, void * pData);
  // -----[ trie_find_exact ]----------------------------------------
  extern void * trie_find_exact(STrie * pTrie, trie_key_t uKey,
				trie_key_len_t uKeyLen);
  // -----[ trie_find_best ]-----------------------------------------
  extern void * trie_find_best(STrie * pTrie, trie_key_t uKey,
			       trie_key_len_t uKeyLen);
  // -----[ trie_remove ]--------------------------------------------
  extern int trie_remove(STrie * pTrie, trie_key_t uKey,
			 trie_key_len_t uKeyLen);
  // -----[ trie_remove ]--------------------------------------------
  extern int trie_replace(STrie * pTrie, trie_key_t uKey,
			  trie_key_len_t uKeyLen, void * pData);
  // -----[ trie_destroy ]-------------------------------------------
  extern void trie_destroy(STrie ** ppTrie);
  // -----[ trie_for_each ]------------------------------------------
  extern int trie_for_each(STrie * pTrie, FTrieForEach fForEach,
			   void * pContext);
  // -----[ trie_get_array ]-------------------------------------------
  extern SPtrArray * trie_get_array(STrie * pTrie);
  // -----[ trie_get_enum ]--------------------------------------------
  extern SEnumerator * trie_get_enum(STrie * pTrie);

  // -----[ patricia_tree_init ]-------------------------------------
  extern void _patricia_tree_init();

  
#ifdef __cplusplus
}
#endif

#endif /* __PATRICIA_TREE_H__ */
