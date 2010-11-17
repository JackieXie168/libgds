
// ==================================================================
// @(#)trie_dico.c
//
// Dictionnaire  compact trie_dico implementation.
//
// @author Stefan Beauport (stefan.beauport@umons.ac.be)
// @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
// @date 19/08/2010
// $Id$
// ==================================================================

#include <string.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>

#include <libgds/array.h>
#include <libgds/memory.h>
#include <libgds/trie_dico.h>
#include <libgds/stack.h>

// -----[ _trie_dico_item_t ]------------------------------------------------
typedef struct _trie_dico_item_t {
  struct _trie_dico_item_t * child;
  struct _trie_dico_item_t * brother;
//  struct _trie_dico_item_t * father;   // null if father is root
  trie_dico_key_t                     key_part;
  trie_dico_key_t                     key;
  uint8_t                       is_final_data;
  void *     data;
} _trie_dico_item_t;



//////////////////////////
//
/// Printing stuff
//
//////////////////////////


static inline
void _trie_dico_print_item(_trie_dico_item_t * item, int depth)
{
    int sizeoftab = 15;

    if (item == NULL )
        return;
    // oneself
    for(int i = 0 ; i < depth; i++ )
    {
        printf("-");
    }
    printf("%s",item->key_part);

    for(int i = 0 ; i < sizeoftab - strlen(item->key); i++ )
    {
        printf(" ");
    }
    printf("(%s)", item->key);

    for(int i = 0 ; i < sizeoftab - strlen(item->key); i++ )
    {
        printf(" ");
    }
    if(item->is_final_data) printf("V");
    else printf("X");

    if(item->child == NULL) printf("    NO CHILD  ");
    else                    printf("       CHILD  ");

    if(item->brother == NULL) printf("  NO BROTHER  ");
    else                      printf("     BROTHER  ");


    printf("\n");

    // my child
    _trie_dico_print_item(item->child, depth + strlen(item->key_part));
    // my brother
    _trie_dico_print_item(item->brother, depth);


}

static inline
void trie_dico_print(gds_trie_dico_t * trie_dico)
{
    printf("\n*****************\n");
    _trie_dico_print_item(trie_dico->root,0);
    printf("*****************\n");

}
static inline
void _trie_dico_print_from(_trie_dico_item_t * root)
{
    printf("\n*****************\n");
    _trie_dico_print_item(root,0);
    printf("*****************\n");

}

// -----[ _trie_dico_item_create_data ]-----------------------------------
/**
 * Create a new node for the Patricia tree. Note: the function will
 * take care of correctly masking the node's key according to its
 * length.
 */
static inline
_trie_dico_item_t * _trie_dico_item_create_data(trie_dico_key_t key,
                                      trie_dico_key_t key_part,
				      void * data)
{
    //printf("@%s\n",__func__);
  _trie_dico_item_t * trie_dico_item= (_trie_dico_item_t *) MALLOC(sizeof(_trie_dico_item_t));
  trie_dico_item->child= NULL;
  trie_dico_item->brother= NULL;
  trie_dico_item->key= key;
  trie_dico_item->key_part= key_part;
  trie_dico_item->is_final_data= 1; // 1 : vrai
  trie_dico_item->data= data;
  return trie_dico_item;
}

// -----[ _trie_dico_item_create_empty ]----------------------------------
static inline
_trie_dico_item_t * _trie_dico_item_create_empty(trie_dico_key_t key,
                                      trie_dico_key_t key_part)
{
 //printf("@%s\n",__func__);

  _trie_dico_item_t * trie_dico_item= (_trie_dico_item_t *) MALLOC(sizeof(_trie_dico_item_t));
  trie_dico_item->child= NULL;
  trie_dico_item->brother= NULL;
  trie_dico_item->key= key;
  trie_dico_item->key_part= key_part;
  trie_dico_item->is_final_data= 0; // 0 : faux
  trie_dico_item->data= NULL;
  return trie_dico_item;
}


// -----[ trie_dico_create ]----------------------------------------------
/**
 * Create a new Dico tree.
 */
