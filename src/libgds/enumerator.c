// ==================================================================
// @(#)enumerator.c
//
// List enumerator object.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 10/08/2005
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <libgds/memory.h>
#include <libgds/enumerator.h>
#include <libgds/types.h>

// ----- enum_create ------------------------------------------------
GDS_EXP_DECL 
gds_enum_t * enum_create(void * ctx,
			 gds_enum_has_next_f has_next,
			 gds_enum_get_next_f get_next,
			 gds_enum_destroy_f destroy)
{
  gds_enum_t * enu= (gds_enum_t *) MALLOC(sizeof(gds_enum_t));
  enu->ctx= ctx;
  enu->ops.has_next= has_next;
  enu->ops.get_next= get_next;
  enu->ops.destroy= destroy;
  return enu;
}

// ----- enum_destroy -----------------------------------------------
GDS_EXP_DECL 
void enum_destroy(gds_enum_t ** enum_ref)
{
  if (*enum_ref != NULL) {
    if ((*enum_ref)->ops.destroy != NULL)
      (*enum_ref)->ops.destroy((*enum_ref)->ctx);
    FREE(*enum_ref);
    *enum_ref= NULL;
  }
}
