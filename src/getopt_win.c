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
	if (optind < argc) {
		if ((argv[optind][0] == '/') || (argv[optind][0] == '-')) {
			const char *arg = args;
			optopt = argv[optind++][1];
			while (*arg) {
				if (optopt == *arg) {
					if (arg[1] == ':') {
						if (argv[optind - 1][2]) {
							optarg = argv[optind - 1] + 2;
						} else if (optind < argc) {
							optarg = argv[optind++];
						} else {
							return '?';
						}
					}
					return optopt;
				}
				arg++;
			}
			return '?';
		}
	}
	return -1;
}
