// ==================================================================
// @(#)cli.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @date 25/06/2003
// @lastdate 25/02/2004
// ==================================================================

#include <assert.h>
#include <libgds/cli.h>
#include <libgds/cli_ctx.h>
#include <libgds/memory.h>
#include <stdlib.h>
#include <string.h>

#define CLI_DELIMITERS " \t"
#define CLI_OPENING_QUOTES "\""
#define CLI_CLOSING_QUOTES "\""

// ----- cli_cmd_param_create ---------------------------------------
/**
 *
 */
SCliCmdParam * cli_cmd_param_create(char * pcName,
				    FCliCheckParam fCheckParam)
{
  SCliCmdParam * pParam= (SCliCmdParam *) MALLOC(sizeof(SCliCmdParam));

  pParam->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pParam->pcName, pcName);
  pParam->fCheckParam= fCheckParam;
  return pParam;
}

// ----- cli_cmd_param_destroy --------------------------------------
/**
 *
 */
void cli_cmd_param_destroy(SCliCmdParam ** ppParam)
{
  if (*ppParam != NULL) {
    FREE((*ppParam)->pcName);
    FREE(*ppParam);
    *ppParam= NULL;
  }
}

// ----- cli_params_item_compare ------------------------------------
int cli_params_item_compare(void * pItem1, void * pItem2,
			    uint32_t uEltSize)
{
  SCliCmdParam * pParam1= *((SCliCmdParam **) pItem1);
  SCliCmdParam * pParam2= *((SCliCmdParam **) pItem2);

  return strcmp(pParam1->pcName, pParam2->pcName);
}

// ----- cli_params_item_destroy ------------------------------------
void cli_params_item_destroy(void * pItem)
{
  cli_cmd_param_destroy((SCliCmdParam **) pItem);
}

// ----- cli_params_create ------------------------------------------
/**
 *
 */
SCliParams * cli_params_create()
{
  return (SCliParams *) ptr_array_create(0,
					 cli_params_item_compare,
					 cli_params_item_destroy);
}

// ----- cli_params_destroy -----------------------------------------
/**
 *
 */
void cli_params_destroy(SCliParams ** ppParams)
{
  ptr_array_destroy((SPtrArray **) ppParams);
}

// ----- cli_params_add ---------------------------------------------
/**
 *
 */
int cli_params_add(SCliParams * pParams, char * pcName,
		   FCliCheckParam fCheckParam)
{
  SCliCmdParam * pParam= cli_cmd_param_create(pcName, fCheckParam);
  return ptr_array_add((SPtrArray *) pParams, &pParam);
}

// ----- cli_cmd_create ---------------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_create(char * pcName,
			 FCliCommand fCommand,
			 SCliCmds * pSubCmds,
			 SCliParams * pParams)
{
  SCliCmd * pCmd= (SCliCmd *) MALLOC(sizeof(SCliCmd));

  pCmd->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pCmd->pcName, pcName);
  pCmd->pParams= pParams;
  pCmd->pSubCmds= pSubCmds;
  pCmd->fCtxCreate= NULL;
  pCmd->fCtxDestroy= NULL;
  pCmd->fCommand= fCommand;
  return pCmd;
}

// ----- cli_cmd_create_ctx -----------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_create_ctx(char * pcName,
			     FCliContextCreate fCtxCreate,
			     FCliContextDestroy fCtxDestroy,
			     SCliCmds * pSubCmds,
			     SCliParams * pParams)
{
  SCliCmd * pCmd= (SCliCmd *) MALLOC(sizeof(SCliCmd));

  pCmd->pcName= (char *) MALLOC(sizeof(char)*(strlen(pcName)+1));
  strcpy(pCmd->pcName, pcName);
  pCmd->pParams= pParams;
  pCmd->pSubCmds= pSubCmds;
  pCmd->fCtxCreate= fCtxCreate;
  pCmd->fCtxDestroy= fCtxDestroy;
  pCmd->fCommand= NULL;
  return pCmd;
}

// ----- cli_cmd_destroy --------------------------------------------
/**
 *
 */
