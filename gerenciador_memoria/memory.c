#include "memory.h"

#include "libs.h"

int OLDEST_TBL = 0;
int OLDEST_PHYSICAL = 0;
time_t seconds;

typedef struct memory {
    int hitTlb;             // contador de acertos na TLB
    int hitPage;            // contador de acertos na page table
    int pageFault;          // contador de page faults
    int frames;             // qntd de frames
    int framesSize;         // tamanho em bytes de cada frame
    void **TLB;             // cache TLB
    void **pageTable;       // tabela de paginas
    void **physicalMemory;  // memoria fisica
} Memory;

typedef struct entry {
    int *pageNumber;
    int *frameNumber;
    double timeEntry;
} Entry;

typedef struct frame {
    int *frameNumber;
    void *data;
    double timeFrame;
} Frame;

typedef struct address {
    // int *binary;          // 32 bits
    int *offset;          // 8 bits menos significativos
    int *pageNumber;      // 8 bits do meio
    int *logicalAddress;  // 16 bits mais significativos
    int decimal;          // numero decimal lido do arquivo
    int value;            // valor armazenado no endereco
} Address;

void memoryManagement(void *memory, FILE *testFile, PageReplaceAlgorithm pageReplaceAlgorithm) {
    Memory *mem = memory;
    int totalAccess = 0;
    FILE *bin = fopen(BIN_FILE, "r");
    FILE *output = NULL;

    clock_t begin = clock();

    if (pageReplaceAlgorithm == FIFO) {
        output = fopen(OUTPUT_FILE_FIFO, "w");
    } else {
        output = fopen(OUTPUT_FILE_LRU, "w");
    }

    while (!feof(testFile)) {
        Address *address = NULL;
        Frame *frame = NULL;
        int toPrint = 0;

        int logicalDecimal;
        fscanf(testFile, "%d", &logicalDecimal);

        int *logicalBinary = decimalToBinary(logicalDecimal, 32);
        int *pageNumber = getPageNumber(logicalBinary);
        int page = binaryToDecimal(pageNumber, 8);
        int *offsetBin = getOffSet(logicalBinary);
        int offset = binaryToDecimal(offsetBin, 8);

        free(logicalBinary);
        Entry *foundFrame = lookForPageTLB(mem, pageNumber);  // binary frame number
        if (!foundFrame) {                                    // TLB MISS

            foundFrame = lookForPageTable(mem, pageNumber);
            if (!foundFrame) {  // PAGE TABLE MISS

                fseek(bin, page, SEEK_SET);  // Go to page in bin file
                frame = createFrame();       // ta vazando memoria aq
                Address **data = frame->data;

                for (int i = 0; i < mem->framesSize; i++) {  // Writing data to frame
                    address = createAddress();
                    address->offset = decimalToBinary(i, 8);
                    address->pageNumber = pageNumber;
                    fscanf(bin, "%d", &address->value);
                    data[i] = address;
                    if (offset == i) {
                        toPrint = address->value;
                    }
                }

                // Adding frame to physical memory
                pageReplaceAlgorithm(mem, frame, NULL, 1);

                Entry *entry = createEntry(pageNumber, frame->frameNumber);

                addToPageTable(mem, entry, PAGE_ENTRIES);

                // Adding new entry to the created frame in the TLB
                pageReplaceAlgorithm(mem, NULL, entry, 0);

                printAddress(output, offsetBin, frame->frameNumber, logicalDecimal, toPrint, "AUX");
                mem->pageFault++;

            } else {  // PAGE TABLE HIT
                // printf("PAGE TABLE HIT\n");
                // printBinary(foundFrame, 8);
                int frameNumber = binaryToDecimal(((Entry *)foundFrame)->frameNumber, 8);
                Frame **auxFrame = (Frame **)mem->physicalMemory;
                Address **data = auxFrame[frameNumber]->data;
                int valor = data[offset]->value;
                printAddress(output, offsetBin, ((Entry *)foundFrame)->frameNumber, logicalDecimal, valor, "PT");
            }

        } else {  // TLB HIT
            // printBinary(foundFrame, 8);
            // printf("TLB HIT %p\n", foundFrame);
            int frameNumber = binaryToDecimal(((Entry *)foundFrame)->frameNumber, 8);
            Frame **auxFrame = (Frame **)mem->physicalMemory;
            Address **data = auxFrame[frameNumber]->data;
            int valor = data[offset]->value;
            printAddress(output, offsetBin, ((Entry *)foundFrame)->frameNumber, logicalDecimal, valor, "TLB");
        }

        free(offsetBin);
        totalAccess++;
    }
    clock_t end = clock();
    double timeSpent = (double)(end - begin) / CLOCKS_PER_SEC;

    printStats(output, mem, totalAccess, pageReplaceAlgorithm, timeSpent);

    fclose(output);
    fclose(bin);
}

