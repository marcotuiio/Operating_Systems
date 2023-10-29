#ifndef _MEMORY_H
#define _MEMORY_H

#include "libs.h"

// Tipo de algoritmo de substituicao de paginas de forma generica no prototipo da funcao
typedef void (*PageReplaceAlgorithm)(void *mem, void *frame, void *entry, int type);

// Realiza a logica principal do gerenciamento de memoria
// Leitura da memoria auxiliar, busca nas tabelas e TLB, e chama substituicao de paginas
void memoryManagement(void *memory, FILE *testFile, PageReplaceAlgorithm pageReplaceAlgorithm);

// Calcula endereço físico e imprime na tela e printa as estatisticas de page fault e tlb hit
void printAddress(FILE *output, int *offsetBin, int *frameBin, int logicalDecimal, int toPrint, char *in);
void printStats(FILE *output, void *memory, int totalAccess, PageReplaceAlgorithm pageReplaceAlgorithm, double timeSpent);

// Algoritmos de substituicao de paginas. 
// Fifo sem segunda chance, leva em consideração apenas a ordem que o elemento foi inserido na memoria
// LRU, leva em consideração a ordem em relação ao tempo que o elemento foi inserido na memoria e a ultima vez que foi acessado
// O update de memoria é para manter a consistencia na TLB e PageTable após uma substituicao na memoria fisica
void FIFO(void *mem, void *frame, void *entry, int type);
void LRU(void *mem, void *frame, void *entry, int type);
void addToPageTable(void *memory, void *entry, int size);
void updateMemory(void *mem, int pageToRemove);

// Cria a estrutura principal da memoria com os tamanhos estabelecidos
void *createMemory(int frames);

// Cria a estrutura de uma entrada da TLB e da tabela de paginas
void *createEntry(int *pageNumber, int *frameNumber);

// Cria a estrutura de um frame da memoria
void *createFrame();

// Cria a estrutura de um endereco de memoria
void *createAddress();

// Converte o decimal lido no endereco logico para binario
void *decimalToBinary(int decimal, int size);

// Converte um binario para decimal
int binaryToDecimal(int *binary, int size);

// Retorna o offset, o numero da pagina e o endereco logico do endeço virtual em binario
int *getOffSet(int *binary);
int *getPageNumber(int *binary);
int *getLogicalAddress(int *binary);

// Busca pela pagina na TLB
void *lookForPageTLB(void *memory, int *page);

// Busca pela pagina na tabela de paginas
void *lookForPageTable(void *memory, int *page);

// Libera a memoria alocada
void freeMemory(void *memory);

#endif