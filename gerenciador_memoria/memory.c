#include "memory.h"

#include "libs.h"

int FRAME_NUMBER = 0;
int OLDEST_TBL = 0;
int OLDEST_PT = 0;
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
    double time;
} Entry;

typedef struct frame {
    int *frameNumber;
    bool available;
    void *data;
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
    FILE *output = fopen(OUTPUT_FILE, "w");

    while (!feof(testFile)) {
        Address *address = NULL;
        Frame *frame = NULL;
        int toPrint = 0;
        int frameToSet;

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
                frame = createFrame(decimalToBinary(FRAME_NUMBER, 8));
                Address **data = frame->data;
                FRAME_NUMBER++;

                for (int i = 0; i < 256; i++) {  // Writing data to frame
                    address = createAddress();
                    address->offset = decimalToBinary(i, 8);
                    address->pageNumber = pageNumber;
                    fscanf(bin, "%d", &address->value);
                    data[i] = address;
                    if (offset == i) {
                        toPrint = address->value;
                    }
                }
                frame->available = false;

                for (int i = 0; i < mem->frames; i++) {  // Add frame to physical memory
                    if (!mem->physicalMemory[i]) {
                        mem->physicalMemory[i] = frame;
                        frameToSet = i;
                        break;
                    }
                }

                Entry *entry = createEntry(pageNumber, decimalToBinary(frameToSet, 8));

                pageReplaceAlgorithm(mem->TLB, entry, TLB_ENTRIES);

                pageReplaceAlgorithm(mem->pageTable, entry, PAGE_ENTRIES);

                int *auxFrame = decimalToBinary(frameToSet, 8);
                printAddress(output, offsetBin, auxFrame, logicalDecimal, toPrint);
                free(auxFrame);
                mem->pageFault++;

            } else {  // PAGE TABLE HIT
                // printf("PAGE TABLE HIT\n");
                // printBinary(foundFrame, 8);
                int frameNumber = binaryToDecimal(((Entry *)foundFrame)->frameNumber, 8);
                Frame **auxFrame = (Frame **)mem->physicalMemory;
                Address **data = auxFrame[frameNumber]->data;
                int valor = data[offset]->value;
                printAddress(output, offsetBin, ((Entry *)foundFrame)->frameNumber, logicalDecimal, valor);
            }

        } else {  // TLB HIT
            // printf("TLB HIT\n");
            // printBinary(foundFrame, 8);
            int frameNumber = binaryToDecimal(((Entry *)foundFrame)->frameNumber, 8);
            Frame **auxFrame = (Frame **)mem->physicalMemory;
            Address **data = auxFrame[frameNumber]->data;
            int valor = data[offset]->value;
            printAddress(output, offsetBin, ((Entry *)foundFrame)->frameNumber, logicalDecimal, valor);
        }

        free(offsetBin);
        totalAccess++;
    }

    fprintf(output, "\n\nAluno: Marco Tulio Alves de Barros\n");
    fprintf(output, "Resultados para %s com %d frames\n", pageReplaceAlgorithm == FIFOAlgorithm ? "FIFO" : "LRU", mem->frames);
    fprintf(output, "\n+------------------+------------+-----------------+\n");
    fprintf(output, "| %-16s | %-10s | %-15s |\n", "CATEGORY", "COUNT", "RATE");
    fprintf(output, "+------------------+------------+-----------------+\n");
    fprintf(output, "| %-16s | %-10d | %-15s |\n", "TOTAL ACCESS", totalAccess, "-");
    fprintf(output, "| %-16s | %-10d | %-15s |\n", "PAGE HITS", mem->hitPage + mem->hitTlb, "-");
    fprintf(output, "| %-16s | %-10d | %.2f%%          |\n", "PAGE FAULT", mem->pageFault, (float)mem->pageFault / totalAccess * 100);
    fprintf(output, "| %-16s | %-10d | %.2f%%           |\n", "TLB HIT", mem->hitTlb, (float)mem->hitTlb / totalAccess * 100);
    fprintf(output, "| %-16s | %-10d | %.2f%%          |\n", "Page Table HIT", mem->hitPage, (float)mem->hitPage / totalAccess * 100);
    fprintf(output, "+------------------+------------+-----------------+\n");


    fclose(output);
    fclose(bin);
}

