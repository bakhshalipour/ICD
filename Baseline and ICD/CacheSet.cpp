/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "CacheSet.h"
#include <algorithm>

CacheSet::CacheSet(int initAssociativity, int initBlockOffset): blockArr(initAssociativity, CacheBlock(initBlockOffset)), associativity(initAssociativity) {
    usageHistory.clear();
    
    for (int i = 0; i < associativity; i++) {
        empty.push_back(i); //all indexes are empty
    }
    
}

CacheSet::~CacheSet() {
    
}

const CacheBlock* CacheSet::insert(lli newAddr, bool dirty, string value) {
    
    if (empty.size() != 0) { //there are some empty places; 
        const CacheBlock* ret = (blockArr.at(empty.at(0)).set(newAddr, dirty, value));
        usageHistory.insert(usageHistory.begin(), empty.at(0));
        empty.erase(empty.begin());
        return ret;
    }
    
    //there are no empty places
    //LRU
    int insertIndex = usageHistory.at(usageHistory.size() - 1);
    usageHistory.pop_back(); //remove last element
    usageHistory.insert(usageHistory.begin(), insertIndex); //the new query placed at the front
    return blockArr.at(insertIndex).set(newAddr, dirty, value);
}

pair<bool, const CacheBlock*> CacheSet::request(lli queryAddr, bool queryDirty, string value) {
    
    for (int i = 0; i < associativity; i++) {
        
        if (blockArr.at(i) == queryAddr) { //if hit
            vector<int>::iterator search = find(usageHistory.begin(), usageHistory.end(), i);
            
            if (search == usageHistory.end()) {
                cout << "Hit Occured While Its Index Doesn't Exist In usageHistory\n";
                exit(EXIT_FAILURE);
            }
            
            usageHistory.erase(search); //remove from former place
            usageHistory.insert(usageHistory.begin(), i); //place it in the front
            
            //check for dirty
            if (queryDirty) {
                blockArr.at(i).setDirty();
            }

            blockArr.at(i).setVal(value); //set new value
            return make_pair(true, nullptr);
        }
        
    }
    
    //miss
    return make_pair(false, insert(queryAddr, queryDirty, value));
}
