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

#ifdef _WIN32

static BOOL is_escape_char (char c) {
	return isspace (c)
		|| (c == '&')
		|| (c == '(') || (c == ')')
		|| (c == '[') || (c == ']')
		|| (c == '{') || (c == '}')
		|| (c == '^')
		|| (c == '=')
		|| (c == ';')
		|| (c == '!')
		|| (c == '\'') || (c == '\"')
		|| (c == '+')
		|| (c == ',')
		|| (c == '`')
		|| (c == '~');
}

static BOOL is_escape_string (const char *psz) {
	while (*psz) {
		if (is_escape_char (*(psz++))) return TRUE;
	}
	return FALSE;
}

static char *create_comspec_command_line () {
	char *psz;
	size_t cch = spawn_argc * 3 + 11; // quotes, space between, null terminator and "cmd.exe /c"
	int i, j;
	for (i = 0; i < spawn_argc; i++) {
		cch += strlen (spawn_argv[i]);
	}
	psz = (char*)malloc (cch);
	if (!psz) return NULL;
	j = sprintf (psz, "cmd.exe /c");
	for (i = 0; i < spawn_argc; i++) {
		j += sprintf (psz + j, is_escape_string (spawn_argv[i]) ? " \"%s\"" : " %s", spawn_argv[i]);
	}
	return psz;
}

static char *create_command_line () {
	char *psz;
	size_t cch = spawn_argc * 3; // quotes, space between, null terminator
	int i, j;
	for (i = 0; i < spawn_argc; i++) {
		cch += strlen (spawn_argv[i]);
	}
	psz = (char*)malloc (cch);
	if (!psz) return NULL;
	j = sprintf (psz, is_escape_string (spawn_argv[0]) ? "\"%s\"" : "%s", spawn_argv[0]);
	for (i = 1; i < spawn_argc; i++) {
		j += sprintf (psz + j,is_escape_string (spawn_argv[i]) ? " \"%s\"" : " %s", spawn_argv[i]);
	}
	return psz;
}

static BOOL spawn_process (LPPROCESS_INFORMATION lppi) {
	char *pszApplication;
	char *pszCommandLine;
	STARTUPINFO si;
	BOOL bResult;
	size_t cch;
	ZeroMemory (&si, sizeof (si));
	si.cb = sizeof (si);
	cch = strlen (spawn_argv[0]);
	if ((cch > 4) && !stricmp (spawn_argv[0] + cch - 4, ".bat")) {
		pszApplication = getenv ("ComSpec");
		pszCommandLine = create_comspec_command_line ();
	} else {
		pszApplication = spawn_argv[0];
		pszCommandLine = create_command_line ();
	}
	if (!pszCommandLine) {
		SetLastError (ERROR_OUTOFMEMORY);
		return FALSE;
	}
	bResult = CreateProcess (pszApplication, pszCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, lppi);
	free (pszCommandLine);
	return bResult;
}

#endif /* ifdef _WIN32 */

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

static int _fork_watchdog0 (
	_WIN32_OR_POSIX (HANDLE, pid_t) child,
	_WIN32_OR_POSIX (HANDLE, pid_t) parent
	) {
    if (watchdog (2, child, parent) == 1) {
        if (verbose) fprintf (stdout, "Killing child process on parent termination\n");
        kill_process (child);
    }
    return 0;
}

int _fork_watchdog (
	_WIN32_OR_POSIX (DWORD, pid_t) child,
	_WIN32_OR_POSIX (DWORD, pid_t) parent
	) {
#ifdef _WIN32
	HANDLE hChild;
	HANDLE hParent;
	int nResult;
	hChild = OpenProcess (PROCESS_QUERY_INFORMATION | SYNCHRONIZE | PROCESS_TERMINATE, FALSE, child);
	if (!hChild) {
		// Child already terminated
		return 0;
	}
	hParent = OpenProcess (PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, parent);
	if (hParent) {
		nResult = _fork_watchdog0 (hChild, hParent);
		CloseHandle (hParent);
	} else {
		// Parent already terminated
		if (verbose) fprintf (stdout, "Killing child process on parent termination\n");
		kill_process (hChild);
		nResult = 0;
	}
	CloseHandle (hChild);
	return nResult;
#else /* ifdef _WIN32 */
	return _fork_watchdog0 (child, parent);
#endif /* ifdef _WIN32 */
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
		PROCESS_INFORMATION pi;
		if (!spawn_process (&pi)) {
			return GetLastError ();
		} else {
			if (verbose) fprintf (stdout, "Child process %u spawned\n", pi.dwProcessId);
			process = pi.hProcess;
			CloseHandle (pi.hThread);
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
				char szExecutable[MAX_PATH];
				char szParams[64];
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				DWORD watch_process = 0;
				sprintf (szParams, "procctrl.exe fork watchdog %u %u", GetProcessId (process), GetProcessId (parent_process));
				ZeroMemory (&si, sizeof (si));
				si.cb = sizeof (si);
				if (GetModuleFileName (NULL, szExecutable, sizeof (szExecutable) / sizeof (TCHAR))
				 && CreateProcess (szExecutable, szParams, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
					watch_process = pi.dwProcessId;
					CloseHandle (pi.hProcess);
					CloseHandle (pi.hThread);
				}
#else /* ifdef _WIN32 */
                pid_t watch_process = fork ();
                if (!watch_process) exit (_fork_watchdog (process, parent_process));
                if (watch_process == (pid_t)-1) return errno;
#endif /* ifdef _WIN32 */
                if (verbose) fprintf (stdout, "Watchdog process %u spawned\n", watch_process);
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
