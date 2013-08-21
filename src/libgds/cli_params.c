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

#include <assert.h>

#include <libgds/cli.h>
#include <libgds/cli_params.h>
#include <libgds/gds.h>
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

// -----[ _cli_arg_create ]----------------------------------------
/**
 * Create an argument with default attributes.
 */
static inline cli_arg_t * _cli_arg_create(cli_arg_type_t type,
					  const char * name,
					  cli_arg_check_f check,
					  cli_arg_enum_f enumerate)
{
  if (strchr(name, '<') || strchr(name, '>') ||
      strchr(name, '='))
    gds_fatal("The CLI does not allow arg names with [<>=] (\"%s\")\n", name);
  if (!strncmp(name, "-", 1))
    gds_fatal("The CLI does not allow arg names starting with '-' (\"%s\")\n",
	      name);

  cli_arg_t * arg= (cli_arg_t *) MALLOC(sizeof(cli_arg_t));

  arg->type= type;
  arg->name= str_create(name);
  arg->parent= NULL;
  arg->need_value= 0;
  arg->present= 0;
  arg->max_args= 1;
  
  arg->ops.check= check;
  arg->ops.enumerate= enumerate;

  switch (arg->type) {
  case CLI_ARG_TYPE_STD:
  case CLI_ARG_TYPE_OPT:
    arg->value= NULL;
    break;
  case CLI_ARG_TYPE_VAR:
    arg->values= NULL;
    break;
  default:
    abort();
  }
  return arg;
}

// -----[ _cli_arg_destroy ]-----------------------------------------
/**
 * Destroy an argument.
 */
static inline void _cli_arg_destroy(cli_arg_t ** arg_ref)
{
  cli_arg_t * arg= *arg_ref;

  if (arg != NULL) {
    str_destroy(&arg->name);
    switch (arg->type) {
    case CLI_ARG_TYPE_STD:
    case CLI_ARG_TYPE_OPT:
      str_destroy(&arg->value);
      break;
    case CLI_ARG_TYPE_VAR:
      tokens_destroy(&arg->values);
      break;
    default:
      abort();
    }
    FREE(arg);
    *arg_ref= NULL;
  }
}

// -----[ _cli_args_item_cmp ]---------------------------------------
/**
 * Private helper function used to compare 2 parameters in a list.
 */
static int _cli_args_item_cmp(const void * item1,
			      const void * item2,
			      unsigned int elt_size)
{
  cli_arg_t * arg1= *((cli_arg_t **) item1);
  cli_arg_t * arg2= *((cli_arg_t **) item2);

  return strcmp(arg1->name, arg2->name);
}

// -----[ _cli_args_item_destroy ]-----------------------------------
/**
 * Private helper function used to destroy each parameter in a list.
 */
static void _cli_args_item_destroy(void * item, const void * ctx)
{
  _cli_arg_destroy((cli_arg_t **) item);
}

// -----[ cli_args_create ]------------------------------------------
/**
 * Create a list of parameters. A list of parameters is a sequence,
 * i.e. the ordering of parameters is the ordering of their insertion
 * in the list.
 */
cli_args_t * cli_args_create()
{
  return (cli_args_t *) ptr_array_create(0,
					 _cli_args_item_cmp,
					 _cli_args_item_destroy, NULL);
}

// -----[ cli_args_create_ref ]--------------------------------------
cli_args_t * cli_args_create_ref()
{
  return (cli_args_t *) ptr_array_create(0,
					 _cli_args_item_cmp,
					 NULL, NULL);
}

// -----[ cli_args_destroy ]-----------------------------------------
/**
 * Destroy a list of parameters.
 */
void cli_args_destroy(cli_args_t ** args_ref)
{
  ptr_array_destroy((ptr_array_t **) args_ref);
}

// -----[ cli_arg_dump ]---------------------------------------------
void cli_arg_dump(gds_stream_t * stream, cli_arg_t * arg)
{
  switch (arg->type) {
  case CLI_ARG_TYPE_STD:
    stream_printf(stream, "<%s", arg->name);
    if (arg->value != NULL)
      stream_printf(stream, "=%s", arg->value);
    stream_printf(stream, ">");
    break;
    
  case CLI_ARG_TYPE_VAR:
    stream_printf(stream, "<%s>", arg->name);
    if (arg->max_args > 0)
      stream_printf(stream, "?(0-%d)", arg->max_args);
    else
      stream_printf(stream, "?(0-any)");
    break;
    
  case CLI_ARG_TYPE_OPT:
    stream_printf(stream, "[--%s", arg->name);
    if (arg->need_value) {
      stream_printf(stream, "=");
      if (arg->present)
	stream_printf(stream, "%s", arg->value);
    } else {
      if (arg->present)
	stream_printf(stream, "(set)", arg->value);
    }
    stream_printf(stream, "]");
    break;
    
  default:
    abort();
  }
}

// -----[ cli_args_dump ]--------------------------------------------
void cli_args_dump(gds_stream_t * stream, cli_args_t * args)
{
  unsigned int index;
  cli_arg_t * arg;

  for (index= 0; index < cli_args_num(args); index++) {
    arg= (cli_arg_t *) args->data[index];
    stream_printf(stream, " ");
    cli_arg_dump(stream, arg);
  }
}

