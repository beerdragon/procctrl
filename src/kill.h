/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_kill_h
#define __inc_kill_h

/// @file
/// @brief Process termination
///
/// Header file for the termination functions published by kill.c.

#ifdef _WIN32

#include <Windows.h>

#define _WIN32_OR_POSIX(a,b) a

#else /* ifdef _WIN32 */

#include <sys/types.h>

#define _WIN32_OR_POSIX(a,b) b

#endif /* ifdef _WIN32 */

int kill_process (_WIN32_OR_POSIX (HANDLE, pid_t) process);

#endif /* ifndef __inc_operations_h */
