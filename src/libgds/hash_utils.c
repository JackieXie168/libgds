// ==================================================================
// @(#)hash.c
//
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @date 14/12/2004
// @lastdate 14/12/2004
// ==================================================================

#include <string.h>

// ----- hash_utils_key_compute_string -------------------------------
/**
 * Universal hash function for string keys (discussed in Sedgewick's
 * "Algorithms in C, 3rd edition") and adapted.
 *
 */
int hash_utils_key_compute_string(char * pcItem, unsigned int uModulo)
{
  int iHash, a = 31415, b = 27183;
  int iIndex;
  
  if (pcItem == NULL)
    return 0;

  iHash= 0;
  for (iIndex= 0; iIndex < strlen(pcItem); iIndex++) {
    iHash= (a*iHash+pcItem[iIndex])%uModulo;
    a= a*b%(uModulo-1);
  }
  return iHash;
}
