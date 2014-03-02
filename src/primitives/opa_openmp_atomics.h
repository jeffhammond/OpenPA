/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*  
 *  (C) 2008, 2014 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

/* FIXME needs to be converted to new style functions with OPA_int_t/OPA_ptr_t-types */
#ifndef OPA_OPENMP_ATOMICS_H_INCLUDED
#define OPA_OPENMP_ATOMICS_H_INCLUDED

#ifndef _OPENMP
# error OpenMP support not available!!!
#else
# if (_OPENMP<201107)
#  error You need OpenMP 3.1 or higher for the required atomic support.
# else

/* FIXME do we need to align these? */
typedef struct { volatile int v;    } OPA_int_t;
typedef struct { void * volatile v; } OPA_ptr_t;

#define OPA_INT_T_INITIALIZER(val_) { (val_) }
#define OPA_PTR_T_INITIALIZER(val_) { (val_) }

/* Assume that loads/stores are atomic on the current platform, even though this
   may not be true at all. */
static _opa_inline int OPA_load_int(_opa_const OPA_int_t *ptr)
{
    int retval;
#pragma omp atomic read
    {
        retval = ptr->v;
    }
    return retval;
}

static _opa_inline void OPA_store_int(OPA_int_t *ptr, int val)
{
#pragma omp atomic update
    {
        ptr->v = val;
    }
}

static _opa_inline void *OPA_load_ptr(_opa_const OPA_ptr_t *ptr)
{
    int * retval;
#pragma omp atomic read
    {
        retval = ptr->v;
    }
    return retval;
}

static _opa_inline void OPA_store_ptr(OPA_ptr_t *ptr, void *val)
{
#pragma omp atomic update
    {
        ptr->v = val;
    }
}

#define OPA_load_acquire_int(ptr_)       OPA_load_int((ptr_))
#define OPA_store_release_int(ptr_,val_) OPA_store_int((ptr_),(val_))
#define OPA_load_acquire_ptr(ptr_)       OPA_load_ptr((ptr_))
#define OPA_store_release_ptr(ptr_,val_) OPA_store_ptr((ptr_),(val_))

static _opa_inline int OPA_fetch_and_add_int(OPA_int_t *ptr, int val)
{
    int prev;
#pragma omp atomic capture
    {
        prev = ptr->v;
        ptr->v += val;
    }
    return prev;
}

static _opa_inline int OPA_decr_and_test_int(OPA_int_t *ptr)
{
    int new_val;
#pragma omp atomic capture
    {
        new_val = --(ptr->v);
    }
    return (0 == new_val);
}

#define OPA_fetch_and_incr_int_by_faa OPA_fetch_and_incr_int
#define OPA_fetch_and_decr_int_by_faa OPA_fetch_and_decr_int
#define OPA_add_int_by_faa OPA_add_int
#define OPA_incr_int_by_fai OPA_incr_int
#define OPA_decr_int_by_fad OPA_decr_int

static _opa_inline void *OPA_swap_ptr(OPA_ptr_t *ptr, void *val)
{
    int * prev;
#pragma omp atomic capture
    {
        prev = ptr->v;
        ptr->v = val;
    }
    return prev;
}

static _opa_inline int OPA_swap_int(OPA_int_t *ptr, int val)
{
    int prev;
#pragma omp atomic capture
    {
        prev = ptr->v;
        ptr->v = val;
    }
    return (int)prev;
}

#define OMP_MEMORY_BARRIER { _Pragma("omp flush") }

#define OPA_write_barrier()      OMP_MEMORY_BARRIER
#define OPA_read_barrier()       OMP_MEMORY_BARRIER
#define OPA_read_write_barrier() OMP_MEMORY_BARRIER
#define OPA_compiler_barrier()   do { } while(0);

#include"opa_emulated.h"

#endif // OpenMP 3.1
#endif // OpenMP any

#endif /* OPA_OPENMP_ATOMICS_H_INCLUDED */
