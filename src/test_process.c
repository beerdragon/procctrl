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
#include "process.h"
#include "test_verbose.h"
#include "params.h"
#include "kill.h"
#include "parent.h"
#include <CUnit/Basic.h>
#ifndef _WIN32
# include <wait.h>
# include <sys/stat.h>
# include <unistd.h>
#endif /* ifndef _WIN32 */
#include <stdlib.h>

#define HK_PATH     64

static _WIN32_OR_POSIX (HANDLE, pid_t) _child = 0;

#ifndef _WIN32
int _wait_for_execvp (pid_t child);
#endif /* ifndef __WIN32 */

static void spawn_example_child_script () {
#ifdef _WIN32
	fprintf (stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
	// TODO: Spawn the script
	_child = INVALID_HANDLE_VALUE;
#else /* ifdef _WIN32 */
    _child = fork ();
	if (!_child) {
        const char *args[3];
        args[0] = "src/example-child-script.sh";
        args[1] = "foo";
        args[2] = NULL;
        execvp (args[0], (char**)args);
	}
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child != _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, (pid_t)-1));
#ifndef _WIN32
    CU_ASSERT (_wait_for_execvp (_child) == 0);
#endif /* ifndef _WIN32 */
}

static void write_info_file (const char *path, int valid) {
    FILE *out = fopen (path, "wt");
    CU_ASSERT_FATAL (out != NULL);
    switch (valid) {
        case -1 :
            // Invalid file (no PID)
            fprintf (out, "cmd: %s\n", "/bin/bash");
            break;
        case -2 :
            // Invalid file (PID, no CMD)
            fprintf (out, "pid: %u\n", _WIN32_OR_POSIX (GetCurrentProcessId (), getpid ()));
            break;
        case -3 :
            // Valid PID but invalid command line
            fprintf (out, "pid: %u\n", _WIN32_OR_POSIX (GetCurrentProcessId (), getpid ()));
            fprintf (out, "cmd: %s\n", "foo/bar");
            break;
        case -4 :
            // Invalid PID
            fprintf (out, "pid: %u\n", 0);
            fprintf (out, "cmd: %s\n", "/bin/bash");
            break;
        case 1 :
            // Valid PID; spawn child script and use script name
            if (_child == 0) spawn_example_child_script ();
            fprintf (out, "pid: %u\n", _WIN32_OR_POSIX (GetProcessId (_child), _child));
            fprintf (out, "cmd: %s\n", "src/example-child-script.sh foo");
            break;
        case 2 :
            // Valid PID; use current executable name
            fprintf (out, "pid: %u\n", _WIN32_OR_POSIX (GetCurrentProcessId (), getpid ()));
            fprintf (out, "cmd: %s\n", "./src/unittest");
            break;
    }
    fclose (out);
}

static void create_folder (const char *path) {
#ifdef _WIN32
	CreateDirectory (path, NULL);
#else /* ifdef _WIN32 */
	mkdir (path, 0755);
#endif /* ifdef _WIN32 */
}

static int dir_exists (const char *path) {
#ifdef _WIN32
	DWORD info = GetFileAttributes (path);
	if (info == INVALID_FILE_ATTRIBUTES) return 0;
	return info & FILE_ATTRIBUTE_DIRECTORY;
#else /* ifdef _WIN32 */
    struct stat info;
    if (stat (path, &info) != 0) return 0;
    return S_ISDIR (info.st_mode);
#endif /* ifdef _WIN32 */
}

static int file_exists (const char *path) {
#ifdef _WIN32
	DWORD info = GetFileAttributes (path);
	if (info == INVALID_FILE_ATTRIBUTES) return 0;
	return !(info & FILE_ATTRIBUTE_DIRECTORY);
#else /* ifdef _WIN32 */
    struct stat info;
    if (stat (path, &info) != 0) return 0;
    return !S_ISDIR (info.st_mode);
#endif /* ifdef _WIN32 */
}

