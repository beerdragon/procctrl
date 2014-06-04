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

int main () {
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
