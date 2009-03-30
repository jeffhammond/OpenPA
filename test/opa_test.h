/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2008 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef OPA_TEST_NAIVE
#include "opa_primitives.h"
#else /* OPA_TEST_NAIVE */
#include "opa_config.h"
#include "opa_util.h"
#endif /* OPA_TEST_NAIVE */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#if defined(HAVE_PTHREAD_H)
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

/*
 * Naive redefinition of OPA functions as simple C operations.  These should
 * pass all the "simple" tests, but fail on the threaded tests (with >1 thread),
 * unless the tested operation is natively atomic.
 */
#ifdef OPA_TEST_NAIVE

#define OPA_UNIVERSAL_PRIMITIVE OPA_CAS

typedef int OPA_int_t;
typedef void *OPA_ptr_t;

#define OPA_load(A) (*(A))
#define OPA_store(A, B) ((void) (*(A) = (B)))
#define OPA_load_ptr(A) (*(A))
#define OPA_store_ptr(A, B) ((void) (*(A) = (B)))

#define OPA_add(A, B) ((void) (*(A) += (B)))
#define OPA_incr(A) ((void) (++(*(A))))
#define OPA_decr(A) ((void) (--(*(A))))

#define OPA_decr_and_test(A) (0 == --(*(A)))
static inline int OPA_fetch_and_add(OPA_int_t *ptr, int val)
{
    int prev = *ptr;
    *ptr += val;
    return prev;
}
#define OPA_fetch_and_incr(A) ((*(A))++)
#define OPA_fetch_and_decr(A) ((*(A))--)

#define OPA_cas_ptr(A, B, C) (*(A) == (B) ? (*(A) = (C), (B)) : (A))
#define OPA_cas_int(A, B, C) (*(A) == (B) ? (*(A) = (C), (B)) : (A))

static inline void *OPA_swap_ptr(OPA_ptr_t *ptr, void *val)
{
    void *prev;
    prev = *ptr;
    *ptr = val;
    return prev;
}

static inline int OPA_swap_int(OPA_int_t *ptr, int val)
{
    int prev;
    prev = *ptr;
    *ptr = val;
    return prev;
}

#endif /* OPA_TEST_NAIVE */

/*
 * Print the current location on the standard output stream.
 */
#define AT()            printf ("        at %s:%d in %s()...\n",              \
                                __FILE__, __LINE__, __FUNCTION__)

/*
 * The name of the test is printed by saying TESTING("something") which will
 * result in the string `Testing something' being flushed to standard output.
 * If a test passes, fails, or is skipped then the PASSED(), FAILED(), or
 * SKIPPED() macro should be called.  After HFAILED() or SKIPPED() the caller
 * should print additional information to stdout indented by at least four
 * spaces.
 */
#define TESTING(WHAT, NTHREADS)                                                \
do {                                                                           \
    if(NTHREADS) {                                                             \
        int nwritten_chars;                                                    \
        printf("Testing %s with %d thread%s %n",                               \
                WHAT,                                                          \
                (int) NTHREADS,                                                \
                (NTHREADS) > 1 ? "s" : "",                                     \
                &nwritten_chars);                                              \
        printf("%*s", nwritten_chars > 70 ? 0 : 70 - nwritten_chars, "");      \
    } /* end if */                                                             \
    else                                                                       \
        printf("Testing %-62s", WHAT);                                         \
    fflush(stdout);                                                            \
} while(0)
#define PASSED()        do {puts(" PASSED");fflush(stdout);} while(0)
#define FAILED()        do {puts("*FAILED*");fflush(stdout);} while(0)
#define WARNING()       do {puts("*WARNING*");fflush(stdout);} while(0)
#define SKIPPED()       do {puts(" -SKIP-");fflush(stdout);} while(0)
#define TEST_ERROR      do {FAILED(); AT(); goto error;} while(0)
#define FAIL_OP_ERROR(OP) do {FAILED(); AT(); OP; goto error;} while(0)

/*
 * Array of number of threads.  Each threaded test is run once for each entry in
 * this array.
 */
static const unsigned num_threads[] = {1, 2, 10, 100};
static const unsigned num_thread_tests = sizeof(num_threads) / sizeof(num_threads[0]);

/*
 * Factor to reduce the number of iterations by for each test.  Must be the same
 * size as num_threads.
 */
static const int iter_reduction[] = {1, 1, 1, 10};

/*
 * Other global variables.
 */
static int curr_test;   /* Current test number bein run */
