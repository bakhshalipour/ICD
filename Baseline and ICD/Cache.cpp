/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Cache.h"

Cache::Cache(int setCount, int initAssociativity, int initBlockOffset, int initIndexSize): setArr(setCount, CacheSet(initAssociativity, initBlockOffset)), writeBackCount(0), hitCount(0), missCount(0), bitsBeforeIndex(initBlockOffset + 2), indexSize(initIndexSize), totalReadAccess(0), readHits(0) {
    rowCount = setCount;
    associativity = initAssociativity;
    
}

Cache::~Cache() {
    
}

lli Cache::getWriteBackCount() const{
    return writeBackCount;
}

lli Cache::getHitCount() const {
    return hitCount;
}

lli Cache::getMissCount() const {
    return missCount;
}

pair<bool, const CacheBlock*> Cache::request(lli queryAddr, bool queryDirty, bool affectReadHitRatio, string value) {
    lli setIndex = queryAddr >> bitsBeforeIndex;
    setIndex <<= (64 - indexSize);
    setIndex >>= 1;
    setIndex &= 0x7fffffffffffffff;
    setIndex >>= (63 - indexSize);
    pair<bool, const CacheBlock*> ret = setArr.at(setIndex).request(queryAddr, queryDirty, value);
    
    if (affectReadHitRatio) {
        totalReadAccess++;
    }
    
    if (ret.first == true) {
        hitCount++;
        
        if (affectReadHitRatio) {
            readHits++;
        }
        
    } else {
        missCount++;
        
        if (ret.second->isDirty() == true) {
            writeBackLog.insert(ret.second->content.addr >> ret.second->shiftCount);
            writeBackCount++;
        }
        
    }
    
    return ret;
}