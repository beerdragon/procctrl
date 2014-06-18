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
#include "params.h"
#include "test_verbose.h"
#include "process.h"
#include "kill.h"
#include <CUnit/Basic.h>
#ifndef _WIN32
# include <wait.h>
# include <unistd.h>
#endif /* ifndef _WIN32 */
#include <stdlib.h>

static _WIN32_OR_POSIX (HANDLE, pid_t) _parent = 0;

static void spawn_parent () {
    CU_ASSERT (_parent == 0);
#ifdef _WIN32
	fprintf (stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
	// TODO: Spawn the process
	_parent = INVALID_HANDLE_VALUE;
#else /* ifdef _WIN32 */
    _parent = fork ();
    if (_parent) {
        CU_ASSERT (_parent != (pid_t)-1);
    } else {
        sleep (30);
        fprintf (stderr, "Child process %u from %s was NOT killed\n", getpid (), __FILE__);
        exit (0);
    }
#endif /* ifdef _WIN32 */
}

static void init_operation_start_spawn () {
    CU_ASSERT_FATAL (params_v (3, "start", _WIN32_OR_POSIX ("src\\example-child-script.bat", "src/example-child-script.sh"), "foo") == 0);
}

static void do_operation_start_spawn () {
    _WIN32_OR_POSIX (HANDLE, pid_t) process;
#ifndef _WIN32
    int status;
#endif /* ifndef _WIN32 */
    // First call will start the process
    CU_ASSERT (operation_start () == 0);
    // Second call will fail
    CU_ASSERT (operation_start () == _WIN32_OR_POSIX (ERROR_ALREADY_EXISTS, EALREADY));
    // Kill the process to tidy up
    process = process_find ();
    CU_ASSERT_FATAL (process != 0);
    kill_process (process);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (process, 5000) == WAIT_OBJECT_0);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (process, &status, 0) == process);
#endif /* ifdef _WIN32 */
    CU_ASSERT (process_housekeep () == 0);
}

VERBOSE_AND_QUIET_TEST (operation_start_spawn)

static void init_operation_start_watchdog () {
    char tmp[32];
    CU_ASSERT_FATAL (_parent == 0);
    spawn_parent ();
    CU_ASSERT_FATAL (snprintf (tmp, sizeof (tmp), "%u", _WIN32_OR_POSIX (GetProcessId (_parent), _parent)) < sizeof (tmp));
    CU_ASSERT_FATAL (params_v (6, "-p", "-P", tmp, "start", "src/example-child-script.sh", "foo") == 0);
}

static void do_operation_start_watchdog () {
    int i;
    CU_ASSERT_FATAL (_parent != 0);
    CU_ASSERT (process_find () == 0);
    // Start the child process
    CU_ASSERT (operation_start () == 0);
    CU_ASSERT (process_find () != 0);
    // Kill the parent and the watchdog will kick in
    kill_process (_parent);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (_parent, 5000) == WAIT_OBJECT_0);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_parent, &i, 0) == _parent);
#endif /* ifdef _WIN32 */
    _parent = 0;
    for (i = 0; (i < 30) && (process_find () != 0); i++) {
        _WIN32_OR_POSIX (Sleep, sleep) (1);
    }
    CU_ASSERT (process_find () == 0);
}

VERBOSE_AND_QUIET_TEST (operation_start_watchdog)

int register_tests_start () {
    CU_pSuite pSuite = CU_add_suite ("start", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "operation_start [spawn,quiet]", test_operation_start_spawn)
     || !CU_add_test (pSuite, "operation_start [spawn,verbose]", test_operation_start_spawn_verbose)
     || !CU_add_test (pSuite, "operation_start [watchdog,quiet]", test_operation_start_watchdog)
     || !CU_add_test (pSuite, "operation_start [watchdog,verbose]", test_operation_start_watchdog_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
