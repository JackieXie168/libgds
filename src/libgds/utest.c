// ==================================================================
// @(#)utest.c
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// $Id$
// ==================================================================

#include <utest.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/utsname.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define UTEST_FILE_MAX 1024
static char _filename[UTEST_FILE_MAX];
static int _line;
#define UTEST_MESSAGE_MAX 1024
static char _msg[UTEST_MESSAGE_MAX];
static FILE * xml_stream= NULL;

static struct {
  char * user;
  char * project;
  char * version;
  int    num_failures;
  int    num_skipped;
  int    num_tests;
  int    max_failures;
  struct timeval tp;
  int    with_fork;
} utest;

static void _utest_time_start();
static double _utest_time_stop();


/////////////////////////////////////////////////////////////////////
//
// UNIT TEST - MESSAGE PASSING BETWEEN CHILD PROCESS (UNIT TEST) AND
//             PARENT PROCESS (UNIT TEST SYSTEM)
//
/////////////////////////////////////////////////////////////////////

// -----[ _proc_msg_t ]----------------------------------------------
/**
 * Used to send message from forked unit test to main process.
 * This data structure should be self-contained (no pointers).
 */
typedef struct {
  int    result;
  char   msg[UTEST_MESSAGE_MAX];
  char   filename[UTEST_FILE_MAX];
  int    line;
  double duration;
} _proc_msg_t;

// -----[ _proc_msg_send ]-------------------------------------------
/**
 * Write results from forked test to main process.
 */
static inline void _proc_msg_send(int pipe_desc[2], unit_test_t * test)
{
  _proc_msg_t proc_msg;

  proc_msg.result= test->result;
  if (test->msg != NULL)
    strncpy(proc_msg.msg, test->msg, UTEST_MESSAGE_MAX);
  else
    proc_msg.msg[0]= '\0';
  if (test->filename != NULL)
    strncpy(proc_msg.filename, test->filename, UTEST_FILE_MAX);
  else
    proc_msg.filename[0]= '\0';
  proc_msg.line= test->line;
  proc_msg.duration= test->duration;
  
  if (write(pipe_desc[1], &proc_msg, sizeof(proc_msg)) < 0) {
    perror("write");
    exit(EXIT_FAILURE);
  }
}

// -----[ _proc_msg_recv ]-------------------------------------------
/**
 * Read test results from forked test in main process.
 */
static inline void _proc_msg_recv(int pipe_desc[2], unit_test_t * test,
				  pid_t pid)
{
  _proc_msg_t proc_msg;
  int status;

  while (waitpid(pid, &status, 0) != pid) {
    perror("waitpid");
    exit(EXIT_FAILURE);
  }

  if (status == 0) {
    if (read(pipe_desc[0], &proc_msg, sizeof(proc_msg)) != sizeof(proc_msg)) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    test->result= proc_msg.result;
    test->msg= strdup(proc_msg.msg);
    test->filename= strdup(proc_msg.filename);
    test->line= proc_msg.line;
    test->duration= proc_msg.duration;
  } else {
    test->result= UTEST_CRASHED;
    test->msg= strdup("Test crashed");
    test->filename= NULL;
    test->line= 0;
    test->duration= 0;
  }
}


/////////////////////////////////////////////////////////////////////
//
// UNIT TEST - SETUP
//
/////////////////////////////////////////////////////////////////////

// -----[ utest_init ]-----------------------------------------------
void utest_init(int max_failures)
{
  utest.user= NULL;
  utest.project= NULL;
  utest.version= NULL;
  utest.num_failures= 0;
  utest.num_skipped= 0;
  utest.num_tests= 0;
  utest.max_failures= max_failures;
  utest.with_fork= 0;

  xml_stream= NULL;
}

// -----[ utest_done ]-----------------------------------------------
void utest_done()
{
  printf("\n==Summary==\n");
  printf("  FAILURES=%d / SKIPPED=%d / TESTS=%d\n",
	 utest.num_failures, utest.num_skipped, utest.num_tests);
  printf("\n");
  
  if (xml_stream != NULL) {
    fprintf(xml_stream, "  <failures>%i</failures>\n",
	    utest.num_failures);
    fprintf(xml_stream, "  <tests>%i</tests>\n",
	    utest.num_tests);
    fprintf(xml_stream, "</utest>\n");
    fclose(xml_stream);
    xml_stream= NULL;
  }

  if (utest.user != NULL)
    free(utest.user);
  if (utest.project != NULL)
    free(utest.project);
  if (utest.version != NULL)
    free(utest.version);
}

