/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Program entry point

#include "operations.h"
#include "params.h"
#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
int _fork_watchdog (DWORD dwChild, DWORD dwParent); // start.c
#endif /* ifdef _WIN32 */

/// @brief Program entry point
///
/// Proceses the parameters from the command line and dispatches the requested
/// operation. Housekeeping operations are performed before and/or after the
/// dispatch as per the housekeep_mode flag.
///
/// See the `man` page for documentation of the available parameters and their
/// behaviour.
///
/// @return the process exit code, zero for success, non-zero for an error.
///         See the documentation for the operations (operations.h) for more
///         details of the errors returned from each.
int main (
    int argc, ///<the number of command line arguments>
    char **argv ///<the command line arguments>
    ) {
    int e;
#ifdef _WIN32
	if ((argc > 2) && !strcmp (argv[1], "fork")) {
		if (!strcmp (argv[2], "watchdog")) {
			return _fork_watchdog (atoi (argv[3]), atoi (argv[4]));
		} else {
			fprintf (stderr, "Bad fork %s\n", argv[2]);
			return ERROR_INVALID_PARAMETER;
		}
	}
#endif /* ifdef _WIN32 */
    if ((e = params (argc, argv)) == 0) {
        if (housekeep_mode & HOUSEKEEP_BEFORE) process_housekeep ();
        if ((operation == NULL) || !strcmp (operation, "query")) {
            e = operation_query ();
        } else if (!strcmp (operation, "start")) {
            e = operation_start ();
        } else if (!strcmp (operation, "stop")) {
            e = operation_stop ();
        } else {
            fprintf (stderr, "Unknown operation '%s'\n", operation);
            e = 1;
        }
        if (!e && (housekeep_mode & HOUSEKEEP_AFTER)) process_housekeep ();
    }
    return e;
}