// -----[ _cli_args_check_latest ]---------------------------------
/**
 * Check that the latest argument on the list is not of type
 * CLI_ARG_TYPE_VAR. This function should be called before adding
 * any new argument.
 */
static inline int _cli_args_check_latest(cli_args_t * args)
{
  int len= cli_args_num(args);
  if ((len > 0) &&
      (((cli_arg_t *) args->data[len-1])->type == CLI_ARG_TYPE_VAR))
    return -1;
  return 0;
}

// -----[ cli_args_add ]-------------------------------------------
/**
 * Add an argument to the set of parameters.
 */
cli_arg_t * cli_args_add(cli_args_t * args, cli_arg_t * arg)
{
  if (_cli_args_check_latest(args) < 0)
    gds_fatal("The CLI does not allow arg \"%s\" to follow a vararg.\n",
	      arg->name);
  if (ptr_array_add((ptr_array_t *) args, &arg) < 0)
    gds_fatal("The CLI could not add argument \"%s\"\n", arg->name);
  return arg;
}

// -----[ cli_arg_check ]------------------------------------------
/**
 * Call the argument 'check' function if provided. Otherwise,
 * succeed.
 *
 * Return value:
 *   0  in case of success
 *   <0 in case of error
 */
int cli_arg_check(const cli_arg_t * arg, const char * value)
{
  if (arg->ops.check != NULL)
    return arg->ops.check(/*arg, */value);
  return 0;
}

// -----[ cli_args_bounds ]------------------------------------------
/**
 * Return the minimum and maximum number of arguments
 * required/accepted by this set og arguments.
 */
void cli_args_bounds(cli_args_t * args, unsigned int * min,
		     unsigned int * max)
{
  unsigned int num_args;
  cli_arg_t * arg;

  if (args == NULL) {
    *min= 0;
    *max= 0;
  } else {
    num_args= cli_args_num(args);
    arg= cli_args_at(args, num_args-1);
    if (arg->type == CLI_ARG_TYPE_VAR) {
      *min= num_args-1;
      *max= UINT_MAX;
    } else {
      *min= num_args;
      *max= num_args;
    }
  }
}


/////////////////////////////////////////////////////////////////////
//
// OPTIONS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// -----[ _cli_arg_clear ]-------------------------------------------
static inline void _cli_arg_clear(cli_arg_t * arg)
{
  arg->present= 0;
  switch (arg->type) {
  case CLI_ARG_TYPE_STD:
  case CLI_ARG_TYPE_OPT:
    str_destroy(&arg->value);
    break;
  case CLI_ARG_TYPE_VAR:
    tokens_destroy(&arg->values);
    break;
  default:
    abort();
  }
}

// -----[ cli_opts_create ]------------------------------------------
cli_opts_t * cli_opts_create()
{
  return (cli_opts_t *) ptr_array_create(ARRAY_OPTION_SORTED,
					 _cli_args_item_cmp,
					 _cli_args_item_destroy,
					 NULL);
}

// -----[ cli_opts_destroy ]-----------------------------------------
void cli_opts_destroy(cli_opts_t ** opts_ref)
{
  ptr_array_destroy((ptr_array_t **) opts_ref);
}

// -----[ cli_opts_add ]---------------------------------------------
cli_arg_t * cli_opts_add(cli_opts_t * opts, cli_arg_t * opt)
{
  if (ptr_array_add(opts, &opt) < 0)
    gds_fatal("The CLI could not add option \"%s\"\n", opt->name);
  return opt;
}

// -----[ cli_opts_find ]--------------------------------------------
/**
 *
 */
cli_arg_t * cli_opts_find(cli_opts_t * opts, const char * name)
{
  cli_arg_t opt= { .name= (char *) name };
  cli_arg_t * ptr_opt= &opt;
  unsigned int index;

  if (opts == NULL)
    return NULL;

  if (ptr_array_sorted_find_index(opts, &ptr_opt, &index))
    return NULL;

  return (cli_arg_t*) opts->data[index];
}

// -----[ cli_opts_has_value ]------------------------------------
/**
 * Test if the given option exists and has a value.
 *
 * Return value:
 *   1 option has a value
 *   0 option does not exist or has no value
 */
int cli_opts_has_value(cli_opts_t * opts, const char * name)
{
  cli_arg_t * opt= cli_opts_find(opts, name);
  if (opt == NULL)
    return 0;
  return (opt->present);
}

// -----[ cli_opts_get_value ]------------------------------------
/**
 * Return the value of an option.
 *
 * Return value:
 *   NULL if the option does not exist or has no value.
 */
char * cli_opts_get_value(cli_opts_t * opts, const char * name)
{
  cli_arg_t * opt= cli_opts_find(opts, name);
  if (opt == NULL)
    return NULL;
  return opt->value;
}

