/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_process_h
#define __inc_process_h

/// @file
/// @brief Process information management
///
/// Header file for exported definitions from process.c used by other components.

#ifdef _WIN32

#include <Windows.h>

#define _WIN32_OR_POSIX(a,b) a

#else /* ifdef _WIN32 */

#include <sys/types.h>

#define _WIN32_OR_POSIX(a,b) b

#endif /* ifdef _WIN32 */

int process_housekeep ();
_WIN32_OR_POSIX (HANDLE, pid_t) process_find ();
int process_save (_WIN32_OR_POSIX (HANDLE, pid_t) process);

#endif /* ifndef __inc_process_h */
