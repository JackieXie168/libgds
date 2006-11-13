// ==================================================================
// @(#)log.c
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be), Sebastien Tandel
// @date 17/05/2005
// @lastdate 03/03/2006
// ==================================================================

#include <assert.h>
#include <errno.h>
#include <config.h>
#include <libgds/memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <libgds/log.h>

#define LOG_STREAM_BUFFER_SIZE 1024

// -----[ log_create_stream ]----------------------------------------
/**
 *
 */
SLogStream * log_create_stream(FILE * pStream)
{
  SLogStream * pLogStream= (SLogStream *) MALLOC(sizeof(SLogStream));
  pLogStream->eLevel= LOG_LEVEL_EVERYTHING;
  pLogStream->eType= LOG_TYPE_STREAM;
  pLogStream->pStream= pStream;
  return pLogStream;
}

// -----[ log_create_file ]-------------------------------------------
/**
 *
 */
SLogStream * log_create_file(char * pcFileName)
{
  SLogStream * pLogStream= NULL;
  FILE * pFile;

  if ((pFile= fopen(pcFileName, "w")) == NULL)
    return NULL;

  pLogStream= log_create_stream(pFile);
  pLogStream->eType= LOG_TYPE_FILE;
  return pLogStream;
}

// -----[ log_create_callback ]--------------------------------------
/**
 *
 */
SLogStream * log_create_callback(FLogStreamCallback fCallback,
				 void * pContext)
{
  SLogStream * pLogStream= log_create_stream(stderr);

  pLogStream->eType= LOG_TYPE_CALLBACK;
  pLogStream->sCallback.fCallback= fCallback;
  pLogStream->sCallback.pContext= pContext;
  
  return pLogStream;
}

// ----- log_destroy ------------------------------------------------
/**
 *
 */
void log_destroy(SLogStream ** ppLogStream)
{
  if (*ppLogStream != NULL) {
    if ((*ppLogStream)->eType == LOG_TYPE_FILE)
      if ((*ppLogStream)->pStream != NULL)
	fclose((*ppLogStream)->pStream);
    FREE(*ppLogStream);
    *ppLogStream= NULL;
  }
}

// ----- log_set_level ----------------------------------------------
/**
 *
 */
void log_set_level(SLogStream * pLogStream, ELogLevel eLevel)
{
  pLogStream->eLevel= eLevel;
}

// ----- log_get_stream ---------------------------------------------
/**
 *
 */
/*
FILE * log_get_stream(SLog * pLog)
{
  return pLog->pStream;
}
*/

// ----- log_set_stream ---------------------------------------------
/**
 *
 */
/*
void log_set_stream(SLog * pLog, FILE * pStream)
{
  pLog->pStream= pStream;
  pLog->uOutput= LOG_OUTPUT_STREAM;
}
*/

// ----- log_set_file -----------------------------------------------
/**
 *
 */
/*
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
*/

// ----- log_enabled ------------------------------------------------
/**
 *
 */
int log_enabled(SLogStream * pLogStream, ELogLevel eLevel)
{
  return (eLevel >= pLogStream->eLevel);
}

// ----- log_printf -------------------------------------------------
/**
 *
 */
void log_printf(SLogStream * pLogStream, char * pcFormat, ...)
{
  static char acBuffer[LOG_STREAM_BUFFER_SIZE];
  va_list ap;

  switch (pLogStream->eType) {
  case LOG_TYPE_STREAM:
  case LOG_TYPE_FILE:
    assert(pLogStream->pStream != NULL);
    va_start(ap, pcFormat);
    vfprintf(pLogStream->pStream, pcFormat, ap);
    /*if ((uLevel > LOG_LEVEL_WARNING) &&
	(pLogStream->pStream != stderr))
	vfprintf(stderr, pcFormat, ap);*/
    break;

  case LOG_TYPE_CALLBACK:
    assert(pLogStream->sCallback.fCallback != NULL);
    va_start(ap, pcFormat);
    vsnprintf(acBuffer, LOG_STREAM_BUFFER_SIZE, pcFormat, ap);
    pLogStream->sCallback.fCallback(pLogStream->sCallback.pContext, acBuffer);
    break;

  default:
    abort();
  }
}

// -----[ log_flush ]------------------------------------------------
/**
 *
 */
void log_flush(SLogStream * pLogStream)
{
  switch (pLogStream->eType) {
  case LOG_TYPE_STREAM:
  case LOG_TYPE_FILE:
    fflush(pLogStream->pStream);
    break;
  case LOG_TYPE_CALLBACK:
    // TO BE IMPLEMENTED...
    break;
  default:
    abort();
  }
}


// ----- log_perror -------------------------------------------------
/**
 *
 */
void log_perror(SLogStream * pLogStream, char * pcFormat, ...)
{
  va_list ap;

  va_start(ap, pcFormat);
  if (log_enabled(pLogStream, LOG_LEVEL_SEVERE)) {
    log_printf(pLogStream, pcFormat, ap);
    log_printf(pLogStream, "%s\n", strerror(errno));
  }
}

// ----- log_str2level ----------------------------------------------
/**
 *
 */
ELogLevel log_str2level(char * pcStr)
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

// -----[ Standard log streams ]-------------------------------------
SLogStream * pLogErr= NULL;
SLogStream * pLogOut= NULL;
SLogStream * pLogDebug= NULL;

// ----- _log_init --------------------------------------------------
/**
 * Initialize "standard" log streams.
 */
void _log_init()
{
  pLogErr= log_create_stream(stderr);
  pLogOut= log_create_stream(stdout);
  pLogDebug= log_create_stream(stderr);
}

// ----- _log_destroy -----------------------------------------------
/**
 * Destroy "standard" log streams.
 */
void _log_destroy()
{
  log_destroy(&pLogErr);
  log_destroy(&pLogOut);
  log_destroy(&pLogDebug);
}
