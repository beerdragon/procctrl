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
#include "process.h"
#include "params.h"
#include <CUnit/Basic.h>
#include <wait.h>

static void init_operation_stop () {
    CU_ASSERT_FATAL (params_v (3, "stop", "src/example-child-script.sh", "foo") == 0);
}

static void do_operation_stop () {
    pid_t process;
    int i;
    // Process isn't running
    CU_ASSERT (operation_stop () == ESRCH);
    // Start the process and 'stop' should then work
    CU_ASSERT (operation_start () == 0);
    process = process_find ();
    CU_ASSERT (process != 0);
    CU_ASSERT (operation_stop () == 0);
    // Wait for the process to halt
    CU_ASSERT (waitpid (process, &i, 0) == process);
    // Process isn't running
    CU_ASSERT (operation_stop () == ESRCH);
}

VERBOSE_AND_QUIET_TEST (operation_stop)

int register_tests_stop () {
    CU_pSuite pSuite = CU_add_suite ("stop", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "operation_stop [quiet]", test_operation_stop)
     || !CU_add_test (pSuite, "operation_stop [verbose]", test_operation_stop_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
