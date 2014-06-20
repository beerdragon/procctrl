/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#include "parent.h"
#ifdef _WIN32
# include <Tlhelp32.h>
#endif /* ifdef _WIN32 */
#include <stdio.h>
#include <stdlib.h>

/// @brief Gets the parent of a process
///
/// Queries the process status information and returns the parent PID/HANDLE.
///
/// @return the parent PID/HANDLE, or (pid_t)-1/INVALID_HANDLE_VALUE if there is a problem
_WIN32_OR_POSIX (HANDLE, pid_t) get_parent (
	_WIN32_OR_POSIX (HANDLE, pid_t) process ///<the process to query>
	) {
	_WIN32_OR_POSIX (HANDLE, pid_t) parent = _WIN32_OR_POSIX (INVALID_HANDLE_VALUE, (pid_t)-1);
#ifdef _WIN32
	HANDLE hSnapshot;
	hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		DWORD dwProcess = GetProcessId (process);
		PROCESSENTRY32 pe;
		printf ("Finding parent for %u\n", dwProcess);
		ZeroMemory (&pe, sizeof (pe));
		pe.dwSize = sizeof (pe);
		if (Process32First (hSnapshot, &pe)) {
			do {
				if (pe.th32ProcessID == dwProcess) {
					parent = OpenProcess (PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pe.th32ParentProcessID);
					printf("Found parent %u\n", pe.th32ParentProcessID);
					break;
				}
			} while (Process32Next (hSnapshot, &pe));
		}
		CloseHandle (hSnapshot);
	}
#else /* ifdef _WIN32 */
    char tmp[32];
    FILE *status;
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
#endif /* ifdef _WIN32 */
    return parent;
}
