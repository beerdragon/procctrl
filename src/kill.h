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

#include <sys/types.h>

int kill_process (pid_t process);

#endif /* ifndef __inc_operations_h */
