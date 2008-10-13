// ==================================================================
// @(#)cli_params.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
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
cli_param_t * cli_param_create(char * name,
			     FCliCheckParam fCheck)
{
  cli_param_t * param= (cli_param_t *) MALLOC(sizeof(cli_param_t));

  param->name= str_create(name);
  param->fCheck= fCheck;
  param->fEnum= NULL;
  param->type= CLI_PARAM_TYPE_STD;
  param->info= NULL;
  return param;
}

// ----- cli_param_destroy ------------------------------------------
/**
 * Destroy a parameter.
 */
void cli_param_destroy(cli_param_t ** param_ref)
{
  if (*param_ref != NULL) {
    str_destroy(&(*param_ref)->name);
    str_destroy(&(*param_ref)->info);
    FREE(*param_ref);
    *param_ref= NULL;
  }
}

// -----[ _cli_params_item_compare ----------------------------------
/**
 * Private helper function used to compare 2 parameters in a list.
 */
static int _cli_params_item_compare(const void * item1,
				    const void * item2,
				    unsigned int elt_size)
{
  cli_param_t * param1= *((cli_param_t **) item1);
  cli_param_t * param2= *((cli_param_t **) item2);

  return strcmp(param1->name, param2->name);
}

// -----[ _cli_params_item_destroy ]---------------------------------
/**
 * Private helper function used to destroy each parameter in a list.
 */
static void _cli_params_item_destroy(void * item, const void * ctx)
{
  cli_param_destroy((cli_param_t **) item);
}

// ----- cli_params_create ------------------------------------------
/**
 * Create a list of parameters. A list of parameters is a sequence,
 * i.e. the ordering of parameters is the ordering of their insertion
 * in the list.
 */
cli_params_t * cli_params_create()
{
  return (cli_params_t *) ptr_array_create(0,
					   _cli_params_item_compare,
					   _cli_params_item_destroy,
					   NULL);
}

// -----[ _cli_params_check_latest ]---------------------------------
/**
 * Check that the latest parameter on the list is not of type
 * CLI_PARAM_TYPE_VARARG. This function should be called before adding
 * any new parameter.
 */
static void _cli_params_check_latest(cli_params_t * params)
{
  int iLength= ptr_array_length(params);
  if ((iLength > 0) &&
      (((cli_param_t *) params->data[iLength-1])->type == CLI_PARAM_TYPE_VARARG)) {
    fprintf(stderr, "Error: can not add a parameter after a vararg parameter.\n");
    abort();
  }
}

// ----- cli_params_destroy -----------------------------------------
/**
 * Destroy a list of parameters.
 */
void cli_params_destroy(cli_params_t ** params_ref)
{
  ptr_array_destroy((SPtrArray **) params_ref);
}

// ----- cli_params_add ---------------------------------------------
/**
 * Add a parameter to the list of parameters.
 *
 * Parameter type: CLI_PARAM_TYPE_STD
 */
int cli_params_add(cli_params_t * params, char * name,
		   FCliCheckParam fCheck)
{
  cli_param_t * param= cli_param_create(name, fCheck);
  _cli_params_check_latest(params);
  return ptr_array_add((SPtrArray *) params, &param);
}

// ----- cli_params_add2 --------------------------------------------
/**
 * Add a parameter to the list of parameters. An enumeration function
 * can be attached to this parameter. An enumeration function is
 * called by the command-line system in interactive mode.
 *
 * Parameter type: CLI_PARAM_TYPE_STD
 */
int cli_params_add2(cli_params_t * params, char * name,
		    FCliCheckParam fCheck,
		    FCliEnumParam fEnum)
{
  cli_param_t * param= cli_param_create(name, fCheck);
  _cli_params_check_latest(params);
  param->fEnum= fEnum;
  return ptr_array_add((SPtrArray *) params, &param);
}

// ----- cli_params_add_vararg --------------------------------------
/**
 * Add a parameter that accepts a variable number of tokens. This type
 * of parameter must always be the last parameter of a command.
 *
 * Parameter type: CLI_PARAM_TYPE_VARARG
 */
int cli_params_add_vararg(cli_params_t * params, char * name,
			  uint8_t max_args,
			  FCliCheckParam fCheck)
{
  cli_param_t * param= cli_param_create(name, fCheck);
  _cli_params_check_latest(params);
  param->type= CLI_PARAM_TYPE_VARARG;
  param->max_args= max_args;
  return ptr_array_add((SPtrArray *) params, &param);
}

// ----- cli_params_num ---------------------------------------------
/**
 * Return the number of parameters.
 */