gds_trie_dico_t * trie_dico_create(gds_trie_dico_destroy_f destroy)
{
//printf("@%s\n",__func__);

  gds_trie_dico_t * trie_dico= (gds_trie_dico_t *) MALLOC(sizeof(gds_trie_dico_t));
  trie_dico->root= NULL;
  trie_dico->destroy= destroy;
  return trie_dico;
}

// -----[ _find_father ]---------------------------------------------
/**
 * we want to insert a key/value pair
 *  this function allows to find the father of our new node.
 *
 * Result: the father of the new node, or NULL if it should be at the first visited floor (root)
 */
static _trie_dico_item_t * _find_father(_trie_dico_item_t * root,
                           trie_dico_key_t key)
{ //printf("@%s\n",__func__);//printf("@find_father\n");
    _trie_dico_item_t * current_father = NULL;
    _trie_dico_item_t * uncle = root;
    
    trie_dico_key_t current_key_part;
    current_key_part = MALLOC(sizeof(char)*( strlen(key) +1));
    strcpy(current_key_part, key);
    trie_dico_key_t key_part_tmp;
    key_part_tmp = MALLOC(sizeof(char)*( strlen(key) +1));
    
    while( uncle != NULL)
    {
        // pour cette profondeur :
        int found_ancester = 0;
        while(!found_ancester && uncle != NULL && strlen(current_key_part)>0 )
        {
            if(current_key_part[0] < (uncle->key_part)[0] )
            { // stop, ca ne sert à rien d'aller plus loin dans cet étage, il n'y a pas d'ancetre commun ici, ==> prendre celui du niveau au dessu, cad current_father
                 uncle = NULL;
            }
            else if ( strlen(current_key_part) <= strlen(uncle->key_part) ) // l'oncle est trop long pour etre mon father
            {
                uncle = uncle->brother;
            }
            else    // c'est un father potentiel.
            {// alors c'est p'tete possible avec cet oncle-ci
                // coupons nous a la taille de l'oncle
                 strncpy(key_part_tmp,current_key_part, strlen(uncle->key_part));
                 key_part_tmp[strlen(uncle->key_part)]='\0';
                 // on a coupé la key a la taille du uncle,
                 // on va vérifier si ils sont identiques.
                 // s'ils sont identiques, on a un ancetre, si pas, continuer
                 if (strcmp(key_part_tmp , uncle->key_part) == 0)
                 {
                     // il s'agit donc d'un ancetre !!
                     // ils ne sont pas de la meme taille sinon on ne serait pas dans ce if ci!
                     found_ancester = 1;
                     current_father = uncle;
                 }
                 else{
                         uncle = uncle->brother;
                 }
            }

        }
       // research for this depth ended
        if(!found_ancester)
        {
            // on a fait le tour de cet étage, et il n'y a pas d'ancêtre commun, on arrête la!
            return current_father;
        }
        else // on vient de trouver un ancetre (se trouve dans current_father)
            // si l'ancêtre  qu'on a trouvé n'a pas de fils, alors c'est le meilleur ancetre comme père
        if(current_father->child == NULL)
        {
            return current_father;
        }
        else
        {
            // on va continuer à descendre dans l'arbre, avec comme oncle potentiel
            //le premier fils de l'ancetre qu'on vient de trouver
            // couper les key_part
            uncle = current_father->child ;
            //strcpy(current_key_part, key);
            // garder la fin de la key (enlever la partie commune avec le père)
            int new_size =  strlen(current_key_part) - strlen(current_father->key_part) ;
            for (int i = 0 ; i < new_size ; i++)
            {
                current_key_part[i] = current_key_part[i+strlen(current_father->key_part)];
            }
            current_key_part[new_size] = '\0';
        }
    }
    return current_father;
}


// -----[ _find_left_brother ]---------------------------------------------
/**
 * we want to insert a key/value pair
 *  this function allows to find the left-brother of our node.
 *  if the answer is NULL, it means that our node does not have a left brother in this tree
 *   it means that it should be the first brother, or that we have to split the first brother.
 *  if the answer is a node, than it is the left brother, meaning that we have to deal with the right brother of this node
 *  that is to insert a new node before the right node, or to split the right brother.
 *
 *
 *
 *
 * Result: the left brother of the new node (meaning that we have to insert before, or to split the right brother. or NULL if it should be the first brother.
 */
