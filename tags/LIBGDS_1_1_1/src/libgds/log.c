// ==================================================================
// @(#)log.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be), Sebastien Tandel
// @date 05/06/2003
// @lastdate 04/11/2003
// ==================================================================

#include <assert.h>
#include <libgds/memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <libgds/log.h>

#define LOG_OUTPUT_NULL   0
#define LOG_OUTPUT_STREAM 1
#define LOG_OUTPUT_FILE   2

SLog * pMainLog= NULL;

// ----- log_create -------------------------------------------------
/**
 *
 */
SLog * log_create()
{
  SLog * pLog= (SLog *) MALLOC(sizeof(SLog));
  pLog->pStream= NULL;
  pLog->uLevel= LOG_LEVEL_EVERYTHING;
  pLog->uOutput= LOG_OUTPUT_NULL;
  return pLog;
}

// ----- log_destroy ------------------------------------------------
/**
 *
 */
void log_destroy(SLog ** ppLog)
{
  if (*ppLog != NULL) {
    if ((*ppLog)->uOutput == LOG_OUTPUT_FILE)
    if ((*ppLog)->pStream != NULL)
      fclose((*ppLog)->pStream);
    FREE(*ppLog);
    *ppLog= NULL;
  }
}

// ----- log_set_level ----------------------------------------------
/**
 *
 */
void log_set_level(SLog * pLog, uint8_t uLevel)
{
  pLog->uLevel= uLevel;
}

// ----- log_get_stream ---------------------------------------------
/**
 *
 */
FILE * log_get_stream(SLog * pLog)
{
  return pLog->pStream;
}

// ----- log_set_stream ---------------------------------------------
/**
 *
 */
void log_set_stream(SLog * pLog, FILE * pStream)
{
  pLog->pStream= pStream;
  pLog->uOutput= LOG_OUTPUT_STREAM;
}

// ----- log_set_file -----------------------------------------------
/**
 *
 */
void log_set_file(SLog * pLog, char * pcFileName)
{
  // Close previously opened file
  if (pLog->uOutput == LOG_OUTPUT_FILE)
    if (pLog->pStream != NULL)
      fclose(pLog->pStream);
  pLog->uOutput= LOG_OUTPUT_NULL;
  // Open new file for appending
  pLog->pStream= fopen(pcFileName, "w");
  if (pLog->pStream != NULL)
    pLog->uOutput= LOG_OUTPUT_FILE;
  else
    perror("Unable to append to file");
}

// ----- log_enabled ------------------------------------------------
/**
 *
 */
int log_enabled(SLog * pLog, uint8_t uLevel)
{
  return (uLevel >= pLog->uLevel);
}

// ----- log_write --------------------------------------------------
/**
 *
 */
void log_write(SLog * pLog, uint8_t uLevel, char * pcFormat, ...)
{
  va_list ap;

  if (pLog->uOutput != LOG_OUTPUT_NULL) {
    assert(pLog->pStream != NULL);
    
    va_start(ap, pcFormat);
    if (log_enabled(pLog, uLevel)) {
      vfprintf(pLog->pStream, pcFormat, ap);
      if ((uLevel > LOG_LEVEL_WARNING) &&
	  (pLog->pStream != stderr))
	vfprintf(stderr, pcFormat, ap);
    }
  }
}

// ----- log_str2level ----------------------------------------------
/**
 *
 */
uint8_t log_str2level(char * pcStr)
{
  if (strcasecmp(pcStr, "everything"))
    return LOG_LEVEL_EVERYTHING;
  if (strcasecmp(pcStr, "debug"))
    return LOG_LEVEL_DEBUG;
  if (strcasecmp(pcStr, "info"))
    return LOG_LEVEL_INFO;
  if (strcasecmp(pcStr, "warning"))
    return LOG_LEVEL_WARNING;
  if (strcasecmp(pcStr, "severe"))
    return LOG_LEVEL_SEVERE;

  return LOG_LEVEL_FATAL;
}

/////////////////////////////////////////////////////////////////////
// INITIALIZATION AND FINALIZATION SECTION
/////////////////////////////////////////////////////////////////////

void _log_init() __attribute__((constructor));
void _log_destroy() __attribute__((destructor));

// ----- _log_init --------------------------------------------------
/**
 *
 */
void _log_init()
{
  pMainLog= log_create();
}

// ----- _log_destroy -----------------------------------------------
/**
 *
 */
void _log_destroy()
{
  log_destroy(&pMainLog);
}
