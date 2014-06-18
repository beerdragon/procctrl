/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_params_h
#define __inc_params_h

/// @file
/// @brief Program parameters
///
/// Header file for the global variables corresponding to the program
/// parameters defined in params.c.

#ifdef _WIN32

#include <Windows.h>

#define _WIN32_OR_POSIX(a,b) a

#else /* ifdef _WIN32 */

#include <sys/types.h>

#define _WIN32_OR_POSIX(a,b) b

#endif /* ifdef _WIN32 */

#ifndef MODULE_VAR_EXTERN
/// @brief Declares a reference to a value defined by params.c
# define MODULE_VAR_EXTERN extern
#endif /* ifndef MODULE_VAR_EXTERN */
#ifndef MODULE_VAR_CONST
/// @brief Declares a read-only reference to a value defined by params.c
# define MODULE_VAR_CONST const
#endif /* ifndef MODULE_VAR_CONST */

/// @brief Disable houskeeping
#define HOUSEKEEP_NONE      0
/// @brief Run housekeeping actions before the main operation
#define HOUSEKEEP_BEFORE    1
/// @brief Run housekeeping actions after the main operation
#define HOUSEKEEP_AFTER     2
/// @brief Run housekeeping actions both before and after the main operation
#define HOUSEKEEP_FULL      (HOUSEKEEP_BEFORE | HOUSEKEEP_AFTER)

/// @brief The `d` parameter
MODULE_VAR_EXTERN char const * MODULE_VAR_CONST data_dir;
/// @brief The `K` parameter
MODULE_VAR_EXTERN int MODULE_VAR_CONST global_identifier;
/// @brief The `k` parameter
MODULE_VAR_EXTERN char const * MODULE_VAR_CONST process_identifier;
/// @brief The `P` parameter
MODULE_VAR_EXTERN _WIN32_OR_POSIX (HANDLE, pid_t) MODULE_VAR_CONST parent_process;
/// @brief The `p` parameter
MODULE_VAR_EXTERN int MODULE_VAR_CONST watch_parent;
/// @brief The `v` parameter
MODULE_VAR_EXTERN int MODULE_VAR_CONST verbose;
/// @brief The control operation
MODULE_VAR_EXTERN char const * MODULE_VAR_CONST operation;
/// @brief The number of spawn arguments (the first is the process to spawn)
MODULE_VAR_EXTERN int MODULE_VAR_CONST spawn_argc;
/// @brief The spawn arguments (the first is the process to spawn)
MODULE_VAR_EXTERN char ** MODULE_VAR_CONST spawn_argv;
/// @brief The `H` parameter
MODULE_VAR_EXTERN int MODULE_VAR_CONST housekeep_mode;

int params (int argc, char **argv);
int params_v (int argc, ...);

#endif /* ifndef __inc_params_h */
