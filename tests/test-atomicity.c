/* Test program for atomicity from https://en.cppreference.com/w/c/language/atomic */

#include <catomics.h>
#include <stdio.h>
#include <assert.h>
#include <cthreads.h>

atomic_size_t acnt;
size_t cnt = 0;

int f(void *thr_data) {
    (void)thr_data;
    int n;
    for (n = 0; n < 1000; ++n) {
        ++cnt;
        // ++acnt;
        // for this example, relaxed memory order is sufficient, e.g.
        atomic_fetch_add(&acnt, 1);
    }
    return 0;
}
#define THREAD_COUNT 10

int main(void) {
    thrd_t thread[THREAD_COUNT];
    int i, counter = 1;
    atomic_init(&acnt, 0);
    while (1) {
        for (i = 0; i < THREAD_COUNT; ++i) {
            thrd_create(&thread[i], f, NULL);
        }

        for (i = 0; i < 10; ++i) {
            thrd_join(thread[i], NULL);
        }

        if (acnt != cnt) {
            assert(acnt > cnt);
            break;
        }

        counter++;
    }

    printf("Found atomicity, took %d tries in %d threads to detect race!\n", counter, THREAD_COUNT);
    printf("The atomic counter is %zu.\n", acnt);
    printf("The non-atomic counter is %zu, bad!\n", cnt);

    return 0;
}
