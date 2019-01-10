/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "CuckooWay.h"
#include <iostream>

CuckooWay::CuckooWay(int initRowCount, lli (*hashFunction)(lli addr), int initBlockOffset): rowCount(initRowCount), blockOffset(initBlockOffset), hash(hashFunction), content(initRowCount, CuckooBlock(initBlockOffset)), former(nullptr) {
    
}

CuckooWay::~CuckooWay() {
    
    if (former != nullptr) {
        delete former;
    }
    
}

const CuckooBlock* CuckooWay::insert(lli newAddr, int counter, string value) {
    return content.at(hash(newAddr)).set(newAddr, counter, value);
}

pair<bool, const CuckooBlock*> CuckooWay::remove(lli query) {
    CuckooBlock& elem = content.at(hash(query));
    
    if (elem != query) {
        return make_pair(false, nullptr);
    }
    
    if (former != nullptr) {
        delete former;
    }
    
    former = new CuckooBlock(elem);
    elem.evict();
    return make_pair(true, former);
}