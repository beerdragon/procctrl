/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_operations_h
#define __inc_operations_h

/// @file
/// @brief Process control operations
///
/// Header file for the operations that the controller can perform. Each is
/// implemented in its own file.

int operation_query ();
int operation_start ();
int operation_stop ();

#endif /* ifndef __inc_operations_h */
