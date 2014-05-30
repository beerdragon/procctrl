/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Process termination watchdog

#include "watchdog.h"
#include "params.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/// @brief Watches one or more PIDs for termination
///
/// The supplied PIDs are monitored for any that have terminated. These may be
/// child PIDs spawned by this process, or other arbitrary PIDs corresponding
/// to logical parent processes.
///
/// Note that a process which cannot be queried, perhaps because of security
/// reasons, will be reported as terminated.
///
/// @return the index of the process whose PID is no longer valid.
int watchdog (
    int count, ///<the number of pid_t parameters to follow>
    ... ///<the pid_t values to monitor>
    ) {
    int i;
    va_list pids;
    if (verbose) {
        va_start (pids, count);
        for (i = 0; i < count; i++) {
            fprintf (stdout, "Watching process %u for termination\n", va_arg (pids, pid_t));
        }
        va_end (pids);
    }
    do {
        va_start (pids, count);
        for (i = 0; i < count; i++) {
            pid_t process = va_arg (pids, pid_t);
            if (kill (process, 0) == 0) {
                int status;
                pid_t terminated;
                terminated = waitpid (process, &status, WNOHANG);
                if (terminated != process) continue;
            }
            if (verbose) fprintf (stdout, "Process %u is no longer valid\n", process);
            va_end (pids);
            return i;
        }
        va_end (pids);
        sleep (1);
    } while (1);
}
