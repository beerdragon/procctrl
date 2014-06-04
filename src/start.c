/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Implements the `start` operation

#include "operations.h"
#include "kill.h"
#include "params.h"
#include "process.h"
#include "watchdog.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/// @brief Waits for the child to call execvp
///
/// After the initial fork, the identity of the child does not correspond to
/// its final identity. This function blocks until the child terminates or its
/// identity changes.
///
/// @return zero if successful, otherwise a non-zero error code
int _wait_for_execvp (
    pid_t child ///<the child PID>
    ) {
    char tmp[32];
    FILE *cmdSelf, *cmdChild;
    int result = EBUSY;
    if (snprintf (tmp, sizeof (tmp), "/proc/%u/cmdline", child) >= sizeof (tmp)) return ENOMEM;
    do {
        cmdSelf = fopen ("/proc/self/cmdline", "rt");
        if (cmdSelf) {
            cmdChild = fopen (tmp, "rt");
            if (cmdChild) {
                while (!feof (cmdSelf) && !feof (cmdChild)) {
                    if (fgetc (cmdSelf) != fgetc (cmdChild)) {
                        // Command lines differ
                        result = 0;
                        break;
                    }
                }
                fclose (cmdChild);
            } else {
                // Child terminated
                result = 0;
            }
            fclose (cmdSelf);
        } else {
            // Internal error; can't get our own command line
            result = EINVAL;
        }
        if (result != EBUSY) return result;
        sleep (1);
    } while (1);
}

/// @brief Starts the child process
///
/// If there is not already an active process with the symbolic identifier
/// then a process is spawned. If the parent process must be watched for
/// termination then an additional watchdog process is also spawned.
///
/// @return zero if successful, otherwise a non-zero error code
int operation_start () {
    int e;
    pid_t process;
    if (verbose) fprintf (stdout, "Spawning child process\n");
    process = process_find ();
    if (process) {
        if (verbose) fprintf (stdout, "Process %u already running\n", process);
        return EALREADY;
    } else {
        process = fork ();
        if (process) {
            if (process == (pid_t)-1) return errno;
            if (verbose) fprintf (stdout, "Child process %u spawned\n", process);
            _wait_for_execvp (process);
            e = process_save (process);
            if (e) {
                fprintf (stderr, "Couldn't write process information, error %d\n", e);
            }
            if (watch_parent) {
                pid_t watch_process = fork ();
                if (watch_process) {
                    if (watch_process == (pid_t)-1) return errno;
                    if (verbose) fprintf (stdout, "Watchdog process %u spawned\n", watch_process);
                } else {
                    if (watchdog (2, process, parent_process) == 1) {
                        if (verbose) fprintf (stdout, "Killing child process on parent termination\n");
                        kill_process (process);
                    }
                    exit (0);
                }
            }
            return 0;
        } else {
            execvp (spawn_argv[0], spawn_argv);
            e = errno;
            fprintf (stderr, "Couldn't run %s, error %d\n", spawn_argv[0], e);
            return e;
        }
    }
}
