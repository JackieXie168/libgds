// ==================================================================
// @(#)utest.h
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// $Id$
// ==================================================================
//
// Example definition for a unit test suite:
//
// #define NUM_TESTS 0
// unit_test_t TEST_SUITE[NUM_TESTS]= {
//   {test_basic, "basic use"},
//   {test_advanced, "advanced use"},
// };
//
// utest_run_suite("Suite #1", NUM_TESTS, TEST_SUITE, NULL, NULL);
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

typedef enum {
  UTEST_SUCCESS= 0,
  UTEST_FAILURE= -1,
  UTEST_CRASHED= -2,
  UTEST_SKIPPED= 1
} utest_result_t;

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
typedef int (*unit_test_func)();

// -----[ Unit Test ]-----
typedef struct {
  unit_test_func   test;
  char           * name;
  int              result;
  char           * msg;
  int              line;
  char           * filename;
  double           duration;
} unit_test_t;

// -----[ Suite of Unit Tests ]-----
typedef struct {
  char           * name;
  unsigned int     num_tests;
  unit_test_t    * tests;
  unit_test_func   before;
  unit_test_func   after;
} unit_test_suite_t;

#ifdef __cplusplus
extern "C" {
#endif 

  // -----[ utest_init ]---------------------------------------------
  void utest_init(int max_failures);
  // -----[ utest_done ]---------------------------------------------
  void utest_done();
  // -----[ utest_set_message ]--------------------------------------
  void utest_set_message(const char * filename, int line,
			 const char * format, ...);
  // -----[ utest_run_suite ]----------------------------------------
  int utest_run_suite(const char * suite_name, unit_test_t * tests,
		      unsigned int num_tests,
		      unit_test_func before, unit_test_func after);
  // -----[ utest_run_suites ]---------------------------------------
  int utest_run_suites(unit_test_suite_t * suites,
		       unsigned int num_suites);

  // -----[ utest_set_fork ]-----------------------------------------
  void utest_set_fork();
  // -----[ utest_set_user ]-----------------------------------------
  void utest_set_user(const char * user);
  // -----[ utest_set_project ]--------------------------------------
  void utest_set_project(const char * project,
			 const char * version);
  // -----[ utest_set_xml_logging ]----------------------------------
  void utest_set_xml_logging(const char * filename);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_UTEST_H__ */
