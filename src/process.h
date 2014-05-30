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

#include <sys/types.h>

void process_housekeep ();
pid_t process_find ();
int process_save (pid_t process);

#endif /* ifndef __inc_process_h */
