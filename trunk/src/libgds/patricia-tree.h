// ==================================================================
// @(#)patricia-tree.h
//
// Patricia tree implementation.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 05/04/2005
// @lastdate 05/04/2005
// ==================================================================

#ifndef __PATRICIA_TREE_H__
#define __PATRICIA_TREE_H__

typedef uint32_t trie_key_t;
typedef uint8_t trie_key_len_t;

#define TRIE_KEY_SIZE (sizeof(trie_key_t)*8)

typedef void (*FTrieForEach)(trie_key_t uKey, trie_key_len_t uKeyLen,
			     void * pData, void * pContext);
typedef void (*FTrieDestroy)(void * pData);

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
  
  // -----[ trie_create ]----------------------------------------------
  extern STrie * trie_create(FTrieDestroy fDestroy);
  // -----[ trie_insert ]----------------------------------------------
  extern int trie_insert(STrie * pTrie, trie_key_t uKey,
			 trie_key_len_t uKeyLen, void * pData);
  // -----[ trie_find_exact ]------------------------------------------
  extern void * trie_find_exact(STrie * pTrie, trie_key_t uKey,
				trie_key_len_t uKeyLen);
  // -----[ trie_find_best ]-------------------------------------------
  extern void * trie_find_best(STrie * pTrie, trie_key_t uKey,
			       trie_key_len_t uKeyLen);
  // -----[ trie_remove ]----------------------------------------------
  extern int trie_remove(STrie * pTrie, trie_key_t uKey,
			 trie_key_len_t uKeyLen);
  // -----[ trie_destroy ]---------------------------------------------
  extern void trie_destroy(STrie ** ppTrie);
  // -----[ trie_for_each ]--------------------------------------------
  extern void trie_for_each(STrie * pTrie, FTrieForEach fForEach,
			    void * pContext);

  
#ifdef __cplusplus
}
#endif

#endif /* __PATRICIA_TREE_H__ */
