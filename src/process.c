/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief Process information management
///
/// Manages the data folder where information about spawned processes is held.

#include "process.h"
#include "params.h"
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

/// @brief The longest line that can be present in an information file
///
/// This value has to be large enough to hold the longest possible command
/// line plus the "cmd: " prefix or the verify_pid(const char*,pid_t) function
/// will not work.
#define MAX_PROCESS_INFO_LINE    256

int _is_running (pid_t process);

/// @brief Checks a PID corresponds to the expected command line
///
/// When a process is spawned, the PID and original command line are written
/// to the information file. Any later operations, such as stopping the
/// process, must verify the PID to give confidence that the originally
/// spawned process has not since died and the PID been reused for something
/// different.
///
/// @return non-zero if the PID exists and corresponds to a process started
///         with the given command line. Zero otherwise.
static int verify_pid (
    const char *command_line, ///<the expected command line to match>
    pid_t process ///<the PID to search for>
    ) {
    FILE *cmdline;
    char tmp[32];
    int verified = 0;
    snprintf (tmp, sizeof (tmp), "/proc/%u/cmdline", process);
    cmdline = fopen (tmp, "rt");
    if (cmdline) {
        const char *script = command_line;
        const char *no_script = command_line;
        int argc = 0;
        do {
            char c = fgetc (cmdline);
            if (feof (cmdline)) break;
            if (argc > 0) {
                if (script) {
                    if ((*script == c) || ((*script == ' ') && !c)) {
                        script++;
                    } else {
                        script = NULL;
                        if (!no_script) break;
                    }
                }
            }
            if (no_script) {
                if ((*no_script == c) || ((*no_script == ' ') && !c)) {
                    no_script++;
                } else {
                    no_script = NULL;
                    if (!script) break;
                }
            }
            if (!c) argc++;
        } while (1);
        if (script && !*script) verified = 1;
        if (no_script && !*no_script) verified = 1;
        fclose (cmdline);
    }
    return verified;
}

/// @brief File handle used to manage the data_dir lock
static int _lock_fd = -1;

/// @brief Obtains the data_dir lock
///
/// Use of the directory should be done while holding the lock. Race
/// conditions may otherwise occur. For example, process A running the
/// housekeeping routine may delete a directory process B created before
/// B could write the info file; B will then be unable to write the file.
///
/// This call will block until the lock can be claimed.
///
/// The caller must use unlock_data_dir() to release the lock when it is
/// finished.
static void lock_data_dir () {
    size_t size;
    char *path;
    if (_lock_fd != -1) abort ();
    size = strlen (data_dir) + 7;
    path = (char*)malloc (size);
    if (!path) abort ();
    sprintf (path, "%s/.lock", data_dir);
    _lock_fd = open (path, O_WRONLY | O_CREAT);
    free (path);
    if (_lock_fd != -1) {
        flock (_lock_fd, LOCK_EX);
    }
}

/// @brief Releases the data_dir lock
///
/// See the description for lock_data_dir() for details.
static void unlock_data_dir () {
    if (_lock_fd != -1) {
        char *path;
        size_t size;
        flock (_lock_fd, LOCK_UN);
        close (_lock_fd);
        size = strlen (data_dir) + 7;
        path = (char*)malloc (size);
        if (!path) abort ();
        sprintf (path, "%s/.lock", data_dir);
        unlink (path);
        free (path);
    }
    _lock_fd = -1;
}