static _trie_dico_item_t * _find_left_brother(_trie_dico_item_t * father,
                            _trie_dico_item_t * root,
                           trie_dico_key_t key)
{ 
    _trie_dico_item_t * current_left_brother = NULL;
    _trie_dico_item_t * temp_left_brother = NULL;
    trie_dico_key_t key_part;

    if(father == NULL) // c'est que c'est au tout premier étage (root)
    {
        temp_left_brother = root;
        key_part = (trie_dico_key_t) MALLOC( (strlen(key)+1) * sizeof(char) );
        strcpy(key_part,key);
    }
    else
    {
        temp_left_brother = father->child;
        int key_part_len = strlen(key) - strlen(father->key);
        key_part = (trie_dico_key_t) MALLOC( (key_part_len  +1) * sizeof(char) );
        for (int i = 0 ; i < key_part_len ; i++)
        {
                key_part[i] = key[i + strlen(father->key)];
        }
        key_part[key_part_len] = '\0';
    }

    while (temp_left_brother != NULL )
    {
        if(temp_left_brother->key_part[0] < key_part[0])
        {
            current_left_brother = temp_left_brother;
            temp_left_brother = temp_left_brother->brother;
        }
        else
            temp_left_brother = NULL;
    }

    return current_left_brother;

}



// -----[ _trie_dico_insert ]---------------------------------------------
/**
 * Insert a new (key, value) pair into the Patricia tree. This
 * function is only an helper function. The 'trie_dico_insert' function
 * should be used instead.
 *
 * Pre: (key length <= TRIE_DICO_KEY_SIZE)
 *
 * Result: 0 on success and -1 on error (duplicate key)
 */
