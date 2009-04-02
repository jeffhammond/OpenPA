/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2008 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

/*
 * Uncomment this definition to disable the OPA library and instead use naive
 * (non-atomic) operations.  This should cause failures.
 */
/*
#define OPA_TEST_NAIVE
*/

#include "opa_test.h"

/*
 * Uncomment these lines to disable only memory barriers, while leaving the rest
 * of the OPA functions intact.
 */
/*
#ifdef OPA_write_barrier
#undef OPA_write_barrier
#endif
#define OPA_write_barrier() ((void) 0)
#ifdef OPA_read_barrier
#undef OPA_read_barrier
#endif
#define OPA_read_barrier() ((void) 0)
#ifdef OPA_read_write_barrier
#undef OPA_read_write_barrier
#endif
#define OPA_read_write_barrier() ((void) 0)
*/

/* Definitions for test_barriers_linear_array */
#define LINEAR_ARRAY_NITER 4000000
#define LINEAR_ARRAY_LEN 100

/* Definitions for test_barriers_variables */
#define VARIABLES_NITER 4000000
#define VARIABLES_NVAR 10
typedef struct {
    OPA_int_t *v_0;
    OPA_int_t *v_1;
    OPA_int_t *v_2;
    OPA_int_t *v_3;
    OPA_int_t *v_4;
    OPA_int_t *v_5;
    OPA_int_t *v_6;
    OPA_int_t *v_7;
    OPA_int_t *v_8;
    OPA_int_t *v_9;
} variables_t;

/* Definitions for test_barriers_scattered_array */
#define SCATTERED_ARRAY_SIZE 100000
#define SCATTERED_ARRAY_LOCS {254, 85920, 255, 35529, 75948, 75947, 253, 99999, 11111, 11112}


/*-------------------------------------------------------------------------
 * Function: test_barriers_sanity
 *
 * Purpose: Essentially tests that memory barriers don't interfere with
 *          normal single threaded operations.  If this fails then
 *          something is *very* wrong.
 *
 * Return: Success: 0
 *         Failure: 1
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int test_barriers_sanity(void)
{
    OPA_int_t   a;
    int         b;


    TESTING("memory barrier sanity", 0);

    /* Store 0 in a and b */
    OPA_store(&a, 0);
    OPA_write_barrier();
    b = 0;

    OPA_read_write_barrier();

    /* Add INT_MIN */
    OPA_add(&a, INT_MIN);
    OPA_read_write_barrier();
    b += INT_MIN;

    OPA_read_write_barrier();

    /* Increment */
    OPA_incr(&a);
    OPA_read_write_barrier();
    b++;

    OPA_read_write_barrier();

    /* Add INT_MAX */
    OPA_add(&a, INT_MAX);
    OPA_read_write_barrier();
    b += INT_MAX;

    OPA_read_write_barrier();

    /* Decrement */
    OPA_decr(&a);
    OPA_read_write_barrier();
    b--;

    OPA_read_write_barrier();

    /* Load the result, verify it is correct */
    if(OPA_load(&a) != INT_MIN + 1 + INT_MAX - 1) TEST_ERROR;
    OPA_read_barrier();
    if(b != OPA_load(&a)) TEST_ERROR;

    OPA_read_write_barrier();

    /* Barriers are now the opposite of what they were before */

    /* Store 0 in a */
    OPA_store(&a, 0);
    OPA_read_barrier();
    b = 0;

    /* Add INT_MAX */
    OPA_add(&a, INT_MAX);
    b += INT_MAX;

    /* Decrement */
    OPA_decr(&a);
    b--;

    /* Add INT_MIN */
    OPA_add(&a, INT_MIN);
    b += INT_MIN;

    /* Increment */
    OPA_incr(&a);
    b++;

    /* Load the result, verify it is correct */
    if(OPA_load(&a) != INT_MAX - 1 + INT_MIN + 1) TEST_ERROR;
    OPA_write_barrier();
    if(b != OPA_load(&a)) TEST_ERROR;

    PASSED();
    return 0;

error:
    return 1;
} /* end test_simple_add_incr_decr() */


