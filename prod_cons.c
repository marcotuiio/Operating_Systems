// Marco Tulio Alves de Barros, Agosto 2023

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *producer(void *data);
void *consumer(void *data);

#define n 10
int running = 1;
int running_time = 3;
int i = 0;
sem_t mutex, empty, full;
int buffer[n];

int main() {
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, n);
    sem_init(&full, 0, 0);

    pthread_t prod_th, cons_th;
    pthread_create(&prod_th, NULL, producer, NULL);
    pthread_create(&cons_th, NULL, consumer, NULL);
    
    sleep(running_time);
    running = 0;
    // printf("\nPRODUCER AND CONSUMER TERMINATED\n");

    pthread_join(prod_th, NULL);
    pthread_join(cons_th, NULL);
    
    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}

void *producer(void *data) {
    while (running) {
        int item = rand() % 10;
        sem_wait(&empty);
        sem_wait(&mutex);
        buffer[i] = item;
        i++;
        printf("ITEM %d CREATED AND INSERTED TO BUFFER AT %d\n", item, i);
        sem_post(&mutex);
        sem_post(&full);
    }
    return NULL;
}

void *consumer(void *data) {
    while(running) {
        sem_wait(&full);
        sem_wait(&mutex);
        int item = buffer[i-1];
        i--;
        printf("ITEM %d REMOVED FROM BUFFER AT %d\n", item, i+1);
        sem_post(&mutex);
        sem_post(&empty);
    }
    return NULL;
}