static int _trie_dico_insert(_trie_dico_item_t ** item, trie_dico_key_t key,
                    void * data,
			gds_trie_dico_destroy_f destroy, int replace)
{

  trie_dico_key_t father_s_key;

  _trie_dico_item_t * new_item;
  _trie_dico_item_t * father = _find_father( *item , key );
  _trie_dico_item_t * left_brother = _find_left_brother(father, *item, key );
  _trie_dico_item_t ** ptrTonodetoanalyse = NULL;

  if(left_brother == NULL)
  {
    if(father == NULL)  // c'est celui pointé par root qu'il faut analyser
        ptrTonodetoanalyse = item;
    else  // c'est le noeud pointé par le fils de father qu'il faut analyser.
        ptrTonodetoanalyse = &(father->child);    
  }
  else // comme on a un left_brother, c'est le noeud pointé par son frère droit qu'il faut analyser
      ptrTonodetoanalyse = &(left_brother->brother);
 
  if(father != NULL)
       father_s_key = father->key;
  else  father_s_key = "";
  

  int end_of_key_len = strlen(key) - strlen(father_s_key) ;
  trie_dico_key_t end_of_key = (trie_dico_key_t) MALLOC( (end_of_key_len +1 ) * sizeof(char));
  for (int i = 0 ; i < end_of_key_len ; i++)
  {
     end_of_key[i] = key[ i + strlen(father_s_key)];
  }
  end_of_key[end_of_key_len] = '\0';

  // si ce qu'il faut pointer est null, alors on crée et on insère.
  // sinon,
  // s'il n'a rien en commun (pas de caractère en commun), alors on crée et on insère (on intercale)
  // sinon s'il y a x caractères en commun
  //            - et que end_of_key à insérer est épuisé
  //            - et que end_of_key n'est pas épuisé
  //       sinon si tout en commun : impossible, sinon aurait été détecté comme 'father' et pas comme brother.
  //

  if((*ptrTonodetoanalyse)==NULL)
  {   // créer et insérer là!
       new_item = _trie_dico_item_create_data( key,
                                       end_of_key,
                                       data);
       *ptrTonodetoanalyse = new_item;
  }
  else
  {
    // si rien en commun (donc première lettre différente) on doit intercaler
      if(end_of_key[0] != ((*ptrTonodetoanalyse)->key_part)[0])
      {
          new_item = _trie_dico_item_create_data( key,
                                       end_of_key,
				       data);
          new_item->brother = (*ptrTonodetoanalyse);
          *ptrTonodetoanalyse = new_item;

      }
      else // au moins le premier caractère est identique, continuer pour compter combien.
      {
          int i = 1;
          while(  i < strlen(end_of_key) &&
                  i < strlen((*ptrTonodetoanalyse)->key_part) &&
                  end_of_key[i] == ((*ptrTonodetoanalyse)->key_part)[i])
              i++;
         // i caractères communs, on doit spliter

          if( i == strlen(end_of_key) ) // end_of_key est épuisé, et l'autre est plus grand (ou égal)
          {
             //i caractères communs, cas 1 - on est entièrement contenu dans l'autre
              if( i == strlen((*ptrTonodetoanalyse)->key_part))
              {
                  // tout est identique, donc juste noter que le noeud en question contient une info (s'il n'en contient pas encore !
                  if((*ptrTonodetoanalyse)->is_final_data == 0)
                  {
                      (*ptrTonodetoanalyse)->is_final_data = 1;
                      (*ptrTonodetoanalyse)->data = data;
                  }
                  else
                  {
		    if (replace != TRIE_DICO_INSERT_OR_REPLACE)
		      return TRIE_DICO_ERROR_DUPLICATE;
		    if (destroy != NULL)
		      destroy(&(*ptrTonodetoanalyse)->data);
		    (*ptrTonodetoanalyse)->data= data;
                  }
              }
              else
              {   // le déjà présent est plus long, il faut le spliter !
                  // concernant le nouvel élément :
                  new_item = _trie_dico_item_create_data( key,
                                       end_of_key,
				       data);
                  new_item->brother = (*ptrTonodetoanalyse)->brother;
                  //concernant l'ancien élément qu'on va modifier:
                  //taille de la chaine :

                  int taille = strlen((*ptrTonodetoanalyse)->key_part) - i ;
                  trie_dico_key_t modified_key_part = (trie_dico_key_t) MALLOC( (1+taille) * sizeof(char));
                  for(;i < strlen((*ptrTonodetoanalyse)->key_part);i++)
                  {
                      modified_key_part[i-(strlen((*ptrTonodetoanalyse)->key_part) - taille)] = ((*ptrTonodetoanalyse)->key_part)[i];
                  }
                  modified_key_part[taille] = '\0';
                  (*ptrTonodetoanalyse)->key_part = modified_key_part;
                  (*ptrTonodetoanalyse)->brother = NULL;
                  new_item->child = *ptrTonodetoanalyse;
                  *ptrTonodetoanalyse = new_item;

              }
          }
          else
          {   //i caractères communs, cas 2 - on a des parties différentes

              // end_of_key n'est pas épuisé, et l'autre non plus (forcément !) spliter !

              // créer le noeud commun (résultat du splitage)
              // créer notre nouveau noeud
              // raccourcir l'ancien noeud qui a une partie commune avec Nous.
              // placer les pointeurs correctement, selon que l'ancien est plus petit ou plus grand que le nouveau.

              _trie_dico_item_t * new_item_commun = (_trie_dico_item_t *) MALLOC(sizeof(_trie_dico_item_t));
              int taille_part_commun = i;
              int taille_commun = strlen(father_s_key) + i;
              trie_dico_key_t commun_key_part = (trie_dico_key_t ) MALLOC( (1+ taille_part_commun ) * sizeof(char));
              trie_dico_key_t commun_key = (trie_dico_key_t ) MALLOC( (1+ taille_commun ) * sizeof(char));
              strncpy(commun_key_part, (*ptrTonodetoanalyse)->key_part ,taille_part_commun);
              commun_key_part[taille_part_commun]='\0';

              strcpy(commun_key, father_s_key);
              strcat(commun_key,commun_key_part);

              new_item_commun = _trie_dico_item_create_empty( commun_key,
                                       commun_key_part);
              new_item_commun->brother = (*ptrTonodetoanalyse)->brother;

              trie_dico_key_t  new_item_key_part = (trie_dico_key_t ) MALLOC( (1+ strlen(end_of_key) - i ) * sizeof(char));
              int j =0;
              while(i < strlen(end_of_key))
              {
                   new_item_key_part[j] = end_of_key[i];
                  i++;
                   j++;
              }
              new_item_key_part[j] = '\0';

              new_item = _trie_dico_item_create_data( key,
                                       new_item_key_part,
				       data);

              int nouvelle_taille_part_ancien_noeud = strlen((*ptrTonodetoanalyse)->key_part) - taille_part_commun;
              trie_dico_key_t  old_item_key_part = (trie_dico_key_t ) MALLOC( (1+nouvelle_taille_part_ancien_noeud) * sizeof(char));
              for(i=0 ; i < nouvelle_taille_part_ancien_noeud; i++)
              {
                  old_item_key_part[i] = ((*ptrTonodetoanalyse)->key_part)[taille_part_commun + i];
              }
              old_item_key_part[nouvelle_taille_part_ancien_noeud] = '\0';

              (*ptrTonodetoanalyse)->key_part = old_item_key_part;
              
               if( ( (*ptrTonodetoanalyse)->key_part)[0] < new_item->key_part[0])
               {
                 // placer l'ancien noeud avant le nouveau
                   new_item_commun->child = (*ptrTonodetoanalyse);
                   (*ptrTonodetoanalyse)->brother = new_item;
               }
               else
               {// placer l'ancien noeud après le nouveau
                   new_item_commun->child = new_item;
                   new_item->brother = (*ptrTonodetoanalyse);
                   (*ptrTonodetoanalyse)->brother = NULL;
               }
              (*ptrTonodetoanalyse) = new_item_commun;
          }


      }
  }
 //printf("\nAPRES : ");
 //_trie_dico_print_from(*item);

 return TRIE_DICO_SUCCESS;
}

