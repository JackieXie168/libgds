// ==================================================================
// @(#)cli_params.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 16/01/2007
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgds/cli.h>
#include <libgds/cli_params.h>
#include <libgds/memory.h>
#include <libgds/str_util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/////////////////////////////////////////////////////////////////////
//
// PARAMETERS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- cli_param_create -------------------------------------------
/**
 * Create a parameter with default attributes.
 */
SCliParam * cli_param_create(char * pcName,
			     FCliCheckParam fCheck)
{
  SCliParam * pParam= (SCliParam *) MALLOC(sizeof(SCliParam));

  pParam->pcName= str_create(pcName);
  pParam->fCheck= fCheck;
  pParam->fEnum= NULL;
  pParam->tType= CLI_PARAM_TYPE_STD;
  return pParam;
}

// ----- cli_param_destroy ------------------------------------------
/**
 * Destroy a parameter.
 */
void cli_param_destroy(SCliParam ** ppParam)
{
  if (*ppParam != NULL) {
    str_destroy(&(*ppParam)->pcName);
    FREE(*ppParam);
    *ppParam= NULL;
  }
}

// -----[ _cli_params_item_compare ----------------------------------
/**
 * Private helper function used to compare 2 parameters in a list.
 */
static int _cli_params_item_compare(void * pItem1, void * pItem2,
				    unsigned int uEltSize)
{
  SCliParam * pParam1= *((SCliParam **) pItem1);
  SCliParam * pParam2= *((SCliParam **) pItem2);

  return strcmp(pParam1->pcName, pParam2->pcName);
}

// -----[ _cli_params_item_destroy ]---------------------------------
/**
 * Private helper function used to destroy each parameter in a list.
 */
static void _cli_params_item_destroy(void * pItem)
{
  cli_param_destroy((SCliParam **) pItem);
}

// ----- cli_params_create ------------------------------------------
/**
 * Create a list of parameters. A list of parameters is a sequence,
 * i.e. the ordering of parameters is the ordering of their insertion
 * in the list.
 */
SCliParams * cli_params_create()
{
  return (SCliParams *) ptr_array_create(0,
					 _cli_params_item_compare,
					 _cli_params_item_destroy);
}

// -----[ _cli_params_check_latest ]---------------------------------
/**
 * Check that the latest parameter on the list is not of type
 * CLI_PARAM_TYPE_VARARG. This function should be called before adding
 * any new parameter.
 */
static void _cli_params_check_latest(SCliParams * pParams)
{
  int iLength= ptr_array_length(pParams);
  if ((iLength > 0) &&
      (((SCliParam *) pParams->data[iLength-1])->tType == CLI_PARAM_TYPE_VARARG)) {
    fprintf(stderr, "Error: can not add a parameter after a vararg parameter.\n");
    abort();
  }
}

// ----- cli_params_destroy -----------------------------------------
/**
 * Destroy a list of parameters.
 */
void cli_params_destroy(SCliParams ** ppParams)
{
  ptr_array_destroy((SPtrArray **) ppParams);
}

// ----- cli_params_add ---------------------------------------------
/**
 * Add a parameter to the list of parameters.
 *
 * Parameter type: CLI_PARAM_TYPE_STD
 */
int cli_params_add(SCliParams * pParams, char * pcName,
		   FCliCheckParam fCheck)
{
  SCliParam * pParam= cli_param_create(pcName, fCheck);
  _cli_params_check_latest(pParams);
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}

// ----- cli_params_add2 --------------------------------------------
/**
 * Add a parameter to the list of parameters. An enumeration function
 * can be attached to this parameter. An enumeration function is
 * called by the command-line system in interactive mode.
 *
 * Parameter type: CLI_PARAM_TYPE_STD
 */
int cli_params_add2(SCliParams * pParams, char * pcName,
		    FCliCheckParam fCheck,
		    FCliEnumParam fEnum)
{
  SCliParam * pParam= cli_param_create(pcName, fCheck);
  _cli_params_check_latest(pParams);
  pParam->fEnum= fEnum;
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}

// ----- cli_params_add_vararg --------------------------------------
/**
 * Add a parameter that accepts a variable number of tokens. This type
 * of parameter must always be the last parameter of a command.
 *
 * Parameter type: CLI_PARAM_TYPE_VARARG
 */
int cli_params_add_vararg(SCliParams * pParams, char * pcName,
			  uint8_t uMaxArgs,
			  FCliCheckParam fCheck)
{
  SCliParam * pParam= cli_param_create(pcName, fCheck);
  _cli_params_check_latest(pParams);
  pParam->tType= CLI_PARAM_TYPE_VARARG;
  pParam->uMaxArgs= uMaxArgs;
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}

// ----- cli_params_num ---------------------------------------------
/**
 * Return the number of parameters.
 */
unsigned int cli_params_num(SCliParams * pParams)
{
  return ptr_array_length((SPtrArray *) pParams);
}

/////////////////////////////////////////////////////////////////////
//
// OPTIONS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// ----- cli_option_create ------------------------------------------
/**
 * Create an option.
 */
