// ==================================================================
// @(#)utest.h
//
// Generic Data Structures (libgds): unit testing framework.
//
// @author Bruno Quoitin (bruno.quoitin@uclouvain.be)
// @author Sebastien Tandel (standel@info.ucl.ac.be)
// $Id$
// ==================================================================

/**
 * \file
 * Provide a simple, lightweight unit testing framework.
 *
 * A typical unit test is a simple function that conforms to the
 * unit_test_func signature. For example:
 * \code
 * static int unit_test() {
 *   UTEST_ASSERT(value == 5, "value should be 5");
 *   return UTEST_SUCCESS;
 * }
 * \endcode
 *
 * A typical unit test suite is defined as follows. This is an array
 * of unit_test_t elements, each specifying a unit test (function +
 * name).
 * \code
 * #define NUM_TESTS 2
 * unit_test_t TEST_SUITE[NUM_TESTS]= {
 *   {test_basic, "basic use"},
 *   {test_advanced, "advanced use"},
 * };
 * \endcode
 *
 * Then, the test suite can be executed as follows. Note that the
 * unit test framework will fork each unit test in a separate
 * process.
 * \code
 * utest_init(0);
 * utest_set_fork();
 * utest_set_user("foo");
 * utest_set_project("", "0.9a");
 * utest_run_suite("Suite #1", NUM_TESTS, TEST_SUITE, NULL, NULL);
 * utest_done();
 * \endcode
*/

#ifndef __GDS_UTEST_H__
#define __GDS_UTEST_H__

/**
 * These are the possible unit test results.
 */
typedef enum {
  /** The test succeeded. */
  UTEST_SUCCESS= 0,
  /** The test failed. */
  UTEST_FAILURE= -1,
  /** The test crashed (in 'forked' mode only). */
  UTEST_CRASHED= -2,
  /** The test was not executed. */
  UTEST_SKIPPED= 1
} utest_result_t;

#ifdef __STDC_VERSION__
#if (__STDC_VERSION__ >= 199901L)
#define __VARIADIC_ELLIPSIS__
#endif
#endif

// -----[ UTEST_ASSERT ]---------------------------------------------
/**
 * Perform a conditional test.
 *
 * The macro tests a user provided condition. If the test fails (i.e.
 * the user condition evaluates to 0), the macro sets the current
 * unit test error message and returns an error value (really returns
 * the unit test function). If the test succeeds (i.e. the user
 * condition does not evaluate to 0), the macro does  nothing.
 *
 * \warning
 * This macros should only be used in unit tests called by the unit
 * test framework..
 */
#ifdef __VARIADIC_ELLIPSIS__
#undef UTEST_ASSERT
#define UTEST_ASSERT(TEST, ...) \
  if (!(TEST)) { \
    utest_set_message(__FILE__, __LINE__, __VA_ARGS__);	\
    return UTEST_FAILURE; \
  }
#else
#define UTEST_ASSERT(TEST, args...) \
  if (!(TEST)) { \
    utest_set_message(__FILE__,__LINE__, args);	\
    return UTEST_FAILURE; \
  }
#endif

// -----[ Unit test function ]---------------------------------------
/**
 * This is the prototype of a typical unit test function. The
 * function takes no argument and it returns the test's result. The
 * result must be one of UTEST_SUCCESS, UTEST_FAILURE and
 * UTEST_SKIPPED.
 *
 * \warning
 * If the unit test system is configured to fork each unit test,
 * this unit test will be ran in a separate process.
 *
 * \see utest_set_fork
 * \see utest_result_t
 */
typedef int (*unit_test_func)();

// -----[ Unit test ]-----
/**
 * Definition of a unit test.
 */
typedef struct {
  /** Unit test function. */
  unit_test_func   test;
  /** Unit test name. */
  char           * name;
  /** \internal Unit test result (set by system). */
  int              result;
  /** \internal Error/warning message. */
  char           * msg;
  /** \internal Error line (in source file). */
  int              line;
  /** \internal Error file (source file). */
  char           * filename;  
  /** \internal Test duration (in microseconds). */
  double           duration;
} unit_test_t;

// -----[ Suite of unit tests ]-----
/**
 * Definition of a unit test suite (set of tests).
 */
