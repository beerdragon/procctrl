/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#include "parent.h"
#include <stdio.h>
#include <stdlib.h>

/// @brief Gets the parent of a process
///
/// Queries the process status information and returns the parent PID/HANDLE.
///
/// @return the parent PID/HANDLE, or (pid_t)-1/INVALID_HANDLE_VALUE if there is a problem
_WIN32_OR_POSIX (HANDLE, pid_t) get_parent (
	_WIN32_OR_POSIX (HANDLE, pid_t) process ///<the process to query>
	) {
#ifdef _WIN32
	fprintf (stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
	// TODO
	return INVALID_HANDLE_VALUE;
#else /* ifdef _WIN32 */
    char tmp[32];
    FILE *status;
    pid_t parent = (pid_t)-1;
    if (snprintf (tmp, sizeof (tmp), "/proc/%u/status", process) < 32) {
        status = fopen (tmp, "rt");
        if (status) {
            while (fgets (tmp, sizeof (tmp), status)) {
                if (!strncmp (tmp, "PPid:\t", 6)) {
                    parent = (pid_t)strtol (tmp + 6, NULL, 10);
                }
            }
            fclose (status);
        }
    }
    return parent;
#endif /* ifdef _WIN32 */
}
