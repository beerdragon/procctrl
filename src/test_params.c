/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* ifdef HAVE_CONFIG_H */
#ifdef HAVE_CUNIT_H
#include "test_units.h"
#define MODULE_VAR_CONST volatile const
#include "params.h"
#include "test_verbose.h"
#include <CUnit/Basic.h>
#ifndef _WIN32
# include <unistd.h>
#endif /* ifndef _WIN32 */

static void test_params_d (void) {
    VERBOSE_WATCH_ALL;
    // Expect parameter for d
    CU_ASSERT (params_v (1, "-d") == _WIN32_OR_POSIX (ERROR_INVALID_PARAMETER, EINVAL));
    VERBOSE_STDERR_ONLY;
    // Default is ~/.procctrl (expanded)
    CU_ASSERT (params_v (0) == 0);
#ifdef _WIN32
    CU_ASSERT (data_dir[1] == ':');
    CU_ASSERT (data_dir[2] == '\\');
#else /* ifdef _WIN32 */
    CU_ASSERT (data_dir[0] == '/');
#endif /* ifdef _WIN32 */
    CU_ASSERT (!strcmp (data_dir + strlen (data_dir) - 10, _WIN32_OR_POSIX ("\\", "/") ".procctrl"));
    // Explicit value
    CU_ASSERT (params_v (2, "-d", _WIN32_OR_POSIX ("C:\\foo\\bar\\path", "/foo/bar/path")) == 0);
    CU_ASSERT (!strcmp (data_dir, _WIN32_OR_POSIX ("C:\\foo\\bar\\path", "/foo/bar/path")));
    VERBOSE_SILENT_ALL;
}

static void test_params_H (void) {
    VERBOSE_WATCH_ALL;
    // Expect parameter for H
    CU_ASSERT (params_v (1, "-H") == _WIN32_OR_POSIX (ERROR_INVALID_PARAMETER, EINVAL));
    VERBOSE_STDERR_ONLY;
    // Default is ALL
    CU_ASSERT (params_v (0) == 0);
    CU_ASSERT (housekeep_mode == HOUSEKEEP_FULL);
    // Explicit value (getopt short form)
    CU_ASSERT (params_v (1, "-H0") == 0);
    CU_ASSERT (housekeep_mode == 0);
    VERBOSE_SILENT_ALL;
}

static void test_params_K (void) {
    VERBOSE_WATCH_ALL;
    // Default is local
    CU_ASSERT (params_v (0) == 0);
    CU_ASSERT (global_identifier == 0);
    // Set flag
    CU_ASSERT (params_v (1, "-K") == 0);
    CU_ASSERT (global_identifier != 0);
    VERBOSE_SILENT_ALL;
}

static void test_params_k (void) {
    VERBOSE_WATCH_ALL;
    // Expect parameter for k
    CU_ASSERT (params_v (1, "-k") == _WIN32_OR_POSIX (ERROR_INVALID_PARAMETER, EINVAL));
    VERBOSE_STDERR_ONLY;
    // Default is command line based
    CU_ASSERT (params_v (3, "query", "foo", "bar") == 0);
	CU_ASSERT_FATAL (process_identifier != NULL);
    CU_ASSERT (!strcmp (process_identifier, "foo bar"));
    // Explicit value
    CU_ASSERT (params_v (5, "-k", "Test", "query", "foo", "bar") == 0);
	CU_ASSERT_FATAL (process_identifier != NULL);
    CU_ASSERT (!strcmp (process_identifier, "Test"));
    VERBOSE_SILENT_ALL;
}

static void test_params_P (void) {
    VERBOSE_WATCH_ALL;
    // Expect parameter for P
    CU_ASSERT (params_v (1, "-P") == _WIN32_OR_POSIX (ERROR_INVALID_PARAMETER, EINVAL));
    VERBOSE_STDERR_ONLY;
    // Default is parent ID
    CU_ASSERT (params_v (0) == 0);
#ifdef _WIN32
    CU_ASSERT (parent_process != INVALID_HANDLE_VALUE);
#else /* ifdef _WIN32 */
    CU_ASSERT (parent_process == getppid ());
#endif /* ifdef _WIN32 */
    // Explicit value
    CU_ASSERT (params_v (2, "-P", "1234") == 0);
#ifdef _WIN32
	CU_ASSERT ((parent_process == NULL) || (GetProcessId (parent_process) == 1234));
#else /* ifdef _WIN32 */
    CU_ASSERT (parent_process == 1234);
#endif /* ifdef _WIN32 */
    VERBOSE_SILENT_ALL;
}

static void test_params_p (void) {
    VERBOSE_WATCH_ALL;
    // Default is not to watch
    CU_ASSERT (params_v (0) == 0);
    CU_ASSERT (watch_parent == 0);
    // Set flag
    CU_ASSERT (params_v (1, "-p") == 0);
    CU_ASSERT (watch_parent != 0);
    VERBOSE_SILENT_ALL;
}

static void test_params_v (void) {
    VERBOSE_WATCH_ALL;
    // Default is not verbose
    CU_ASSERT (params_v (0) == 0);
    CU_ASSERT (verbose == 0);
    VERBOSE_SILENT_ALL;
    // Set flag
    CU_ASSERT (params_v (1, "-v") == 0);
    CU_ASSERT (verbose != 0);
    VERBOSE_STDOUT_ONLY;
}

static void test_params_inval (void) {
    VERBOSE_WATCH_ALL;
    // Unrecognised option
    CU_ASSERT (params_v (1, "-x") == _WIN32_OR_POSIX (ERROR_INVALID_PARAMETER, EINVAL));
    VERBOSE_STDERR_ONLY;
    CU_ASSERT (params_v (1, "-\n") == _WIN32_OR_POSIX (ERROR_INVALID_PARAMETER, EINVAL));
    VERBOSE_STDERR_ONLY;
}

int register_tests_params () {
    CU_pSuite pSuite = CU_add_suite ("params", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "params [d]", test_params_d)
     || !CU_add_test (pSuite, "params [H]", test_params_H)
     || !CU_add_test (pSuite, "params [K]", test_params_K)
     || !CU_add_test (pSuite, "params [k]", test_params_k)
     || !CU_add_test (pSuite, "params [P]", test_params_P)
     || !CU_add_test (pSuite, "params [p]", test_params_p)
     || !CU_add_test (pSuite, "params [v]", test_params_v)
     || !CU_add_test (pSuite, "params [?]", test_params_inval)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
