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
