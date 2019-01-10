/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "CompleteCache.h"

int CompleteCache::cuckooRowCount = 0;
int CompleteCache::cuckooRowCount_log2 = 0;
int CompleteCache::tileCount_log2 = 0;
int CompleteCache::coherenceUnit_log2 = 0;
int CompleteCache::cuckooWayCount = 0;

CompleteCache::CompleteCache(int initRowCount, int initAssociativity, int cuckooSetCount, int cuckooSetCount_log2, int initCuckooWayCount, int initBlockOffset, unsigned int initThreshold, int initIndexSize, int initRowCount_log2, int initTileCount_log2, int initCoherenceUnit_log2): fVec({f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15}), associativity(initAssociativity), blockOffset(initBlockOffset), threshold(initThreshold), hits(0), misses(0), writeBackCount(0), componentCuckoo(cuckooSetCount, initCuckooWayCount, initBlockOffset, fVec, initThreshold), componentNormal(initRowCount, initAssociativity, initBlockOffset, initIndexSize), readHits(0), totalReadAccess(0) {
    cuckooRowCount = cuckooSetCount;
    cuckooRowCount_log2 = cuckooSetCount_log2;
    tileCount_log2 = initTileCount_log2;
    coherenceUnit_log2 = initCoherenceUnit_log2;
    cuckooWayCount = initCuckooWayCount;
}

CompleteCache::~CompleteCache() {
    
}

QueryRet CompleteCache::query(lli addr, bool l1EvictDirty, bool affectReadHitRatio, string value) {
    pair<bool, const CacheBlock*> l2Result = componentNormal.request(addr, l1EvictDirty, affectReadHitRatio, value);
    pair<bool, const CuckooBlock*> cuckooResult;
    QueryRet toRet;

    if (affectReadHitRatio == true) {
        totalReadAccess++;
    }
    
    if (l2Result.first == true) { //hit
        hits++;

        if (affectReadHitRatio == true) {
            readHits++;
        }

        toRet.hit = true;
        toRet.dirtyEviction = false;
        toRet.evictedContent.addr = -1;
    } else { //miss
        cuckooResult = componentCuckoo.remove(addr); //lookup cuckoo
        
        if (cuckooResult.first == true) { //found in cuckoo
            hits++;
            
            if (affectReadHitRatio == true) {
                readHits++;
            }

            toRet.hit = true;
            //block missed in the normal section and was removed from cuckoo section and moved (Cache::request) to the normal section, now we have to deal with the block evicted from normal section
        } else {
            misses++;
            toRet.hit = false;
            //block missed in the normal section and missed in the cuckoo section too! it was fetched from main memory; now we have to deal with the block evicted from normal section
        }

        toRet.dirtyEviction = false;
        toRet.evictedContent.addr = -1;

        if (l2Result.second->isDirty() == true) { //dirty eviction from l2
            //block evicted from l2 was dirty
            cuckooResult = componentCuckoo.insert(l2Result.second->getContent().addr, l2Result.second->getContent().value);
            //block inserted in the cuckoo section

            if (cuckooResult.first == true) { //dirty eviction from cuckoo
                //a block was evicted from cuckoo section (cuckoo blocks are always dirty)
                writeBackCount++;
                toRet.dirtyEviction = true;
                toRet.evictedContent.addr = cuckooResult.second->getContent().addr;
                toRet.evictedContent.value = cuckooResult.second->getContent().value;
                delete cuckooResult.second;
            }

        }
        
    }

    return toRet;
    
}

lli CompleteCache::getHitCount() const {
    return hits;
}

lli CompleteCache::getMissCount() const {
    return misses;
}

lli CompleteCache::getWriteBackCount() const {
    return writeBackCount;
}

const Cuckoo& CompleteCache::getComponentCuckoo() const {
    return componentCuckoo;
}

const Cache& CompleteCache::getComponentNormal() const {
    return componentNormal;
}