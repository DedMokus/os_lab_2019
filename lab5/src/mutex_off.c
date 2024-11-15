#include <stdio.h>
#include <stdlib.h>

void do_one_thing(int *);
void do_another_thing(int *);
void do_wrap_up(int);

int common = 0; /* A shared variable for two threads */
int r1 = 0, r2 = 0, r3 = 0;

int main() {
    do_one_thing(&common);
    do_another_thing(&common);
    do_wrap_up(common);

    return 0;
}

void do_one_thing(int *pnum_times) {
    int i, j, x;
    unsigned long k;
    int work;

    for (i = 0; i < 50; i++) {
        printf("doing one thing\n");
        work = *pnum_times;
        printf("counter = %d\n", work);
        work++; /* increment, but not write */
        for (k = 0; k < 500000; k++) ; /* long cycle */
        *pnum_times = work; /* write back */
    }
}

void do_another_thing(int *pnum_times) {
    int i, j, x;
    unsigned long k;
    int work;

    for (i = 0; i < 50; i++) {
        printf("doing another thing\n");
        work = *pnum_times;
        printf("counter = %d\n", work);
        work++; /* increment, but not write */
        for (k = 0; k < 500000; k++) ; /* long cycle */
        *pnum_times = work; /* write back */
    }
}

void do_wrap_up(int counter) {
    int total;
    printf("All done, counter = %d\n", counter);
}