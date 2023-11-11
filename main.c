#include "Performance.h"
#include "ApplicationMemory.h"
#include "VirtualMemory.h"
#include "DRAM_Cache.h"
#include <stdio.h>

#define PAGE_SIZE 1024

void setupPageTable()
{
    perfNote("\ncreating the page table\n");
    // build a page table that puts the lowest virtual page in the highest physical page
    int phyPage = 48;
    for (int p = 0; p < PAGE_TABLE_ENTRIES; ++p) {
        --phyPage;
        int pte = 0;
        if (phyPage >= 0) {
            pte = 0x40 | phyPage;
        }
        writeMemory(p * 4, pte);
    }
    vmEnable(0);
}

void reportError(Address addr, int expected, int found)
{
    char msg[200];
    sprintf(msg, "ERROR: bad value @ %08x (%d)  value: %d  expected: %d\n", addr, addr, found, expected);
    printf("%s", msg);
    perfNote(msg);
}

int main(int argc, char**argv)
{
    initCache();
    clearPerformanceCounters();
    perfNote("Setting up page table\n");
    setupPageTable();
    perfNote("\nVirtual memory enabled\nstarting test with vm\n");
    int value;
    for (int i = 8*PAGE_SIZE; i < 32*PAGE_SIZE; i += PAGE_SIZE) {
        writeMemory(i, i);
    }
    for (int i = 8*PAGE_SIZE; i < 32*PAGE_SIZE; i += PAGE_SIZE) {
        value = readMemory(i);
        if (value != i) {
            reportError(i, i, value);
        }
    }
    flushMemory();
    struct PerformanceCounters pc;
    getPerformanceCounters(&pc);
    printPerformanceInfo(&pc);
    clearPerformanceCounters();


    printf("done\n");
}
