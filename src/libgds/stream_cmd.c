// ==================================================================
// @(#)stream_cmd.c
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @date 25/09/2008
// $Id$
// ==================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <libgds/memory.h>
#include <libgds/stream_cmd.h>
#include <libgds/tokenizer.h>

typedef struct {
  FILE  * stream;
  pid_t   pid;
  char  * cmd;
} _ctx_t;

// -----[ _open ]----------------------------------------------------
static int _open(gds_stream_t * stream)
{
  _ctx_t * ctx= (_ctx_t *) stream->ctx;
  int pipe_desc[2]; // 0: read, 1: write
  gds_tokenizer_t * tokenizer;
  const gds_tokens_t * tokens;
  unsigned int index;
  char ** argv= NULL;

  if (pipe(pipe_desc) < 0) {
    perror("could not create pipe");
    return -1;
  }

  ctx->pid= fork();

  if (ctx->pid < 0) {
    perror("could not fork");
    return -1;
  }

  if (ctx->pid == 0) {

    /* --- Child  process --- */
    close(pipe_desc[1]); // close child's write size
    if (dup2(pipe_desc[0], STDIN_FILENO) < 0) {
      perror("couldn't dup2");
      exit(EXIT_FAILURE);
    }

    tokenizer= tokenizer_create(" ", NULL, NULL);
    tokenizer_run(tokenizer, ctx->cmd);
    tokens= tokenizer_get_tokens(tokenizer);
    // Don't use GDS's malloc here as process will be replaced
    argv= malloc(sizeof(char *)*(tokens_get_num(tokens)+1));
    assert(argv != NULL);
    for (index= 0; index < tokens_get_num(tokens); index++)
      argv[index]= tokens_get_string_at(tokens, index);
    argv[tokens_get_num(tokens)]= NULL;

    if (execvp(argv[0], argv) < 0)
      perror("could not exec");

    free(argv);
    tokenizer_destroy(&tokenizer);

    // execxx should only return if an error occurred
    exit(EXIT_FAILURE);

  }

  /* --- Parent process --- */
  close(pipe_desc[0]); // close parent's read size
  ctx->stream= fdopen(pipe_desc[1], "w");
  if (ctx->stream < 0) {
    perror("could not fdopen");
    return -1;
  }

  return 0;
} 

// -----[ _destroy ]-------------------------------------------------
static void _destroy(gds_stream_t * stream)
{
  _ctx_t * ctx= (_ctx_t *) stream->ctx;
  int status;

  fclose(ctx->stream);

  if (ctx->cmd != NULL)
    free(ctx->cmd);

  if (ctx->pid >= 0)
    waitpid(ctx->pid, &status, 0);
  ctx->pid= -1;
}

// -----[ _flush ]---------------------------------------------------
static int _flush(gds_stream_t * stream)
{
  _ctx_t * ctx= (_ctx_t *) stream->ctx;
  return fflush(ctx->stream);
}

// -----[ _vprintf ]-------------------------------------------------
static int _vprintf(gds_stream_t * stream, const char * format,
		    va_list ap)
{
  _ctx_t * ctx= (_ctx_t *) stream->ctx;
  return vfprintf(ctx->stream, format, ap);
}

// -----[ stream_create_cmd ]----------------------------------------
gds_stream_t * stream_create_cmd(const char * cmd)
{
  gds_stream_t * stream= stream_create(stderr);
  _ctx_t * ctx= MALLOC(sizeof(_ctx_t));

  stream->type= STREAM_TYPE_CMD;
  stream->stream= NULL;

  ctx->pid= -1;
  ctx->stream= NULL;
  ctx->cmd= strdup(cmd);
  stream->ctx= ctx;

  stream->ops.destroy= _destroy;
  stream->ops.flush  = _flush;
  stream->ops.vprintf= _vprintf;

  if (_open(stream) < 0) {
    _destroy(stream);
    FREE(stream);
    return NULL;
  }

  return stream;
}