#if defined(OPA_HAVE_PTHREAD_H)
/*-------------------------------------------------------------------------
 * Function: test_barriers_linear_array_write
 *
 * Purpose: Helper (write thread) routine for test_barriers_linear_array.
 *          Writes successive increments to the shared array with memory
 *          barriers between each increment.
 *
 * Return: NULL
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *test_barriers_linear_array_write(void *_shared_array)
{
    OPA_int_t           *shared_array = (OPA_int_t *)_shared_array;
    int                 niter = LINEAR_ARRAY_NITER / LINEAR_ARRAY_LEN
                                / iter_reduction[curr_test];
    int                 i, j;

    /* Main loop */
    for(i=0; i<niter; i++)
        for(j=0; j<LINEAR_ARRAY_LEN; j++) {
            /* Increment the value in the array */
            OPA_incr(&shared_array[j]);

            /* Write barrier */
            OPA_write_barrier();
        } /* end for */

    /* Exit */
    pthread_exit(NULL);
} /* end test_barriers_linear_array_write() */


/*-------------------------------------------------------------------------
 * Function: test_barriers_linear_array_read
 *
 * Purpose: Helper (read thread) routine for test_barriers_linear_array.
 *          Reads successive increments from the shared array in reverse
 *          order with memory barriers between each read.
 *
 * Return: Success: NULL
 *         Failure: non-NULL
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *test_barriers_linear_array_read(void *_shared_array)
{
    OPA_int_t           *shared_array = (OPA_int_t *)_shared_array;
    int                 read_buffer[LINEAR_ARRAY_LEN];
    int                 niter = LINEAR_ARRAY_NITER / LINEAR_ARRAY_LEN
                                / iter_reduction[curr_test];
    int                 nerrors = 0;    /* Number of errors */
    int                 i, j;

    /* Main loop */
    for(i=0; i<niter; i++) {
        /* Load the values from the array into the read buffer in reverse
         * order */
        for(j = LINEAR_ARRAY_LEN - 1; j >= 0; j--) {
            read_buffer[j] = OPA_load(&shared_array[j]);

            /* Read barrier */
            OPA_read_barrier();
        } /* end for */

        /* Verify that the values never increase when read back in forward
        * order */
         for(j=1; j<LINEAR_ARRAY_LEN; j++)
            if(read_buffer[j-1] < read_buffer[j]) {
                printf("    Unexpected load: %d is less than %d\n",
                        read_buffer[j-1], read_buffer[j]);
                nerrors++;
            } /* end if */
    } /* end for */

    /* Any non-NULL exit value indicates an error, we use &i here */
    pthread_exit(nerrors ? &i : NULL);
} /* end test_barriers_linear_array_read() */
#endif /* OPA_HAVE_PTHREAD_H */


/*-------------------------------------------------------------------------
 * Function: test_barriers_linear_array
 *
 * Purpose: Tests memory barriers using simultaneous reads and writes to
 *          a linear array.  Launches nthreads threads split into read
 *          and write threads.
 *
 * Return: Success: 0
 *         Failure: 1
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int test_barriers_linear_array(void)
{
#if defined(OPA_HAVE_PTHREAD_H)
    pthread_t           *threads = NULL; /* Threads */
    pthread_attr_t      ptattr;         /* Thread attributes */
    static OPA_int_t    shared_array[LINEAR_ARRAY_LEN]; /* Array to operate on */
    void                *ret;           /* Thread return value */
    unsigned            nthreads = num_threads[curr_test];
    int                 nerrors = 0;    /* number of errors */
    int                 i;

    TESTING("memory barriers with linear array", nthreads);

    /* Allocate array of threads */
    if(NULL == (threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t))))
        TEST_ERROR;

    /* Set threads to be joinable */
    pthread_attr_init(&ptattr);
    pthread_attr_setdetachstate(&ptattr, PTHREAD_CREATE_JOINABLE);

    /* Initialize shared array */
    for(i=0; i<LINEAR_ARRAY_LEN; i++)
        OPA_store(&shared_array[i], 0);

    /* Create the threads. */
    for(i=0; i<nthreads; i++) {
        if(pthread_create(&threads[i], &ptattr, test_barriers_linear_array_write,
                shared_array)) TEST_ERROR;
        if(++i < nthreads)
            if(pthread_create(&threads[i], &ptattr, test_barriers_linear_array_read,
                    shared_array)) TEST_ERROR;
    } /* end for */

    /* Free the attribute */
    if(pthread_attr_destroy(&ptattr)) TEST_ERROR;

    /* Join the threads */
    for (i=0; i<nthreads; i++) {
        if(pthread_join(threads[i], &ret)) TEST_ERROR;
        if(ret)
            nerrors++;
    } /* end for */

    /* Check for errors */
    if(nerrors)
        FAIL_OP_ERROR(printf("    Unexpected return from %d thread%s\n", nerrors,
                nerrors == 1 ? "" : "s"));

    /* Free memory */
    free(threads);

    PASSED();

