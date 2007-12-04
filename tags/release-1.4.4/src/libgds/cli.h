// ==================================================================
// @(#)cli.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 20/11/2007
// ==================================================================

#ifndef __GDS_CLI_H__
#define __GDS_CLI_H__

#include <libgds/array.h>
#include <libgds/cli_params.h>
#include <libgds/log.h>
#include <libgds/stack.h>
#include <stdio.h>
#include <libgds/tokenizer.h>

#define MAX_CLI_LINE_LENGTH 1024

#define CLI_SUCCESS                    0   // The execution was successful
#define CLI_ERROR_GENERIC             -1   // Generic error (no details)
#define CLI_ERROR_UNEXPECTED          -2   // Error detected, cause unknown
#define CLI_ERROR_UNKNOWN_COMMAND     -3   // Command does not exist in context
#define CLI_ERROR_MISSING_PARAM       -4   // Command needs more parameters
#define CLI_ERROR_NOT_A_COMMAND       -5   // Command cannot be executed
#define CLI_ERROR_COMMAND_FAILED      -6   // Command returned an error
#define CLI_ERROR_BAD_PARAM           -7   // Invalid parameter value
#define CLI_ERROR_CTX_CREATE          -8   // Context creation failed
#define CLI_ERROR_TOO_MANY_PARAMS     -9   // Too many parameters provided
#define CLI_ERROR_UNKNOWN_OPTION      -10  // Option does not exist in command
#define CLI_ERROR_BAD_OPTION          -11  // Invalid option value
#define CLI_WARNING_EMPTY_COMMAND      1
#define CLI_SUCCESS_TERMINATE          2
#define CLI_SUCCESS_HELP               3

#define CLI_MATCH_NOTHING      0
#define CLI_MATCH_COMMAND      1
#define CLI_MATCH_OPTION_NAMES 2
#define CLI_MATCH_OPTION_VALUE 3
#define CLI_MATCH_PARAM_VALUE  4

typedef SPtrArray SCliCmds;

struct TCliCmd;
typedef struct TCliCmd SCliCmd;

typedef struct {
  void * pUserData;         // Current user data
  SCliCmd * pCmd;           // Current command
  SStack * pCmdStack;       // Stack of commands
  unsigned int uSavedDepth;
  char * pString;
} SCliContext;

typedef int (*FCliContextCreate)(SCliContext * pContext, void ** ppItem);
typedef void (*FCliContextDestroy)(void ** pItem);
typedef int (*FCliCommand)(SCliContext * pContext, SCliCmd * pCmd);
typedef void (*FCliHelp)(SCliCmd * pCmd, void * pContext);
typedef int (*FCliExitOnError)(int iResult);

struct TCliCmd {
  char * pcName;
  SCliCmds * pSubCmds;
  SCliParams * pParams;
  SCliCmd * pParent;
  STokens * pParamValues;
  SCliOptions * pOptions;
  FCliContextCreate fCtxCreate;
  FCliContextDestroy fCtxDestroy;
  FCliCommand fCommand;
  char * pcHelp;
};

typedef struct {
  void * pUserData;         // User data
  SCliCmd * pCmd;           // Saved command
} SCliCtxItem;

typedef struct {
  int    iErrorCode;   // Last error code
  int    iLineNumber;  // Line number of last command
  int    iUserError;   // Last application error code
                       // (has a meaning only if last command failed)
  char * pcUserError;  // Last application error message
                       // (has a meaning only if last command failed)
} SCliErrorDetails;

typedef struct {
  STokenizer      * pTokenizer;
  SCliCmd         * pBaseCommand;
  SCliContext     * pCtx;            // Current execution context (stack)
  FCliExitOnError   fExitOnError;    // Exit callback function
  FCliHelp          fHelp;           // Help callback function
  // --- Variables used for error reporting purpose ---
  int               uExecTokenIndex; // Index to current token in command-line
  SCliParam       * pExecParam;      // Parameter which is currently expected
  SCliErrorDetails  sErrorDetails;
} SCli;

