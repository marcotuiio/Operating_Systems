#include "libs.h"  // OLHAR O ARQUIVO libs.h e fazer alteracoes se necessario
#include "memory.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("ERRO Numero de argumentos invalido!\nUsage: ./<executavel> <arq_teste> <frames> <tipo_algoritmo>\n");
        return 1;
    }

    FILE *test = fopen(argv[1], "r");
    if (!test) {
        printf("ERRO Arquivo de teste nao encontrado!\n");
        return 1;
    }

    void *memory = createMemory(atoi(argv[2]));
    
    if (!strcmp(argv[3], "fifo") || !strcmp(argv[3], "FIFO")) {
        memoryManagement(memory, test, FIFO);

    } else if (!strcmp(argv[3], "lru") || !strcmp(argv[3], "LRU")) {
        memoryManagement(memory, test, LRU);

    } else {
        printf("ERRO Algoritmo nao reconhecido!\nApenas FIFO ou LRU disponiveis\n");
    }

    fclose(test);
    freeMemory(memory);

    return 0;
}