#else /* OPA_HAVE_PTHREAD_H */
    TESTING("memory barriers with linear array", 0);
    SKIPPED();
    puts("    pthread.h not available");
#endif /* OPA_HAVE_PTHREAD_H */

    return 0;

#if defined(OPA_HAVE_PTHREAD_H)
error:
    if(threads) free(threads);
    return 1;
#endif /* OPA_HAVE_PTHREAD_H */
} /* end test_barriers_linear_array() */


#if defined(OPA_HAVE_PTHREAD_H)
/*-------------------------------------------------------------------------
 * Function: test_barriers_variables_write
 *
 * Purpose: Helper (write thread) routine for test_barriers_variables.
 *          Writes successive increments to the shared variables with
 *          memory barriers between each increment.
 *
 * Return: NULL
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *test_barriers_variables_write(void *_udata)
{
    variables_t         *udata = (variables_t *)_udata;
    OPA_int_t           *v_0, *v_1, *v_2, *v_3, *v_4, *v_5, *v_6, *v_7, *v_8, *v_9;
    int                 niter = VARIABLES_NITER / VARIABLES_NVAR
                                / iter_reduction[curr_test];
    int                 i;

    /* Make local copies of the pointers in udata, to maximize the chance of the
     * compiler reordering instructions (if the barriers don't work) */
    v_0 = udata->v_0;
    v_1 = udata->v_1;
    v_2 = udata->v_2;
    v_3 = udata->v_3;
    v_4 = udata->v_4;
    v_5 = udata->v_5;
    v_6 = udata->v_6;
    v_7 = udata->v_7;
    v_8 = udata->v_8;
    v_9 = udata->v_9;

    /* Main loop */
    for(i=0; i<niter; i++) {
        /* Incrememnt the variables in forward order */
        OPA_incr(v_0);
        OPA_write_barrier();
        OPA_incr(v_1);
        OPA_write_barrier();
        OPA_incr(v_2);
        OPA_write_barrier();
        OPA_incr(v_3);
        OPA_write_barrier();
        OPA_incr(v_4);
        OPA_write_barrier();
        OPA_incr(v_5);
        OPA_write_barrier();
        OPA_incr(v_6);
        OPA_write_barrier();
        OPA_incr(v_7);
        OPA_write_barrier();
        OPA_incr(v_8);
        OPA_write_barrier();
        OPA_incr(v_9);
        OPA_write_barrier();
    } /* end for */

    /* Exit */
    pthread_exit(NULL);
} /* end test_barriers_variables_write() */


/*-------------------------------------------------------------------------
 * Function: test_barriers_variables_read
 *
 * Purpose: Helper (read thread) routine for test_barriers_variables.
 *          Reads successive increments from the variables in reverse
 *          order with memory barriers between each read.
 *
 * Return: Success: NULL
 *         Failure: non-NULL
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *test_barriers_variables_read(void *_udata)
{
    variables_t         *udata = (variables_t *)_udata;
    OPA_int_t           *v_0, *v_1, *v_2, *v_3, *v_4, *v_5, *v_6, *v_7, *v_8, *v_9;
    int                 read_buffer[VARIABLES_NVAR];
    int                 niter = VARIABLES_NITER / VARIABLES_NVAR
                                / iter_reduction[curr_test];
    int                 nerrors = 0;    /* Number of errors */
    int                 i, j;

    /* Make local copies of the pointers in udata, to maximize the chance of the
     * compiler reordering instructions (if the barriers don't work) */
    v_0 = udata->v_0;
    v_1 = udata->v_1;
    v_2 = udata->v_2;
    v_3 = udata->v_3;
    v_4 = udata->v_4;
    v_5 = udata->v_5;
    v_6 = udata->v_6;
    v_7 = udata->v_7;
    v_8 = udata->v_8;
    v_9 = udata->v_9;

    /* Main loop */
    for(i=0; i<niter; i++) {
        /* Load the values from the array into the read buffer in reverse
         * order*/
        read_buffer[9] = OPA_load(v_9);
        OPA_read_barrier();
        read_buffer[8] = OPA_load(v_8);
        OPA_read_barrier();
        read_buffer[7] = OPA_load(v_7);
        OPA_read_barrier();
        read_buffer[6] = OPA_load(v_6);
        OPA_read_barrier();
        read_buffer[5] = OPA_load(v_5);
        OPA_read_barrier();
        read_buffer[4] = OPA_load(v_4);
        OPA_read_barrier();
        read_buffer[3] = OPA_load(v_3);
        OPA_read_barrier();
        read_buffer[2] = OPA_load(v_2);
        OPA_read_barrier();
        read_buffer[1] = OPA_load(v_1);
        OPA_read_barrier();
        read_buffer[0] = OPA_load(v_0);
        OPA_read_barrier();

        /* Verify that the values never increase when read back in forward
        * order */
         for(j=1; j<VARIABLES_NVAR; j++)
            if(read_buffer[j-1] < read_buffer[j]) {
                printf("    Unexpected load: %d is less than %d\n",
                        read_buffer[j-1], read_buffer[j]);
                nerrors++;
            } /* end if */
    } /* end for */

    /* Any non-NULL exit value indicates an error, we use &i here */
    pthread_exit(nerrors ? &i : NULL);
} /* end test_barriers_variables_read() */
#endif /* OPA_HAVE_PTHREAD_H */