static void init_process_housekeep () {
    int i;
    char path[HK_PATH];
#ifdef _WIN32
	char tmpdir[16];
	HANDLE hParent;
	snprintf (tmpdir, sizeof (tmpdir), "test%u", GetCurrentProcessId ());
	create_folder (tmpdir);
	hParent = get_parent (GetCurrentProcess ());
#else /* ifdef _WIN32 */
    char tmp[] = "testXXXXXX";
    char *tmpdir = mkdtemp (tmp);
    CU_ASSERT_FATAL (tmpdir != NULL);
#endif /* ifdef _WIN32 */
    CU_ASSERT_FATAL (_child == 0);
    CU_ASSERT_FATAL (params_v (2, "-d", tmpdir) == 0);
    // Create GLOBAL, invalid PPID and valid PPID parent directories
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL", tmpdir) < HK_PATH);
	create_folder (path);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/0", tmpdir) < HK_PATH);
	create_folder (path);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d", tmpdir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
	create_folder (path);
    // Create invalid info files in GLOBAL
    for (i = 1; i <= 4; i++) {
        CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL/invalid%d", tmpdir, i) < HK_PATH);
        write_info_file (path, -i);
    }
    // Create a valid info file in GLOBAL
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL/valid", tmpdir) < HK_PATH);
    write_info_file (path, 1);
    // Create a valid info file in invalid PPID
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/0/test", tmpdir) < HK_PATH);
    write_info_file (path, 2);
    // Create a valid info file in valid PPID
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d/test", tmpdir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
    write_info_file (path, 1);
#ifdef _WIN32
	CloseHandle (hParent);
#endif /* ifdef _WIN32 */
}

static void do_process_housekeep () {
    int i;
    char path[HK_PATH];
#ifdef _WIN32
	HANDLE hParent;
	hParent = get_parent (GetCurrentProcess ());
#endif /* ifdef _WIN32 */
    CU_ASSERT_FATAL (_child != 0);
    // Run the housekeep
    CU_ASSERT (process_housekeep () == 0);
    // Invalid PPID directory should be deleted
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/0", data_dir) < HK_PATH);
    CU_ASSERT (dir_exists (path) == 0);
    // Other folders remain
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL", data_dir) < HK_PATH);
    CU_ASSERT (dir_exists (path) != 0);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
    CU_ASSERT (dir_exists (path) != 0);
    // Invalid info files should be deleted
    for (i = 1; i <= 4; i++) {
        CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL/invalid%d", data_dir, i) < HK_PATH);
        CU_ASSERT (file_exists (path) == 0);
    }
    // Other files should be untouched
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL/valid", data_dir) < HK_PATH);
    CU_ASSERT (file_exists (path) != 0);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d/test", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
    CU_ASSERT (file_exists (path) != 0);
    // Terminate the child process
    kill_process (_child);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (_child, 5000) == WAIT_OBJECT_0);
	CloseHandle (_child);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_child, &i, 0) == _child);
#endif /* ifdef _WIN32 */
    _child = 0;
    // Run the housekeep
    CU_ASSERT (process_housekeep () == 0);
    // GLOBAL and PPID directories should now be deleted, but HK folder remains
    CU_ASSERT (dir_exists (data_dir) != 0);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/GLOBAL", data_dir) < HK_PATH);
    CU_ASSERT (dir_exists (path) == 0);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
    CU_ASSERT (dir_exists (path) == 0);
    // Delete the housekeep folder
    _WIN32_OR_POSIX (RemoveDirectory, rmdir) (data_dir);
    // Run the housekeep
    CU_ASSERT (process_housekeep () == ENOENT);
#ifdef _WIN32
	CloseHandle (hParent);
#endif /* ifdef _WIN32 */
}

VERBOSE_AND_QUIET_TEST (process_housekeep)

