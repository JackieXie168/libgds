// ==================================================================
// @(#)utest.c
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @lastdate 26/10/2007
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
static char acTmpFile[UTEST_FILE_MAX];
static int iTmpLine;
#define UTEST_MESSAGE_MAX 1024
static char acTmpMessage[UTEST_MESSAGE_MAX];
static FILE * pXMLStream= NULL;

static struct {
  char * pcUser;
  char * pcProject;
  char * pcVersion;
  int iNumFailures;
  int iNumSkipped;
  int iNumTests;
  int iMaxFailures;
  struct timeval tp;
  int iWithFork;
} sUTest;

static void _utest_time_start();
static double _utest_time_stop();

// -----[ utest_init ]-----------------------------------------------
void utest_init(int iMaxFail)
{
  sUTest.pcUser= NULL;
  sUTest.pcProject= NULL;
  sUTest.pcVersion= NULL;
  sUTest.iNumFailures= 0;
  sUTest.iNumSkipped= 0;
  sUTest.iNumTests= 0;
  sUTest.iMaxFailures= iMaxFail;
  sUTest.iWithFork= 0;

  pXMLStream= NULL;
}

// -----[ utest_done ]-----------------------------------------------
void utest_done()
{
  printf("\n==Summary==\n");
  printf("  FAILURES=%d / SKIPPED=%d / TESTS=%d\n",
	 sUTest.iNumFailures, sUTest.iNumSkipped, sUTest.iNumTests);
  printf("\n");
  
  if (pXMLStream != NULL) {
    fprintf(pXMLStream, "  <failures>%i</failures>\n",
	    sUTest.iNumFailures);
    fprintf(pXMLStream, "  <tests>%i</tests>\n",
	    sUTest.iNumTests);
    fprintf(pXMLStream, "</utest>\n");
    fclose(pXMLStream);
    pXMLStream= NULL;
  }

  if (sUTest.pcUser != NULL)
    free(sUTest.pcUser);
  if (sUTest.pcProject != NULL)
    free(sUTest.pcProject);
  if (sUTest.pcVersion != NULL)
    free(sUTest.pcVersion);
}

// -----[ utest_perror ]---------------------------------------------
void utest_perror(FILE * pStream, int iError, int iColor)
{
  if (iColor) {
    switch (iError) {
    case UTEST_SUCCESS:
      fprintf(pStream, ""TXT_COL_GREEN"SUCCESS"TXT_DEFAULT); break;
    case UTEST_SKIPPED:
      fprintf(pStream, ""TXT_COL_RED"NOT-TESTED"TXT_DEFAULT); break;
    case UTEST_FAILURE:
      fprintf(pStream, ""TXT_COL_RED"FAILURE"TXT_DEFAULT); break;
    case UTEST_CRASHED:
      fprintf(pStream, ""TXT_COL_RED"!CRASH!"TXT_DEFAULT); break;
    default:
      fprintf(pStream, ""TXT_COL_RED"UNKNOWN"TXT_DEFAULT); break;
    }

  } else {
    switch (iError) {
    case UTEST_SUCCESS: fprintf(pStream, "Success"); break;
    case UTEST_SKIPPED: fprintf(pStream, "Skipped"); break;
    case UTEST_FAILURE: fprintf(pStream, "Failure"); break;
    case UTEST_CRASHED: fprintf(pStream, "Crashed"); break;
    default: fprintf(pStream, "Unknown"); break;
    }

  }
}

// -----[ utest_set_fork ]-------------------------------------------
void utest_set_fork()
{
  sUTest.iWithFork= 1;
}

// -----[ utest_set_user ]-------------------------------------------
void utest_set_user(const char * pcUser)
{
  if (sUTest.pcUser != NULL)
    free(sUTest.pcUser);

  sUTest.pcUser= NULL;

  if (pcUser != NULL) {
    sUTest.pcUser= strdup(pcUser);
    assert(sUTest.pcUser != NULL);
  }
}

// -----[ utest_set_project ]----------------------------------------
void utest_set_project(const char * pcProject, const char * pcVersion)
{
  if (sUTest.pcProject != NULL)
    free(sUTest.pcProject);
  if (sUTest.pcVersion != NULL)
    free(sUTest.pcVersion);

  sUTest.pcProject= NULL;
  sUTest.pcVersion= NULL;

  if (pcProject != NULL) {
    sUTest.pcProject= strdup(pcProject);
    assert(sUTest.pcProject != NULL);
  }
  if (pcVersion != NULL) {
    sUTest.pcVersion= strdup(pcVersion);
    assert(sUTest.pcVersion != NULL);
  }
}

