/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Cuckoo.h"

Cuckoo::Cuckoo(int initRowCount, int wayCount, int initBlockOffset, vector<lli (*)(lli)> initF, unsigned int initThreshold): rowCount(initRowCount), associativity(wayCount), blockOffset(initBlockOffset), fIndex(0), writeBackCount(0), threshold(initThreshold), hits(0), misses(0), dispAvgPerAcc({0, 0}), dispAvgPerIns({0, 0}) {
    
    for (int i = 0; i < associativity; i++) {
        CuckooWay newCol(rowCount, initF.at(i), blockOffset);
        content.push_back(newCol);
    }
    
}

Cuckoo::~Cuckoo() {
    
}

pair<bool, const CuckooBlock*> Cuckoo::remove(lli addr) {
    pair<bool, const CuckooBlock*> ret;
    dispAvgPerAcc.counter++;
    
    for (int i = 0; i < associativity; i++) {
        ret = content.at(i).remove(addr);
        
        if (ret.first == true) {
            hits++;
            return ret; //hit
        }
        
    }
    
    //miss
    misses++;
    ret.first = false;
    ret.second = nullptr;
    return ret;
}

pair<bool, const CuckooBlock*> Cuckoo::insert(lli addr, string value) {
    CuckooBlock *looper = new CuckooBlock(*(content.at(fIndex).insert(addr, 0, value)));
    fIndex++;
    fIndex %= associativity;
    dispAvgPerIns.counter++;
    dispAvgPerAcc.counter++;
    
    while (looper->isValid == true) {
        looper->counter++;
        dispAvgPerIns.value++;
        dispAvgPerAcc.value++;
        
        if (looper->counter >= threshold) {
            writeBackCount++; //evict and write back to PCM
            return make_pair(true, looper); //caller should delete it!
        }
        
        *looper = *(content.at(fIndex).insert(looper->getContent().addr, looper->counter, looper->getContent().value));
        fIndex++;
        fIndex %= associativity;
    }

    delete looper;
    return make_pair(false, nullptr); //no dirty eviction
}

lli Cuckoo::getWriteBackCount() const {
    return writeBackCount;
}

lli Cuckoo::getHitCount() const {
    return hits;
}

lli Cuckoo::getMissCount() const {
    return misses;
}