// -----[ utest_set_fork ]-------------------------------------------
void utest_set_fork()
{
  utest.with_fork= 1;
}

// -----[ utest_set_user ]-------------------------------------------
void utest_set_user(const char * user)
{
  if (utest.user != NULL)
    free(utest.user);

  utest.user= NULL;

  if (user != NULL) {
    utest.user= strdup(user);
    assert(utest.user != NULL);
  }
}

// -----[ utest_set_project ]----------------------------------------
void utest_set_project(const char * project, const char * version)
{
  if (utest.project != NULL)
    free(utest.project);
  if (utest.version != NULL)
    free(utest.version);

  utest.project= NULL;
  utest.version= NULL;

  if (project != NULL) {
    utest.project= strdup(project);
    assert(utest.project != NULL);
  }
  if (version != NULL) {
    utest.version= strdup(version);
    assert(utest.version != NULL);
  }
}

// -----[ _utest_strerror ]------------------------------------------
static inline const char * _utest_strerror(int error)
{
  switch (error) {
  case UTEST_SUCCESS:
    return "SUCCESS";
  case UTEST_SKIPPED:
    return "SKIPPED";
  case UTEST_FAILURE:
    return "FAILURE";
  case UTEST_CRASHED:
    return "CRASHED";
  default:
    return NULL;
  }
}

typedef enum {
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_DEFAULT,
  COLOR_BOLD,
} color_t;

// -----[ _strcolor ]------------------------------------------------
static inline const char * _strcolor(color_t color)
{
  switch (color) {
  case COLOR_RED:
    return "\033[31;1m";
  case COLOR_GREEN:
    return "\033[32;1m";
  case COLOR_YELLOW:
    return "\033[33;1m";
  case COLOR_DEFAULT:
    return "\033[0m";
  case COLOR_BOLD:
    return "\033[1m";
  default:
    return "";
  }
}

// -----[ _pcolor ]--------------------------------------------------
static inline void _pcolor(FILE * stream, color_t color,
			   const char * format, ...)
{
  va_list ap;

  va_start(ap, format);
  //if (isatty(0))
    fprintf(stream, _strcolor(color));
  vfprintf(stream, format, ap);
  //if (isatty(0))
    fprintf(stream, _strcolor(COLOR_DEFAULT));
  va_end(ap);
}

// -----[ _pgotoy ]---------------------------------------------------
static inline void _pgotoy(FILE * stream, int y)
{
  fprintf(stream, "\033[%uG", y);
}

// -----[ _utest_error_color ]---------------------------------------
static inline color_t _utest_error_color(int error)
{
  switch (error) {
  case UTEST_SUCCESS:
    return COLOR_GREEN;
  case UTEST_SKIPPED:
    return COLOR_RED;
  case UTEST_FAILURE:
  case UTEST_CRASHED:
    return COLOR_RED;
  default:
    return COLOR_DEFAULT;
  }
}

// -----[ _utest_perror ]--------------------------------------------
static inline void _utest_perror(FILE * stream, int error, int color)
{
  const char * msg= _utest_strerror(error);
  if (msg != NULL)
    _pcolor(stream, _utest_error_color(error), msg);
  else
    _pcolor(stream, _utest_error_color(error), "??? (%d)", error);
}

