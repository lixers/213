//
//  csim.c
//  cachelab
//
//  Created by Xin Li on 10/2/14.
//  Copyright (c) 2014 Xin Li. All rights reserved.
//
//  Andrew ID: xinli1
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "cachelab.h"

/* struct declaration */
// line struct
typedef struct {
    int validBit;       // valid bits per line
    int tag;            // tag of the line
    int lastTimeUse;    // time left to be evict
} line;

// set struct, a set contains several lines
typedef struct {
    line *lines;
} set;

// cache struct, one cache contains several sets
typedef struct {
    int setNum;        // set number
    int setLine;       // lines per set
    set *sets;
} cache;

// contruct a cache parameter strcut,
// make it easier to manage the parameters.
typedef struct {
    int v;          // valid bit
    int s;          // there are intotal 2 to the s cache sets
	int b;          // a single cache line block size 2**b bytes 
	int E;          // number of cache lines per set
	int S;          // number of sets,
                    // we can get it from the equation: S = 2**s
	int B;          // cache line block size,
                    // we can get it from the equation: B = 2**b
    
	int hits;       // the number of hits
	int misses;     // the number of misses
	int evicts;     // the number of evicts
} cacheParameter;

/* methods declaration */
int getSetBits(int address, int s, int b);
int getTagBits(int address, int s, int b);
void getInfo(int argc, char **argv, cacheParameter *para, char *traceFile);
void initCache(cache *ch, int s, int E);
void traceProcess(cache *ch, int addr, char type, cacheParameter *para);
void printInfo();
void freeCache(cache *ch);

int main(int argc, char** argv) {
    cache ch;
    cacheParameter para;
    char traceFile[255];
    char operationType;
    int address;
    int size;
    
    para.hits = 0;
    para.misses = 0;
    para.evicts = 0;
    
    // get the parameter information here
    getInfo(argc, argv, &para, traceFile);   
    initCache(&ch, para.s, para.E);        // malloc memory to the cache
    
    FILE *fp;                              // open the file
    fp = fopen(traceFile, "r");
    if (fp != NULL) {
        while (fscanf(fp, " %c %x,%d", &operationType, &address, &size) > 0){
            switch (operationType) {
                case 'M':
                    traceProcess(&ch, address, operationType, &para);
                    break;
                case 'S':
                    traceProcess(&ch, address, operationType, &para);
                    break;
                case 'L':
                    traceProcess(&ch, address, operationType, &para);
                    break;
                default:
                    break;
            }
        }
    }
    fclose(fp);
    
    printSummary(para.hits, para.misses, para.evicts);
    // we need to free the memory we malloc.
    freeCache(&ch);
    return 0;
}

/* *
 * argc is the number of argument, 
 * and argv[](*argv) is the first content of the argument
 * from the command line. So the **argv is the 
 * list of command line input seperated by ' '.
 * getopt parse the command line input. 
 * ':' means there's a parameter after the character.
 * *optarg is the parameter pointer.
 * */
void getInfo(int argc, char **argv, cacheParameter *para,
             char *traceFile){
    int opt;
    
    // according to the recitation slides, use a switch method.
    while ((opt = getopt(argc, argv, "vs:E:b:t:")) != -1) {
        switch (opt) {
            case 'v':
                para->v = 1;
                break;
            case 's':
                para->s = atoi(optarg);
                break;
            case 'E':
                para->E = atoi(optarg);
                break;
            case 'b':
                para->b = atoi(optarg);
                break;
            case 't':
                strcpy(traceFile, optarg);
                break;
            default:
                printInfo();
                exit(-1);
                break;
        }
    }
    
    // if didn't get all the s, E, b and taceFile,
    // fail this getInfo operation
    if(para->s == 0 || para->E == 0 ||
       para->b == 0 || traceFile == NULL) {
        printInfo();
        exit(-1);
    }
}

