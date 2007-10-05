// ==================================================================
// @(#)utest.h
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bqu@info.ucl.ac.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// @lastdate 20/07/2007
// ==================================================================
//
// Example definition for a unit test suite:
//
// #define NUM_TESTS 0
// SUnitTest TEST_SUITE[NUM_TESTS]= {
//   {test_basic, "basic use"},
//   {test_advanced, "advanced use"},
// };
//
// ==================================================================


#ifndef __GDS_UTEST_H__
#define __GDS_UTEST_H__

#define TXT_BOLD       "\033[1m"
#define TXT_COL_RED    "\033[31;1m"
#define TXT_COL_GREEN  "\033[32;1m"
#define TXT_COL_YELLOW "\033[33;1m"
#define TXT_DEFAULT    "\033[0m"
#define TXT_POS        "\033[65G"

#define UTEST_SUCCESS 0
#define UTEST_FAILURE -1
#define UTEST_CRASHED -2
#define UTEST_SKIPPED 1

#ifdef __STDC_VERSION__
#if (__STDC_VERSION__ >= 199901L)
#define __VARIADIC_ELLIPSIS__
#endif
#endif

#ifdef __VARIADIC_ELLIPSIS__
#undef ASSERT_RETURN
#define ASSERT_RETURN(TEST, ...) \
  if (!(TEST)) { \
    utest_set_message(__FILE__, __LINE__, __VA_ARGS__);	\
    return UTEST_FAILURE; \
  }
#else
#define ASSERT_RETURN(TEST, args...) \
  if (!(TEST)) { \
    utest_set_message(__FILE__,__LINE__, args);	\
    return UTEST_FAILURE; \
  }
#endif

// -----[ Unit Test function ]-----
/**
 * Should return one of (UTEST_SUCCESS, UTEST_FAILURE, UTEST_SKIPPED)
 */
typedef int (*FUnitTest)();

// -----[ Unit Test ]-----
typedef struct {
  FUnitTest fTest;
  char * pcName;
  int iResult;
  char * pcMessage;
  int iLine;
  char * pcFile;
  double dDuration;
} SUnitTest;

// -----[ Suite of Unit Tests ]-----
typedef struct {
  char * pcName;
  unsigned int uNumTests;
  SUnitTest * acTests;
} SUnitTestSuite;

#ifdef __cplusplus
extern "C" {
#endif 

  // -----[ utest_init ]---------------------------------------------
  void utest_init(int iMaxFail);
  // -----[ utest_done ]---------------------------------------------
  void utest_done();
  // -----[ utest_set_message ]--------------------------------------
  void utest_set_message(const char * pcFile, int iLine,
			 const char * pcFormat, ...);
  // -----[ utest_run_suite ]----------------------------------------
  int utest_run_suite(const char * pcName, SUnitTest * paTests,
		      unsigned int uNumTests);
  // -----[ utest_run_suites ]---------------------------------------
  int utest_run_suites(SUnitTestSuite * paSuites, unsigned int uNumSuites);

  // -----[ utest_set_fork ]-----------------------------------------
  void utest_set_fork();
  // -----[ utest_set_user ]-----------------------------------------
  void utest_set_user(const char * pcUser);
  // -----[ utest_set_project ]--------------------------------------
  void utest_set_project(const char * pcProject,
			 const char * pcVersion);
  // -----[ utest_set_xml_logging ]----------------------------------
  void utest_set_xml_logging(const char * pcFileName);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_UTEST_H__ */
