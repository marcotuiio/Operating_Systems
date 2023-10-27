#include "memory.h"

#include "libs.h"

int FRAME_NUMBER = 0;

typedef struct memory {
    int hitTlb;             // contador de acertos na TLB
    int missTlb;            // contador de erros na TLB
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
    int *physicalAddress;
} Address;

void memoryManagement(void *memory, FILE *testFile, PageReplaceAlgorith pageReplaceAlgorith) {
    Memory *mem = memory;

    FILE *bin = fopen(BIN_FILE, "r");

    while (!feof(testFile)) {
        int logicalDecimal;
        fscanf(testFile, "%d", &logicalDecimal);

        int *logicalBinary = decimalToBinary(logicalDecimal, 32);
        int *pageNumber = getPageNumber(logicalBinary);
        int page = binaryToDecimal(pageNumber, 8);
        int *offsetBin = getOffSet(logicalBinary);
        // int offset = binaryToDecimal(offsetBin, 8);
        // int toPrint;
        free(offsetBin);
        free(logicalBinary);

        int *foundFrame = lookForPageTLB(mem, pageNumber);
        if (!foundFrame) {  // TLB MISS
            foundFrame = lookForPageTable(mem, pageNumber);
            if (!foundFrame) {  // PAGE TABLE MISS

                fseek(bin, page, SEEK_SET);
                Frame *frame = createFrame(decimalToBinary(FRAME_NUMBER, 8));
                Address **data = frame->data;
                FRAME_NUMBER++;

                for (int i = 0; i < 256; i++) {
                    Address *address = createAddress();
                    address->offset = decimalToBinary(i, 8);
                    address->pageNumber = pageNumber;
                    fscanf(bin, "%d", &address->value);
                    data[i] = address;
                    frame->available = false;

                    // if (offset == i) toPrint = address->value;
                }

                for (int i = 0; i < mem->frames; i++) {
                    if (!mem->physicalMemory[i]) {
                        mem->physicalMemory[i] = frame;
                        break;
                    }
                }

                // Add to TLB
                // Add to page table
            }
            // foundFrame = pageReplaceAlgorith(mem, address);
            if (!foundFrame) {
                printf("ERRO Falha ao alocar frame!\n");
                return;
            }
        }
    }
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
            if (((Entry *)mem->TLB[i])->pageNumber[j] != page[j]) {
                break;
            } else {
                if (j == 7) {
                    int *foundFrame = ((Entry *)mem->TLB[i])->frameNumber;
                    // printf("TLB HIT\n");
                    // printBinary(foundFrame, 8);
                    mem->hitTlb++;
                    return foundFrame;
                }
            }
        }
    }
    mem->missTlb++;
    return NULL;
}

void *lookForPageTable(void *memory, int *page) {
    Memory *mem = memory;
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        for (int j = 0; j < 8; j++) {
            if (((Entry *)mem->pageTable[i])->pageNumber[j] != page[j]) {
                break;
            } else {
                if (j == 7) {
                    int *foundFrame = ((Entry *)mem->pageTable[i])->frameNumber;
                    // printf("PAGE TABLE HIT\n");
                    // printBinary(foundFrame, 8);
                    return foundFrame;
                }
            }
        }
    }
    return NULL;
}

void freeMemory(void *memory) {
    Memory *mem = memory;
    for (int i = 0; i < TLB_ENTRIES; i++)
        if (mem->TLB[i]) free(mem->TLB[i]);

    for (int i = 0; i < PAGE_ENTRIES; i++)
        if (mem->pageTable[i]) free(mem->pageTable[i]);

    for (int i = 0; i < mem->frames; i++)
        if (mem->physicalMemory[i]) {  // frame
            for (int j = 0; j < PAGE_SIZE; j++) {
                Frame **frame = (Frame **)mem->physicalMemory;
                Frame *f = frame[i];
                Address **data = f->data;
                Address *address = data[j];
                if (address) free(address);
            }
            free(((Frame *)mem->physicalMemory[i])->data);
        }

    free(mem->TLB);
    free(mem->pageTable);
    free(mem->physicalMemory);
    free(mem);
}