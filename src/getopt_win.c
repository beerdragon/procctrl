/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

/// @file
/// @brief GNU compatible parameter passing
///
/// Provides a `getopt` sufficiently similar to the GNU one for the Windows
/// build to work.

/// @brief Defines a global variable exported through getopt.h
#define MODULE_VAR_EXTERN
#include "getopt_win.h"
#undef MODULE_VAR_EXTERN
#include <stdio.h>

int getopt (int argc, char **argv, const char *args) {
	fprintf (stderr, "TODO: %s (%d)\n", __FUNCTION__, __LINE__);
	// TODO
	optopt = '?';
	return '?';
}
