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
#include "operations.h"
#include "test_verbose.h"
#include "params.h"
#include "process.h"
#include "kill.h"
#include <CUnit/Basic.h>
#ifndef _WIN32
# include <wait.h>
# include <unistd.h>
#endif /* ifndef _WIN32 */
#include <stdlib.h>

static _WIN32_OR_POSIX (HANDLE, pid_t) _child = 0;

static void init_operation_query () {
    CU_ASSERT_FATAL (_child == 0);
#ifdef _WIN32
	fprintf (stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
	// TODO
	_child = INVALID_HANDLE_VALUE;
#else /* ifdef _WIN32 */
    _child = fork ();
    if (!_child) {
        sleep (30);
        fprintf (stderr, "Child process %u from %s was not killed\n", getpid (), __FILE__);
        exit (0);
	}
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child != _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, (pid_t)-1));
    CU_ASSERT_FATAL (params_v (2, "query", "./src/unittest") == 0);
}

static void do_operation_query () {
#ifndef _WIN32
    int status;
#endif /* ifndef _WIN32 */
    CU_ASSERT_FATAL (_child != 0);
    // Initial query fails; info file not written
    CU_ASSERT (operation_query () == ESRCH);
    // Write the info file and the query succeeds
    CU_ASSERT (process_save (_child) == 0);
    CU_ASSERT (operation_query () == 0);
    // Kill the child and the query fails
    kill_process (_child);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (_child, 5000) == WAIT_OBJECT_0);
	CloseHandle (_child);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
#endif /* ifdef _WIN32 */
    _child = 0;
    CU_ASSERT (operation_query () == ESRCH);
    // Tidy up
    CU_ASSERT (process_housekeep () == 0);
}

VERBOSE_AND_QUIET_TEST (operation_query)

int register_tests_query () {
    CU_pSuite pSuite = CU_add_suite ("query", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "operation_query [quiet]", test_operation_query)
     || !CU_add_test (pSuite, "operation_query [verbose]", test_operation_query_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
