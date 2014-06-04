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
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @brief Gets the parent PID of a process
///
/// Queries the process status information and returns the parent ID.
///
/// @return the parent PID, or (pid_t)-1 if there is a problem
static pid_t get_parent (
    pid_t process ///<the process to query>
    ) {
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
}

/// @brief Linked list of pid_t values
struct _pid_t_list {
    /// @brief The process identifier
    pid_t pid;
    /// @brief The next entry in the list, or NULL if this is the last
    struct _pid_t_list *next;
};

/// @brief Sends a signal to all processes in a tree
///
/// The signal is sent from bottom to top, with the processes stopped during
/// the enumeration. For example, a process is sent SIGSTOP, the requested
/// signal (after its children), and then SIGCONT.
///
/// @return zero if the process was signalled, a non-zero error code otherwise
static int signal_tree (
    pid_t process, ///<the process at the head of the tree to signal>
    int signal ///<the signal number to send>
    ) {
    DIR *dir;
    struct dirent *ent;
    struct _pid_t_list *children = NULL;
    // Pre-signal
    if (verbose) fprintf (stdout, "Signalling %u (SIGSTOP)\n", process);
    if (kill (process, SIGSTOP) != 0) return errno;
    // Find the process' children
    dir = opendir ("/proc");
    if (!dir) return ENOENT;
    while ((ent = readdir (dir)) != NULL) {
        pid_t proc;
        if (!isdigit (ent->d_name[0])) continue;
        proc = (pid_t)strtol (ent->d_name, NULL, 10);
        if (get_parent (proc) == process) {
            struct _pid_t_list *entry = (struct _pid_t_list*)malloc (sizeof (struct _pid_t_list));
            if (entry) {
                entry->pid = proc;
                entry->next = children;
                children = entry;
            }
        }
    }
    closedir (dir);
    // Signal the children
    while (children) {
        struct _pid_t_list *next = children->next;
        pid_t proc = children->pid;
        free (children);
        children = next;
        signal_tree (proc, signal);
    }
    // Post-signal
    if (verbose) fprintf (stdout, "Signalling %u (%d+SIGCONT)\n", process, signal);
    if (kill (process, signal) != 0) return errno;
    if (kill (process, SIGCONT) != 0) return errno;
    return 0;
}

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
    // Stop the processes before child enumeration, terminate them afterwards
    return signal_tree (process, SIGTERM);
}