typedef struct {
  /** Name of the suite. */
  char           * name;      
  /** Number of tests. */
  unsigned int     num_tests;
  /** Pointer to array of tests. */
  unit_test_t    * tests;
  /** Optional initialization function. */
  unit_test_func   before;
  /** Optional suite finalization function. */
  unit_test_func   after;
} unit_test_suite_t;

#ifdef __cplusplus
extern "C" {
#endif 

  // -----[ utest_init ]---------------------------------------------
  /**
   * Initialize unit testing.
   *
   * This function must be called exactly once before any unit test
   * function is called.
   *
   * \param max_failures specifies the maximum number of failures
   *                     accepted before processing skips further
   *                     unit tests.
   */
  void utest_init(int max_failures);

  // -----[ utest_done ]---------------------------------------------
  /**
   * Finalize unit testing.
   *
   * This function must be called exactly once after unit test
   * functions have been called.
   */
  void utest_done();

  // -----[ utest_set_fork ]-----------------------------------------
  /**
   * Configure the unit test system to run (fork).
   *
   * If this function is called, each unit test will be executed in a
   * separate process (using fork). This is a better way to provide
   * isolation between unit tests. This also avoids that crashing
   * tests stop the processing of subsequent tests.
   */
  void utest_set_fork();

  // -----[ utest_run_suite ]----------------------------------------
  /**
   * Run a single suite of unit tests.
   *
   * Run all the unit tests in the suite sequentially. Optional
   * initialization (before) and finalization (after) functions for
   * the test suite can be provided. These functions will be called
   * before/after each individual unit test.
   *
   * \param name      is the name of the suite (used for reporting).
   * \param tests     is an array of unit tests.
   * \param num_tests is the number of unit tests in \p tests.
   * \param before    optional initialization function (can be NULL).
   * \param after     optional finalization function (can be NULL).
   */
  int utest_run_suite(const char * name, unit_test_t * tests,
		      unsigned int num_tests,
		      unit_test_func before, unit_test_func after);

  // -----[ utest_run_suites ]---------------------------------------
  /**
   * Run a set of unit tests suites.
   *
   * Run all the unit test suites sequentially.
   *
   * \param suites     is an array of test suites.
   * \param num_suites is the number of suites in \p suites.
   */
  int utest_run_suites(unit_test_suite_t * suites,
		       unsigned int num_suites);

  // -----[ utest_set_message ]--------------------------------------
  /**
   * Associate a message (error/warning) with the current unit test.
   *
   * \attention
   * You should probably not use this function directly. Use the
   * macro UTEST_ASSERT() instead.
   *
   * \param filename is the name of the file associated to the
   *                 message (__FILE__ for example).
   * \param line     is the line number associated to the message
   *                 (__LINE__ for example).
   * \param format   is the message format specifier (same as for
   *                 stdio's printf).
   * \param ...      is a variable list of arguments.
   */
  void utest_set_message(const char * filename, int line,
			 const char * format, ...);


  ///////////////////////////////////////////////////////////////////
  // UNIT TEST - REPORTING OPTIONS
  ///////////////////////////////////////////////////////////////////

  // -----[ utest_set_user ]-----------------------------------------
  /**
   * Configure the user name of the person who has run the tests.
   * This is used for the unit test report only. Calling this
   * function is optional.
   *
   * \param user is the user name.
   */
  void utest_set_user(const char * user);

  // -----[ utest_set_project ]--------------------------------------
  /**
   * Configure the project name and project version.
   * This is used for the unit test report only. Calling this
   * function is optional.
   *
   * \param project is the project name.
   * \param version is the project version.
   */
  void utest_set_project(const char * project,
			 const char * version);

  // -----[ utest_set_xml_logging ]----------------------------------
  /**
   * Configure the unit test framework for XML reporting.
   *
   * Tell the unit test system to report unit tests messages and
   * results in a XML file. This is used for reporting only. Calling
   * this function is optional.
   *
   * \param filename is the name of the XML output file.
   */
  void utest_set_xml_logging(const char * filename);

#ifdef __cplusplus
}
#endif

#endif /* __GDS_UTEST_H__ */
