// ==================================================================
// @(#)cli.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 25/02/2004
// ==================================================================

#ifndef __GDS_CLI_H__
#define __GDS_CLI_H__

#include <libgds/array.h>
#include <libgds/stack.h>
#include <stdio.h>
#include <libgds/tokenizer.h>

#define CLI_SUCCESS                  0
#define CLI_ERROR_UNEXPECTED        -1
#define CLI_ERROR_UNKNOWN_COMMAND   -2
#define CLI_ERROR_MISSING_PARAMETER -3
#define CLI_ERROR_NOT_A_COMMAND     -4
#define CLI_ERROR_COMMAND_FAILED    -5
#define CLI_ERROR_BAD_PARAMETER     -6
#define CLI_ERROR_CTX_CREATE        -7
#define CLI_WARNING_EMPTY_COMMAND    1

typedef SPtrArray SCliCmds;
typedef SPtrArray SCliParams;

typedef struct {
  SStack * pStack;   // Stack with current contexts
  unsigned int uSavedDepth;
  STokens * pTokens; // List of current parameter values
  void * pContext;
  char * pString;
} SCliContext;

typedef int (*FCliContextCreate)(SCliContext * pContext, void ** ppItem);
typedef void (*FCliContextDestroy)(void ** pItem);
typedef int (*FCliCommand)(SCliContext * pContext, STokens * pTokens);
typedef int (*FCliCheckParam)(char * pcValue);

typedef struct {
  char * pcName;
  SCliCmds * pSubCmds;
  SCliParams * pParams;
  FCliContextCreate fCtxCreate;
  FCliContextDestroy fCtxDestroy;
  FCliCommand fCommand;
} SCliCmd;

typedef struct {
  void * pItem;
  SCliCmd * pCmd;
  unsigned int uNumParams;
} SCliCtxItem;

typedef struct {
  char * pcName;
  FCliCheckParam fCheckParam;
} SCliCmdParam;

typedef struct {
  STokenizer * pTokenizer;
  SCliCmd * pBaseCommand;
  SCliContext * pExecContext;     // Current execution context (stack)
  // Variables used for error reporting purpose:
  int iExecTokenIndex;           // Index to current token in command-line
  SCliCmd * pExecCmd;            // Command which is currently processed
  SCliCmdParam * pExecParam;     // Parameter which is currently expected
} SCli;

// ----- cli_cmd_param_create ---------------------------------------
extern SCliCmdParam * cli_cmd_param_create(char * pcName,
					   FCliCheckParam fCheckParam);
// ----- cli_cmd_param_destroy --------------------------------------
extern void cli_cmd_param_destroy(SCliCmdParam ** ppParam);
// ----- cli_cmds_create --------------------------------------------
extern SCliCmds * cli_cmds_create();
// ----- cli_cmds_destroy -------------------------------------------
extern void cli_cmds_destroy(SCliCmds ** ppCmds);
// ----- cli_cmds_add -----------------------------------------------
extern int cli_cmds_add(SCliCmds * pCmds, SCliCmd * pCmd);
// ----- cli_params_create ------------------------------------------
extern SCliParams * cli_params_create();
// ----- cli_params_destroy -----------------------------------------
extern void cli_params_destroy(SCliParams ** ppParams);
// ----- cli_params_add ---------------------------------------------
extern int cli_params_add(SCliParams * pParams, char * pcName,
			  FCliCheckParam fCheckParam);
// ----- cli_cmd_create ---------------------------------------------
extern SCliCmd * cli_cmd_create(char * pcName, FCliCommand fCommand,
				SCliCmds * pSubCmds,
				SCliParams * pParams);
// ----- cli_cmd_create ---------------------------------------------
extern SCliCmd * cli_cmd_create_ctx(char * pcName,
				    FCliContextCreate fCtxCreate,
				    FCliContextDestroy fCtxDestroy,
				    SCliCmds * pSubCmds,
				    SCliParams * pParams);
// ----- cli_cmd_destroy --------------------------------------------
extern void cli_cmd_destroy(SCliCmd ** ppCmd);
// ----- cli_cmd_dump -----------------------------------------------
extern void cli_cmd_dump(FILE * pStream, char * pcPrefix,
			 SCliCmd * pCmd);
// ----- cli_cmd_add_subcmd -----------------------------------------
extern int cli_cmd_add_subcmd(SCliCmd * pCmd, SCliCmd * pSubCmd);
// ----- cli_cmd_find_submd -----------------------------------------
extern SCliCmd * cli_cmd_find_subcmd(SCliCmd * pCmd, char * pcName);
// ----- cli_cmd_get_num_subcmds ------------------------------------
extern int cli_cmd_get_num_subcmds(SCliCmd * pCmd);
// ----- cli_cmd_get_subcmd_at --------------------------------------
extern SCliCmd * cli_cmd_get_subcmd_at(SCliCmd * pCmd, int iIndex);
// ----- cli_cmd_add_param ------------------------------------------
extern int cli_cmd_add_param(SCliCmd * pCmd, char * pcName,
			     FCliCheckParam fCheckParam);
// ----- cli_cmd_get_num_params -------------------------------------
extern int cli_cmd_get_num_params(SCliCmd * pCmd);
// ----- cli_cmd_get_param_at ---------------------------------------
extern SCliCmdParam * cli_cmd_get_param_at(SCliCmd * pCmd,
					   uint32_t uIndex);
// ----- cli_create -------------------------------------------------
extern SCli * cli_create();
// ----- cli_destroy ------------------------------------------------
extern void cli_destroy(SCli ** ppCli);
// ----- cli_execute_ctx --------------------------------------------
extern int cli_execute_ctx(SCli * pCli, char * pcCmd,
			   void * pContext);
// ----- cli_execute ------------------------------------------------
extern int cli_execute(SCli * pCli, char * pcCmd);
// ----- cli_register_cmd -------------------------------------------
extern int cli_register_cmd(SCli * pCli, SCliCmd * pCmd);
// ----- cli_perror -------------------------------------------------
extern void cli_perror(FILE * pStream, int iErrorCode);
// ----- cli_execute_file -------------------------------------------
extern int cli_execute_file(SCli * pCli, FILE * pStream);
// ----- cli_execute_line -------------------------------------------
extern int cli_execute_line(SCli * pCli, char * pcLine);

#endif
