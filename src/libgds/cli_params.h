// ==================================================================
// @(#)cli_params.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

#ifndef __GDS_CLI_PARAMS_H__
#define __GDS_CLI_PARAMS_H__

#include <libgds/array.h>

typedef int (*FCliCheckParam)(const char * value);
typedef char * (*FCliEnumParam)(const char * text, int state);

/////////////////////////////////////////////////////////////////////
//
// PARAMETERS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

typedef SPtrArray cli_params_t;
typedef cli_params_t SCliParams;
typedef enum {
  CLI_PARAM_TYPE_STD,
  CLI_PARAM_TYPE_VARARG,
} cli_param_type_t;

typedef struct {
  char             * name;
  cli_param_type_t   type;
  uint8_t            max_args;
  FCliCheckParam     fCheck;
  FCliEnumParam      fEnum;
  char             * info;
} cli_param_t;
typedef cli_param_t SCliParam;

#ifdef __cplusplus
extern"C" {
#endif

  // ----- cli_cmd_param_create -------------------------------------
  cli_param_t * cli_cmd_param_create(char * name,
				   FCliCheckParam fCheckParam);
  // ----- cli_cmd_param_destroy ------------------------------------
  void cli_cmd_param_destroy(cli_param_t ** param_ref);
  // ----- cli_params_create ----------------------------------------
  cli_params_t * cli_params_create();
  // ----- cli_params_destroy ---------------------------------------
  void cli_params_destroy(cli_params_t ** params_ref);
  // ----- cli_params_add -------------------------------------------
  int cli_params_add(cli_params_t * params, char * name,
		     FCliCheckParam fCheckParam);
  // ----- cli_params_add -------------------------------------------
  int cli_params_add2(cli_params_t * params, char * name,
		      FCliCheckParam fCheckParam,
		      FCliEnumParam fEnumParam);
  // ----- cli_params_add_vararg ------------------------------------
  int cli_params_add_vararg(cli_params_t * params, char * name,
			    uint8_t uMaxArgs,
			    FCliCheckParam fCheckParam);
  // ----- cli_params_num -------------------------------------------
  unsigned int cli_params_num(cli_params_t * params);
  
#ifdef __cplusplus
}
#endif


/////////////////////////////////////////////////////////////////////
//
// OPTIONS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

typedef SPtrArray cli_options_t;
typedef struct {
  char           * name;
  char           * value;
  uint8_t          present;
  FCliCheckParam   fCheck;
  char           * info;
} cli_option_t;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- cli_options_create ---------------------------------------
  cli_options_t * cli_options_create();
  // ----- cli_options_destroy --------------------------------------
  void cli_options_destroy(cli_options_t ** options_ref);
  // ----- cli_options_find -----------------------------------------
  cli_option_t * cli_options_find(cli_options_t * options,
				  const char * name);
  // ----- cli_options_add ------------------------------------------
  int cli_options_add(cli_options_t * options, char * name,
		      FCliCheckParam fCheck);
  // ----- cli_options_has_value ------------------------------------
  int cli_options_has_value(cli_options_t * options, char * name);
  // ----- cli_options_get_value ------------------------------------
  char * cli_options_get_value(cli_options_t * options, char * name);
  // ----- cli_options_set_value ------------------------------------
  int cli_options_set_value(cli_options_t * options, char * name,
			    char * value);
  // ----- cli_options_num ------------------------------------------
  unsigned int cli_options_num(cli_options_t * options);
  // ----- cli_options_init -------------------------------------------
  void cli_options_init(cli_options_t * options);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_PARAMS_H__ */
