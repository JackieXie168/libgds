// ==================================================================
// @(#)stream.h
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel
// @date 17/05/2005
// $Id: log.h 273 2008-08-21 10:00:30Z bquoitin $
// ==================================================================

#ifndef __GDS_STREAM_H__
#define __GDS_STREAM_H__

#include <stdarg.h>
#include <stdio.h>

#include <libgds/types.h>

// -----[ stream_level_t ]-------------------------------------------
typedef enum {
  STREAM_LEVEL_EVERYTHING,
  STREAM_LEVEL_DEBUG,
  STREAM_LEVEL_INFO,
  STREAM_LEVEL_WARNING,
  STREAM_LEVEL_SEVERE,
  STREAM_LEVEL_FATAL,
  STREAM_LEVEL_MAX
} stream_level_t;

// -----[ stream_type_t ]--------------------------------------------
typedef enum {
  STREAM_TYPE_STREAM,
  STREAM_TYPE_FILE,
  STREAM_TYPE_CALLBACK,
  STREAM_TYPE_CMD,
} stream_type_t ;

// -----[ FLogStreamCallback ]---------------------------------------
/**
 * The FLogStreamCallback function prototype is used to implement
 * arbitrary log streams. The main motivation for defining such a
 * callback is to send the log output to a Java application through
 * the Java Native Interface (JNI).
 */
typedef int (*FLogStreamCallback)(void * ctx, char * output);

// -----[ stream_callback_t ]----------------------------------------
typedef struct {
  FLogStreamCallback callback;
  void * context;
} stream_callback_t;

struct gds_stream_t;

// -----[ gds_stream_ops_t ]-----------------------------------------
typedef struct {
  void (*destroy)(struct gds_stream_t * stream);
  int  (*flush)  (struct gds_stream_t * stream);
  int  (*vprintf)(struct gds_stream_t * stream, const char * format,
		  va_list ap);
} gds_stream_ops_t;

// -----[ gds_stream_t ]---------------------------------------------
/**
 * The gds_stream_t data structure hold all the data related to a log
 * stream.
 */
typedef struct gds_stream_t {
  stream_type_t      type;
  stream_level_t     level;
  gds_stream_ops_t   ops;
  void             * ctx;
  union {
    FILE              * stream;
    stream_callback_t   callback;
  };
} gds_stream_t;

// -----[ standard log streams ]-------------------------------------
/**
 * Definition of "standard" log streams. These log streams are
 * initialized to send all their output.
 *
 * - gdserr is initialized to send its output on stderr.
 * - gdsout is initialized to send its output on stdout.
 * - gdsdebug is initialized to send its output on stderr.
 */
extern gds_stream_t * gdserr;
extern gds_stream_t * gdsout;
extern gds_stream_t * gdsdebug;

#ifdef __cplusplus
extern "C" {
#endif

  // -----[ stream_create ]------------------------------------------
  gds_stream_t * stream_create(FILE * stream);
  // -----[ stream_create_file ]-------------------------------------
  gds_stream_t * stream_create_file(const char * filename);
  // -----[ stream_create_callback ]---------------------------------
  gds_stream_t * stream_create_callback(FLogStreamCallback callback,
					void * context);
  // -----[ stream_destroy ]-----------------------------------------
  void stream_destroy(gds_stream_t ** stream_ref);
  // -----[ stream_set_level ]---------------------------------------
  void stream_set_level(gds_stream_t * stream, stream_level_t level);
  // -----[ stream_str2level ]---------------------------------------
  stream_level_t stream_str2level(char * str);
  // -----[ stream_enabled ]-----------------------------------------
  int stream_enabled(gds_stream_t * stream, stream_level_t level);
  // -----[ stream_printf ]------------------------------------------
  int stream_printf(gds_stream_t * log, const char * format, ...);
  // -----[ stream_vprintf ]-----------------------------------------
  int stream_vprintf(gds_stream_t * log, const char * format, va_list ap);
  // -----[ stream_flush ]-------------------------------------------
  void stream_flush(gds_stream_t * stream);
  // -----[ stream_perror ]------------------------------------------
  void stream_perror(gds_stream_t * stream, const char * format, ...);

  ///////////////////////////////////////////////////////////////////
  // INITIALIZATION AND FINALIZATION FUNCTIONS
  ///////////////////////////////////////////////////////////////////

  // -----[ _stream_init ]-------------------------------------------
  void _stream_init();
  // -----[ _stream_destroy ]----------------------------------------
  void _stream_destroy();

#ifdef __cplusplus
}
#endif



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

#define STREAM_DEBUG_ENABLED(LEVEL) \
  if (stream_enabled(gdsdebug, LEVEL))
#define STREAM_ERR_ENABLED(LEVEL) \
  if (stream_enabled(gdserr, LEVEL))
#define STREAM_OUT_ENABLED(LEVEL) \
  if (stream_enabled(gdsout, LEVEL))

#ifdef __VARIADIC_ELLIPSIS__

#define STREAM_DEBUG(LEVEL, ...) \
  if (stream_enabled(gdsdebug, LEVEL)) stream_printf(gdsdebug, __VA_ARGS__)
#define STREAM_ERR(LEVEL, ...) \
  if (stream_enabled(logerr, LEVEL)) stream_printf(gdserr, __VA_ARGS__)
#define STREAM_OUT(LEVEL, ...) \
  if (stream_enabled(logout, LEVEL)) stream_printf(gdsout, __VA_ARGS__)

# else // __VARIADIC_ELLIPSIS__

#define STREAM_DEBUG(LEVEL, args...) \
  if (stream_enabled(gdsdebug, LEVEL)) stream_printf(gdsdebug, args)
#define STREAM_ERR(LEVEL, args...) \
  if (stream_enabled(gdserr, LEVEL)) stream_printf(gdserr, args)
#define STREAM_OUT(LEVEL, args...) \
  if (stream_enabled(gdsout, LEVEL)) stream_printf(gdsout, args)

#endif

#endif /* __GDS_STREAM_H__ */