void cli_cmd_destroy(SCliCmd ** ppCmd)
{
  if (*ppCmd != NULL) {
    FREE((*ppCmd)->pcName);
    ptr_array_destroy(&(*ppCmd)->pSubCmds);
    ptr_array_destroy(&(*ppCmd)->pParams);
    FREE(*ppCmd);
    *ppCmd= NULL;
  }
}

// ----- cli_cmd_dump -----------------------------------------------
/**
 *
 */
void cli_cmd_dump(FILE * pStream, char * pcPrefix, SCliCmd * pCmd)
{
  int iIndex;
  SCliCmd * pSubCmd;
  SCliCmdParam * pParam;
  char * pcNewPrefix;

  fprintf(pStream, "%s%s", pcPrefix, pCmd->pcName);
  if (pCmd->pParams != NULL)
    for (iIndex= 0; iIndex < ptr_array_length(pCmd->pParams); iIndex++) {
      pParam= (SCliCmdParam *) pCmd->pParams->data[iIndex];
      fprintf(pStream, " %s", pParam->pcName);
    }
  fprintf(pStream, "\n");
  if (pCmd->pSubCmds != NULL) {
    pcNewPrefix= (char *) MALLOC(sizeof(char)*(strlen(pcPrefix)+3));
    strcpy(pcNewPrefix, pcPrefix);
    strcat(pcNewPrefix, "  ");
    for (iIndex= 0; iIndex < ptr_array_length(pCmd->pSubCmds); iIndex++) {
      pSubCmd= (SCliCmd *) pCmd->pSubCmds->data[iIndex];
      cli_cmd_dump(pStream, pcNewPrefix, pSubCmd);
    }
    FREE(pcNewPrefix);
  }
}

// ----- cli_cmds_item_compare --------------------------------------
int cli_cmds_item_compare(void * pItem1, void * pItem2,
			  uint32_t EltSize)
{
  SCliCmd * pCmd1= *((SCliCmd **) pItem1);
  SCliCmd * pCmd2= *((SCliCmd **) pItem2);

  return strcmp(pCmd1->pcName, pCmd2->pcName);
}

// ----- cli_cmds_item_destroy --------------------------------------
void cli_cmds_item_destroy(void * pItem)
{
  cli_cmd_destroy((SCliCmd **) pItem);
}

// ----- cli_cmds_create --------------------------------------------
/**
 *
 */
SCliCmds * cli_cmds_create()
{
  return (SCliCmds *) ptr_array_create(ARRAY_OPTION_SORTED,
				       cli_cmds_item_compare,
				       cli_cmds_item_destroy);
}

// ----- cli_cmds_destroy -------------------------------------------
/**
 *
 */
void cli_cmds_destroy(SCliCmds ** ppCmds)
{
  ptr_array_destroy((SPtrArray **) ppCmds);
}

// ----- cli_cmds_add -----------------------------------------------
/**
 *
 */
int cli_cmds_add(SCliCmds * pCmds, SCliCmd * pCmd)
{
  return ptr_array_add((SPtrArray *) pCmds, &pCmd);
}

// ----- cli_cmd_find_submd -----------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_find_subcmd(SCliCmd * pCmd, char * pcName)
{
  char ** x= &pcName;
  int iIndex;

  if (pCmd->pSubCmds != NULL)
    if (!ptr_array_sorted_find_index(pCmd->pSubCmds, &x, &iIndex))
      return pCmd->pSubCmds->data[iIndex];
  return NULL;
}

// ----- cli_cmd_get_num_subcmds ------------------------------------
/**
 *
 */
int cli_cmd_get_num_subcmds(SCliCmd * pCmd)
{
  if (pCmd->pSubCmds != NULL)
    return ptr_array_length(pCmd->pSubCmds);
  return 0;
}

// ----- cli_cmd_get_subcmd_at --------------------------------------
/**
 *
 */
SCliCmd * cli_cmd_get_subcmd_at(SCliCmd * pCmd, int iIndex)
{
  return (SCliCmd *) pCmd->pSubCmds->data[iIndex];
}

// ----- cli_cmd_add_subcmd -----------------------------------------
/**
 *
 */