// -----[ cli_arg_set_value ]----------------------------------------
int cli_arg_set_value(cli_arg_t * arg, const char * value)
{
  int result;

  // Check value (if required)
  if (arg->ops.check != NULL) {
    result= arg->ops.check(/*arg, */value);
    if (result)
      return CLI_ERROR_BAD_ARG_VALUE;
  }

  switch (arg->type) {
  case CLI_ARG_TYPE_STD:
    // Clear previous value
    if (arg->value != NULL)
      str_destroy(&arg->value);
    // Set value
    if (value != NULL)
      arg->value= str_create(value);
    else
      arg->value= NULL;
    break;

  case CLI_ARG_TYPE_OPT:
    if ((value == NULL) && (arg->need_value))
      return CLI_ERROR_MISSING_OPT_VALUE;
    if ((value != NULL) && (!arg->need_value))
      return CLI_ERROR_OPT_NO_VALUE;
    arg->present= 1;
    // Clear previous value
    if (arg->value != NULL)
      str_destroy(&arg->value);
    // Set value
    if (value != NULL)
      arg->value= str_create(value);
    else
      arg->value= NULL;
    break;

  case CLI_ARG_TYPE_VAR:
    if (arg->values == NULL)
      arg->values= tokens_create();
    if ((arg->max_args > 0) && (tokens_get_num(arg->values) >= arg->max_args))
      return CLI_ERROR_TOO_MANY_ARGS;
    tokens_add_copy(arg->values, value);
    break;
  default:
    abort();
  }

  arg->present= 1;
  return CLI_SUCCESS;
}

// -----[ cli_arg_get_num_values ]---------------------------------
unsigned int cli_arg_get_num_values(cli_arg_t * arg)
{
  switch (arg->type) {
  case CLI_ARG_TYPE_STD:
    return 1;
  case CLI_ARG_TYPE_OPT:
    return 1;
  case CLI_ARG_TYPE_VAR:
    if (arg->values == NULL)
      return 0;
    return tokens_get_num(arg->values);
  default:
    abort();
  }
}

// -----[ cli_arg_get_value ]--------------------------------------
const char * cli_arg_get_value(cli_arg_t * arg, unsigned int index)
{
  switch (arg->type) {
  case CLI_ARG_TYPE_STD:
  case CLI_ARG_TYPE_OPT:
    assert(index == 0);
    return arg->value;
  case CLI_ARG_TYPE_VAR:
    assert(arg->values != NULL);
    assert(index < tokens_get_num(arg->values));
    return tokens_get_string_at(arg->values, index);
  default:
    abort();
  }
}


// -----[ cli_opts_set_value ]------------------------------------
/**
 * Set the value of an option.
 *
 * Return value:
 *   CLI_SUCCESS               value was set successfully.
 *   CLI_ERROR_UNKNOWN_OPT     option does not exist
 *   CLI_ERROR_BAD_OPT         option value is not valid
 */
int cli_opts_set_value(cli_opts_t * opts, const char * name,
		       const char * value)
{
  cli_arg_t * opt= cli_opts_find(opts, name);
  if (opt == NULL)
    return CLI_ERROR_UNKNOWN_OPT;

  return cli_arg_set_value(opt, value);
}

// -----[ cli_args_clear ]-------------------------------------------
/**
 * Initialize the options' values.
 */
void cli_args_clear(cli_args_t * args)
{
  unsigned int index;
  cli_arg_t * arg;

  for (index= 0; index < cli_args_num(args); index++) {
    arg= cli_args_at(args, index);
    _cli_arg_clear(arg);
  }
}


/////////////////////////////////////////////////////////////////////
//
// CLI ARG/VARARG/OPT CREATION
//
/////////////////////////////////////////////////////////////////////

// -----[ cli_arg ]--------------------------------------------------
cli_arg_t * cli_arg(const char * name, cli_arg_check_f check)
{
  return _cli_arg_create(CLI_ARG_TYPE_STD, name, check, NULL);
}

// -----[ cli_arg2 ]-------------------------------------------------
cli_arg_t * cli_arg2(const char * name, cli_arg_check_f check,
		     cli_arg_enum_f enumerate)
{
  return _cli_arg_create(CLI_ARG_TYPE_STD, name, check, enumerate);
}

// -----[ cli_vararg ]-----------------------------------------------
cli_arg_t * cli_vararg(const char * name, uint8_t max_args,
		       cli_arg_check_f check)
{
  cli_arg_t * arg= _cli_arg_create(CLI_ARG_TYPE_VAR, name, check, NULL);
  arg->max_args= max_args;
  return arg;
}

// -----[ cli_opt ]--------------------------------------------------
cli_arg_t * cli_opt(const char * name, cli_arg_check_f check)
{
  return cli_opt2(name, check, NULL);
}

// -----[ cli_opt2 ]-------------------------------------------------
cli_arg_t * cli_opt2(const char * name, cli_arg_check_f check,
		     cli_arg_enum_f enumerate)
{
  char * name2= str_create(name);
  char * real_name= strsep(&name2, "=");
  cli_arg_t * arg= _cli_arg_create(CLI_ARG_TYPE_OPT, real_name, check, enumerate);
  if (name2 != NULL)
    arg->need_value= 1;
  str_destroy(&real_name);
  return arg;
}

