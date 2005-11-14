// ==================================================================
// @(#)log.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be), Sebastien Tandel
// @date 05/06/2003
// @lastdate 04/11/2003
// ==================================================================

#ifndef __GDS_LOG_H__
#define __GDS_LOG_H__

#include <stdio.h>
#include <libgds/types.h>

#define LOG_LEVEL_EVERYTHING 0
#define LOG_LEVEL_DEBUG      1
#define LOG_LEVEL_INFO       2
#define LOG_LEVEL_WARNING    3
#define LOG_LEVEL_SEVERE     4
#define LOG_LEVEL_FATAL      5
#define LOG_LEVEL_MAX        255

#define LOG_ENABLED_EVERYTHING() \
  if (log_enabled(pMainLog, LOG_LEVEL_EVERYTHING))
#define LOG_ENABLED_DEBUG() if (log_enabled(pMainLog, LOG_LEVEL_DEBUG))
#define LOG_ENABLED_INFO() if (log_enabled(pMainLog, LOG_LEVEL_INFO))
#define LOG_ENABLED_WARNING() if (log_enabled(pMainLog, LOG_LEVEL_WARNING))
#define LOG_ENABLED_SEVERE() if (log_enabled(pMainLog, LOG_LEVEL_SEVERE))
#define LOG_ENABLED_FATAL() if (log_enabled(pMainLog, LOG_LEVEL_FATAL))

#define LOG_EVERYTHING(...) \
  log_write(pMainLog, LOG_LEVEL_EVERYTHING, __VA_ARGS__)
#define LOG_DEBUG(...) log_write(pMainLog, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) log_write(pMainLog, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARNING(...) log_write(pMainLog, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_SEVERE(...) log_write(pMainLog, LOG_LEVEL_SEVERE, __VA_ARGS__)
#define LOG_FATAL(...) log_write(pMainLog, LOG_LEVEL_FATAL, __VA_ARGS__)

typedef struct {
  FILE * pStream;
  uint8_t uLevel;
  uint8_t uOutput;
} SLog;

extern SLog * pMainLog;

// ----- log_create -------------------------------------------------
extern SLog * log_create();
// ----- log_destroy ------------------------------------------------
extern void log_destroy(SLog ** ppLog);
// ----- log_set_level ----------------------------------------------
extern void log_set_level(SLog * pLog, uint8_t uLevel);
// ----- log_get_stream ---------------------------------------------
extern FILE * log_get_stream(SLog * pLog);
// ----- log_set_stream ---------------------------------------------
extern void log_set_stream(SLog * pLog, FILE * pStream);
// ----- log_set_file -----------------------------------------------
extern void log_set_file(SLog * pLog, char * pcFileName);
// ----- log_enabled ------------------------------------------------
extern int log_enabled(SLog * pLog, uint8_t uLevel);
// ----- log_write --------------------------------------------------
extern void log_write(SLog *pLog, uint8_t uLevel, char * pcFormat, ...);
// ----- log_str2level ----------------------------------------------
extern uint8_t log_str2level(char * pcStr);

#endif