int cli_cmd_add_subcmd(SCliCmd * pCmd, SCliCmd * pSubCmd)
{
  if (pCmd->pSubCmds == NULL)
    pCmd->pSubCmds= cli_cmds_create();
  return cli_cmds_add(pCmd->pSubCmds, pSubCmd);
}

// ----- cli_cmd_add_param ------------------------------------------
/**
 *
 */
int cli_cmd_add_param(SCliCmd * pCmd, char * pcName,
		      FCliCheckParam fCheckParam)
{
  if (pCmd->pParams == NULL)
    pCmd->pParams= cli_params_create();
  return cli_params_add(pCmd->pParams, pcName, fCheckParam);
}

// ----- cli_cmd_get_num_params -------------------------------------
/**
 *
 */
int cli_cmd_get_num_params(SCliCmd * pCmd)
{
  if (pCmd->pParams == NULL)
    return 0;
  return ptr_array_length(pCmd->pParams);
}

// ----- cli_cmd_get_param_at ---------------------------------------
/**
 *
 */
SCliCmdParam * cli_cmd_get_param_at(SCliCmd * pCmd, uint32_t uIndex)
{
  return (SCliCmdParam *) pCmd->pParams->data[uIndex];
}

// ----- cli_create -------------------------------------------------
/**
 *
 */
SCli * cli_create()
{
  SCli * pCli= (SCli *) MALLOC(sizeof(SCli));

  pCli->pTokenizer= tokenizer_create(CLI_DELIMITERS,
				     0,
				     CLI_OPENING_QUOTES,
				     CLI_CLOSING_QUOTES);
  pCli->pBaseCommand= NULL;
  pCli->pExecCmd= NULL;
  pCli->pExecParam= NULL;
  pCli->pExecContext= cli_context_create();
  return pCli;
}

// ----- cli_destroy ------------------------------------------------
/**
 *
 */
void cli_destroy(SCli ** ppCli)
{
  if (*ppCli != NULL) {
    tokenizer_destroy(&(*ppCli)->pTokenizer);
    cli_context_destroy(&(*ppCli)->pExecContext);
    cli_cmd_destroy(&(*ppCli)->pBaseCommand);
    FREE(*ppCli);
    *ppCli= NULL;
  }
}

// ----- cli_execute_cmd --------------------------------------------
/**
 *
 */
int cli_execute_cmd(SCli * pCli, SCliCmd * pCmd)
{
  int iParamIndex;
  int iResult;
  char * pcParamValue;
  STokens * pTokens= tokenizer_get_tokens(pCli->pTokenizer);
  void * pContextItem;

  // Match all the command's parameters
  for (iParamIndex= 0;
       iParamIndex < cli_cmd_get_num_params(pCli->pExecCmd);
       iParamIndex++) {
    pCli->pExecParam= cli_cmd_get_param_at(pCli->pExecCmd, iParamIndex);
    // Still enough parameters in command-line ?
    if (pCli->iExecTokenIndex >= tokens_get_num(pTokens))
      return CLI_ERROR_MISSING_PARAMETER;
    pcParamValue= tokens_get_string_at(pTokens, pCli->iExecTokenIndex);

    // Check the parameter's value (if required)
    if (pCli->pExecParam->fCheckParam != NULL) {
      iResult= pCli->pExecParam->fCheckParam(pcParamValue);
      if (iResult)
	return iResult; // parameter error !!!
    }
    cli_context_add_token_copy(pCli->pExecContext, pcParamValue);
    pCli->iExecTokenIndex++;
  }

  // Produce context ?
  if (pCli->pExecCmd->fCtxCreate != NULL) {
    pContextItem= NULL;
    iResult= pCli->pExecCmd->fCtxCreate(pCli->pExecContext, &pContextItem);
    if (iResult)
      return iResult;
    cli_context_push(pCli->pExecContext, pContextItem, pCli->pExecCmd);
  }

  // Execute command ?
  if (pCli->iExecTokenIndex == tokens_get_num(pTokens)) {
    if (pCli->pExecCmd->fCommand != NULL) {
      iResult= pCli->pExecCmd->fCommand(pCli->pExecContext,
					pCli->pExecContext->pTokens);
      // Return to saved context level
      cli_context_restore_depth(pCli->pExecContext);
      if (iResult)
	return iResult;
    } else
      if (pCli->pExecCmd->fCtxCreate == NULL)
	return CLI_ERROR_NOT_A_COMMAND;
  }

  return CLI_SUCCESS;
}

