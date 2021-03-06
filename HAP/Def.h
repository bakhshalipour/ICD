/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#ifndef Def_h
#define Def_h

#include <vector>
#include <algorithm>
#include <string>

#define GENERATE_ENERGY_TRACE

#define SIZE 512 //KB
#define ASSOCIATIVITY 16
#define BLOCK_SIZE 64 //byte
#define ONE_BIT_MASK(n) ((1LL << (n)) - 1)
#define LOG_EVERY_QUERY_COUNT 1000000
#define SAMPLE_SET_COUNT 40
#define SAMPLE_SET_EVERY 1024
#define SAMPLE_SET_TYPES 5
#define SAMPLE_POINT 100000000 //every 100 million instructions
#define L_RATIO 3

typedef long long unsigned llu;

struct BitLen {
    int block;
    int sets;
};

struct CBlock {
    llu tag;
    bool isNVM;
    bool isDirty;
    bool isChance;
    std::string value;
};

struct QRet { //what query returns
    bool hit;
    bool dirtyEviction;
    CBlock evictedBlock;
};

struct Threshold {
    int hi;
    int lo;
};

#endif /* Def_h */
