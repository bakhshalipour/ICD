/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "Cache.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>

Cache::Cache(int size, int associativity) {
    setCount = (size * 1024) / associativity / 64;
    
    for (int i = 0; i < SAMPLE_SET_TYPES; i++) {
        staticThresholds[i].lo = 0;
        staticThresholds[i].hi = associativity / (SAMPLE_SET_TYPES - 1) * i;
    }
    
    for (int i = 0; i < setCount; i++) {
        //create a CSet
        CSet *tmpCSet = new CSet(associativity);
        tmpCSet->threshold = &threshold;
        
        if ((i % SAMPLE_SET_EVERY) < SAMPLE_SET_COUNT) {
            tmpCSet->isSample = true;
            tmpCSet->type = ((i % SAMPLE_SET_EVERY)) % SAMPLE_SET_TYPES;
            tmpCSet->threshold = &(staticThresholds[tmpCSet->type]);
        }
        
        sets.push_back(tmpCSet);
    }
    
    hitCount = 0;
    missCount = 0;
    readHitCount = 0;
    readCount = 0;
    writebackCount = 0;
    queryCounter = 0;
    bitLen.block = log2(BLOCK_SIZE);
    bitLen.sets = log2(sets.size());
}

QRet Cache::query(llu addr, bool l1EvictDirty, bool isNVM, bool affectReadHitRatio, string value) {
    queryCounter++;
    
    if ((queryCounter % SAMPLE_POINT) == 0) {
        samplePoint();
    }
    
    if (affectReadHitRatio) {
        readCount++;
    }
    
    //extract set from addr
    llu addrSet = addr >> bitLen.block;
    addrSet &= ONE_BIT_MASK(bitLen.sets);
    CSet *querySet = sets.at(addrSet);
    //extract tag from addr
    CBlock queryBlock;
    queryBlock.tag = addr >> (bitLen.block + bitLen.sets);
    queryBlock.isNVM = isNVM;
    queryBlock.isDirty = l1EvictDirty;
    queryBlock.isChance = true;
    queryBlock.value = value;
    
    QRet ret = querySet->query(queryBlock);
    
    if (ret.hit) {
        
        if (affectReadHitRatio) {
            readHitCount++;
        }
        
        hitCount++;
    } else {
        missCount++;
    }
    
    if (ret.dirtyEviction) {
        writebackCount++;
    }
    
    static llu histo = 0;
    
    if (queryCounter - histo >= LOG_EVERY_QUERY_COUNT) {
        cout << setfill(' ') << "QUERY NO " << setw(10) << queryCounter << " FINISHED! ";
        cout.flush();
        cout << "\n";
        histo = queryCounter;
    }
    
    return ret;
}

void Cache::samplePoint() {
    vector<double> evalTypes;
    
    for (int type = 0; type < SAMPLE_SET_TYPES; type++) {
        llu totalCost = 0, totalAccess = 0;
        
        for (int region = 0; region < setCount; region += SAMPLE_SET_EVERY) {
            
            for (int local = type; local < SAMPLE_SET_COUNT; local += SAMPLE_SET_TYPES) {
                llu setNum = region + local;
                assert(sets.at(setNum)->isSample);
                assert(sets.at(setNum)->type == type);
                
                totalAccess += sets.at(setNum)->accessCounter;
                totalCost += sets.at(setNum)->costCounter;
                //reset counters
                sets.at(setNum)->accessCounter = 0;
                sets.at(setNum)->costCounter = 0;
            }
            
        }
        
        evalTypes.push_back(1.0 * totalCost / totalAccess); //average over same types
    }
    
    //find minimum cost
    double minVal = evalTypes.at(0);
    int minIndx = 0;
    
    for (int i = 1; i < evalTypes.size(); i++) {
        
        if (evalTypes.at(i) <= minVal) {
            minVal = evalTypes.at(i);
            minIndx = i;
        }
        
    }
    
    //set threshold to the minimum cost
    threshold.hi = staticThresholds[minIndx].hi;
    threshold.lo = ( (minIndx = 0) ? (0) : (staticThresholds[minIndx - 1].hi) );
}