/*-------------------------------------------------------------------------
 * Function: test_barriers_variables
 *
 * Purpose: Tests memory barriers using simultaneous reads and writes to
 *          a linear array.  Launches nthreads threads split into read
 *          and write threads.
 *
 * Return: Success: 0
 *         Failure: 1
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int test_barriers_variables(void)
{
#if defined(OPA_HAVE_PTHREAD_H)
    pthread_t           *threads = NULL; /* Threads */
    pthread_attr_t      ptattr;         /* Thread attributes */
    OPA_int_t           v_0, v_1, v_2, v_3, v_4, v_5, v_6, v_7, v_8, v_9;
    variables_t         udata;          /* List of pointers to shared variables */
    void                *ret;           /* Thread return value */
    unsigned            nthreads = num_threads[curr_test];
    int                 nerrors = 0;    /* number of errors */
    int                 i;

    TESTING("memory barriers with local variables", nthreads);

    /* Allocate array of threads */
    if(NULL == (threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t))))
        TEST_ERROR;

    /* Set threads to be joinable */
    pthread_attr_init(&ptattr);
    pthread_attr_setdetachstate(&ptattr, PTHREAD_CREATE_JOINABLE);

    /* Initialize shared variables */
    OPA_store(&v_0, 0);
    OPA_store(&v_1, 0);
    OPA_store(&v_2, 0);
    OPA_store(&v_3, 0);
    OPA_store(&v_4, 0);
    OPA_store(&v_5, 0);
    OPA_store(&v_6, 0);
    OPA_store(&v_7, 0);
    OPA_store(&v_8, 0);
    OPA_store(&v_9, 0);

    /* Initialize udata struct */
    udata.v_0 = &v_0;
    udata.v_1 = &v_1;
    udata.v_2 = &v_2;
    udata.v_3 = &v_3;
    udata.v_4 = &v_4;
    udata.v_5 = &v_5;
    udata.v_6 = &v_6;
    udata.v_7 = &v_7;
    udata.v_8 = &v_8;
    udata.v_9 = &v_9;

    /* Create the threads. */
    for(i=0; i<nthreads; i++) {
        if(pthread_create(&threads[i], &ptattr, test_barriers_variables_write,
                &udata)) TEST_ERROR;
        if(++i < nthreads)
            if(pthread_create(&threads[i], &ptattr, test_barriers_variables_read,
                    &udata)) TEST_ERROR;
    } /* end for */

    /* Free the attribute */
    if(pthread_attr_destroy(&ptattr)) TEST_ERROR;

    /* Join the threads */
    for (i=0; i<nthreads; i++) {
        if(pthread_join(threads[i], &ret)) TEST_ERROR;
        if(ret)
            nerrors++;
    } /* end for */

    /* Check for errors */
    if(nerrors)
        FAIL_OP_ERROR(printf("    Unexpected return from %d thread%s\n", nerrors,
                nerrors == 1 ? "" : "s"));

    /* Free memory */
    free(threads);

    PASSED();

#else /* OPA_HAVE_PTHREAD_H */
    TESTING("memory barriers with local variables", 0);
    SKIPPED();
    puts("    pthread.h not available");
#endif /* OPA_HAVE_PTHREAD_H */

    return 0;

#if defined(OPA_HAVE_PTHREAD_H)
error:
    if(threads) free(threads);
    return 1;
#endif /* OPA_HAVE_PTHREAD_H */
} /* end test_barriers_variables() */


