#include "DRAM_Cache.h"
#include "Performance.h"
#include <stdbool.h>
#include <string.h>

int currentTime = 0;

struct Line {
    CacheLine data;
    bool valid;
    bool dirty;
    unsigned int tag;
    int timeStamp;
};

struct Set {
    struct Line A;
    struct Line B;
};

struct Set Cache[4];

void initCache() {
    for (int i=0; i<4; i++) {
        struct Line *lineA = &Cache[i].A;
        struct Line *lineB = &Cache[i].B;
        lineA->valid = false;
        lineB->valid = false;
    }
}

int processReadWrite(Address addr, int value, bool write) {

    currentTime++;
    int offset = addr & 0b11111;
    int setIndex = (addr >> 5) & 0b11;
    unsigned int tag = (addr >> 7);
    unsigned int lineAddress = addr & 0xFFFFFFE0;

    struct Set *set = &(Cache[setIndex]);
    struct Line *lineA = &(set->A);
    struct Line *lineB = &(set->B);
    int readResult;

    if (lineA->valid && lineA->tag == tag) {
            lineA->timeStamp = currentTime;
            if (write) {
                lineA->dirty = true;
                memcpy(&lineA->data[offset], &value, 4);
                return -1;
            } else {
                memcpy(&readResult, &lineA->data[offset], 4);
                return readResult;
            }
            
            perfCacheHit(addr, setIndex, readResult);
    } else if (lineB->valid && lineB->tag == tag) {
            lineB->timeStamp = currentTime;
            if (write) {
                lineB->dirty = true;
                memcpy(&lineB->data[offset], &value, 4);
                return -1;
            } else {
                memcpy(&readResult, &lineB->data[offset], 4);
                return readResult;
            }
            perfCacheHit(addr, setIndex, readResult);
    } else { 

       if (!lineA->valid) { 
            
            readDramCacheLine(lineAddress, lineA->data);
            perfCacheMiss(addr, setIndex, 0, true);
            lineA->valid = true;
            lineA->tag = tag;
            lineA->timeStamp = currentTime;
            if (write) {
                lineA->dirty = true;
                memcpy(&lineA->data[offset], &value, 4);
                return -1;
            } else {
                memcpy(&readResult, &lineA->data[offset], 4);
                return readResult;
            }
            


       } else if (!lineB->valid) {
            
            readDramCacheLine(lineAddress, lineB->data);
            perfCacheMiss(addr, setIndex, 1, true);
            lineB->valid = true;
            lineB->tag = tag;
            lineB->timeStamp = currentTime;
            if (write) {
                lineB->dirty = true;
                memcpy(&lineB->data[offset], &value, 4);
                return -1;
            } else {
                memcpy(&readResult, &lineB->data[offset], 4);
                return readResult;
            }

       } else { 

            if (lineA->timeStamp > lineB->timeStamp) {
                if (lineB->dirty) {
                    unsigned int dirtyTag = lineB->tag;
                    int dirtySetIndex = setIndex;
                    unsigned int dirtyLineAddress = (dirtyTag << 7) | (dirtySetIndex<<5); 
                    writeDramCacheLine(dirtyLineAddress, lineB->data);
                    lineB->dirty = false;
                } 

                lineB->tag = tag;
                lineB->timeStamp = currentTime;
                lineB->valid = true;

                readDramCacheLine(lineAddress, lineB->data);
                perfCacheMiss(addr, setIndex, 1, false);

                if (write) {
                    memcpy(&lineB->data[offset], &value, 4);
                    lineB->dirty = true;
                    return -1;
                } else {
                    memcpy(&readResult, &lineB->data[offset], 4);
                    return readResult;
                    }

            } else {
                
                if (lineA->dirty) {
                    unsigned int dirtyTag = lineA->tag;
                    int dirtySetIndex = setIndex;
                    unsigned int dirtyLineAddress = (dirtyTag << 7) | (dirtySetIndex<<5);
                    writeDramCacheLine(dirtyLineAddress, lineA->data);
                    lineA->dirty = false;
                }

                lineA->tag = tag;
                lineA->timeStamp = currentTime;
                lineA->valid = true;

                readDramCacheLine(lineAddress, lineA->data);
                perfCacheMiss(addr, setIndex, 0, false);

                if (write) {
                    memcpy(&lineA->data[offset], &value, 4);
                    lineA->dirty = true;
                    return -1;
                } else {
                    memcpy(&readResult, &lineA->data[offset], 4);
                    return readResult;
                }
            }

       }
    }

    return 0;
}

int readWithCache(Address addr) {
    return processReadWrite(addr, 0, false);
}

void writeWithCache(Address addr, int value) {
    processReadWrite(addr, value, true);
}

void flushCache() {
    for (int i=0; i<4; i++) {
        struct Set *set = &(Cache[i]);
        struct Line *lineA = &(set->A);
        struct Line *lineB = &(set->B);
        if (lineA->dirty) {
            unsigned int tagA = lineA->tag;
            unsigned int addressA = (tagA << 7) | (i << 5);
            writeDramCacheLine(addressA, lineA->data);
            lineA->dirty = false;
        }
        if (lineB->dirty) {
            unsigned int tagB = lineB->tag;
            unsigned int addressB = (tagB << 7) | (i << 5);
            writeDramCacheLine(addressB, lineB->data);
            lineB->dirty = false;
        }
        
    }
}