SCliOption * cli_option_create(char * pcName,
			       FCliCheckParam fCheck)
{
  SCliOption * pOption= (SCliOption *) MALLOC(sizeof(SCliOption));
  pOption->pcName= str_create(pcName);
  pOption->pcValue= NULL;
  pOption->uPresent= 0;
  pOption->fCheck= fCheck;
  return pOption;
}

// ----- cli_option_destroy -----------------------------------------
/**
 * Destroy an option.
 */
void cli_option_destroy(SCliOption ** ppOption)
{
  if (*ppOption != NULL) {
    str_destroy(&(*ppOption)->pcName);
    str_destroy(&(*ppOption)->pcValue);
    FREE(*ppOption);
    *ppOption= NULL;
  }
}

// -----[ _cli_options_item_compare ---------------------------------
/**
 * Private helper function used to compare 2 options in a list.
 */
static int _cli_options_item_compare(void * pItem1, void * pItem2,
				     unsigned int uEltSize)
{
  SCliOption * pOption1= *((SCliOption **) pItem1);
  SCliOption * pOption2= *((SCliOption **) pItem2);

  return strcmp(pOption1->pcName, pOption2->pcName);
}

// -----[ _cli_options_item_destroy ]--------------------------------
/**
 * Private helper function used to destroy each parameter in a list.
 */
static void _cli_options_item_destroy(void * pItem)
{
  cli_option_destroy((SCliOption **) pItem);
}

// ----- cli_options_create -----------------------------------------
/**
 *
 */
SCliOptions * cli_options_create()
{
  return (SCliOptions *) ptr_array_create(ARRAY_OPTION_SORTED,
					  _cli_options_item_compare,
					  _cli_options_item_destroy);
}

// ----- cli_options_destroy ----------------------------------------
/**
 *
 */
void cli_options_destroy(SCliOptions ** ppOptions)
{
  ptr_array_destroy((SPtrArray **) ppOptions);
}

// ----- cli_options_add --------------------------------------------
/**
 * Add an option to the list of options.
 */
int cli_options_add(SCliOptions * pOptions, char * pcName,
		    FCliCheckParam fCheck)
{
  SCliOption * pOption= cli_option_create(pcName, fCheck);
  return ptr_array_add((SPtrArray *) pOptions, &pOption);
}

// ----- cli_options_has_value --------------------------------------
/**
 * Test if the given option exists and has a value.
 *
 * Return value:
 *   1 option has a value
 *   0 option does not exist or has no value
 */
int cli_options_has_value(SCliOptions * pOptions, char * pcName)
{
  SCliOption sTmp;
  SCliOption * pOption= &sTmp;
  unsigned int uIndex;

  sTmp.pcName= pcName;
  if (ptr_array_sorted_find_index(pOptions, &pOption, &uIndex))
    return 0;

  pOption= (SCliOption*) pOptions->data[uIndex];

  return (pOption->uPresent);
}

// ----- cli_options_get_value --------------------------------------
/**
 * Return the value of an option.
 *
 * Return value:
 *   NULL if the option does not exist or has no value.
 */
char * cli_options_get_value(SCliOptions * pOptions, char * pcName)
{
  SCliOption sTmp;
  SCliOption * pOption= &sTmp;
  unsigned int uIndex;

  sTmp.pcName= pcName;
  if (ptr_array_sorted_find_index(pOptions, &pOption, &uIndex))
    return NULL;

  pOption= (SCliOption*) pOptions->data[uIndex];
  return pOption->pcValue;
}

// ----- cli_options_set_value --------------------------------------
/**
 * Set the value of an option.
 *
 * Return value:
 *   CLI_SUCCESS               value was set successfully.
 *   CLI_ERROR_UNKNOWN_OPTION  option does not exist
 *   CLI_ERROR_BAD_OPTION      option value is not valid
 */
int cli_options_set_value(SCliOptions * pOptions, char * pcName,
			  char * pcValue)
{
  SCliOption sTmp;
  SCliOption * pOption= &sTmp;
  unsigned int uIndex;
  int iResult;

  sTmp.pcName= pcName;
  if (ptr_array_sorted_find_index(pOptions, &pOption, &uIndex))
    return CLI_ERROR_UNKNOWN_OPTION;

  pOption= (SCliOption*) pOptions->data[uIndex];
    
  // Check value if required
  if (pOption->fCheck != NULL) {
    iResult= pOption->fCheck(pcValue);
    if (iResult)
      return CLI_ERROR_BAD_OPTION;
  }
    
  if (pcValue != NULL)
    pOption->pcValue= str_create(pcValue);
  else
    pOption->pcValue= NULL;
  pOption->uPresent= 1;

  return CLI_SUCCESS;
}

// ----- cli_options_num --------------------------------------------
unsigned int cli_options_num(SCliOptions * pOptions)
{
  return ptr_array_length((SArray *) pOptions);
}

// ----- cli_options_init -------------------------------------------
/**
 * Initialize the options' values.
 */
void cli_options_init(SCliOptions * pOptions)
{
  unsigned int uIndex;
  SCliOption * pOption;

  for (uIndex= 0; uIndex < ptr_array_length((SArray *) pOptions); uIndex++) {
    pOption= (SCliOption *) pOptions->data[uIndex];
    pOption->uPresent= 0;
    str_destroy(&pOption->pcValue);
  }
}