/* to get the set value from the address */
int getSet(int address, int s, int b) {
	int temp = 0x7fffffff >> (31 - s);
	return ((address >> b ) & temp);
}

/* to get the tag value from the address */
int getTag(int address, int s, int b) {
	return address >> (s + b);
}

/* this method initial the the cache memory */
void initCache(cache *ch, int s, int E) {
    int i, j;
    ch->setNum = (1 << s);
    ch->setLine = E;
    // malloc memory to the sets
    ch->sets = (set *)malloc(ch->setNum * sizeof(set));
    // for each set, we need malloc memory to the lines
    for (i = 0; i < ch->setNum; i++) {
        ch->sets[i].lines = (line *)malloc(ch->setLine*sizeof(line));
        for (j = 0; j < ch->setLine; j++) {
            // at the same time, we can set the valid bit to zero.
            // just to ensure.
            ch->sets[i].lines[j].validBit = 0;
        }
    }
}

/* free the cache */
void freeCache(cache *ch) {
    int i;
    for (i = 0; i < ch->setNum; i++) {
        if(ch->sets[i].lines != NULL){
            free(ch->sets[i].lines);
        }
    }
    if(ch->sets != NULL){
        free(ch->sets);
    }
}

void traceProcess(cache *ch, int addr, char type, cacheParameter *para) {
    int hitFlag, evictIdx, tag, setNum, i, tempTime1, tempTime2;
    hitFlag = 0;
    evictIdx = 0;
    tag = getTag(addr, para->s, para->b);
    setNum = getSet(addr, para->s, para->b);
    
    set tempSet = ch->sets[setNum];
    for (i = 0; i < para->E; i++) {
        // if valid bit == 1 and tag is match, then we got a hit.
        if (tempSet.lines[i].validBit == 1 &&
            tempSet.lines[i].tag == tag){
            para->hits++;
            // we use this line, so we set lastTimeUse to 0.
            tempSet.lines[i].lastTimeUse = 0;
            hitFlag = 1;
            if (type == 'M') {
                // if the operation type is M then we get one more hit.
                para->hits++;
            }
        }
        // if we didn't get the hit, we need keep looking.
        else if (tempSet.lines[i].validBit == 1 &&
                 tempSet.lines[i].tag != tag) {
            tempSet.lines[i].lastTimeUse++;
            tempTime1 = tempSet.lines[i].lastTimeUse;
            tempTime2 = tempSet.lines[evictIdx].lastTimeUse;
            if(tempSet.lines[evictIdx].validBit == 1 &&
                 tempTime1 > tempTime2) {
                evictIdx = i;
            }
        }
        // this means we have a empty line in the cache.
        else {
            evictIdx = i;
        }
    }
    
    // if we got the hit, then we just simply return.
    if (hitFlag == 1) {
        return;
    }
    
    // if we got a miss, then misses++, but if the type is 'M',
    // then next argument is ensured to be a hit.
    para->misses++;
    if (type == 'M') {
        para->hits++;
    }
    
    // if we missed, but we have an empty line,
    // we don't need to evict.
    if (tempSet.lines[evictIdx].validBit != 0) {
        para->evicts++;
    }
    tempSet.lines[evictIdx].validBit = 1;
    tempSet.lines[evictIdx].lastTimeUse = 0;
    tempSet.lines[evictIdx].tag = tag;
    return;
}

/* *
 * this print message simply copied from the writeup.
 */
void printInfo() {
    printf("Usage: ./csim [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>\n\
           • -h: Optional help flag that prints usage info\n\
           • -v: Optional verbose flag that displays trace info\n\
           • -s <s>: Number of set index bits \n\
                     \(S = 2**s is the number of sets)\n\
           • -E <E>: Associativity (number of lines per set) \n\
           • -b <b>: Number of block bits (B = 2**b is the block size)\n\
           • -t <tracefile>: Name of the valgrind trace to replay\n");
}


