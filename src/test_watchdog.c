/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* ifdef HAVE_CONFIG_H */
#ifdef HAVE_CUNIT_H
#include "test_units.h"
#include "watchdog.h"
#include "params.h"
#include "test_verbose.h"
#include "kill.h"
#include <CUnit/Basic.h>
#ifdef _WIN32
# include "parent.h"
#else /* ifdef _WIN32 */
# include <wait.h>
# include <unistd.h>
#endif /* ifdef _WIN32 */
#include <stdarg.h>
#include <stdlib.h>

static _WIN32_OR_POSIX (HANDLE, pid_t) _child = 0;

int _watchdog0_v (int count, va_list pids);

static int _watchdog0 (int count, ...) {
    int i;
    va_list processes;
    va_start (processes, count);
    i = _watchdog0_v (count, processes);
    va_end (processes);
    return i;
}

int _fork_spawn_child () {
    _WIN32_OR_POSIX (Sleep (30000), sleep (30));
    fprintf (stderr, "Child process %u from %s was NOT killed\n", _WIN32_OR_POSIX (GetCurrentProcessId (), getpid ()), __FUNCTION__);
    return 0;
}

static void spawn_child () {
#ifdef _WIN32
	TCHAR szExecutable[MAX_PATH];
	TCHAR szParams[] = TEXT ("test.exe fork spawn_child");
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#endif /* ifdef _WIN32 */
    CU_ASSERT_FATAL (_child == 0);
    // Start a child process
#ifdef _WIN32
	ZeroMemory (&si, sizeof (si));
	si.cb = sizeof (si);
	if (GetModuleFileName (NULL, szExecutable, sizeof (szExecutable) / sizeof (TCHAR))
	 && CreateProcess (szExecutable, szParams, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		_child = pi.hProcess;
		CloseHandle (pi.hThread);
	} else {
		_child = INVALID_HANDLE_VALUE;
	}
#else /* ifdef _WIN32 */
    _child = fork ();
	if (!_child) exit (_fork_spawn_child ());
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child != _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, (pid_t)-1));
}

static void init_watchdog_parent () {
    CU_ASSERT (params_v (0) == 0);
    spawn_child ();
}

static void do_watchdog_parent () {
#ifdef _WIN32
	HANDLE hParent = get_parent (GetCurrentProcess ());
#else /* ifdef _WIN32 */
    int status;
#endif /* ifdef _WIN32 */
    CU_ASSERT_FATAL (_child != 0);
    // Poll the watchdog; both processes are running
    CU_ASSERT (_watchdog0 (2, _WIN32_OR_POSIX (hParent, getppid ()), _child) == -1);
    // Call the blocking watchdog with an invalid parent PID
    CU_ASSERT (watchdog (2, _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, 0), _child) == 0);
    // Kill the child process that was started
    kill_process (_child);
#ifdef _WIN32
	CloseHandle (hParent);
	CU_ASSERT (WaitForSingleObject (_child, 5000) == WAIT_OBJECT_0);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
#endif /* ifdef _WIN32 */
    _child = 0;
}

VERBOSE_AND_QUIET_TEST (watchdog_parent)

static void init_watchdog_child () {
    params_v (0);
    spawn_child ();
}

static void do_watchdog_child () {
#ifdef _WIN32
	HANDLE hParent = get_parent (GetCurrentProcess ());
#else /* ifdef _WIN32 */
    int status;
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child != 0);
    // Poll the watchdog; both processes are running
    CU_ASSERT (_watchdog0 (2, _WIN32_OR_POSIX (hParent, getppid ()), _child) == -1);
    // Kill the child process that was started
#ifdef _WIN32
	TerminateProcess (_child, ERROR_ALERTED);
#else /* ifdef _WIN32 */
    kill (_child, SIGTERM);
#endif /* ifdef _WIN32 */
    // The blocking watchdog call will now complete, indicating the child
    CU_ASSERT (watchdog (2, _WIN32_OR_POSIX (hParent, getppid ()), _child) == 1);
#ifdef _WIN32
	CloseHandle (_child);
#else /* ifdef _WIN32 */
    // The watchdog *may* have consumed the child signal
    waitpid (_child, &status, WNOHANG);
#endif /* ifdef _WIN32 */
    _child = 0;
}

VERBOSE_AND_QUIET_TEST (watchdog_child)

int register_tests_watchdog () {
    CU_pSuite pSuite = CU_add_suite ("watchdog", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "watchdog [parent,quiet]", test_watchdog_parent)
     || !CU_add_test (pSuite, "watchdog [parent,verbose]", test_watchdog_parent_verbose)
     || !CU_add_test (pSuite, "watchdog [child,quiet]", test_watchdog_child)
     || !CU_add_test (pSuite, "watchdog [child,verbose]", test_watchdog_child_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