/*-------------------------------------------------------------------------
 * Function: test_barriers_scattered_array
 *
 * Purpose: Tests memory barriers using simultaneous reads and writes to
 *          locations scattered in an array.  Launches nthreads threads
 *          split into read and write threads.
 *
 * Return: Success: 0
 *         Failure: 1
 *
 * Programmer: Neil Fortner
 *             Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int test_barriers_scattered_array(void)
{
#if defined(OPA_HAVE_PTHREAD_H)
    pthread_t           *threads = NULL; /* Threads */
    pthread_attr_t      ptattr;         /* Thread attributes */
    static OPA_int_t    shared_array[SCATTERED_ARRAY_SIZE];
    int                 shared_locs[VARIABLES_NVAR] = SCATTERED_ARRAY_LOCS;
    variables_t         udata;          /* List of pointers to shared variables */
    void                *ret;           /* Thread return value */
    unsigned            nthreads = num_threads[curr_test];
    int                 nerrors = 0;    /* number of errors */
    int                 i;

    TESTING("memory barriers with scattered array", nthreads);

    /* Allocate array of threads */
    if(NULL == (threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t))))
        TEST_ERROR;

    /* Set threads to be joinable */
    pthread_attr_init(&ptattr);
    pthread_attr_setdetachstate(&ptattr, PTHREAD_CREATE_JOINABLE);

    /* Initialize shared variables */
    for(i=0; i<VARIABLES_NVAR; i++)
        OPA_store(&shared_array[shared_locs[i]], 0);

    /* Initialize udata struct */
    udata.v_0 = &shared_array[shared_locs[0]];
    udata.v_1 = &shared_array[shared_locs[1]];
    udata.v_2 = &shared_array[shared_locs[2]];
    udata.v_3 = &shared_array[shared_locs[3]];
    udata.v_4 = &shared_array[shared_locs[4]];
    udata.v_5 = &shared_array[shared_locs[5]];
    udata.v_6 = &shared_array[shared_locs[6]];
    udata.v_7 = &shared_array[shared_locs[7]];
    udata.v_8 = &shared_array[shared_locs[8]];
    udata.v_9 = &shared_array[shared_locs[9]];

    /* Create the threads.  We will use the helper routines for
     * test_barriers_variables. */
    for(i=0; i<nthreads; i++) {
        if(pthread_create(&threads[i], &ptattr, test_barriers_variables_write,
                &udata)) TEST_ERROR;
        if(++i < nthreads)
            if(pthread_create(&threads[i], &ptattr, test_barriers_variables_read,
                    &udata)) TEST_ERROR;
    } /* end for */

    /* Free the attribute */
    if(pthread_attr_destroy(&ptattr)) TEST_ERROR;

    /* Join the threads */
    for (i=0; i<nthreads; i++) {
        if(pthread_join(threads[i], &ret)) TEST_ERROR;
        if(ret)
            nerrors++;
    } /* end for */

    /* Check for errors */
    if(nerrors)
        FAIL_OP_ERROR(printf("    Unexpected return from %d thread%s\n", nerrors,
                nerrors == 1 ? "" : "s"));

    /* Free memory */
    free(threads);

    PASSED();

#else /* OPA_HAVE_PTHREAD_H */
    TESTING("memory barriers with scattered array", 0);
    SKIPPED();
    puts("    pthread.h not available");
#endif /* OPA_HAVE_PTHREAD_H */

    return 0;

#if defined(OPA_HAVE_PTHREAD_H)
error:
    if(threads) free(threads);
    return 1;
#endif /* OPA_HAVE_PTHREAD_H */
} /* end test_barriers_scattered_array() */


/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Tests the opa memory barriers
 *
 * Return:      Success:        exit(0)
 *
 *              Failure:        exit(1)
 *
 * Programmer:  Neil Fortner
 *              Wednesday, April 1, 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int main(int argc, char **argv)
{
    unsigned nerrors = 0;

    /* Simple tests */
    nerrors += test_barriers_sanity();

    /* Loop over test configurations */
    for(curr_test=0; curr_test<num_thread_tests; curr_test++) {
        /* Don't test with only 1 thread */
        if(num_threads[curr_test] == 1)
            continue;

        /* Threaded tests */
        nerrors += test_barriers_linear_array();
        nerrors += test_barriers_variables();
        nerrors += test_barriers_scattered_array();
    }

    if(nerrors)
        goto error;
    printf("All barriers tests passed.\n");

    return 0;

error:
    if(!nerrors)
        nerrors = 1;
    printf("***** %d BARRIERS TEST%s FAILED! *****\n",
            nerrors, 1 == nerrors ? "" : "S");
    return 1;
} /* end main() */
