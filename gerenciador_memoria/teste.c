#include <stdio.h>
#include <math.h>

void decimalToBinary(int decimal);
void separeteAddress(int *binary);
void printBinary(int *binary, int size);
int binaryToDecimal(int *binary, int size);

int main() {
    int decimal = 16916;
    decimalToBinary(decimal);
    return 0;
}

void decimalToBinary(int decimal) {
    int binary[32];

    for (int j = 0; j < 32; j++) {
        binary[j] = 0;
    }

    int i = 0;
    while (decimal > 0) {
        binary[i] = decimal % 2;
        decimal = decimal / 2;
        i++;
    }

    separeteAddress(binary);
}

void separeteAddress(int *binary) {
    int logicalAddress[16];
    int pageNumber[8];
    int offset[8];

    for (int i = 0; i < 8; i++) {
        offset[i] = binary[i];
    }
    for (int i = 8; i < 16; i++) {
        pageNumber[i - 8] = binary[i];
    }
    for (int i = 16; i < 32; i++) {
        logicalAddress[i - 16] = binary[i];
    }
    printf("Offset:             ");
    printBinary(offset, 8);
    printf("            %d\n", binaryToDecimal(offset, 8));
    printf("Page Number:        ");
    printBinary(pageNumber, 8);
    printf("            %d\n", binaryToDecimal(pageNumber, 8));
    printf("Logical Address:    ");
    printBinary(logicalAddress, 16);
    printf("    %d\n", binaryToDecimal(logicalAddress, 16));
}

void printBinary(int *binary, int size) {
    for (int i = size - 1; i >= 0; i--) {
        printf("%d", binary[i]);
    }
}

int binaryToDecimal(int *binary, int size) {
    int decimal = 0;
    for (int i = 0; i < size; i++) {
        decimal += binary[i] * pow(2, i);
    }
    return decimal;
}