#include "VirtualMemory.h"
#include "DRAM_Cache.h"
#include "Performance.h"
#include <string.h>

int vmEnabled = 0; //false initially
int lastReplaced = 1;
Address pageTableAddress;

struct TLBEntry {
    short vpn;
    struct PageTableEntry PTE;

};
struct TLBEntry TLB[2];

int processVmReadWrite(Address addr, int value, bool write) {

    if (vmEnabled) { 
        Address physicalAddress;
        short virtualPageNumber = addr >> 10;
        short offset = addr & 0x3FF;

        lastReplaced = 1 - lastReplaced;
        if (TLB[0].vpn == virtualPageNumber) {
            physicalAddress = (TLB[0].PTE.physicalPageNumber << 10) | offset;
            
            perfTlbHit(TLB[0].PTE.physicalPageNumber);
            if (write) {
                writeWithCache(physicalAddress, value);
            } else {
                return readWithCache(physicalAddress);
            }
            
        } else if (TLB[1].vpn == virtualPageNumber) {
            physicalAddress = (TLB[1].PTE.physicalPageNumber << 10) | offset;
            perfTlbHit(TLB[1].PTE.physicalPageNumber);
            if (write) {
                writeWithCache(physicalAddress, value);
            } else {
                return readWithCache(physicalAddress);
            }
            
        } else {
            perfPageFault(virtualPageNumber);
            Address ptAddress = pageTableAddress + 4*virtualPageNumber;
            int result = readWithCache(ptAddress);
            memcpy(&TLB[lastReplaced].PTE, &result, 4);
            TLB[lastReplaced].vpn = virtualPageNumber;
            
            physicalAddress = (TLB[lastReplaced].PTE.physicalPageNumber << 10) | offset;
            perfTlbMiss(TLB[lastReplaced].PTE.physicalPageNumber);

            if (write) {
                writeWithCache(physicalAddress, value);
            } else {
                return readWithCache(physicalAddress);
            }

        }
        

    } else {

        if (write) {
            writeWithCache(addr, value);
        } else {
            return readWithCache(addr);
        }

    }

}

int vmRead(Address addr) {
    return processVmReadWrite(addr, -1, false);
}

void vmWrite(Address addr, int value) {
    processVmReadWrite(addr, value, true);
}

// control of the virtual memory.  
// When disabled the memory addresses are interpreted as physical addresses - no translation occurs.
void vmDisable() {
    vmEnabled = 0;
}

void vmEnable(Address pageTable) {
    vmEnabled = 1;
    if (!vmEnabled) {
        pageTableAddress = pageTable;
        TLB[0].vpn = 100;
        TLB[1].vpn = 100;
    }
    
}

void tlbFlush() {

}