// -----[ trie_dico_insert ]----------------------------------------------
/**
 * Insert one (key, value) pair into the dico tree.
 *
 * PRECONDITION:
 *  ???
 */
int trie_dico_insert(gds_trie_dico_t * trie_dico, trie_dico_key_t key,
                void * data,
		int replace)
{ 
    //printf("\nBEFORE : ");
    //trie_dico_print(trie_dico);

  if (trie_dico->root == NULL) {
    trie_dico->root= _trie_dico_item_create_data(key, 
                                key,
                                data);
    //printf("\nAFTER : ");
    //trie_dico_print(trie_dico);

    return TRIE_DICO_SUCCESS;
  }

  return _trie_dico_insert(&trie_dico->root, key,
		      data, trie_dico->destroy, replace);
}

// -----[ trie_dico_find_exact ]------------------------------------------
void * trie_dico_find_exact(gds_trie_dico_t * trie_dico,
			    trie_dico_key_t key)
{ 
  _trie_dico_item_t * father = _find_father( trie_dico->root , key ) ; 
  _trie_dico_item_t * left_brother = _find_left_brother(father, trie_dico->root, key );
  _trie_dico_item_t * theNode = NULL;

  if(left_brother == NULL)
  {
    if(father == NULL)// c'est celui pointé par root qu'il faut analyser
        theNode = trie_dico->root;
    else  // c'est le noeud pointé par le fils de father qu'il faut analyser.
       theNode = father->child;
  }
  else // comme on a un left_brother, c'est le noeud pointé par son frère droit qu'il faut analyser
      theNode = left_brother->brother;
  
  if(theNode != NULL)    // alors s'il est présent, c'est là que devrait se trouver le noeud.
  {
       if(strcmp(theNode->key,key) == 0 ) // il est là !!!
       {
           return theNode->data;
       }
  }else
      return NULL;
  
  return NULL;
}