// -----[ utest_set_xml_logging ]------------------------------------
void utest_set_xml_logging(const char * filename)
{
  struct utsname name;
  time_t tTime;

  xml_stream= fopen(filename, "w");
  if (xml_stream == NULL) {
    fprintf(stderr, "ERROR: unable to create \"%s\"\n", filename);
    exit(EXIT_FAILURE);
  } else {
    fprintf(xml_stream, "<?xml version=\"1.0\"?>\n");
    fprintf(xml_stream, "<utest>\n");
    //fprintf(xml_stream, "<utest xmlns=\"http://libgds.info.ucl.ac.be\">\n");
    if (uname(&name) >= 0) {
      fprintf(xml_stream, "  <uname>\n");
      fprintf(xml_stream, "    <sysname>%s</sysname>\n", name.sysname);
      fprintf(xml_stream, "    <nodename>%s</nodename>\n", name.nodename);
      fprintf(xml_stream, "    <release>%s</release>\n", name.release);
      fprintf(xml_stream, "    <version>%s</version>\n", name.version);
      fprintf(xml_stream, "    <machine>%s</machine>\n", name.machine);
      fprintf(xml_stream, "  </uname>\n");
    }
    fprintf(xml_stream, "  <info>\n");
    if ((utest.project != NULL) && (utest.version != NULL)) {
      fprintf(xml_stream, "    <project>%s</project>\n", utest.project);
      fprintf(xml_stream, "    <version>%s</version>\n", utest.version);
    }
    if (utest.user != NULL) {
      fprintf(xml_stream, "    <user>%s</user>\n", utest.user);
    }
    if (utest.with_fork) {
      fprintf(xml_stream, "    <option>fork</option>\n");
    }
    fprintf(xml_stream, "  </info>\n");
    tTime= time(NULL);
    fprintf(xml_stream, "<datetime>%s</datetime>\n", ctime(&tTime));
  }
}

// -----[ _utest_write_suite_open ]----------------------------------
/**
 * <suite name="abc">
 *   <tests>
 *     <test name="xyz">
 *     ...
 *     </test>
 *     ...
 *   </tests>
 * </suite>
*/
static inline void _utest_write_suite_open(const char * name)
{
  if (xml_stream != NULL) {
    fprintf(xml_stream, "  <suite>\n");
    fprintf(xml_stream, "    <name>%s</name>\n", name);
  }
}

// -----[ _utest_write_suite_close ]---------------------------------
static inline void _utest_write_suite_close()
{
  if (xml_stream != NULL) {
    fprintf(xml_stream, "  </suite>\n");
  }
}

// ----[ utest_write_test ]------------------------------------------
void utest_write_test(unit_test_t * test)
{
  _pgotoy(stdout, 65);
  _utest_perror(stdout, test->result, 1);
  switch(test->result) {
  case UTEST_SUCCESS:
    printf(" (%1.1fs)", test->duration);
    break;
  case UTEST_FAILURE:
    printf("\n\t-> %s (%s, %d)", test->msg, test->filename, test->line);
    break;
  }
  printf("\n");

  if (xml_stream != NULL) {
    fprintf(xml_stream, "    <test>\n");
    fprintf(xml_stream, "      <name>%s</name>\n", test->name);
    fprintf(xml_stream, "      <code>%d</code>\n", test->result);
    fprintf(xml_stream, "      <result>");
    _utest_perror(xml_stream, test->result, 0);
    fprintf(xml_stream, "</result>\n");
    fprintf(xml_stream, "      <description></description>\n");
    if ((test->result != UTEST_SUCCESS) &&
	(test->result != UTEST_SKIPPED)) {
      fprintf(xml_stream, "      <reason>%s</reason>\n", test->msg);
      fprintf(xml_stream, "      <file>%s</file>\n", test->filename);
      fprintf(xml_stream, "      <line>%d</line>\n", test->line);
      fprintf(xml_stream, "      <function></function>\n");
    } 
    fprintf(xml_stream, "      <duration>%f</duration>\n",
	    test->duration);
    fprintf(xml_stream, "    </test>\n");
  }
}

// -----[ utest_set_message ]----------------------------------------
void utest_set_message(const char * filename,
		       int line,
		       const char * format, ...)
{
  va_list ap;

  snprintf(_filename, UTEST_FILE_MAX, filename);
  _line= line;

  va_start(ap, format);
  vsnprintf(_msg, UTEST_MESSAGE_MAX, format, ap);
  va_end(ap);
}

// -----[ _utest_time_start ]----------------------------------------
static inline void _utest_time_start()
{
  if (gettimeofday(&utest.tp, NULL) < 0) {
    perror("gettimeofday");
    abort();
  }
}