void printAddress(FILE *output, int *offsetBin, int *frameBin, int logicalDecimal, int toPrint) {
    int *physicalAddress = calloc(16, sizeof(int));
    for (int i = 0; i < 8; i++) {
        physicalAddress[i] = offsetBin[i];
    }
    for (int i = 8; i < 16; i++) {
        physicalAddress[i] = frameBin[i - 8];
    }

    fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", logicalDecimal, binaryToDecimal(physicalAddress, 16), toPrint);
    free(physicalAddress);
}

void FIFOAlgorithm(void **memoryType, void *entry, int size) {
    for (int i = 0; i < size; i++) {
        if (!memoryType[i]) {
            memoryType[i] = entry;
            return;
        }
    }

    if (size == 16) {
        memoryType[OLDEST_TBL] = entry;
        OLDEST_TBL = (OLDEST_TBL + 1) % 16;

    } else {
        memoryType[OLDEST_PT] = entry;
        OLDEST_PT = (OLDEST_PT + 1) % 256;
    }
}

void LRUAlgorithm(void **memoryType, void *entry, int size) {
    if (!memoryType[0]) {
        memoryType[0] = entry;
        return;
    }
    double oldest = ((Entry *)memoryType[0])->time;
    int oldestIndex = 0;

    for (int i = 0; i < size; i++) {
        if (!memoryType[i]) {
            memoryType[i] = entry;
            return;
        }
        if (((Entry *)memoryType[i])->time < oldest) {
            oldest = ((Entry *)memoryType[i])->time;
            oldestIndex = i;
        }
    }
    memoryType[oldestIndex] = entry;
}

void *createMemory(int frames) {
    Memory *memory = calloc(1, sizeof(Memory));
    memory->frames = frames;
    memory->framesSize = PHYSICAL_MEM_SIZE / frames;
    memory->TLB = calloc(TLB_ENTRIES, sizeof(Entry));
    memory->pageTable = calloc(PAGE_ENTRIES, sizeof(Entry));
    memory->physicalMemory = calloc(frames, sizeof(Frame));
    return memory;
}

void *createEntry(int *pageNumber, int *frameNumber) {
    Entry *entry = calloc(1, sizeof(Entry));
    entry->pageNumber = pageNumber;
    entry->frameNumber = frameNumber;
    entry->time = time(&seconds);
    return entry;
}

void *createFrame(int *frameNumber) {
    Frame *frame = calloc(1, sizeof(Frame));
    frame->frameNumber = frameNumber;
    frame->available = true;
    frame->data = calloc(PAGE_SIZE, sizeof(Address));
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

bool compareBinarys(int *binary1, int *binary2) {
    for (int i = 0; i < 32; i++) {
        if (binary1[i] != binary2[i]) {
            return false;
        }
    }
    return true;
}

void printBinary(int *binary, int size) {
    for (int i = size - 1; i >= 0; i--) {
        printf("%d", binary[i]);
    }
    printf("\n");
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
                    ((Entry *)mem->TLB[i])->time = time(&seconds);
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
                    ((Entry *)mem->pageTable[i])->time = time(&seconds);
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
                    if (address->offset) free(address->offset);

                    if (address) free(address);
                }
                free(data);
            }
            if (f->frameNumber) free(f->frameNumber);
            free(f);
        }
    }

    for (int i = 0; i < PAGE_ENTRIES; i++) {
        Entry *entry = mem->pageTable[i];
        if (entry) continue;
        // if (entry->pageNumber) free(entry->pageNumber);
        // if (entry->frameNumber) free(entry->frameNumber);
        free(entry);
    }

    if (mem->TLB) free(mem->TLB);
    if (mem->pageTable) free(mem->pageTable);
    if (mem->physicalMemory) free(mem->physicalMemory);
    if (mem) free(mem);
}