// -----[ trie_dico_find_best ]-------------------------------------------
void * trie_dico_find_best(gds_trie_dico_t * trie_dico, trie_dico_key_t key)
{
  /*
  _trie_dico_item_t * tmp;
  void * data;
  int data_found= 0;
  trie_dico_key_t prefix;
  trie_dico_key_len_t prefix_len;
  trie_dico_key_t search_key= _trie_dico_mask_key(key, key_len);

  if (trie_dico->root == NULL)
    return NULL;
  tmp= trie_dico->root;
  data= NULL;
  while (tmp != NULL) {

    // requested key is smaller than current => no match found
    if (key_len < tmp->key_len)
      break;

    // requested key has same length
    if (key_len == tmp->key_len) {
      // (keys are equal) <=> match found
      if (search_key == tmp->key) {
	if (tmp->has_data) {
	  return tmp->data;
	} else {
	  return NULL;
	}
      } else
	break;
    }

    // requested key is longer => check if common parts match
    if (key_len > tmp->key_len) {
      _longest_common_prefix(tmp->key, tmp->key_len,
			     search_key, key_len, &prefix, &prefix_len);

      // Current key is too long => no match found
      if (prefix_len < tmp->key_len)
	break;

      if (tmp->has_data) {
	data= tmp->data;
	data_found= 1;
      }

      if (search_key & (1 << (TRIE_DICO_KEY_SIZE-prefix_len-1)))
	tmp= tmp->right;
      else
	tmp= tmp->left;
    }
  }
  if (data_found)
    return data;
  */
    return NULL;
}

// -----[ _trie_dico_remove_item ]----------------------------------------
static inline void _trie_dico_remove_item(_trie_dico_item_t ** item,
				     gds_trie_dico_destroy_f destroy)
{
  //?
}

// -----[ _trie_dico_remove ]---------------------------------------------
/*
  static int _trie_dico_remove(_trie_dico_item_t ** item,
const trie_dico_key_t key,
			gds_trie_dico_destroy_f destroy)
{
 
  return TRIE_DICO_ERROR_NO_MATCH;
}*/

// -----[ trie_dico_remove ]----------------------------------------------
/**
 * Remove the value associated with the given key. Remove any
 * unnecessary nodes in the tree.
 *
 * Pre: (key length < TRIE_DICO_KEY_SIZE)
 *
 * RETURNS:
 *   -1 if key does not exist
 *    0 if key has been removed.
 */
int trie_dico_remove(gds_trie_dico_t * trie_dico, trie_dico_key_t key) //, trie_dico_key_len_t key_len)
{
    /*
  if (trie_dico->root == NULL)
    return TRIE_DICO_ERROR_NO_MATCH;

  return _trie_dico_remove(&trie_dico->root, _trie_dico_mask_key(key, key_len),
		      key_len, trie_dico->destroy);
     */
    return -1000;
}

// -----[ _trie_dico_replace ]--------------------------------------------
/*static int _trie_dico_replace(_trie_dico_item_t * item, const trie_dico_key_t key,
			 trie_dico_key_len_t key_len, void * data,
			 gds_trie_dico_destroy_f destroy)
{//printf("@%s\n",__func__);
  trie_dico_key_t prefix;
  trie_dico_key_len_t prefix_len;

  // requested key is smaller than current => no match found
  if (key_len < item->key_len)
    return TRIE_DICO_ERROR_NO_MATCH;

  // requested key has same length
  if (key_len == item->key_len) {
    if ((key == item->key) && item->has_data) {
      if (destroy != NULL)
	destroy(&item->data);
      item->data= data;
      return TRIE_DICO_SUCCESS;
    } else
      return TRIE_DICO_ERROR_NO_MATCH;
  }

  // requested key is longer => check if common parts match
  if (key_len > item->key_len) {
    _longest_common_prefix(item->key, item->key_len,
			   key, key_len, &prefix, &prefix_len);

    // Current key is too long => no match found
    if (prefix_len < item->key_len)
      return TRIE_DICO_ERROR_NO_MATCH;

    if (key & (1 << (TRIE_DICO_KEY_SIZE-prefix_len-1))) {
      if (item->right != NULL)
	return _trie_dico_replace(item->right, key, key_len, data, destroy);
      else
	return TRIE_DICO_ERROR_NO_MATCH;
    } else {
      if (item->left != NULL)
	return _trie_dico_replace(item->left, key, key_len, data, destroy);
      else
	return TRIE_DICO_ERROR_NO_MATCH;
    }
  }

    return TRIE_DICO_ERROR_NO_MATCH;
}
*/