// -----[ _utest_time_stop ]-----------------------------------------
static inline double _utest_time_stop()
{
  struct timeval tp;
  double duration;

  if (gettimeofday(&tp, NULL) < 0) {
    perror("gettimeofday");
    abort();
  }
  
  duration= tp.tv_sec-utest.tp.tv_sec;
  duration+= (((double) tp.tv_usec)-utest.tp.tv_usec)/1000000;

  // NOTE:
  //   The gettimeofday() function is not monotonic, that is it can
  //   go back in time. In this case, the test duration could
  //   eventually be reported as negative. In this case, we consider
  //   that the duration is 0.
  if (duration < 0)
    duration= 0;

  return duration;
}

// -----[ _utest_run_test ]------------------------------------------
/**
 * Runs a single test.
 *
 * ARGUMENTS:
 *   suite's name
 *   test
 *
 * RETURNS:
 *    0 on success
 *   -1 on failure
 */
static inline
int _utest_run_test(const char * name, unit_test_t * test,
		    unit_test_func before, unit_test_func after)
{
  int result= UTEST_SUCCESS;

  _utest_time_start();
  
  if (before != NULL)
    result= before();
  if (result == UTEST_SUCCESS)
    result= test->test();
  if (result == UTEST_SUCCESS)
    if (after != NULL)
      result= after();

  test->duration= _utest_time_stop();
  test->result= result;

  test->msg= _msg;
  test->filename= _filename;
  test->line= _line;

  return test->result;
}

// -----[ _utest_run_forked_test ]-----------------------------------
static inline
int _utest_run_forked_test(const char * name, unit_test_t * test,
			   unit_test_func before, unit_test_func after)
{
  pid_t pid;
  int pipe_desc[2]; // 0:read, 1:write

  // Create pipe
  if (pipe(pipe_desc) != 0) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // Flush output streams
  fflush(stdout);
  fflush(stderr);
  if (xml_stream != NULL)
    fflush(xml_stream);

  // Fork
  pid= fork();
  if (pid < 0) {
    perror("fork");
    close(pipe_desc[0]);
    close(pipe_desc[1]);
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {

  // CHILD CODE (where test is executed)
    close(pipe_desc[0]); // Close read direction
    _utest_run_test(name, test, before, after);
    _proc_msg_send(pipe_desc, test);
    close(pipe_desc[1]); // Close write direction
    exit(EXIT_SUCCESS);

  } else {

    // FATHER CODE
    close(pipe_desc[1]); // Close write direction
    _proc_msg_recv(pipe_desc, test, pid);
    close(pipe_desc[0]); // Close read direction

  }

  return test->result;
}

// -----[ utest_run_suite ]------------------------------------------
int utest_run_suite(const char * name, unit_test_t * tests,
		    unsigned int num_tests,
		    unit_test_func before, unit_test_func after)
{
  int result= 0, test_result;
  unsigned int index;
  unit_test_t * test;

  _utest_write_suite_open(name);

  for (index= 0; index < num_tests; index++) {

    utest.num_tests++;

    test= &tests[index];
    printf("Testing: ");
    _pcolor(stdout, COLOR_BOLD, "%s:%s", name, test->name);

    // Run the test
    if (utest.with_fork) {
      test_result= _utest_run_forked_test(name, test, before, after);
    } else {
      test_result= _utest_run_test(name, test, before, after);
    }

    utest_write_test(test);

    if (test_result != UTEST_SUCCESS) {
      if (test_result == UTEST_SKIPPED) {
	utest.num_skipped++;
      } else {
	result= -1;
	utest.num_failures++;
	if ((utest.max_failures != 0) &&
	    (utest.num_failures > utest.max_failures)) {
	  break;
	}
      }
    }
  }

  _utest_write_suite_close();

  return result;
}

// -----[ utest_run_suites ]---------------------------------------
int utest_run_suites(unit_test_suite_t * suites, unsigned int num_suites)
{
  unsigned int index;
  int result= 0;

  for (index= 0; index < num_suites; index++) {
    result= utest_run_suite(suites[index].name,
			    suites[index].tests,
			    suites[index].num_tests,
			    suites[index].before,
			    suites[index].after);
    if ((utest.max_failures != 0) &&
	(utest.num_failures > utest.max_failures)) {
      break;
    }
  }
  return result;
}

