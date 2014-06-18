/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_getopt_h
#define __inc_getopt_h

/// @file
/// @brief GNU compatible parameter passing
///
/// Provides a `getopt` sufficiently similar to the GNU one for the Windows
/// build to work.

#ifndef MODULE_VAR_EXTERN
/// @brief Declares a reference to a value defined by getopt.c
# define MODULE_VAR_EXTERN extern
#endif /* ifndef MODULE_VAR_EXTERN */

MODULE_VAR_EXTERN int optind;
MODULE_VAR_EXTERN char *optarg;
MODULE_VAR_EXTERN char optopt;

int getopt (int argc, char **argv, const char *args);

#endif /* ifndef __inc_getopt_h */