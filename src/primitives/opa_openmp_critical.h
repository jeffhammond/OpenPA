/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2014 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef OPA_OPENMP_CRITICAL_H_INCLUDED
#define OPA_OPENMP_CRITICAL_H_INCLUDED

#ifndef _OPENMP
# error OpenMP support not available!!!
#else

#define OPA_IPC_SINGLE_CS_ENTER(msg)          \
    _Pragma("omp critical") {

#define OPA_IPC_SINGLE_CS_EXIT(msg)           \
    }

typedef struct { volatile int v;  } OPA_int_t;
typedef struct { int * volatile v; } OPA_ptr_t;

#define OPA_INT_T_INITIALIZER(val_) { (val_) }
#define OPA_PTR_T_INITIALIZER(val_) { (val_) }

/*
    Emulated atomic primitives
    --------------------------

    These are versions of the atomic primitives that emulate the proper behavior
    via the use of OpenMP critical section.
*/

static _opa_inline int OPA_load_int(_opa_const OPA_int_t *ptr)
{
    int retval;
    OPA_IPC_SINGLE_CS_ENTER("atomic_add");
    retval = ptr->v;
    OPA_IPC_SINGLE_CS_EXIT("atomic_add");
    return retval;
}

static _opa_inline void OPA_store_int(OPA_int_t *ptr, int val)
{
    OPA_IPC_SINGLE_CS_ENTER("atomic_add");
    ptr->v = val;
    OPA_IPC_SINGLE_CS_EXIT("atomic_add");
}

static _opa_inline void *OPA_load_ptr(_opa_const OPA_ptr_t *ptr)
{
    int *retval;
    OPA_IPC_SINGLE_CS_ENTER("atomic_add");
    retval = ptr->v;
    OPA_IPC_SINGLE_CS_EXIT("atomic_add");
    return retval;
}

static _opa_inline void OPA_store_ptr(OPA_ptr_t *ptr, void *val)
{
    OPA_IPC_SINGLE_CS_ENTER("atomic_add");
    ptr->v = val;
    OPA_IPC_SINGLE_CS_EXIT("atomic_add");
}

/* normal loads/stores are fully ordered, so just use them */
#define OPA_load_acquire_int(ptr_)       OPA_load_int((ptr_))
#define OPA_store_release_int(ptr_,val_) OPA_store_int((ptr_),(val_))
#define OPA_load_acquire_ptr(ptr_)       OPA_load_ptr((ptr_))
#define OPA_store_release_ptr(ptr_,val_) OPA_store_ptr((ptr_),(val_))

static _opa_inline void OPA_add_int(OPA_int_t *ptr, int val)
{
    OPA_IPC_SINGLE_CS_ENTER("atomic_add");
    ptr->v += val;
    OPA_IPC_SINGLE_CS_EXIT("atomic_add");
}

static _opa_inline void *OPA_cas_ptr(OPA_ptr_t *ptr, int *oldv, int *newv)
{
    int *prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_cas");
    prev = ptr->v;
    if (prev == oldv) {
        ptr->v = newv;
    }
    OPA_IPC_SINGLE_CS_EXIT("atomic_cas");
    return prev;
}

static _opa_inline int OPA_cas_int(OPA_int_t *ptr, int oldv, int newv)
{
    int prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_cas");
    prev = ptr->v;
    if (prev == oldv) {
        ptr->v = newv;
    }
    OPA_IPC_SINGLE_CS_EXIT("atomic_cas");
    return prev;
}

static _opa_inline int OPA_decr_and_test_int(OPA_int_t *ptr)
{
    int new_val;
    OPA_IPC_SINGLE_CS_ENTER("atomic_decr_and_test");
    new_val = --(ptr->v);
    OPA_IPC_SINGLE_CS_EXIT("atomic_decr_and_test");
    return (0 == new_val);
}

static _opa_inline void OPA_decr_int(OPA_int_t *ptr)
{
    OPA_IPC_SINGLE_CS_ENTER("atomic_decr");
    --(ptr->v);
    OPA_IPC_SINGLE_CS_EXIT("atomic_decr");
}

static _opa_inline int OPA_fetch_and_add_int(OPA_int_t *ptr, int val)
{
    int prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_fetch_and_add");
    prev = ptr->v;
    ptr->v += val;
    OPA_IPC_SINGLE_CS_EXIT("atomic_fetch_and_add");
    return prev;
}

static _opa_inline int OPA_fetch_and_decr_int(OPA_int_t *ptr)
{
    int prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_fetch_and_decr");
    prev = ptr->v;
    --(ptr->v);
    OPA_IPC_SINGLE_CS_EXIT("atomic_fetch_and_decr");
    return prev;
}

static _opa_inline int OPA_fetch_and_incr_int(OPA_int_t *ptr)
{
    int prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_fetch_and_incr");
    prev = ptr->v;
    ++(ptr->v);
    OPA_IPC_SINGLE_CS_EXIT("atomic_fetch_and_incr");
    return prev;
}

static _opa_inline void OPA_incr_int(OPA_int_t *ptr)
{
    OPA_IPC_SINGLE_CS_ENTER("atomic_incr");
    ++(ptr->v);
    OPA_IPC_SINGLE_CS_EXIT("atomic_incr");
}

static _opa_inline void *OPA_swap_ptr(OPA_ptr_t *ptr, void *val)
{
    int *prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_swap_ptr");
    prev = ptr->v;
    ptr->v = val;
    OPA_IPC_SINGLE_CS_EXIT("atomic_swap_ptr");
    return prev;
}

static _opa_inline int OPA_swap_int(OPA_int_t *ptr, int val)
{
    int prev;
    OPA_IPC_SINGLE_CS_ENTER("atomic_swap_int");
    prev = ptr->v;
    ptr->v = val;
    OPA_IPC_SINGLE_CS_EXIT("atomic_swap_int");
    return (int)prev;
}

#define OMP_MEMORY_BARRIER _Pragma("omp flush")

/* lock/unlock provides barrier */
#define OPA_write_barrier()      OMP_MEMORY_BARRIER
#define OPA_read_barrier()       OMP_MEMORY_BARRIER
#define OPA_read_write_barrier() OMP_MEMORY_BARRIER

#endif // OpenMP any

#endif /* !defined(OPA_OPENMP_CRITICAL_H_INCLUDED) */
