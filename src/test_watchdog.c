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
#include "watchdog.h"
#include "params.h"
#include "test_verbose.h"
#include "kill.h"
#include <CUnit/Basic.h>
#include <wait.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

static pid_t _child = 0;

int _watchdog0_v (int count, va_list pids);

static int _watchdog0 (int count, ...) {
    int i;
    va_list pids;
    va_start (pids, count);
    i = _watchdog0_v (count, pids);
    va_end (pids);
    return i;
}

static void spawn_child () {
    CU_ASSERT_FATAL (_child == 0);
    _child = fork ();
    if (_child) {
        CU_ASSERT (_child != (pid_t)-1);
    } else {
        sleep (30);
        fprintf (stderr, "Child process %u from %s was NOT killed\n", getpid (), __FILE__);
        exit (0);
    }
}

static void init_watchdog_parent () {
    CU_ASSERT (params_v (0) == 0);
    spawn_child ();
}

static void do_watchdog_parent () {
    int status;
    CU_ASSERT_FATAL (_child != 0);
    // Poll the watchdog; both processes are running
    CU_ASSERT (_watchdog0 (2, getppid (), _child) == -1);
    // Call the blocking watchdog with an invalid parent PID
    CU_ASSERT (watchdog (2, 0, _child) == 0);
    // Kill the child process that was started
    kill_process (_child);
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
    _child = 0;
}

VERBOSE_AND_QUIET_TEST (watchdog_parent)

static void init_watchdog_child () {
    params_v (0);
    spawn_child ();
}

static void do_watchdog_child () {
    int status;
    CU_ASSERT (_child != 0);
    // Poll the watchdog; both processes are running
    CU_ASSERT (_watchdog0 (2, getppid (), _child) == -1);
    // Kill the child process that was started
    kill (_child, SIGTERM);
    // The blocking watchdog call will now complete, indicating the child
    CU_ASSERT (watchdog (2, getppid (), _child) == 1);
    // The watchdog *may* have consumed the child signal
    waitpid (_child, &status, WNOHANG);
    _child = 0;
}

VERBOSE_AND_QUIET_TEST (watchdog_child)

int register_tests_watchdog () {
    CU_pSuite pSuite = CU_add_suite ("watchdog", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "watchdog [parent,quiet]", test_watchdog_parent)
     || !CU_add_test (pSuite, "watchdog [parent,verbose]", test_watchdog_parent_verbose)
     || !CU_add_test (pSuite, "watchdog [child,quiet]", test_watchdog_child)
     || !CU_add_test (pSuite, "watchdog [child,verbose]", test_watchdog_child_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
