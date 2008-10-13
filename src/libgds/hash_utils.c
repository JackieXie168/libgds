// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 14/12/2004
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <libgds/hash_utils.h>

// ----- hash_utils_key_compute_string -------------------------------
/**
 * Universal hash function for string keys (discussed in Sedgewick's
 * "Algorithms in C, 3rd edition") and adapted.
 *
 */
uint32_t hash_utils_key_compute_string(const void * item,
				  unsigned int hash_size)
{
  const char * str= (const char *) item;
  uint32_t key, a = 31415, b = 27183;
  
  if (str == NULL)
    return 0;

  key= 0;
  while (*str != '\0') {
    key= (a*key + *str);
    a= a*b%(hash_size-1);
    str++;
  }
  return key % hash_size;
}

// -----[ hash_utils_compare_string ]------------------------------
int hash_utils_compare_string(const void * item1,
			      const void * item2,
			      unsigned int item_size)
{
  const char * str1= (const char *) item1;
  const char * str2= (const char *) item2;

  if ((str1 == NULL) && (str2 == NULL)) {
    return 0;
  } else if (str1 == NULL) {
    return -1;
  } else if (str2 == NULL) {
    return 1;
  }

  return strcmp(str1, str2);
}
