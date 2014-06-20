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
#include "operations.h"
#include "params.h"
#include "test_verbose.h"
#include "process.h"
#include "kill.h"
#include <CUnit/Basic.h>
#ifndef _WIN32
# include <wait.h>
# include <unistd.h>
#endif /* ifndef _WIN32 */
#include <stdlib.h>

static _WIN32_OR_POSIX (HANDLE, pid_t) _parent = 0;

int _fork_spawn_parent () {
    _WIN32_OR_POSIX (Sleep (30000), sleep (30));
    fprintf (stderr, "Child process %u from %s was NOT killed\n", _WIN32_OR_POSIX (GetCurrentProcessId (), getpid ()), __FUNCTION__);
    return 0;
}

static void spawn_parent () {
#ifdef _WIN32
	TCHAR szExecutable[MAX_PATH];
	TCHAR szParams[] = TEXT ("test.exe fork spawn_parent");
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#endif /* ifdef _WIN32 */
    CU_ASSERT (_parent == 0);
#ifdef _WIN32
	ZeroMemory (&si, sizeof (si));
	si.cb = sizeof (si);
	if (GetModuleFileName (NULL, szExecutable, sizeof (szExecutable) / sizeof (TCHAR))
	 && CreateProcess (szExecutable, szParams, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		_parent = pi.hProcess;
		CloseHandle (pi.hThread);
	} else {
		_parent = INVALID_HANDLE_VALUE;
	}
#else /* ifdef _WIN32 */
    _parent = fork ();
	if (!_parent) exit (_fork_spawn_parent ());
#endif /* ifdef _WIN32 */
    CU_ASSERT_FATAL (_parent != _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, (pid_t)-1));
}

static void init_operation_start_spawn () {
    CU_ASSERT_FATAL (params_v (3, "start", _WIN32_OR_POSIX ("src\\example-child-script.bat", "src/example-child-script.sh"), "foo") == 0);
}

static void do_operation_start_spawn () {
    _WIN32_OR_POSIX (HANDLE, pid_t) process;
#ifndef _WIN32
    int status;
#endif /* ifndef _WIN32 */
    // First call will start the process
    CU_ASSERT (operation_start () == 0);
    // Second call will fail
    CU_ASSERT (operation_start () == _WIN32_OR_POSIX (ERROR_ALREADY_EXISTS, EALREADY));
    // Kill the process to tidy up
    process = process_find ();
    CU_ASSERT_FATAL (process != 0);
    kill_process (process);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (process, 5000) == WAIT_OBJECT_0);
	CloseHandle (process);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (process, &status, 0) == process);
#endif /* ifdef _WIN32 */
    CU_ASSERT (process_housekeep () == 0);
}

VERBOSE_AND_QUIET_TEST (operation_start_spawn)

static void init_operation_start_watchdog () {
    char tmp[32];
    CU_ASSERT_FATAL (_parent == 0);
    spawn_parent ();
    CU_ASSERT_FATAL (snprintf (tmp, sizeof (tmp), "%u", _WIN32_OR_POSIX (GetProcessId (_parent), _parent)) < sizeof (tmp));
    CU_ASSERT_FATAL (params_v (6, "-p", "-P", tmp, "start", _WIN32_OR_POSIX ("src\\example-child-script.bat", "src/example-child-script.sh"), "foo") == 0);
}

static void do_operation_start_watchdog () {
    _WIN32_OR_POSIX (HANDLE, pid_t) process;
    int i;
    CU_ASSERT_FATAL (_parent != 0);
    CU_ASSERT (process_find () == 0);
    // Start the child process
    CU_ASSERT (operation_start () == 0);
	process = process_find ();
    CU_ASSERT (process != 0);
#ifdef _WIN32
	CloseHandle (process);
#endif /* ifdef _WIN32 */
    // Kill the parent and the watchdog will kick in
    kill_process (_parent);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (_parent, 5000) == WAIT_OBJECT_0);
	CloseHandle (_parent);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_parent, &i, 0) == _parent);
#endif /* ifdef _WIN32 */
    _parent = 0;
    for (i = 0; (i < 30) && (process_find () != 0); i++) {
        _WIN32_OR_POSIX (Sleep (1000), sleep (1));
    }
    CU_ASSERT (process_find () == 0);
}

VERBOSE_AND_QUIET_TEST (operation_start_watchdog)

int register_tests_start () {
    CU_pSuite pSuite = CU_add_suite ("start", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "operation_start [spawn,verbose]", test_operation_start_spawn_verbose)
     || !CU_add_test (pSuite, "operation_start [spawn,quiet]", test_operation_start_spawn)
     || !CU_add_test (pSuite, "operation_start [watchdog,verbose]", test_operation_start_watchdog_verbose)
     || !CU_add_test (pSuite, "operation_start [watchdog,quiet]", test_operation_start_watchdog)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