static void init_process_find () {
    char path[HK_PATH];
#ifdef _WIN32
	char tmpdir[16];
	HANDLE hParent;
	snprintf (tmpdir, sizeof (tmpdir), "test%u", GetCurrentProcessId ());
	create_folder (tmpdir);
	hParent = get_parent (GetCurrentProcess ());
#else /* ifdef _WIN32 */
    char tmp[] = "testXXXXXX";
    char *tmpdir = mkdtemp (tmp);
    CU_ASSERT_FATAL (tmpdir != NULL);
#endif /* ifdef _WIN32 */
    CU_ASSERT (_child == 0);
    CU_ASSERT_FATAL (params_v (4, "-d", tmpdir, "-k", "/Te^st\\") == 0);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
	create_folder (path);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d/^%02XTe^%02Xst^%02X", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ()), (int)'/' & 0xFF, (int)'^' & 0xFF, (int)'\\' & 0xFF) < HK_PATH);
    write_info_file (path, 1);
#ifdef _WIN32
	CloseHandle (hParent);
#endif /* ifdef _WIN32 */
}

static void do_process_find () {
#ifndef _WIN32
    int status;
#endif /* ifndef _WIN32 */
    CU_ASSERT_FATAL (_child != 0);
    // Should find the child
    CU_ASSERT (process_find () == _child);
    // Terminate the child
    kill_process (_child);
#ifdef _WIN32
	CU_ASSERT (WaitForSingleObject (_child, 5000) == WAIT_OBJECT_0);
	CloseHandle (_child);
#else /* ifdef _WIN32 */
    CU_ASSERT (waitpid (_child, &status, 0) == _child);
#endif /* ifdef _WIN32 */
    _child = 0;
    // Should not find the child - invalid PID in the file
    CU_ASSERT (process_find () == 0);
    // Delete the info file
    CU_ASSERT (process_housekeep () == 0);
    // Should not find the child - no info file
    CU_ASSERT (process_find () == 0);
    _WIN32_OR_POSIX (RemoveDirectory, rmdir) (data_dir);
}

VERBOSE_AND_QUIET_TEST (process_find)

static void init_process_save () {
#ifdef _WIN32
	char tmpdir[16];
	snprintf (tmpdir, sizeof (tmpdir), "test%u", GetCurrentProcessId ());
#else /* ifdef _WIN32 */
    char tmp[] = "testXXXXXX";
    char *tmpdir = mkdtemp (tmp);
    rmdir (tmpdir);
#endif /* ifdef _WIN32 */
    CU_ASSERT_FATAL (params_v (5, "-d", tmpdir, "-ktest", "start", "foo bar") == 0);
}

static void do_process_save () {
    char path[HK_PATH];
#ifdef _WIN32
	HANDLE hParent = get_parent (GetCurrentProcess ());
#endif /* ifdef _WIN32 */
    CU_ASSERT (process_save (_WIN32_OR_POSIX (INVALID_HANDLE_VALUE, 1234)) == 0);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d/test", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
    CU_ASSERT (file_exists (path) != 0);
    _WIN32_OR_POSIX (DeleteFile, unlink) (path);
    CU_ASSERT_FATAL (snprintf (path, HK_PATH, "%s/%d", data_dir, _WIN32_OR_POSIX (GetProcessId (hParent), getppid ())) < HK_PATH);
    _WIN32_OR_POSIX (RemoveDirectory, rmdir) (path);
    _WIN32_OR_POSIX (RemoveDirectory, rmdir) (data_dir);
#ifdef _WIN32
	CloseHandle (hParent);
#endif /* ifdef _WIN32 */
}

VERBOSE_AND_QUIET_TEST (process_save)

int register_tests_process () {
    CU_pSuite pSuite = CU_add_suite ("process", NULL, NULL);
    if (!pSuite
     || !CU_add_test (pSuite, "process_housekeep [quiet]", test_process_housekeep)
     || !CU_add_test (pSuite, "process_housekeep [verbose]", test_process_housekeep_verbose)
     || !CU_add_test (pSuite, "process_find [quiet]", test_process_find)
     || !CU_add_test (pSuite, "process_find [verbose]", test_process_find_verbose)
     || !CU_add_test (pSuite, "process_save [quiet]", test_process_save)
     || !CU_add_test (pSuite, "process_save [verbose]", test_process_save_verbose)) {
        return CU_get_error ();
    }
    return 0;
}

#endif /* ifdef HAVE_CUNIT_H */
