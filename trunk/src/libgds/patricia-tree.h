// ==================================================================
// @(#)patricia-tree.h
//
// Patricia tree implementation.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 17/05/2005
// $Id$
// ==================================================================

#ifndef __PATRICIA_TREE_H__
#define __PATRICIA_TREE_H__

#include <libgds/array.h>
#include <libgds/stream.h>

typedef uint32_t trie_key_t;
typedef uint8_t trie_key_len_t;

#define TRIE_SUCCESS          0
#define TRIE_ERROR_DUPLICATE -1
#define TRIE_ERROR_NO_MATCH  -2

#define TRIE_INSERT_OR_REPLACE 1

#define TRIE_KEY_SIZE (sizeof(trie_key_t)*8)

typedef int (*FTrieForEach)(trie_key_t key, trie_key_len_t key_len,
			    void * data, void * ctx);
typedef void (*FTrieDestroy)(void ** data);

// -----[ STrie ]----------------------------------------------------
typedef struct gds_trie_t {
  struct _trie_item_t * root;
  FTrieDestroy destroy;
} gds_trie_t;

#ifdef __cplusplus
extern "C" {
#endif
  
  // -----[ trie_create ]--------------------------------------------
  gds_trie_t * trie_create(FTrieDestroy destroy);
  // -----[ trie_destroy ]-------------------------------------------
  void trie_destroy(gds_trie_t ** trie_ref);
  // -----[ trie_find_exact ]----------------------------------------
  void * trie_find_exact(gds_trie_t * trie, trie_key_t key,
			 trie_key_len_t key_len);
  // -----[ trie_find_best ]-----------------------------------------
  void * trie_find_best(gds_trie_t * trie, trie_key_t key,
			trie_key_len_t key_len);
  // -----[ trie_insert ]--------------------------------------------
  int trie_insert(gds_trie_t * trie, trie_key_t key,
		  trie_key_len_t key_len, void * data,
		  int replace);
  // -----[ trie_remove ]--------------------------------------------
  int trie_remove(gds_trie_t * trie, trie_key_t key,
		  trie_key_len_t key_len);
  // -----[ trie_remove ]--------------------------------------------
  int trie_replace(gds_trie_t * trie, trie_key_t key,
		   trie_key_len_t key_len, void * data);
  // -----[ trie_for_each ]------------------------------------------
  int trie_for_each(gds_trie_t * trie, FTrieForEach fForEach,
		    void * ctx);
  // -----[ trie_get_array ]-----------------------------------------
  ptr_array_t * trie_get_array(gds_trie_t * trie);
  // -----[ trie_get_enum ]------------------------------------------
  gds_enum_t * trie_get_enum(gds_trie_t * trie);
  // -----[ trie_num_nodes ]-----------------------------------------
  int trie_num_nodes(gds_trie_t * trie, int with_data);
  // -----[ trie_to_graphviz ]-----------------------------------------
  void trie_to_graphviz(gds_stream_t * stream, gds_trie_t * trie);

  // -----[ patricia_tree_init ]-------------------------------------
  void _patricia_tree_init();
  
#ifdef __cplusplus
}
#endif

#endif /* __PATRICIA_TREE_H__ */
