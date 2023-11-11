#include "DRAM.h"
#include "Performance.h"
#include <stdio.h>
#include <string.h>

#define MEMORY_SIZE 49152

char memory[MEMORY_SIZE];

int readDram(Address addr) {
  
    int result;
    memcpy(&result, &memory[addr], 4);
    perfDramRead(addr, result);
    return result;

}
void writeDram(Address addr, int value) {
   
    memcpy(&memory[addr], &value, 4);
    perfDramWrite(addr, value);

}

void readDramCacheLine(Address addr, CacheLine line) {
 
    memcpy(&line[0], &memory[addr], 32);
    perfDramCacheLineRead(addr, line);

}

void writeDramCacheLine(Address addr, CacheLine line) {

    memcpy(&memory[addr], &line[0], 32);
    perfDramCacheLineWrite(addr, line);

}