void printAddress(FILE *output, int *offsetBin, int *frameBin, int logicalDecimal, int toPrint, char *in) {
    int *physicalAddress = calloc(16, sizeof(int));
    for (int i = 0; i < 8; i++) {
        physicalAddress[i] = offsetBin[i];
    }
    for (int i = 8; i < 16; i++) {
        physicalAddress[i] = frameBin[i - 8];
    }

    fprintf(output, "%s Virtual address: %d Physical address: %d Value: %d\n", in, logicalDecimal, binaryToDecimal(physicalAddress, 16), toPrint);
    free(physicalAddress);
}

void printStats(FILE *output, void *memory, int totalAccess, PageReplaceAlgorithm pageReplaceAlgorithm, double timeSpent) {
    Memory *mem = memory;

    fprintf(output, "\n\nAluno: Marco Tulio Alves de Barros\n");
    fprintf(output, "Resultados para %s com %d frames\n", pageReplaceAlgorithm == FIFO ? "FIFO" : "LRU", mem->frames);
    fprintf(output, "Running time: %f\n", timeSpent);
    fprintf(output, "\n+------------------+------------+-----------------+\n");
    fprintf(output, "| %-16s | %-10s | %-15s |\n", "CATEGORY", "COUNT", "RATE");
    fprintf(output, "+------------------+------------+-----------------+\n");
    fprintf(output, "| %-16s | %-10d | %-15s |\n", "TOTAL ACCESS", totalAccess, "-");
    fprintf(output, "| %-16s | %-10d | %.2f%%          |\n", "PAGE HITS", mem->hitPage + mem->hitTlb, (float)(mem->hitPage + mem->hitTlb) / totalAccess * 100);
    fprintf(output, "| %-16s | %-10d | %.2f%%          |\n", "PAGE FAULT", mem->pageFault, (float)mem->pageFault / totalAccess * 100);
    fprintf(output, "| %-16s | %-10d | %.2f%%           |\n", "TLB HIT", mem->hitTlb, (float)mem->hitTlb / totalAccess * 100);
    fprintf(output, "| %-16s | %-10d | %.2f%%          |\n", "Page Table HIT", mem->hitPage, (float)mem->hitPage / totalAccess * 100);
    fprintf(output, "+------------------+------------+-----------------+\n");
}

void FIFO(void *mem, void *frame, void *entry, int type) {
    Memory *memory = mem;

    if (type == 0) {
        for (int i = 0; i < TLB_ENTRIES; i++) {
            void **tlb = memory->TLB;
            if (!tlb[i]) {  // ainda tem espaço na tlb
                tlb[i] = entry;
                return;
            }
        }

        // FIFO sem segunda chance para substituir
        memory->TLB[OLDEST_TBL] = entry;
        OLDEST_TBL = (OLDEST_TBL + 1) % TLB_ENTRIES;  

    } else {
        for (int i = 0; i < memory->frames; i++) {
            if (!memory->physicalMemory[i]) {  // tinha espaço na memoria fisica
                memory->physicalMemory[i] = frame;
                ((Frame *)frame)->frameNumber = decimalToBinary(i, 8);
                return;
            }
        }

        // limpando frame mais antigo da memoria fisica
        int pageToRemove;
        Frame *oldest = memory->physicalMemory[OLDEST_PHYSICAL];
        for (int i = 0; i < memory->framesSize; i++) {
            void **data = oldest->data;
            if (data) {
                Address *address = data[i];
                if (address) {
                    pageToRemove = binaryToDecimal(address->pageNumber, 8);
                    if (address->offset) {
                        free(address->offset);
                        address->offset = NULL;
                    }
                    if (address->logicalAddress) {
                        free(address->logicalAddress);
                        address->logicalAddress = NULL;
                    }
                    if (address) free(address);
                }
            }
        }
        if (oldest->data) {
            free(oldest->data);
            oldest->data = NULL;
        } 
            
        if (oldest->frameNumber) {
            free(oldest->frameNumber);
            oldest->frameNumber = NULL;
        }
        free(oldest);
        memory->physicalMemory[OLDEST_PHYSICAL] = NULL;

        updateMemory(memory, pageToRemove);

        ((Frame *) frame)->frameNumber = decimalToBinary(OLDEST_PHYSICAL, 8);
        memory->physicalMemory[OLDEST_PHYSICAL] = frame;
        OLDEST_PHYSICAL = (OLDEST_PHYSICAL + 1) % memory->frames;  // FIFO sem segunda chance

        // a entrada desse elemento na tabela de paginas ou tlb nao foi removida
    }
}

