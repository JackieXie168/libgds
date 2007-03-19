// ==================================================================
// @(#)utest.c
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @lastdate 19/03/2007
// ==================================================================

#include <utest.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/utsname.h>

#define UTEST_FILE_MAX 2048
static char acTmpFile[UTEST_FILE_MAX];
static int iTmpLine;
#define UTEST_MESSAGE_MAX 2048
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
} sUTest;

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

  pXMLStream= NULL;
}

// ----[ utest_done ]------------------------------------------------
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
    default:
      fprintf(pStream, ""TXT_COL_RED"UNKNOWN"TXT_DEFAULT); break;
    }

  } else {
    switch (iError) {
    case UTEST_SUCCESS: fprintf(pStream, "Success"); break;
    case UTEST_SKIPPED: fprintf(pStream, "Skipped"); break;
    case UTEST_FAILURE: fprintf(pStream, "Failure"); break;
    default: fprintf(pStream, "Unknown"); break;
    }

  }
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

  pXMLStream= fopen(pcFileName, "w");
  if (pXMLStream == NULL) {
    fprintf(stderr, "ERROR: unable to create \"%s\"\n", pcFileName);
    exit(EXIT_FAILURE);
  } else {
    fprintf(pXMLStream, "<?xml version=\"1.0\"?>\n");
    fprintf(pXMLStream, "<utest>\n");
    //fprintf(pXMLStream, "<utest xmlns=\"http://libgds.info.ucl.ac.be\">\n");
    if (uname(&name) == 0) {
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
    fprintf(pXMLStream, "  </info>\n");
    //fprintf(pXMLStream, "<datetime></datetime>\n");
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
void utest_write_test(SUnitTest * pTest)
{
  printf(""TXT_POS);
  utest_perror(stdout, pTest->iResult, 1);    
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
    fprintf(pXMLStream, "      <duration></duration>\n");
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
int utest_run_test(const char * pcName, SUnitTest * pTest)
{
  printf("Testing: "TXT_BOLD"%s:%s"TXT_DEFAULT, pcName, pTest->pcName);
  pTest->iResult= pTest->fTest();
  pTest->pcMessage= acTmpMessage;
  pTest->pcFile= acTmpFile;
  pTest->iLine= iTmpLine;

  utest_write_test(pTest);
  return pTest->iResult;
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
int utest_run_suite(const char * pcName, SUnitTest * paTests,
		    unsigned int uNumTests)
{
  int iResult= 0, iTestResult;
  unsigned int uIndex;

  utest_write_suite_open(pcName);

  for (uIndex= 0; uIndex < uNumTests; uIndex++) {

    sUTest.iNumTests++;

    // Run the test
    iTestResult= utest_run_test(pcName, &paTests[uIndex]);
    if (iTestResult != UTEST_SUCCESS) {
      if (iTestResult == UTEST_SKIPPED) {
	sUTest.iNumSkipped++;
      } else {
	iResult= -1;
	sUTest.iNumFailures++;
	if ((sUTest.iMaxFailures != 0) &&
	    (sUTest.iNumFailures > sUTest.iMaxFailures)) {
	  break;
	}
      }
    }
  }

  utest_write_suite_close();

  return iResult;
}

// -----[ utest_run_suites ]---------------------------------------
int utest_run_suites(SUnitTestSuite * paSuites, unsigned int uNumSuites)
{
  unsigned int uIndex;
  int iResult= 0;

  for (uIndex= 0; uIndex < uNumSuites; uIndex++) {
    iResult= utest_run_suite(paSuites[uIndex].pcName,
			     paSuites[uIndex].acTests,
			     paSuites[uIndex].uNumTests);
    if ((sUTest.iMaxFailures != 0) &&
	(sUTest.iNumFailures > sUTest.iMaxFailures)) {
      break;
    }
  }
  return iResult;
}

