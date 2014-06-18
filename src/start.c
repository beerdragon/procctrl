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
#ifndef _WIN32
# include <unistd.h>
# include <errno.h>
# include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
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
#endif /* ifndef _WIN32 */

/// @brief Starts the child process
///
/// If there is not already an active process with the symbolic identifier
/// then a process is spawned. If the parent process must be watched for
/// termination then an additional watchdog process is also spawned.
///
/// @return zero if successful, otherwise a non-zero error code
int operation_start () {
    int e;
	_WIN32_OR_POSIX (HANDLE, pid_t) process;
    if (verbose) fprintf (stdout, "Spawning child process\n");
    process = process_find ();
    if (process) {
        if (verbose) fprintf (stdout, "Process %u already running\n", _WIN32_OR_POSIX (GetProcessId (process), process));
#ifdef _WIN32
		CloseHandle (process);
#endif /* ifdef _WIN32 */
        return _WIN32_OR_POSIX (ERROR_ALREADY_EXISTS, EALREADY);
    } else {
#ifdef _WIN32
		fprintf(stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
		// TODO: Spawn the process
		if (1) {
#else /* ifdef _WIN32 */
        process = fork ();
        if (process) {
            if (process == (pid_t)-1) return errno;
            if (verbose) fprintf (stdout, "Child process %u spawned\n", process);
            _wait_for_execvp (process);
#endif /* ifdef _WIN32 */
            e = process_save (process);
            if (e) {
                fprintf (stderr, "Couldn't write process information, error %d\n", e);
            }
            if (watch_parent) {
#ifdef _WIN32
				HANDLE watch_process = INVALID_HANDLE_VALUE;
				fprintf(stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
				// TODO: Spawn the watch process
				if (1) {
#else /* ifdef _WIN32 */
                pid_t watch_process = fork ();
                if (watch_process) {
                    if (watch_process == (pid_t)-1) return errno;
#endif /* ifdef _WIN32 */
                    if (verbose) fprintf (stdout, "Watchdog process %u spawned\n", _WIN32_OR_POSIX (GetProcessId (watch_process), watch_process));
#ifndef _WIN32
                } else {
                    if (watchdog (2, process, parent_process) == 1) {
                        if (verbose) fprintf (stdout, "Killing child process on parent termination\n");
                        kill_process (process);
                    }
                    exit (0);
#endif /* ifndef _WIN32 */
                }
            }
            return 0;
#ifndef _WIN32
        } else {
            execvp (spawn_argv[0], spawn_argv);
            e = errno;
            fprintf (stderr, "Couldn't run %s, error %d\n", spawn_argv[0], e);
            return e;
#endif /* ifndef _WIN32 */
        }
    }
}
