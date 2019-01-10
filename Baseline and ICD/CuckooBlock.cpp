/*
The source code of "Reducing Writebacks Through In-Cache Displacement" paper, which is accepted in ACM TODAES 2019.

In this repo, we provide the source codes that are used in our ACM TODAES 2019 paper. This includes the implementation of our proposal, named ICD, as well as the implementation of competitor methods like WADE and HAP. We also provide the source code which acts as an interface between our simulator and Simics. This so-called Simics Interface extends the interface of Flexus simulator with Simics and gets the value of memory accesses, into the bargain.

Please cite the following paper when using the provided source codes:

M. Bakhshalipour, A. Faraji, A. Vakil-Ghahani, F. Samandi, P. Lotfi-Kamran, and H. Sarbazi-Azad, "Reducing Writebacks Through In-Cache Displacement," in ACM Transactions on Design Automation of Electronic Systems (TODAES), 2019.
*/

#include "CuckooBlock.h"

CuckooBlock::CuckooBlock(int initBlockOffset): Block(initBlockOffset), counter(0), former(nullptr) {
}

CuckooBlock::CuckooBlock(const CuckooBlock& src): Block(src), counter(src.counter), former(nullptr) {
    
}

CuckooBlock::~CuckooBlock() {
    
    if (former != nullptr) {
        delete former;
    }
    
}

const CuckooBlock* CuckooBlock::set(lli newAddr, int newCounter, std::string value) {
    
    if (former != nullptr) {
        delete former;
    }
    
    former = new CuckooBlock(*this);
    content.addr = newAddr;
    content.value = value;
    counter = newCounter;
    isValid = true;
    return former;
}

CuckooBlock CuckooBlock::operator=(const CuckooBlock &right) {
    counter = right.counter;
    former = nullptr;
    Block::operator=(right);
    return (*this);
}