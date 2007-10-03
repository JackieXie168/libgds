// ==================================================================
// @(#)cli_params.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 25/06/2007
// ==================================================================

#ifndef __GDS_CLI_PARAMS_H__
#define __GDS_CLI_PARAMS_H__

#include <libgds/array.h>

typedef int (*FCliCheckParam)(const char * pcValue);
typedef char * (*FCliEnumParam)(const char * pcText, int state);

/////////////////////////////////////////////////////////////////////
//
// PARAMETERS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

typedef SPtrArray SCliParams;
typedef enum {
  CLI_PARAM_TYPE_STD,
  CLI_PARAM_TYPE_VARARG,
} cli_param_type_t;

typedef struct {
  char * pcName;
  cli_param_type_t tType;
  uint8_t uMaxArgs;
  FCliCheckParam fCheck;
  FCliEnumParam fEnum;
  char * pcInfo;
} SCliParam;

#ifdef __cplusplus
extern"C" {
#endif

  // ----- cli_cmd_param_create -------------------------------------
  SCliParam * cli_cmd_param_create(char * pcName,
				   FCliCheckParam fCheckParam);
  // ----- cli_cmd_param_destroy ------------------------------------
  void cli_cmd_param_destroy(SCliParam ** ppParam);
  // ----- cli_params_create ----------------------------------------
  SCliParams * cli_params_create();
  // ----- cli_params_destroy ---------------------------------------
  void cli_params_destroy(SCliParams ** ppParams);
  // ----- cli_params_add -------------------------------------------
  int cli_params_add(SCliParams * pParams, char * pcName,
		     FCliCheckParam fCheckParam);
  // ----- cli_params_add -------------------------------------------
  int cli_params_add2(SCliParams * pParams, char * pcName,
		      FCliCheckParam fCheckParam,
		      FCliEnumParam fEnumParam);
  // ----- cli_params_add_vararg ------------------------------------
  int cli_params_add_vararg(SCliParams * pParams, char * pcName,
			    uint8_t uMaxArgs,
			    FCliCheckParam fCheckParam);
  // ----- cli_params_num -------------------------------------------
  unsigned int cli_params_num(SCliParams * pParams);
  
#ifdef __cplusplus
}
#endif


/////////////////////////////////////////////////////////////////////
//
// OPTIONS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

typedef SPtrArray SCliOptions;
typedef struct {
  char * pcName;
  char * pcValue;
  uint8_t uPresent;
  FCliCheckParam fCheck;
  char * pcInfo;
} SCliOption;

#ifdef __cplusplus
extern "C" {
#endif

  // ----- cli_options_create ---------------------------------------
  SCliOptions * cli_options_create();
  // ----- cli_options_destroy --------------------------------------
  void cli_options_destroy(SCliOptions ** ppOptions);
  // ----- cli_options_find -----------------------------------------
  SCliOption * cli_options_find(SCliOptions * pOptions, const char * pcName);
  // ----- cli_options_add ------------------------------------------
  int cli_options_add(SCliOptions * pOptions, char * pcName,
		      FCliCheckParam fCheck);
  // ----- cli_options_has_value ------------------------------------
  int cli_options_has_value(SCliOptions * pOptions, char * pcName);
  // ----- cli_options_get_value ------------------------------------
  char * cli_options_get_value(SCliOptions * pOptions, char * pcName);
  // ----- cli_options_set_value ------------------------------------
  int cli_options_set_value(SCliOptions * pOptions, char * pcName,
			    char * pcValue);
  // ----- cli_options_num ------------------------------------------
  unsigned int cli_options_num(SCliOptions * pOptions);
  // ----- cli_options_init -------------------------------------------
  void cli_options_init(SCliOptions * pOptions);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_PARAMS_H__ */
