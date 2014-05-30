/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Process termination

#include "kill.h"
#include "params.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

/// @brief Terminates the process
///
/// Sends the termination signal (SIGTERM) to the process.
///
/// An improvement would be to allow the signal to be specified as a parameter
/// and wait, for a given period, for the process to terminate before sending
/// a more severe kill signal.
///
/// @return zero if the process was terminated, a non-zero error code otherwise
int kill_process (
    pid_t process ///<the PID of the process to terminate>
    ) {
    if (verbose) fprintf (stdout, "Signalling %u\n", process);
    return kill (process, SIGTERM);
}