// -----[ utest_set_xml_logging ]------------------------------------
void utest_set_xml_logging(const char * pcFileName)
{
  struct utsname name;
  time_t tTime;

  pXMLStream= fopen(pcFileName, "w");
  if (pXMLStream == NULL) {
    fprintf(stderr, "ERROR: unable to create \"%s\"\n", pcFileName);
    exit(EXIT_FAILURE);
  } else {
    fprintf(pXMLStream, "<?xml version=\"1.0\"?>\n");
    fprintf(pXMLStream, "<utest>\n");
    //fprintf(pXMLStream, "<utest xmlns=\"http://libgds.info.ucl.ac.be\">\n");
    if (uname(&name) >= 0) {
      fprintf(pXMLStream, "  <uname>\n");
      fprintf(pXMLStream, "    <sysname>%s</sysname>\n", name.sysname);
      fprintf(pXMLStream, "    <nodename>%s</nodename>\n", name.nodename);
      fprintf(pXMLStream, "    <release>%s</release>\n", name.release);
      fprintf(pXMLStream, "    <version>%s</version>\n", name.version);
      fprintf(pXMLStream, "    <machine>%s</machine>\n", name.machine);
      fprintf(pXMLStream, "  </uname>\n");
    }
    fprintf(pXMLStream, "  <info>\n");
    if ((sUTest.pcProject != NULL) && (sUTest.pcVersion != NULL)) {
      fprintf(pXMLStream, "    <project>%s</project>\n", sUTest.pcProject);
      fprintf(pXMLStream, "    <version>%s</version>\n", sUTest.pcVersion);
    }
    if (sUTest.pcUser != NULL) {
      fprintf(pXMLStream, "    <user>%s</user>\n", sUTest.pcUser);
    }
    if (sUTest.iWithFork) {
      fprintf(pXMLStream, "    <option>fork</option>\n");
    }
    fprintf(pXMLStream, "  </info>\n");
    tTime= time(NULL);
    fprintf(pXMLStream, "<datetime>%s</datetime>\n", ctime(&tTime));
  }
}

// -----[ utest_write_suite_open ]-----------------------------------
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
void utest_write_suite_open(const char * pcName)
{
  if (pXMLStream != NULL) {
    fprintf(pXMLStream, "  <suite>\n");
    fprintf(pXMLStream, "    <name>%s</name>\n", pcName);
  }
}

// -----[ utest_write_suite_close ]----------------------------------
void utest_write_suite_close()
{
  if (pXMLStream != NULL) {
    fprintf(pXMLStream, "  </suite>\n");
  }
}

// ----[ utest_write_test ]------------------------------------------
void utest_write_test(unit_test_t * pTest)
{
  printf(""TXT_POS);
  utest_perror(stdout, pTest->iResult, 1);    
  if (pTest->iResult == UTEST_SUCCESS) {
    printf(" (%1.1fs)", pTest->dDuration);
  } else if (pTest->iResult != UTEST_SKIPPED) {
    printf("\n\t-> %s (%s, %d)", pTest->pcMessage, pTest->pcFile,
	   pTest->iLine);
  }
  printf("\n");

  if (pXMLStream != NULL) {
    fprintf(pXMLStream, "    <test>\n");
    fprintf(pXMLStream, "      <name>%s</name>\n", pTest->pcName);
    fprintf(pXMLStream, "      <code>%d</code>\n", pTest->iResult);
    fprintf(pXMLStream, "      <result>");
    utest_perror(pXMLStream, pTest->iResult, 0);
    fprintf(pXMLStream, "</result>\n");
    fprintf(pXMLStream, "      <description></description>\n");
    if ((pTest->iResult != UTEST_SUCCESS) &&
	(pTest->iResult != UTEST_SKIPPED)) {
      fprintf(pXMLStream, "      <reason>%s</reason>\n", pTest->pcMessage);
      fprintf(pXMLStream, "      <file>%s</file>\n", pTest->pcFile);
      fprintf(pXMLStream, "      <line>%d</line>\n", pTest->iLine);
      fprintf(pXMLStream, "      <function></function>\n");
    } 
    fprintf(pXMLStream, "      <duration>%f</duration>\n",
	    pTest->dDuration);
    fprintf(pXMLStream, "    </test>\n");
  }
}

// -----[ utest_set_message ]----------------------------------------
void utest_set_message(const char * pcFile,
		       int iLine,
		       const char * pcFormat, ...)
{
  va_list ap;

  snprintf(acTmpFile, UTEST_FILE_MAX, pcFile);
  iTmpLine= iLine;

  va_start(ap, pcFormat);
  vsnprintf(acTmpMessage, UTEST_MESSAGE_MAX, pcFormat, ap);
}

// -----[ _utest_time_start ]----------------------------------------
static void _utest_time_start()
{
  assert(gettimeofday(&sUTest.tp, NULL) >= 0);
}

// -----[ _utest_time_stop ]-----------------------------------------
static double _utest_time_stop()
{
  struct timeval tp;
  double dDuration;

  assert(gettimeofday(&tp, NULL) >= 0);
  
  /* Note that gettimeofday() is not monotonic, that is it can go
     back in time. In this case, the test duration will eventually
     be reported as negative. */
  dDuration= tp.tv_sec-sUTest.tp.tv_sec;
  dDuration+= (((double) tp.tv_usec)-sUTest.tp.tv_usec)/1000000;
  return dDuration;
}

