/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*  
 *  (C) 2008 by Argonne National Laboratory.
 *  (C) 2014 by Argonne National Laboratory, Leadership Computing Facility.
 *      See COPYRIGHT in top-level directory.
 */

/* FIXME needs to be converted to new style functions with OPA_int_t/OPA_ptr_t-types */
#ifndef OPA_GCC_NEW_INTRINSICS_H_INCLUDED
#define OPA_GCC_NEW_INTRINSICS_H_INCLUDED

typedef int OPA_int_t;

/* TODO Is this really the right type to use here? */
#include <stdint.h>
typedef intptr_t OPA_ptr_t;

#define OPA_INT_T_INITIALIZER(val_) (val_)
#define OPA_PTR_T_INITIALIZER(val_) (val_)

static _opa_inline int OPA_load_int(_opa_const OPA_int_t *ptr)
{
    return __atomic_load_n(ptr, __ATOMIC_RELAXED);
}

static _opa_inline void OPA_store_int(OPA_int_t *ptr, int val)
{
    __atomic_store_n(ptr, val, __ATOMIC_RELAXED);
}

static _opa_inline void *OPA_load_ptr(_opa_const OPA_ptr_t *ptr)
{
    return (void*)__atomic_load_n(ptr, __ATOMIC_RELAXED);
}

static _opa_inline void OPA_store_ptr(OPA_ptr_t *ptr, void *val)
{
    __atomic_store_n(ptr, (intptr_t)val, __ATOMIC_RELAXED);
}

static _opa_inline int OPA_load_acquire_int(_opa_const OPA_int_t *ptr)
{
    return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
}

static _opa_inline void OPA_store_release_int(OPA_int_t *ptr, int val)
{
    __atomic_store_n(ptr, val, __ATOMIC_RELEASE);
}

static _opa_inline void *OPA_load_acquire_ptr(_opa_const OPA_ptr_t *ptr)
{
    return (void*)__atomic_load_n(ptr, __ATOMIC_ACQUIRE);
}

static _opa_inline void OPA_store_release_ptr(OPA_ptr_t *ptr, void *val)
{
    __atomic_store_n(ptr, (intptr_t)val, __ATOMIC_RELEASE);
}

static _opa_inline void OPA_add_int(OPA_int_t *ptr, int val)
{
    __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}

static _opa_inline int OPA_fetch_and_add_int(OPA_int_t *ptr, int val)
{
    return __atomic_fetch_add(ptr, val, __ATOMIC_RELAXED);
}

static _opa_inline int OPA_fetch_and_decr_int(OPA_int_t *ptr)
{
    return __atomic_fetch_add(ptr, -1, __ATOMIC_RELAXED);
}

static _opa_inline int OPA_fetch_and_incr_int(OPA_int_t *ptr)
{
    return __atomic_fetch_add(ptr, 1, __ATOMIC_RELAXED);
}

static _opa_inline void OPA_incr_int(OPA_int_t *ptr)
{
    __atomic_fetch_add(ptr, 1, __ATOMIC_RELAXED);
}

static _opa_inline void OPA_decr_int(OPA_int_t *ptr)
{
    __atomic_fetch_add(ptr, -1, __ATOMIC_RELAXED);
}

static _opa_inline int OPA_decr_and_test_int(OPA_int_t *ptr)
{
    return (1 == __atomic_fetch_add(ptr, -1, __ATOMIC_RELAXED));
}

/* Dave Goodell says weak is fine.  OpenPA does not expect strong. */

#include <stdbool.h>

static const bool weak = false;

static _opa_inline void *OPA_cas_ptr(OPA_ptr_t *ptr, void *comparand, void *swaperand)
{
    __atomic_compare_exchange_n(ptr, (intptr_t*)&comparand, (intptr_t)swaperand, weak, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    return comparand;
}

static _opa_inline int OPA_cas_int(OPA_int_t *ptr, int comparand, int swaperand)
{
    __atomic_compare_exchange_n(ptr, &comparand, swaperand, weak, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    return comparand;
}

static _opa_inline void *OPA_swap_ptr(OPA_ptr_t *ptr, void *val)
{
    return (void*)__atomic_exchange_n(ptr, (intptr_t)val, __ATOMIC_RELAXED);
}

static _opa_inline int OPA_swap_int(OPA_int_t *ptr, int val)
{
    return __atomic_exchange_n(ptr, val, __ATOMIC_RELAXED);
}

/* Dave says that read/write don't match acq/rel perfectly so we use heavy hammer. */
#define OPA_write_barrier()      __atomic_thread_fence(__ATOMIC_ACQ_REL);
#define OPA_read_barrier()       __atomic_thread_fence(__ATOMIC_ACQ_REL);
#define OPA_read_write_barrier() __atomic_thread_fence(__ATOMIC_ACQ_REL);
#define OPA_compiler_barrier()   __atomic_signal_fence(__ATOMIC_ACQ_REL);

#endif /* OPA_GCC_NEW_INTRINSICS_H_INCLUDED */