// ----- cli_execute_ctx --------------------------------------------
/**
 *
 */
int cli_execute_ctx(SCli * pCli, char * pcCmd, void * pContext)
{
  int iResult= CLI_SUCCESS;
  STokens * pTokens;
  char * pcToken;
  SCliCmd * pNewCmd;
  SCliCtxItem * pCtxItem;

  cli_context_set(pCli->pExecContext, pContext);

  // Split command line into tokens...
  if (tokenizer_run(pCli->pTokenizer, pcCmd))
    return CLI_ERROR_UNEXPECTED;
  pTokens= tokenizer_get_tokens(pCli->pTokenizer);

  // Empty line ?
  if (tokens_get_num(pTokens) == 0) {
    cli_context_clear(pCli->pExecContext);
    return CLI_SUCCESS;
  }
  // Single "exit" token ?
  if ((tokens_get_num(pTokens) == 1) &&
      (!strcmp("exit", tokens_get_string_at(pTokens, 0)))) {
    cli_context_backtrack(pCli->pExecContext);
    return CLI_SUCCESS;
  }
  // Context available ?
  if (!cli_context_is_empty(pCli->pExecContext)) {
    pCtxItem= cli_context_top(pCli->pExecContext);
    assert(pCtxItem != NULL);
    pCli->pExecCmd= pCtxItem->pCmd;
    // Strip params ?
    while (tokens_get_num(pCli->pExecContext->pTokens) > pCtxItem->uNumParams)
      ptr_array_remove_at((SPtrArray *) pCli->pExecContext->pTokens,
			  tokens_get_num(pCli->pExecContext->pTokens)-1);
  } else {
    pCli->pExecCmd= pCli->pBaseCommand;
    tokens_destroy(&pCli->pExecContext->pTokens);
    pCli->pExecContext->pTokens= tokens_create();
  }
  pCli->iExecTokenIndex= 0;
  pCli->pExecParam= NULL;
  cli_context_save_depth(pCli->pExecContext);

  // Match all tokens...
  while (pCli->iExecTokenIndex < tokenizer_get_num_tokens(pCli->pTokenizer)) {
    if (pCli->pExecCmd != NULL) {
      pcToken= tokens_get_string_at(pTokens, pCli->iExecTokenIndex);
      pNewCmd= cli_cmd_find_subcmd(pCli->pExecCmd, pcToken);
      if (pNewCmd == NULL) {
	iResult= CLI_ERROR_UNKNOWN_COMMAND;
	break;
      }
      pCli->pExecCmd= pNewCmd;
      pCli->iExecTokenIndex++;
      iResult= cli_execute_cmd(pCli, pCli->pExecCmd);
      if (iResult)
	break;
    } else {
      iResult= CLI_ERROR_UNKNOWN_COMMAND;
      break;
    }
  }
  
  if (iResult < 0)
    cli_context_restore_depth(pCli->pExecContext);

  return iResult;
}

// ----- cli_execute ------------------------------------------------
/**
 *
 */
int cli_execute(SCli * pCli, char * pcCmd)
{
  return cli_execute_ctx(pCli, pcCmd, NULL);
}

// ----- cli_register_cmd -------------------------------------------
/**
 *
 */
int cli_register_cmd(SCli * pCli, SCliCmd * pCmd)
{
  if (pCli->pBaseCommand == NULL)
    pCli->pBaseCommand= cli_cmd_create("", NULL, NULL, NULL);
  return cli_cmd_add_subcmd(pCli->pBaseCommand, pCmd);
}

// ----- cli_perror -------------------------------------------------
/**
 *
 */
