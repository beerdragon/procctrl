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
#include "kill.h"
#include "params.h"
#include "test_verbose.h"
#include <CUnit/Basic.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>

static pid_t _child = 0;

static void init_kill_process () {
    CU_ASSERT (_child == (pid_t)0);
    params_v (0);
    // Start a child process
    _child = fork ();
    if (_child) {
        CU_ASSERT (_child != (pid_t)-1);
    }  else {
        // This is the child process; wait to be killed
        sleep (30);
        fprintf (stderr, "Child process %u from %s was NOT killed\n", getpid (), __func__);
        exit (0);
    }
}

static void do_kill_process () {
    int status;
    CU_ASSERT (_child != (pid_t)0);
    // Kill the child process
    kill_process (_child);
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
    _child = 0;
}

VERBOSE_AND_QUIET_TEST (kill_process)

int register_tests_kill () {
    CU_pSuite pSuite = CU_add_suite ("kill", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "kill_process [quiet]", test_kill_process)
     || !CU_add_test (pSuite, "kill_process [verbose]", test_kill_process_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