void LRU(void *mem, void *frame, void *entry, int type) {
    Memory *memory = mem;

    if (type == 0) {
        void **tlb = memory->TLB;
        double oldestTime;
        for (int i = 0; i < TLB_ENTRIES; i++) {
            if (tlb[i]) {
                oldestTime = ((Entry *)tlb[i])->timeEntry;
                break;
            }
        }
        int oldestIndex = 0;
        for (int i = 0; i < TLB_ENTRIES; i++) {
            if (!tlb[i]) {
                tlb[i] = entry;
                return;
            }
            if (((Entry *)tlb[i])->timeEntry < oldestTime) {
                oldestTime = ((Entry *)tlb[i])->timeEntry;
                oldestIndex = i;
            }
        }

        tlb[oldestIndex] = entry;
    
    } else {

        double oldestTime;
        for (int i = 0; i < memory->frames; i++) {
            if (memory->physicalMemory[i]) {
                oldestTime = ((Frame *)memory->physicalMemory[i])->timeFrame;
                break;
            }
        }
        int oldestIndex = 0;
        for (int i = 0; i < memory->frames; i++) {
            if (!memory->physicalMemory[i]) {  // tinha espaço na memoria fisica
                memory->physicalMemory[i] = frame;
                ((Frame *)frame)->frameNumber = decimalToBinary(i, 8);
                return;
            }
            if (((Frame *)memory->physicalMemory[i])->timeFrame < oldestTime) {
                oldestTime = ((Frame *)memory->physicalMemory[i])->timeFrame;
                oldestIndex = i;
            }
        }

        // limpando frame mais antigo da memoria fisica
        int pageToRemove;
        Frame *oldest = memory->physicalMemory[oldestIndex];
        for (int i = 0; i < memory->framesSize; i++) {
            void **data = oldest->data;
            if (data) {
                Address *address = data[i];
                if (address) {
                    pageToRemove = binaryToDecimal(address->pageNumber, 8);
                    if (address->offset) {
                        free(address->offset);
                        address->offset = NULL;
                    }
                    if (address->logicalAddress) {
                        free(address->logicalAddress);
                        address->logicalAddress = NULL;
                    }
                    if (address) free(address);
                }
            }
        }
        if (oldest->data) {
            free(oldest->data);
            oldest->data = NULL;
        }
        if (oldest->frameNumber) {
            free(oldest->frameNumber);
            oldest->frameNumber = NULL;
        }
        free(oldest);
        memory->physicalMemory[oldestIndex] = NULL;

        updateMemory(memory, pageToRemove);

        ((Frame *) frame)->frameNumber = decimalToBinary(oldestIndex, 8);
        memory->physicalMemory[oldestIndex] = frame;

        // a entrada desse elemento na tabela de paginas ou tlb nao foi removida
    }
}

void addToPageTable(void *memory, void *entry, int size) {
    Memory *mem = memory;
    for (int i = 0; i < size; i++) {
        if (!mem->pageTable[i]) {
            mem->pageTable[i] = entry;
            return;
        }
    }
}

void updateMemory(void *mem, int pageToRemove) {
    Memory *memory = mem;
    Entry *entryTLB = NULL;
    Entry *entryPageTable = NULL;
    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (!memory->TLB[i]) continue;
        if (binaryToDecimal(((Entry *)memory->TLB[i])->pageNumber, 8) == pageToRemove) {
            entryTLB = memory->TLB[i];
            memory->TLB[i] = NULL;
        }
    }
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        if (!memory->pageTable[i]) continue;
        if (binaryToDecimal(((Entry *)memory->pageTable[i])->pageNumber, 8) == pageToRemove) {
            entryPageTable = memory->pageTable[i];
            memory->pageTable[i] = NULL;
        }
    }

    if (entryTLB == entryPageTable) {
        if (entryTLB->pageNumber) {
            free(entryTLB->pageNumber);
            entryTLB->pageNumber = NULL;
        }
        free(entryTLB);
        entryTLB = NULL;
        return;
    }
    if (entryTLB) {
        if (entryTLB->pageNumber) {
            free(entryTLB->pageNumber);
            entryTLB->pageNumber = NULL;
        }
        free(entryTLB);
        entryTLB = NULL;
        return;
    }
    if (entryPageTable) {
        if (entryPageTable->pageNumber) {
            free(entryPageTable->pageNumber);
            entryPageTable->pageNumber = NULL;
        }
        free(entryPageTable);
        entryPageTable = NULL;
        return;
    }
}

void *createMemory(int frames) {
    Memory *memory = calloc(1, sizeof(Memory));
    memory->frames = frames;
    memory->framesSize = PAGE_SIZE;
    memory->TLB = calloc(TLB_ENTRIES, sizeof(Entry));
    memory->pageTable = calloc(PAGE_ENTRIES, sizeof(Entry));
    memory->physicalMemory = calloc(frames, sizeof(Frame));
    return memory;
}