unsigned int cli_params_num(cli_params_t * params)
{
  return ptr_array_length((SPtrArray *) params);
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
cli_option_t * cli_option_create(char * name,
			       FCliCheckParam fCheck)
{
  cli_option_t * option= (cli_option_t *) MALLOC(sizeof(cli_option_t));
  option->name= str_create(name);
  option->value= NULL;
  option->present= 0;
  option->fCheck= fCheck;
  option->info= NULL;
  return option;
}

// ----- cli_option_destroy -----------------------------------------
/**
 * Destroy an option.
 */
void cli_option_destroy(cli_option_t ** option_ref)
{
  if (*option_ref != NULL) {
    str_destroy(&(*option_ref)->name);
    str_destroy(&(*option_ref)->value);
    str_destroy(&(*option_ref)->info);
    FREE(*option_ref);
    *option_ref= NULL;
  }
}

// -----[ _cli_options_item_compare ---------------------------------
/**
 * Private helper function used to compare 2 options in a list.
 */
static int _cli_options_item_compare(const void * item1,
				     const void * item2,
				     unsigned int elt_size)
{
  cli_option_t * option1= *((cli_option_t **) item1);
  cli_option_t * option2= *((cli_option_t **) item2);

  return strcmp(option1->name, option2->name);
}

// -----[ _cli_options_item_destroy ]--------------------------------
/**
 * Private helper function used to destroy each parameter in a list.
 */
static void _cli_options_item_destroy(void * item, const void * ctx)
{
  cli_option_destroy((cli_option_t **) item);
}

// ----- cli_options_create -----------------------------------------
/**
 *
 */
cli_options_t * cli_options_create()
{
  return (cli_options_t *) ptr_array_create(ARRAY_OPTION_SORTED,
					    _cli_options_item_compare,
					    _cli_options_item_destroy,
					    NULL);
}

// ----- cli_options_destroy ----------------------------------------
/**
 *
 */
void cli_options_destroy(cli_options_t ** options_ref)
{
  ptr_array_destroy((SPtrArray **) options_ref);
}

// ----- cli_options_find -------------------------------------------
/**
 *
 */
cli_option_t * cli_options_find(cli_options_t * options, const char * name)
{
  cli_option_t sTmp;
  cli_option_t * option= &sTmp;
  unsigned int index;

  sTmp.name= (char *) name;
  if (ptr_array_sorted_find_index(options, &option, &index))
    return NULL;

  return (cli_option_t*) options->data[index];
}

// ----- cli_options_add --------------------------------------------
/**
 * Add an option to the list of options.
 */
int cli_options_add(cli_options_t * options, char * name,
		    FCliCheckParam fCheck)
{
  cli_option_t * option= cli_option_create(name, fCheck);
  return ptr_array_add((SPtrArray *) options, &option);
}

// ----- cli_options_has_value --------------------------------------
/**
 * Test if the given option exists and has a value.
 *
 * Return value:
 *   1 option has a value
 *   0 option does not exist or has no value
 */
int cli_options_has_value(cli_options_t * options, char * name)
{
  cli_option_t sTmp;
  cli_option_t * option= &sTmp;
  unsigned int index;

  sTmp.name= name;
  if (ptr_array_sorted_find_index(options, &option, &index))
    return 0;

  option= (cli_option_t*) options->data[index];

  return (option->present);
}

// ----- cli_options_get_value --------------------------------------
/**
 * Return the value of an option.
 *
 * Return value:
 *   NULL if the option does not exist or has no value.
 */
char * cli_options_get_value(cli_options_t * options, char * name)
{
  cli_option_t sTmp;
  cli_option_t * option= &sTmp;
  unsigned int index;

  sTmp.name= name;
  if (ptr_array_sorted_find_index(options, &option, &index))
    return NULL;

  option= (cli_option_t*) options->data[index];
  return option->value;
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
int cli_options_set_value(cli_options_t * options, char * name,
			  char * value)
{
  cli_option_t sTmp;
  cli_option_t * option= &sTmp;
  unsigned int index;
  int result;

  sTmp.name= name;
  if (ptr_array_sorted_find_index(options, &option, &index))
    return CLI_ERROR_UNKNOWN_OPTION;

  option= (cli_option_t*) options->data[index];
    
  // Check value if required
  if (option->fCheck != NULL) {
    result= option->fCheck(value);
    if (result)
      return CLI_ERROR_BAD_OPTION;
  }
    
  if (value != NULL)
    option->value= str_create(value);
  else
    option->value= NULL;
  option->present= 1;

  return CLI_SUCCESS;
}

// ----- cli_options_num --------------------------------------------
unsigned int cli_options_num(cli_options_t * options)
{
  return ptr_array_length(options);
}

// ----- cli_options_init -------------------------------------------
/**
 * Initialize the options' values.
 */
void cli_options_init(cli_options_t * options)
{
  unsigned int index;
  cli_option_t * option;

  for (index= 0; index < ptr_array_length(options); index++) {
    option= (cli_option_t *) options->data[index];
    option->present= 0;
    str_destroy(&option->value);
  }
}
