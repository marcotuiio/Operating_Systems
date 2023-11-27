## Memory Management

### Marco Túlio Alves de Barros     27/10/2023

## Introduction

This project aims to simulate a memory manager using the C programming language. It utilizes a Translation Lookaside Buffer (TLB) and a page table for address translation and physical address lookup.

The secondary memory (or disk) is simulated by a `.bin` file defined in the "libs.h" file.

## Operating Logic

The program converts logical addresses, read from an input `.txt` file, into physical addresses. To do this, the logical addresses are converted into binary and separated into three parts: the offset (least significant 8 bits), the page number (middle 8 bits), and the address (most significant 16 bits).

The program first looks for the page in the TLB. If it's not found there, it looks in the page table. If it's not found in the page table either, it performs a lookup in the disk (the `.bin` file), and the page is loaded into the page table and the TLB.

To perform the lookup in the `.bin` file, the program uses the page number (fseek of page * PAGE_SIZE). It reads 256 bytes starting from the address of the page in question. The physical address is composed of the start address of the page plus the offset.

### Page Replacement Algorithm

* FIFO:

The FIFO page replacement algorithm evicts the oldest page first, i.e., the page that has been in memory the longest.

In this code, when a page fault occurs (the page is not found in the TLB or page table), a new frame is created and it loads the page from the .bin file into this frame. If there’s no space left in memory, replace the oldest frame (the one that was loaded into memory first) with the new frame. To keep track of the oldest frame the program uses the OLDEST_PHYSICAL variable, which is updated in a round-robin manner.

The same logic applies to the TLB. When a new entry needs to be added to the TLB and the TLB is full, replace the oldest entry with the new one. The OLDEST_TBL variable keeps track of the oldest entry in the TLB.

* LRU:

The LRU page replacement algorithm evicts the least recently used page first, i.e., the page that has not been accessed for the longest time.

In the LRU function, when a page fault occurs and there’s no space left in memory, it searches for the frame that has the oldest timeFrame value and replace it with the new frame. This means its replacing the frame that was least recently used.

The same logic applies to the TLB. When a new entry needs to be added to the TLB and the TLB is full, replace the entry with the oldest timeEntry value with the new entry.

In both FIFO and LRU, after loading a page into memory, its necessary to update the TLB and page table to reflect the new state of memory.

#### Structures Used

* Memory:
    Stores information about the virtual memory, including the TLB, page table, and physical memory.

* Entry:
    Represents an entry in the TLB or page table with page number and frame information.

* Frame:
    Represents a frame in physical memory with information about its availability and data.

* Address:
    Contains information about a memory address, including binary representation and separated fields.

I hope this helps! If you have more questions, feel free to ask me @marcotulio.barros@uel.br! 😊