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
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>

static pid_t _child = 0;

static void init_operation_query () {
    CU_ASSERT_FATAL (_child == 0);
    _child = fork ();
    if (_child) {
        CU_ASSERT (_child != (pid_t)-1);
        CU_ASSERT_FATAL (params_v (2, "query", "./src/unittest") == 0);
    } else {
        sleep (30);
        fprintf (stderr, "Child process %u from %s was not killed\n", getpid (), __FILE__);
        exit (0);
    }
}

static void do_operation_query () {
    int status;
    CU_ASSERT_FATAL (_child != 0);
    // Initial query fails; info file not written
    CU_ASSERT (operation_query () == ESRCH);
    // Write the info file and the query succeeds
    CU_ASSERT (process_save (_child) == 0);
    CU_ASSERT (operation_query () == 0);
    // Kill the child and the query fails
    kill_process (_child);
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
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