/// @brief Cleans up the data folder
///
/// Scans the data folder, checking that any information files correspond to
/// active processes. Any files that cannot be matched to processes (for
/// example a process has terminated) are deleted.
///
/// @return zero if the housekeep was run, non-zero if there was an issue
int process_housekeep () {
    DIR *dir;
    struct dirent *ent;
    if (verbose) fprintf (stdout, "Cleaning up data area (%s)\n", data_dir);
    dir = opendir (data_dir);
    if (!dir) return ENOENT;
    lock_data_dir ();
    while ((ent = readdir (dir)) != NULL) {
        DIR *subdir;
        char *dirpath, *subdirpath;
        size_t size;
        if (ent->d_name[0] == '.') continue;
        size = strlen (ent->d_name) + strlen (data_dir) + 2;
        dirpath = (char*)malloc (size);
        if (!dirpath) {
            closedir (dir);
            unlock_data_dir ();
            return ENOMEM;
        }
        sprintf (dirpath, "%s/%s", data_dir, ent->d_name);
        if (isdigit (*ent->d_name)) {
            pid_t ppid = (pid_t)strtol (ent->d_name, NULL, 10);
            if (_is_running (ppid) == 0) {
                if (verbose) fprintf (stdout, "Deleting %s - invalid\n", dirpath);
                subdir = opendir (dirpath);
                if (subdir) {
                    while ((ent = readdir (subdir)) != NULL) {
                        if (ent->d_name[0] == '.') {
                            if (!ent->d_name[1]) continue;
                            if ((ent->d_name[1] == '.') && !ent->d_name[2]) continue;
                        }
                        size = strlen (ent->d_name) + strlen (dirpath) + 2;
                        subdirpath = (char*)malloc (size);
                        if (subdirpath) {
                            sprintf (subdirpath, "%s/%s", dirpath, ent->d_name);
                            unlink (subdirpath);
                            free (subdirpath);
                        }
                    }
                    closedir (subdir);
                }
                rmdir (dirpath);
                free (dirpath);
                continue;
            }
        }
        subdir = opendir (dirpath);
        if (subdir) {
            int files = 0;
            while ((ent = readdir (subdir)) != NULL) {
                if (ent->d_name[0] == '.') {
                    if (!ent->d_name[1]) continue;
                    if ((ent->d_name[1] == '.') && !ent->d_name[2]) continue;
                }
                files++;
                size = strlen (ent->d_name) + strlen (dirpath) + 2;
                subdirpath = (char*)malloc (size);
                if (subdirpath) {
                    FILE *info;
                    char *tmp;
                    sprintf (subdirpath, "%s/%s", dirpath, ent->d_name);
                    info = fopen (subdirpath, "rt");
                    if (info) {
                        tmp = (char*)malloc (MAX_PROCESS_INFO_LINE);
                        if (tmp) {
                            pid_t process = 0;
                            char *cmdline = NULL;
                            while (fgets (tmp, MAX_PROCESS_INFO_LINE, info)) {
                                if (!strncmp (tmp, "pid: ", 5)) {
                                    process = (pid_t)strtol (tmp + 5, NULL, 10);
                                } else if (!strncmp (tmp, "cmd: ", 5)) {
                                    cmdline = strdup (tmp + 5);
                                    strtok (cmdline, "\r\n");
                                }
                            }
                            if (!cmdline || !process || !verify_pid (cmdline, process)) {
                                if (verbose) fprintf (stdout, "Deleting %s - invalid\n", subdirpath);
                                fclose (info);
                                info = NULL;
                                unlink (subdirpath);
                                files--;
                            }
                            if (cmdline) free (cmdline);
                            free (tmp);
                        }
                        if (info) fclose (info);
                    }
                    free (subdirpath);
                }
            }
            closedir (subdir);
            if (!files) {
                if (verbose) fprintf (stdout, "Deleting %s - empty\n", dirpath);
                rmdir (dirpath);
            }
        }
        free (dirpath);
    }
    closedir (dir);
    unlock_data_dir ();
    return 0;
}

/// @brief Calculates the length of a PID if expressed as a decimal
///
/// @return the number of decimal characters
static size_t pidlen (
    pid_t pid ///<the PID to evaluate>
    ) {
    size_t digits = 0;
    while (pid) {
        digits++;
        pid /= 10;
    }
    return digits;
}

/// @brief Tests if a character requires escaping
///
/// The process identifier key is used to construct a filename in the data
/// area. Any characters, or the escape character used, which are not valid in
/// a filename must be escaped.
///
/// @return non-zero if the character must be escaped, zero otherwise
static int escape_char (
    char c ///<the character to test>
    ) {
    return !isalnum (c)
        && (c != ' ')
        && (c != '-')
        && (c != '+')
        && (c != '.')
        && (c != '_')
        && (c != '(')
        && (c != ')');
}

/// @brief Calculates the length of the process identifier filename string
///
/// The process identifier might contain characters which need escaping to
/// form a valid filename.
///
/// @return the length of the escaped string
static size_t process_identifier_len () {
    size_t chars = 0;
    const char *ptr = process_identifier;
    while (*ptr) {
        if (escape_char (*ptr++)) {
            chars += 3;
        } else {
            chars++;
        }
    }
    return chars;
}

