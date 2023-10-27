#ifndef _MEMORY_H
#define _MEMORY_H

#include "libs.h"

// Tipo de algoritmo de substituicao de paginas de forma generica no prototipo da funcao
typedef void (*PageReplaceAlgorithm)(void **memoryType, void *entry, int size);

// Realiza a logica principal do gerenciamento de memoria
// Leitura da memoria auxiliar, busca nas tabelas e TLB, e chama substituicao de paginas
void memoryManagement(void *memory, FILE *testFile, PageReplaceAlgorithm pageReplaceAlgorithm);

// Calcula endereço físico e imprime na tela
void printAddress(FILE *output, int *offsetBin, int *frameBin, int logicalDecimal, int toPrint);

// Algoritmos de substituicao de paginas
void FIFOAlgorithm(void **memoryType, void *entry, int size);
void LRUAlgorithm(void **memoryType, void *entry, int size);

// Cria a estrutura principal da memoria com os tamanhos estabelecidos
void *createMemory(int frames);

// Cria a estrutura de uma entrada da TLB e da tabela de paginas
void *createEntry(int *pageNumber, int *frameNumber);

// Cria a estrutura de um frame da memoria
void *createFrame(int *frameNumber);

// Cria a estrutura de um endereco de memoria
void *createAddress();

// Converte o decimal lido no endereco logico para binario
void *decimalToBinary(int decimal, int size);

// Converte um binario para decimal
int binaryToDecimal(int *binary, int size);

// Separa o binario em endereco, pagina e offset
void separeteAddress(void *addr);
int *getOffSet(int *binary);
int *getPageNumber(int *binary);
int *getLogicalAddress(int *binary);

// Comparacao dois binarios
bool compareBinarys(int *binary1, int *binary2);

// Printa um binario
void printBinary(int *binary, int size);

// Busca pela pagina na TLB
void *lookForPageTLB(void *memory, int *page);

// Busca pela pagina na tabela de paginas
void *lookForPageTable(void *memory, int *page);

// Libera a memoria alocada
void freeMemory(void *memory);

#endif