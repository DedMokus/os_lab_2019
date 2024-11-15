#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 100

int k, pnum, mod;
int result[MAX_THREADS];

void *factorial(void *arg) {
    int id = *(int*)arg;
    int start = id * (k/pnum) + 1;
    int end = (id == pnum-1) ? k : (id+1) * (k/pnum);
    int res = 1;
    for (int i = start; i <= end; i++) {
        res = (res * i) % mod;
    }
    result[id] = res;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Usage: %s -k <number> --pnum <number of threads> --mod <modulus>\n", argv[0]);
        return 1;
    }

    k = atoi(argv[2]);
    pnum = atoi(argv[4]);
    mod = atoi(argv[6]);

    //printf("\n%d, %d, %d\n", k, pnum, mod);
    if (pnum > MAX_THREADS) {
        printf("Error: maximum number of threads is %d\n", MAX_THREADS);
        return 1;
    }
    pthread_t threads[MAX_THREADS];
    int ids[MAX_THREADS];
    for (int i = 0; i < pnum; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, factorial, &ids[i]);
    }
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }
    int res = 1;
    for (int i = 0; i < pnum; i++) {
        res = (res * result[i]) % mod;
    }
    printf("%d! mod %d = %d\n", k, mod, res);
    return 0;
}