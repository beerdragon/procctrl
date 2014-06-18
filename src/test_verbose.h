/*
 * Process control utility
 *
 * Copyright 2014 by Andrew Ian William Griffin <griffin@beerdragon.co.uk>
 * Released under the GNU General Public License.
 */

#ifndef __inc_test_verbose_h
#define __inc_test_verbose_h

void _verbose_test ();

#define VERBOSE_WATCH(stream) long stream##c = ftell (stream)

#define VERBOSE_WATCH_ALL \
    VERBOSE_WATCH (stdout); \
    VERBOSE_WATCH (stderr)

#define VERBOSE_EXPECT(stream) \
    { \
        long now = ftell (stream); \
        CU_ASSERT (now != stream##c); \
        stream##c = now; \
    }

#define VERBOSE_SILENT(stream) CU_ASSERT (stream##c == ftell (stream))

#define VERBOSE_STDERR_ONLY \
    VERBOSE_EXPECT (stderr); \
    VERBOSE_SILENT (stdout)

#define VERBOSE_STDOUT_ONLY \
    VERBOSE_EXPECT (stdout); \
    VERBOSE_SILENT (stderr)

#define VERBOSE_SILENT_ALL \
    VERBOSE_SILENT (stdout); \
    VERBOSE_SILENT (stderr)

#define VERBOSE_AND_QUIET_TEST(testcase) \
    static void test_##testcase (void) { \
        VERBOSE_WATCH (stdout); \
        init_##testcase (); \
        do_##testcase (); \
        VERBOSE_SILENT (stdout); \
    } \
    static void test_##testcase##_verbose (void) { \
        VERBOSE_WATCH (stdout); \
        init_##testcase (); \
        _verbose_test (); \
        do_##testcase (); \
        VERBOSE_EXPECT (stdout); \
    }

#endif /* ifndef __inc_test_verbose_h */