void *createEntry(int *pageNumber, int *frameNumber) {
    Entry *entry = calloc(1, sizeof(Entry));
    entry->pageNumber = pageNumber;
    entry->frameNumber = frameNumber;
    entry->timeEntry = time(&seconds);
    return entry;
}

void *createFrame() {
    Frame *frame = calloc(1, sizeof(Frame));
    frame->data = calloc(PAGE_SIZE, sizeof(Address));
    frame->timeFrame = time(&seconds);
    return frame;
}

void *createAddress() {
    Address *address = calloc(1, sizeof(Address));
    return address;
}

void *decimalToBinary(int decimal, int size) {
    int *binary = calloc(size, sizeof(int));

    for (int j = 0; j < size; j++) {
        binary[j] = 0;
    }

    int i = 0;
    while (decimal > 0) {
        binary[i] = decimal % 2;
        decimal = decimal / 2;
        i++;
    }

    return binary;
}

int binaryToDecimal(int *binary, int size) {
    int decimal = 0;
    for (int i = 0; i < size; i++) {
        decimal += binary[i] * pow(2, i);
    }
    return decimal;
}

int *getOffSet(int *binary) {
    int *offset = calloc(8, sizeof(int));
    for (int i = 0; i < 8; i++) {
        offset[i] = binary[i];
    }
    return offset;
}

int *getPageNumber(int *binary) {
    int *pageNumber = calloc(8, sizeof(int));
    for (int i = 8; i < 16; i++) {
        pageNumber[i - 8] = binary[i];
    }
    return pageNumber;
}

int *getLogicalAddress(int *binary) {
    int *logicalAddress = calloc(16, sizeof(int));
    for (int i = 16; i < 32; i++) {
        logicalAddress[i - 16] = binary[i];
    }
    return logicalAddress;
}

void *lookForPageTLB(void *memory, int *page) {
    Memory *mem = memory;
    for (int i = 0; i < TLB_ENTRIES; i++) {
        for (int j = 0; j < 8; j++) {
            if (!mem->TLB[i]) continue;
            if (((Entry *)mem->TLB[i])->pageNumber[j] != page[j]) {
                break;
            } else {
                if (j == 7) {
                    // int *foundFrame = ((Entry *)mem->TLB[i])->frameNumber;
                    // printf("TLB HIT\n");
                    // printBinary(foundFrame, 8);
                    ((Entry *)mem->TLB[i])->timeEntry = time(&seconds);
                    mem->hitTlb++;
                    return mem->TLB[i];
                }
            }
        }
    }
    return NULL;
}

void *lookForPageTable(void *memory, int *page) {
    Memory *mem = memory;
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        for (int j = 0; j < 8; j++) {
            if (!mem->pageTable[i]) continue;
            if (((Entry *)mem->pageTable[i])->pageNumber[j] != page[j]) {
                break;
            } else {
                if (j == 7) {
                    // int *foundFrame = ((Entry *)mem->pageTable[i])->frameNumber;
                    // printf("PAGE TABLE HIT\n");
                    // printBinary(foundFrame, 8);
                    ((Entry *)mem->pageTable[i])->timeEntry = time(&seconds);
                    mem->hitPage++;
                    return mem->pageTable[i];
                }
            }
        }
    }
    return NULL;
}

void freeMemory(void *memory) {
    Memory *mem = memory;

    Frame **frames = (Frame **)mem->physicalMemory;
    if (frames) {
        for (int i = 0; i < mem->frames; i++) {
            Frame *f = frames[i];
            if (!f) continue;
            Address **data = f->data;
            if (data) {
                for (int j = 0; j < PAGE_SIZE; j++) {
                    Address *address = data[j];
                    if (!address) continue;
                    if (address->offset) {
                        free(address->offset);
                        address->offset = NULL;
                    }
                    if (address->logicalAddress) {
                        free(address->logicalAddress);
                        address->logicalAddress = NULL;
                    }
                    if (address) free(address);
                }
                free(data);
            }
            if (f->frameNumber) {
                free(f->frameNumber);
                f->frameNumber = NULL;
            }
            free(f);
            f = NULL;
        }
    }

    for (int i = 0; i < PAGE_ENTRIES; i++) {
        Entry *entry = mem->pageTable[i];
        if (!entry) continue;
        if (entry->pageNumber) {
            free(entry->pageNumber);
            entry->pageNumber = NULL;
        }
        free(entry);
        entry = NULL;
    }

    if (mem->TLB) free(mem->TLB);
    if (mem->pageTable) free(mem->pageTable);
    if (mem->physicalMemory) free(mem->physicalMemory);
    if (mem) free(mem);
}