// ==================================================================
// @(#)log.h
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be), Sebastien Tandel
// @date 17/05/2005
// @lastdate 03/03/2006
// ==================================================================

#ifndef __GDS_LOG_H__
#define __GDS_LOG_H__

#include <stdio.h>
#include <libgds/types.h>

// -----[ ELogLevel ]----------------------------------------------
typedef enum { LOG_LEVEL_EVERYTHING,
	       LOG_LEVEL_DEBUG,
	       LOG_LEVEL_INFO,
	       LOG_LEVEL_WARNING,
	       LOG_LEVEL_SEVERE,
	       LOG_LEVEL_FATAL,
	       LOG_LEVEL_MAX } ELogLevel;

// -----[ ELogType ]-------------------------------------------------
typedef enum { LOG_TYPE_STREAM,
	       LOG_TYPE_FILE,
	       LOG_TYPE_CALLBACK } ELogType;

// -----[ FLogStreamCallback ]---------------------------------------
/**
 * The FLogStreamCallback function prototype is used to implement
 * arbitrary log streams. The main motivation for defining such a
 * callback is to send the log output to a Java application through
 * the Java Native Interface (JNI).
 */
typedef void (*FLogStreamCallback)(void * pContext, char * pcOutput);

// -----[ SLogCallback ]---------------------------------------------
typedef struct {
  FLogStreamCallback fCallback;
  void * pContext;
} SLogCallback;

// -----[ SLogStream ]-----------------------------------------------
/**
 * The SLogStream data structure hold all the data related to a log
 * stream.
 */
typedef struct {
  ELogType eType;
  ELogLevel eLevel;
  union {
    FILE * pStream;
    SLogCallback sCallback;
  };
} SLogStream;

// -----[ log_create_stream ]----------------------------------------
extern SLogStream * log_create_stream(FILE * pStream);
// -----[ log_create_file ]------------------------------------------
extern SLogStream * log_create_file(char * pcFileName);
// -----[ log_create_callback ]--------------------------------------
extern SLogStream * log_create_callback(FLogStreamCallback fCallback,
					void * pContext);
// ----- log_destroy ------------------------------------------------
extern void log_destroy(SLogStream ** ppLogStream);
// ----- log_set_level ----------------------------------------------
extern void log_set_level(SLogStream * pLogStream, ELogLevel eLevel);
// -----[ log_str2level ]--------------------------------------------
extern ELogLevel log_str2level(char * pcStr);

// ----- log_get_stream ---------------------------------------------
// SHOULD NOT BE USED extern FILE * log_get_stream(SLog * pLog);
// ----- log_set_stream ---------------------------------------------
// SHOULD NOT BE USED extern void log_set_stream(SLog * pLog, FILE * pStream);
// ----- log_set_file -----------------------------------------------
// SHOULD NOT BE USED extern void log_set_file(SLog * pLog, char * pcFileName);

// ----- log_enabled ------------------------------------------------
extern int log_enabled(SLogStream * pLogStream, ELogLevel eLevel);
// ----- log_printf --------------------------------------------------
extern void log_printf(SLogStream * pLogStream, char * pcFormat, ...);
// -----[ log_flush ]------------------------------------------------
extern void log_flush(SLogStream * pLogStream);
// ----- log_perror --------------------------------------------------
extern void log_perror(SLogStream * pLogStream, char * pcFormat, ...);


/////////////////////////////////////////////////////////////////////
//
// INITIALIZATION AND FINALIZATION FUNCTIONS
//
/////////////////////////////////////////////////////////////////////

// -----[ standard log streams ]-------------------------------------
/**
 * Definition of "standard" log streams. These log streams are
 * initialized to send all their output.
 *
 * - pLogErr is initialized to send its output on stderr.
 * - pLogOut is initialized to send its output on stdout.
 * - pLogDebug is initialized to send its output on stderr.
 */
extern SLogStream * pLogErr;
extern SLogStream * pLogOut;
extern SLogStream * pLogDebug;

// ----- _log_init --------------------------------------------------
extern void _log_init();
// ----- _log_destroy -----------------------------------------------
extern void _log_destroy();


/////////////////////////////////////////////////////////////////////
//
// LOG MACROS
//
/////////////////////////////////////////////////////////////////////

/* Note about variadic macros:
 * ---------------------------
 * Old versions of CPP, the C preprocessor, only support named
 * variable argument (args...).
 *
 * However, newer C99 conforming applications may only support
 * __VA_ARGS__.
 */
#ifdef __STDC_VERSION__
#if (__STDC_VERSION__ >= 199901L)
#define __VARIADIC_ELLIPSIS__
#endif
#endif

#define LOG_DEBUG_ENABLED(LEVEL) \
  if (log_enabled(pLogDebug, LEVEL))
#define LOG_ERR_ENABLED(LEVEL) \
  if (log_enabled(pLogErr, LEVEL))
#define LOG_OUT_ENABLED(LEVEL) \
  if (log_enabled(pLogOut, LEVEL))

#ifdef __VARIADIC_ELLIPSIS__

#define LOG_DEBUG(LEVEL, ...) \
  if (log_enabled(pLogDebug, LEVEL)) log_printf(pLogDebug, __VA_ARGS__)
#define LOG_ERR(LEVEL, ...) \
  if (log_enabled(pLogErr, LEVEL)) log_printf(pLogErr, __VA_ARGS__)
#define LOG_OUT(LEVEL, ...) \
  if (log_enabled(pLogOut, LEVEL)) log_printf(pLogOut, __VA_ARGS__)

# else // __VARIADIC_ELLIPSIS__

#define LOG_DEBUG(LEVEL, args...) \
  if (log_enabled(pLogDebug, LEVEL)) log_printf(pLogDebug, args)
#define LOG_ERR(LEVEL, args...) \
  if (log_enabled(pLogErr, LEVEL)) log_printf(pLogErr, args)
#define LOG_OUT(LEVEL, args...) \
  if (log_enabled(pLogOut, LEVEL)) log_printf(pLogOut, args)

#endif

#endif