// -----[ utest_run_test ]-------------------------------------------
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
int utest_run_test(const char * suite_name, unit_test_t * test,
		   FUnitTest before, FUnitTest after)
{
  int result;

  _utest_time_start();
  
  if (before != NULL)
    result= before();
  if (result == UTEST_SUCCESS)
    result= test->fTest();
  if (result == UTEST_SUCCESS)
    if (after != NULL)
      result= after();

  test->dDuration= _utest_time_stop();
  test->iResult= result;

  test->pcMessage= acTmpMessage;
  test->pcFile= acTmpFile;
  test->iLine= iTmpLine;

  return test->iResult;
}

typedef struct {
  int iResult;
  char acMessage[UTEST_MESSAGE_MAX];
  char acFile[UTEST_FILE_MAX];
  int iLine;
  double dDuration;
} unit_test_msg_t;

// -----[ utest_run_forked_test ]------------------------------------
int utest_run_forked_test(const char * suite_name, unit_test_t * test,
			  FUnitTest before, FUnitTest after)
{
  pid_t tPID;
  int iPipeDes[2]; // 0:read, 1:write
  int iStatus;
  unit_test_msg_t msg;

  // Create pipe
  if (pipe(iPipeDes) != 0) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // Flush output streams
  fflush(stdout);
  fflush(stderr);
  if (pXMLStream != NULL)
    fflush(pXMLStream);

  // Fork
  tPID= fork();
  if (tPID < 0) {
    perror("fork");
    close(iPipeDes[0]);
    close(iPipeDes[1]);
    exit(EXIT_FAILURE);

  } else if (tPID == 0) {
    // *** CHILD  CODE ***
    close(iPipeDes[0]); // Close read direction
    
    msg.iResult= utest_run_test(suite_name, test, before, after);

    if (test->pcMessage != NULL)
      strncpy(msg.acMessage, test->pcMessage, UTEST_MESSAGE_MAX);
    else
      msg.acMessage[0]= '\0';
    if (test->pcFile != NULL)
      strncpy(msg.acFile, test->pcFile, UTEST_FILE_MAX);
    else
      msg.acFile[0]= '\0';
    msg.iLine= test->iLine;
    msg.dDuration= test->dDuration;
      
    if (write(iPipeDes[1], &msg, sizeof(msg)) < 0) {
      perror("write");
      exit(EXIT_FAILURE);
    }
    close(iPipeDes[1]);

    exit(EXIT_SUCCESS);
  }

  // *** FATHER CODE ***
  close(iPipeDes[1]); // Close write direction

  while (waitpid(tPID, &iStatus, 0) != tPID) {
    perror("waitpid");
  }

  if (iStatus == 0) {
    if (read(iPipeDes[0], &msg, sizeof(msg)) != sizeof(msg)) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    test->iResult= msg.iResult;
    test->pcMessage= NULL;
    test->pcFile= NULL;
    test->pcMessage= strdup(msg.acMessage);
    test->pcFile= strdup(msg.acFile);
    test->iLine= msg.iLine;
    test->dDuration= msg.dDuration;
  } else {
    test->iResult= UTEST_CRASHED;
    test->pcMessage= strdup("Test crashed");
    test->pcFile= NULL;
    test->iLine= 0;
  }

  close(iPipeDes[0]);

  return test->iResult;
}

// -----[ utest_run_suite ]------------------------------------------
/**
 * Runs a test suite.
 *
 * ARGUMENTS:
 *  suite's name
 *  suite's tests (array)
 *  number of tests
 *
 * RETURNS:
 *    0 on success
 *   -1 on failure (at least one test failed)
 */
int utest_run_suite(const char * suite_name, unit_test_t * tests,
		    unsigned int num_tests,
		    FUnitTest before, FUnitTest after)
{
  int result= 0, test_result;
  unsigned int index;
  unit_test_t * test;

  utest_write_suite_open(suite_name);

  for (index= 0; index < num_tests; index++) {

    sUTest.iNumTests++;

    test= &tests[index];
    printf("Testing: "TXT_BOLD"%s:%s"TXT_DEFAULT, suite_name, test->pcName);

    // Run the test
    if (sUTest.iWithFork) {
      test_result= utest_run_forked_test(suite_name, test, before, after);
    } else {
      test_result= utest_run_test(suite_name, test, before, after);
    }

    utest_write_test(test);

    if (test_result != UTEST_SUCCESS) {
      if (test_result == UTEST_SKIPPED) {
	sUTest.iNumSkipped++;
      } else {
	result= -1;
	sUTest.iNumFailures++;
	if ((sUTest.iMaxFailures != 0) &&
	    (sUTest.iNumFailures > sUTest.iMaxFailures)) {
	  break;
	}
      }
    }
  }

  utest_write_suite_close();

  return result;
}

// -----[ utest_run_suites ]---------------------------------------
int utest_run_suites(unit_test_suite_t * suites, unsigned int num_suites)
{
  unsigned int index;
  int result= 0;

  for (index= 0; index < num_suites; index++) {
    result= utest_run_suite(suites[index].pcName,
			    suites[index].acTests,
			    suites[index].uNumTests,
			    suites[index].before,
			    suites[index].after);
    if ((sUTest.iMaxFailures != 0) &&
	(sUTest.iNumFailures > sUTest.iMaxFailures)) {
      break;
    }
  }
  return result;
}

