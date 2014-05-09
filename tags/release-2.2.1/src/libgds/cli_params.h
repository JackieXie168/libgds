// ==================================================================
// @(#)cli_params.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/06/2003
// $Id$
// ==================================================================

/**
 * \file
 * Provide functions to manage CLI arguments and options. Most of
 * these functions are for internal use only.
 */

#ifndef __GDS_CLI_PARAMS_H__
#define __GDS_CLI_PARAMS_H__

#include <libgds/array.h>
#include <libgds/stream.h>
#include <libgds/tokens.h>

/////////////////////////////////////////////////////////////////////
//
// ARGUMENTS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern"C" {
#endif
  
  // -----[ cli_args_create ]----------------------------------------
  cli_args_t * cli_args_create();
  // -----[ cli_args_create_ref ]------------------------------------
  cli_args_t * cli_args_create_ref();
  // -----[ cli_args_destroy ]---------------------------------------
  void cli_args_destroy(cli_args_t ** args_ref);
  // -----[ cli_arg_dump ]-------------------------------------------
  void cli_arg_dump(gds_stream_t * stream, cli_arg_t * arg);
  // -----[ cli_args_dump ]------------------------------------------
  void cli_args_dump(gds_stream_t * stream, cli_args_t * args);
  // -----[ cli_args_add ]-------------------------------------------
  cli_arg_t * cli_args_add(cli_args_t * args, cli_arg_t * arg);


  // -----[ cli_arg_check ]------------------------------------------
  int cli_arg_check(const cli_arg_t * arg, const char * value);
  // -----[ cli_args_bound ]-----------------------------------------
  void cli_args_bounds(cli_args_t * args, unsigned int * min,
		       unsigned int * max);
  // -----[ cli_args_num ]-------------------------------------------
  static inline unsigned int cli_args_num(cli_args_t * args) {
    if (args == NULL)
      return 0;
    return ptr_array_length((ptr_array_t *) args);
  }
  // -----[ cli_args_at ]--------------------------------------------
  static inline cli_arg_t * cli_args_at(cli_args_t * args,
					unsigned int index) {
    return (cli_arg_t *) args->data[index];
  }
  
#ifdef __cplusplus
}
#endif


/////////////////////////////////////////////////////////////////////
//
// OPTIONS MANAGEMENT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ cli_opts_create ]----------------------------------------
  cli_opts_t * cli_opts_create();
  // -----[ cli_opts_destroy ]---------------------------------------
  void cli_opts_destroy(cli_opts_t ** opts_ref);
  // -----[ cli_opts_add ]-------------------------------------------
  cli_arg_t * cli_opts_add(cli_opts_t * opts, cli_arg_t * opt);

  // -----[ cli_opts_find ]------------------------------------------
  cli_arg_t * cli_opts_find(cli_opts_t * opts, const char * name);

  // -----[ cli_opts_has_value ]-------------------------------------
  int cli_opts_has_value(cli_opts_t * opts,
			 const char * name);
  // ----- cli_opts_get_value ---------------------------------------
  char * cli_opts_get_value(cli_opts_t * opts,
			    const char * name);
  // -----[ cli_opts_set_value ]-------------------------------------
  int cli_opts_set_value(cli_opts_t * opts,
			 const char * name,
			 const char * value);
  // -----[ cli_args_clear ]-----------------------------------------
  void cli_args_clear(cli_opts_t * opts);

  // -----[ cli_arg_set_value ]--------------------------------------
  int cli_arg_set_value(cli_arg_t * arg, const char * value);
  // -----[ cli_arg_get_value ]--------------------------------------
  const char * cli_arg_get_value(cli_arg_t * arg, unsigned int index);
  // -----[ cli_arg_get_num_values ]---------------------------------
  unsigned int cli_arg_get_num_values(cli_arg_t * arg);


  ///////////////////////////////////////////////////////////////////
  // CLI ARG/VARARG/OPT CREATION
  ///////////////////////////////////////////////////////////////////

  // -----[ cli_arg ]------------------------------------------------
  /**
   * Create a CLI argument.
   *
   * \param name  is the argument's name.
   * \param check is an argument value check function (can be NULL).
   * \retval the argument.
   */
  cli_arg_t * cli_arg(const char * name, cli_arg_check_f check);

  // -----[ cli_arg2 ]-----------------------------------------------
  /**
   * Create a CLI argument (with enumeration function).
   *
   * \param name      is the argument's name.
   * \param check     is an argument value check function (can be NULL).
   * \param enumerate is an argument value enumeration function
   *   (can be NULL).
   * \retval the argument.
   */
  cli_arg_t * cli_arg2(const char * name, cli_arg_check_f check,
		       cli_arg_enum_f enumerate);

  // -----[ cli_vararg ]---------------------------------------------
  /**
   * Create a CLI vararg.
   *
   * A vararg is an argument that accepts a variable number of
   * values. This type of argument must always be the last argument
   * of a command.
   *
   * \param name     is the arguments name.
   * \param max_args is the maximum number of values accepted.
   * \param check    is an argument value check function.
   * \retval the argument.
   */
  cli_arg_t * cli_vararg(const char * name, uint8_t max_args,
			 cli_arg_check_f check);

  // -----[ cli_opt ]------------------------------------------------
  /**
   * Create an optional argument.
   *
   * \param name is the option's name.
   * \param check is the option value check function (can be NULL).
   * \retval the option.
   */
  cli_arg_t * cli_opt(const char * name, cli_arg_check_f check);

  // -----[ cli_opt2 ]-------------------------------------------------
  /**
   * Create an optional argument (with enumeration function).
   *
   * \param name      is the option's name.
   * \param check     is the option value check function (can be NULL).
   * \param enumerate is the option value enumerate function
   *   (can be NULL).
   * \retval the option.
   */
  cli_arg_t * cli_opt2(const char * name, cli_arg_check_f check,
		       cli_arg_enum_f enumerate);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_PARAMS_H__ */
