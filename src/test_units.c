/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* ifdef HAVE_CONFIG_H */
#include "test_units.h"
#ifdef _WIN32
# include <Windows.h>
#endif /* ifdef _WIN32 */
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_CUNIT_H
# include <CUnit/Basic.h>
#endif /* ifdef HAVE_CUNIT_H */

#define SUITE(test) \
    if ((e = register_tests_##test ()) != 0) { \
        CU_cleanup_registry (); \
        return e; \
    }

#ifdef _WIN32
int _fork_init_kill_process (); // test_kill.c
int _fork_init_operation_query (); // test_query.c
int _fork_spawn_child (); // test_watchdog.c
int _fork_spawn_parent (); // test_start.c
int _fork_watchdog (DWORD dwChild, DWORD dwParent); // start.c
#endif /* ifdef _WIN32 */

int main (int argc, char **argv) {
#ifdef _WIN32
	if ((argc > 2) && !strcmp (argv[1], "fork")) {
		if (!strcmp (argv[2], "init_kill_process")) {
			return _fork_init_kill_process ();
		} else if (!strcmp (argv[2], "init_operation_query")) {
			return _fork_init_operation_query ();
		} else if (!strcmp (argv[2], "spawn_child")) {
			return _fork_spawn_child ();
		} else if (!strcmp (argv[2], "spawn_parent")) {
			return _fork_spawn_parent ();
		} else if (!strcmp (argv[2], "watchdog")) {
			return _fork_watchdog (atoi (argv[3]), atoi (argv[4]));
		} else {
			fprintf (stderr, "Bad fork %s\n", argv[2]);
			return ERROR_INVALID_PARAMETER;
		}
	}
#endif /* ifdef _WIN32 */
#ifdef HAVE_CUNIT_H
    int e;
    // Initialise CUnit
    if ((e = CU_initialize_registry ()) != CUE_SUCCESS) return e;
    // Add/init all of the suites
    SUITE (kill)
    SUITE (params)
    SUITE (process)
    SUITE (query)
    SUITE (start)
    SUITE (stop)
    SUITE (watchdog)
    // Run the tests
    CU_basic_set_mode (CU_BRM_VERBOSE);
    CU_basic_run_tests ();
    e = CU_get_number_of_failures ();
    // Clean up and return
    CU_cleanup_registry ();
    // Non-zero exit code if a test failed
    return (e > 0) ? 99 : 0;
#else /* ifdef HAVE_CUNIT_H */
    return 0;
#endif /* ifdef HAVE_CUNIT_H */
}
