#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>

int main(int argc, char **argv)
{
    int a, b;
    int c;

    //OPA_store_int(&a, 0);
    __atomic_store_n(&a, 0, __ATOMIC_RELAXED);

    //OPA_store_int(&b, 1);
    __atomic_store_n(&b, 1, __ATOMIC_RELAXED);

    //OPA_add_int(&a, 10);
    __atomic_fetch_add(&a, 10, __ATOMIC_RELAXED);

    //assert(10 == OPA_load_int(&a));
    assert(10 == __atomic_load_n(&a, __ATOMIC_RELAXED));

    //c = OPA_cas_int(&a, 10, 11);
    {
      bool weak = false;
      int old = 10;
      int new = 11;
      __atomic_compare_exchange_n(&a, &old, new, weak,
                                  __ATOMIC_RELAXED,
                                  __ATOMIC_RELAXED);
      c = old;
    }
    assert(10 == c);

    //c = OPA_swap_int(&a, OPA_load_int(&b));
    c = __atomic_exchange_n(&a,
                            __atomic_load_n(&b, __ATOMIC_RELAXED),
                            __ATOMIC_RELAXED);

    assert(11 == c);

    //assert(1 == OPA_load_int(&a));
    assert(1 == __atomic_load_n(&a, __ATOMIC_RELAXED));

    printf("success!\n");

    return 0;
}
