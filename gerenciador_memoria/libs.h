#ifndef _LIBS_H
#define _LIBS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define PAGE_ENTRIES 256  // qntd de paginas
#define PAGE_SIZE 256  // tamanho em bytes de cada pagina
#define TLB_ENTRIES 16   // qntd de entradas na cache TLB
#define PHYSICAL_MEM_SIZE 65536  // tamanho em bytes da memoria fisica
#define BIN_FILE "BACKING_STORE.bin"

#endif