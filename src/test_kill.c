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
#include "kill.h"
#include "params.h"
#include "test_verbose.h"
#include <CUnit/Basic.h>
#ifndef _WIN32
# include <wait.h>
# include <unistd.h>
#endif /* ifndef _WIN32 */
#include <stdlib.h>

static _WIN32_OR_POSIX (HANDLE, pid_t) _child = 0;

int _fork_init_kill_process () {
    _WIN32_OR_POSIX (Sleep (30000), sleep (30));
    fprintf (stderr, "Child process %u from %s was NOT killed\n", _WIN32_OR_POSIX (GetCurrentProcessId (), getpid ()), __FUNCTION__);
    return 0;
}

static void init_kill_process () {
#ifdef _WIN32
	TCHAR szExecutable[MAX_PATH];
	TCHAR szParams[] = TEXT ("test.exe fork init_kill_process");
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child == 0);
    params_v (0);
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
	if (!_child) _fork_init_kill_process ();
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child != _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, (pid_t)-1));
}

static void do_kill_process () {
#ifndef _WIN32
    int status;
#endif /* ifndef _WIN32 */
    CU_ASSERT (_child != 0);
    // Kill the child process
    kill_process (_child);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (_child, 5000) == WAIT_OBJECT_0);
	CloseHandle (_child);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
#endif /* ifdef _WIN32 */
    _child = 0;
}

VERBOSE_AND_QUIET_TEST (kill_process)

int register_tests_kill () {
    CU_pSuite pSuite = CU_add_suite ("kill", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "kill_process [quiet]", test_kill_process)
     || !CU_add_test (pSuite, "kill_process [verbose]", test_kill_process_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
