#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void *thread1(void *arg) {
    pthread_mutex_lock(&lock1);
    printf("Thread 1: Holding lock 1...\n");
    sleep(1);
    printf("Thread 1: Waiting for lock 2...\n");
    pthread_mutex_lock(&lock2);
    printf("Thread 1: Holding lock 1 & 2...\n");
    pthread_mutex_unlock(&lock2);
    pthread_mutex_unlock(&lock1);
    pthread_exit(NULL);
}

void *thread2(void *arg) {
    pthread_mutex_lock(&lock2);
    printf("Thread 2: Holding lock 2...\n");
    sleep(1);
    printf("Thread 2: Waiting for lock 1...\n");
    pthread_mutex_lock(&lock1);
    printf("Thread 2: Holding lock 1 & 2...\n");
    pthread_mutex_unlock(&lock1);
    pthread_mutex_unlock(&lock2);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t t1, t2;

    // Create threads
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}