void cli_perror(FILE * pStream, int iErrorCode)
{
  switch (iErrorCode) {
  case CLI_SUCCESS:
    fprintf(pStream, "success\n"); break;
  case CLI_ERROR_UNEXPECTED:
	fprintf(pStream, "unexpected error\n"); break;
  case CLI_ERROR_UNKNOWN_COMMAND:
    fprintf(pStream, "unknown command\n"); break;
  case CLI_ERROR_MISSING_PARAMETER:
    fprintf(pStream, "missing parameter\n"); break;
  case CLI_ERROR_NOT_A_COMMAND:
    fprintf(pStream, "not a command\n"); break;
  case CLI_ERROR_COMMAND_FAILED:
    fprintf(pStream, "command failed\n"); break;
  case CLI_ERROR_BAD_PARAMETER:
    fprintf(pStream, "bad parameter value\n"); break;
  case CLI_ERROR_CTX_CREATE:
    fprintf(pStream, "unable to create context\n"); break;
  case CLI_WARNING_EMPTY_COMMAND:
    fprintf(pStream, "empty command\n"); break;
  default:
    fprintf(pStream, "unknown error\n");
  }
}

// ----- cli_perror_details -----------------------------------------
/**
 *
 */
void cli_perror_details(FILE * pStream, int iResult, SCli * pCli,
			char * pcLine)
{
  STokens * pTokens;
  int iIndex;

  fprintf(pStream, "*** command: \"%s\"\n", pcLine);
  if ((iResult == CLI_ERROR_UNKNOWN_COMMAND) ||
      (iResult == CLI_ERROR_MISSING_PARAMETER) ||
      (iResult == CLI_ERROR_BAD_PARAMETER)) {
    fprintf(pStream, "*** error  : \"");
    pTokens= tokenizer_get_tokens(pCli->pTokenizer);
    for (iIndex= 0; iIndex < pCli->iExecTokenIndex; iIndex++)
      fprintf(pStream, "%s ", tokens_get_string_at(pTokens, iIndex));
    fprintf(pStream, "^^^\"\n");
    if ((iResult == CLI_ERROR_UNKNOWN_COMMAND) &&
	(pCli->pExecCmd != NULL)) {
      fprintf(pStream, "*** expect : ");


      for (iIndex= 0;
	   iIndex < cli_cmd_get_num_subcmds(pCli->pExecCmd);
	   iIndex++) {
	if (iIndex > 0)
	  fprintf(pStream, ", ");
	fprintf(pStream, "%s",
		cli_cmd_get_subcmd_at(pCli->pExecCmd, iIndex)->pcName);
      }
      fprintf(pStream, "\n");
    } else if (((iResult == CLI_ERROR_MISSING_PARAMETER) ||
	 (iResult == CLI_ERROR_BAD_PARAMETER)) &&
	(pCli->pExecParam != NULL))
      fprintf(pStream, "*** expect : %s\n", pCli->pExecParam->pcName);
  }
}

// ----- cli_execute_line -------------------------------------------
/**
 *
 */
int cli_execute_line(SCli * pCli, char * pcLine)
{
  int iLen;
  int iResult= CLI_SUCCESS;

  // Skip commented lines
  if (pcLine[0] != '#') {
    // Chop '\n' at end of line
    iLen= strlen(pcLine);
    if ((iLen >= 1) && (pcLine[iLen-1] == '\n'))
      pcLine[iLen-1]= '\0';
    // Parse and execute command
    iResult= cli_execute(pCli, pcLine);
    if (iResult != CLI_SUCCESS) {
      fprintf(stderr, "Error: ");
      cli_perror(stderr, iResult);
      cli_perror_details(stderr, iResult, pCli, pcLine);
    }
  }
  return iResult;
}

// ----- cli_execute_file -------------------------------------------
/**
 *
 */
int cli_execute_file(SCli * pCli, FILE * pStream)
{
  char acLine[1024];
  int iResult;
  uint32_t uLineNumber= 1;

  while (fgets(acLine, sizeof(acLine), pStream) != NULL) {
    iResult= cli_execute_line(pCli, acLine);
    if (iResult) {
      fprintf(stderr, "Error: in script file, line %u\n", uLineNumber);
      return iResult;
    }
    uLineNumber++;
  }
  return CLI_SUCCESS;
}