#ifdef __cplusplus
extern "C" {
#endif
  
  ///////////////////////////////////////////////////////////////////
  // COMMANDS MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // ----- cli_cmds_create ------------------------------------------
  SCliCmds * cli_cmds_create();
  // ----- cli_cmds_destroy -----------------------------------------
  void cli_cmds_destroy(SCliCmds ** ppCmds);
  // ----- cli_matching_cmds ----------------------------------------
  SCliCmds * cli_matching_cmds(SCliCmds * pCmds, const char * pcText);
  // ----- cli_cmds_add ---------------------------------------------
  int cli_cmds_add(SCliCmds * pCmds, SCliCmd * pCmd);
  // ----- cli_cmd_create -------------------------------------------
  SCliCmd * cli_cmd_create(char * pcName, FCliCommand fCommand,
			   SCliCmds * pSubCmds,
			   SCliParams * pParams);
  // ----- cli_cmd_create -------------------------------------------
  SCliCmd * cli_cmd_create_ctx(char * pcName,
			       FCliContextCreate fCtxCreate,
			       FCliContextDestroy fCtxDestroy,
			       SCliCmds * pSubCmds,
			       SCliParams * pParams);
  // ----- cli_cmd_destroy ------------------------------------------
  void cli_cmd_destroy(SCliCmd ** ppCmd);
  // ----- cli_cmd_dump ---------------------------------------------
  void cli_cmd_dump(SLogStream * pStream, char * pcPrefix,
		    SCliCmd * pCmd);
  // ----- cli_cmd_add_subcmd ---------------------------------------
  int cli_cmd_add_subcmd(SCliCmd * pCmd, SCliCmd * pSubCmd);
  // ----- cli_cmd_find_submd ---------------------------------------
  SCliCmd * cli_cmd_find_subcmd(SCliCmd * pCmd, char * pcName);
  // ----- cli_cmd_get_num_subcmds ----------------------------------
  int cli_cmd_get_num_subcmds(SCliCmd * pCmd);
  // ----- cli_cmd_get_subcmd_at ------------------------------------
  SCliCmd * cli_cmd_get_subcmd_at(SCliCmd * pCmd, int iIndex);
  // ----- cli_cmd_add_param ----------------------------------------
  int cli_cmd_add_param(SCliCmd * pCmd, char * pcName,
			FCliCheckParam fCheckParam);
  // ----- cli_cmd_get_num_params -----------------------------------
  int cli_cmd_get_num_params(SCliCmd * pCmd);
  // ----- cli_cmd_get_param_at -------------------------------------
  SCliParam * cli_cmd_get_param_at(SCliCmd * pCmd,
				   uint32_t uIndex);
  // ----- cli_cmd_add_option ---------------------------------------
  int cli_cmd_add_option(SCliCmd * pCmd,
			 char * pcName,
			 FCliCheckParam fCheckParam);
  // ----- _cli_cmd_match -------------------------------------------
  int cli_cmd_match(SCli * pCli, SCliCmd * pCmd, char * pcStartCmd,
		    char * pcEndCmd, void ** ppCtx);


  ///////////////////////////////////////////////////////////////////
  // CLI MANAGMENT FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // ----- cli_create -----------------------------------------------
  SCli * cli_create();
  // ----- cli_destroy ----------------------------------------------
  void cli_destroy(SCli ** ppCli);
  // ----- cli_set_exit_callback ------------------------------------
  void cli_set_exit_callback(SCli * pCli, FCliExitOnError fExitOnError);
  // ----- cli_execute_ctx ------------------------------------------
  int cli_execute_ctx(SCli * pCli, char * pcCmd,
		      void * pContext);
  // ----- cli_execute ----------------------------------------------
  int cli_execute(SCli * pCli, char * pcCmd);
  // ----- cli_register_cmd -----------------------------------------
  int cli_register_cmd(SCli * pCli, SCliCmd * pCmd);
  // ----- cli_perror -----------------------------------------------
  void cli_perror(SLogStream * pStream, int iErrorCode);
  // ----- cli_strerror -----------------------------------------------
  char * cli_strerror(int iErrorCode);
  // ----- cli_execute_file -----------------------------------------
  int cli_execute_file(SCli * pCli, FILE * pStream);
  // ----- cli_execute_line -----------------------------------------
  int cli_execute_line(SCli * pCli, const char * pcLine);
  // ----- cli_get_cmd_context --------------------------------------
  SCliCmd * cli_get_cmd_context(SCli * pCli);
  // ----- cli_get_error_details ------------------------------------
  int cli_get_error_details(SCli * pCli, SCliErrorDetails * psDetails);
  // ----- cli_set_user_error -----------------------------------------
  void cli_set_user_error(SCli * pCli, char * pcFormat, ...);


#ifdef __cplusplus
}
#endif

#endif /* __GDS_CLI_H__ */