// -----[ trie_dico_replace ]---------------------------------------------
/**
 * Replace an existing key. An existing key is a node which has its
 * 'has_data' field equal to '1'.
 *
 * Returns:
 *   TRIE_DICO_SUCCESS
 *     if the key was found. In this case, the data 'field' is
 *     replaced with the new data value (can be NULL).
 *   TRIE_DICO_ERROR_NO_MATCH
 *     if no matching key was found.
 */
int trie_dico_replace(gds_trie_dico_t * trie_dico, trie_dico_key_t key,
                            //trie_dico_key_len_t key_len,
                            void * data)
{
    //printf("@%s\n",__func__);
    /*
  if (trie_dico->root == NULL)
    return TRIE_DICO_ERROR_NO_MATCH;

  return _trie_dico_replace(trie_dico->root, _trie_dico_mask_key(key, key_len),
		       key_len, data, trie_dico->destroy);
     */
    return -1000;
}

// -----[ _trie_dico_destroy ]---------------------------------------
static void _trie_dico_destroy(_trie_dico_item_t ** item,
			       gds_trie_dico_destroy_f destroy)
{
  if (*item != NULL) {
    // Destroy content of data item
    if ((*item)->is_final_data)
      if (destroy != NULL)
	destroy(&(*item)->data);
    
    // Recursive descent (brother, then child)
    if ((*item)->brother != NULL)
      _trie_dico_destroy(&(*item)->brother, destroy);
    if ((*item)->child != NULL)
      _trie_dico_destroy(&(*item)->child, destroy);
    
    FREE(*item);
  }
}

// -----[ trie_dico_destroy ]----------------------------------------
void trie_dico_destroy(gds_trie_dico_t ** trie_dico_ref)
{
  if (*trie_dico_ref != NULL) {
    _trie_dico_destroy(&(*trie_dico_ref)->root,
		       (*trie_dico_ref)->destroy);
    FREE(*trie_dico_ref);
    *trie_dico_ref= NULL;
  }
}

// -----[ _trie_dico_item_for_each ]---------------------------------
static int _trie_dico_item_for_each(_trie_dico_item_t * item,
			       gds_trie_dico_foreach_f foreach, void * ctx)
{
  int result;

  if (item->child != NULL) {
    result= _trie_dico_item_for_each(item->child, foreach, ctx);
    if (result != 0)
      return result;
  }

  if (item->is_final_data) {
    result = foreach(item->key, item->data, ctx);
    if (result != 0)
      return result;
  }

  if (item->brother != NULL) {
    result= _trie_dico_item_for_each(item->brother, foreach, ctx);
    if (result != 0)
      return result;
  }
  
  return 0;
}

// -----[ trie_dico_for_each ]---------------------------------------
int trie_dico_for_each(gds_trie_dico_t * trie_dico,
		       gds_trie_dico_foreach_f foreach, void * ctx)
{
  if (trie_dico->root != NULL)
    return _trie_dico_item_for_each(trie_dico->root, foreach, ctx);
  return 0;

}

// -----[ _trie_dico_num_nodes ]-------------------------------------
static int _trie_dico_num_nodes(_trie_dico_item_t * item, int with_data)
{
  int bro_child;

  if (item != NULL) {
    bro_child= (_trie_dico_num_nodes(item->child, with_data) +
		_trie_dico_num_nodes(item->brother, with_data));
    if (!with_data || item->is_final_data)
      return 1 + bro_child;
    else
      return bro_child;
  }
  return 0;
}

// -----[ trie_dico_num_nodes ]--------------------------------------
/**
 * Count the number of nodes in the trie_dico. The algorithm uses a
 * divide-and-conquer recursive approach.
 */
int trie_dico_num_nodes(gds_trie_dico_t * trie_dico, int with_data)
{
  return _trie_dico_num_nodes(trie_dico->root, with_data);
}

