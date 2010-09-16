// ==================================================================
// @(#)stream.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel
// @date 17/05/2005
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <libgds/memory.h>
#include <libgds/stream.h>

// -----[ stream_create ]--------------------------------------------
gds_stream_t * stream_create(FILE * file)
{
  gds_stream_t * stream= (gds_stream_t *) MALLOC(sizeof(gds_stream_t));
  stream->level= STREAM_LEVEL_EVERYTHING;
  stream->type= STREAM_TYPE_STREAM;
  stream->stream= file;
  return stream;
}

// -----[ stream_create_file ]-------------------------------------------
gds_stream_t * stream_create_file(const char * filename)
{
  gds_stream_t * stream= NULL;
  FILE * file;

  if ((file= fopen(filename, "w")) == NULL)
    return NULL;

  stream= stream_create(file);
  stream->type= STREAM_TYPE_FILE;
  return stream;
}

// -----[ stream_create_callback ]-----------------------------------
gds_stream_t * stream_create_callback(gds_stream_cb_f callback,
				      void * context)
{
  gds_stream_t * stream= stream_create(stderr);

  stream->type= STREAM_TYPE_CALLBACK;
  stream->callback.callback= callback;
  stream->callback.context= context;
  
  return stream;
}

// -----[ stream_create_proc ]---------------------------------------
gds_stream_t * stream_create_proc(const char * cmd)
{
  FILE * proc;
  gds_stream_t * stream;

  proc= popen(cmd, "w");
  if (proc == NULL)
    return NULL;
  stream= stream_create(proc);
  stream->type= STREAM_TYPE_PROCESS;
  return stream;
}

// -----[ stream_destroy ]-------------------------------------------
void stream_destroy(gds_stream_t ** stream_ref)
{
  gds_stream_t * stream= *stream_ref;

  if (stream == NULL)
    return;

  switch (stream->type) {
  case STREAM_TYPE_FILE:
    if (stream->stream != NULL)
      fclose(stream->stream);
    break;
  case STREAM_TYPE_CMD:
    stream->ops.destroy(stream);
    break;
  case STREAM_TYPE_PROCESS:
    pclose(stream->stream);
    break;
  default:
    break;
  }
  FREE(stream);
  *stream_ref= NULL;
}

// -----[ stream_set_level ]-----------------------------------------
void stream_set_level(gds_stream_t * stream, stream_level_t level)
{
  stream->level= level;
}

// -----[ stream_enabled ]-------------------------------------------
int stream_enabled(gds_stream_t * stream, stream_level_t level)
{
  if (stream == NULL)
    return 0;
  return (level >= stream->level);
}

// -----[ stream_vprintf ]-------------------------------------------
int stream_vprintf(gds_stream_t * stream, const char * format, va_list ap)
{
  char * str= NULL;
  int result= -1;

  if (stream == NULL)
    return 0;

  switch (stream->type) {
  case STREAM_TYPE_STREAM:
  case STREAM_TYPE_FILE:
  case STREAM_TYPE_PROCESS:
    assert(stream->stream != NULL);
    result= vfprintf(stream->stream, format, ap);
    break;

  case STREAM_TYPE_CALLBACK:
    assert(stream->callback.callback != NULL);
#ifdef HAVE_VASPRINTF
    assert(vasprintf(&str, format, ap) >= 0);
#else
    /* Guess who has no vasprintf ? hello Solaris... */
    result= vsnprintf(str, 0, format, ap);
    assert(result >= 0);
    str= malloc(sizeof(char)*(result+1));
    assert(str != NULL);
    assert(vsnprintf(str, result+1, format, ap) >= 0);
#endif /* HAVE_VASPRINTF */
    assert(str != NULL);
    result= stream->callback.callback(stream->callback.context, str);
    free(str);
    break;

  case STREAM_TYPE_CMD:
    result= stream->ops.vprintf(stream, format, ap);
    break;

  default:
    abort();
  }

  return result;
}

// -----[ stream_printf ]--------------------------------------------
int stream_printf(gds_stream_t * stream, const char * format, ...)
{
  va_list ap;
  int result;

  va_start(ap, format);
  result= stream_vprintf(stream, format, ap);
  va_end(ap);
  return result;
}

// -----[ stream_flush ]------------------------------------------------
void stream_flush(gds_stream_t * stream)
{
  if (stream == NULL)
    return;

  switch (stream->type) {
  case STREAM_TYPE_STREAM:
  case STREAM_TYPE_FILE:
  case STREAM_TYPE_PROCESS:
    fflush(stream->stream);
    break;
  case STREAM_TYPE_CALLBACK:
    // TO BE IMPLEMENTED...
    break;
  case STREAM_TYPE_CMD:
    stream->ops.flush(stream);
    break;
  default:
    abort();
  }
}


// -----[ stream_perror ]--------------------------------------------
void stream_perror(gds_stream_t * stream, const char * format, ...)
{
  va_list ap;

  va_start(ap, format);
  if (stream_enabled(stream, STREAM_LEVEL_SEVERE)) {
    stream_printf(stream, format, ap);
    stream_printf(stream, "%s\n", strerror(errno));
  }
  va_end(ap);
}

// -----[ stream_str2level ]------------------------------------------
int stream_str2level(const char * str, stream_level_t * level)
{
  if (strcasecmp(str, "everything"))
    *level= STREAM_LEVEL_EVERYTHING;
  else if (strcasecmp(str, "debug"))
    *level= STREAM_LEVEL_DEBUG;
  else if (strcasecmp(str, "info"))
    *level= STREAM_LEVEL_INFO;
  else if (strcasecmp(str, "warning"))
    *level= STREAM_LEVEL_WARNING;
  else if (strcasecmp(str, "severe"))
    *level= STREAM_LEVEL_SEVERE;
  else
    return -1;
  return 0;
}

/////////////////////////////////////////////////////////////////////
// INITIALIZATION AND FINALIZATION SECTION
/////////////////////////////////////////////////////////////////////

// -----[ Standard log streams ]-------------------------------------
gds_stream_t * gdserr= NULL;
gds_stream_t * gdsout= NULL;
gds_stream_t * gdsdebug= NULL;

// -----[ _stream_init ]---------------------------------------------
/**
 * Initialize "standard" log streams.
 */
void _stream_init()
{
  gdserr= stream_create(stderr);
  gdsout= stream_create(stdout);
  gdsdebug= stream_create(stderr);
}

// -----[ _stream_destroy ]------------------------------------------
/**
 * Destroy "standard" log streams.
 */
void _stream_destroy()
{
  stream_destroy(&gdserr);
  stream_destroy(&gdsout);
  stream_destroy(&gdsdebug);
}
