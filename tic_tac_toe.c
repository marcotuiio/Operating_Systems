// Marco Tulio Alves de Barros, Agosto 2023

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *tic(void *data);
void *tac(void *data);
void *toe(void *data);

int running = 1;
int running_time = 2;

sem_t disable_tic, disable_tac, disable_toe;

int main() {
    sem_init(&disable_tic, 0, 1);
    sem_init(&disable_tac, 0, 0);
    sem_init(&disable_toe, 0, 0);
    pthread_t tic_th, tac_th, toe_th;

    pthread_create(&tic_th, NULL, tic, NULL);
    pthread_create(&tac_th, NULL, tac, NULL);
    pthread_create(&toe_th, NULL, toe, NULL);

    sleep(running_time);
    running = 0;
    // printf("\n\nTERMINATED\n");

    sem_destroy(&disable_tic);
    sem_destroy(&disable_tac);
    sem_destroy(&disable_toe); 
    return 0;
}

void *tic(void *data) {
    while(running) {
        sem_wait(&disable_tic);
        printf("\nTIC ");
        sem_post(&disable_tac);
    }
    return NULL;
}

void *tac(void *data) {
    while(running) {
        sem_wait(&disable_tac);
        printf("TAC");
        sem_post(&disable_toe);
    }
    return NULL;
}

void *toe(void *data) {
    while(running) {
        sem_wait(&disable_toe);
        printf(" TOE\n");
        sem_post(&disable_tic);
    }
    return NULL;
}