// -----[ trie_dico_to_graphviz ]------------------------------------
void trie_dico_to_graphviz(gds_stream_t * stream,
			   gds_trie_dico_t * trie_dico)
{
  gds_stack_t * stack= stack_create(32);
  _trie_dico_item_t * item;

  stream_printf(stream, "digraph trie_dico {\n");

  if (trie_dico->root != NULL)
    stack_push(stack, trie_dico->root);

  while (!stack_is_empty(stack)) {
    item= (_trie_dico_item_t *) stack_pop(stack);

    stream_printf(stream, "  \"%s/%s\" ", item->key, item->key_part);
    stream_printf(stream, "[label=\"%s/%s\\n", item->key, item->key_part);
    if (item->is_final_data)
      stream_printf(stream, "data=%p", item->data);
    stream_printf(stream, "\"]");
    stream_printf(stream, " ;\n");

    if (item->brother != NULL) {
      stack_push(stack, item->brother);
      stream_printf(stream, "  \"%s/%s\" -> \"%s/%s\" [style=\"dashed\"];\n",
		    item->key, item->key_part,
		    item->brother->key, item->brother->key_part);
    }
    if (item->child != NULL) {
      stack_push(stack, item->child);
      stream_printf(stream, "  \"%s/%s\" -> \"%s/%s\" ;\n",
		    item->key, item->key_part,
		    item->child->key, item->child->key_part);
    }
  }

  stream_printf(stream, "}\n");

  stack_destroy(&stack);
}

/////////////////////////////////////////////////////////////////////
//
// ENUMERATION
//
/////////////////////////////////////////////////////////////////////

// -----[ _trie_dico_get_array_for_each ]---------------------------------
static int _trie_dico_get_array_for_each(trie_dico_key_t key,
				    void * data, void * ctx)
{
  ptr_array_t * array= (ptr_array_t *) ctx;
  if (ptr_array_append(array, data) < 0)
    return -1;
  return 0;
}

// -----[ _trie_dico_get_array ]-------------------------------------------
static ptr_array_t * _trie_dico_get_array(gds_trie_dico_t * trie_dico)
{
    //printf("@%s\n",__func__);

  ptr_array_t * array= ptr_array_create_ref(0);
  if (trie_dico_for_each(trie_dico, _trie_dico_get_array_for_each, array)) {
    ptr_array_destroy(&array);

    array= NULL;
  }

  return array;

}

// ----- _enum_ctx_t -------------------------------------------
typedef struct {
  ptr_array_t * array;
  gds_enum_t  * enu;
} _enum_ctx_t;

// -----[ _trie_dico_get_enum_has_next ]----------------------------------
static int _trie_dico_get_enum_has_next(void * ctx)
{
  _enum_ctx_t * ectx= (_enum_ctx_t *) ctx;
  return enum_has_next(ectx->enu);
}

// -----[ _trie_dico_get_enum_get_next ]----------------------------------
static void * _trie_dico_get_enum_get_next(void * ctx)
{
  _enum_ctx_t * ectx= (_enum_ctx_t *) ctx;
  return enum_get_next(ectx->enu);
}

// -----[ _trie_dico_get_enum_destroy ]-----------------------------------
static void _trie_dico_get_enum_destroy(void * ctx)
{
  _enum_ctx_t * ectx= (_enum_ctx_t *) ctx;
  enum_destroy(&ectx->enu);
  ptr_array_destroy(&ectx->array);
  FREE(ectx);
}

// -----[ trie_dico_get_enum ]--------------------------------------------
gds_enum_t * trie_dico_get_enum(gds_trie_dico_t * trie_dico)
{
    //printf("@%s\n",__func__);
  _enum_ctx_t * ectx=
    (_enum_ctx_t *) MALLOC(sizeof(_enum_ctx_t));
  ectx->array= _trie_dico_get_array(trie_dico);
  ectx->enu= _array_get_enum((array_t *) ectx->array);

  return enum_create(ectx,
		     _trie_dico_get_enum_has_next,
		     _trie_dico_get_enum_get_next,
		     _trie_dico_get_enum_destroy);

    return NULL;
}

/////////////////////////////////////////////////////////////////////
//
// INITIALIZATION PART
//
/////////////////////////////////////////////////////////////////////


