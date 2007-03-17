// ==================================================================
// @(#)utest.c
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @lastdate 17/03/2007
// ==================================================================

#include <utest.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/utsname.h>

#define UTEST_FILE_MAX 2048
static char acTmpFile[UTEST_FILE_MAX];
static int iTmpLine;
#define UTEST_MESSAGE_MAX 2048
static char acTmpMessage[UTEST_MESSAGE_MAX];
static FILE * pXMLStream= NULL;
static int iNumFailures= 0;
static int iNumTests= 0;
static int iMaxFailures= 0;

// -----[ utest_init ]-----------------------------------------------
void utest_init(int iMaxFail)
{
  iNumFailures= 0;
  iNumTests= 0;
  iMaxFailures= iMaxFail;
  pXMLStream= NULL;
}

// ----[ utest_done ]------------------------------------------------
void utest_done()
{
  printf("\n==Summary==\n");
  printf("  FAILURES=%d / TESTS=%d\n", iNumFailures, iNumTests);
  printf("\n");
  
  if (pXMLStream != NULL) {
    fprintf(pXMLStream, "  <failures>%i</failures>\n", iNumFailures);
    fprintf(pXMLStream, "  <tests>%i</tests>\n", iNumTests);
    fprintf(pXMLStream, "</testsuites>\n");
    fclose(pXMLStream);
    pXMLStream= NULL;
  }
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
    fprintf(pXMLStream, "<testsuites>\n");
    //fprintf(pXMLStream, "<testsuites xmlns=\"http://libgds.info.ucl.ac.be\">\n");
    if (uname(&name) == 0) {
      fprintf(pXMLStream, "  <uname>\n");
      fprintf(pXMLStream, "    <sysname>%s</sysname>\n", name.sysname);
      fprintf(pXMLStream, "    <nodename>%s</nodename>\n", name.nodename);
      fprintf(pXMLStream, "    <release>%s</release>\n", name.release);
      fprintf(pXMLStream, "    <version>%s</version>\n", name.version);
      fprintf(pXMLStream, "    <machine>%s</machine>\n", name.machine);
      fprintf(pXMLStream, "  </uname>\n");
    }
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
    fprintf(pXMLStream, "  <suite name=\"%s\">\n", pcName);
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
    fprintf(pXMLStream, "    <test name=\"%s\">\n", pTest->pcName);
    fprintf(pXMLStream, "      <result>");
    utest_perror(pXMLStream, pTest->iResult, 0);
    fprintf(pXMLStream, "</result>\n");
    fprintf(pXMLStream, "      <description></description>\n");
    if (pTest->iResult != 0) {
      fprintf(pXMLStream, "      <reason>%s</reason>\n", pTest->pcMessage);
      fprintf(pXMLStream, "      <file></file>\n");
      fprintf(pXMLStream, "      <line></line>\n");
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

    iNumTests++;

    // Run the test
    iTestResult= utest_run_test(pcName, &paTests[uIndex]);
    if ((iTestResult != UTEST_SUCCESS) &&
	(iTestResult != UTEST_SKIPPED)) {
      iResult= -1;
      iNumFailures++;
      if ((iMaxFailures != 0) &&
	  (iNumFailures > iMaxFailures)) {
	break;
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
    if ((iMaxFailures != 0) &&
	(iNumFailures > iMaxFailures)) {
      break;
    }
  }
  return iResult;
}

