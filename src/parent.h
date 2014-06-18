/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_parent_h
#define __inc_parent_h

/// @file
/// @brief Helper functions for finding a process' parent

#ifdef _WIN32

#include <Windows.h>

#define _WIN32_OR_POSIX(a, b) a

#else /* ifdef _WIN32 */

#include <sys/types.h>

#define _WIN32_OR_POSIX(a, b) b

#endif /* ifdef _WIN32 */

_WIN32_OR_POSIX (HANDLE, pid_t) get_parent (_WIN32_OR_POSIX (HANDLE, pid_t) process);

#endif /* ifndef __inc_getopt_h */