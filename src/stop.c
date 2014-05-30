/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Implements the `stop` operation

#include "operations.h"
#include "kill.h"
#include "params.h"
#include "process.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Stops the child process
///
/// If there is an active process with the symbolic identifier then it is
/// killed. If there is a watchdog process from the original spawn then that
/// will also terminate when it detects the child termination.
///
/// @return zero if successful, otherwise a non-zero error code
int operation_stop () {
    pid_t process;
    if (verbose) fprintf (stdout, "Stopping spawned process\n");
    process = process_find ();
    if (process) {
        if (verbose) fprintf (stdout, "Killing process %u\n", process);
        return kill_process (process);
    } else {
        if (verbose) fprintf (stdout, "No process to stop\n");
        return ESRCH;
    }
}