/// @brief Creates the requested path
///
/// All elements in the path are created if they do not exist.
static void create_path (
    const char *path ///<the path to create>
    ) {
    char *copy = strdup (path);
    char *ptr;
    if (!copy) abort ();
    ptr = copy;
    while (*ptr) {
        if (*ptr == '/') {
            *ptr = 0;
            mkdir (copy, 0755);
            *ptr = '/';
        }
        ptr++;
    }
}

/// @brief Copies the escaped process identifier string into a buffer
///
/// The buffer should be sized to take the escaped string using the length
/// indicated by process_identifier_len(). The identifier is followed by a
/// null terminator character.
static void copy_process_identifier (
    char *ptr, ///<the buffer to copy into>
    size_t chars ///<the size of the buffer>
    ) {
    const char *identifier = process_identifier;
    while (*identifier && chars) {
        if (escape_char (*identifier)) {
            if (chars < 3) return;
            sprintf (ptr, "^%02X", *(identifier++));
            ptr += 3;
            chars -= 3;
        } else {
            *(ptr++) = *(identifier++);
            chars--;
        }
    }
    if (chars) *(ptr++) = 0;
}

/// @brief Generates the path to the process information file
///
/// The path is formed in the data directory as either `GLOBAL/<em>identifier</em>`
/// or `<em>parent-pid</em>/<em>identifier</em>` for global and parent-scoped
/// identifiers respectively.
///
/// The caller must free the allocated string.
///
/// @return the generated path
static char *get_process_path (
    int create ///<non-zero to ensure the path exists, zero to not create it>
    ) {
    size_t buffer_len = strlen (data_dir) + process_identifier_len () + 3;
    char *path;
    int n;
    buffer_len += global_identifier ? 6 : pidlen (parent_process);
    path = (char*)malloc (buffer_len);
    if (!path) abort ();
    n = global_identifier ? snprintf (path, buffer_len, "%s/%s/", data_dir, "GLOBAL") : snprintf (path, buffer_len, "%s/%u/", data_dir, parent_process);
    if (n < 0) abort ();
    if (create) create_path (path);
    copy_process_identifier (path + n, buffer_len - n);
    return path;
}

/// @brief Tests if the controlled process is already running
///
/// A process information file is checked for, and if present the PID of the
/// process is returned. This is used to identify the PID to be killed during
/// a stop operation, or to prevent starting a duplicate process.
///
/// @return the PID if found, 0 otherwise
pid_t process_find () {
    char *path = get_process_path (0);
    FILE *info;
    pid_t pid = 0;
    if (verbose) fprintf (stdout, "Checking for process at %s\n", path);
    lock_data_dir ();
    info = fopen (path, "rt");
    if (info) {
        char *tmp = (char*)malloc (MAX_PROCESS_INFO_LINE);
        if (tmp) {
            char *cmd = NULL;
            while (fgets (tmp, MAX_PROCESS_INFO_LINE, info)) {
                if (!strncmp (tmp, "pid: ", 5)) {
                    pid = (pid_t)strtol (tmp + 5, NULL, 10);
                } else if (!strncmp (tmp, "cmd: ", 5)) {
                    cmd = strdup (tmp + 5);
                    strtok (cmd, "\r\n");
                }
            }
            if (cmd) {
                if (!verify_pid (cmd, pid)) {
                    if (verbose) fprintf (stdout, "Found PID %u but it's invalid or command line is incorrect\n", pid);
                    pid = 0;
                }
                free (cmd);
            }
            free (tmp);
        }
        fclose (info);
    }
    unlock_data_dir ();
    free (path);
    return pid;
}

/// @brief Writes an information file for the controlled process
///
/// A process information file is written, overwriting any that already
/// exists for the controlled process.
///
/// @return zero if successful, otherwise a non-zero error code
int process_save (
    pid_t process ///<the PID of the controlled process>
    ) {
    char *path;
    FILE *info;
    int result;
    lock_data_dir ();
    path = get_process_path (1);
    if (verbose) fprintf (stdout, "Writing state to %s\n", path);
    info = fopen (path, "wt");
    if (info) {
        int i;
        fprintf (info, "pid: %u\n", process);
        fprintf (info, "sid: %s\n", process_identifier);
        fprintf (info, "ppid: %u\n", parent_process);
        fprintf (info, "cmd:");
        for (i = 0; i < spawn_argc; i++) {
            fprintf (info, " %s", spawn_argv[i]);
        }
        fprintf (info, "\n");
        fclose (info);
        result = 0;
    } else {
        result = errno;
    }
    unlock_data_dir ();
    free (path);
    return result;
}
